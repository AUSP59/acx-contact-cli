#pragma once
#include <string>
#include "utils.hpp"

namespace acx {
bool verify_sha256_file(const std::string& path, const std::string& expected_hex);
}
