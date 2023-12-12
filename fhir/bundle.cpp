//
// Created by jeo on 08.12.2023.
//

#include <fhir/bundle.h>
#include <fhir/value.h>

void FhirBundle::AddLink(const std::string &relation, const std::string &url) {
    link.emplace_back(relation, url);
}

void FhirBundle::AddLink(std::string &&relation, std::string &&url) {
    link.emplace_back(relation, url);
}

void FhirBundle::AddEntry(const FhirBundleEntry &entry) {
    entries.emplace_back(entry);
}

void FhirBundle::AddEntry(FhirBundleEntry &&entry) {
    entries.emplace_back(std::move(entry));
}

web::json::value FhirBundle::ToJson() const {
    auto obj = Fhir::ToJson();
    if (!type.empty()) {
        obj["type"] = web::json::value::string(type);
    }
    if (!link.empty()) {
        auto arr = web::json::value::array(link.size());
        typeof(link.size()) i = 0;
        for (const auto &l : link) {
            arr[i++] = l.ToJson();
        }
        obj["link"] = arr;
    }
    obj["total"] = web::json::value::number(entries.size());
    auto arr = web::json::value::array(entries.size());
    typeof(entries.size()) i = 0;
    for (const auto &e : entries) {
        arr[i++] = e.ToJson();
    }
    obj["entry"] = arr;
    return obj;
}

FhirBundle FhirBundle::Parse(const web::json::value &obj) {
    FhirBundle bundle{};

    if (!bundle.ParseInline(obj)) {
        throw std::exception();
    }

    if (obj.has_string_field("type")) {
        bundle.type = obj.at("type").as_string();
    }
    if (obj.has_number_field("total")) {
        bundle.total = obj.at("total").as_number().to_int32();
    }
    if (obj.has_array_field("link")) {
        for (const auto &value : obj.at("link").as_array()) {
            bundle.link.push_back(FhirLink::Parse(value));
        }
    }
    if (obj.has_array_field("entry")) {
        for (const auto &value : obj.at("entry").as_array()) {
            bundle.entries.push_back(FhirBundleEntry::Parse(value));
        }
    }

    return bundle;
}