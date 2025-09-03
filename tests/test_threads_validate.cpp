#include "unit_micro.hpp"
#include <cstdlib>

UT(threads_flag_does_not_crash){
  int rc = system("ACX_THREADS=2 ./build/acx validate examples/contacts.jsonl --json >/dev/null 2>&1");
  OK(rc==0);
  rc = system("./build/acx --threads 3 validate examples/contacts.jsonl --json >/dev/null 2>&1");
  OK(rc==0);
}
int main(){ ut::run(); }
