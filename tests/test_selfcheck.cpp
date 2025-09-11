#include "unit_micro.hpp"
#include <cstdio>
#include <array>
#include <string>

static std::string run(const char* cmd){
  std::array<char, 1024> b{}; std::string out;
  FILE* p = popen(cmd, "r"); if(!p) return out;
  while (fgets(b.data(), b.size(), p)) out += b.data();
  pclose(p); return out;
}

UT(selfcheck_runs){
  auto out = run("./build/acx selfcheck --json 2>/dev/null");
  OK(out.find("\"selfcheck\"") != std::string::npos);
}
int main(){ ut::run(); }
