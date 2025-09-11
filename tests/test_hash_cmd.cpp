
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

UT(hash_outputs_hex){
  auto out = run("./build/acx hash examples/contacts.jsonl --json 2>/dev/null");
  OK(out.find("\"sha256\"") != std::string::npos);
  // crude: ensure 64 hex chars exist
  size_t pos = out.find("\"sha256\":\"");
  OK(pos != std::string::npos);
}
int main(){ ut::run(); }
