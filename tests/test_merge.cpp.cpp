
#include "unit_micro.hpp"
#include <sys/stat.h>
#include <cstdlib>
static bool exists(const std::string& p){ struct stat st{}; return ::stat(p.c_str(), &st)==0; }

UT(merge_produces_output){
  system("printf '{\"email\":\"a@x\"}\n' > f1.jsonl; printf '{\"email\":\"a@x\"}\n{\"email\":\"b@x\"}\n' > f2.jsonl");
  int rc = system("./build/acx merge out-merged.jsonl f1.jsonl f2.jsonl >/dev/null 2>&1");
  OK(rc==0);
  OK(exists("out-merged.jsonl"));
}
int main(){ ut::run(); }
