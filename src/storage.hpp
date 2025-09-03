#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include "model.hpp"

namespace acx {

struct Storage {
    Storage();
    std::filesystem::path data_file = "acx-data.json";
    std::filesystem::path backup_dir = "backups";
    std::filesystem::path audit_log = "acx-audit.log";

    bool load(ContactBook& book) const;
    bool save(const ContactBook& book) const;
    bool backup_last() const;
    bool append_audit(const std::string& line) const; // CSV
    bool verify_backup_signature(const std::filesystem::path& bak_file, const std::string& key) const;
};

// High-level operations (dedup/validation included)
bool add_contact(Storage& st, ContactBook& book, const std::string& name,
                 const std::string& email, const std::string& phone,
                 const std::string& note, std::string user, std::string& out_id, std::string& err);

bool edit_contact(Storage& st, ContactBook& book, const std::string& id,
                  const std::string& name, const std::string& email,
                  const std::string& phone, const std::string& note,
                  std::string user, std::string& err);

bool delete_contact(Storage& st, ContactBook& book, const std::string& id,
                    std::string user, std::string& err);

std::vector<Contact> search_contacts(const ContactBook& book,
                                     const std::string& name,
                                     const std::string& email,
                                     const std::string& phone,
                                     const std::string& id);

bool export_data(const ContactBook& book, const std::string& fmt, const std::filesystem::path& out_file, std::string& err);
bool import_data(Storage& st, ContactBook& book, const std::string& fmt, const std::filesystem::path& in_file, std::string& err);
bool import_data_parallel(Storage& st, ContactBook& book, const std::string& fmt, const std::filesystem::path& in_file, std::string& err);

} // namespace acx
