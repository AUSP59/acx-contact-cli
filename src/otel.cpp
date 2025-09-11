#include "otel.hpp"
#include <cstdlib>
#include <iostream>

namespace acx {

bool otel_enabled(){
    const char* v = std::getenv("ACX_OTEL");
    return v && std::string(v) == "1";
}

static std::string esc(const std::string& s){
    std::string o; o.reserve(s.size()+8);
    for(char c: s){
        switch(c){
            case '"': o += "\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n"; break;
            default: o += c;
        }
    }
    return o;
}

void otel_event(const std::string& name, const std::map<std::string, std::string>& attrs){
    if (!otel_enabled()) return;
    std::cout << "{"type":"event","name":"" << esc(name) << "","attrs":{";
    size_t i=0;
    for (auto& kv : attrs){
        std::cout << "\"" << esc(kv.first) << "\":\"" << esc(kv.second) << "\"";
        if (++i<attrs.size()) std::cout << ",";
    }
    std::cout << "}}" << std::endl;
}

void otel_metric(const std::string& name, double value, const std::map<std::string, std::string>& attrs){
    if (!otel_enabled()) return;
    std::cout << "{"type":"metric","name":"" << esc(name) << "","value":" << value << ","attrs":{";
    size_t i=0;
    for (auto& kv : attrs){
        std::cout << "\"" << esc(kv.first) << "\":\"" << esc(kv.second) << "\"";
        if (++i<attrs.size()) std::cout << ",";
    }
    std::cout << "}}" << std::endl;
}

} // namespace acx
