#ifndef ROPE_LIB_H
#define ROPE_LIB_H

#define ROPE_STRING_MAX_ROOT_SIZE 6 // fragmentation between root nodes in Tree
#define ROPE_STRING_MAX_LEAF_SIZE 2 // fragmentation between string value in node

#include <iostream>
#include <string>
#include <source_location>
#include <RopeString.h>
inline void assert(bool cond, const char* message, const std::source_location& loc = std::source_location::current()) {
    if (!cond) {
        std::string where = std::string(loc.file_name()) + ":" + std::to_string((int)loc.line());
        throw std::runtime_error(std::string("assertion failed at ") + where + ": " + message);
    }
}

#endif //ROPE_LIB_H