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
    if (propertyName == FhirDateTimeValue::PropertyName()) {
        return FhirDateTimeValue::Parse(property);
    }
    if (propertyName == FhirBooleanValue::PropertyName()) {
        return FhirBooleanValue::Parse(property);
    }
    throw std::exception();
}

web::json::value FhirCoding::ToJson() const {
    auto obj = FhirObject::ToJson();
    if (!system.empty())
        obj["system"] = web::json::value::string(system);
    if (!code.empty())
        obj["code"] = web::json::value::string(code);
    if (!display.empty())
        obj["display"] = web::json::value::string(display);
    return obj;
}

FhirCoding FhirCoding::Parse(const web::json::value &obj) {
    std::string system{};
    if (obj.has_string_field("system")) {
        system = obj.at("system").as_string();
    }
    std::string code{};
    if (obj.has_string_field("code")) {
        code = obj.at("code").as_string();
    }
    std::string display{};
    if (obj.has_string_field("display")) {
        display = obj.at("display").as_string();
    }
    return {std::move(system), std::move(code), std::move(display)};
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

web::json::value FhirIdentifier::ToJson() const {
    auto obj = FhirObject::ToJson();
    obj["use"] = web::json::value::string(use);
    obj["type"] = type.ToJson();
    obj["value"] = web::json::value::string(value);
    return obj;
}

FhirIdentifier FhirIdentifier::Parse(const web::json::value &obj) {
    std::string use{};
    if (obj.has_string_field("use")) {
        use = obj.at("use").as_string();
    }

    FhirCodeableConcept type{};
    if (obj.has_object_field("type")) {
        type = FhirCodeableConcept::Parse(obj.at("type"));
    }

    std::string value{};
    if (obj.has_string_field("value")) {
        value = obj.at("value").as_string();
    }

    return {std::move(type), std::move(use), std::move(value)};
}

web::json::value FhirLink::ToJson() const {
    auto obj = FhirObject::ToJson();
    if (!relation.empty()) {
        obj["relation"] = web::json::value::string(relation);
    }
    if (!url.empty()) {
        obj["url"] = web::json::value::string(url);
    }
    return obj;
}

FhirLink FhirLink::Parse(const web::json::value &obj) {
    std::string relation{};
    if (obj.has_string_field("relation")) {
        relation = obj.at("relation").as_string();
    }

    std::string url{};
    if (obj.has_string_field("url")) {
        url = obj.at("url").as_string();
    }

    return {std::move(relation), std::move(url)};
}

web::json::value FhirName::ToJson() const {
    auto obj = FhirObject::ToJson();
    if (!use.empty()) {
        obj["use"] = web::json::value::string(use);
    }
    if (!family.empty()) {
        obj["family"] = web::json::value::string(family);
    }
    if (!given.empty()) {
        auto arr = web::json::value::array(1);
        arr[0] = web::json::value::string(given);
        obj["given"] = arr;
    }
    return obj;
}

FhirName FhirName::Parse(const web::json::value &obj) {
    FhirName name{};

    if (obj.has_string_field("use")) {
        name.use = obj.at("use").as_string();
    }
    if (obj.has_string_field("family")) {
        name.family = obj.at("family").as_string();
    }
    if (obj.has_array_field("given")) {
        name.given = obj.at("given").as_array().at(0).as_string();
    }

    return name;
}

web::json::value FhirAddress::ToJson() const {
    auto obj = FhirObject::ToJson();
    if (!type.empty()) {
        obj["type"] = web::json::value::string(type);
    }
    if (!city.empty()) {
        obj["city"] = web::json::value::string(city);
    }
    if (!postalCode.empty()) {
        obj["postalCode"] = web::json::value::string(postalCode);
    }
    if (!lines.empty()) {
        auto arr = web::json::value::array(lines.size());
        typeof(lines.size()) i = 0;
        for (const auto &line : lines) {
            arr[i++] = web::json::value::string(line);
        }
        obj["line"] = arr;
    }
    if (!use.empty()) {
        obj["use"] = web::json::value::string(use);
    }
    return obj;
}

FhirAddress FhirAddress::Parse(const web::json::value &obj) {
    std::string type{};
    if (obj.has_string_field("type")) {
        type = obj.at("type").as_string();
    }

    std::string city{};
    if (obj.has_string_field("city")) {
        city = obj.at("city").as_string();
    }

    std::string postalCode{};
    if (obj.has_string_field("postalCode")) {
        postalCode = obj.at("postalCode").as_string();
    }

    std::vector<std::string> lines{};
    if (obj.has_array_field("line")) {
        for (const auto &line : obj.at("line").as_array()) {
            lines.emplace_back(line.as_string());
        }
    }

    std::string use{};
    if (obj.has_string_field("use")) {
        use = obj.at("use").as_string();
    }

    return {std::move(lines), std::move(use), std::move(type), std::move(city), std::move(postalCode)};
}

std::string FhirDateTimeValue::PropertyName() {
    return "valueDateTime";
}

std::string FhirDateTimeValue::GetPropertyName() const {
    return PropertyName();
}

web::json::value FhirDateTimeValue::ToJson() const {
    return web::json::value::string(dateTime);
}

std::shared_ptr<FhirDateTimeValue> FhirDateTimeValue::Parse(const web::json::value &obj) {
    auto dateTimeValue = std::make_shared<FhirDateTimeValue>();
    dateTimeValue->dateTime = obj.as_string();
    return dateTimeValue;
}

std::string FhirBooleanValue::PropertyName() {
    return "valueBoolean";
}

std::string FhirBooleanValue::GetPropertyName() const {
    return PropertyName();
}

web::json::value FhirBooleanValue::ToJson() const {
    return web::json::value::boolean(value);
}

std::shared_ptr<FhirBooleanValue> FhirBooleanValue::Parse(const web::json::value &obj) {
    auto value = std::make_shared<FhirBooleanValue>();
    value->value = obj.as_bool();
    return value;
}
