#pragma once
#include <string>
#include <filesystem>
#include "model.hpp"
#include "storage.hpp"

namespace acx {

enum class MergeStrategy { Skip, PreferNewer, PreferExisting };
bool merge_from_file(Storage& st, ContactBook& book, const std::string& fmt, const std::filesystem::path& in_file, MergeStrategy strat, std::string& err);

}
