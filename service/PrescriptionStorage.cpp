//
// Created by sigsegv on 3/1/24.
//

#include "PrescriptionStorage.h"
#include "DataDirectory.h"
#include <boost/uuid/uuid_generators.hpp> // for random_generator
#include <boost/uuid/uuid_io.hpp> // for to_string
#include "../domain/prescription.h"
#include <cpprest/json.h>

std::string PrescriptionStorage::Store(const std::string &patient, const Prescription &prescription) const {
    if (patient.empty()) {
        return {};
    }
    auto dir = DataDirectory::Data("sfmbasisfaker").Sub("prescription").Sub(patient);
    std::string clobId{};
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        clobId = boost::uuids::to_string(randomUUID);
    }
    dir.WriteFile(clobId, prescription.Serialize());
    return clobId;
}

Prescription PrescriptionStorage::Load(const std::string &patient, const std::string &prescriptionId) const {
    if (patient.empty() || prescriptionId.empty()) {
        return {};
    }
    auto dir = DataDirectory::Data("sfmbasisfaker").Sub("prescription").Sub(patient);
    auto json = dir.ReadFile(prescriptionId);
    if (json.empty()) {
        return {};
    }
    return Prescription::Parse(json);
}

void PrescriptionStorage::StorePatientMap(const std::string &patient, const std::vector<std::string> &prescriptions) {
    if (patient.empty()) {
        return;
    }
    std::string clob{};
    {
        auto arr = web::json::value::array();
        int i = 0;
        for (const auto &p: prescriptions) {
            arr[i++] = web::json::value::string(p);
        }
        clob = arr.serialize();
    }
    auto dir = DataDirectory::Data("sfmbasisfaker").Sub("prescription").Sub(patient);
    dir.WriteFile("map.json", clob);
}

std::vector<std::string> PrescriptionStorage::LoadPatientMap(const std::string &patient) const {
    if (patient.empty()) {
        return {};
    }
    auto dir = DataDirectory::Data("sfmbasisfaker").Sub("prescription").Sub(patient);
    auto clob = dir.ReadFile("map.json");
    if (clob.empty()) {
        return {};
    }
    auto arr = web::json::value::parse(clob).as_array();
    std::vector<std::string> prescriptions;
    for (const auto &val: arr) {
        prescriptions.push_back(val.as_string());
    }
    return prescriptions;
}
