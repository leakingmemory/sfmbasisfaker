//
// Created by sigsegv on 12/9/23.
//

#ifndef SFMBASISFAKER_MEDICATIONCONTROLLER_H
#define SFMBASISFAKER_MEDICATIONCONTROLLER_H

#include <fhir/parameters.h>
#include <fhir/person.h>
#include <fhir/bundle.h>

class MedicationController {
public:
    [[nodiscard]] FhirParameters GetMedication(const std::string &selfUrl, const FhirPerson &patient);
    [[nodiscard]] FhirParameters SendMedication(const FhirBundle &bundle);
};

#endif //SFMBASISFAKER_MEDICATIONCONTROLLER_H
