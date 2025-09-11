#pragma once
#include <string>
#include <filesystem>

namespace acx {

enum class SBOMFormat { SPDX, CycloneDX };

bool generate_sbom(const std::filesystem::path& root, SBOMFormat fmt, const std::filesystem::path& out_file, std::string& err);

} // namespace acx
