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
                FhirValue::Parse(input.GetPropertyName(), input.ToJson()),
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
                FhirValue::Parse(input.GetPropertyName(), input.ToJson()),
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
                FhirValue::Parse(input.GetPropertyName(), input.ToJson()),
                [](const FhirCodeableConceptValue &cc) {
                    auto coding = cc.GetCoding();
                    AreEqual(0, coding.size());
                    AreEqual("text", cc.GetText());
                });
    }
    {
        auto input = FhirQuantity(10.0, "mg");
        auto output = FhirQuantity::Parse(input.ToJson());
        AreEqual(10, (((long) (output.GetValue() * ((double)10.0))) + 5) / 10);
        AreEqual("mg", output.GetUnit());
    }
    {
        auto input = FhirRatio(FhirQuantity(10.0, "mg"), FhirQuantity(80.0, "ml"));
        auto output = FhirRatio::Parse(input.ToJson());
        AreEqual(10, (((long) (output.GetNumerator().GetValue() * ((double)10.0))) + 5) / 10);
        AreEqual("mg", output.GetNumerator().GetUnit());
        AreEqual(80, (((long) (output.GetDenominator().GetValue() * ((double)10.0))) + 5) / 10);
        AreEqual("ml", output.GetDenominator().GetUnit());
    }
    {
        auto input = FhirReference("testRef", "ReferenceType", "Reference Display");
        auto output = FhirReference::Parse(input.ToJson());
        AreEqual("testRef", output.GetReference());
        AreEqual("ReferenceType", output.GetType());
        AreEqual("Reference Display", output.GetDisplay());
    }
    {
        auto input = FhirIdentifier(FhirCodeableConcept("text"), "use", "value");
        auto output = FhirIdentifier::Parse(input.ToJson());
        AreEqual("text", output.GetType().GetText());
        AreEqual("use", output.GetUse());
        AreEqual("value", output.GetValue());
    }

    /*
     * Value extension:
     */
    {
        auto input = std::make_shared<FhirString>("test");
        OfDynamicType<FhirValueExtension>(
                FhirExtension::Parse(FhirValueExtension("string", input).ToJson()),
                [](const FhirValueExtension &ext) {
                    AreEqual("string", ext.GetUrl());
                    OfDynamicType<FhirString>(ext.GetValue(), [] (const FhirString &str) {
                        AreEqual("test", str.GetValue());
                    });
                });
    }
    {
        auto coding = FhirCoding::Parse(FhirCoding("system", "code", "display").ToJson());
        AreEqual("system", coding.GetSystem());
        AreEqual("code", coding.GetCode());
        AreEqual("display", coding.GetDisplay());
    }
    {
        auto input = std::make_shared<FhirCodeableConceptValue>(FhirCodeableConceptValue({{"s0", "c0", "d0"}, {"s1", "c1", "d1"}}));
        OfDynamicType<FhirValueExtension>(FhirExtension::Parse(FhirValueExtension("cc", input).ToJson()), [] (const FhirValueExtension &ext) {
            OfDynamicType<FhirCodeableConceptValue>(
                    ext.GetValue(),
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
        });
    }
    {
        auto input = std::make_shared<FhirCodeableConceptValue>(FhirCodeableConceptValue("text"));
        OfDynamicType<FhirValueExtension>(FhirExtension::Parse(FhirValueExtension("cc", input).ToJson()), [] (const FhirValueExtension &ext) {
            OfDynamicType<FhirCodeableConceptValue>(
                    ext.GetValue(),
                    [](const FhirCodeableConceptValue &cc) {
                        auto coding = cc.GetCoding();
                        AreEqual(0, coding.size());
                        AreEqual("text", cc.GetText());
                    });
        });
    }
    return 0;
}
