#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>

namespace acx {

// Minimal JSON representation sufficient for our storage.
struct J;
using JV = std::variant<std::nullptr_t, bool, double, std::string,
                        std::vector<J>, std::map<std::string, J>>;

struct J {
    JV v;
    J(): v(nullptr) {}
    J(std::nullptr_t): v(nullptr) {}
    J(bool b): v(b) {}
    J(double d): v(d) {}
    J(const std::string& s): v(s) {}
    J(const char* s): v(std::string(s)) {}
    J(const std::vector<J>& a): v(a) {}
    J(const std::map<std::string, J>& o): v(o) {}

    std::string dump(int indent=0) const;
    static std::string escape(const std::string& s);
    static J parse(const std::string& s, size_t& i);
    static J parse(const std::string& s);
};

// JSON Lines helpers
std::vector<J> parse_jsonl(const std::string& data);
std::string dump_jsonl(const std::vector<J>& arr);

} // namespace acx
