// SPDX-License-Identifier: Apache-2.0
#include <cstdint>
#include <cstddef>
#include <string>
#include "src/csv.hpp"
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size){
    std::string s(reinterpret_cast<const char*>(Data), Size);
    try { auto rows = acx::parse_csv(s); (void)rows.size(); } catch(...) {}
    return 0;
}
