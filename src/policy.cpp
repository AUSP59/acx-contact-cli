#include "policy.hpp"
#include "jsonl.hpp"
#include "utils.hpp"
#include <cctype>

namespace acx {
Policy load_policy(const std::string& path){
    Policy p;
    if (path.empty()) return p;
    try {
        auto t = read_file(path);
        auto j = J::parse(t);
        if (j.is_object()){
            auto& o = j.as_object();
            if (o.count("redact_name")) p.redact_name = o["redact_name"].as_bool();
            if (o.count("redact_email")) p.redact_email = o["redact_email"].as_bool();
            if (o.count("redact_phone")) p.redact_phone = o["redact_phone"].as_bool();
        }
    } catch (...) {}
    return p;
}
std::string mask_name(const std::string& s){
    if (s.empty()) return s;
    std::string out; out.reserve(s.size());
    bool first=true;
    for (char c: s){
        if (std::isalpha((unsigned char)c)){
            if (first){ out.push_back(c); first=false; }
            else out.push_back('*');
        } else {
            out.push_back(c);
        }
    }
    return out;
}
std::string mask_email(const std::string& s){
    auto at = s.find('@'); if (at==std::string::npos) return s;
    if (at==0) return s;
    std::string out; out.reserve(s.size());
    out.push_back(s[0]);
    for (size_t i=1;i<at;i++) out.push_back('*');
    out.append(s.substr(at));
    return out;
}
std::string mask_phone(const std::string& s){
    // keep only digits/+, mask all but last 4 digits
    std::string d; d.reserve(s.size());
    for (char c: s){ if (std::isdigit((unsigned char)c) || c=='+') d.push_back(c); }
    int digits=0; for (char c: d) if (std::isdigit((unsigned char)c)) digits++;
    if (digits<=4) return d;
    int to_mask = digits-4;
    std::string out; out.reserve(d.size());
    for (char c: d){
        if (std::isdigit((unsigned char)c)){
            if (to_mask>0){ out.push_back('x'); --to_mask; } else out.push_back(c);
        } else out.push_back(c);
    }
    return out;
}
} // namespace acx
