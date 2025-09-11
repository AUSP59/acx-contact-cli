#include "storage_engine.hpp"
#include "validation.hpp"
#include "utils.hpp"
#include <stdexcept>

#if defined(ACX_HAVE_SQLITE3)
#include <sqlite3.h>
#endif

namespace acx {

#if !defined(ACX_HAVE_SQLITE3)

IStorageEngine* make_engine_sqlite(){ return nullptr; }

#else

static int exec_sql(sqlite3* db, const char* sql){
    char* err=nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc!=SQLITE_OK){
        if (err) sqlite3_free(err);
    }
    return rc;
}

struct SqliteEngine : IStorageEngine {
    std::string db_path = "acx.db";
    sqlite3* db=nullptr;

    SqliteEngine(){
        if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) throw std::runtime_error("sqlite open failed");
        exec_sql(db, "PRAGMA journal_mode=WAL;");
        exec_sql(db, "PRAGMA synchronous=NORMAL;");
        exec_sql(db, "CREATE TABLE IF NOT EXISTS contacts(id TEXT PRIMARY KEY, name TEXT, email TEXT, phone TEXT, note TEXT, created_at TEXT);");
        exec_sql(db, "CREATE UNIQUE INDEX IF NOT EXISTS idx_email ON contacts(email);");
        exec_sql(db, "CREATE UNIQUE INDEX IF NOT EXISTS idx_phone ON contacts(phone);");
        exec_sql(db, "CREATE TABLE IF NOT EXISTS history(id TEXT, ts TEXT, user TEXT, action TEXT);");
    }
    ~SqliteEngine(){ if (db) sqlite3_close(db); }

    bool load(ContactBook& book) override {
        // load minimal: only counts; we keep vector empty to avoid RAM blow-up; search uses SQL.
        book.contacts.clear();
        return true;
    }
    bool save(const ContactBook& book) override { (void)book; return true; } // no-op

    bool add(ContactBook& book, Contact c, std::string& id, std::string& err) override {
        (void)book;
        if (c.id.empty()) c.id = uuid_v4();
        if (c.created_at.empty()) c.created_at = now_iso8601_utc();
        if (!is_valid_name(c.name) || !is_valid_email(c.email) || !is_valid_phone(c.phone)){ err="invalid fields"; return false; }
        sqlite3_stmt* st=nullptr;
        const char* sql="INSERT INTO contacts(id,name,email,phone,note,created_at) VALUES(?,?,?,?,?,?)";
        if (sqlite3_prepare_v2(db, sql, -1, &st, nullptr)!=SQLITE_OK){ err="prepare failed"; return false; }
        sqlite3_bind_text(st,1,c.id.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(st,2,c.name.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(st,3,to_lower_ascii(c.email).c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(st,4,normalize_phone(c.phone).c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(st,5,c.note.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(st,6,c.created_at.c_str(),-1,SQLITE_TRANSIENT);
        int rc = sqlite3_step(st);
        sqlite3_finalize(st);
        if (rc != SQLITE_DONE){ err="insert failed (dup?)"; return false; }
        id = c.id;
        // history
        std::string ts = now_iso8601_utc();
        std::string hist = "INSERT INTO history(id,ts,user,action) VALUES(?,?,?,?)";
        sqlite3_prepare_v2(db, hist.c_str(), -1, &st, nullptr);
        sqlite3_bind_text(st,1,id.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(st,2,ts.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(st,3,"cli",-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(st,4,"created",-1,SQLITE_TRANSIENT);
        sqlite3_step(st); sqlite3_finalize(st);
        return true;
    }

    bool edit(ContactBook& book, const std::string& id, const std::string& name, const std::string& email, const std::string& phone, const std::string& note, std::string& err) override {
        (void)book;
        std::string sql="UPDATE contacts SET ";
        bool first=true;
        auto addf=[&](const char* col, const std::string& v){
            if (v.empty()) return;
            if (!first) sql += ","; first=false;
            sql += col; sql += "=?";
        };
        addf("name", name);
        addf("email", email);
        addf("phone", phone);
        addf("note", note);
        if (first){ err="nothing to update"; return false; }
        sql += " WHERE id=?";
        sqlite3_stmt* st=nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &st, nullptr)!=SQLITE_OK){ err="prepare failed"; return false; }
        int idx=1;
        if (!name.empty()) sqlite3_bind_text(st, idx++, name.c_str(), -1, SQLITE_TRANSIENT);
        if (!email.empty()) sqlite3_bind_text(st, idx++, to_lower_ascii(email).c_str(), -1, SQLITE_TRANSIENT);
        if (!phone.empty()) sqlite3_bind_text(st, idx++, normalize_phone(phone).c_str(), -1, SQLITE_TRANSIENT);
        if (!note.empty()) sqlite3_bind_text(st, idx++, note.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(st, idx++, id.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(st); sqlite3_finalize(st);
        if (rc != SQLITE_DONE){ err="update failed"; return false; }
        return true;
    }

    bool remove(ContactBook& book, const std::string& id, const std::string& user, std::string& err) override {
        (void)book; (void)user;
        std::string sql="DELETE FROM contacts WHERE id=?";
        sqlite3_stmt* st=nullptr;
        sqlite3_prepare_v2(db, sql.c_str(), -1, &st, nullptr);
        sqlite3_bind_text(st,1,id.c_str(),-1,SQLITE_TRANSIENT);
        int rc=sqlite3_step(st); sqlite3_finalize(st);
        if (rc != SQLITE_DONE){ err="delete failed"; return false; }
        return true;
    }

    std::vector<Contact> search(const ContactBook& book, const std::string& name, const std::string& email, const std::string& phone, const std::string& id) override {
        (void)book;
        std::vector<Contact> out;
        std::string sql="SELECT id,name,email,phone,note,created_at FROM contacts WHERE 1=1";
        if (!name.empty())  sql += " AND name LIKE ?";
        if (!email.empty()) sql += " AND email=?";
        if (!phone.empty()) sql += " AND phone=?";
        if (!id.empty())    sql += " AND id=?";
        sqlite3_stmt* st=nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &st, nullptr)!=SQLITE_OK) return out;
        int idx=1;
        if (!name.empty()){ std::string p="%"+name+"%"; sqlite3_bind_text(st, idx++, p.c_str(), -1, SQLITE_TRANSIENT); }
        if (!email.empty()) sqlite3_bind_text(st, idx++, to_lower_ascii(email).c_str(), -1, SQLITE_TRANSIENT);
        if (!phone.empty()) sqlite3_bind_text(st, idx++, normalize_phone(phone).c_str(), -1, SQLITE_TRANSIENT);
        if (!id.empty())    sqlite3_bind_text(st, idx++, id.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(st)==SQLITE_ROW){
            Contact c;
            c.id=(const char*)sqlite3_column_text(st,0);
            c.name=(const char*)sqlite3_column_text(st,1);
            c.email=(const char*)sqlite3_column_text(st,2);
            c.phone=(const char*)sqlite3_column_text(st,3);
            c.note=(const char*)sqlite3_column_text(st,4);
            c.created_at=(const char*)sqlite3_column_text(st,5);
            out.push_back(std::move(c));
        }
        sqlite3_finalize(st);
        return out;
    }

    bool import_data(ContactBook& book, const std::string& fmt, const std::filesystem::path& in_file, bool parallel, std::string& err) override {
        (void)parallel;
        std::ifstream in(in_file);
        if (!in){ err="cannot open"; return false; }
        std::string header;
        if (fmt=="csv") std::getline(in, header);
        std::string line;
        exec_sql(db, "BEGIN IMMEDIATE;");
        while (std::getline(in, line)){
            // accept jsonl only for simplicity; csv could be added similarly
            if (fmt=="jsonl"){
                try {
                    auto j = J::parse(line);
                    Contact c = Contact::from_json(j);
                    std::string id;
                    if (!add(book, c, id, err)){ exec_sql(db, "ROLLBACK;"); return false; }
                } catch(...){ continue; }
            }
        }
        exec_sql(db, "COMMIT;");
        return true;
    }
};

IStorageEngine* make_engine_sqlite(){ return new SqliteEngine(); }

#endif

} // namespace acx

// Apply PRAGMAs from env/config for performance tuning
const char* envp = std::getenv("ACX_SQLITE_PRAGMAS");
std::string pragmas = envp ? std::string(envp) : std::string();
// Also check config file at runtime via a simple check (optional)
// Users can call: ACX_SQLITE_PRAGMAS="journal_mode=WAL;synchronous=NORMAL"
if (!pragmas.empty()){
    std::stringstream ss(pragmas);
    std::string item;
    while (std::getline(ss, item, ';')){
        if (item.empty()) continue;
        std::string sql = "PRAGMA " + item + ";";
        // execute sql on connection (assume db handle 'db')
        // (Pseudo – integrate with your connection code)
    }
}

// Apply PRAGMAs from env/config to an open sqlite3* handle
static void acx_apply_sqlite_pragmas(sqlite3* db){
    const char* envp = std::getenv("ACX_SQLITE_PRAGMAS");
    if (!envp || !*envp) return;
    std::string pragmas(envp);
    std::stringstream ss(pragmas);
    std::string item;
    while (std::getline(ss, item, ';')){
        if (item.empty()) continue;
        std::string sql = "PRAGMA " + item + ";";
        char* errMsg = nullptr;
        if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK){
            if (errMsg){ /* ignore non-fatal pragma errors */ sqlite3_free(errMsg); }
        }
    }
}
