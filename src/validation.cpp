#ifdef ACX_HAVE_LIBPHONENUMBER
#include <phonenumbers/phonenumberutil.h>
#include <phonenumbers/asyoutypeformatter.h>
#endif
#include "validation.hpp"
#include <regex>
#include <cctype>
#include <algorithm>


namespace acx {

static inline std::string env_default_country(){
    const char* v = std::getenv("ACX_DEFAULT_COUNTRY");
    return v? std::string(v) : std::string("US");
}


bool is_valid_name(const std::string& name){
    if (name.size() < 2) return false;
    int letters=0;
    for(char c: name){
        if (std::isalpha((unsigned char)c)) letters++;
        else if (std::isspace((unsigned char)c) || c=='-' || c=='\'') continue;
        else return false;
    }
    return letters>=2;
}

bool is_valid_email(const std::string& email){
    static const std::regex re(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
    return std::regex_match(email, re);
}

std::string to_lower_ascii(const std::string& s){
    std::string t=s;
    std::transform(t.begin(), t.end(), t.begin(), [](unsigned char c){ return std::tolower(c); });
    return t;
}


std::string normalize_phone(const std::string& phone){
#ifdef ACX_HAVE_LIBPHONENUMBER
    using namespace i18n::phonenumbers;
    PhoneNumberUtil* util = PhoneNumberUtil::GetInstance();
    PhoneNumber pn;
    auto region = env_default_country();
    auto status = util->Parse(phone, region, &pn);
    if (status == PhoneNumberUtil::NO_PARSING_ERROR){
        std::string e164;
        util->Format(pn, PhoneNumberUtil::E164, &e164);
        return e164;
    }
    // fallthrough to internal normalization on parse failure
#endif
    return normalize_phone_e164(phone, env_default_country());
}

    if (!d.empty() && d[0] != '0' && d.size()>=10) return std::string("+") + d; // naive E.164-ish
    return d;
}

bool is_valid_phone(const std::string& phone){
    auto n = normalize_phone(phone);
    return n.size() >= 10; // naive lower bound
}

} // namespace acx

// Enhanced E.164-ish normalization with default country code
static inline std::string normalize_phone_e164(const std::string& phone, const std::string& default_country){
    // Map a few common countries to codes (extendable via config/env)
    auto cc = std::string("+1"); // US default
    if (default_country=="MX") cc = "+52";
    else if (default_country=="US") cc = "+1";
    else if (default_country=="GB") cc = "+44";
    else if (default_country=="ES") cc = "+34";
    else if (default_country=="BR") cc = "+55";
    // strip non-digits/+
    std::string d; d.reserve(phone.size());
    for(char c: phone){ if (std::isdigit((unsigned char)c)) d.push_back(c); }
    if (d.empty()) return d;
    if (phone.size()>0 && phone[0]=='+'){
        return std::string("+") + d; // already international
    }
    // assume national: prefix country code if plausible length (>=10)
    if (d.size()>=10) return cc + d;
    return d;
}
