//
// Created by jeo on 08.12.2023.
//

#include <fhir/parameters.h>

web::json::value FhirParameters::ToJson() const {
    auto obj = Fhir::ToJson();
    auto arr = web::json::value::array(parameters.size());
    typeof(parameters.size()) i = 0;
    for (const auto &parameter : parameters) {
        arr[i++] = parameter.ToJson();
    }
    obj["parameter"] = arr;
    return obj;
}

FhirParameters FhirParameters::Parse(const web::json::value &obj) {
    FhirParameters parameters{};
    if (!parameters.ParseInline(obj)) {
        throw std::exception();
    }
    if (obj.has_array_field("parameter")) {
        for (const auto &parameter : obj.at("parameter").as_array()) {
            parameters.parameters.emplace_back(FhirParameter::Parse(parameter));
        }
    }
    return parameters;
}