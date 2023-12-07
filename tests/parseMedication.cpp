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
    auto medication = FhirMedication::Parse(web::json::value::parse(json));
    AreEqual("Medication", medication.GetResourceType());
    AreEqual("43033452", medication.GetId());
    AreEqual("http://ehelse.no/fhir/StructureDefinition/sfm-Magistrell-Medication", medication.GetProfile());
    AreEqual((int) FhirStatus::ACTIVE, (int) medication.GetStatus());
}