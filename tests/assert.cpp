//
// Created by jeo on 12/7/23.
//

#include "assert.h"
#include <iostream>
#include <concepts>

void AreEqual(const std::string &expected, const std::string &value) {
    AreEqual<const std::string &>(expected, value);
}

void AreEqual(size_t expected, size_t value) {
    AreEqual<size_t>(expected, value);
}

void FailOfDynamicType(const char *expType, const char *inType) {
    std::cerr << "Expected type " << expType << ", but got " << inType << std::endl;
    abort();
}