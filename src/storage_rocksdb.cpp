#include "storage_engine.hpp"
#include "validation.hpp"
#include "utils.hpp"
#include <stdexcept>
#if defined(ACX_HAVE_ROCKSDB)
#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>
#include <rocksdb/utilities/checkpoint.h>
#endif

namespace acx {

#if !defined(ACX_HAVE_ROCKSDB)

IStorageEngine* make_engine_rocksdb(){ return nullptr; }

#else

static std::string kEmailKey(const std::string& e){ return "email:" + to_lower_ascii(e); }
static std::string kPhoneKey(const std::string& p){ return "phone:" + normalize_phone(p); }
static std::string kContactKey(const std::string& id){ return "contact:" + id; }

struct RocksEngine : IStorageEngine {
    std::string path = "acx.rocksdb";
    rocksdb::DB* db=nullptr;
    RocksEngine(){
        rocksdb::Options opts;
        opts.create_if_missing = true;
        opts.max_background_jobs = 2;
        opts.bytes_per_sync = 1<<20;
        auto st = rocksdb::DB::Open(opts, path, &db);
        if (!st.ok()) throw std::runtime_error("rocksdb open failed");
    }
    ~RocksEngine(){ delete db; }

    bool load(ContactBook& book) override {
        book.contacts.clear(); // we operate directly on DB
        return true;
    }
    bool save(const ContactBook& book) override { (void)book; return true; }

    bool add(ContactBook& book, Contact c, std::string& id, std::string& err) override {
        (void)book;
        if (c.id.empty()) c.id = uuid_v4();
        if (c.created_at.empty()) c.created_at = now_iso8601_utc();
        if (!is_valid_name(c.name) || !is_valid_email(c.email) || !is_valid_phone(c.phone)){ err="invalid fields"; return false; }
        std::string eid = kEmailKey(c.email);
        std::string pid = kPhoneKey(c.phone);
        std::string existing;
        if (db->Get(rocksdb::ReadOptions(), eid, &existing).ok()){ err="duplicate email"; return false; }
        if (db->Get(rocksdb::ReadOptions(), pid, &existing).ok()){ err="duplicate phone"; return false; }
        auto j = c.to_json().dump();
        auto wb = rocksdb::WriteBatch();
        wb.Put(kContactKey(c.id), j);
        wb.Put(eid, c.id);
        wb.Put(pid, c.id);
        auto s = db->Write(rocksdb::WriteOptions(), &wb);
        if (!s.ok()){ err="write failed"; return false; }
        id = c.id;
        return true;
    }

    bool edit(ContactBook& book, const std::string& id, const std::string& name,
              const std::string& email, const std::string& phone, const std::string& note, std::string& err) override {
        (void)book;
        std::string j;
        if (!db->Get(rocksdb::ReadOptions(), kContactKey(id), &j).ok()){ err="not found"; return false; }
        Contact c = Contact::from_json(J::parse(j));
        if (!name.empty()) c.name = name;
        if (!email.empty()) c.email = email;
        if (!phone.empty()) c.phone = phone;
        if (!note.empty()) c.note = note;
        if (!is_valid_name(c.name) || !is_valid_email(c.email) || !is_valid_phone(c.phone)){ err="invalid fields"; return false; }
        // reindex if needed
        auto wb = rocksdb::WriteBatch();
        wb.Put(kContactKey(id), c.to_json().dump());
        // naive: update secondary indexes: delete old, add new
        wb.Delete(kEmailKey(Contact::from_json(J::parse(j)).email));
        wb.Delete(kPhoneKey(Contact::from_json(J::parse(j)).phone));
        wb.Put(kEmailKey(c.email), id);
        wb.Put(kPhoneKey(c.phone), id);
        auto s = db->Write(rocksdb::WriteOptions(), &wb);
        return s.ok();
    }

    bool remove(ContactBook& book, const std::string& id, const std::string& user, std::string& err) override {
        (void)book; (void)user;
        std::string j;
        if (!db->Get(rocksdb::ReadOptions(), kContactKey(id), &j).ok()){ err="not found"; return false; }
        Contact c = Contact::from_json(J::parse(j));
        auto wb = rocksdb::WriteBatch();
        wb.Delete(kContactKey(id));
        wb.Delete(kEmailKey(c.email));
        wb.Delete(kPhoneKey(c.phone));
        auto s = db->Write(rocksdb::WriteOptions(), &wb);
        return s.ok();
    }

    std::vector<Contact> search(const ContactBook& book, const std::string& name, const std::string& email, const std::string& phone, const std::string& id) override {
        (void)book;
        std::vector<Contact> out;
        std::string j;
        if (!id.empty()){
            if (db->Get(rocksdb::ReadOptions(), kContactKey(id), &j).ok()) out.push_back(Contact::from_json(J::parse(j)));
            return out;
        }
        if (!email.empty()){
            std::string cid;
            if (db->Get(rocksdb::ReadOptions(), kEmailKey(email), &cid).ok()){
                if (db->Get(rocksdb::ReadOptions(), kContactKey(cid), &j).ok()) out.push_back(Contact::from_json(J::parse(j)));
            }
            return out;
        }
        if (!phone.empty()){
            std::string cid;
            if (db->Get(rocksdb::ReadOptions(), kPhoneKey(phone), &cid).ok()){
                if (db->Get(rocksdb::ReadOptions(), kContactKey(cid), &j).ok()) out.push_back(Contact::from_json(J::parse(j)));
            }
            return out;
        }
        // name: scan (simple impl)
        std::unique_ptr<rocksdb::Iterator> it(db->NewIterator(rocksdb::ReadOptions()));
        for (it->Seek("contact:"); it->Valid() && it->key().starts_with("contact:"); it->Next()){
            Contact c = Contact::from_json(J::parse(it->value().ToString()));
            if (name.empty() || c.name.find(name) != std::string::npos) out.push_back(std::move(c));
        }
        return out;
    }

    bool import_data(ContactBook& book, const std::string& fmt, const std::filesystem::path& in_file, bool parallel, std::string& err) override {
        (void)parallel; (void)book;
        std::ifstream in(in_file);
        if (!in){ err="cannot open"; return false; }
        std::string line;
        if (fmt=="jsonl"){
            while (std::getline(in, line)){
                try {
                    Contact c = Contact::from_json(J::parse(line));
                    std::string id;
                    std::string e;
                    if (!add(book, c, id, e)){ /* skip dup/invalid */ }
                } catch(...){}
            }
            return true;
        } else if (fmt=="csv"){
            std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            std::vector<std::vector<std::string>> rows; std::string perr;
            if (!csv_parse_rfc4180(data, rows, perr)){ err="CSV parse error: "+perr; return false; }
            for (size_t i=1;i<rows.size();++i){
                auto& f = rows[i]; if (f.size()<6) continue;
                Contact c; c.id=f[0].empty()? uuid_v4():f[0]; c.name=f[1]; c.email=f[2]; c.phone=f[3]; c.note=f[4]; c.created_at=f[5].empty()? now_iso8601_utc():f[5];
                std::string id; std::string e; (void)add(book, c, id, e);
            }
            return true;
        }
        err="unknown format"; return false;
    }
};

IStorageEngine* make_engine_rocksdb(){ return new RocksEngine(); }

#endif

} // namespace acx
