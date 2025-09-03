#include "index.hpp"
#include "utils.hpp"
#include "jsonl.hpp"
#include <fstream>
#include <sstream>

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

bool PersistentIndex::load_or_build(const fs::path& data_file, ContactBook& book){
    // compute data hash
    std::string data = read_file(data_file);
    std::string dh = sha256_hex(data);
    if (fs::exists(index_file)){
        std::string idx = read_file(index_file);
        // very small parser: expect lines "key=value"
        // format: first line: hash=<hex>
        // next: email:<email>=<pos>
        // next: phone:<phone>=<pos>
        std::istringstream iss(idx);
        std::string line;
        std::string hashline;
        if (std::getline(iss, hashline)){
            if (hashline.rfind("hash=",0)==0 && hashline.substr(5)==dh){
                // matches, load mappings
                book.email_to_i.clear(); book.phone_to_i.clear();
                size_t pos;
                while (std::getline(iss, line)){
                    if (line.rfind("email:",0)==0){
                        auto eq = line.find('=');
                        if (eq!=std::string::npos){
                            std::string key = line.substr(6, eq-6);
                            pos = (size_t)std::stoull(line.substr(eq+1));
                            book.email_to_i[key]=pos;
                        }
                    } else if (line.rfind("phone:",0)==0){
                        auto eq = line.find('=');
                        if (eq!=std::string::npos){
                            std::string key = line.substr(6, eq-6);
                            pos = (size_t)std::stoull(line.substr(eq+1));
                            book.phone_to_i[key]=pos;
                        }
                    }
                }
                return true;
            }
        }
    }
    // rebuild
    book.email_to_i.clear(); book.phone_to_i.clear();
    for (size_t i=0;i<book.contacts.size();++i){
        book.email_to_i[to_lower_ascii(book.contacts[i].email)] = i;
        book.phone_to_i[normalize_phone(book.contacts[i].phone)] = i;
    }
    // save
    return save(data_file, book);
}

bool PersistentIndex::save(const fs::path& data_file, const ContactBook& book){
    std::string data = read_file(data_file);
    std::string dh = sha256_hex(data);
    std::ostringstream o;
    o << "hash=" << dh << "\n";
    for (auto& kv : book.email_to_i){
        o << "email:" << kv.first << "=" << kv.second << "\n";
    }
    for (auto& kv : book.phone_to_i){
        o << "phone:" << kv.first << "=" << kv.second << "\n";
    }
    return write_file_atomic(index_file, o.str());
}

} // namespace acx
