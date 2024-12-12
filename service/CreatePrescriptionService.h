//
// Created by sigsegv on 2/22/24.
//

#ifndef SFMBASISFAKER_CREATEPRESCRIPTIONSERVICE_H
#define SFMBASISFAKER_CREATEPRESCRIPTIONSERVICE_H

#include <memory>
#include <vector>
#include "../domain/prescription.h"

class FhirMedicationStatement;
class FhirMedication;
class FhirBundle;
class FhirReference;
class Person;
class FhirPerson;
class FhirBundleEntry;

struct PractitionerRef {
    std::string id;
    std::string name;
};

class CreatePrescriptionService {
private:
    [[nodiscard]] Person GetPerson(const FhirPerson &fhir) const;
public:
    [[nodiscard]] Person GetPerson(const FhirBundle &bundle, const std::string &fullUrl) const;
    [[nodiscard]] std::shared_ptr<Medication> CreateMedication(const FhirReference &medicationReference, const FhirBundle &bundle) const;
    [[nodiscard]] Prescription CreatePrescription(const std::shared_ptr<FhirMedicationStatement> &medicationStatement, const FhirBundle &bundle) const;
private:
    [[nodiscard]] std::vector<FhirBundleEntry> CreateBundleEntryFromMedication(const std::shared_ptr<FhirMedication> &medication) const;
    void SetCommonFhirMedication(FhirMedication &entry, const Medication &medication) const;
    [[nodiscard]] std::vector<FhirBundleEntry> CreateFhirMedicationFromMagistral(const std::shared_ptr<MagistralMedication> &medication) const;
    [[nodiscard]] std::vector<FhirBundleEntry> CreateFhirMedicationFromPackage(const std::shared_ptr<PackageMedication> &medication) const;
    [[nodiscard]] std::vector<FhirBundleEntry> CreateFhirMedicationFromBrandName(const std::shared_ptr<BrandNameMedication> &medication) const;
    [[nodiscard]] std::vector<FhirBundleEntry> CreateFhirMedicationFromGeneric(const std::shared_ptr<GenericMedication> &generic) const;
public:
    [[nodiscard]] std::vector<FhirBundleEntry> CreateFhirMedication(const std::shared_ptr<Medication> &medication) const;
    [[nodiscard]] PractitionerRef GetPractitionerRef(const std::string &id, std::vector<FhirBundleEntry> &practitioners) const;
    [[nodiscard]] FhirBundleEntry CreateFhirMedicationStatement(const Prescription &prescription, std::vector<FhirBundleEntry> &practitioners);
};


#endif //SFMBASISFAKER_CREATEPRESCRIPTIONSERVICE_H
