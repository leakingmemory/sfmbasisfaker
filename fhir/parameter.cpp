//
// Created by jeo on 08.12.2023.
//

#include <fhir/parameter.h>
#include <fhir/fhir.h>

web::json::value FhirParameter::ToJson() const {
    auto obj = FhirObject::ToJson();
    obj["name"] = web::json::value::string(name);
    obj["resource"] = resource ? resource->ToJson() : web::json::value::object();
    return obj;
}

FhirParameter FhirParameter::Parse(const web::json::value &obj) {
    FhirParameter parameter{};
    parameter.name = obj.at("name").as_string();
    parameter.resource = Fhir::Parse(obj.at("resource"));
    return parameter;
}