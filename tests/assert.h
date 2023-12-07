//
// Created by jeo on 12/7/23.
//

#ifndef SFMBASISFAKER_ASSERT_H
#define SFMBASISFAKER_ASSERT_H

#include <string>
#include <functional>
#include <memory>
#include <iostream>

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

void AreEqual(const std::string &expected, const std::string &value);
void AreEqual(size_t expected, size_t value);
[[noreturn]] void FailOfDynamicType(const char *expType, const char *inType);
template <class T, class P> void OfDynamicType(const P &obj, const std::function<void (const T &)> &func) {
    const T *tobj = dynamic_cast<const T *>(&obj);
    if (tobj == nullptr) {
        FailOfDynamicType(typeid(T).name(), typeid(obj).name());
    }
    func(*tobj);
}
template <class T, class P> void OfDynamicType(std::shared_ptr<P> obj, const std::function<void (const T &)> &func) {
    OfDynamicType(*obj, func);
}

#endif //SFMBASISFAKER_ASSERT_H
