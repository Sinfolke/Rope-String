#include "lib.h"

int main() {
    Rope::String s("hello world, hello rope");

    // find
    assert(s.find('h') == 0, "find char");
    assert(s.find("world") == 6, "find cstr");
    Rope::String needle("hello");
    assert(s.find(needle, 1) == 13, "find substring from pos");

    // rfind
    assert(s.rfind('o') == 20, "rfind char"); // 'o' in "rope"
    assert(s.rfind("hello") == 13, "rfind cstr");

    // find_first_of / not_of
    assert(s.find_first_of("aeiou") == 1, "find_first_of vowels at 'e'");
    assert(s.find_first_not_of("hel") == 4, "find_first_not_of skips h,e,l -> at 'o' index 4");

    // find_last_of / not_of
    assert(s.find_last_of("aeiou") == 22, "find_last_of vowel 'e' at end");
    assert(s.find_last_not_of(" ehlorwpd,") == Rope::String::npos, "all are in the set");

    // contains
    assert(s.contains('r'), "contains char r");
    assert(s.contains("rope"), "contains cstr rope");
}
