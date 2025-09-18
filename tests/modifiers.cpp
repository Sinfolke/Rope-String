#include "lib.h"

int main() {
    Rope::String s("hello, world");
    s.clear();
    assert(s.empty(), "s.empty()");
    s = "Hello, world";
    s.insert(7, "my dear ");
    std::cout << s.c_str() << std::endl;
    assert(s == "Hello, my dear world", "s == \"Hello, my dear world\"");
}