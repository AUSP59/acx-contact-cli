#include "verify.hpp"
namespace acx {
bool verify_sha256_file(const std::string& path, const std::string& expected_hex){
    std::string got = sha256_file_hex(path);
    return !got.empty() && got == expected_hex;
}
}
