#include "lib.h"

int main() {
    Rope::String str {"0123456789-9876543210"};
    assert(str.data().size() != 1, "tree should be partitioned across multiple roots");
    std::cout << "at: " << str.at(5) << std::endl;
    assert(str.at(5) == '5', "str.at(5) expected '5'");
    assert(str.at(sizeof("0123456789-9876543210") - 2) == '0', "last char expected '0'");

    assert(str[5] == '5', "str[5] expected '5'");
    std::cout << "front: " << str.front() << std::endl;
    assert(str.front() == '0', "str.front() expected '0'");
    assert(str.back() == '0', "str.back() expected '0'");

    assert(std::strcmp(str.c_str().get(), "0123456789-9876543210") == 0, "c_str content matches source literal");
}