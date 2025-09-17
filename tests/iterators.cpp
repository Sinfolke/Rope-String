#include "lib.h"

int main() {
    const char* cstr = "0123456789";
    Rope::String str(cstr);
    std::size_t count = 0;
    for (const auto c : str) {
        assert(c == cstr[count++], "c == cstr[count++]");
    }
    count = strlen(cstr) - 1; // start at last valid index
    for (auto rbegin = str.rbegin(); rbegin != str.rend(); ++rbegin) {
        std::cout << *rbegin << ", " << cstr[count] << std::endl;
        assert(*rbegin == cstr[count--], "*rbegin == cstr[count]");
    }
}