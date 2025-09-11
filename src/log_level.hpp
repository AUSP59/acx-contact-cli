#pragma once
#include <string>
namespace acx {
  enum class LogLevel { Error=0, Warn=1, Info=2, Debug=3, Trace=4 };
  LogLevel current_log_level();
  void set_log_level(LogLevel);
  LogLevel parse_log_level(const std::string& s);
}
