acx::Storage::Storage(){
    const char* df = std::getenv("ACX_DATA_FILE");
    const char* bd = std::getenv("ACX_BACKUP_DIR");
    const char* al = std::getenv("ACX_AUDIT_LOG");
    if (df && *df) data_file = df;
    if (bd && *bd) backup_dir = bd;
    if (al && *al) audit_log = al;
}

#include "storage.hpp"
#include <cstdlib>
#include "audit.hpp"
#include "jsonl.hpp"
#include "validation.hpp"
#include "utils.hpp"
#include "index.hpp"
#include "disk_index.hpp"
#include "csv.hpp"
#include "hmac.hpp"
#include "zstd_wrap.hpp"
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <set>\n#include <unordered_map>

namespace fs = std::filesystem;

namespace acx {

bool Storage::load(ContactBook& book) const {
    book.contacts.clear();
    if (!file_exists_nonempty(data_file)) return true;
    auto data = read_file(data_file);
    auto rows = parse_jsonl(data);
    for (auto& j : rows) book.contacts.push_back(Contact::from_json(j));
    // persistent index
    PersistentIndex pi; pi.load_or_build(data_file, book);
    return true;
}

bool Storage::save(const ContactBook& book) const {
    std::vector<J> rows;
    for (auto& c : book.contacts) rows.push_back(c.to_json());
    // Prepare offsets while dumping
    std::string out; out.reserve(rows.size()*64);
    std::vector<uint64_t> offsets; offsets.reserve(rows.size());
    uint64_t cur = 0;
    std::vector<std::pair<std::string,uint64_t>> email_kv, phone_kv;
    for (size_t i=0;i<rows.size();++i){
        offsets.push_back(cur);
        std::string line = rows[i].dump(0);
        out += line; out += "\n"; cur += (uint64_t)line.size()+1;
        const auto& c = book.contacts[i];
        email_kv.emplace_back(to_lower_ascii(c.email), offsets[i]);
        phone_kv.emplace_back(normalize_phone(c.phone), offsets[i]);
    }
    fs::create_directories(data_file.parent_path());
    bool ok = write_file_atomic(data_file, out);
    if (ok){
        PersistentIndex pi; pi.save(data_file, book);
        DiskIndex di; di.build(data_file, email_kv, phone_kv);
    }
    return ok;
}

bool Storage::backup_last() const {
    if (!file_exists_nonempty(data_file)) return true;
    fs::create_directories(backup_dir);
    auto ts = now_iso8601_utc();
    std::string stamp = ts;
    for(char& c : stamp) if (c==':'||c=='-') c=''; // yyyymmddThhmmssZ-ish
    std::string data = read_file(data_file);
    std::string h = sha256_hex(data);
    fs::path bak = backup_dir / ("acx-data-" + stamp + "-" + h.substr(0,12) + ".json.bak");
    bool ok = true;
    std::string content = data;
    const char* use_z = std::getenv("ACX_ZSTD");
    if (use_z && std::string(use_z)=="1" && zstd_available()){
        std::string z; if (zstd_compress(data, z)) { content.swap(z); bak.replace_extension(".bak.zst"); }
    }
    ok = write_file_atomic(bak, content);
    const char* key = std::getenv("ACX_HMAC_KEY");
    if (ok && key && *key){
        std::string sig = hmac_sha256_hex(key, content);
        write_file_atomic(bak.string()+".sig", sig+"\n");
    }
    return ok;
}

bool Storage::append_audit(const std::string& line) const {
    std::ofstream out(audit_log, std::ios::app);
    if (!out) return false;
    out << line << "\n";
    return true;
}

static bool email_exists(const ContactBook& book, const std::string& email, const std::string& skip_id=""){
    std::string e = to_lower_ascii(email);
    auto it = book.email_to_i.find(e);
    if (it != book.email_to_i.end()){
        const auto& c = book.contacts[it->second];
        if (c.id != skip_id) return true;
    }
    return false;
}
static bool phone_exists(const ContactBook& book, const std::string& phone, const std::string& skip_id=""){
    std::string p = normalize_phone(phone);
    auto it = book.phone_to_i.find(p);
    if (it != book.phone_to_i.end()){
        const auto& c = book.contacts[it->second];
        if (c.id != skip_id) return true;
    }
    return false;
}

bool add_contact(Storage& st, ContactBook& book, const std::string& name,
                 const std::string& email, const std::string& phone,
                 const std::string& note, std::string user, std::string& out_id, std::string& err) {
    // Enforce max contacts (safety limit), overridable via env ACX_MAX_CONTACTS
    {
        size_t maxc = 1000000; // sane default
        if (const char* e = std::getenv("ACX_MAX_CONTACTS")) { try { maxc = (size_t)std::stoull(e); } catch (...) {} }
        if (book.contacts.size() >= maxc){ err = "Max contacts limit reached"; return false; }
    }

    if (!is_valid_name(name)){ err="Invalid name"; return false; }
    if (!is_valid_email(email)){ err="Invalid email"; return false; }
    if (!is_valid_phone(phone)){ err="Invalid phone"; return false; }
    if (email_exists(book, email)){ err="Email already exists"; return false; }
    if (phone_exists(book, phone)){ err="Phone already exists"; return false; }

    Contact c;
    c.id = uuid_v4();
    c.name = name;
    c.email = to_lower_ascii(email);
    c.phone = normalize_phone(phone);
    c.note = note;
    c.created_at = now_iso8601_utc();
    c.history.push_back(HistoryEntry{c.created_at, user, "created"});

    book.contacts.push_back(c);
    size_t idx = book.contacts.size()-1;
    book.email_to_i[to_lower_ascii(c.email)] = idx;
    book.phone_to_i[normalize_phone(c.phone)] = idx;
    if (!/* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
st.save(book)) { err="Failed to save"; return false; }
    st.backup_last();
    std::ostringstream audit;
    audit << now_iso8601_utc() << ",CREATE," << c.id << "," << user;
    st.append_audit(audit.str());
    out_id = c.id;
    return true;
}

bool edit_contact(Storage& st, ContactBook& book, const std::string& id,
                  const std::string& name, const std::string& email,
                  const std::string& phone, const std::string& note,
                  std::string user, std::string& err) {
    for (size_t i=0;i<book.contacts.size();++i){ auto& c = book.contacts[i]; if (c.id == id) {
        if (!name.empty()) {
            if (!is_valid_name(name)){ err="Invalid name"; return false; }
            c.name = name;
        }
        if (!email.empty()) {
            if (!is_valid_email(email)){ err="Invalid email"; return false; }
            if (email_exists(book, email, id)){ err="Email already exists"; return false; }
            // update email index
            book.email_to_i.erase(to_lower_ascii(c.email));
            c.email = to_lower_ascii(email);
            book.email_to_i[c.email] = i;
        }
        if (!phone.empty()) {
            if (!is_valid_phone(phone)){ err="Invalid phone"; return false; }
            if (phone_exists(book, phone, id)){ err="Phone already exists"; return false; }
            // update phone index
            book.phone_to_i.erase(normalize_phone(c.phone));
            c.phone = normalize_phone(phone);
            book.phone_to_i[c.phone] = i;
        }
        if (!note.empty()) c.note = note;

        auto ts = now_iso8601_utc();
        c.history.push_back(HistoryEntry{ts, user, "edited"});
        if (!/* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
st.save(book)) { err="Failed to save"; return false; }
        st.backup_last();
        std::ostringstream audit;
        audit << now_iso8601_utc() << ",EDIT," << c.id << "," << user;
        st.append_audit(audit.str());
        return true;
    }}
    err = "Contact not found";
    return false;
}

bool delete_contact(Storage& st, ContactBook& book, const std::string& id,
                    std::string user, std::string& err) {
    for (size_t i=0;i<book.contacts.size();++i) if (book.contacts[i].id==id) {
        auto ts = now_iso8601_utc();
        book.contacts[i].history.push_back(HistoryEntry{ts, user, "deleted"});
        // keep tombstone? For simplicity, we will remove the record from persisted set, but we log in audit.
        // erase from indexes
        book.email_to_i.erase(to_lower_ascii(book.contacts[i].email));
        book.phone_to_i.erase(normalize_phone(book.contacts[i].phone));
        book.contacts.erase(book.contacts.begin()+i);
        // reindex tail
        for (size_t k=i;k<book.contacts.size();++k){
            book.email_to_i[to_lower_ascii(book.contacts[k].email)] = k;
            book.phone_to_i[normalize_phone(book.contacts[k].phone)] = k;
        }
        if (!/* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
st.save(book)) { err="Failed to save"; return false; }
        st.backup_last();
        std::ostringstream audit;
        audit << now_iso8601_utc() << ",DELETE," << id << "," << user;
        st.append_audit(audit.str());
        return true;
    }}
    err = "Contact not found";
    return false;
}

std::vector<Contact> search_contacts(const ContactBook& book,
                                     const std::string& name,
                                     const std::string& email,
                                     const std::string& phone,
                                     const std::string& id) {
    std::vector<Contact> out;
    for (auto& c : book.contacts) {
        bool ok = true;
        if (!name.empty())  ok = ok && (acx::to_lower_ascii(c.name).find(acx::to_lower_ascii(name)) != std::string::npos);
        if (!email.empty()) ok = ok && (acx::to_lower_ascii(c.email).find(acx::to_lower_ascii(email)) != std::string::npos);
        if (!phone.empty()) ok = ok && (acx::normalize_phone(c.phone).find(acx::normalize_phone(phone)) != std::string::npos);
        if (!id.empty())    ok = ok && (c.id == id);
        if (ok) out.push_back(c);
    }
    return out;
}

static std::string csv_escape(const std::string& s){
    bool need = s.find_first_of(","\n") != std::string::npos;
    if (!need) return s;
    std::string t = s; size_t pos=0;
    while ((pos = t.find('"', pos)) != std::string::npos){ t.insert(pos, 1, '"'); pos += 2; }
    return std::string(""")+t+""";
}

bool export_data(const ContactBook& book, const std::string& fmt, const fs::path& out_file, std::string& err){
    
if (fmt == "csv"){
    std::ostringstream o;
    o << "id,name,email,phone,note,created_at\n";
    /* SUMMIT_SORT */
    auto sorted = book.contacts; std::sort(sorted.begin(), sorted.end(), [](const acx::Contact& a, const acx::Contact& b){return a.id < b.id;});
    for (auto& c : sorted){
        o << csv_escape(c.id) << "," << csv_escape(c.name) << ","
          << csv_escape(c.email) << "," << csv_escape(c.phone) << ","
          << csv_escape(c.note) << "," << csv_escape(c.created_at) << "\n";
    }
    bool w=write_file_atomic(out_file, o.str()); if(w) acx::audit_log("export", {{"format","csv"},{"out",out_file}}); return w;
} else if (fmt == "tsv"){
    std::ostringstream o;
    o << "id\tname\temail\tphone\tnote\tcreated_at\n";
    /* SUMMIT_SORT */
    auto sorted = book.contacts; std::sort(sorted.begin(), sorted.end(), [](const acx::Contact& a, const acx::Contact& b){return a.id < b.id;});
    for (auto& c : sorted){
        o << c.id << "\t" << c.name << "\t" << c.email << "\t" << c.phone << "\t" << c.note << "\t" << c.created_at << "\n";
    }
    bool w=write_file_atomic(out_file, o.str()); if(w) acx::audit_log("export", {{"format","csv"},{"out",out_file}}); return w;

} else if (fmt == "tsv"){
    auto lines = split(data, '\n');
    bool header = true;
    for (auto& line : lines){
        if (line.empty()) continue;
        if (header) { header = false; continue; }
        auto parts = split(line, '\t');
        if (parts.size() < 6) continue;
        Contact c;
        c.id = parts[0];
        c.name = parts[1];
        c.email = parts[2];
        c.phone = parts[3];
        c.note = parts[4];
        c.created_at = parts[5];
        std::string tmp;
        st.add(book, c, tmp, err);
    }
    return /* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
/* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
st.save(book);
} else if (fmt == "jsonl"){


        std::ostringstream o;
        o << "id,name,email,phone,note,created_at\n";
        /* SUMMIT_SORT */
    auto sorted = book.contacts; std::sort(sorted.begin(), sorted.end(), [](const acx::Contact& a, const acx::Contact& b){return a.id < b.id;});
    for (auto& c : sorted){
            o << csv_escape(c.id) << "," << csv_escape(c.name) << ","
              << csv_escape(c.email) << "," << csv_escape(c.phone) << ","
              << csv_escape(c.note) << "," << csv_escape(c.created_at) << "\n";
        }
        bool w=write_file_atomic(out_file, o.str()); if(w) acx::audit_log("export", {{"format","csv"},{"out",out_file}}); return w;
    
} else if (fmt == "tsv"){
    auto lines = split(data, '\n');
    bool header = true;
    for (auto& line : lines){
        if (line.empty()) continue;
        if (header) { header = false; continue; }
        auto parts = split(line, '\t');
        if (parts.size() < 6) continue;
        Contact c;
        c.id = parts[0];
        c.name = parts[1];
        c.email = parts[2];
        c.phone = parts[3];
        c.note = parts[4];
        c.created_at = parts[5];
        std::string tmp;
        st.add(book, c, tmp, err);
    }
    return /* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
/* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
st.save(book);
} else if (fmt == "jsonl"){

        std::vector<J> rows; for (auto& c : book.contacts) rows.push_back(c.to_json());
        return write_file_atomic(out_file, dump_jsonl(rows));
    } else {
        err="Unknown format"; return false;
    }
}

bool import_data(Storage& st, ContactBook& book, const std::string& fmt, const fs::path& in_file, std::string& err){
    auto data = read_file(in_file);
    if (fmt == "jsonl"){
        auto rows = parse_jsonl(data);
        for (auto& j : rows) {
            auto c = Contact::from_json(j);
            if (email_exists(book, c.email) || phone_exists(book, c.phone)) continue; // skip duplicates
            book.contacts.push_back(c);
    size_t idx = book.contacts.size()-1;
    book.email_to_i[to_lower_ascii(c.email)] = idx;
    book.phone_to_i[normalize_phone(c.phone)] = idx;
        }
        return /* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
/* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
st.save(book);
    } else 
if (fmt == "csv"){
    std::ostringstream o;
    o << "id,name,email,phone,note,created_at\n";
    /* SUMMIT_SORT */
    auto sorted = book.contacts; std::sort(sorted.begin(), sorted.end(), [](const acx::Contact& a, const acx::Contact& b){return a.id < b.id;});
    for (auto& c : sorted){
        o << csv_escape(c.id) << "," << csv_escape(c.name) << ","
          << csv_escape(c.email) << "," << csv_escape(c.phone) << ","
          << csv_escape(c.note) << "," << csv_escape(c.created_at) << "\n";
    }
    bool w=write_file_atomic(out_file, o.str()); if(w) acx::audit_log("export", {{"format","csv"},{"out",out_file}}); return w;
} else if (fmt == "tsv"){
    std::ostringstream o;
    o << "id\tname\temail\tphone\tnote\tcreated_at\n";
    /* SUMMIT_SORT */
    auto sorted = book.contacts; std::sort(sorted.begin(), sorted.end(), [](const acx::Contact& a, const acx::Contact& b){return a.id < b.id;});
    for (auto& c : sorted){
        o << c.id << "\t" << c.name << "\t" << c.email << "\t" << c.phone << "\t" << c.note << "\t" << c.created_at << "\n";
    }
    bool w=write_file_atomic(out_file, o.str()); if(w) acx::audit_log("export", {{"format","csv"},{"out",out_file}}); return w;

} else if (fmt == "tsv"){
    auto lines = split(data, '\n');
    bool header = true;
    for (auto& line : lines){
        if (line.empty()) continue;
        if (header) { header = false; continue; }
        auto parts = split(line, '\t');
        if (parts.size() < 6) continue;
        Contact c;
        c.id = parts[0];
        c.name = parts[1];
        c.email = parts[2];
        c.phone = parts[3];
        c.note = parts[4];
        c.created_at = parts[5];
        std::string tmp;
        st.add(book, c, tmp, err);
    }
    return /* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
/* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
st.save(book);
} else if (fmt == "jsonl"){


        auto lines = split(data, '\n');
        bool header = true;
        for (auto& line : lines){
            if (line.empty()) continue;
            if (header) { header = false; continue; }
            // naive CSV split (handles no quoted commas)
            auto parts = split(line, ',');
            if (parts.size() < 6) continue;
            Contact c;
            c.id = parts[0];
            c.name = parts[1];
            c.email = parts[2];
            c.phone = parts[3];
            c.note = parts[4];
            c.created_at = parts[5];
            if (c.id.empty()) c.id = uuid_v4();
            if (c.created_at.empty()) c.created_at = now_iso8601_utc();
            c.history.push_back(HistoryEntry{now_iso8601_utc(), "import", "created"});
            if (email_exists(book, c.email) || phone_exists(book, c.phone)) continue;
            book.contacts.push_back(c);
    size_t idx = book.contacts.size()-1;
    book.email_to_i[to_lower_ascii(c.email)] = idx;
    book.phone_to_i[normalize_phone(c.phone)] = idx;
        }
        return /* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
/* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
st.save(book);
    } else {
        err="Unknown format"; return false;
    }
}

} // namespace acx

bool Storage::verify_backup_signature(const std::filesystem::path& bak_file, const std::string& key) const {
    std::string data = read_file(bak_file);
    std::string want = hmac_sha256_hex(key, data);
    std::string got = read_file(bak_file.string()+".sig");
    if (!got.empty() && got.back()=='\n') got.pop_back();
    return want == got;
}


#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

namespace acx {

struct LineTask { size_t seq; std::string line; };

static bool parse_csv_line_basic(const std::string& line, Contact& c){
    // naive: id,name,email,phone,note,created_at
    auto parts = split(line, ',');
    if (parts.size() < 6) return false;
    c.id = parts[0].empty() ? uuid_v4() : parts[0];
    c.name = parts[1];
    c.email = parts[2];
    c.phone = parts[3];
    c.note = parts[4];
    c.created_at = parts[5].empty() ? now_iso8601_utc() : parts[5];
    return true;
}

bool import_data_parallel(Storage& st, ContactBook& book, const std::string& fmt, const std::filesystem::path& in_file, std::string& err){
    const int workers = std::max(1, env_int("ACX_IMPORT_WORKERS", (int)std::thread::hardware_concurrency()));
    const size_t max_record = env_size("ACX_RECORD_MAX_SIZE", 1024*1024);
    const size_t max_lines = env_size("ACX_IMPORT_MAX", 0); // 0 = unlimited

    std::ifstream in(in_file);
    if (!in){ err="Cannot open input"; return false; }

    // initialize dedup sets from existing book
    std::mutex dedup_mu;
    std::set<std::string> seen_email, seen_phone;
    /* SUMMIT_SORT */
    auto sorted = book.contacts; std::sort(sorted.begin(), sorted.end(), [](const acx::Contact& a, const acx::Contact& b){return a.id < b.id;});
    for (auto& c : sorted){ seen_email.insert(to_lower_ascii(c.email)); seen_phone.insert(normalize_phone(c.phone)); }

    std::mutex q_mu, out_mu;
    std::condition_variable q_cv;
    std::queue<LineTask> q;
    bool done = false;
    std::vector<std::pair<size_t, Contact>> outv;
    outv.reserve(1024);

    auto worker = [&](){
        while (true){
            LineTask t;
            {
                std::unique_lock<std::mutex> lk(q_mu);
                q_cv.wait(lk, [&]{ return done || !q.empty(); });
                if (q.empty()){
                    if (done) break;
                    else continue;
                }
                t = q.front(); q.pop();
            }
            if (t.line.size() > max_record) continue;
            Contact c;
            bool ok=false;
            try {
                if (fmt=="jsonl"){
                    auto j = J::parse(t.line);
                    c = Contact::from_json(j);
                    if (c.id.empty()) c.id = uuid_v4();
                    if (c.created_at.empty()) c.created_at = now_iso8601_utc();
                } else { // csv
                    ok = parse_csv_line_basic(t.line, c);
                    if (!ok) continue;
                }
                // validate
                if (!is_valid_name(c.name) || !is_valid_email(c.email) || !is_valid_phone(c.phone)) continue;
                // dedup global (thread-safe)
                {
                    std::lock_guard<std::mutex> lk(dedup_mu);
                    auto e = to_lower_ascii(c.email);
                    auto p = normalize_phone(c.phone);
                    if (seen_email.count(e) || seen_phone.count(p)) continue;
                    seen_email.insert(e); seen_phone.insert(p);
                }
                {
                    std::lock_guard<std::mutex> ok2(out_mu);
                    outv.emplace_back(t.seq, std::move(c));
                }
            } catch(...){ /* skip malformed */ }
        }
    };

    std::vector<std::thread> threads;
    for (int i=0;i<workers;i++) threads.emplace_back(worker);

    // producer
    std::string line; size_t seq=0; size_t lines=0;
    if (fmt=="csv" && std::getline(in, line)) { /* skip header */ }
    while (std::getline(in, line)){
        if (max_lines && ++lines > max_lines) break;
        {
            std::lock_guard<std::mutex> lk(q_mu);
            q.push(LineTask{seq++, line});
        }
        q_cv.notify_one();
    }
    {
        std::lock_guard<std::mutex> lk(q_mu);
        done = true;
    }
    q_cv.notify_all();
    for (auto& th : threads) th.join();

    // deterministic order by seq
    std::sort(outv.begin(), outv.end(), [](auto& a, auto& b){ return a.first < b.first; });
    for (auto& kv : outv){
        book.contacts.push_back(kv.second);
    }
    return /* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
/* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
st.save(book);
}

} // namespace acx

static bool env_on(const char* n){ const char* v=getenv(n); return v && std::string(v)=="1"; }

static bool fast_search_one(const Storage& st, const std::string& email,
                            const std::string& phone, Contact& out){
    if (!env_on("ACX_FAST_SEARCH")) return false;
    if (!file_exists_nonempty(st.data_file)) return false;
    // validate disk index matches data
    std::string data = read_file(st.data_file);
    std::string dh = sha256_hex(data);
    DiskIndex di;
    if (!di.valid_for_data_hash(dh)) return false;
    uint64_t off=0;
    bool found=false;
    if (!email.empty()){
        if (di.lookup_email(to_lower_ascii(email), off)) found=true;
    } else if (!phone.empty()){
        if (di.lookup_phone(normalize_phone(phone), off)) found=true;
    }
    if (!found) return false;
    // read one line at offset
    // naive: read entire file (mmap) and slice by newline around offset
    std::string all = read_file_mmap(st.data_file);
    if (off >= all.size()) return false;
    size_t start = off;
    size_t end = all.find('\n', start);
    if (end == std::string::npos) end = all.size();
    std::string line = all.substr(start, end-start);
    try {
        J j = J::parse(line);
        out = Contact::from_json(j);
        return true;
    } catch(...){ return false; }
}


static bool import_csv_rfc(Storage& st, ContactBook& book, const std::filesystem::path& in_file, std::string& err){
    std::string data = read_file_mmap(in_file);
    std::vector<std::vector<std::string>> rows; std::string perr;
    if (!csv_parse_rfc4180(data, rows, perr)){ err = "CSV parse error: " + perr; return false; }
    if (rows.empty()) return true;
    // Assume header: id,name,email,phone,note,created_at
    size_t i = 1;
    for (; i<rows.size(); ++i){
        auto& f = rows[i];
        if (f.size() < 6) continue;
        Contact c;
        c.id = f[0].empty()? uuid_v4() : f[0];
        c.name = f[1];
        c.email = f[2];
        c.phone = f[3];
        c.note = f[4];
        c.created_at = f[5].empty()? now_iso8601_utc() : f[5];
        std::string tmp; if (!st.add(book, c, tmp, err)) { /* skip dup/invalid */ }
    }
    return /* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
/* dry-run: skip persistence if ACX_DRY_RUN is set */
if (const char* __dr = std::getenv("ACX_DRY_RUN")) {
    (void)__dr; /* no-op, already updated in-memory 'book' */
    return true;
}
st.save(book);
}
