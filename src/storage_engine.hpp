#pragma once
#include <string>
#include <filesystem>
#include "model.hpp"

namespace acx {

struct IStorageEngine {
    virtual ~IStorageEngine() = default;
    virtual bool load(ContactBook& book) = 0;
    virtual bool save(const ContactBook& book) = 0;
    virtual bool add(ContactBook& book, Contact c, std::string& id, std::string& err) = 0;
    virtual bool edit(ContactBook& book, const std::string& id, const std::string& name, const std::string& email, const std::string& phone, const std::string& note, std::string& err) = 0;
    virtual bool remove(ContactBook& book, const std::string& id, const std::string& user, std::string& err) = 0;
    virtual std::vector<Contact> search(const ContactBook& book, const std::string& name, const std::string& email, const std::string& phone, const std::string& id) = 0;
    virtual bool import_data(ContactBook& book, const std::string& fmt, const std::filesystem::path& in_file, bool parallel, std::string& err) = 0;
};

IStorageEngine* make_engine_jsonl();
IStorageEngine* make_engine_sqlite(); // may return nullptr if not compiled

} // namespace acx
