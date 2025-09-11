// SPDX-License-Identifier: Apache-2.0
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <optional>
#include "storage.hpp"
#include "utils.hpp"
#include "validation.hpp"
#include "audit.hpp"
#include "storage_engine.hpp"
#include "log.hpp"
#include "sbom.hpp"
#include "metrics.hpp"
#include "otel.hpp"
#include "doctor.hpp"
#include "hmac.hpp"
#include "crypto.hpp"
#include "version.hpp"
#include "config.hpp"
#include "acx/exit_codes.hpp"
#include "acx/sha256.hpp"
#include "policy.hpp"
#include "redact.hpp"
#include "crypto_file.hpp"
#include "diff.hpp"
#include "version.hpp"

#include "jsonl.hpp"
#include "verify.hpp"
#include "selftest.hpp"
#include "diff.hpp"
#include "policy.hpp"
#include "merge.hpp"
#include "lock.hpp"
#include "disk_index.hpp"
#include <csignal>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include <random>
#include <unordered_map>
#include <algorithm>
#include <regex>
#include <unordered_set>
#include <set>
#include <ctime>
#include <locale>


using namespace acx;
namespace fs = std::filesystem;

struct Args {
    std::string cmd;
    std::map<std::string, std::string> kv;
    bool json = false;
};


static void help(){
    std::cout << "AgendaCyberneticX Enterprise CLI\n\n"
              << "Usage:\n  --json           Output machine-readable JSON when supported\n  --no-color       Disable ANSI colors\n
  # Optional envs: ACX_AUDIT_LOG=path (append JSONL events), ACX_LOCK_DIR=path
  # Global: --dry-run simulates state changes (no writes)\n"
              << "  acx add --name N --email E --phone P [--note TEXT] [--user U]\n"
              << "  acx edit --id ID [--name N] [--email E] [--phone P] [--note TEXT] [--user U]\n"
              << "  acx delete --id ID [--user U] [--yes]\n"
              << "  acx list [--limit N]\n"
              << "  acx search [--name N|--email E|--phone P|--id ID]\n"
              << "  acx export --format csv|tsv|jsonl --out FILE [--policy FILE]
  acx scrub [--policy FILE] [--out FILE]    # redact database in-place or to FILE
  acx --lock-dir DIR <...>                  # acquire advisory lock for mutating commands
  acx checksum --file FILE                  # print SHA-256
  acx sign --file FILE                      # HMAC-SHA256 using ACX_HMAC_KEY
  acx verify --file FILE --sig HEX          # verify HMAC-SHA256 signature
  acx encrypt --in FILE --out FILE [--aad STR]   # AES-256-GCM (OpenSSL required)
  acx decrypt --in FILE --out FILE               # decrypt AES-256-GCM
  acx keygen --bytes 32                          # print random key (hex)
  acx diff --fmt csv|tsv|jsonl --a FILE --b FILE # JSON diff of two exports
  acx doctor                                     # environment & build diagnosis\n"
              << "  acx import --format csv|jsonl --in FILE\n"
              << "  acx backup\n"
              << "  acx audit [--strict]\n"
              << "  acx help
  # Global option: --region CC to set default country for phone normalization
  acx version                       # print version/build info
  acx validate --format csv|jsonl --in FILE
  acx schema --list | --name <add|list|search|audit|backup>\n  acx --json <command>   # machine-readable output\n"
              << "  acx sbom --format spdx|cyclonedx --out FILE\n"
              << "  acx metrics
  acx schema --list | --name <add|list|search|audit|backup>\n"
              << "  [global] --output json (prints JSON lines)\n"
              << "  acx verify-backup --file FILE --key KEY\n"              << "  acx merge --format jsonl|csv --in FILE --strategy skip|prefer-newer|prefer-existing\n"              << "  acx doctor\n"
              << "  acx validate [--policy policy.json] [--output json]\n"
              << "  acx diff --format jsonl|csv --in FILE [--output json]\n"
              << "  acx verify-file --file PATH --sha256 HEX\n"
              << "  acx prune --before 2025-01-01\n"
              << "  acx selftest\n"
              << "  acx determinism-check\n";
}

static Args parse(int argc, char** argv){
    Args a;
    if (argc < 2) { a.cmd="help"; return a; }
    a.cmd = argv[1];
    for (int i=2; i<argc; ++i){
        std::string s = argv[i];
        if (s.rfind("--",0)==0 && i+1<argc) {
            a.kv[s.substr(2)] = argv[++i];
        } else if (s=="--yes") {
            a.kv["yes"]="1";
        }
    }
    return a;
}


// ===== Helpers for new subcommands =====
static int acx_selfcheck(bool json_mode){
  #ifdef _WIN32
  const char* os = "Windows";
  #else
  const char* os = "Unix";
  #endif
  if (json_mode) {
    printf("{\"selfcheck\":\"ok\",\"os\":\"%s\"}\n", os);
  } else {
    printf("Selfcheck: OK (%s)\n", os);
  }
  return 0;
}
static int acx_doctor(bool json_mode){
  // Simple environment sanity: HOME, write tmp, config path detection
  const char* home = std::getenv("HOME");
  bool home_ok = home && *home;
  FILE* f = std::fopen("._acx_tmp_doctor", "w");
  bool tmp_ok = f != nullptr;
  if (f) { std::fclose(f); std::remove("._acx_tmp_doctor"); }
  const char* ac = std::getenv("ACX_CONFIG");
  if (json_mode) {
    printf("{\"doctor\":\"ok\",\"home\":%s,\"tmp\":%s,\"acx_config\":%s}\n",
           home_ok? "true":"false", tmp_ok? "true":"false", (ac&&*ac)? "true":"false");
  } else {
    printf("Doctor: home=%s tmp=%s acx_config=%s\n",
           home_ok? "OK":"MISSING", tmp_ok? "OK":"NO-WRITE", (ac&&*ac)? "SET":"unset");
  }
  return (home_ok && tmp_ok) ? 0 : 1;
}
static int acx_config_init(){
  // Write ~/.config/acx/config.json if not present
  const char* home = std::getenv("HOME");
  if (!home || !*home) { fprintf(stderr, "HOME not set\n"); return 1; }
  std::string dir = std::string(home) + "/.config/acx";
  std::string file = dir + "/config.json";
  #ifdef _WIN32
  _mkdir(dir.c_str());
  #else
  ::system((std::string("mkdir -p ")+dir).c_str());
  #endif
  FILE* f = std::fopen(file.c_str(), "w");
  if (!f) { perror("open"); return 1; }
  const char* content = "{\n  \"data_file\": \"acx-data.jsonl\",\n  \"backup_dir\": \"backups\",\n  \"audit_log\": \"acx-audit.jsonl\",\n  \"lock_dir\": \".acx-locks\",\n  \"region\": \"MX\",\n  \"output\": \"human\"\n}\n";
  std::fwrite(content, 1, std::strlen(content), f);
  std::fclose(f);
  printf("Wrote %s\n", file.c_str());
  return 0;
}


static int acx_hash_file_cmd(bool json_mode, const char* target_path){
  std::string path;
  if (target_path && *target_path) path = target_path;
  else {
    const char* envp = std::getenv("ACX_DATA_FILE");
    path = envp ? envp : "acx-data.jsonl";
  }
  std::string h = acx::sha256::hash_file(path);
  if (h.empty()) { fprintf(stderr, "Failed to read %s\n", path.c_str()); return acx::EX_IO; }
  if (json_mode) { printf("{\"file\":\"%s\",\"sha256\":\"%s\"}\n", path.c_str(), h.c_str()); }
  else { printf("%s  %s\n", h.c_str(), path.c_str()); }
  return acx::EX_OK;
}

static int acx_report_cmd(bool json_mode, const char* target_path){
  std::string path;
  if (target_path && *target_path) path = target_path;
  else {
    const char* envp = std::getenv("ACX_DATA_FILE");
    path = envp ? envp : "acx-data.jsonl";
  }
  // Count lines (records) and basic fields via simple JSONL parsing
  std::ifstream ifs(path);
  if (!ifs) { fprintf(stderr, "Failed to open %s\n", path.c_str()); return acx::EX_IO; }
  size_t count=0, emails=0;
  std::set<std::string> uniqEmails;
  std::string line;
  while (std::getline(ifs, line)){
    if (line.empty()) continue;
    ++count;
    try{
      auto j = J::parse(line);
      if (j.is_object()){
        auto& m = j.as_object();
        auto it = m.find("email");
        if (it!=m.end() && it->second.is_string()){
          ++emails; uniqEmails.insert(it->second.as_string());
        }
      }
    } catch (...) {}
  }
  if (json_mode){
    printf("{\"file\":\"%s\",\"records\":%zu,\"unique_emails\":%zu}\n",
           path.c_str(), count, uniqEmails.size());
  } else {
    printf("File: %s\nRecords: %zu\nUnique emails: %zu\n", path.c_str(), count, uniqEmails.size());
  }
  return acx::EX_OK;
}


// === Data-ops helpers (validate/dedupe/backup/diff/redact) ===
static bool json_get_string(const J::Value& v, const std::string& key, std::string& out){
  if (!v.is_object()) return false;
  auto it = v.as_object().find(key);
  if (it==v.as_object().end() || !it->second.is_string()) return false;
  out = it->second.as_string(); return true;
}
static bool is_email_like(const std::string& s){
  static const std::regex re(R"([A-Za-z0-9._%+\-]+@[A-Za-z0-9.\-]+\.[A-Za-z]{2,})");
  return std::regex_search(s, re);
}
static std::string mask_email(const std::string& e){
  auto pos = e.find('@');
  if (pos==std::string::npos) return std::string(e.size(), '*');
  std::string user = e.substr(0,pos);
  std::string dom  = e.substr(pos+1);
  if (user.size()>2) user = user.substr(0,1) + std::string(user.size()-2,'*') + user.substr(user.size()-1);
  else user = std::string(user.size(), '*');
  return user + "@" + dom;
}
static std::string mask_phone(const std::string& p){
  std::string o;
  for (char c: p){
    if (std::isdigit((unsigned char)c)) o.push_back('*');
    else o.push_back(c);
  }
  return o;
}
static int acx_validate_cmd(bool json_mode, const char* target_path){
  std::string path = (target_path && *target_path) ? target_path : (std::getenv("ACX_DATA_FILE")? std::getenv("ACX_DATA_FILE"): "acx-data.jsonl");
  std::ifstream ifs(path);
  if(!ifs){ fprintf(stderr, "Failed to open %s\n", path.c_str()); return acx::EX_IO; }
  size_t count=0, ok=0, bad=0;
  std::string line;
  while (std::getline(ifs, line)){
    if(line.empty()) continue;
    ++count;
    try{
      auto j = J::parse(line);
      std::string name;
      bool has_name = json_get_string(j, "name", name) && !name.empty();
      bool email_ok = true;
      std::string email;
      if (json_get_string(j, "email", email)) email_ok = is_email_like(email);
      if (has_name && email_ok) ++ok; else ++bad;
    } catch (...) { ++bad; }
  }
  if (json_mode) printf("{\"file\":\"%s\",\"records\":%zu,\"valid\":%zu,\"invalid\":%zu}\n", path.c_str(), count, ok, bad);
  else printf("Validate %s\nRecords: %zu\nValid: %zu\nInvalid: %zu\n", path.c_str(), count, ok, bad);
  return bad? acx::EX_VALIDATION : acx::EX_OK;
}
static int acx_backup_cmd(const char* target_path, const char* dest_dir){
  std::string path = (target_path && *target_path) ? target_path : (std::getenv("ACX_DATA_FILE")? std::getenv("ACX_DATA_FILE"): "acx-data.jsonl");
  std::string dest = (dest_dir && *dest_dir)? dest_dir : (std::getenv("ACX_BACKUP_DIR")? std::getenv("ACX_BACKUP_DIR"): "backups");
  std::error_code ec; std::filesystem::create_directories(dest, ec);
  std::time_t t = std::time(nullptr);
  char buf[64]; std::strftime(buf, sizeof(buf), "%Y%m%d-%H%M%S", std::localtime(&t));
  std::string out = dest + "/acx-" + buf + ".jsonl";
  std::ifstream src(path, std::ios::binary); if(!src){ perror("open src"); return acx::EX_IO; }
  std::ofstream dst(out, std::ios::binary); if(!dst){ perror("open dst"); return acx::EX_IO; }
  dst << src.rdbuf();
  printf("%s\n", out.c_str());
  return acx::EX_OK;
}
static int acx_dedupe_cmd(const char* target_path, const char* out_path){
  std::string path = (target_path && *target_path) ? target_path : (std::getenv("ACX_DATA_FILE")? std::getenv("ACX_DATA_FILE"): "acx-data.jsonl");
  std::string out  = (out_path && *out_path) ? out_path : "acx-dedupe.jsonl";
  std::ifstream ifs(path); if(!ifs){ fprintf(stderr,"Failed to open %s\n", path.c_str()); return acx::EX_IO; }
  std::ofstream ofs(out); if(!ofs){ fprintf(stderr,"Failed to write %s\n", out.c_str()); return acx::EX_IO; }
  std::unordered_set<std::string> seen;
  std::string line; size_t kept=0, dropped=0;
  while (std::getline(ifs, line)){
    if (line.empty()) continue;
    try{
      auto j = J::parse(line);
      std::string email; std::string key;
      if (json_get_string(j, "email", email)) key=email;
      else key = line; // fall back to entire record
      if (seen.insert(key).second){ ofs << line << "\n"; ++kept; } else { ++dropped; }
    } catch (...) { ofs << line << "\n"; ++kept; }
  }
  fprintf(stderr, "dedupe kept=%zu dropped=%zu -> %s\n", kept, dropped, out.c_str());
  return acx::EX_OK;
}
static int acx_diff_cmd(bool json_mode, const char* a_path, const char* b_path){
  if (!(a_path && b_path)) { fprintf(stderr, "diff requires <A> <B>\n"); return acx::EX_USAGE; }
  auto load = [](const char* p){
    std::set<std::string> s; std::ifstream f(p); std::string ln;
    while (std::getline(f, ln)){
      if (ln.empty()) continue;
      try{ auto j=J::parse(ln); std::string e; if (json_get_string(j,"email",e)) s.insert(e); } catch(...) {}
    } return s;
  };
  auto A = load(a_path), B = load(b_path);
  std::vector<std::string> added, removed;
  std::set_difference(B.begin(),B.end(),A.begin(),A.end(), std::back_inserter(added));
  std::set_difference(A.begin(),A.end(),B.begin(),B.end(), std::back_inserter(removed));
  if (json_mode){
    printf("{\"added\":%zu,\"removed\":%zu}\n", added.size(), removed.size());
  } else {
    printf("Added: %zu\nRemoved: %zu\n", added.size(), removed.size());
  }
  return (added.empty() && removed.empty())? acx::EX_OK : acx::EX_OK;
}
static int acx_redact_cmd(const char* in_path, const char* out_path){
  std::string in  = (in_path && *in_path)? in_path : (std::getenv("ACX_DATA_FILE")? std::getenv("ACX_DATA_FILE"): "acx-data.jsonl");
  std::string out = (out_path && *out_path)? out_path: "acx-redacted.jsonl";
  std::ifstream ifs(in); if(!ifs){ fprintf(stderr,"Failed to open %s\n", in.c_str()); return acx::EX_IO; }
  std::ofstream ofs(out); if(!ofs){ fprintf(stderr,"Failed to write %s\n", out.c_str()); return acx::EX_IO; }
  std::string line;
  while (std::getline(ifs,line)){
    if (line.empty()) continue;
    try{
      auto j = J::parse(line);
      if (j.is_object()){
        auto &m = j.as_object();
        auto it = m.find("email");
        if (it!=m.end() && it->second.is_string()) it->second = J::Value(mask_email(it->second.as_string()));
        auto it2 = m.find("phone");
        if (it2!=m.end() && it2->second.is_string()) it2->second = J::Value(mask_phone(it2->second.as_string()));
        ofs << j.dump() << "\n";
      } else {
        ofs << line << "\n";
      }
    } catch (...) { ofs << line << "\n"; }
  }
  fprintf(stderr, "redacted -> %s\n", out.c_str());
  return acx::EX_OK;
}


// === Atomic write helper ===
static bool atomic_write_all(const std::string& path, const std::string& content){
  std::error_code ec;
  std::filesystem::path p(path);
  std::filesystem::create_directories(p.parent_path(), ec);
  std::string tmp = path + ".tmpXXXXXX";
  // portable poor-man's temp: append pid/time
  tmp = path + ".tmp." + std::to_string(::getpid()) + "." + std::to_string(std::time(nullptr));
  { std::ofstream ofs(tmp, std::ios::binary); if(!ofs) return false; ofs << content; }
  std::filesystem::rename(tmp, path, ec);
  if (ec) return false;
  return true;
}
// === String helpers ===
static inline std::string ltrim(const std::string& s){ size_t i=0; while (i<s.size() && std::isspace((unsigned char)s[i])) ++i; return s.substr(i); }
static inline std::string rtrim(const std::string& s){ size_t i=s.size(); while (i>0 && std::isspace((unsigned char)s[i-1])) --i; return s.substr(0,i); }
static inline std::string trim(const std::string& s){ return rtrim(ltrim(s)); }
static inline std::string lower(const std::string& s){ std::string o=s; for(char& c:o) c=(char)std::tolower((unsigned char)c); return o; }
static inline std::string phone_digits(const std::string& s){ std::string o; o.reserve(s.size()); for(char c:s){ if(std::isdigit((unsigned char)c) || (c=='+' && o.empty())) o.push_back(c); } return o; }

static int acx_normalize_cmd(const char* in_path, const char* out_path){
  std::string in  = (in_path && *in_path)? in_path : (std::getenv("ACX_DATA_FILE")? std::getenv("ACX_DATA_FILE"):"acx-data.jsonl");
  std::string out = (out_path && *out_path)? out_path: "acx-normalized.jsonl";
  std::ifstream ifs(in); if(!ifs){ fprintf(stderr,"Failed to open %s\n", in.c_str()); return acx::EX_IO; }
  std::string line; std::string acc;
  while (std::getline(ifs,line)){
    if(line.empty()){ acc += "\n"; continue; }
    try{
      auto j = J::parse(line);
      if (j.is_object()){
        auto &m = j.as_object();
        auto itn=m.find("name"); if (itn!=m.end() && itn->second.is_string()) itn->second = J::Value(trim(itn->second.as_string()));
        auto ite=m.find("email"); if (ite!=m.end() && ite->second.is_string()) ite->second = J::Value(lower(trim(ite->second.as_string())));
        auto itp=m.find("phone"); if (itp!=m.end() && itp->second.is_string()) itp->second = J::Value(phone_digits(itp->second.as_string()));
        acc += j.dump(); acc += "\n";
      } else acc += line + "\n";
    } catch(...) { acc += line + "\n"; }
  }
  if (!atomic_write_all(out, acc)){ fprintf(stderr,"Failed to write %s\n", out.c_str()); return acx::EX_IO; }
  fprintf(stderr,"normalized -> %s\n", out.c_str());
  return acx::EX_OK;
}

static int acx_merge_cmd(int argc, char** argv, int start, const char* out_path){
  std::string out = (out_path && *out_path)? out_path: "acx-merged.jsonl";
  std::unordered_set<std::string> seen;
  std::string acc;
  for (int k=start; k<argc; ++k){
    std::ifstream f(argv[k]); if(!f){ fprintf(stderr,"skip %s\n", argv[k]); continue; }
    std::string line;
    while (std::getline(f,line)){
      if (line.empty()) continue;
      try{
        auto j=J::parse(line);
        std::string email;
        if (j.is_object() && j.as_object().count("email") && j.as_object().at("email").is_string())
          email = j.as_object().at("email").as_string();
        std::string key = !email.empty()? email : line;
        if (seen.insert(key).second){ acc += line; acc += "\n"; }
      } catch(...) { acc += line; acc += "\n"; }
    }
  }
  if (!atomic_write_all(out, acc)){ fprintf(stderr,"Failed to write %s\n", out.c_str()); return acx::EX_IO; }
  fprintf(stderr,"merged -> %s\n", out.c_str());
  return acx::EX_OK;
}

static int acx_sample_cmd(const char* in_path, const char* out_path, size_t n){
  std::string in  = (in_path && *in_path)? in_path : (std::getenv("ACX_DATA_FILE")? std::getenv("ACX_DATA_FILE"):"acx-data.jsonl");
  std::string out = (out_path && *out_path)? out_path: "acx-sample.jsonl";
  std::ifstream ifs(in); if(!ifs){ fprintf(stderr,"Failed to open %s\n", in.c_str()); return acx::EX_IO; }
  std::vector<std::string> res; res.reserve(n);
  std::string line; size_t count=0;
  std::mt19937_64 rng((unsigned)std::time(nullptr));
  while (std::getline(ifs,line)){
    if (line.empty()) continue;
    if (count < n) res.push_back(line);
    else {
      std::uniform_int_distribution<size_t> dist(0, count);
      size_t j = dist(rng);
      if (j < n) res[j] = line;
    }
    ++count;
  }
  std::string acc;
  for (auto& s: res){ acc += s; acc += "\n"; }
  if (!atomic_write_all(out, acc)){ fprintf(stderr,"Failed to write %s\n", out.c_str()); return acx::EX_IO; }
  fprintf(stderr,"sample -> %s (%zu of %zu)\n", out.c_str(), res.size(), count);
  return acx::EX_OK;
}

static int acx_grep_cmd(const char* in_path, const char* out_path, const char* pattern){
  if (!pattern || !*pattern){ fprintf(stderr,"grep requires a pattern\n"); return acx::EX_USAGE; }
  std::regex re(pattern, std::regex::icase);
  std::string in  = (in_path && *in_path)? in_path : (std::getenv("ACX_DATA_FILE")? std::getenv("ACX_DATA_FILE"):"acx-data.jsonl");
  std::string out = (out_path && *out_path)? out_path: "acx-grep.jsonl";
  std::ifstream ifs(in); if(!ifs){ fprintf(stderr,"Failed to open %s\n", in.c_str()); return acx::EX_IO; }
  std::string line; std::string acc;
  while (std::getline(ifs,line)){
    if (line.empty()) continue;
    try{
      auto j=J::parse(line);
      std::string name, email;
      if (json_get_string(j, "name", name) && std::regex_search(name, re)) { acc += line + "\n"; continue; }
      if (json_get_string(j, "email", email) && std::regex_search(email, re)) { acc += line + "\n"; continue; }
    } catch(...) {}
  }
  if (!atomic_write_all(out, acc)){ fprintf(stderr,"Failed to write %s\n", out.c_str()); return acx::EX_IO; }
  fprintf(stderr,"grep -> %s\n", out.c_str());
  return acx::EX_OK;
}

static int acx_checksum_cmd(const char* file_path, const char* expected_or_file){
  if (!file_path){ fprintf(stderr,"checksum requires <file> <expected|.sha256>\n"); return acx::EX_USAGE; }
  std::string h = acx::sha256::hash_file(file_path);
  if (h.empty()){ fprintf(stderr,"Failed to read %s\n", file_path); return acx::EX_IO; }
  std::string expected;
  if (expected_or_file && std::strlen(expected_or_file)>0){
    std::string s(expected_or_file);
    if (s.size()>7 && s.rfind(".sha256")==s.size()-7){
      std::ifstream f(s); if (!f){ fprintf(stderr,"Failed to read %s\n", s.c_str()); return acx::EX_IO; }
      std::getline(f, expected);
      if (!expected.empty()){ auto sp=expected.find(' '); if (sp!=std::string::npos) expected=expected.substr(0,sp); }
    } else expected = s;
  } else {
    fprintf(stderr,"Missing expected checksum or .sha256 file\n"); return acx::EX_USAGE;
  }
  bool ok = (lower(h) == lower(expected));
  printf("%s  %s  [%s]\n", h.c_str(), file_path, ok? "OK":"MISMATCH");
  return ok? acx::EX_OK : acx::EX_FAIL;
}


// === Global threading and help-json ===
static size_t acx_threads = 0;

struct LineQueue {
  std::queue<std::string> q;
  std::mutex m;
  std::condition_variable cv;
  bool done=false;
  void push(std::string s){ std::lock_guard<std::mutex> lk(m); q.push(std::move(s)); cv.notify_one(); }
  bool pop(std::string& out){
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [&]{ return !q.empty() || done; });
    if (q.empty()) return false;
    out = std::move(q.front()); q.pop(); return true;
  }
};

static size_t detect_threads(){
  const char* ev = std::getenv("ACX_THREADS");
  if (ev && std::strlen(ev)>0){ try { return (size_t)std::stoull(ev); } catch (...) {} }
  auto n = std::thread::hardware_concurrency();
  return n? (size_t)n : 2;
}

static void acx_print_help_json(){
  printf("{\"name\":\"acx\",\"globals\":[\"--json\",\"--no-color\",\"--threads N\",\"--version\"],"
         "\"commands\":[\"help\",\"version\",\"doctor\",\"selfcheck\",\"config init\","
         "\"hash\",\"report\",\"validate\",\"backup\",\"dedupe\",\"diff\",\"redact\","
         "\"normalize\",\"merge\",\"sample\",\"grep\",\"checksum\"]}\n");
}

// Parallel validator/report using a queue
template<typename Fn>
static void process_lines_parallel(const char* path, size_t threads, Fn fn){
  LineQueue lq;
  std::vector<std::thread> workers;
  for (size_t i=0;i<threads;i++){
    workers.emplace_back([&](){
      std::string line;
      while (lq.pop(line)){
        if (!line.empty()) fn(line);
      }
    });
  }
  { // producer
    std::ifstream ifs(path);
    if (ifs){
      std::string line;
      while (std::getline(ifs, line)) lq.push(std::move(line));
    }
  }
  { std::lock_guard<std::mutex> lk(lq.m); lq.done=true; lq.cv.notify_all(); }
  for (auto& t: workers) t.join();
}


// Threaded versions
static int acx_validate_cmd_thr(bool json_mode, const char* target_path, size_t threads){
  std::string path = (target_path && *target_path) ? target_path : (std::getenv("ACX_DATA_FILE")? std::getenv("ACX_DATA_FILE"): "acx-data.jsonl");
  std::atomic<size_t> count{0}, ok{0}, bad{0};
  auto fn = [&](const std::string& line){
    if(line.empty()) return;
    count++;
    try{
      auto j = J::parse(line);
      std::string name;
      bool has_name = j.is_object() && j.as_object().count("name") && j.as_object().at("name").is_string() && !j.as_object().at("name").as_string().empty();
      bool email_ok = true;
      auto it = j.is_object()? j.as_object().find("email"): j.as_object().end();
      if (it!=j.as_object().end() && it->second.is_string()){
        auto s = it->second.as_string();
        email_ok = (s.find('@')!=std::string::npos && s.find('.')!=std::string::npos);
      }
      if (has_name && email_ok) ok++; else bad++;
    } catch (...) { bad++; }
  };
  process_lines_parallel(path.c_str(), threads, fn);
  if (json_mode) printf("{\"file\":\"%s\",\"records\":%zu,\"valid\":%zu,\"invalid\":%zu}\n", path.c_str(), (size_t)count, (size_t)ok, (size_t)bad);
  else printf("Validate %s\nRecords: %zu\nValid: %zu\nInvalid: %zu\n", path.c_str(), (size_t)count, (size_t)ok, (size_t)bad);
  return bad? acx::EX_VALIDATION : acx::EX_OK;
}

static int acx_report_cmd_thr(bool json_mode, const char* target_path, size_t threads){
  std::string path = (target_path && *target_path) ? target_path : (std::getenv("ACX_DATA_FILE")? std::getenv("ACX_DATA_FILE"): "acx-data.jsonl");
  std::atomic<size_t> count{0};
  std::mutex mu;
  std::set<std::string> uniq;
  auto fn = [&](const std::string& line){
    if (line.empty()) return;
    count++;
    try{
      auto j = J::parse(line);
      if (j.is_object()){
        auto it = j.as_object().find("email");
        if (it!=j.as_object().end() && it->second.is_string()){
          std::string e = it->second.as_string();
          std::lock_guard<std::mutex> lk(mu);
          uniq.insert(e);
        }
      }
    } catch (...) {}
  };
  process_lines_parallel(path.c_str(), threads, fn);
  if (json_mode) printf("{\"file\":\"%s\",\"records\":%zu,\"unique_emails\":%zu}\n", path.c_str(), (size_t)count, uniq.size());
  else printf("File: %s\nRecords: %zu\nUnique emails: %zu\n", path.c_str(), (size_t)count, uniq.size());
  return acx::EX_OK;
}

int main(int argc, char** argv){
    std::setlocale(LC_ALL, "");


// === Graceful interrupt handling ===
static std::atomic<bool> acx_interrupt_flag{false};
auto acx_signal_handler = [](int){ acx_interrupt_flag.store(true); };
std::signal(SIGINT, acx_signal_handler);
std::signal(SIGTERM, acx_signal_handler);

// === Windows UTF-8 console & VT sequences ===
#ifdef _WIN32
try {
  SetConsoleOutputCP(CP_UTF8);
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut != INVALID_HANDLE_VALUE) {
    DWORD mode = 0;
    if (GetConsoleMode(hOut, &mode)) {
      mode |= 0x0004; /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
      SetConsoleMode(hOut, mode);
    }
  }
} catch(...) {}
#endif


// Load configuration from ACX_CONFIG / XDG / ~/.config and apply env defaults
if (auto cfg = acx::load_config()) { acx::apply_config_env(*cfg); }

// Load config (file + env overrides)
std::string cfg_path;
if (args.kv.count("config")) cfg_path = args.kv["config"];
acx::Config cfg = acx::load_config(cfg_path);
if (args.kv.count("dry-run")){
#ifdef _WIN32
    _putenv_s("ACX_DRY_RUN","1");
#else
    setenv("ACX_DRY_RUN","1",1);
#endif
}


// Apply region precedence: CLI --region > config.default_country > env
{
    std::string region = cfg.default_country;
    if (args.kv.count("region")) region = args.kv["region"];
    if (!region.empty()){
    #ifdef _WIN32
        _putenv_s("ACX_DEFAULT_COUNTRY", region.c_str());
    #else
        setenv("ACX_DEFAULT_COUNTRY", region.c_str(), 1);
    #endif
    }
}


auto emit_ok = [&](const std::string& msg){
    if (args.json) {
        acx::J o(std::map<std::string, acx::J>{{"status","ok"},{"message", msg}});
        std::cout << o.dump(0) << "\n";
    } else {
        std::cout << msg << "\n";
    }
};
auto emit_err = [&](const std::string& msg){
    if (args.json) {
        acx::J o(std::map<std::string, acx::J>{{"status","error"},{"message", msg}});
        std::cout << o.dump(0) << "\n";
    } else {
        std::cerr << msg << "\n";
    }
};

    log(acx::LogLevel::INFO, "acx start");
    Args args = parse(argc, argv);
    if (args.cmd=="help" || args.cmd=="--help" || args.cmd=="-h") { help(); return 0; }

    // Select engine: jsonl (default) or sqlite (if built)
    std::unique_ptr<IStorageEngine> eng;
    {
        std::string en = std::getenv("ACX_ENGINE")? std::string(std::getenv("ACX_ENGINE")) : std::string("jsonl");
        if (args.kv.count("engine")) en = args.kv["engine"];
        if (en=="sqlite") eng.reset(make_engine_sqlite());
        if (en=="rocksdb") eng.reset(make_engine_rocksdb());
        if (!eng) eng.reset(make_engine_jsonl());
    }
    ContactBook book;
    eng->load(book);
    ContactBook book;
    if (!st.load(book)) { std::cerr << "Failed to load storage\n"; return 2; }

    try {
        if (args.cmd=="add"){
            std::string id, err;
            std::string user = args.kv.count("user")? args.kv["user"] : "cli";
            bool ok = add_contact(st, book,
                args.kv["name"], args.kv["email"], args.kv["phone"],
                args.kv.count("note")? args.kv["note"]:"",
                user, id, err);
            if (!ok) { std::cerr << "ERROR: " << err << "\n"; return 1; }
            log(acx::LogLevel::INFO, std::string("created: ")+id); std::cout << "Created contact: " << id << "\n";
            return 0;
        } else if (args.cmd=="edit"){ acx::RepoLock lk; if (!lk.acquired()) { std::cerr<<"ERROR: cannot acquire repo lock\n"; return 2; }
            std::string err;
            std::string id = args.kv["id"];
            std::string user = args.kv.count("user")? args.kv["user"] : "cli";
            bool ok = edit_contact(st, book, id,
                args.kv.count("name")? args.kv["name"]:"",
                args.kv.count("email")? args.kv["email"]:"",
                args.kv.count("phone")? args.kv["phone"]:"",
                args.kv.count("note")? args.kv["note"]:"",
                user, err);
            if (!ok) { std::cerr << "ERROR: " << err << "\n"; return 1; }
            log(acx::LogLevel::INFO, std::string("updated: ")+id); std::cout << "Updated contact: " << id << "\n";
            return 0;
        } else if (args.cmd=="delete"){ acx::RepoLock lk; if (!lk.acquired()) { std::cerr<<"ERROR: cannot acquire repo lock\n"; return 2; }
            if (!args.kv.count("yes")) { std::cerr << "Confirm with --yes\n"; return 1; }
            std::string err;
            std::string user = args.kv.count("user")? args.kv["user"] : "cli";
            bool ok = delete_contact(st, book, args.kv["id"], user, err);
            if (!ok) { std::cerr << "ERROR: " << err << "\n"; return 1; }
            log(acx::LogLevel::INFO, std::string("deleted: ")+args.kv["id"]); std::cout << "Deleted contact: " << args.kv["id"] << "\n";
            return 0;
        } else if (args.cmd=="list"){
            int limit = args.kv.count("limit") ? std::stoi(args.kv["limit"]) : 0;
            int n=0;
            for (auto& c : book.contacts){
                std::cout << c.id << " | " << c.name << " | " << c.email << " | " << c.phone << "\n";
                if (limit && ++n>=limit) break;
            }
            return 0;
        } else if (args.cmd=="search"){
            std::string qname = args.kv.count("name")? args.kv["name"]:"";
            std::string qmail = args.kv.count("email")? args.kv["email"]:"";
            std::string qphone= args.kv.count("phone")? args.kv["phone"]:"";
            std::string qid   = args.kv.count("id")? args.kv["id"]:"";
            // Fast path for email/phone using disk index without full load
            if (getenv("ACX_FAST_SEARCH") && ( !qmail.empty() || !qphone.empty() )){
                Contact cfast; Storage st2; if (fast_search_one(st2, qmail, qphone, cfast)){
                    std::cout << cfast.id << " | " << cfast.name << " | " << cfast.email << " | " << cfast.phone << "\n";
                    return 0;
                }
            }
            auto res = eng->search(book, qname, qmail, qphone, qid);
            bool as_json = (args.kv.count("json") || (args.kv.count("output") && args.kv["output"]=="json"));
for (auto& c : res){
  if (as_json) { auto j = c.to_json(); std::cout << j.dump() << "\n"; }
  else { std::cout << c.id << " | " << c.name << " | " << c.email << " | " << c.phone << "\n"; }
}
            return 0;
        } else if (args.cmd=="scrub"){
    std::string pol = args.kv.count("policy") ? args.kv["policy"] : cfg.policy_file;
    acx::Policy p = acx::load_policy(pol);
    acx::ContactBook outBook = (p.redact_email || p.redact_name || p.redact_phone) ? acx::redact_book(book, p) : book;
    std::string err;
    if (args.kv.count("out")){
        if (!acx::save_book_to_path(outBook, args.kv["out"], err)){
            if (args.json){ acx::J o({{"status","error"},{"message",err}}); std::cout<<o.dump()<<"\n"; return 2; }
            std::cerr << "ERROR: " << err << "\n"; return 2;
        }
        if (args.json){ acx::J o({{"status","ok"},{"out",args.kv["out"]}}); std::cout<<o.dump()<<"\n"; }
        else std::cout << "Redacted copy written to " << args.kv["out"] << "\n";
    } else {
        if (!st.save(outBook)){ if(args.json){ acx::J o({{"status","error"},{"message","save failed"}}); std::cout<<o.dump()<<"\n"; } else std::cerr<<"save failed\n"; return 2; }
        if (args.json){ acx::J o({{"status","ok"}}); std::cout<<o.dump()<<"\n"; }
        else std::cout << "Database redacted in-place\n";
    }
    return 0;
} else if (args.cmd=="export"){
    std::string fmt = args.kv["format"];
    std::string out = args.kv["out"];
    std::string pol = args.kv.count("policy") ? args.kv["policy"] : cfg.policy_file;
    acx::Policy p = acx::load_policy(pol);
    acx::ContactBook outBook = p.redact_email || p.redact_name || p.redact_phone ? acx::redact_book(book, p) : book;
    std::string err;
    if (!acx::export_data(outBook, fmt, out, err)){
        if (args.json){
            acx::J o(std::map<std::string,acx::J>{{"status","error"},{"message",err}});
            std::cout << o.dump() << "\n"; return 2;
        } else { std::cerr << "ERROR: " << err << "\n"; return 2; }
    }
    if (args.json){
        acx::J o(std::map<std::string,acx::J>{{"status","ok"},{"out",out}});
        std::cout << o.dump() << "\n";
    } else std::cout << "Exported to " << out << "\n";
    return 0;

            std::string err;
            bool ok = export_data(book, args.kv["format"], args.kv["out"], err);
            if (!ok) { std::cerr << "ERROR: " << err << "\n"; return 2; }
            std::cout << "Exported to " << args.kv["out"] << "\n";
            return 0;
        } else if (args.cmd=="import"){ acx::RepoLock lk; if (!lk.acquired()) { std::cerr<<"ERROR: cannot acquire repo lock\n"; return 2; }
            std::string err;
            bool par = args.kv.count("parallel");
            bool ok = eng->import_data(book, args.kv["format"], args.kv["in"], par, err);
            if (!ok) { std::cerr << "ERROR: " << err << "\n"; return 2; }
            acx::otel_event("import", {{"format", args.kv["format"]}, {"parallel", par? "1":"0"}});
            std::cout << "Imported from " << args.kv["in"] << (par? " (parallel)":"") << "\n";
            return 0;
        } else if (args.cmd=="backup"){ acx::RepoLock lk; if (!lk.acquired()) { std::cerr<<"ERROR: cannot acquire repo lock\n"; return 2; }
            bool ok = Storage().backup_last();
            if (!ok) { emit_err("ERROR: backup failed"); return 2; }
            emit_ok("Backup created");
            return 0;
        } else if (args.cmd=="sbom"){
            std::string err;
            SBOMFormat fmt = SBOMFormat::SPDX;
            if (args.kv.count("format")) {
                std::string f = args.kv["format"];
                if (f=="cyclonedx") fmt = SBOMFormat::CycloneDX;
            }
            if (!args.kv.count("out")) { std::cerr << "ERROR: missing --out\n"; return 2; }
            bool ok = generate_sbom(std::filesystem::current_path(), fmt, args.kv["out"], err);
            if (!ok){ std::cerr << "ERROR: " << err << "\n"; return 2; }
            std::cout << "SBOM written to " << args.kv["out"] << "\n";
            return 0;
        } else if (args.cmd=="checksum"){
    std::string f = args.kv["file"];
    auto data = acx::read_file(f);
    auto h = acx::sha256_hex(data);
    if (args.json){ acx::J o({{"status","ok"},{"sha256",h}}); std::cout<<o.dump()<<"\n"; }
    else std::cout << h << "\n";
    return 0;
} else if (args.cmd=="sign"){
    std::string f = args.kv["file"];
    const char* key = std::getenv("ACX_HMAC_KEY");
    if (!key){ std::cerr << "ACX_HMAC_KEY not set\n"; return 2; }
    auto data = acx::read_file(f);
    auto sig = acx::hmac_sha256_hex(key, data);
    if (args.json){ acx::J o({{"status","ok"},{"sig",sig}}); std::cout<<o.dump()<<"\n"; }
    else std::cout << sig << "\n";
    return 0;
} else if (args.cmd=="verify"){
    std::string f = args.kv["file"];
    std::string sig = args.kv["sig"];
    const char* key = std::getenv("ACX_HMAC_KEY");
    if (!key){ std::cerr << "ACX_HMAC_KEY not set\n"; return 2; }
    auto data = acx::read_file(f);
    auto expected = acx::hmac_sha256_hex(key, data);
    bool ok = (expected.size()==sig.size());
    if (ok){ for (size_t i=0;i<sig.size();++i){ ok = ok && (expected[i]==sig[i]); } }
    if (args.json){ acx::J o({{"status", ok?"ok":"error"}}); std::cout<<o.dump()<<"\n"; }
    else std::cout << (ok?"OK\n":"FAIL\n");
    return ok?0:2;
} else if (args.cmd=="encrypt"){
    std::string in = args.kv["in"], out = args.kv["out"];
    std::string aad = args.kv.count("aad")? args.kv["aad"] : "";
    const char* k = std::getenv("ACX_ENC_KEY");
    const char* kf = std::getenv("ACX_ENC_KEY_FILE");
    std::string keyhex;
    if (k && *k) keyhex = k;
    else if (kf && *kf) keyhex = acx::read_file(kf);
    else { std::cerr << "Set ACX_ENC_KEY (hex) or ACX_ENC_KEY_FILE (hex)\n"; return 2; }
    std::string err;
    bool ok = acx::encrypt_file_aes256gcm(in, out, keyhex, aad, err);
    if (args.json){ acx::J o({{"status", ok?"ok":"error"}}); if(!ok) o["message"]=err; std::cout<<o.dump()<<"\n"; }
    else { if(!ok) std::cerr<<"ERROR: "<<err<<"\n"; else std::cout<<"Encrypted -> "<<out<<"\n"; }
    return ok?0:2;
} else if (args.cmd=="decrypt"){
    std::string in = args.kv["in"], out = args.kv["out"];
    const char* k = std::getenv("ACX_ENC_KEY");
    const char* kf = std::getenv("ACX_ENC_KEY_FILE");
    std::string keyhex;
    if (k && *k) keyhex = k;
    else if (kf && *kf) keyhex = acx::read_file(kf);
    else { std::cerr << "Set ACX_ENC_KEY (hex) or ACX_ENC_KEY_FILE (hex)\n"; return 2; }
    std::string err;
    bool ok = acx::decrypt_file_aes256gcm(in, out, keyhex, err);
    if (args.json){ acx::J o({{"status", ok?"ok":"error"}}); if(!ok) o["message"]=err; std::cout<<o.dump()<<"\n"; }
    else { if(!ok) std::cerr<<"ERROR: "<<err<<"\n"; else std::cout<<"Decrypted -> "<<out<<"\n"; }
    return ok?0:2;
} else if (args.cmd=="keygen"){
    size_t n = 32; if (args.kv.count("bytes")) n = (size_t)std::stoul(args.kv["bytes"]);
    auto hx = acx::random_key_hex(n);
    if (args.json){ acx::J o({{"status","ok"},{"key_hex",hx}}); std::cout<<o.dump()<<"\n"; }
    else std::cout << hx << "\n";
    return 0;
} else if (args.cmd=="diff"){
    std::string fmt = args.kv["fmt"]; std::string A = args.kv["a"]; std::string B = args.kv["b"];
    acx::ContactBook a,b; std::string err;
    if (!acx::load_book_from_file(fmt, A, a, err)){ std::cerr << "ERROR: "<<err<<"\n"; return 2; }
    if (!acx::load_book_from_file(fmt, B, b, err)){ std::cerr << "ERROR: "<<err<<"\n"; return 2; }
    auto out = acx::diff_books_json(a,b);
    if (args.json){ std::cout<< out << "\n"; }
    else { std::cout << out << "\n"; }
    return 0;
} else if (args.cmd=="doctor"){
    std::map<std::string, acx::J> o;
    o["version"] = acx::version_string();
    o["git_sha"] = acx::git_sha();
    o["build_time"] = acx::build_time_utc();
#ifdef ACX_HAVE_OPENSSL
    o["crypto_backend"] = "OpenSSL";
#else
    o["crypto_backend"] = "internal";
#endif
    auto env = [&](const char* k){ const char* v=getenv(k); return v? std::string(v): std::string(); };
    auto mask = [](const std::string& s){ if (s.size()<=6) return std::string(s.size(),'x'); return s.substr(0,2)+std::string(s.size()-6,'x')+s.substr(s.size()-4); };
    o["env.ACX_ENGINE"] = env("ACX_ENGINE");
    o["env.ACX_DEFAULT_COUNTRY"] = env("ACX_DEFAULT_COUNTRY");
    std::string h = env("ACX_HMAC_KEY"); if(!h.empty()) h = mask(h);
    o["env.ACX_HMAC_KEY"] = h;
    o["env.ACX_HMAC_KEY_FILE"] = !env("ACX_HMAC_KEY_FILE").empty();
    std::string ek = env("ACX_ENC_KEY"); if(!ek.empty()) ek = mask(ek);
    o["env.ACX_ENC_KEY"] = ek;
    o["env.ACX_ENC_KEY_FILE"] = !env("ACX_ENC_KEY_FILE").empty();
    if (args.json){ acx::J j(o); std::cout<<j.dump()<<"\n"; }
    else { acx::J j(o); std::cout<<j.dump(2)<<"\n"; }
    return 0;
} else if (args.cmd=="schema"){

    auto itList = args.kv.find("list");
    auto itName = args.kv.find("name");
    if (itList != args.kv.end()){
        std::cout << "add\nlist\nsearch\naudit\nbackup\n";
        return 0;
    } else if (itName != args.kv.end()){
        std::string nm = itName->second;
        std::string file = "schemas/" + nm + ".json";
        try {
            std::cout << acx::read_file(file) << "\n";
            return 0;
        } catch (...) {
            std::cerr << "Unknown schema name\n";
            return 2;
        }
    } else {
        std::cout << "Usage:\n  --json           Output machine-readable JSON when supported\n  --no-color       Disable ANSI colors\n
  # Optional envs: ACX_AUDIT_LOG=path (append JSONL events), ACX_LOCK_DIR=path
  # Global: --dry-run simulates state changes (no writes) acx schema --list | --name NAME\n";
        return 2;
    }
} else if (args.cmd=="metrics"){
            std::cout << metrics_json() << "\n"; return 0;
        } else if (args.cmd=="merge"){ acx::RepoLock lk; if (!lk.acquired()) { std::cerr<<"ERROR: cannot acquire repo lock\n"; return 2; }
            if (!args.kv.count("format") || !args.kv.count("in")){ std::cerr<<"ERROR: --format and --in required\n"; return 2; }
            std::string s = args.kv.count("strategy")? args.kv["strategy"]:"skip";
            MergeStrategy strat = MergeStrategy::Skip;
            if (s=="prefer-newer") strat = MergeStrategy::PreferNewer; else if (s=="prefer-existing") strat = MergeStrategy::PreferExisting;
            std::string err; Storage st2; bool ok = merge_from_file(st2, book, args.kv["format"], args.kv["in"], strat, err);
            if (!ok){ std::cerr<<"ERROR: "<<err<<"\n"; return 2; }
            std::cout << "Merged "<< args.kv["in"] <<" with strategy "<< s <<"\n"; return 0;
        } else if (args.cmd=="doctor"){
            std::cout << run_doctor() << "\n";
            return 0;
        } else if (args.cmd=="validate"){
            std::string path = args.kv.count("policy")? args.kv["policy"] : "policy.json";
            std::string err; auto pol = acx::Policy::load_from_file(path, err);
            if (!err.empty()) { std::cerr << "ERROR: " << err << "\n"; return 2; }
            size_t bad=0; for (auto& c : book.contacts){ std::string why; if (!pol.validate_contact(c, why)){ ++bad; if (args.kv.count("output") && args.kv["output"]=="json"){ auto j=c.to_json(); j["policy_error"]=why; std::cout<< j.dump() << "\n"; } else { std::cout << c.id << " " << why << "\n"; } } }
            std::cout << "violations=" << bad << "\n";
            return bad? 1:0;
        } else if (args.cmd=="diff"){
            if (!args.kv.count("format") || !args.kv.count("in")){ std::cerr<<"ERROR: --format and --in required\n"; return 2; }
            acx::DiffResult dr; std::string err; Storage st2; if (!compute_diff(st2, book, args.kv["format"], args.kv["in"], dr, err)){ std::cerr<<"ERROR: "<<err<<"\n"; return 2; }
            bool json = args.kv.count("output") && args.kv["output"]=="json";
            if (json){ J o; J a=J::array(), rm=J::array(), ch=J::array(); for(auto& x:dr.added)a.push_back(x.to_json()); for(auto& x:dr.removed)rm.push_back(x.to_json()); for(auto& x:dr.changed)ch.push_back(x.to_json()); o["added"]=a; o["removed"]=rm; o["changed"]=ch; std::cout<<o.dump()<<"\n"; }
            else { std::cout << "+"<<dr.added.size()<<" -"<<dr.removed.size()<<" ~"<<dr.changed.size()<<"\n"; }
            return 0;
        } else if (args.cmd=="verify-file"){
            if (!args.kv.count("file") || !args.kv.count("sha256")){ std::cerr<<"ERROR: --file and --sha256 required\n"; return 2; }
            bool ok = acx::verify_sha256_file(args.kv["file"], args.kv["sha256"]); std::cout << (ok? "OK":"MISMATCH") << "\n"; return ok?0:3;
        } else if (args.cmd=="prune"){
            if (!args.kv.count("before")){ std::cerr<<"ERROR: --before YYYY-mm-dd required\n"; return 2; }
            acx::RepoLock lk; if (!lk.acquired()) { std::cerr<<"ERROR: cannot acquire repo lock\n"; return 2; }
            std::string cut = args.kv["before"];
            ContactBook out; for (auto& c : book.contacts){ if (c.created_at < cut) continue; out.contacts.push_back(c); }
            if (args.kv.count("dry-run")){ std::cout << "Would remove " << (book.contacts.size()-out.contacts.size()) << " contacts\n"; return 0; }
            Storage st2; bool ok = st2.save(out); std::cout << "Pruned " << (book.contacts.size()-out.contacts.size()) << " contacts\n"; return ok?0:2;
        } else if (args.cmd=="selftest"){
            std::cout << run_selftest_json() << "\n"; return 0;
        } else if (args.cmd=="determinism-check"){
            std::cout << run_determinism_check_json() << "\n"; return 0;
        } else if (args.cmd=="verify-backup"){
            if (!args.kv.count("file") || !args.kv.count("key")){ std::cerr << "ERROR: --file and --key required\n"; return 2; }
            bool ok = Storage().verify_backup_signature(args.kv["file"], args.kv["key"]);
            std::cout << (ok? "VALID":"INVALID") << "\n"; return ok?0:1;
        } else if (false) {{
            bool ok = Storage().backup_last();
            if (!ok) { emit_err("ERROR: backup failed"); return 2; }
            emit_ok("Backup created");
            return 0;
        } else if (args.cmd=="audit"){
            bool strict = args.kv.count("strict")>0;
            int rc = run_audit(strict); if(args.json){ acx::J o(std::map<std::string,acx::J>{{"status", rc==0?"ok":"error"},{"code",(double)rc}}); std::cout<<o.dump()<<"\n"; return rc;} else { return rc; }
        } else {
            help(); return 2;
        }
    } catch (const std::exception& e){
        std::cerr << "Unhandled exception: " << e.what() << "\n";
        return 2;
    }
}
