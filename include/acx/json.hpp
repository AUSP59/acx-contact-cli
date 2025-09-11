// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <map>

namespace acx {
struct Json {
  static std::string esc(const std::string& s){
    std::ostringstream o;
    for (unsigned char c : s) {
      switch(c){
        case '\"': o << "\\\""; break;
        case '\\\\': o << "\\\\"; break;
        case '\b': o << "\\b"; break;
        case '\f': o << "\\f"; break;
        case '\n': o << "\\n"; break;
        case '\r': o << "\\r"; break;
        case '\t': o << "\\t"; break;
        default:
          if (c < 0x20) { o << "\\\\u" << std::hex << (int)c; }
          else { o << c; }
      }
    }
    return o.str();
  }
  template <class T>
  static std::string kv(const std::string& k, const T& v){
    std::ostringstream o;
    o << '\"' << esc(k) << '\"' << ':' << v;
    return o.str();
  }
  static std::string str(const std::string& s){
    std::ostringstream o; o << '\"' << esc(s) << '\"'; return o.str();
  }
};
} // namespace acx
