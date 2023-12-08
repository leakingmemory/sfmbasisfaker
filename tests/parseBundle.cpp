//
// Created by jeo on 08.12.2023.
//
#include <fhir/bundle.h>
#include <fhir/person.h>
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
    auto bundle = FhirBundle::Parse(FhirBundle::Parse(web::json::value::parse(json)).ToJson());
    AreEqual(8, bundle.GetTotal());
    std::shared_ptr<FhirPerson> practitioner{};
    std::shared_ptr<FhirPerson> patient{};
    for (const auto &entry : bundle.GetEntries()) {
        std::shared_ptr<FhirPerson> person = std::dynamic_pointer_cast<FhirPerson>(entry.GetResource());
        if (person) {
            auto resourceType = person->GetResourceType();
            if (resourceType == "Practitioner") {
                practitioner = person;
            } else if (resourceType == "Patient") {
                patient = person;
            }
        }
    }
    AreEqual(true, practitioner.operator bool());
    AreEqual(true, patient.operator bool());
    AreEqual("Test", patient->GetName().at(0).GetFamily());
    AreEqual("Jeo 48", patient->GetName().at(0).GetGiven());
    AreEqual("Hansen", practitioner->GetName().at(0).GetFamily());
    AreEqual("Jesper Odd", practitioner->GetName().at(0).GetGiven());
}