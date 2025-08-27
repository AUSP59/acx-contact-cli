#pragma once
#include <string>
#include <vector>
#include "jsonl.hpp"

namespace acx {

struct HistoryEntry {
    std::string timestamp;
    std::string user;
    std::string action; // created, edited, deleted
    J to_json() const;
    static HistoryEntry from_json(const J& j);
};

struct Contact {
    std::string id;
    std::string name;
    std::string email;
    std::string phone;
    std::string note;
    std::string created_at;
    std::vector<HistoryEntry> history;

    J to_json() const;
    static Contact from_json(const J& j);
};


#include <unordered_map>

struct ContactBook {
    std::vector<Contact> contacts;
    // Fast lookup indexes (normalized keys)
    std::unordered_map<std::string, size_t> email_to_i;
    std::unordered_map<std::string, size_t> phone_to_i;
};


} // namespace acx
