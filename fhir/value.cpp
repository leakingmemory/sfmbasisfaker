//
// Created by jeo on 12/7/23.
//

#include <fhir/value.h>

std::string FhirString::GetPropertyName() {
    return "valueString";
}

web::json::value FhirString::ToJson() const {
    return web::json::value::string(value);
}

std::shared_ptr<FhirString> FhirString::Parse(web::json::value value) {
    return std::make_shared<FhirString>(value.as_string());
}

std::shared_ptr<FhirValue> FhirValue::Parse(const web::json::value &propertyName, const web::json::value &property) {
    auto str = propertyName.as_string();
    if (str == "valueString") {
        return FhirString::Parse(property);
    }
    throw std::exception();
}

web::json::value FhirCoding::ToJson() const {
    auto obj = FhirObject::ToJson();
    obj["system"] = web::json::value::string(system);
    obj["code"] = web::json::value::string(code);
    obj["display"] = web::json::value::string(display);
    return obj;
}

FhirCoding FhirCoding::Parse(const web::json::value &obj) {
    return FhirCoding(
            obj.at("system").as_string(),
            obj.at("code").as_string(),
            obj.at("display").as_string()
            );
}