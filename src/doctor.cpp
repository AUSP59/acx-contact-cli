#include "doctor.hpp"
#include "utils.hpp"
#include "hmac.hpp"
#include "index.hpp"
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;
namespace acx {

std::string run_doctor(){
    std::ostringstream o;
    Storage st;
    ContactBook b;
    bool ok = st.load(b);
    o << "{";
    o << "\"data_file_exists\":" << (fs::exists(st.data_file)?"true":"false") << ",";
    std::string data = fs::exists(st.data_file)? read_file(st.data_file) : std::string();
    std::string dh = data.empty()? "" : sha256_hex(data);
    o << "\"data_sha256\":\"" << dh << "\",";
    // index check
    PersistentIndex pi;
    bool idx_ok = false;
    if (fs::exists("acx-index.json")){
        idx_ok = pi.load_or_build(st.data_file, b);
    }
    o << "\"index_ok\":" << (idx_ok?"true":"false") << ",";
    // latest backup signature check if present
    std::string verdict="unknown";
    if (fs::exists("backups")){
        for (auto& p : fs::directory_iterator("backups")){
            if (p.path().extension()==".sig"){
                std::string sig = read_file(p.path());
                // cannot verify without key; just report presence
                verdict="present";
            }
        }
    }
    o << "\"backup_signature\":\"" << verdict << "\"";
    o << "}";
    return o.str();
}

} // namespace acx
