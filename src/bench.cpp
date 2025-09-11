// SPDX-License-Identifier: Apache-2.0
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include "storage.hpp"
#include "jsonl.hpp"

using namespace std::chrono;

int main(){
    acx::Storage st;
    acx::ContactBook book;
    st.load(book);
    auto start = high_resolution_clock::now();
    const int N = 1000;
    std::string err, id;
    for (int i=0;i<N;++i){
        acx::Contact c;
        c.name = "User" + std::to_string(i);
        c.email = "u"+std::to_string(i)+"@ex.org";
        c.phone = "+5255"+std::to_string(1000000+i);
        c.note = "bench";
        if (!acx::add_contact(st, book, c, "bench", id, err)){
            std::cerr << "add failed: " << err << "\n"; return 2;
        }
    }
    auto mid = high_resolution_clock::now();
    int found=0;
    for (int i=0;i<N;++i){
        auto v = acx::search_contacts(book, "", "u"+std::to_string(i)+"@ex.org", "", "");
        found += (int)v.size();
    }
    auto end = high_resolution_clock::now();
    double add_ms = duration<double, std::milli>(mid-start).count();
    double search_ms = duration<double, std::milli>(end-mid).count();
    acx::J out(std::map<std::string,acx::J>{
        {"status","ok"},
        {"adds_ms", add_ms},
        {"search_ms", search_ms},
        {"adds_per_sec", (double)N / (add_ms/1000.0)},
        {"search_per_sec", (double)N / (search_ms/1000.0)},
        {"found", (double)found}
    });
    std::cout << out.dump() << "\n";
    return 0;
}
