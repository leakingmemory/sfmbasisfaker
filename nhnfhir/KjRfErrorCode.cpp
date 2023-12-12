//
// Created by sigsegv on 12/10/23.
//

#include <nhnfhir/KjRfErrorCode.h>

constexpr std::vector<FhirCoding> Coding(std::string &&system, std::string &&code, std::string &&name) {
    std::vector<FhirCoding> codings{};
    codings.emplace_back(system, code, name);
    return codings;
}

FhirCodeableConcept KjRfErrorCode::Ok{Coding("http://ehelse.no/fhir/CodeSystem/sfm-kj-rf-error-code", "0", "OK")};
