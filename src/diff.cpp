// SPDX-License-Identifier: Apache-2.0
#include "diff.hpp"
#include "utils.hpp"
#include "csv.hpp"
#include "jsonl.hpp"
#include <set>
#include <map>

namespace acx {
bool load_book_from_file(const std::string& fmt, const std::string& path, ContactBook& out, std::string& err){
    std::string data = read_file(path);
    if (data.empty()){ err="empty file"; return false; }
    if (fmt=="jsonl"){
        auto lines = split(data, '\n');
        for (auto &ln: lines){
            if (ln.empty()) continue;
            auto j = J::parse(ln);
            if (!j.is_object()) continue;
            Contact c;
            auto &o = j.as_object();
            if (o.count("id")) c.id = o["id"].as_string();
            if (o.count("name")) c.name = o["name"].as_string();
            if (o.count("email")) c.email = o["email"].as_string();
            if (o.count("phone")) c.phone = o["phone"].as_string();
            if (o.count("note")) c.note = o["note"].as_string();
            out.contacts.push_back(c);
        }
        return true;
    } else if (fmt=="csv" || fmt=="tsv"){
        char sep = (fmt=="csv") ? ',' : '\t';
        auto rows = parse_csv_with_sep(data, sep);
        bool header = true;
        for (auto &r : rows){
            if (header){ header=false; continue; }
            if (r.size()<6) continue;
            Contact c; c.id=r[0]; c.name=r[1]; c.email=r[2]; c.phone=r[3]; c.note=r[4];
            out.contacts.push_back(c);
        }
        return true;
    }
    err="unknown format"; return false;
}

static inline std::map<std::string, Contact> index(const ContactBook& b){
    std::map<std::string, Contact> m;
    for (auto &c : b.contacts) m[c.id] = c;
    return m;
}

std::string diff_books_json(const ContactBook& a, const ContactBook& b){
    auto A = index(a), B = index(b);
    std::set<std::string> ids;
    for (auto &kv : A) ids.insert(kv.first);
    for (auto &kv : B) ids.insert(kv.first);
    std::vector<J> added, removed, changed;
    for (auto &id : ids){
        bool inA = A.count(id), inB = B.count(id);
        if (inA && !inB){ removed.push_back(J(id)); }
        else if (!inA && inB){ added.push_back(J(id)); }
        else {
            const Contact& ca = A[id];
            const Contact& cb = B[id];
            if (ca.name!=cb.name || ca.email!=cb.email || ca.phone!=cb.phone || ca.note!=cb.note){
                std::map<std::string, J> ch;
                ch["id"]=id;
                if (ca.name!=cb.name){ ch["name"]=J::array({ca.name, cb.name}); }
                if (ca.email!=cb.email){ ch["email"]=J::array({ca.email, cb.email}); }
                if (ca.phone!=cb.phone){ ch["phone"]=J::array({ca.phone, cb.phone}); }
                if (ca.note!=cb.note){ ch["note"]=J::array({ca.note, cb.note}); }
                changed.push_back(J(ch));
            }
        }
    }
    std::map<std::string, J> out{
        {"added", J(added)},
        {"removed", J(removed)},
        {"changed", J(changed)}
    };
    return J(out).dump();
}
} // namespace acx
