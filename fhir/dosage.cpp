//
// Created by sigsegv on 12/8/23.
//

#include <fhir/dosage.h>

web::json::value FhirDosage::ToJson() const {
    auto obj = FhirExtendable::ToJson();
    obj["sequence"] = web::json::value::number(sequence);
    obj["text"] = web::json::value::string(text);
    return obj;
}

FhirDosage FhirDosage::Parse(const web::json::value &obj) {
    FhirDosage dosage{};
    if (!dosage.ParseInline(obj)) {
        throw std::exception();
    }
    if (obj.has_number_field("sequence")) {
        dosage.sequence = obj.at("sequence").as_number().to_int32();
    }
    if (obj.has_string_field("text")) {
        dosage.text = obj.at("text").as_string();
    }
    return dosage;
}