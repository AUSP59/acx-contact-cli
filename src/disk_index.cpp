#include "disk_index.hpp"
#include "utils.hpp"
#include <fstream>
#include <algorithm>

namespace fs = std::filesystem;

namespace acx {

static void write_u32(std::ofstream& o, uint32_t v){ o.write((const char*)&v, sizeof(v)); }
static void write_u64(std::ofstream& o, uint64_t v){ o.write((const char*)&v, sizeof(v)); }
static uint32_t read_u32(std::ifstream& i){ uint32_t v; i.read((char*)&v, sizeof(v)); return v; }
static uint64_t read_u64(std::ifstream& i){ uint64_t v; i.read((char*)&v, sizeof(v)); return v; }

bool DiskIndex::build(const fs::path& data_file,
                      const std::vector<std::pair<std::string,uint64_t>>& email_kv,
                      const std::vector<std::pair<std::string,uint64_t>>& phone_kv){
    // meta
    std::string data = read_file(data_file);
    std::string dh = sha256_hex(data);
    write_file_atomic(meta, std::string("hash=")+dh+"\n");

    // file: MAGIC | ver | ec | pc | blocks
    std::ofstream o(file, std::ios::binary);
    if (!o) return false;
    const char magic[8] = {'A','C','X','I','D','X','2','\0'};
    o.write(magic, 8);
    write_u32(o, 1); // version
    write_u32(o, (uint32_t)email_kv.size());
    write_u32(o, (uint32_t)phone_kv.size());

    auto write_block = [&](const std::vector<std::pair<std::string,uint64_t>>& kv){
        // sort by key
        std::vector<std::pair<std::string,uint64_t>> v = kv;
        std::sort(v.begin(), v.end(), [](auto& a, auto& b){ return a.first < b.first; });
        for (auto& p : v){
            uint16_t klen = (uint16_t)std::min<size_t>(p.first.size(), 65535);
            o.write((const char*)&klen, sizeof(klen));
            o.write(p.first.data(), klen);
            write_u64(o, p.second);
        }
    };
    write_block(email_kv);
    write_block(phone_kv);
    return (bool)o;
}

bool DiskIndex::valid_for_data_hash(const std::string& data_sha) const {
    if (!fs::exists(meta)) return false;
    std::string m = read_file(meta);
    if (m.rfind("hash=",0)!=0) return false;
    std::string h = m.substr(5);
    if (!h.empty() && (h.back()=='\n' || h.back()=='\r')) h.pop_back();
    return h == data_sha;
}

static bool load_block(std::ifstream& in, uint32_t count, std::vector<DiskIndexEntry>& out){
    out.clear(); out.reserve(count);
    for (uint32_t i=0;i<count;++i){
        uint16_t klen; in.read((char*)&klen, sizeof(klen));
        std::string key(klen, '\0'); in.read(&key[0], klen);
        uint64_t off = read_u64(in);
        out.push_back({key, off});
    }
    return (bool)in;
}

bool DiskIndex::load(std::vector<DiskIndexEntry>& email, std::vector<DiskIndexEntry>& phone) const{
    std::ifstream in(file, std::ios::binary);
    if (!in) return false;
    char magic[8]; in.read(magic,8);
    if (std::string(magic, magic+7)!="ACXIDX2") return false;
    uint32_t ver = read_u32(in); (void)ver;
    uint32_t ec = read_u32(in);
    uint32_t pc = read_u32(in);
    if (!load_block(in, ec, email)) return false;
    if (!load_block(in, pc, phone)) return false;
    return true;
}

static bool bsearch_key(const std::vector<DiskIndexEntry>& v, const std::string& key, uint64_t& out_off){
    size_t lo=0, hi=v.size();
    while (lo<hi){
        size_t mid = (lo+hi)/2;
        if (v[mid].key == key){ out_off = v[mid].offset; return true; }
        if (v[mid].key < key) lo = mid+1; else hi = mid;
    }
    return false;
}

bool DiskIndex::lookup_email(const std::string& norm_email, uint64_t& out_offset) const{
    std::vector<DiskIndexEntry> email, phone;
    if (!load(email, phone)) return false;
    return bsearch_key(email, norm_email, out_offset);
}
bool DiskIndex::lookup_phone(const std::string& norm_phone, uint64_t& out_offset) const{
    std::vector<DiskIndexEntry> email, phone;
    if (!load(email, phone)) return false;
    return bsearch_key(phone, norm_phone, out_offset);
}

} // namespace acx
