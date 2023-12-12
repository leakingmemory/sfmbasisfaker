//
// Created by jeo on 08.12.2023.
//

#include <fhir/bundleentry.h>
#include <fhir/fhir.h>
#include <fhir/value.h>

web::json::value FhirBundleEntry::ToJson() const {
    auto obj = FhirObject::ToJson();
    if (!fullUrl.empty()) {
        obj["fullUrl"] = web::json::value::string(fullUrl);
    }
    if (resource) {
        obj["resource"] = resource->ToJson();
    }
    return obj;
}

FhirBundleEntry FhirBundleEntry::Parse(const web::json::value &obj) {
    FhirBundleEntry entry{};
    if (obj.has_string_field("fullUrl")) {
        entry.fullUrl = obj.at("fullUrl").as_string();
    }
    if (obj.has_object_field("resource")) {
        entry.resource = Fhir::Parse(obj.at("resource"));
    }
    return entry;
}

FhirReference FhirBundleEntry::CreateReference(const std::string &type) const {
    return {fullUrl, type, resource ? resource->GetDisplay() : "Display"};
}