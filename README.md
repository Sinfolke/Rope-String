# RopeString
A C++ Rope-String is a string with efficient insert/erase/replace in middle. It provides following classes
- Rope::Node - the node of tree (Rope.Node module, Node.h header)
- Rope::Tree - the Tree with core integration API (Rope.Tree module, Tree.h header)
- Rope::String - The String class (Rope.String module, String.h header)

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
- Fetch content:
  ```cmake
  include(FetchContent)
  FetchContent_Declare(
    RopeString
    GIT_REPOSITORY https://github.com/Sinfolke/Rope-String
    # GIT_TAG        <commit-or-tag>
  )
  ```
- Make available:
  ```cmake
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
  Or import C++20 module
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

      // or print directly to buffer
      s.print(std::cout);
  }
  ```

## API overview and std::string compatibility
The API aims to be familiar to users of std::basic_string, but due to rope storage there are important differences. Below I've listed all methods with information you to note. For exact signatures please see include/BasicString.h.

### Access
  1. at()
  2. operator[]
  3. back()
  4. front()
  5. data()
  6. c_str()


  - data() returns hold Rope::Tree class
  - c_str() returns std::unique_ptr<CharT[]>, holding C like string on heap. Note that it is more expensive than std::string::c_str() as it requires concat entire string
### Iterators
  1. begin()
  2. end()
  3. cbegin()
  4. cend()
  5. rbegin()
  6. rend()
  7. rcbegin()
  8. rcend()

    
  - Iteration is linear but may step across leaves
  - iterator category is forward, not random-access.

### Capacity
  1. empty()
  2. size()
  3. length()
 
### Modifiers
  1. clear()
  2. insert()
  3. insert_range()
  4. erase()
  5. push_back()
  6. pop_back()
  7. append
  8. append_range
  9. opeartor+=
  10. replace
  11. replace_with_range
  12. copy
  13. resize
  14. resize_and_overwrite
  15. swap

### Search
  1. find()
  2. rfind()
  3. find_first_of()
  4. find_first_not_of()
  5. find_last_of()
  6 find_last_not_of()

### Operations
  1. compare()
  2. starts_with()
  3. ends_with()
  4. contains()
  5. substr()

## Building the tests (optional)
This repository includes small test executables in tests/ driven by CMake targets:
- access_test
- assignment_test
- capacity_test
- iterators_test
- modifiers_test
- operations_test
- search_test

Generic CMake usage:

  cmake -S . -B build
  cmake --build build --target iterators_test && ./build/iterators_test

In CLion, simply build and run the desired test target from the IDE.
