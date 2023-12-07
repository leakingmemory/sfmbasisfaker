//
// Created by sigsegv on 12/7/23.
//

#include <fhir/fhirobject.h>

web::json::value FhirObject::ToJson() const {
    return web::json::value::object();
}
