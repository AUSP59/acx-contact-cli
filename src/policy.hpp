#pragma once
#include <string>

namespace acx {
struct Policy {
    bool redact_name = false;
    bool redact_email = true;
    bool redact_phone = true;
    // strategies: keep first char/local, last 4 digits, etc.
};
Policy load_policy(const std::string& path);
std::string mask_name(const std::string& s);
std::string mask_email(const std::string& s);
std::string mask_phone(const std::string& s);
} // namespace acx
