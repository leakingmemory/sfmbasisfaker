//
// Created by jeo on 12/7/23.
//

#include <fhir/value.h>

std::string FhirString::GetPropertyName() const {
    return PropertyName();
}

web::json::value FhirString::ToJson() const {
    return web::json::value::string(value);
}

std::shared_ptr<FhirString> FhirString::Parse(web::json::value value) {
    return std::make_shared<FhirString>(value.as_string());
}

std::shared_ptr<FhirValue> FhirValue::Parse(const std::string &propertyName, const web::json::value &property) {
    if (propertyName == FhirString::PropertyName()) {
        return FhirString::Parse(property);
    }
    if (propertyName == FhirCodeableConceptValue::PropertyName()) {
        return FhirCodeableConceptValue::Parse(property);
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

web::json::value FhirCodeableConcept::ToJson() const {
    auto obj = FhirObject::ToJson();
    auto jsonCoding = web::json::value::array();
    typeof(coding.size()) i = 0;
    for (const auto &c : coding) {
        jsonCoding[i++] = c.ToJson();
    }
    if (i > 0 || text.empty()) {
        obj["coding"] = jsonCoding;
    }
    if (!text.empty()) {
        obj["text"] = web::json::value::string(text);
    }
    return obj;
}

FhirCodeableConcept FhirCodeableConcept::Parse(const web::json::value &obj) {
    std::vector<FhirCoding> coding{};
    if (obj.has_array_field("coding")) {
        for (const auto &c : obj.at("coding").as_array()) {
            coding.emplace_back(FhirCoding::Parse(c));
        }
    }
    std::string text{};
    if (obj.has_string_field("text")) {
        text = obj.at("text").as_string();
    }
    return FhirCodeableConcept(std::move(coding), std::move(text));
}

std::string FhirCodeableConceptValue::GetPropertyName() const {
    return PropertyName();
}

web::json::value FhirCodeableConceptValue::ToJson() const {
    return FhirCodeableConcept::ToJson();
}

std::shared_ptr<FhirCodeableConceptValue> FhirCodeableConceptValue::Parse(const web::json::value &obj) {
    return std::make_shared<FhirCodeableConceptValue>(FhirCodeableConcept::Parse(obj));
}