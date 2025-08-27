#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

namespace acx {
// Constant-time comparison
inline bool ct_memeq(const unsigned char* a, const unsigned char* b, size_t n){
    unsigned char r=0;
    for (size_t i=0;i<n;++i) r |= (unsigned char)(a[i] ^ b[i]);
    return r==0;
}
inline bool ct_hex_eq(const std::string& ha, const std::string& hb){
    if (ha.size()!=hb.size()) return false;
    unsigned char r=0;
    for (size_t i=0;i<ha.size();++i) r |= (unsigned char)(ha[i]^hb[i]);
    return r==0;
}
} // namespace acx
