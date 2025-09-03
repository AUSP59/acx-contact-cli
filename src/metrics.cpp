#include "metrics.hpp"
#include "storage.hpp"
#include "utils.hpp"
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

namespace acx {

std::string metrics_json(){
    Storage st;
    ContactBook b;
    st.load(b);
    std::ostringstream o;
    o << "{";
    o << ""timestamp":"" << now_iso8601_utc() << "",";
    o << ""contacts":" << b.contacts.size() << ",";
    auto data_size = fs::exists(st.data_file)? fs::file_size(st.data_file) : 0;
    auto audit_size = fs::exists(st.audit_log)? fs::file_size(st.audit_log) : 0;
    o << ""data_bytes":" << data_size << ",";
    o << ""audit_bytes":" << audit_size;
    o << "}";
    return o.str();
}

} // namespace acx
