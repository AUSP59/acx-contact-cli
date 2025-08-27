// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <string>
namespace acx {
// Returns true on success; err on failure. Requires ACX_HAVE_OPENSSL.
bool encrypt_file_aes256gcm(const std::string& in, const std::string& out, const std::string& key, const std::string& aad, std::string& err);
bool decrypt_file_aes256gcm(const std::string& in, const std::string& out, const std::string& key, std::string& err);
std::string random_key_hex(size_t bytes); // 32 bytes recommended
}
