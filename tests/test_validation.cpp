#include "unit_micro.hpp"
#include "src/validation.hpp"

UT(name_and_email){
    OK(acx::is_valid_name("Grace Hopper"));
    OK(!acx::is_valid_name("1"));
    OK(acx::is_valid_email("a@b.co"));
    OK(!acx::is_valid_email("bad@@co"));
}
int main(){ ut::run(); }
