#include "log.hpp"
#include <string>
#include <cstdlib>

namespace acx {

static LogLevel cached = LogLevel::INFO;
static bool cached_set=false;
static bool cached_json=false;
static bool cached_json_set=false;

LogLevel log_level(){
    if (cached_set) return cached;
    const char* v = std::getenv("ACX_LOG_LEVEL");
    if (!v){ cached = LogLevel::INFO; cached_set=true; return cached; }
    std::string s = v;
    if (s=="debug") cached = LogLevel::DEBUG;
    else if (s=="info") cached = LogLevel::INFO;
    else if (s=="warn") cached = LogLevel::WARN;
    else cached = LogLevel::ERROR;
    cached_set=true;
    return cached;
}

bool log_json(){
    if (cached_json_set) return cached_json;
    const char* v = std::getenv("ACX_LOG_FORMAT");
    cached_json = (v && std::string(v)=="json");
    cached_json_set=true;
    return cached_json;
}

} // namespace acx
