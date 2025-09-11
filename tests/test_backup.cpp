
#include "unit_micro.hpp"
#include <cstdio>
#include <string>
#include <array>

static std::string run(const char* cmd){
  std::array<char, 4096> b{}; std::string out;
  FILE* p = popen(cmd, "r"); if(!p) return out;
  while (fgets(b.data(), b.size(), p)) out += b.data();
  pclose(p); return out;
}

UT(backup_creates_file){
  auto out = run("./build/acx backup examples/contacts.jsonl backups 2>/dev/null");
  OK(out.find("backups/") != std::string::npos);
}
int main(){ ut::run(); }
