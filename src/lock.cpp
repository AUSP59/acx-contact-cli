// SPDX-License-Identifier: Apache-2.0
#include "lock.hpp"
#include <filesystem>
namespace fs = std::filesystem;
namespace acx {
Lock::Lock(const std::string& p): path(p){
    std::error_code ec;
    ok = fs::create_directory(path, ec) && !ec;
    // If already exists, ok=false
}
Lock::~Lock(){
    if (ok){
        std::error_code ec;
        fs::remove_all(path, ec);
    }
}
}
