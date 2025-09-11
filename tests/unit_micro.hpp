#pragma once
#include <cstdio>
#include <functional>
#include <vector>
#include <string>
#include <exception>

namespace ut {
    inline std::vector<std::pair<std::string, std::function<void()>>>& R() {
        static std::vector<std::pair<std::string, std::function<void()>>> r; return r;
    }
    struct reg { reg(const char* n, std::function<void()> f){ R().emplace_back(n, std::move(f)); } };
    inline void run(){
        int pass=0;
        for (auto &t: R()){
            try { t.second(); std::printf("[PASS] %s\n", t.first.c_str()); ++pass; }
            catch (const std::exception& e){ std::printf("[FAIL] %s: %s\n", t.first.c_str(), e.what()); }
        }
        std::printf("Total: %zu\n", R().size());
        if (pass != (int)R().size()) std::exit(1);
    }
    inline void ok(bool v, const char* msg){ if (!v) throw std::runtime_error(msg); }
}
#define UT(name) static void name(); static ::ut::reg reg_##name(#name, name); static void name()
#define OK(x) ::ut::ok((x), "assertion failed: " #x)
