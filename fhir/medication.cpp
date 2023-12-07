//
// Created by jeo on 12/7/23.
//

#include <fhir/medication.h>

FhirMedication FhirMedication::Parse(const web::json::value &obj) {
    FhirMedication medication{};
    if (!medication.ParseInline(obj)) {
        throw std::exception();
    }
    if (obj.has_object_field("code")) {
        medication.code = FhirCodeableConcept::Parse(obj.at("code"));
    }
    if (obj.has_object_field("form")) {
        medication.form = FhirCodeableConcept::Parse(obj.at("form"));
    }
    return medication;
}