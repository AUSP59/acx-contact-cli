#pragma once
#include <string>

namespace acx {

bool is_valid_name(const std::string& name);
bool is_valid_email(const std::string& email);
bool is_valid_phone(const std::string& phone);
std::string normalize_phone(const std::string& phone);
std::string to_lower_ascii(const std::string& s);

} // namespace acx
