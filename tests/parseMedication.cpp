//
// Created by jeo on 12/7/23.
//

#include <fhir/medication.h>
#include <fstream>
#include <sstream>
#include "assert.h"

int main(int argc, char **argv) {
    AreEqual(3, argc);
    std::string json{};
    {
        std::string filename = argv[1];
        std::ifstream input{filename};
        if (!input.is_open()) {
            throw std::exception();
        }
        std::stringstream sstr{};
        sstr << input.rdbuf();
        json = sstr.str();
        input.close();
    }
    auto medication = FhirMedication::Parse(FhirMedication::Parse(web::json::value::parse(json)).ToJson());
    AreEqual("Medication", medication.GetResourceType());
    AreEqual("43033452", medication.GetId());
    AreEqual("http://ehelse.no/fhir/StructureDefinition/sfm-Magistrell-Medication", medication.GetProfile().at(0));
    AreEqual((int) FhirStatus::ACTIVE, (int) medication.GetStatus());
    AreEqual("urn:oid:2.16.578.1.12.4.1.1.7424", medication.GetCode().GetCoding().at(0).GetSystem());
    AreEqual("10", medication.GetCode().GetCoding().at(0).GetCode());
    AreEqual("Magistrell", medication.GetCode().GetCoding().at(0).GetDisplay());
    AreEqual("urn:oid:2.16.578.1.12.4.1.1.7448", medication.GetForm().GetCoding().at(0).GetSystem());
    AreEqual("880", medication.GetForm().GetCoding().at(0).GetCode());
    AreEqual("inj/inf, oppl", medication.GetForm().GetCoding().at(0).GetDisplay());
    AreEqual(2, medication.GetIngredients().size());
    AreEqual(750, ((long) (((medication.GetAmount().GetNumerator().GetValue() * ((double)10.0)) + ((double)5.0)))) / 10);
}