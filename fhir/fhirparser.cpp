//
// Created by jeo on 08.12.2023.
//

#include <fhir/fhir.h>
#include <fhir/medication.h>
#include <fhir/medstatement.h>
#include <fhir/composition.h>
#include <fhir/person.h>
#include <fhir/organization.h>
#include <fhir/substance.h>

std::shared_ptr<Fhir> Fhir::Parse(const web::json::value &obj) {
    if (!obj.has_string_field("resourceType")) {
        throw std::exception();
    }
    auto resourceType = obj.at("resourceType").as_string();
    if (resourceType == "Medication") {
        return std::make_shared<FhirMedication>(FhirMedication::Parse(obj));
    }
    if (resourceType == "MedicationStatement") {
        return std::make_shared<FhirMedicationStatement>(FhirMedicationStatement::Parse(obj));
    }
    if (resourceType == "Composition") {
        return std::make_shared<FhirComposition>(FhirComposition::Parse(obj));
    }
    if (resourceType == "Practitioner" || resourceType == "Patient") {
        return std::make_shared<FhirPerson>(FhirPerson::Parse(obj));
    }
    if (resourceType == "Organization") {
        return std::make_shared<FhirOrganization>(FhirOrganization::Parse(obj));
    }
    if (resourceType == "Substance") {
        return std::make_shared<FhirSubstance>(FhirSubstance::Parse(obj));
    }
    throw std::exception();
}