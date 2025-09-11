#pragma once
#include <string>
#include <vector>

namespace acx {
std::string hmac_sha256_hex(const std::string& key, const std::string& data);
}
