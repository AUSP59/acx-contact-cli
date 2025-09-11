#pragma once
#include <string>
#include <filesystem>
#include "model.hpp"

namespace acx {

struct PersistentIndex {
    std::filesystem::path index_file = "acx-index.json";
    bool load_or_build(const std::filesystem::path& data_file, ContactBook& book);
    bool save(const std::filesystem::path& data_file, const ContactBook& book);
};

} // namespace acx
