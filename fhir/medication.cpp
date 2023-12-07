//
// Created by jeo on 12/7/23.
//

#include <fhir/medication.h>

FhirMedication FhirMedication::Parse(const web::json::value &obj) {
    FhirMedication medication{};
    if (!medication.ParseInline(obj)) {
        throw std::exception();
    }
    return medication;
}