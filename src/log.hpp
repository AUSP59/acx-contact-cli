#pragma once

// Added: honor NO_COLOR and ACX_NO_COLOR
#include <cstdlib>
inline bool acx_color_enabled(){
  const char* n1 = std::getenv("NO_COLOR");
  const char* n2 = std::getenv("ACX_NO_COLOR");
  return !(n1 || n2);
}

#include <string>
#include <iostream>
#include <cstdlib>
#include <map>

namespace acx {

enum class LogLevel { DEBUG=0, INFO=1, WARN=2, ERROR=3 };
LogLevel log_level();
bool log_json();

inline const char* lvl_name(LogLevel lv){
    switch(lv){ case LogLevel::DEBUG: return "DEBUG"; case LogLevel::INFO: return "INFO";
                case LogLevel::WARN: return "WARN"; default: return "ERROR"; }
}

inline void log(LogLevel lv, const std::string& msg){
    if ((int)lv < (int)log_level()) return;
    if (log_json()){
        std::cout << "{"ts":"", "level":"" << lvl_name(lv) << "", "msg":"" << msg << ""}\n";
    } else {
        std::cerr << "[" << lvl_name(lv) << "] " << msg << "\n";
    }
}

} // namespace acx
