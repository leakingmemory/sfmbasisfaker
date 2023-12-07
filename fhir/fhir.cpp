//
// Created by jeo on 12/7/23.
//

#include <fhir/fhir.h>

web::json::value FhirObject::ToJson() const {
    return web::json::value::object();
}

web::json::value FhirExtension::ToJson() const {
    auto obj = FhirObject::ToJson();
    obj["url"] = web::json::value::string(url);
    return obj;
}

std::shared_ptr<FhirExtension> FhirExtension::Parse(const web::json::value &obj) {
    std::string url{};
    if (obj.has_string_field("url")) {
        url = obj.at("url").as_string();
        bool generic{false};
        std::string valueProperty{};
        web::json::value value{};
        for (const auto &child : obj.as_object()) {
            if (child.first == "url") {
                continue;
            }
            if (!valueProperty.empty() || !child.first.starts_with("value")) {
                generic = true;
                break;
            }
            valueProperty = child.first;
            value = child.second;
        }
        if (!generic && !valueProperty.empty()) {
            return std::make_shared<FhirValueExtension>(url, FhirValue::Parse(valueProperty, value));
        }
    }
    return std::make_shared<FhirGenericExtension>(obj);
}

web::json::value FhirValueExtension::ToJson() const {
    auto obj = FhirExtension::ToJson();
    if (value) {
        obj[value->GetPropertyName()] = value->ToJson();
    }
    return obj;
}

FhirGenericExtension::FhirGenericExtension(const web::json::value &json) : FhirExtension(), json(json) {
    if (json.has_string_field("url")) {
        SetUrl(json.at("url").as_string());
    }
}

web::json::value FhirGenericExtension::ToJson() const {
    return json;
}
