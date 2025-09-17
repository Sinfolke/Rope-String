#ifndef ROPE_LIB_H
#define ROPE_LIB_H

#define ROPE_STRING_MAX_ROOT_SIZE 5 // fragmentation between root nodes in Tree
#define ROPE_STRING_MAX_LEAF_SIZE 2 // fragmentation between string value in node

#include <iostream>
#include <RopeString.h>
const void assert(bool cond, const char* message) {
    if (!cond) {
        throw std::runtime_error(std::string("assertion failed: ") + message);
    }
}

#endif //ROPE_LIB_H