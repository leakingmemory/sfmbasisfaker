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
