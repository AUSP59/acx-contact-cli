#include "unit_micro.hpp"
#include "src/storage.hpp"
#include <cstdlib>
#include <string>

UT(storage_env_overrides){
    setenv("ACX_DATA_FILE","/tmp/acx-test.jsonl",1);
    acx::Storage st;
    OK(st.data_file.string().find("acx-test.jsonl")!=std::string::npos);
}
int main(){ ut::run(); }
