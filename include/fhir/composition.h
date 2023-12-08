//
// Created by jeo on 08.12.2023.
//

#ifndef SFMBASISFAKER_COMPOSITION_H
#define SFMBASISFAKER_COMPOSITION_H

#include "fhir.h"
#include "value.h"
#include "compositionsection.h"

class FhirComposition : public Fhir {
private:
    FhirIdentifier identifier{};
    FhirCodeableConcept type{};
    FhirReference subject{};
    std::vector<FhirReference> authors{};
    std::vector<FhirCompositionSection> sections{};
    std::string title{};
    std::string confidentiality{};
public:
    [[nodiscard]] FhirIdentifier GetIdentifier() const { return identifier; }
    [[nodiscard]] FhirCodeableConcept GetType() const { return type; }
    [[nodiscard]] FhirReference GetSubject() const { return subject; }
    [[nodiscard]] std::vector<FhirReference> GetAuthors() const { return authors; }
    [[nodiscard]] std::vector<FhirCompositionSection> GetSections() const { return sections; }
    [[nodiscard]] std::string GetTitle() const { return title; }
    [[nodiscard]] std::string GetConfidentiality() const { return confidentiality; }

    [[nodiscard]] web::json::value ToJson() const;
    static FhirComposition Parse(const web::json::value &obj);
};

#endif //SFMBASISFAKER_COMPOSITION_H
