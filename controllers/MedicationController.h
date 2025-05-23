//
// Created by sigsegv on 12/9/23.
//

#ifndef SFMBASISFAKER_MEDICATIONCONTROLLER_H
#define SFMBASISFAKER_MEDICATIONCONTROLLER_H

#include <sfmbasisapi/fhir/parameters.h>
#include <sfmbasisapi/fhir/person.h>
#include <sfmbasisapi/fhir/bundle.h>

class Person;

class MedicationController {
public:
    [[nodiscard]] FhirParameters GetMedication(const std::string &selfUrl, const Person &practitioner, const FhirPerson &patient);
    [[nodiscard]] std::shared_ptr<Fhir> SendMedication(const FhirBundle &bundle, const Person &practitioner);
};

#endif //SFMBASISFAKER_MEDICATIONCONTROLLER_H
