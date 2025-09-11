// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <string>
namespace acx {
std::string version_string();         // e.g. "2.0.0"
std::string build_time_utc();         // ISO 8601
std::string git_sha();                // short sha or empty
std::string runtime_summary();        // human summary
}
