//
// Created by jeo on 08.12.2023.
//

#ifndef SFMBASISFAKER_PERSON_H
#define SFMBASISFAKER_PERSON_H

#include "fhir.h"
#include "value.h"

class FhirPerson : public Fhir {
private:
    std::vector<FhirIdentifier> identifiers{};
    std::vector<FhirAddress> address{};
    std::vector<FhirName> name{};
    std::string gender{};
    std::string birthDate{};
    bool active{false};
public:
    constexpr FhirPerson() : Fhir() {}
    constexpr FhirPerson(const std::string &resourceType) : Fhir(resourceType) {}
    [[nodiscard]] std::vector<FhirIdentifier> GetIdentifiers() const { return identifiers; }
    [[nodiscard]] std::vector<FhirName> GetName() const { return name; }
    [[nodiscard]] std::vector<FhirAddress> GetAddress() const { return address; }
    [[nodiscard]] std::string GetGender() const { return gender; }
    [[nodiscard]] std::string GetBirthDate() const { return birthDate; }
    bool IsActive() const { return active; }

    void SetIdentifiers(const std::vector<FhirIdentifier>& newIdentifiers) { identifiers = newIdentifiers; }
    void SetAddress(const std::vector<FhirAddress> &newAddresses) { address = newAddresses; }
    void SetName(const std::vector<FhirName>& newNames) { name = newNames; }
    void SetGender(const std::string &newGender) { gender = newGender; }
    void SetBirthDate(const std::string &newBirthDate) { birthDate = newBirthDate; }
    void SetActive(bool newActive) { active = newActive; }
    [[nodiscard]] web::json::value ToJson() const;
    static FhirPerson Parse(const web::json::value &obj);
    std::string GetDisplay() const override;
};

class FhirPatient : public FhirPerson {
public:
    constexpr FhirPatient() : FhirPerson("Patient") {}
};

class FhirPractitioner : public FhirPerson {
public:
    constexpr FhirPractitioner() : FhirPerson("Practitioner") {}
};

#endif //SFMBASISFAKER_PERSON_H
