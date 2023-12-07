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

bool Fhir::ParseInline(const web::json::value &json) {
    if (!FhirExtendable::ParseInline(json)) {
        return false;
    }
    if (json.has_string_field("resourceType")) {
        resourceType = json.at("resourceType").as_string();
    }
    if (json.has_string_field("id")) {
        id = json.at("id").as_string();
    }
    if (json.has_object_field("meta")) {
        auto meta = json.at("meta");
        if (meta.has_array_field("profile")) {
            for (const auto &p : meta.at("profile").as_array()) {
                if (p.is_string()) {
                    profile.emplace_back(p.as_string());
                }
            }
        }
    }
    if (json.has_string_field("status")) {
        auto s = json.at("status").as_string();
        if (s == "active") {
            status = FhirStatus::ACTIVE;
        } else {
            throw std::exception();
        }
    }
    return true;
}

web::json::value Fhir::ToJson() const {
    auto obj = FhirExtendable::ToJson();
    if (!resourceType.empty()) {
        obj["resourceType"] = web::json::value::string(resourceType);
    }
    if (!id.empty()) {
        obj["id"] = web::json::value::string(id);
    }
    if (!profile.empty()) {
        auto meta = web::json::value::object();
        {
            auto arr = web::json::value::array(profile.size());
            typeof(profile.size()) i = 0;
            for (const auto &p: profile) {
                arr[i++] = web::json::value::string(p);
            }
            meta["profile"] = arr;
        }
        obj["meta"] = meta;
    }
    switch (status) {
        case FhirStatus::ACTIVE:
            obj["status"] = web::json::value::string("active");
            break;
    }
    return obj;
}