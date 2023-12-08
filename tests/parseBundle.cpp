//
// Created by jeo on 08.12.2023.
//
#include <fhir/bundle.h>
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
}