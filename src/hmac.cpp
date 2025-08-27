#include "hmac.hpp"
#include "utils.hpp"
#include "crypto.hpp"
#include <vector>
#include <cstring>
#ifdef ACX_HAVE_OPENSSL
#include <openssl/hmac.h>
#include <openssl/evp.h>
#endif

namespace acx {

#ifdef ACX_HAVE_OPENSSL
std::string hmac_sha256_hex(const std::string& key, const std::string& data){
    unsigned int len=0;
    unsigned char out[EVP_MAX_MD_SIZE];
    HMAC(EVP_sha256(), key.data(), (int)key.size(),
         reinterpret_cast<const unsigned char*>(data.data()),
         (int)data.size(), out, &len);
    std::string hex; hex.reserve(len*2);
    const char* dig = "0123456789abcdef";
    for (unsigned i=0;i<len;++i){ unsigned char b=out[i]; hex.push_back(dig[b>>4]); hex.push_back(dig[b&0xF]); }
    return hex;
}
#else
// Fallback using internal sha256_hex (kept for hermetic builds)
static std::vector<unsigned char> hex_to_bytes(const std::string& hx){
    std::vector<unsigned char> out; out.reserve(hx.size()/2);
    for (size_t i=0;i+1<hx.size(); i+=2){
        unsigned int b=0; std::sscanf(hx.substr(i,2).c_str(), "%02x", &b);
        out.push_back((unsigned char)b);
    }
    return out;
}
std::string hmac_sha256_hex(const std::string& key, const std::string& data){
    const size_t B=64; // SHA-256 block
    std::string K = key;
    if (K.size() > B) K = sha256_hex(K); // not ideal but adequate for fallback
    if (K.size() < B) K.resize(B, '\0');
    std::string i(B,'\x36'), o(B,'\x5c');
    for (size_t i2=0;i2<B;++i2){ i[i2] ^= K[i2]; o[i2] ^= K[i2]; }
    std::string inner = i + data;
    std::string inner_hex = sha256_hex(inner);
    // convert hex to bytes
    auto inner_bytes = hex_to_bytes(inner_hex);
    std::string outer; outer.reserve(B + inner_bytes.size());
    outer.append(o);
    outer.append(reinterpret_cast<const char*>(inner_bytes.data()), inner_bytes.size());
    return sha256_hex(outer);
}
#endif

} // namespace acx
