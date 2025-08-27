
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

UT(checksum_ok){
  system("echo hello > ck.txt");
  auto h = run("./build/acx hash ck.txt --json 2>/dev/null");
  size_t p=h.find(\"\\\"sha256\\\":\\\"\"); if (p==std::string::npos) return 0;
  auto hex = h.substr(p+12, 64);
  std::string cmd = std::string(\"./build/acx checksum ck.txt \")+hex+\" 2>/dev/null\";
  int rc = system(cmd.c_str());
  OK(rc==0);
}
int main(){ ut::run(); }
