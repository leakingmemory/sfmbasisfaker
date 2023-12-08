//
// Created by jeo on 08.12.2023.
//

#include <fhir/organization.h>

web::json::value FhirOrganization::ToJson() const {
    auto obj = Fhir::ToJson();
    if (!identifiers.empty()) {
        auto arr = web::json::value::array(identifiers.size());
        typeof(identifiers.size()) i = 0;
        for (const auto &identifier : identifiers) {
            arr[i++] = identifier.ToJson();
        }
        obj["identifier"] = arr;
    }
    return obj;
}

FhirOrganization FhirOrganization::Parse(const web::json::value &obj) {
    FhirOrganization org{};
    if (!org.ParseInline(obj)) {
        throw std::exception();
    }

    if (obj.has_array_field("identifier")){
        auto arr = obj.at("identifier").as_array();
        for (const auto &identifier : arr) {
            org.identifiers.push_back(FhirIdentifier::Parse(identifier));
        }
    }
    return org;
}