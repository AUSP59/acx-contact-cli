// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <string>
namespace acx {
struct Lock {
    bool ok=false;
    std::string path;
    Lock() = default;
    explicit Lock(const std::string& p);
    ~Lock();
};
}
