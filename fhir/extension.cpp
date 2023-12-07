//
// Created by sigsegv on 12/7/23.
//

#include <fhir/extension.h>
#include <fhir/fhir.h>

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
