// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <string>
#include <map>
namespace acx {
// Append a JSONL audit event if ACX_AUDIT_LOG is set (path to file).
void audit_log(const std::string& action, const std::map<std::string,std::string>& fields);
}
