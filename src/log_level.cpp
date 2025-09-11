#include "log_level.hpp"
#include <cstdlib>
#include <algorithm>
namespace acx {
  static LogLevel g_level = LogLevel::Info;
  static inline std::string lower(std::string s){ std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){return (char)std::tolower(c);}); return s; }
  LogLevel parse_log_level(const std::string& s){
    auto v = lower(s);
    if (v=="error") return LogLevel::Error;
    if (v=="warn"||v=="warning") return LogLevel::Warn;
    if (v=="info") return LogLevel::Info;
    if (v=="debug") return LogLevel::Debug;
    if (v=="trace") return LogLevel::Trace;
    return LogLevel::Info;
  }
  LogLevel current_log_level(){ return g_level; }
  void set_log_level(LogLevel l){ g_level = l; }
  struct Init {
    Init(){
      const char* e = std::getenv("ACX_LOG_LEVEL");
      if (e) g_level = parse_log_level(e);
    }
  } _init;
}
