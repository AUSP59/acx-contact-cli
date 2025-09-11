// SPDX-License-Identifier: Apache-2.0
#include "audit.hpp"
#include "jsonl.hpp"
#include <ctime>
#include <fstream>
#include <cstdlib>

namespace acx {
static std::string now_iso_utc(){
    char buf[64]; std::time_t t = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return std::string(buf);
}
void audit_log(const std::string& action, const std::map<std::string,std::string>& fields){
    const char* path = std::getenv("ACX_AUDIT_LOG");
    if (!path || !*path) return;
    std::map<std::string, acx::J> obj;
    obj["ts"] = now_iso_utc();
    obj["action"] = action;
    const char* user = std::getenv("USER"); if (!user) user = std::getenv("USERNAME");
    if (user && *user) obj["user"] = std::string(user);
    for (auto &kv : fields) obj[kv.first] = kv.second;
    acx::J j(obj);
    std::ofstream out(path, std::ios::app);
    if (!out.good()) return;
    out << j.dump(0) << "\n";
}
}
