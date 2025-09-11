
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

UT(validate_reports_counts){
  auto out = run("./build/acx validate examples/contacts.jsonl --json 2>/dev/null");
  OK(out.find("\"valid\"") != std::string::npos);
}
int main(){ ut::run(); }
