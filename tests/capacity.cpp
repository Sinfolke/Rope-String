#include "lib.h"

int main() {
    Rope::String s{"1234567890"};
    assert(s.size() == s.length(), "s.size() == s.length()");
    assert(s.size() == 10, "s.size() == 10");
}