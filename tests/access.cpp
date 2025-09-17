#include "lib.h"

int main() {
    Rope::String str {"0123456789-9876543210"};
    assert(str.data().size() != 1, "incorrect parritioning");
    assert(str.at(5) == '5', "str.at(5) == '5'");
    assert(str.at(sizeof("0123456789-9876543210") - 2) == '0', "str.at(sizeof(\"0123456789-9876543210\")) == '0'");

    assert(str[5] == '5', "str[5] == '4'");

    assert(str.front() == '0', "str.front() == '0");
    assert(str.back() == '0', "str.back() == '0'");

    assert(std::strcmp(str.c_str().get(), "0123456789-9876543210") == 0, "std::strcmp(str.c_str(), \"0123456789-9876543210\") == 0");
}