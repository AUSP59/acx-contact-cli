#include "unit_micro.hpp"
#include <cstdio>
#include <array>
#include <string>
#include <memory>

static std::string run(const char* cmd){
  std::array<char, 1024> b{}; std::string out;
  FILE* p = popen(cmd, "r"); if(!p) return out;
  while (fgets(b.data(), b.size(), p)) out += b.data();
  pclose(p); return out;
}

UT(doctor_runs){
  auto out = run("./build/acx doctor --json 2>/dev/null");
  OK(out.find("\"doctor\"") != std::string::npos);
}
int main(){ ut::run(); }
