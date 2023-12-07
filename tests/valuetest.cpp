//
// Created by jeo on 12/7/23.
//

#include <iostream>
#include <fhir/value.h>
#include "assert.h"

int main() {
    OfDynamicType<FhirString>(FhirValue::Parse(web::json::value::string("valueString"), FhirString("test").ToJson()), [] (const FhirString &str) {
        AreEqual("test", str.GetValue());
    });
    auto coding = FhirCoding::Parse(FhirCoding("system", "code", "display").ToJson());
    AreEqual("system", coding.GetSystem());
    AreEqual("code", coding.GetCode());
    AreEqual("display", coding.GetDisplay());
    return 0;
}
