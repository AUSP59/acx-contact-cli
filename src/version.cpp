// SPDX-License-Identifier: Apache-2.0
#include "version.hpp"
#include <sstream>

#ifndef ACX_VERSION
#define ACX_VERSION "0.0.0"
#endif
#ifndef ACX_BUILD_TIME
#define ACX_BUILD_TIME "unknown"
#endif
#ifndef ACX_GIT_SHA
#define ACX_GIT_SHA ""
#endif

namespace acx {
std::string version_string(){ return std::string(ACX_VERSION); }
std::string build_time_utc(){ return std::string(ACX_BUILD_TIME); }
std::string git_sha(){ return std::string(ACX_GIT_SHA); }
std::string runtime_summary(){
    std::ostringstream o;
    o << "acx " << version_string();
    auto sha = git_sha(); if (!sha.empty()) o << " (" << sha << ")";
    o << " built " << build_time_utc();
#if defined(ACX_HAVE_OPENSSL)
    o << " crypto=OpenSSL";
#else
    o << " crypto=internal";
#endif
#ifdef NDEBUG
    o << " mode=Release";
#else
    o << " mode=Debug";
#endif
    return o.str();
}
}
