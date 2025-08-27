#include "sbom.hpp"
#include "utils.hpp"
#include <vector>
#include <map>
#include <fstream>

namespace fs = std::filesystem;

namespace acx {

static std::string json_escape(const std::string& s){
    std::string o; o.reserve(s.size()+8);
    for (char c : s){
        switch(c){
            case '"': o += "\\""; break;
            case '\\': o += "\\\\"; break;
            case '\n': o += "\\n"; break;
            case '\r': o += "\\r"; break;
            case '\t': o += "\\t"; break;
            default: o += c; break;
        }
    }
    return o;
}

static bool write_file(const fs::path& p, const std::string& content){
    std::ofstream out(p, std::ios::binary);
    if (!out) return false;
    out << content;
    return (bool)out;
}

bool generate_sbom(const fs::path& root, SBOMFormat fmt, const fs::path& out_file, std::string& err){
    auto files = list_files_recursive(root);
    std::string out;
    if (fmt == SBOMFormat::SPDX){
        out += "{\n";
        out += "  "spdxVersion": "SPDX-2.3",\n";
        out += "  "dataLicense": "CC0-1.0",\n";
        out += "  "SPDXID": "SPDXRef-DOCUMENT",\n";
        out += "  "name": "" + json_escape(root.filename().string()) + "",\n";
        out += "  "creationInfo": {"created": "" + now_iso8601_utc() + "", "creators": ["Tool: ACX"]},\n";
        out += "  "files": [\n";
        for (size_t i=0;i<files.size();++i){
            const auto& p = files[i];
            std::string rel = fs::relative(p, root).generic_string();
            std::string sha = sha256_file_hex(p.string());
            out += "    {"fileName": "" + json_escape(rel) + "", "SPDXID": "SPDXRef-" + json_escape(rel) + "", "checksums": [{"algorithm": "SHA256", "checksumValue": "" + sha + ""}]}";
            if (i+1<files.size()) out += ",";
            out += "\n";
        }
        out += "  ]\n}\n";
    } else {
        out += "{\n";
        out += "  "bomFormat": "CycloneDX",\n";
        out += "  "specVersion": "1.4",\n";
        out += "  "version": 1,\n";
        out += "  "metadata": {"timestamp": "" + now_iso8601_utc() + "", "tools": [{"name":"ACX"}], "component": {"name":"" + json_escape(root.filename().string()) + ""}},\n";
        out += "  "components": [\n";
        for (size_t i=0;i<files.size();++i){
            const auto& p = files[i];
            std::string rel = fs::relative(p, root).generic_string();
            std::string sha = sha256_file_hex(p.string());
            out += "    {"type":"file","name":"" + json_escape(rel) + "","hashes":[{"alg":"SHA-256","content":"" + sha + ""}]}";
            if (i+1<files.size()) out += ",";
            out += "\n";
        }
        out += "  ]\n}\n";
    }
    if (!write_file(out_file, out)){ err="Failed to write SBOM"; return false; }
    return true;
}

} // namespace acx
