//
// Created by jeo on 12/7/23.
//

#include "assert.h"
#include <iostream>
#include <concepts>

template <typename T> concept EqualityAssertable = requires (T a) {
    { a != a } -> std::convertible_to<bool>;
    { std::cerr << a };
};

template <EqualityAssertable T> void AreEqual(T expected, T value) {
    if (expected != value) {
        std::cerr << "Assertion failed" << std::endl;
        std::cerr << "Extected: " << expected << "\nGot: " << value << "\n";
        abort();
    }
}

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