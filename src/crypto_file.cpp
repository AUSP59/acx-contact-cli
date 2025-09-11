// SPDX-License-Identifier: Apache-2.0
#include "crypto_file.hpp"
#include "utils.hpp"
#include <vector>
#include <fstream>
#include <random>
#include <cstring>

#ifdef ACX_HAVE_OPENSSL
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#endif

namespace acx {

static inline std::vector<unsigned char> hex2bin(const std::string& hx){
    std::vector<unsigned char> out; out.reserve(hx.size()/2);
    for (size_t i=0;i+1<hx.size(); i+=2){
        unsigned int b=0; std::sscanf(hx.substr(i,2).c_str(), "%02x", &b);
        out.push_back((unsigned char)b);
    }
    return out;
}
static inline std::string bin2hex(const unsigned char* p, size_t n){
    static const char* dig="0123456789abcdef";
    std::string s; s.reserve(n*2);
    for (size_t i=0;i<n;++i){ unsigned char b=p[i]; s.push_back(dig[b>>4]); s.push_back(dig[b&0xF]); }
    return s;
}
static inline std::string getenv_str(const char* k){ const char* v = std::getenv(k); return v? std::string(v): std::string(); }

#ifdef ACX_HAVE_OPENSSL
static bool derive_key_pbkdf2(const std::string& pass, const unsigned char* salt, int saltlen, int iters, unsigned char* out32){
    return PKCS5_PBKDF2_HMAC(pass.c_str(), (int)pass.size(), salt, saltlen, iters, EVP_sha256(), 32, out32) == 1;
}
#endif

std::string random_key_hex(size_t bytes){
    std::vector<unsigned char> b(bytes);
#ifdef ACX_HAVE_OPENSSL
    RAND_bytes(b.data(), (int)b.size());
#else
    std::random_device rd; for (auto &x: b) x = (unsigned char)rd();
#endif
    return bin2hex(b.data(), b.size());
}

bool encrypt_file_aes256gcm(const std::string& in, const std::string& out, const std::string& key_hex, const std::string& aad, std::string& err){
#ifndef ACX_HAVE_OPENSSL
    (void)in;(void)out;(void)key_hex;(void)aad; err="OpenSSL not available"; return false;
#else
    auto key_env = getenv_str("ACX_ENC_KEY");
    auto keyfile = getenv_str("ACX_ENC_KEY_FILE");
    auto pass_env = getenv_str("ACX_ENC_PASSPHRASE");
    auto passfile = getenv_str("ACX_ENC_PASSPHRASE_FILE");
    std::string mode = "KEY"; // or PASS
    std::string keyhex_local = key_hex;
    if (keyhex_local.empty()){
        if (!key_env.empty()) keyhex_local = key_env;
        else if (!keyfile.empty()) keyhex_local = read_file(keyfile);
        else if (!pass_env.empty() || !passfile.empty()) mode = "PASS";
        else { err="Provide ACX_ENC_KEY(_FILE) or ACX_ENC_PASSPHRASE(_FILE)"; return false; }
    } else {
        mode = "KEY";
    }
    auto pt = read_file_bin(in);
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx){ err="EVP_CIPHER_CTX_new failed"; return false; }
    bool ok=false;
    unsigned char iv[12]; RAND_bytes(iv, sizeof(iv));
    int outlen=0; int total=0;
    std::vector<unsigned char> ct(pt.size()+16);
    unsigned char tag[16];
    if (mode=="KEY"){
        auto key = hex2bin(keyhex_local);
        if (key.size()!=32){ err="Key must be 32 bytes hex (64 chars)"; EVP_CIPHER_CTX_free(ctx); return false; }
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1){ err="EncryptInit"; }
        else if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), nullptr) != 1){ err="SetIVLen"; }
        else if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), iv) != 1){ err="SetKeyIV"; }
        else {
            if (!aad.empty()){ int tmplen=0; if (EVP_EncryptUpdate(ctx, nullptr, &tmplen, reinterpret_cast<const unsigned char*>(aad.data()), (int)aad.size()) != 1){ err="AAD"; } }
            if (EVP_EncryptUpdate(ctx, ct.data(), &outlen, reinterpret_cast<const unsigned char*>(pt.data()), (int)pt.size()) != 1){ err="EncryptUpdate"; }
            else { total=outlen; if (EVP_EncryptFinal_ex(ctx, ct.data()+total, &outlen) != 1){ err="EncryptFinal"; }
            else { total+=outlen; if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag) != 1){ err="GetTag"; } else {
                std::string outbuf; outbuf.reserve(6+12+16+total);
                outbuf.append("ACXEG1",6);
                outbuf.append(reinterpret_cast<const char*>(iv), sizeof(iv));
                outbuf.append(reinterpret_cast<const char*>(tag), sizeof(tag));
                outbuf.append(reinterpret_cast<const char*>(ct.data()), total);
                if (!write_file_bin(out, outbuf)){ err="write failed"; } else ok=true;
            } } }
        }
    } else { // PASS
        std::string pass = !pass_env.empty()? pass_env : read_file(passfile);
        unsigned char salt[16]; RAND_bytes(salt, sizeof(salt));
        int iters = 200000;
        unsigned char key[32];
        if (!derive_key_pbkdf2(pass, salt, sizeof(salt), iters, key)){ err="PBKDF2"; EVP_CIPHER_CTX_free(ctx); return false; }
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1){ err="EncryptInit"; }
        else if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(iv), nullptr) != 1){ err="SetIVLen"; }
        else if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv) != 1){ err="SetKeyIV"; }
        else {
            if (!aad.empty()){ int tmplen=0; if (EVP_EncryptUpdate(ctx, nullptr, &tmplen, reinterpret_cast<const unsigned char*>(aad.data()), (int)aad.size()) != 1){ err="AAD"; } }
            if (EVP_EncryptUpdate(ctx, ct.data(), &outlen, reinterpret_cast<const unsigned char*>(pt.data()), (int)pt.size()) != 1){ err="EncryptUpdate"; }
            else { total=outlen; if (EVP_EncryptFinal_ex(ctx, ct.data()+total, &outlen) != 1){ err="EncryptFinal"; }
            else { total+=outlen; if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(tag), tag) != 1){ err="GetTag"; } else {
                // ACXEG2: magic + salt(16) + iter(4, big endian) + IV(12) + TAG(16) + CT
                std::string outbuf; outbuf.reserve(6+16+4+12+16+total);
                outbuf.append("ACXEG2",6);
                outbuf.append(reinterpret_cast<const char*>(salt), sizeof(salt));
                unsigned char iterbe[4] = { (unsigned char)((iters>>24)&0xFF), (unsigned char)((iters>>16)&0xFF), (unsigned char)((iters>>8)&0xFF), (unsigned char)(iters&0xFF) };
                outbuf.append(reinterpret_cast<const char*>(iterbe), 4);
                outbuf.append(reinterpret_cast<const char*>(iv), sizeof(iv));
                outbuf.append(reinterpret_cast<const char*>(tag), sizeof(tag));
                outbuf.append(reinterpret_cast<const char*>(ct.data()), total);
                if (!write_file_bin(out, outbuf)){ err="write failed"; } else ok=true;
            } } }
        }
    }
    EVP_CIPHER_CTX_free(ctx);
    return ok;
#endif
}

bool decrypt_file_aes256gcm(const std::string& in, const std::string& out, const std::string& key_hex, std::string& err){
#ifndef ACX_HAVE_OPENSSL
    (void)in;(void)out;(void)key_hex; err="OpenSSL not available"; return false;
#else
    auto buf = read_file_bin(in);
    if (buf.size()<6+12+16){ err="file too small"; return false; }
    std::string magic(buf.data(), buf.data()+6);
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new(); if(!ctx){ err="EVP_CIPHER_CTX_new failed"; return false; }
    bool ok=false;
    if (magic=="ACXEG1"){
        auto keyhex_local = key_hex.empty()? getenv_str("ACX_ENC_KEY") : key_hex;
        if (keyhex_local.empty()){ auto kf = getenv_str("ACX_ENC_KEY_FILE"); if(!kf.empty()) keyhex_local = read_file(kf); }
        auto key = hex2bin(keyhex_local);
        if (key.size()!=32){ err="Key must be 32 bytes hex (64 chars)"; EVP_CIPHER_CTX_free(ctx); return false; }
        const unsigned char* iv = reinterpret_cast<const unsigned char*>(buf.data()+6);
        const unsigned char* tag = reinterpret_cast<const unsigned char*>(buf.data()+6+12);
        const unsigned char* ct  = reinterpret_cast<const unsigned char*>(buf.data()+6+12+16);
        size_t clen = buf.size()-6-12-16;
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1){ err="DecryptInit"; }
        else if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, nullptr) != 1){ err="SetIVLen"; }
        else if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), iv) != 1){ err="SetKeyIV"; }
        else {
            std::vector<unsigned char> pt(clen);
            int outlen=0;
            if (EVP_DecryptUpdate(ctx, pt.data(), &outlen, ct, (int)clen) != 1){ err="DecryptUpdate"; }
            else if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void*)tag) != 1){ err="SetTag"; }
            else {
                int fin=0;
                if (EVP_DecryptFinal_ex(ctx, pt.data()+outlen, &fin) != 1){ err="DecryptFinal (auth failed)"; }
                else {
                    size_t total = (size_t)(outlen+fin);
                    if (!write_file_bin(out, std::string(reinterpret_cast<const char*>(pt.data()), total))){ err="write failed"; }
                    else ok=true;
                }
            }
        }
    } else if (magic=="ACXEG2"){
        auto pass = getenv_str("ACX_ENC_PASSPHRASE");
        if (pass.empty()){ auto pf = getenv_str("ACX_ENC_PASSPHRASE_FILE"); if(!pf.empty()) pass = read_file(pf); }
        if (pass.empty()){ err="Set ACX_ENC_PASSPHRASE(_FILE) for ACXEG2"; EVP_CIPHER_CTX_free(ctx); return false; }
        const unsigned char* salt = reinterpret_cast<const unsigned char*>(buf.data()+6);
        const unsigned char* iterbe = reinterpret_cast<const unsigned char*>(buf.data()+6+16);
        int iters = (int)( (iterbe[0]<<24) | (iterbe[1]<<16) | (iterbe[2]<<8) | iterbe[3] );
        const unsigned char* iv = reinterpret_cast<const unsigned char*>(buf.data()+6+16+4);
        const unsigned char* tag = reinterpret_cast<const unsigned char*>(buf.data()+6+16+4+12);
        const unsigned char* ct  = reinterpret_cast<const unsigned char*>(buf.data()+6+16+4+12+16);
        size_t clen = buf.size()-6-16-4-12-16;
        unsigned char key[32];
        if (!derive_key_pbkdf2(pass, salt, 16, iters, key)){ err="PBKDF2"; EVP_CIPHER_CTX_free(ctx); return false; }
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1){ err="DecryptInit"; }
        else if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, nullptr) != 1){ err="SetIVLen"; }
        else if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key, iv) != 1){ err="SetKeyIV"; }
        else {
            std::vector<unsigned char> pt(clen);
            int outlen=0;
            if (EVP_DecryptUpdate(ctx, pt.data(), &outlen, ct, (int)clen) != 1){ err="DecryptUpdate"; }
            else if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void*)tag) != 1){ err="SetTag"; }
            else {
                int fin=0;
                if (EVP_DecryptFinal_ex(ctx, pt.data()+outlen, &fin) != 1){ err="DecryptFinal (auth failed)"; }
                else {
                    size_t total = (size_t)(outlen+fin);
                    if (!write_file_bin(out, std::string(reinterpret_cast<const char*>(pt.data()), total))){ err="write failed"; }
                    else ok=true;
                }
            }
        }
    } else { err="unknown magic"; }
    EVP_CIPHER_CTX_free(ctx);
    return ok;
#endif
}

} // namespace acx
