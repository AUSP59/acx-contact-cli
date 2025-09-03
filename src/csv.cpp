#include "csv.hpp"
#include <string>
#include <vector>

namespace acx {

bool csv_parse_rfc4180(const std::string& input, std::vector<std::vector<std::string>>& out, std::string& err){
    out.clear();
    std::vector<std::string> row;
    std::string field;
    bool in_quotes=false;
    for (size_t i=0;i<input.size();++i){
        char c = input[i];
        if (in_quotes){
            if (c=='"'){
                if (i+1<input.size() && input[i+1]=='"'){ field.push_back('"'); ++i; }
                else { in_quotes=false; }
            } else {
                field.push_back(c);
            }
        } else {
            if (c=='"'){ in_quotes=true; }
            else if (c==','){ row.push_back(field); field.clear(); }
            else if (c=='\r'){
                if (i+1<input.size() && input[i+1]=='\n'){ ++i; }
                row.push_back(field); field.clear(); out.push_back(row); row.clear();
            } else if (c=='\n'){
                row.push_back(field); field.clear(); out.push_back(row); row.clear();
            } else {
                field.push_back(c);
            }
        }
    }
    // last field/row
    row.push_back(field);
    if (!row.empty()) out.push_back(row);
    // sanity: all rows same width?
    size_t width = out.empty()? 0 : out[0].size();
    for (auto& r : out){ if (r.size() != width){ err="Inconsistent column count"; return false; } }
    return true;
}

} // namespace acx
