//
// Created by sigsegv on 2/22/24.
//

#ifndef SFMBASISFAKER_CREATEPRESCRIPTIONSERVICE_H
#define SFMBASISFAKER_CREATEPRESCRIPTIONSERVICE_H

#include <memory>
#include <vector>
#include "../domain/prescription.h"

class FhirMedicationStatement;
class FhirBundle;
class FhirReference;
class Person;
class FhirPerson;
class FhirBundleEntry;

class CreatePrescriptionService {
private:
    [[nodiscard]] Person GetPerson(const FhirPerson &fhir) const;
    [[nodiscard]] Person GetPerson(const FhirBundle &bundle, const std::string &fullUrl) const;
public:
    [[nodiscard]] std::shared_ptr<Medication> CreateMedication(const FhirReference &medicationReference, const FhirBundle &bundle) const;
    [[nodiscard]] Prescription CreatePrescription(const std::shared_ptr<FhirMedicationStatement> &medicationStatement, const FhirBundle &bundle) const;
private:
    [[nodiscard]] std::vector<FhirBundleEntry> CreateFhirMedicationFromMagistral(const std::shared_ptr<MagistralMedication> &medication) const;
public:
    [[nodiscard]] std::vector<FhirBundleEntry> CreateFhirMedication(const std::shared_ptr<Medication> &medication) const;
    [[nodiscard]] FhirBundleEntry CreateFhirMedicationStatement(const Prescription &prescription, std::vector<FhirBundleEntry> &practitioners);
};


#endif //SFMBASISFAKER_CREATEPRESCRIPTIONSERVICE_H
