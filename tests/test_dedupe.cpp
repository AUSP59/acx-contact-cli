
#include "unit_micro.hpp"
#include <sys/stat.h>
#include <string>
#include <cstdlib>

static bool exists(const std::string& p){ struct stat st{}; return ::stat(p.c_str(), &st)==0; }

UT(dedupe_writes_output){
  system("cp examples/contacts.jsonl tmp-ded.jsonl");
  int rc = system("./build/acx dedupe tmp-ded.jsonl out-ded.jsonl >/dev/null 2>&1");
  OK(rc==0);
  OK(exists("out-ded.jsonl"));
}
int main(){ ut::run(); }
