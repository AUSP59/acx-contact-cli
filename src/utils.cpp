#include "utils.hpp"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <random>
#include <cstdio>
#include <array>

namespace fs = std::filesystem;

namespace acx {

std::string read_file(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

bool write_file_atomic(const fs::path& p, const std::string& content) {
    fs::path tmp = p;
    tmp += ".tmp";
    {
        std::ofstream out(tmp, std::ios::binary);
        if (!out) return false;
        out.write(content.data(), static_cast<std::streamsize>(content.size()));
        if (!out) return false;
    }
    std::error_code ec;
    fs::rename(tmp, p, ec);
    if (ec) {
        // fallback: copy + remove
        if (!copy_file_to(tmp, p)) return false;
        fs::remove(tmp, ec);
    }
    return true;
}

bool copy_file_to(const fs::path& src, const fs::path& dst) {
    std::ifstream in(src, std::ios::binary);
    std::ofstream out(dst, std::ios::binary);
    if (!in || !out) return false;
    out << in.rdbuf();
    return static_cast<bool>(out);
}

std::string now_iso8601_utc() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *gmtime(&tt);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return std::string(buf);
}

std::string uuid_v4() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    auto r = [&](){ return static_cast<uint64_t>(gen()); };
    uint64_t a=r(), b=r();
    // Set version and variant bits
    a = (a & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    b = (b & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
    std::ostringstream o;
    o << std::hex << std::nouppercase;
    o << ((a>>48)&0xFFFF) << ((a>>32)&0xFFFF) << "-"
      << ((a>>16)&0x0FFF) << "-" << ((a)&0xFFFF) << "-"
      << ((b>>48)&0xFFFF) << "-" << (b & 0xFFFFFFFFFFFFULL);
    return o.str();
}

// Minimal SHA-256 (simple, not optimized)
static inline uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
std::string sha256_hex(const std::string& data) {
    static const uint32_t k[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };
    std::vector<uint8_t> msg(data.begin(), data.end());
    uint64_t bitlen = msg.size()*8;
    msg.push_back(0x80);
    while ((msg.size() % 64) != 56) msg.push_back(0);
    for (int i=7;i>=0;--i) msg.push_back((bitlen>>(i*8))&0xFF);
    uint32_t h0=0x6a09e667,h1=0xbb67ae85,h2=0x3c6ef372,h3=0xa54ff53a,
             h4=0x510e527f,h5=0x9b05688c,h6=0x1f83d9ab,h7=0x5be0cd19;
    for (size_t off=0; off<msg.size(); off+=64) {
        uint32_t w[64];
        for (int i=0;i<16;++i) {
            w[i] = (msg[off+i*4]<<24)|(msg[off+i*4+1]<<16)|(msg[off+i*4+2]<<8)|(msg[off+i*4+3]);
        }
        for (int i=16;i<64;++i) {
            uint32_t s0 = rotr(w[i-15],7) ^ rotr(w[i-15],18) ^ (w[i-15]>>3);
            uint32_t s1 = rotr(w[i-2],17) ^ rotr(w[i-2],19) ^ (w[i-2]>>10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }
        uint32_t a=h0,b=h1,c=h2,d=h3,e=h4,f=h5,g=h6,h=h7;
        for (int i=0;i<64;++i) {
            uint32_t S1=rotr(e,6)^rotr(e,11)^rotr(e,25);
            uint32_t ch=(e&f)^((~e)&g);
            uint32_t temp1=h+S1+ch+k[i]+w[i];
            uint32_t S0=rotr(a,2)^rotr(a,13)^rotr(a,22);
            uint32_t maj=(a&b)^(a&c)^(b&c);
            uint32_t temp2=S0+maj;
            h=g; g=f; f=e; e=d+temp1; d=c; c=b; b=a; a=temp1+temp2;
        }
        h0+=a; h1+=b; h2+=c; h3+=d; h4+=e; h5+=f; h6+=g; h7+=h;
    }
    std::ostringstream o;
    o<<std::hex<<std::setfill('0');
    auto pr=[&](uint32_t v){o<<std::setw(8)<<v;};
    pr(h0);pr(h1);pr(h2);pr(h3);pr(h4);pr(h5);pr(h6);pr(h7);
    return o.str();
}

std::vector<std::string> split(const std::string& s, char d) {
    std::vector<std::string> out; std::string cur;
    for(char c: s){ if(c==d){ out.push_back(cur); cur.clear(); } else cur.push_back(c); }
    out.push_back(cur); return out;
}

std::string trim(const std::string& s) {
    size_t a=0,b=s.size();
    while (a<b && isspace((unsigned char)s[a])) ++a;
    while (b>a && isspace((unsigned char)s[b-1])) --b;
    return s.substr(a,b-a);
}

bool file_exists_nonempty(const fs::path& p) {
    return fs::exists(p) && fs::file_size(p) > 0;
}

} // namespace acx

#include <filesystem>
#include <algorithm>
namespace fs = std::filesystem;
namespace acx {

std::vector<fs::path> list_files_recursive(const fs::path& root){
    std::vector<fs::path> out;
    for (auto it = fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied);
         it != fs::recursive_directory_iterator(); ++it){
        if (it->is_regular_file()){
            auto p = it->path();
            auto name = p.filename().string();
            if (name == ".DS_Store" || name == "Thumbs.db") continue;
            if (p.string().find("/.git/") != std::string::npos) continue;
            out.push_back(p);
        }
    }
    std::sort(out.begin(), out.end());
    return out;
}

// File SHA-256 using existing hex of string by reading file
std::string sha256_file_hex(const std::string& path){
    std::ifstream in(path, std::ios::binary);
    if(!in) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return sha256_hex(ss.str());
}

} // namespace acx

#include <cstdlib>
#ifdef __unix__
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace acx {

int env_int(const char* name, int def){
    const char* v = std::getenv(name);
    if (!v) return def;
    try { return std::stoi(std::string(v)); } catch (...) { return def; }
}
size_t env_size(const char* name, size_t def){
    const char* v = std::getenv(name);
    if (!v) return def;
    try { return (size_t)std::stoull(std::string(v)); } catch (...) { return def; }
}

std::string read_file_mmap(const fs::path& p){
#ifdef __unix__
    int fd = ::open(p.c_str(), O_RDONLY);
    if (fd < 0) return read_file(p);
    struct stat st;
    if (fstat(fd, &st) != 0) { ::close(fd); return read_file(p); }
    if (st.st_size == 0) { ::close(fd); return std::string(); }
    void* addr = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) { ::close(fd); return read_file(p); }
    std::string out((const char*)addr, (size_t)st.st_size);
    munmap(addr, st.st_size);
    ::close(fd);
    return out;
#else
    return read_file(p);
#endif
}

} // namespace acx
