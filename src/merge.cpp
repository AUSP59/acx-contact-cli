#include "merge.hpp"
#include "csv.hpp"
#include "utils.hpp"
#include <vector>

namespace acx {

static bool parse_contacts(const std::string& fmt, const std::filesystem::path& in_file, std::vector<Contact>& out, std::string& err){
    if (fmt=="jsonl"){
        std::string all = read_file_mmap(in_file);
        size_t pos=0;
        while (pos < all.size()){
            size_t end = all.find('\n', pos);
            if (end == std::string::npos) end = all.size();
            std::string line = all.substr(pos, end-pos);
            pos = end + 1;
            if (line.empty()) continue;
            try { J j = J::parse(line); out.push_back(Contact::from_json(j)); } catch(...) { /* skip */ }
        }
        return true;
    } else if (fmt=="csv"){
        std::string all = read_file_mmap(in_file);
        std::vector<std::vector<std::string>> rows; std::string perr;
        if (!csv_parse_rfc4180(all, rows, perr)){ err = "CSV parse error: " + perr; return false; }
        for (size_t i=1;i<rows.size();++i){
            auto& f = rows[i]; if (f.size()<6) continue;
            Contact c;
            c.id = f[0];
            c.name = f[1];
            c.email = f[2];
            c.phone = f[3];
            c.note = f[4];
            c.created_at = f[5];
            out.push_back(std::move(c));
        }
        return true;
    }
    err = "unknown format";
    return false;
}

static bool newer(const std::string& a, const std::string& b){
    return a > b; // ISO-8601 lexicographic
}

bool merge_from_file(Storage& st, ContactBook& book, const std::string& fmt, const std::filesystem::path& in_file, MergeStrategy strat, std::string& err){
    std::vector<Contact> incoming;
    if (!parse_contacts(fmt, in_file, incoming, err)) return false;
    // index existing by email/phone
    for (auto& c : incoming){
        auto e = to_lower_ascii(c.email);
        auto p = normalize_phone(c.phone);
        auto itE = book.email_to_i.find(e);
        auto itP = book.phone_to_i.find(p);
        size_t idx = (size_t)-1;
        if (itE != book.email_to_i.end()) idx = itE->second;
        else if (itP != book.phone_to_i.end()) idx = itP->second;
        if (idx == (size_t)-1){
            std::string id; if (!st.add(book, c, id, err)) { /* skip invalid */ }
        } else {
            if (strat == MergeStrategy::PreferNewer){
                if (newer(c.created_at, book.contacts[idx].created_at)){
                    std::string id = book.contacts[idx].id;
                    st.edit(book, id, c.name, c.email, c.phone, c.note, err);
                }
            } else if (strat == MergeStrategy::PreferExisting){
                // keep current, maybe enrich note if missing
                if (book.contacts[idx].note.empty() && !c.note.empty()){
                    std::string id = book.contacts[idx].id;
                    st.edit(book, id, "", "", "", c.note, err);
                }
            } else {
                // Skip (default)
            }
        }
    }
    return st.save(book);
}

} // namespace acx
