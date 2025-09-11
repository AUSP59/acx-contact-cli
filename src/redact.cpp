#include "redact.hpp"

namespace acx {
Contact redact_contact(const Contact& c, const Policy& p){
    Contact r = c;
    if (p.redact_name) r.name = mask_name(r.name);
    if (p.redact_email) r.email = mask_email(r.email);
    if (p.redact_phone) r.phone = mask_phone(r.phone);
    return r;
}
ContactBook redact_book(const ContactBook& b, const Policy& p){
    ContactBook rb = b;
    for (auto &c : rb.contacts) c = redact_contact(c, p);
    return rb;
}
} // namespace acx
