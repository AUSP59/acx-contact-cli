// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "model.hpp"
#include <string>
namespace acx {
bool load_book_from_file(const std::string& fmt, const std::string& path, ContactBook& out, std::string& err);
std::string diff_books_json(const ContactBook& a, const ContactBook& b);
}
