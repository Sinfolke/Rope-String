#include "lib.h"
int main() {
    Rope::String str = "abc, 10";
    str.print();
    assert(str == "abc, 10", "str == abc, 10");
    str.assign("cba");
    assert(str == "cba", "str == cba");
}