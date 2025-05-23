//
// Created by jeo on 5/22/25.
//

#include "PharmacyController.h"
#include "../service/PersonStorage.h"
#include "../domain/person.h"

std::vector<PharmacyPatient> PharmacyController::GetPatients() {
    std::vector<PharmacyPatient> patients{};
    PersonStorage personStorage{};
    for (const auto &person : personStorage.GetPersons()) {
        PharmacyPatient pharmacyPatient{.id = person.GetId()};
        patients.emplace_back(pharmacyPatient);
    }
    return patients;
}