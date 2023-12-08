//
// Created by jeo on 08.12.2023.
//

#include <fhir/compositionsection.h>

web::json::value FhirCompositionSection::ToJson() const {
    auto obj = FhirObject::ToJson();
    if (!title.empty()) {
        obj["title"] = web::json::value::string(title);
    }
    if (code.IsSet()) {
        obj["code"] = code.ToJson();
    }
    if (!textStatus.empty() || !textXhtml.empty()) {
        auto text = web::json::value::object();
        if (!textStatus.empty()) {
            text["status"] = web::json::value::string(textStatus);
        }
        if (!textXhtml.empty()) {
            text["div"] = web::json::value::string(textXhtml);
        }
        obj["text"] = text;
    }
    if (!entries.empty()) {
        auto arr = web::json::value::array(entries.size());
        typeof(entries.size()) i = 0;
        for (const auto &e : entries) {
            arr[i++] = e.ToJson();
        }
        obj["entry"] = arr;
    } else if (emptyReason.IsSet()) {
        obj["emptyReason"] = emptyReason.ToJson();
    }
    return obj;
}

FhirCompositionSection FhirCompositionSection::Parse(const web::json::value &obj) {
    auto section = FhirCompositionSection();

    if (obj.has_string_field("title")) {
        section.title = obj.at("title").as_string();
    }

    if (obj.has_object_field("code")) {
        section.code = FhirCodeableConcept::Parse(obj.at("code"));
    }

    if (obj.has_object_field("text")) {
        auto text = obj.at("text");
        if (text.has_string_field("status")) {
            section.textStatus = text.at("status").as_string();
        }
        if (text.has_string_field("div")) {
            section.textXhtml = text.at("div").as_string();
        }
    }

    if (obj.has_array_field("entry")) {
        auto arr = obj.at("entry").as_array();
        for (const auto &e : arr) {
            section.entries.push_back(FhirReference::Parse(e));
        }
    } else if (obj.has_object_field("emptyReason")) {
        section.emptyReason = FhirCodeableConcept::Parse(obj.at("emptyReason"));
    }

    return section;
}