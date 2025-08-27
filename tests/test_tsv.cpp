#include "tests/unit_micro.hpp"
#include "src/storage.hpp"
#include <string>

UT(tsv_roundtrip){
    acx::Storage st; acx::ContactBook b; st.load(b);
    std::string err, id;
    acx::Contact c; c.name="TSV User"; c.email="t@ex.org"; c.phone="+525511110000"; c.note="n";
    OK(acx::add_contact(st,b,c,"tsvtest",id,err));
    OK(acx::export_data(b, "tsv", "book.tsv", err));
    acx::ContactBook b2; // fresh
    OK(acx::import_data(st, b2, "tsv", "book.tsv", err));
}
int main(){ ut::run(); }
