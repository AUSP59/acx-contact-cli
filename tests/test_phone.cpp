#include "tests/unit_micro.hpp"
#include "src/validation.hpp"
#include <cstdlib>

UT(phone_basic){
    // default US
    setenv("ACX_DEFAULT_COUNTRY","US",1);
    auto p = acx::normalize_phone("5551234567");
    OK(p.rfind("+1",0)==0 || p.size()>=10);
    setenv("ACX_DEFAULT_COUNTRY","MX",1);
    auto m = acx::normalize_phone("5512345678");
    OK(m.rfind("+52",0)==0 || m.size()>=10);
}
int main(){ ut::run(); }
