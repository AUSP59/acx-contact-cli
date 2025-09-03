// SPDX-License-Identifier: Apache-2.0
#pragma once
#include <string>
#include <optional>
#include <map>

namespace acx {

struct Config {
    std::string data_file;
    std::string backup_dir;
    std::string audit_log;
    std::string lock_dir;
    std::string region;     // default country for phone normalization, e.g., "MX"
    std::string output;     // default output format: "human" or "json"
};

// Load configuration from a JSON file.
// Recognized keys: data_file, backup_dir, audit_log, lock_dir, region, output
std::optional<Config> load_config_file(const std::string& path);

// Load configuration, searching in order:
// 1) ACX_CONFIG env var (explicit path)
// 2) $XDG_CONFIG_HOME/acx/config.json
// 3) ~/.config/acx/config.json
std::optional<Config> load_config();

// Apply configuration to process environment so subsystems can read it.
void apply_config_env(const Config& cfg);

} // namespace acx
