# RopeString

A header-only C++ rope-based string library that provides a std::string-like API but with a tree-backed storage (rope) optimized for efficient insert/erase/concat in the middle of large strings. It is useful when you need to edit long texts frequently (e.g., editors, parsers) without paying for costly reallocations and memmoves typical for contiguous strings.

This project exposes templated Rope::BasicString<CharT> and convenient aliases:
- Rope::String   = BasicString<char>
- Rope::WString  = BasicString<wchar_t>
- Rope::U8String = BasicString<char8_t>
- Rope::U16String= BasicString<char16_t>
- Rope::U32String= BasicString<char32_t>

See include/RopeString.h for the aliases and include/BasicString.h for the full implementation.


## Key properties
- Non-contiguous storage (rope) consisting of leaf chunks connected by a balanced tree.
- Many std::basic_string-like constructors and member functions for familiarity.
- Forward iterators over characters; also reverse iteration is supported.
- Efficient insert/erase/replace operations in the middle of a large string.
- Works with multiple character types (char, wchar_t, char8_t, char16_t, char32_t).


## Installation
You can use this library in three common ways.

1) add_subdirectory (recommended for vendoring)
- Copy or add this repository as a subdirectory.
- In your CMakeLists.txt:

  add_subdirectory(external/RopeString)
  target_link_libraries(your_target PRIVATE Rope)
  target_include_directories(your_target PRIVATE external/RopeString/include)

2) FetchContent

  include(FetchContent)
  FetchContent_Declare(
    RopeString
    GIT_REPOSITORY https://github.com/your-org-or-user/RopeString.git
    GIT_TAG        <commit-or-tag>
  )
  FetchContent_MakeAvailable(RopeString)
  target_link_libraries(your_target PRIVATE Rope)

3) find_package (if you have installed it)
- Configure and install the library somewhere first:

  cmake -S . -B build
  cmake --build build --target install

- Then in your project:

  find_package(Rope CONFIG REQUIRED)
  target_link_libraries(your_target PRIVATE Rope)

Include the header in your sources:

  #include <RopeString.h>
  using Rope::String;


## Quick start

  #include <RopeString.h>
  #include <iostream>

  int main() {
      Rope::String s = "hello";
      s.append(", world");
      s.insert(5, 1, '!');        // efficient insert near the middle
      s.erase(0, 1);              // remove first char
      auto sub = s.substr(1, 5);  // take substring efficiently

      // Iteration (forward)
      for (auto ch : s) std::cout << ch;
      std::cout << "\n";

      // Obtain a contiguous C string view (allocates)
      auto c = s.c_str();         // unique_ptr<CharT[]>; zero-terminated copy
      std::cout << c.get() << "\n";
  }


## API overview and std::string compatibility
The API aims to be familiar to users of std::basic_string, but due to rope storage there are important differences. Below is a high-level map of what is available. For exact signatures please see include/BasicString.h.

Constructors (provided)
- Default, copy, move, count+char, iterator range, from_range, C-string, string-view-like, initializer_list, substring constructors.

Assignment/append/insert/replace/erase (provided)
- assign(...), operator=(...), append(...), push_back/pop_back, insert(...), erase(...), replace(...), swap, clear.

Element access (differences!)
- at(pos), operator[](pos) return CharT by value (not reference). This means you cannot modify characters through operator[] or at like in std::string.
- front()/back() return CharT& allowing modification of first/last character.
- data(): returns internal tree type (non-contiguous). Use c_str() to get a contiguous buffer copy.
- c_str(): returns std::unique_ptr<CharT[]> holding a null-terminated copy. This allocates and copies.

Iterators
- begin/end, cbegin/cend return forward iterators (std::forward_iterator_tag). Random access iterator semantics are not provided.
- rbegin/rend, crbegin/crend for reverse forward iteration.

Capacity
- size(), length(), empty() are implemented.
- capacity(), reserve(), shrink_to_fit() are not meaningful for rope and are not provided.

Substrings and comparisons
- substr(pos, count) -> BasicString
- compare(...) with BasicString, std::basic_string and C strings
- starts_with/ends_with/contains

Search
- find/rfind/find_first_of/find_first_not_of/find_last_of/find_last_not_of for BasicString, std::basic_string, const CharT* and single CharT as appropriate.

Copying out
- copy(dest, count, pos) to copy into a user buffer.

Equality
- operator== is provided for comparisons with BasicString, std::basic_string and C strings; comparing with nullptr is defined to be false.

Other notes
- resize(...), resize_and_overwrite(...) are available. resize may allocate/mutate tree chunks as needed; there is no capacity reservation.


## Design and performance notes
- Operations that insert/erase/replace in the middle are generally more efficient than with contiguous strings because the rope avoids moving the entire tail.
- Iteration is linear but may step across leaves; iterator category is forward, not random-access.
- c_str() creates a contiguous snapshot; avoid calling it in hot paths repeatedly.


## Building the tests (optional)
This repository includes small test executables in tests/ driven by CMake targets:
- access_test, assignment_test, capacity_test, iterators_test, modifiers_test, operations_test, search_test

Generic CMake usage:

  cmake -S . -B build
  cmake --build build --target iterators_test && ./build/iterators_test

In CLion, simply build and run the desired test target from the IDE.


## Limitations and differences from std::string
- Storage is not contiguous; data() does not return a pointer to characters. Use c_str() to obtain a contiguous copy.
- operator[] and at() return by value, not by reference. Modify characters via algorithms that replace ranges, via push_back/pop_back for ends, or via front()/back() for first/last.
- No capacity(), reserve(), shrink_to_fit().
- Iterator category is forward, not random-access; algorithms requiring random-access iterators will not compile.
- Certain operators present on std::string (e.g., operator+= with various overloads) may be missing; prefer append/insert/replace APIs.


## License
This project’s licensing terms were not specified in this repository snapshot. Add a LICENSE file to clarify distribution terms if you intend to share or publish this library.
