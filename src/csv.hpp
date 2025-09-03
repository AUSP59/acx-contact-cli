#pragma once
#include <string>
#include <vector>

namespace acx {
// Parse RFC4180 CSV into records (vector of vector<string>)
// Supports quoted fields, escaped quotes ("") and embedded newlines in quotes.
bool csv_parse_rfc4180(const std::string& input, std::vector<std::vector<std::string>>& out, std::string& err);
}
