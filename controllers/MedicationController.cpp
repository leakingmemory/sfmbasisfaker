//
// Created by sigsegv on 12/9/23.
//

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "MedicationController.h"
#include <fhir/organization.h>
#include <nhnfhir/KjRfErrorCode.h>
#include <fhir/composition.h>
#include <IsoDateTime.h>

FhirParameters MedicationController::GetMedication(const std::string &selfUrl, const FhirPerson &patient) {
    // TODO - from JWT
    FhirBundleEntry practitionerEntry{};
    {
        std::string url{"urn:uuid:"};
        {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string uuid_string = to_string(uuid);
            url.append(uuid_string);
        }
        practitionerEntry = FhirBundleEntry::Create<FhirPractitioner>(url);
        auto p = *(std::dynamic_pointer_cast<FhirPerson>(practitionerEntry.GetResource()));
        {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string uuid_string = to_string(uuid);
            p.SetId(uuid_string);
        }
        p.SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Practitioner");
        {
            std::vector<FhirIdentifier> identifiers{};
            // TODO - HPR
            identifiers.emplace_back(FhirCodeableConcept("http://hl7.no/fhir/NamingSystem/HPR", "HPR-nummer", ""), "official", "urn:oid:2.16.578.1.12.4.1.4.4", "7479654");
            // TODO - fnr
            identifiers.emplace_back(FhirCodeableConcept("http://hl7.no/fhir/NamingSystem/FNR", "FNR-nummer", ""), "official", "urn:oid:2.16.578.1.12.4.1.4.1", "23048201385");
            p.SetIdentifiers(identifiers);
        }
        p.SetActive(true);
        p.SetName({{"official", "Hansen", "Jesper Odd"}});
        p.SetGender("female");
        p.SetBirthDate("1982-04-23");
    }
    // TODO - HER-id
    FhirBundleEntry organizationEntry{};
    {
        std::string url{"urn:uuid:"};
        {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string uuid_string = to_string(uuid);
            url.append(uuid_string);
        }
        organizationEntry = FhirBundleEntry::Create<FhirOrganization>(url);
        auto o = *(std::dynamic_pointer_cast<FhirOrganization>(practitionerEntry.GetResource()));
        {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string uuid_string = to_string(uuid);
            o.SetId(uuid_string);
        }
        o.SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Organization");
        {
            std::vector<FhirIdentifier> identifiers{};
            identifiers.emplace_back("official", "urn:oid:2.16.578.1.12.4.1.2", "79744");
            o.SetIdentifiers(identifiers);
        }
    }
    FhirBundleEntry patientEntry{};
    {
        std::string url{"urn:uuid:"};
        {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string uuid_string = to_string(uuid);
            url.append(uuid_string);
        }
        patientEntry = FhirBundleEntry::Create<FhirPatient>(url);
        auto &p = *(std::dynamic_pointer_cast<FhirPerson>(patientEntry.GetResource()));
        {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string uuid_string = to_string(uuid);
            p.SetId(uuid_string);
        }
        p.SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Patient");
        p.SetIdentifiers(patient.GetIdentifiers());
        p.SetActive(patient.IsActive());
        p.SetName(patient.GetName());
        p.SetGender(patient.GetGender());
        p.SetBirthDate(patient.GetBirthDate());
        p.SetAddress(patient.GetAddress());
    }
    FhirCompositionSection medicationSection{};
    medicationSection.SetTitle("Medication");
    medicationSection.SetCode(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-section-types", "sectionMedication", "List of Medication statements"));
    medicationSection.SetTextStatus("generated");
    medicationSection.SetTextXhtml("<xhtml:div xmlns:xhtml=\"http://www.w3.org/1999/xhtml\">List of medications</xhtml:div>");
    medicationSection.SetEmptyReason(FhirCodeableConcept("http://terminology.hl7.org/CodeSystem/list-empty-reason", "unavailable", "Unavailable"));
    FhirCompositionSection allergiesSection{};
    allergiesSection.SetTitle("Allergies");
    allergiesSection.SetCode(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-section-types", "sectionAllergies", "Section allergies"));
    allergiesSection.SetTextStatus("generated");
    allergiesSection.SetTextXhtml("<xhtml:div xmlns:xhtml=\"http://www.w3.org/1999/xhtml\"></xhtml:div>");
    allergiesSection.SetEmptyReason(FhirCodeableConcept("http://terminology.hl7.org/CodeSystem/list-empty-reason", "unavailable", "Unavailable"));
    FhirCompositionSection otherPrescriptionsSection{};
    otherPrescriptionsSection.SetTitle("Other Prescriptions");
    otherPrescriptionsSection.SetCode(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-section-types", "sectionOtherPrescriptions", "List of non medical prescriptions"));
    otherPrescriptionsSection.SetTextStatus("generated");
    otherPrescriptionsSection.SetTextXhtml("<xhtml:div xmlns:xhtml=\"http://www.w3.org/1999/xhtml\"></xhtml:div>");
    otherPrescriptionsSection.SetEmptyReason(FhirCodeableConcept("http://terminology.hl7.org/CodeSystem/list-empty-reason", "unavailable", "Unavailable"));
    FhirBundleEntry medicationComposition{};
    {
        std::string url{"urn:uuid:"};
        {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string uuid_string = to_string(uuid);
            url.append(uuid_string);
        }
        medicationComposition = FhirBundleEntry::Create<FhirComposition>(url);
        auto &resource = *(std::dynamic_pointer_cast<FhirComposition>(medicationComposition.GetResource()));
        {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string uuid_string = to_string(uuid);
            resource.SetId(uuid_string);
        }
        resource.SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-MedicationComposition");
        {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string uuid_string = to_string(uuid);
            resource.SetIdentifier(FhirIdentifier("official", uuid_string));
        }
        resource.SetStatus(FhirStatus::FINAL);
        resource.SetType(FhirCodeableConcept("http://loinc.org", "11503-0", "Medical records"));
        resource.SetSubject(patientEntry.CreateReference("http://ehelse.no/fhir/StructureDefinition/sfm-Patient"));
        resource.SetDate(toIsoDate(std::time(nullptr)));
        resource.SetAuthors({
            practitionerEntry.CreateReference("http://ehelse.no/fhir/StructureDefinition/sfm-Practitioner"),
            organizationEntry.CreateReference("http://ehelse.no/fhir/StructureDefinition/sfm-Organization")
        });
        resource.SetTitle("Medication summary");
        resource.SetConfidentiality("N");
        resource.AddSection(medicationSection);
        resource.AddSection(allergiesSection);
        resource.AddSection(otherPrescriptionsSection);
    }
    auto bundle = std::make_shared<FhirBundle>();
    {
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        std::string uuid_string = to_string(uuid);
        bundle->SetId(uuid_string);
    }
    bundle->AddLink("self", selfUrl);
    bundle->SetType("document");
    auto msgTimestamp = toIsoDateTime(((int64_t) std::time(nullptr)) * ((int64_t)1000));
    bundle->SetLastUpdated(msgTimestamp);
    bundle->SetSource(selfUrl);
    bundle->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-MedicationBundle");
    bundle->SetTimestamp(msgTimestamp);
    bundle->AddEntry(medicationComposition);
    bundle->AddEntry(practitionerEntry);
    bundle->AddEntry(organizationEntry);
    bundle->AddEntry(patientEntry);
    FhirParameters parameters{};
    parameters.AddParameter("medication", bundle);
    parameters.AddParameter("KJHentetTidspunkt", std::make_shared<FhirDateTimeValue>("2023-12-06T14:13:03.7904472+01:00"));
    parameters.AddParameter("RFHentetTidspunkt", std::make_shared<FhirDateTimeValue>("2023-12-06T14:13:03.689+01:00"));
    parameters.AddParameter("KJFeilkode", std::make_shared<FhirCodeableConceptValue>(KjRfErrorCode::Ok));
    parameters.AddParameter("RFM96Feilkode", std::make_shared<FhirCodeableConceptValue>(KjRfErrorCode::Ok));
    parameters.AddParameter("RFM912Feilkode", std::make_shared<FhirCodeableConceptValue>(KjRfErrorCode::Ok));
    parameters.AddParameter("KJHarLegemidler", std::make_shared<FhirBooleanValue>(false));
    parameters.AddParameter("KJHarLaste", std::make_shared<FhirBooleanValue>(false));
    parameters.AddParameter("RFHarLaste", std::make_shared<FhirBooleanValue>(false));
    return parameters;
}

FhirParameters MedicationController::SendMedication(const FhirBundle &bundle) {
    FhirParameters parameters{};
    parameters.AddParameter("recallCount", std::make_shared<FhirIntegerValue>(0));
    parameters.AddParameter("prescriptionCount", std::make_shared<FhirIntegerValue>(0));
    return parameters;
}
