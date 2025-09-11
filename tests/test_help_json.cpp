#include "unit_micro.hpp"
#include <cstdio>
#include <array>
#include <string>
#include <memory>

static std::string run(const char* cmd){
  std::array<char, 4096> b{}; std::string out;
  FILE* p = popen(cmd, "r"); if(!p) return out;
  while (fgets(b.data(), b.size(), p)) out += b.data();
  pclose(p); return out;
}

UT(help_json_lists_commands){
  auto out = run("./build/acx help --json 2>/dev/null");
  OK(out.find("\"commands\"") != std::string::npos);
  OK(out.find("\"validate\"") != std::string::npos);
}
int main(){ ut::run(); }
