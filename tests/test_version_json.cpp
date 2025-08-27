#include "unit_micro.hpp"
#include <cstdlib>
#include <cstdio>
#include <string>
#include <array>
#include <memory>

static std::string run(const char* cmd){
  std::array<char, 4096> buf{};
  std::string out;
  FILE* p = popen(cmd, "r");
  if(!p) return out;
  while (fgets(buf.data(), buf.size(), p)) out += buf.data();
  pclose(p);
  return out;
}

UT(version_json_shape){
  auto o = run("./build/acx --version --json 2>/dev/null");
  OK(o.find("\"version\"") != std::string::npos);
  OK(o.find("\"git\"") != std::string::npos);
}
int main(){ ut::run(); }
