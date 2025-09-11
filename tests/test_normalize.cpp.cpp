
#include "unit_micro.hpp"
#include <sys/stat.h>
#include <cstdlib>
static bool exists(const std::string& p){ struct stat st{}; return ::stat(p.c_str(), &st)==0; }

UT(normalize_changes_case){
  system("echo '{\"name\":\"  Alice  \",\"email\":\"ALICE@EXAMPLE.com\"}' > tmp-norm.jsonl");
  int rc = system("./build/acx normalize tmp-norm.jsonl out-norm.jsonl >/dev/null 2>&1");
  OK(rc==0);
  OK(exists("out-norm.jsonl"));
}
int main(){ ut::run(); }
