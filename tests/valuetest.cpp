//
// Created by jeo on 12/7/23.
//

#include <iostream>
#include <fhir/value.h>
#include "assert.h"

int main() {
    {
        auto input = FhirString("test");
        OfDynamicType<FhirString>(
                FhirValue::Parse(web::json::value::string(input.GetPropertyName()), input.ToJson()),
                [](const FhirString &str) {
                    AreEqual("test", str.GetValue());
                });
    }
    {
        auto coding = FhirCoding::Parse(FhirCoding("system", "code", "display").ToJson());
        AreEqual("system", coding.GetSystem());
        AreEqual("code", coding.GetCode());
        AreEqual("display", coding.GetDisplay());
    }
    {
        auto input = FhirCodeableConceptValue({{"s0", "c0", "d0"}, {"s1", "c1", "d1"}});
        OfDynamicType<FhirCodeableConceptValue>(
                FhirValue::Parse(web::json::value::string(input.GetPropertyName()), input.ToJson()),
                [](const FhirCodeableConceptValue &cc) {
                    auto coding = cc.GetCoding();
                    AreEqual(2, coding.size());
                    auto c0 = coding.at(0);
                    auto c1 = coding.at(1);
                    AreEqual("s0", c0.GetSystem());
                    AreEqual("c0", c0.GetCode());
                    AreEqual("d0", c0.GetDisplay());
                    AreEqual("s1", c1.GetSystem());
                    AreEqual("c1", c1.GetCode());
                    AreEqual("d1", c1.GetDisplay());
                    AreEqual("", cc.GetText());
                });
    }
    {
        auto input = FhirCodeableConceptValue("text");
        OfDynamicType<FhirCodeableConceptValue>(
                FhirValue::Parse(web::json::value::string(input.GetPropertyName()), input.ToJson()),
                [](const FhirCodeableConceptValue &cc) {
                    auto coding = cc.GetCoding();
                    AreEqual(0, coding.size());
                    AreEqual("text", cc.GetText());
                });
    }
    return 0;
}
