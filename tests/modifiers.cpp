#include "lib.h"

int main() {
    Rope::String s("hello, world");
    s.clear();
    assert(s.empty(), "s.empty()");

    // Prepare a string and insert a phrase
    s = "Hello, world";
    s.insert(7, "my dear ");
    // ensure insert works before testing erase
    assert(s == "Hello, my dear world", "s == \"Hello, my dear world\"");

    // Erase the inserted phrase "my dear " (8 chars)
    s.erase(7, 8);
    assert(s == "Hello, world", "erase middle restores original");

    // Erase from the beginning
    s.erase(0, 7); // remove "Hello, "
    assert(s == "world", "erase from beginning");

    // Erase from the end
    s.insert(0, "Hello, "); // back to "Hello, world"
    s.erase(s.size() - 5, 5); // remove "world"
    assert(s == "Hello, ", "erase from end");

    // Erase to end using npos
    s.erase(0); // remove everything
    assert(s.empty(), "erase to end");
    s.assign("Hello, World");
    s.push_back('!');
    assert(s == "Hello, World!", "push_back");
    s.pop_back();
    assert(s == "Hello, World", "pop_back");

    s.append("! This is my Rope String");
    assert(s == "Hello, World! This is my Rope String", "append");

    s.replace(23, 2, "your");
}