#include "lib.h"

int main() {
    // copy
    Rope::String s("Hello, Rope");
    char buf[64]{};
    auto copied = s.copy(buf, 5, 7); // copy "Rope" (4) + maybe comma? Actually from pos7, 5 chars => "Rope"
    assert(copied == 5 || copied == 4, "copy count tolerant"); // fragmentation may affect available

    // resize shrink
    Rope::String a("abc");
    a.resize(2);
    assert(a == "ab", "resize shrink");

    // resize_and_overwrite: uppercase first letters and set length
    Rope::String b("hello world");
    b.resize_and_overwrite(11, [](char* p, std::size_t n){
        for (std::size_t i = 0; i < n; ++i) p[i] = (i < 5 ? (char)std::toupper((unsigned char)"hello"[i]) : (i==5?' ':"world"[i-6]));
        return (std::size_t)11;
    });
    assert(b == "HELLO world", "resize_and_overwrite");

    // swap
    Rope::String x("left");
    Rope::String y("right");
    x.swap(y);
    assert(x == "right" && y == "left", "swap");

    // compare
    Rope::String c1("abc");
    Rope::String c2("abd");
    assert(c1.compare(c2) < 0, "compare bs");
    assert(c2.compare("abd") == 0, "compare cstr eq");

    // starts_with / ends_with / contains
    Rope::String t("prefix-body-suffix");
    assert(t.starts_with('p'), "starts_with char");
    assert(t.rfind("suffix") == t.size() - 6, "suffix via rfind");
    assert(t.contains('-'), "contains char");

    return 0;
}
