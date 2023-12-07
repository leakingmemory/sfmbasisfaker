//
// Created by sigsegv on 12/7/23.
//

#include <fhir/fhirextendable.h>
#include <fhir/extension.h>

bool FhirExtendable::ParseInline(const web::json::value &json) {
    if (json.has_array_field("extension")) {
        for (const auto &ext : json.at("extension").as_array()) {
            extensions.emplace_back(FhirExtension::Parse(ext));
        }
    }
    return true;
}

web::json::value FhirExtendable::ToJson() const {
    auto obj = FhirObject::ToJson();
    if (!extensions.empty()) {
        auto arr = web::json::value::array(extensions.size());
        typeof(extensions.size()) i = 0;
        for (const auto &ext : extensions) {
            arr[i++] = ext->ToJson();
        }
        obj["extension"] = arr;
    }
    return obj;
}
