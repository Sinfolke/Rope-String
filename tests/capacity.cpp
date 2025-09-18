#include "lib.h"

int main() {
    Rope::String s{"1234567890"};
    assert(s.size() == 10, "s.size() == 10");
    assert(s.length() == 10, "s.size() == 10");
    assert(s.length() == 10, "s.size() == 10");
}