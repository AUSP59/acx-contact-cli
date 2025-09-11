#pragma once
#include <string>
#include <filesystem>
#include <vector>
#include <cstdint>

namespace acx {

struct DiskIndexEntry { std::string key; uint64_t offset; };

struct DiskIndex {
    std::filesystem::path file = "acx-index2.bin";
    std::filesystem::path meta = "acx-index2.meta"; // contains data sha256
    bool build(const std::filesystem::path& data_file,
               const std::vector<std::pair<std::string,uint64_t>>& email_kv,
               const std::vector<std::pair<std::string,uint64_t>>& phone_kv);
    bool valid_for_data_hash(const std::string& data_sha) const;
    bool load(std::vector<DiskIndexEntry>& email, std::vector<DiskIndexEntry>& phone) const;
    bool lookup_email(const std::string& norm_email, uint64_t& out_offset) const;
    bool lookup_phone(const std::string& norm_phone, uint64_t& out_offset) const;
};

} // namespace acx
