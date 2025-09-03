#include "tests/unit_micro.hpp"
#include "src/policy.hpp"
#include "src/redact.hpp"

UT(masking){
    acx::Policy p; p.redact_email=true; p.redact_phone=true;
    acx::Contact c; c.name="Ada Lovelace"; c.email="ada@ex.org"; c.phone="+525511112222";
    auto r = acx::redact_contact(c, p);
    OK(r.email != c.email);
    OK(r.phone != c.phone);
}
int main(){ ut::run(); }
