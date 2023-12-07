//
// Created by jeo on 12/7/23.
//

#ifndef SFMBASISFAKER_MEDICATION_H
#define SFMBASISFAKER_MEDICATION_H

#include "value.h"

class FhirMedication : public Fhir {
    FhirCodeableConcept code{};
    FhirCodeableConcept form{};
public:
    static FhirMedication Parse(const web::json::value &);
    [[nodiscard]] FhirCodeableConcept GetCode() const {
        return code;
    }
    [[nodiscard]] FhirCodeableConcept GetForm() const {
        return form;
    }
};

#endif //SFMBASISFAKER_MEDICATION_H
