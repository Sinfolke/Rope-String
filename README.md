# RopeString
A C++ Rope-String is a string with efficient insert/erase/replace in middle. The string is made following std::basic_string API.

## Key properties
- Non-contiguous storage (rope) consisting of leaf chunks connected by a balanced tree.
- Many std::basic_string-like constructors and member functions for familiarity.
- Forward and reverse iterators over characters.
- Efficient insert/erase/replace operations in the middle of a large string.
- Works with multiple character types (char, wchar_t, char8_t, char16_t, char32_t).

## Installation
You can use this library in three common ways.

1) add_subdirectory (recommended for vendoring)
- Copy or add this repository as a subdirectory.
- In your CMakeLists.txt:
  ```cmake
  add_subdirectory(external/RopeString)
  target_link_libraries(your_target PRIVATE Rope)
  target_include_directories(your_target PRIVATE external/RopeString/include)
  ```
2) FetchContent
  ```cmake
  include(FetchContent)
  FetchContent_Declare(
    RopeString
    GIT_REPOSITORY https://github.com/Sinfolke/Rope-String
    # GIT_TAG        <commit-or-tag>
  )
  FetchContent_MakeAvailable(RopeString)
  target_link_libraries(your_target PRIVATE Rope)
  ```
3) find_package (if you have installed it)
- Configure and install the library somewhere first:
  ```
  cmake -S . -B build
  cmake --build build --target install
  ```
- Then in your project:
  ```cmake
  find_package(Rope CONFIG REQUIRED)
  target_link_libraries(your_target PRIVATE Rope)
  ```
Include the header in your sources:
  ```C++
  #include <RopeString.h>
  ```
  Or import a C++20 module
  ```C++
  import Rope.String;
  ```

## Quick start
  ```C++
  #include <RopeString.h>
  #include <iostream>

  int main() {
      Rope::String s = "Hello";
      s.append(", World!");
      s.insert(5, 1, '!');
      s.erase(0, 1);
      auto sub = s.substr(1, 5);

      // Iteration (forward)
      for (auto ch : s) std::cout << ch;
      std::cout << "\n";

      // Obtain a contiguous C string view (allocates)
      auto c = s.c_str();         // unique_ptr<CharT[]>; zero-terminated copy
      std::cout << c.get() << "\n";
  }
  ```

## API overview and std::string compatibility
The API aims to be familiar to users of std::basic_string, but due to rope storage there are important differences. Below is a high-level map of what is available. For exact signatures please see include/BasicString.h.

### Constructors
```C++
BasicString();

BasicString(const Allocator &alloc);

BasicString(size_type count, CharT ch, const Allocator& alloc = Allocator());

template<typename InputIt>
BasicString(const InputIt first, InputIt last, Allocator alloc = Allocator());

#ifdef __cpp_lib_from_range
template<std::ranges::range R>
requires std::convertible_to<std::ranges::range_value_t<R>, CharT>
BasicString(std::from_range_t, R&& rg, const Allocator& alloc = Allocator());
#endif

BasicString( const CharT* s, size_type count, const Allocator& alloc = Allocator() );

BasicString( const CharT* s, const Allocator& alloc = Allocator() );

BasicString(std::nullptr_t) = delete;

template<typename StringViewLike>
explicit BasicString( const StringViewLike& t, const Allocator& alloc = Allocator() );

template<typename StringViewLike>
BasicString(const StringViewLike& t, size_type pos, size_type count, const Allocator& alloc = Allocator() );

BasicString( const BasicString& other );

BasicString(BasicString &&other) noexcept;

BasicString( const BasicString& other, const Allocator &alloc);

BasicString(BasicString &&other, const Allocator &alloc);

BasicString( const BasicString& other, size_type pos, const Allocator& alloc = Allocator() );

BasicString( const BasicString& other, size_type pos, size_type count, const Allocator& alloc = Allocator() );

BasicString(std::initializer_list<CharT> ilist, const Allocator& alloc = Allocator() );
```
- Default, copy, move, count+char, iterator range, from_range, C-string, string-view-like, initializer_list, substring constructors.
- detailed overview [here](https://en.cppreference.com/w/cpp/string/basic_string/basic_string.html)
### Assign
``` C++
// (1) assign from const basic_string&
auto assign(const BasicString& str) -> BasicString&;

// (2) assign from rvalue basic_string
auto assign(BasicString&& str) noexcept -> BasicString&;

// (3) assign count copies of a char
auto assign(size_type count, CharT ch) -> BasicString&;

// (4) assign from const CharT* with count
auto assign(const CharT* s, size_type count) -> BasicString&;

// (5) assign from const CharT* null-terminated
auto assign(const CharT* s) -> BasicString&;

// (6) assign from StringViewLike
template<typename SV>
auto assign(const SV& t) -> BasicString&;

// (7) assign from StringViewLike with pos/count
template<typename SV>
auto assign(const SV& t, size_type pos, size_type count = StringType::npos) -> BasicString&;

// (8) assign from basic_string with pos/count
auto assign(const BasicString& str, size_type pos, size_type count = StringType::npos) -> BasicString&;

// (9) assign from input iterators
template<typename InputIt>
auto assign(InputIt first, InputIt last) -> BasicString&;

// (10) assign from initializer_list
auto assign(std::initializer_list<CharT> ilist) -> BasicString&;
```
detailed see [here](https://en.cppreference.com/w/cpp/string/basic_string/assign.html)
### at
``` C++
// non-const
auto at(size_type pos) -> CharT;
// const
auto at(size_type pos) const -> const CharT;
```
### []
```C++
auto operator[](size_type pos) -> CharT;
auto operator[](size_type pos) const -> const CharT;
```
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
