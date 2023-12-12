//
// Created by jeo on 08.12.2023.
//

#ifndef SFMBASISFAKER_ORGANIZATION_H
#define SFMBASISFAKER_ORGANIZATION_H

#include "value.h"
#include "fhir.h"

class FhirOrganization : public Fhir {
private:
    std::vector<FhirIdentifier> identifiers{};
public:
    [[nodiscard]] std::vector<FhirIdentifier> getIdentifiers() const {
        return identifiers;
    }
    void SetIdentifiers(const std::vector<FhirIdentifier> &newIdentifiers) {
        identifiers = newIdentifiers;
    }

    [[nodiscard]] web::json::value ToJson() const;
    static FhirOrganization Parse(const web::json::value &obj);
    std::string GetDisplay() const override;
};

#endif //SFMBASISFAKER_ORGANIZATION_H
