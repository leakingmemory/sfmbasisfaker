//
// Created by sigsegv on 12/9/23.
//

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "MedicationController.h"
#include <sfmbasisapi/fhir/organization.h>
#include <sfmbasisapi/nhnfhir/KjRfErrorCode.h>
#include <sfmbasisapi/fhir/composition.h>
#include <sfmbasisapi/fhir/medstatement.h>
#include <sfmbasisapi/IsoDateTime.h>
#include "../domain/person.h"
#include "../domain/prescription.h"
#include "../service/PersonStorage.h"
#include "../service/CreatePrescriptionService.h"
#include "../service/PrescriptionStorage.h"

FhirParameters MedicationController::GetMedication(const std::string &selfUrl, const Person &practitioner, const FhirPerson &fhirPatient) {
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
            identifiers.emplace_back(FhirCodeableConcept("http://hl7.no/fhir/NamingSystem/HPR", "HPR-nummer", ""), "official", "urn:oid:2.16.578.1.12.4.1.4.4", practitioner.GetHpr());
            identifiers.emplace_back(FhirCodeableConcept("http://hl7.no/fhir/NamingSystem/FNR", "FNR-nummer", ""), "official", "urn:oid:2.16.578.1.12.4.1.4.1", practitioner.GetFodselsnummer());
            p.SetIdentifiers(identifiers);
        }
        p.SetActive(true);
        p.SetName({{"official", practitioner.GetFamilyName(), practitioner.GetGivenName()}});
        p.SetGender(practitioner.GetGender() == PersonGender::FEMALE ? "female" : "male");
        p.SetBirthDate(practitioner.GetDateOfBirth());
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
        auto o = *(std::dynamic_pointer_cast<FhirOrganization>(organizationEntry.GetResource()));
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
    Person patient{};
    std::string patientPid{};
    for (const auto &identifier : fhirPatient.GetIdentifiers()) {
        if (identifier.GetSystem() == "urn:oid:2.16.578.1.12.4.1.4.1") {
            patientPid = identifier.GetValue();
        }
    }
    PersonStorage personStorage{};
    if (!patientPid.empty()) {
        patient = personStorage.GetByFodselsnummer(patientPid);
        bool modified{false};
        if (patient.GetFodselsnummer().empty()) {
            patient.SetFodselsnummer(patientPid);
            modified = true;
        }
        auto fhirNames = fhirPatient.GetName();
        {
            bool found{false};
            std::string family{};
            std::string given{};
            for (const auto &fhirName : fhirNames) {
                if (fhirName.GetUse() == "official") {
                    family = fhirName.GetFamily();
                    given = fhirName.GetGiven();
                    found = true;
                }
            }
            if (!found && !fhirNames.empty()) {
                auto fhirName = fhirNames[0];
                family = fhirName.GetFamily();
                given = fhirName.GetGiven();
                found = true;
            }
            if (found) {
                if (patient.GetFamilyName() != family) {
                    patient.SetFamilyName(family);
                    modified = true;
                }
                if (patient.GetGivenName() != given) {
                    patient.SetGivenName(given);
                    modified = true;
                }
            }
        }
        auto fhirAddresses = fhirPatient.GetAddress();
        {
            bool found{false};
            std::string postcode{};
            std::string city{};
            for (const auto &fhirAddress : fhirAddresses) {
                if (fhirAddress.GetUse() == "home") {
                    postcode = fhirAddress.GetPostalCode();
                    city = fhirAddress.GetCity();
                    found = true;
                }
            }
            if (!found && !fhirAddresses.empty()) {
                auto fhirAddress = fhirAddresses[0];
                postcode = fhirAddress.GetPostalCode();
                city = fhirAddress.GetCity();
                found = true;
            }
            if (found) {
                if (patient.GetHomePostalCode() != postcode) {
                    patient.SetHomePostalCode(postcode);
                    modified = true;
                }
                if (patient.GetHomeCity() != city) {
                    patient.SetHomeCity(city);
                    modified = true;
                }
            }
        }
        auto dob = fhirPatient.GetBirthDate();
        if (patient.GetDateOfBirth() != dob) {
            patient.SetDateOfBirth(dob);
            modified = true;
        }
        if (modified) {
            personStorage.Store(patient);
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
        if (!patient.GetId().empty()) {
            auto fnr = patient.GetFodselsnummer();
            if (!fnr.empty()) {
                std::vector<FhirIdentifier> identifiers{};
                {
                    FhirCodeableConcept code{"http://hl7.no/fhir/NamingSystem/FNR", "FNR-nummer", ""};
                    identifiers.emplace_back(code, "official", "urn:oid:2.16.578.1.12.4.1.4.1", fnr);
                }
                p.SetIdentifiers(identifiers);
            }
            p.SetActive(true);
            {
                std::vector<FhirName> names{};
                names.emplace_back("official", patient.GetFamilyName(), patient.GetGivenName());
                p.SetName(names);
            }
            p.SetGender(patient.GetGender() == PersonGender::FEMALE ? "female" : "male");
            p.SetBirthDate(patient.GetDateOfBirth());
            {
                std::vector<FhirAddress> addresses{};
                {
                    std::vector<std::string> lines{};
                    addresses.emplace_back(lines, "home", "physical", patient.GetHomeCity(),
                                           patient.GetHomePostalCode());
                }
                p.SetAddress(addresses);
            }
        } else {
            p.SetIdentifiers(fhirPatient.GetIdentifiers());
            p.SetActive(fhirPatient.IsActive());
            p.SetName(fhirPatient.GetName());
            p.SetGender(fhirPatient.GetGender());
            p.SetBirthDate(fhirPatient.GetBirthDate());
            p.SetAddress(fhirPatient.GetAddress());
        }
    }
    std::vector<FhirBundleEntry> practitioners{};
    practitioners.emplace_back(practitionerEntry);
    std::vector<FhirBundleEntry> medicamentEntries{};
    std::vector<FhirBundleEntry> medicationStatementEntries{};
    std::vector<FhirReference> medicationSectionEntries{};
    {
        PrescriptionStorage prescriptionStorage{};
        CreatePrescriptionService createPrescriptionService{};
        auto lookupList = prescriptionStorage.LoadPatientMap(patient.GetId());
        for (const auto &lookup : lookupList) {
            auto prescription = prescriptionStorage.Load(patient.GetId(), lookup);
            auto medications = createPrescriptionService.CreateFhirMedication(prescription.GetMedication());
            if (medications.empty()) {
                continue;
            }
            auto medicationStatement = createPrescriptionService.CreateFhirMedicationStatement(prescription, practitioners);
            std::string medicationStatementRef = medicationStatement.GetFullUrl();
            if (medicationStatementRef.empty()) {
                continue;
            }
            auto medicationStatementResource = std::dynamic_pointer_cast<FhirMedicationStatement>(medicationStatement.GetResource());
            if (!medicationStatementResource) {
                continue;
            }
            std::string medicationStatementDisplay = medicationStatementResource->GetDisplay();
            std::string medicationRef{};
            std::string medicationType{};
            std::string medicationDisplay{};
            {
                const FhirBundleEntry *lastEntry;
                for (const auto &entry : medications) {
                    lastEntry = &entry;
                    medicamentEntries.emplace_back(entry);
                }
                medicationRef = lastEntry->GetFullUrl();
                const auto &resource = lastEntry->GetResource();
                {
                    auto profiles = resource->GetProfile();
                    if (!profiles.empty()) {
                        medicationType = profiles[0];
                    }
                }
                medicationDisplay = resource->GetDisplay();
            }
            {
                FhirReference medicationReference{medicationRef, medicationType, medicationDisplay};
                medicationStatementResource->SetMedicationReference(medicationReference);
            }
            {
                auto patientResource = patientEntry.GetResource();
                FhirReference subjectReference{patientEntry.GetFullUrl(), "http://ehelse.no/fhir/StructureDefinition/sfm-Patient", patientResource->GetDisplay()};
                medicationStatementResource->SetSubject(subjectReference);
            }
            medicationStatementEntries.emplace_back(medicationStatement);
            medicationSectionEntries.emplace_back(medicationStatementRef, "http://ehelse.no/fhir/StructureDefinition/sfm-MedicationStatement", medicationStatementDisplay);
        }
    }
    FhirCompositionSection medicationSection{};
    medicationSection.SetTitle("Medication");
    medicationSection.SetCode(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-section-types", "sectionMedication", "List of Medication statements"));
    medicationSection.SetTextStatus("generated");
    medicationSection.SetTextXhtml("<xhtml:div xmlns:xhtml=\"http://www.w3.org/1999/xhtml\">List of medications</xhtml:div>");
    if (medicationSectionEntries.empty()) {
        medicationSection.SetEmptyReason(
                FhirCodeableConcept("http://terminology.hl7.org/CodeSystem/list-empty-reason", "unavailable",
                                    "Unavailable"));
    } else {
        medicationSection.SetEntries(medicationSectionEntries);
    }
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
    bundle->AddEntry(organizationEntry);
    bundle->AddEntry(patientEntry);
    for (const auto &entry : practitioners) {
        bundle->AddEntry(entry);
    }
    for (const auto &entry : medicamentEntries) {
        bundle->AddEntry(entry);
    }
    for (const auto &entry : medicationStatementEntries) {
        bundle->AddEntry(entry);
    }
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
    std::string patientId{};
    std::vector<Prescription> prescriptions{};
    {
        std::vector<std::shared_ptr<FhirMedicationStatement>> medicationStatements{};
        {
            std::shared_ptr<FhirComposition> composition{};
            for (const auto &entry: bundle.GetEntries()) {
                auto compositionResource = std::dynamic_pointer_cast<FhirComposition>(entry.GetResource());
                if (compositionResource) {
                    if (composition) {
                        // TODO
                        return {};
                    }
                    composition = compositionResource;
                }
            }
            if (!composition) {
                // TODO
                return {};
            }
            FhirCompositionSection medicationSection{};
            {
                bool found{false};
                for (const auto &section: composition->GetSections()) {
                    bool isMedication{false};
                    for (const auto &coding: section.GetCode().GetCoding()) {
                        if (coding.GetCode() == "sectionMedication") {
                            isMedication = true;
                            break;
                        }
                    }
                    if (isMedication) {
                        if (found) {
                            // TODO
                            return {};
                        }
                        medicationSection = section;
                        found = true;
                    }
                }
                if (!found) {
                    // TODO
                    return {};
                }
            }
            {
                std::vector<std::string> medicationStatementReferences{};
                for (const auto &entryReference: medicationSection.GetEntries()) {
                    medicationStatementReferences.emplace_back(entryReference.GetReference());
                }
                for (const auto &entry: bundle.GetEntries()) {
                    auto fullUrl = entry.GetFullUrl();
                    auto iterator = std::find(medicationStatementReferences.begin(),
                                              medicationStatementReferences.end(),
                                              fullUrl);
                    if (iterator != medicationStatementReferences.end()) {
                        auto medStatement = std::dynamic_pointer_cast<FhirMedicationStatement>(entry.GetResource());
                        if (medStatement) {
                            medicationStatements.emplace_back(medStatement);
                        }
                    }
                }
            }
            std::vector<Prescription> prescriptions{};
            auto medicationStatementIterator = medicationStatements.begin();
            while (medicationStatementIterator != medicationStatements.end()) {
                auto medicationStatement = *medicationStatementIterator;
                bool createeresept{false};
                for (const auto &extension: medicationStatement->GetExtensions()) {
                    if (extension->GetUrl() == "http://ehelse.no/fhir/StructureDefinition/sfm-reseptamendment") {
                        for (const auto &subExtension: extension->GetExtensions()) {
                            if (subExtension->GetUrl() == "createeresept") {
                                auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(subExtension);
                                if (valueExtension) {
                                    auto value = std::dynamic_pointer_cast<FhirBooleanValue>(
                                            valueExtension->GetValue());
                                    if (value) {
                                        createeresept = value->IsTrue();
                                    }
                                }
                            }
                        }
                    }
                }
                if (createeresept) {
                    ++medicationStatementIterator;
                } else {
                    medicationStatementIterator = medicationStatements.erase(medicationStatementIterator);
                }
            }
        }
        CreatePrescriptionService createPrescriptionService{};
        for (const auto &medicationStatement : medicationStatements) {
            Prescription prescription = createPrescriptionService.CreatePrescription(medicationStatement, bundle);
            patientId = prescription.GetPatient();
            prescriptions.emplace_back(prescription);
        }
        if (!prescriptions.empty() && patientId.empty()) {
            // TODO
            return {};
        }
        for (const auto &prescription : prescriptions) {
            if (prescription.GetPatient() != patientId) {
                // TODO
                return {};
            }
        }
    }
    int prescriptionCount{0};
    if (!patientId.empty()) {
        PrescriptionStorage prescriptionStorage{};
        auto list = prescriptionStorage.LoadPatientMap(patientId);
        for (const auto &prescription : prescriptions) {
            auto id = prescriptionStorage.Store(patientId, prescription);
            list.emplace_back(id);
            ++prescriptionCount;
        }
        prescriptionStorage.StorePatientMap(patientId, list);
    }
    FhirParameters parameters{};
    parameters.AddParameter("recallCount", std::make_shared<FhirIntegerValue>(0));
    parameters.AddParameter("prescriptionCount", std::make_shared<FhirIntegerValue>(prescriptionCount));
    return parameters;
}
