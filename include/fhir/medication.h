//
// Created by jeo on 12/7/23.
//

#ifndef SFMBASISFAKER_MEDICATION_H
#define SFMBASISFAKER_MEDICATION_H

#include "fhir.h"

class FhirMedication : public Fhir {
public:
    static FhirMedication Parse(const web::json::value &);
};

#endif //SFMBASISFAKER_MEDICATION_H
