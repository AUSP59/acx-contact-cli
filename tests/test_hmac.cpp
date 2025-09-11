#include "unit_micro.hpp"
#include "src/hmac.hpp"
#include <string>
// Basic regression (not official NIST vector, but deterministic)
UT(hmac_basic){
    std::string h = acx::hmac_sha256_hex("key","data");
    OK(h.size()==64);
}
int main(){ ut::run(); }
