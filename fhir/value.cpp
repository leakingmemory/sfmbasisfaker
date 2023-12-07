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

web::json::value FhirQuantity::ToJson() const {
    auto obj = FhirObject::ToJson();
    obj["value"] = web::json::value::number(GetValue());
    obj["unit"] = web::json::value::string(unit);
    return obj;
}

FhirQuantity FhirQuantity::Parse(const web::json::value &obj) {
    std::string unit{};
    if (obj.has_string_field("unit")) {
        unit = obj.at("unit").as_string();
    }
    if (obj.has_number_field("value")) {
        return FhirQuantity(obj.at("value").as_double(), unit);
    } else if (!unit.empty()) {
        return FhirQuantity(0.0, unit);
    } else {
        return {};
    }
}

web::json::value FhirRatio::ToJson() const {
    auto obj = FhirObject::ToJson();
    if (numerator.IsSet()) {
        obj["numerator"] = numerator.ToJson();
    }
    if (denominator.IsSet()) {
        obj["denominator"] = denominator.ToJson();
    }
    return obj;
}

FhirRatio FhirRatio::Parse(const web::json::value &obj) {
    FhirQuantity numerator{};
    if (obj.has_object_field("numerator")) {
        numerator = FhirQuantity::Parse(obj.at("numerator"));
    }
    FhirQuantity denominator{};
    if (obj.has_object_field("denominator")) {
        denominator = FhirQuantity::Parse(obj.at("denominator"));
    }
    return {std::move(numerator), std::move(denominator)};
}

web::json::value FhirReference::ToJson() const {
    auto obj = FhirObject::ToJson();
    if (!reference.empty()) {
        obj["reference"] = web::json::value::string(reference);
    }
    if (!type.empty()) {
        obj["type"] = web::json::value::string(type);
    }
    if (!display.empty()) {
        obj["display"] = web::json::value::string(display);
    }
    return obj;
}

FhirReference FhirReference::Parse(const web::json::value &obj) {
    std::string reference{};
    if (obj.has_string_field("reference")) {
        reference = obj.at("reference").as_string();
    }

    std::string type{};
    if (obj.has_string_field("type")) {
        type = obj.at("type").as_string();
    }

    std::string display{};
    if (obj.has_string_field("display")) {
        display = obj.at("display").as_string();
    }

    return {std::move(reference), std::move(type), std::move(display)};
}
