//
// Created by jeo on 12/7/23.
//

#include "assert.h"
#include <iostream>

void AreEqual(const std::string &expected, const std::string &value) {
    if (expected != value) {
        std::cerr << "Assertion failed" << std::endl;
        std::cerr << "Extected: " << expected << "\nGot: " << value << "\n";
        abort();
    }
}

void FailOfDynamicType(const char *expType, const char *inType) {
    std::cerr << "Expected type " << expType << ", but got " << inType << std::endl;
    abort();
}