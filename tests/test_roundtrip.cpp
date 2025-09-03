#include "unit_micro.hpp"
#include "src/storage.hpp"
#include <string>

UT(roundtrip){
    acx::Storage st;
    acx::ContactBook book;
    std::string err, id;
    // clean slate
    st.save(book);
    acx::Contact c; c.name="Alan Turing"; c.email="alan@ex.org"; c.phone="+441234567890"; c.note="legend";
    OK(acx::add_contact(st, book, c, "test", id, err));
    OK(!id.empty());
    acx::ContactBook loaded;
    OK(st.load(loaded));
    auto found = acx::search_contacts(loaded, "", "alan@ex.org", "", "");
    OK(found.size()==1);
}
int main(){ ut::run(); }
