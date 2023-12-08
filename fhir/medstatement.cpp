//
// Created by sigsegv on 12/8/23.
//

#include <fhir/medstatement.h>

web::json::value FhirMedicationStatement::ToJson() const {
    auto obj = Fhir::ToJson();
    if (medicationReference.IsSet()) {
        obj["medicationReference"] = medicationReference.ToJson();
    }
    if (subject.IsSet()) {
        obj["subject"] = subject.ToJson();
    }
    if (!identifiers.empty()) {
        auto arr = web::json::value::array(identifiers.size());
        typeof(identifiers.size()) i = 0;
        for (const auto &id : identifiers) {
            arr[i++] = id.ToJson();
        }
        obj["identifier"] = arr;
    }
    if (!dosage.empty()) {
        auto arr = web::json::value::array(dosage.size());
        typeof(dosage.size()) i = 0;
        for (const auto &d : dosage) {
            arr[i++] = d.ToJson();
        }
        obj["dosage"] = arr;
    }
    return obj;
}

FhirMedicationStatement FhirMedicationStatement::Parse(const web::json::value &obj) {
    FhirMedicationStatement medStatement{};

    if (!medStatement.ParseInline(obj)) {
        throw std::exception();
    }

    if (obj.has_object_field("medicationReference")) {
        medStatement.medicationReference = FhirReference::Parse(obj.at("medicationReference"));
    }
    if (obj.has_field("subject")) {
        medStatement.subject = FhirReference::Parse(obj.at("subject"));
    }
    if (obj.has_field("identifier")) {
        const auto arr = obj.at("identifier").as_array();
        for (const auto &id : arr) {
            medStatement.identifiers.push_back(FhirIdentifier::Parse(id));
        }
    }
    if (obj.has_field("dosage")) {
        auto arr = obj.at("dosage").as_array();
        for (const auto &d : arr) {
            medStatement.dosage.push_back(FhirDosage::Parse(d));
        }
    }

    return medStatement;
}