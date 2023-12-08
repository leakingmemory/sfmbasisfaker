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
    [[nodiscard]] std::vector<FhirIdentifier> GetIdentifiers() const { return identifiers; }
    [[nodiscard]] std::vector<FhirName> GetName() const { return name; }
    [[nodiscard]] std::string GetGender() const { return gender; }
    [[nodiscard]] std::string GetBirthDate() const { return birthDate; }
    bool IsActive() const { return active; }

    [[nodiscard]] web::json::value ToJson() const;
    static FhirPerson Parse(const web::json::value &obj);
};

#endif //SFMBASISFAKER_PERSON_H
