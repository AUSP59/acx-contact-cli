#include "src/storage.hpp"
#include <iostream>
int main(){
    acx::Storage st; acx::ContactBook b; st.load(b);
    std::string id, err; acx::Contact c;
    c.name="Demo"; c.email="demo@ex.org"; c.phone="+525511110000";
    if (acx::add_contact(st, b, c, "embed", id, err)) std::cout << "OK " << id << "\n";
    else std::cout << "ERR " << err << "\n";
    return 0;
}
