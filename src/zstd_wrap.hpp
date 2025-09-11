#pragma once
#include <string>
#include <vector>

namespace acx {

bool zstd_available();
bool zstd_compress(const std::string& in, std::string& out);
bool zstd_decompress(const std::string& in, std::string& out);

} // namespace acx
