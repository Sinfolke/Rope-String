#ifndef ISPA_ROPESTRING_H
#define ISPA_ROPESTRING_H
#include <Node.h>
#include <Tree.h>
#include <BasicString.h>
#include <cstdlib>

namespace Rope {
    using String   = BasicString<char>;     // Standard 8-bit string
    using WString  = BasicString<wchar_t>;  // Platform-wide string
    using U8String = BasicString<char8_t>;  // UTF-8 (C++20+)
    using U16String= BasicString<char16_t>; // UTF-16
    using U32String= BasicString<char32_t>; // UTF-32
}


#endif //ISPA_ROPESTRING_H