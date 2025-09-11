#include "unit_micro.hpp"
#include "src/config.hpp"
#include <fstream>

UT(config_loads_file){
    const char* p = "examples/config.json";
    std::ofstream f(p); f << "{\"data_file\":\"/tmp/acx-data.jsonl\",\"region\":\"MX\"}"; f.close();
    auto c = acx::load_config_file(p);
    OK(bool(c));
    OK(c->data_file == "/tmp/acx-data.jsonl");
    OK(c->region == "MX");
}
int main(){ ut::run(); }
