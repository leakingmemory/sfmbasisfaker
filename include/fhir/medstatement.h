//
// Created by sigsegv on 12/7/23.
//

#ifndef SFMBASISFAKER_MEDSTATEMENT_H
#define SFMBASISFAKER_MEDSTATEMENT_H

#include "value.h"
#include "dosage.h"

class FhirMedicationStatement : public Fhir {
private:
    std::vector<FhirIdentifier> identifiers{};
    std::vector<FhirDosage> dosage{};
    FhirReference medicationReference{};
    FhirReference subject{};
public:
    web::json::value ToJson() const;
    static FhirMedicationStatement Parse(const web::json::value &obj);

    std::vector<FhirIdentifier> GetIdentifiers() const { return identifiers; }
    std::vector<FhirDosage> GetDosage() const { return dosage; }
    FhirReference GetMedicationReference() const { return medicationReference; }
    FhirReference GetSubject() const { return subject; }
};

#endif //SFMBASISFAKER_MEDSTATEMENT_H
