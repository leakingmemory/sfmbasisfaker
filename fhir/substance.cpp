//
// Created by jeo on 08.12.2023.
//

#include <fhir/substance.h>

web::json::value FhirSubstance::ToJson() const {
    auto obj = Fhir::ToJson();
    if (!identifiers.empty()) {
        auto arr = web::json::value::array(identifiers.size());
        typeof(identifiers.size()) i = 0;
        for (const auto &identifier : identifiers) {
            arr[i++] = identifier.ToJson();
        }
        obj["identifier"] = arr;
    }
    if (code.IsSet()) {
        obj["code"] = code.ToJson();
    }
    return obj;
}

FhirSubstance FhirSubstance::Parse(const web::json::value &obj) {
    FhirSubstance substance{};
    if (!substance.ParseInline(obj)) {
        throw std::exception();
    }
    if (obj.has_field("identifier")) {
        auto arr = obj.at("identifier").as_array();
        for (const auto &val : arr) {
            substance.identifiers.emplace_back(FhirIdentifier::Parse(val));
        }
    }
    if (obj.has_field("code")) {
        substance.code = FhirCodeableConcept::Parse(obj.at("code"));
    }
    return substance;
}