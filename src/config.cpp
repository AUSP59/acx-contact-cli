// SPDX-License-Identifier: Apache-2.0
#include "config.hpp"
#include "jsonl.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;
namespace acx {

static std::string getenv_str(const char* k){
    const char* v = std::getenv(k);
    return v ? std::string(v) : std::string();
}

static bool read_file_str(const std::string& p, std::string& out){
    std::ifstream ifs(p, std::ios::binary);
    if(!ifs) return false;
    std::vector<char> buf((std::istreambuf_iterator<char>(ifs)), {});
    out.assign(buf.begin(), buf.end());
    return true;
}

std::optional<Config> load_config_file(const std::string& path){
    std::string data;
    if(!read_file_str(path, data)) return std::nullopt;
    auto j = J::parse(data);
    if(!j.is_object()) return std::nullopt;
    Config c;
    auto& m = j.as_object();
    if(auto it=m.find("data_file"); it!=m.end() && it->second.is_string()) c.data_file = it->second.as_string();
    if(auto it=m.find("backup_dir"); it!=m.end() && it->second.is_string()) c.backup_dir = it->second.as_string();
    if(auto it=m.find("audit_log"); it!=m.end() && it->second.is_string()) c.audit_log = it->second.as_string();
    if(auto it=m.find("lock_dir");  it!=m.end() && it->second.is_string()) c.lock_dir  = it->second.as_string();
    if(auto it=m.find("region");    it!=m.end() && it->second.is_string()) c.region    = it->second.as_string();
    if(auto it=m.find("output");    it!=m.end() && it->second.is_string()) c.output    = it->second.as_string();
    return c;
}

std::optional<Config> load_config(){
    // 1) ACX_CONFIG
    std::string p = getenv_str("ACX_CONFIG");
    if(!p.empty()){
        if(auto c = load_config_file(p)) return c;
    }
    // 2) XDG
    std::string xdg = getenv_str("XDG_CONFIG_HOME");
    if(!xdg.empty()){
        fs::path xp = fs::path(xdg) / "acx" / "config.json";
        if(fs::exists(xp)){
            if(auto c = load_config_file(xp.string())) return c;
        }
    }
    // 3) ~/.config
    std::string home = getenv_str("HOME");
    if(!home.empty()){
        fs::path hp = fs::path(home) / ".config" / "acx" / "config.json";
        if(fs::exists(hp)){
            if(auto c = load_config_file(hp.string())) return c;
        }
    }
    return std::nullopt;
}

void apply_config_env(const Config& cfg){
    if(!cfg.data_file.empty()) setenv("ACX_DATA_FILE", cfg.data_file.c_str(), 1);
    if(!cfg.backup_dir.empty()) setenv("ACX_BACKUP_DIR", cfg.backup_dir.c_str(), 1);
    if(!cfg.audit_log.empty()) setenv("ACX_AUDIT_LOG", cfg.audit_log.c_str(), 1);
    if(!cfg.lock_dir.empty())  setenv("ACX_LOCK_DIR", cfg.lock_dir.c_str(), 1);
    if(!cfg.region.empty())    setenv("ACX_REGION", cfg.region.c_str(), 1);
    if(!cfg.output.empty())    setenv("ACX_OUTPUT", cfg.output.c_str(), 1);
}

} // namespace acx
