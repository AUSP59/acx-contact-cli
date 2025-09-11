#include "unit_micro.hpp"
#include <cstdlib>
#include <sys/stat.h>
#include <string>

static bool exists(const std::string& p){ struct stat st{}; return ::stat(p.c_str(), &st)==0; }

UT(config_init_creates_file){
  const char* home = std::getenv("HOME");
  if (!home) return 0;
  system("rm -rf ~/.config/acx"); // best-effort
  int rc = system("./build/acx config init >/dev/null 2>&1");
  OK(rc==0);
  OK(exists(std::string(home)+"/.config/acx/config.json"));
}
int main(){ ut::run(); }
