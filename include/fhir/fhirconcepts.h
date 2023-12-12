//
// Created by sigsegv on 12/12/23.
//

#ifndef SFMBASISFAKER_FHIRCONCEPTS_H
#define SFMBASISFAKER_FHIRCONCEPTS_H

#include <concepts>
#include <type_traits>

class Fhir;

template <class T> concept FhirSubclass = requires (T a) {
    { a } -> std::convertible_to<const Fhir &>;
};

#endif //SFMBASISFAKER_FHIRCONCEPTS_H
