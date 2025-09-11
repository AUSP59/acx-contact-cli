// SPDX-License-Identifier: Apache-2.0
#include <cstdint>
#include <cstddef>
#include <string>
#include "src/jsonl.hpp"
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size){
    std::string s(reinterpret_cast<const char*>(Data), Size);
    try { (void)acx::J::parse(s); } catch(...) {}
    return 0;
}
