//
// Created by jeo on 08.12.2023.
//

#ifndef SFMBASISFAKER_COMPOSITIONSECTION_H
#define SFMBASISFAKER_COMPOSITIONSECTION_H

#include "value.h"

class FhirCompositionSection : public FhirObject {
private:
    FhirCodeableConcept code{};
    FhirCodeableConcept emptyReason{};
    std::vector<FhirReference> entries{};
    std::string title{};
    std::string textStatus{};
    std::string textXhtml{};
public:
    [[nodiscard]] FhirCodeableConcept GetCode() const { return code; }
    [[nodiscard]] FhirCodeableConcept GetEmptyReason() const { return emptyReason; }
    [[nodiscard]] std::vector<FhirReference> GetEntries() const { return entries; }
    [[nodiscard]] std::string GetTitle() const { return title; }
    [[nodiscard]] std::string GetTextStatus() const { return textStatus; }
    [[nodiscard]] std::string GetTextXhtml() const { return textXhtml; }

    [[nodiscard]] web::json::value ToJson() const;
    static FhirCompositionSection Parse(const web::json::value &obj);
};

#endif //SFMBASISFAKER_COMPOSITIONSECTION_H