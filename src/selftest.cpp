#include "selftest.hpp"
#include "storage.hpp"
#include "merge.hpp"
#include "diff.hpp"
#include "policy.hpp"
#include "utils.hpp"
#include <sstream>

namespace acx {

std::string run_selftest_json(){
    Storage st;
    ContactBook book;
    st.load(book);
    std::ostringstream o;
    o<<"{";
    // 1) add/edit/delete roundtrip
    std::string id, err;
    Contact c; c.name="Test User"; c.email="test@example.com"; c.phone="+12025550123"; c.note="note";
    bool add_ok = st.add(book, c, id, err);
    bool edit_ok=false, del_ok=false;
    if (add_ok){
        edit_ok = st.edit(book, id, "Test User 2", "", "", "note2", err);
        del_ok = st.remove(book, id, "selftest", err);
    }
    o<<"\"roundtrip\":"<<(add_ok&&edit_ok&&del_ok?"true":"false")<<",";

    // 2) policy validate default (should pass on empty set)
    Policy pol = Policy::load_default();
    size_t bad=0; for (auto& x: book.contacts){ std::string why; if(!pol.validate_contact(x, why)) ++bad; }
    o<<"\"policy_bad\":"<<bad<<",";

    // 3) export/import determinism on small sample
    Contact d; d.name="Ada"; d.email="ada@example.com"; d.phone="+12025550111"; d.note="hi";
    std::string id2; st.add(book, d, id2, err); st.save(book);
    write_file("tmp_export.jsonl", d.to_json().dump()+"\n");
    ContactBook b2; Storage st2; std::string e2;
    st2.load(b2);
    st2.import_data(b2, "jsonl", "tmp_export.jsonl", false, e2);
    DiffResult dr; Storage st3; compute_diff(st3, b2, "jsonl", "tmp_export.jsonl", dr, e2);
    bool det_ok = dr.added.size()==0 && dr.changed.size()==0; // re-import shouldn't change
    o<<"\"determinism\":"<<(det_ok?"true":"false")<<",";
    // cleanup
    std::filesystem::remove("tmp_export.jsonl");
    o<<"\"status\":\"ok\"";
    o<<"}";
    return o.str();
}

std::string run_determinism_check_json(){
    Storage st;
    ContactBook b;
    st.load(b);
    // export current book to jsonl
    std::string path = "tmp_book.jsonl";
    std::ostringstream out;
    for (auto& c : b.contacts){ out<<c.to_json().dump()<<'\n'; }
    write_file(path, out.str());
    // reimport into empty and diff
    ContactBook b2; Storage st2; std::string err;
    st2.load(b2);
    st2.import_data(b2, "jsonl", path, false, err);
    DiffResult dr; Storage st3; compute_diff(st3, b2, "jsonl", path, dr, err);
    bool ok = dr.added.size()==0 && dr.changed.size()==0;
    std::ostringstream o;
    o<<"{\"ok\":"<<(ok?"true":"false")<<",\"added\":"<<dr.added.size()<<",\"changed\":"<<dr.changed.size()<<"}";
    std::filesystem::remove(path);
    return o.str();
}

} // namespace acx
