//
// Created by sigsegv on 12/8/23.
//
#include <fhir/medstatement.h>
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
    auto medicationStatement = FhirMedicationStatement::Parse(FhirMedicationStatement::Parse(web::json::value::parse(json)).ToJson());
    AreEqual("urn:uuid:f874c81b-b1ce-4da3-966f-652fe569c5dd", medicationStatement.GetMedicationReference().GetReference());
    AreEqual("urn:uuid:a325b9dc-2aa2-421f-ab93-5bdf99481981", medicationStatement.GetSubject().GetReference());
    AreEqual(1, medicationStatement.GetDosage().size());
}