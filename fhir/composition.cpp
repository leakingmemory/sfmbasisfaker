//
// Created by jeo on 08.12.2023.
//

#include <fhir/composition.h>

web::json::value FhirComposition::ToJson() const {
    auto obj = Fhir::ToJson();
    if (identifier.IsSet()) {
        obj["identifier"] = identifier.ToJson();
    }
    if (type.IsSet()) {
        obj["type"] = type.ToJson();
    }
    if (subject.IsSet()) {
        obj["subject"] = subject.ToJson();
    }
    if (!authors.empty()) {
        auto arr = web::json::value::array(authors.size());
        typeof(authors.size()) i = 0;
        for (const auto &a : authors) {
            arr[i++] = a.ToJson();
        }
        obj["author"] = arr;
    }
    if (!title.empty()) {
        obj["title"] = web::json::value::string(title);
    }
    if (!confidentiality.empty()) {
        obj["confidentiality"] = web::json::value::string(confidentiality);
    }
    if (!sections.empty()) {
        auto arr = web::json::value::array(sections.size());
        typeof(sections.size()) i = 0;
        for (const auto &s : sections) {
            arr[i++] = s.ToJson();
        }
        obj["section"] = arr;
    }
    return obj;
}

FhirComposition FhirComposition::Parse(const web::json::value &obj) {
    FhirComposition comp{};

    if (!comp.ParseInline(obj)) {
        throw std::exception();
    }

    if(obj.has_object_field("identifier"))
        comp.identifier = FhirIdentifier::Parse(obj.at("identifier"));

    if(obj.has_object_field("type"))
        comp.type = FhirCodeableConcept::Parse(obj.at("type"));

    if(obj.has_object_field("subject"))
        comp.subject = FhirReference::Parse(obj.at("subject"));

    if(obj.has_array_field("author")) {
        web::json::array authors_arr = obj.at("author").as_array();

        for(int i=0; i<authors_arr.size(); i++) {
            comp.authors.push_back(FhirReference::Parse(authors_arr[i]));
        }
    }

    if(obj.has_string_field("title"))
        comp.title = obj.at("title").as_string();

    if(obj.has_string_field("confidentiality"))
        comp.confidentiality = obj.at("confidentiality").as_string();

    if(obj.has_array_field("section")) {
        web::json::array sections_arr = obj.at("section").as_array();

        for(int i=0; i<sections_arr.size(); i++) {
            comp.sections.push_back(FhirCompositionSection::Parse(sections_arr[i]));
        }
    }

    return comp;
}
