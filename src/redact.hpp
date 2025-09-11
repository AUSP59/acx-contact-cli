#pragma once
#include "policy.hpp"
#include "model.hpp"

namespace acx {
Contact redact_contact(const Contact& c, const Policy& p);
ContactBook redact_book(const ContactBook& b, const Policy& p);
} // namespace acx
