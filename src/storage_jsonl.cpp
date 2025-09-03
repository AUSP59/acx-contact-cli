#include "storage_engine.hpp"
#include "storage.hpp"

namespace acx {

struct JsonlEngine : IStorageEngine {
    Storage st;
    bool load(ContactBook& book) override { return st.load(book); }
    bool save(const ContactBook& book) override { return st.save(const_cast<ContactBook&>(book)); }
    bool add(ContactBook& book, Contact c, std::string& id, std::string& err) override { return st.add(book, c, id, err); }
    bool edit(ContactBook& book, const std::string& id, const std::string& name, const std::string& email, const std::string& phone, const std::string& note, std::string& err) override {
        return st.edit(book, id, name, email, phone, note, err);
    }
    bool remove(ContactBook& book, const std::string& id, const std::string& user, std::string& err) override { return st.remove(book, id, user, err); }
    std::vector<Contact> search(const ContactBook& book, const std::string& name, const std::string& email, const std::string& phone, const std::string& id) override {
        return search_contacts(book, name, email, phone, id);
    }
    bool import_data(ContactBook& book, const std::string& fmt, const std::filesystem::path& in_file, bool parallel, std::string& err) override {
        if (parallel) return import_data_parallel(st, book, fmt, in_file, err);
        return import_data(st, book, fmt, in_file, err);
    }
};

IStorageEngine* make_engine_jsonl(){ return new JsonlEngine(); }
IStorageEngine* make_engine_sqlite(){ return nullptr; } // not available in this TU

} // namespace acx
