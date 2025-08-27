#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>

namespace acx {

std::string read_file(const std::filesystem::path& p);
bool write_file_atomic(const std::filesystem::path& p, const std::string& content);
bool copy_file_to(const std::filesystem::path& src, const std::filesystem::path& dst);
std::string now_iso8601_utc();
std::string uuid_v4();
std::string sha256_hex(const std::string& data);
std::vector<std::string> split(const std::string& s, char delim);
std::string trim(const std::string& s);
bool file_exists_nonempty(const std::filesystem::path& p);
std::vector<std::filesystem::path> list_files_recursive(const std::filesystem::path& root);
std::string sha256_file_hex(const std::string& path);
std::string read_file_mmap(const std::filesystem::path& p);
int env_int(const char* name, int def);
size_t env_size(const char* name, size_t def);

} // namespace acx
