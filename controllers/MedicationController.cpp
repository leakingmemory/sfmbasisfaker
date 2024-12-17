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
#include <sfmbasisapi/fhir/fhirbasic.h>
#include <sfmbasisapi/fhir/operationoutcome.h>
#include <sfmbasisapi/fhir/allergy.h>
#include <sfmbasisapi/IsoDateTime.h>
#include "../domain/person.h"
#include "../domain/prescription.h"
#include "../service/PersonStorage.h"
#include "../service/CreatePrescriptionService.h"
#include "../service/PrescriptionStorage.h"
#include "../service/PllStorage.h"
#include "../service/DateTime.h"
#include <map>

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
    std::map<std::string,std::shared_ptr<FhirMedicationStatement>> urlToMedicationStatementMap{};
    std::map<std::string,FhirReference> urlToStatementReferenceMap{};
    std::map<std::string,std::string> prescriptionIdToUrlMap{};
    std::map<std::string,std::string> prescriptionIdToPreviousIdMap{};
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
            urlToMedicationStatementMap.insert_or_assign(medicationStatementRef,medicationStatementResource);
            std::string medicationStatementDisplay = medicationStatementResource->GetDisplay();
            FhirReference medicationStatementReferenceObject{medicationStatementRef, "http://ehelse.no/fhir/StructureDefinition/sfm-MedicationStatement", medicationStatementDisplay};
            urlToStatementReferenceMap.insert_or_assign(medicationStatementRef,medicationStatementReferenceObject);
            auto prescriptionId = prescription.GetId();
            if (!prescriptionId.empty()) {
                prescriptionIdToUrlMap.insert_or_assign(prescriptionId,medicationStatementRef);
            }
            auto previousPrescriptionId = prescription.GetPreviousId();
            if (!previousPrescriptionId.empty()) {
                prescriptionIdToPreviousIdMap.insert_or_assign(prescriptionId,previousPrescriptionId);
            }
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
            medicationSectionEntries.emplace_back(medicationStatementReferenceObject);
        }
    }
    {
        std::vector<std::string> urlRefsToRemove{};
        for (const auto &prevMapEntry : prescriptionIdToPreviousIdMap) {
            auto prescriptionId = prevMapEntry.first;
            auto prevId = prevMapEntry.second;
            auto prevUrl = prescriptionIdToUrlMap.find(prevId);
            if (prevUrl == prescriptionIdToUrlMap.end()) {
                continue;
            }
            urlRefsToRemove.emplace_back(prevUrl->second);
            auto prescriptionUrl = prescriptionIdToUrlMap.find(prescriptionId);
            if (prescriptionUrl == prescriptionIdToUrlMap.end()) {
                continue;
            }
            auto prevStatement = urlToMedicationStatementMap.find(prevUrl->second);
            if (prevStatement == urlToMedicationStatementMap.end()) {
                continue;
            }
            auto statement = urlToMedicationStatementMap.find(prescriptionUrl->second);
            if (statement == urlToMedicationStatementMap.end()) {
                continue;
            }
            auto prevRef = urlToStatementReferenceMap.find(prevUrl->second);
            if (prevRef != urlToStatementReferenceMap.end()) {
                statement->second->SetBasedOn({prevRef->second});
            }
            auto nextRef = urlToStatementReferenceMap.find(prescriptionUrl->second);
            if (nextRef != urlToStatementReferenceMap.end()) {
                prevStatement->second->SetPartOf({nextRef->second});
            }
        }
        auto iterator = medicationSectionEntries.begin();
        while (iterator != medicationSectionEntries.end()) {
            auto ref = iterator->GetReference();
            if (std::find(urlRefsToRemove.cbegin(), urlRefsToRemove.cend(), ref) == urlRefsToRemove.cend()) {
                ++iterator;
            } else {
                iterator = medicationSectionEntries.erase(iterator);
            }
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
    std::vector<FhirBundleEntry> allergyEntries{};
    std::vector<FhirReference> allergyReferences{};
    std::shared_ptr<FhirBundleEntry> m251Entry{};
    {
        PllStorage pllStorage{};
        auto pll = pllStorage.Load(patient.GetId());
        if (pll.IsValid()) {
            for (auto &pllAllergy : pll.GetAllergies()) {
                std::string url{"urn:uuid:"};
                {
                    boost::uuids::uuid uuid = boost::uuids::random_generator()();
                    std::string uuid_string = to_string(uuid);
                    url.append(uuid_string);
                }
                {
                    boost::uuids::uuid uuid = boost::uuids::random_generator()();
                    std::string uuid_string = to_string(uuid);
                    pllAllergy.SetId(uuid_string);
                }
                auto fhirAllergy = pllAllergy.ToFhir(practitioners);
                auto &bundleEntry = allergyEntries.emplace_back(url, fhirAllergy);
                allergyReferences.emplace_back(url, "http://nhn.no/kj/fhir/StructureDefinition/KjAllergyIntolerance", bundleEntry.GetResource()->GetDisplay());
                fhirAllergy->SetPatient(patientEntry.CreateReference("http://ehelse.no/fhir/StructureDefinition/sfm-Patient"));
            }
            m251Entry = std::make_shared<FhirBundleEntry>();
            std::string url{"urn:uuid:"};
            {
                boost::uuids::uuid uuid = boost::uuids::random_generator()();
                std::string uuid_string = to_string(uuid);
                url.append(uuid_string);
            }
            *m251Entry = FhirBundleEntry::Create<FhirBasic>(url);
            auto m251 = std::dynamic_pointer_cast<FhirBasic>(m251Entry->GetResource());
            m251->SetCode(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-message-type", "M25.1", "PLL"));
            {
                boost::uuids::uuid uuid = boost::uuids::random_generator()();
                std::string uuid_string = to_string(uuid);
                m251->SetId(uuid_string);
            }
            std::vector<FhirIdentifier> identifiers{};
            identifiers.emplace_back("usual", pll.GetId());
            m251->SetIdentifiers(identifiers);
            m251->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-PLL-info");
            m251->SetSubject(patientEntry.CreateReference("http://ehelse.no/fhir/StructureDefinition/sfm-Patient"));
        }
    }
    FhirCompositionSection pllInfoSection{};
    pllInfoSection.SetTitle("PllInfo");
    pllInfoSection.SetCode(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-section-types", "sectionPLLinfo", "PLL Info"));
    std::vector<FhirReference> pllEntries{};
    if (m251Entry) {
        pllEntries.emplace_back(m251Entry->CreateReference("http://ehelse.no/fhir/StructureDefinition/sfm-PLL-info"));
    }
    if (!pllEntries.empty()) {
        pllInfoSection.SetEntries(pllEntries);
    } else {
        pllInfoSection.SetEmptyReason(
                FhirCodeableConcept("http://terminology.hl7.org/CodeSystem/list-empty-reason", "unavailable",
                                    "Unavailable"));
        pllInfoSection.SetTextXhtml("<xhtml:div xmlns:xhtml=\"http://www.w3.org/1999/xhtml\"></xhtml:div>");
    }
    pllInfoSection.SetTextStatus("generated");
    FhirCompositionSection allergiesSection{};
    allergiesSection.SetTitle("Allergies");
    allergiesSection.SetCode(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-section-types", "sectionAllergies", "Section allergies"));
    allergiesSection.SetTextStatus("generated");
    if (allergyReferences.empty()) {
        allergiesSection.SetTextXhtml("<xhtml:div xmlns:xhtml=\"http://www.w3.org/1999/xhtml\"></xhtml:div>");
        allergiesSection.SetEmptyReason(
                FhirCodeableConcept("http://terminology.hl7.org/CodeSystem/list-empty-reason", "unavailable",
                                    "Unavailable"));
    } else {
        allergiesSection.SetTextXhtml("<xhtml:div xmlns:xhtml=\"http://www.w3.org/1999/xhtml\">List of allergies</xhtml:div>");
        allergiesSection.SetEntries(allergyReferences);
    }
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
        resource.AddSection(pllInfoSection);
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
    for (const auto &entry : allergyEntries) {
        bundle->AddEntry(entry);
    }
    if (m251Entry) {
        bundle->AddEntry(*m251Entry);
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

FhirIssue CreateIssue(std::string code, std::string diagnostics, std::string severity) {
    FhirIssue issue{};
    issue.SetCode(code);
    issue.SetDiagnostics(diagnostics);
    issue.SetSeverity(severity);
    return issue;
}

std::vector<FhirIssue> CreateIssues(const FhirIssue &issue) {
    std::vector<FhirIssue> issues{};
    issues.emplace_back(issue);
    return issues;
}

static std::shared_ptr<FhirOperationOutcome> CreateOperationOutcome(const std::vector<FhirIssue> &issues) {
    auto op = std::make_shared<FhirOperationOutcome>();
    op->SetIssue(issues);
    return op;
}

struct PllCessation {
    Code reason{};
    std::string timeDate{};
};

struct PllEntryData {
    std::string effectiveDate{};
};

std::shared_ptr<Fhir> MedicationController::SendMedication(const FhirBundle &bundle) {
    bool createPll{false};
    std::string patientId{};
    std::vector<std::string> ePrescriptionIds{};
    std::vector<std::string> injectedPllIds{};
    std::vector<Prescription> prescriptions{};
    std::vector<PllAllergy> allergies{};
    std::map<std::string,Code> potentialRecalls{};
    std::map<std::string,PllCessation> potentialPllCessations{};
    std::map<std::string,PllEntryData> pllEntryDataMap{};
    std::map<std::string,std::string> toPreviousPrescription{};
    std::map<std::string,std::string> pllToPrescriptionIdMap{};
    {
        std::vector<std::shared_ptr<FhirMedicationStatement>> medicationStatements{};
        std::map<std::string,std::shared_ptr<FhirBasic>> fhirBasics{};
        std::map<std::string,std::shared_ptr<FhirAllergyIntolerance>> fhirAllergies{};
        std::map<std::string,std::tuple<Code,std::shared_ptr<FhirMedicationStatement>>> potentialRecallsWithStatements{};
        CreatePrescriptionService createPrescriptionService{};
        {
            std::shared_ptr<FhirComposition> composition{};
            for (const auto &entry: bundle.GetEntries()) {
                auto compositionResource = std::dynamic_pointer_cast<FhirComposition>(entry.GetResource());
                if (compositionResource) {
                    if (composition) {
                        return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Multiple composition entries", "Fatal")));
                    }
                    composition = compositionResource;
                    continue;
                }
                auto basic = std::dynamic_pointer_cast<FhirBasic>(entry.GetResource());
                if (basic) {
                    fhirBasics.insert_or_assign(entry.GetFullUrl(), basic);
                    continue;
                }
                auto allergy = std::dynamic_pointer_cast<FhirAllergyIntolerance>(entry.GetResource());
                if (allergy) {
                    fhirAllergies.insert_or_assign(entry.GetFullUrl(), allergy);
                }
            }
            if (!composition) {
                return CreateOperationOutcome(CreateIssues(CreateIssue("X", "No composition entries", "Fatal")));
            }
            FhirCompositionSection medicationSection{};
            FhirCompositionSection pllSection{};
            FhirCompositionSection allergySection{};
            {
                bool foundMedication{false};
                bool foundPll{false};
                bool foundAllergies{false};
                for (const auto &section: composition->GetSections()) {
                    bool isMedication{false};
                    bool isPllInfo{false};
                    bool isAllergies{false};
                    for (const auto &coding: section.GetCode().GetCoding()) {
                        auto code = coding.GetCode();
                        std::transform(code.cbegin(), code.cend(), code.begin(), [] (char ch) { return std::tolower(ch); });
                        if (code == "sectionmedication") {
                            isMedication = true;
                            break;
                        }
                        if (code == "sectionpllinfo") {
                            isPllInfo = true;
                            break;
                        }
                        if (code == "sectionallergies") {
                            isAllergies = true;
                            break;
                        }
                    }
                    if (isMedication) {
                        if (isPllInfo) {
                            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Ambiguous section (med & pll)", "Fatal")));
                        }
                        if (foundMedication) {
                            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Multiple medication sections", "Fatal")));
                        }
                        medicationSection = section;
                        foundMedication = true;
                    }
                    if (isPllInfo) {
                        if (foundPll) {
                            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Multiple PLL info sections", "Fatal")));
                        }
                        pllSection = section;
                        foundPll = true;
                    }
                    if (isAllergies) {
                        if (foundAllergies) {
                            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Multiple allergies sections", "Fatal")));
                        }
                        allergySection = section;
                        foundAllergies = true;
                    }
                }
                if (!foundMedication) {
                    return CreateOperationOutcome(CreateIssues(CreateIssue("X", "No medication section", "Fatal")));
                }
                if (!foundPll) {
                    return CreateOperationOutcome(CreateIssues(CreateIssue("X", "No PLL info section", "Fatal")));
                }
                if (!foundAllergies) {
                    return CreateOperationOutcome(CreateIssues(CreateIssue("X", "No allergy section", "Fatal")));
                }
            }
            std::shared_ptr<FhirBasic> m251{};
            for (const auto &m25ref : pllSection.GetEntries()) {
                auto display = m25ref.GetDisplay();
                std::transform(display.cbegin(), display.cend(), display.begin(), [] (char ch) { return std::tolower(ch); });
                if (display == "m25.1") {
                    auto m251Iterator = fhirBasics.find(m25ref.GetReference());
                    if (m251Iterator != fhirBasics.end()) {
                        m251 = m251Iterator->second;
                    }
                }
            }
            if (!m251 && !pllSection.GetEntries().empty()) {
                auto m251Iterator = fhirBasics.find(pllSection.GetEntries()[0].GetReference());
                if (m251Iterator != fhirBasics.end()) {
                    m251 = m251Iterator->second;
                }
            }
            if (m251) {
                auto person = createPrescriptionService.GetPerson(bundle, m251->GetSubject().GetReference());
                {
                    auto personId = person.GetId();
                    if (!personId.empty()) {
                        if (patientId.empty()) {
                            patientId = personId;
                        } else if (patientId != personId) {
                            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Different patient", "Fatal")));
                        }
                    }
                }
                auto m251exts = m251->GetExtensions();
                for (const auto &m251ext : m251exts) {
                    auto url = m251ext->GetUrl();
                    std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) {return std::tolower(ch);});
                    if (url == "http://ehelse.no/fhir/structuredefinition/sfm-pllinformation") {
                        auto pllInfoExts = m251ext->GetExtensions();
                        for (const auto &pllInfoExt : pllInfoExts) {
                            auto url = pllInfoExt->GetUrl();
                            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) {return std::tolower(ch);});
                            if (url == "createpll") {
                                auto createPllExt = std::dynamic_pointer_cast<FhirValueExtension>(pllInfoExt);
                                if (createPllExt) {
                                    auto value = std::dynamic_pointer_cast<FhirBooleanValue>(createPllExt->GetValue());
                                    createPll = value && value->IsTrue();
                                }
                            }
                        }
                    }
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
            auto medicationStatementIterator = medicationStatements.begin();
            while (medicationStatementIterator != medicationStatements.end()) {
                auto medicationStatement = *medicationStatementIterator;
                bool createeresept{false};
                std::string prescriptionId{};
                std::string pllId{};
                for (const auto &identifier : medicationStatement->GetIdentifiers()) {
                    auto type = identifier.GetType().GetText();
                    std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) { return std::tolower(ch); });
                    if (type == "reseptid") {
                        prescriptionId = identifier.GetValue();
                    } else if (type == "pll") {
                        pllId = identifier.GetValue();
                    }
                }
                if (createPll && pllId.empty()) {
                    boost::uuids::uuid uuid = boost::uuids::random_generator()();
                    pllId = to_string(uuid);
                    auto identifiers = medicationStatement->GetIdentifiers();
                    identifiers.emplace_back(FhirCodeableConcept("pll"), "official", pllId);
                    medicationStatement->SetIdentifiers(identifiers);
                }
                if (createPll && !prescriptionId.empty() && !pllId.empty()) {
                    pllToPrescriptionIdMap.insert_or_assign(pllId, prescriptionId);
                }
                PllCessation pllCessation{};
                PllEntryData pllEntryData{};
                for (const auto &extension: medicationStatement->GetExtensions()) {
                    auto url = extension->GetUrl();
                    std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) { return std::tolower(ch); });
                    if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
                        for (const auto &subExtension: extension->GetExtensions()) {
                            auto url = subExtension->GetUrl();
                            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) { return std::tolower(ch); });
                            if (url == "createeresept") {
                                auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(subExtension);
                                if (valueExtension) {
                                    auto value = std::dynamic_pointer_cast<FhirBooleanValue>(
                                            valueExtension->GetValue());
                                    if (value) {
                                        createeresept = value->IsTrue();
                                    }
                                }
                            } else if (url == "recallinfo") {
                                std::string recallId{};
                                std::string recallCode{};
                                std::string recallDisplay{};
                                std::string recallSystem{};
                                auto extensions = subExtension->GetExtensions();
                                for (const auto &extension : extensions) {
                                    std::string url = extension->GetUrl();
                                    std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) { return std::tolower(ch); });
                                    if (url == "recallcode") {
                                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                                        if (valueExtension) {
                                            auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                                            if (value) {
                                                auto coding = value->GetCoding();
                                                if (!coding.empty()) {
                                                    std::string codeValue{coding[0].GetCode()};
                                                    if (!codeValue.empty()) {
                                                        recallCode = codeValue;
                                                        recallDisplay = coding[0].GetDisplay();
                                                        recallSystem = coding[0].GetSystem();
                                                    }
                                                }
                                            }
                                        }
                                    } else if (url == "recallid") {
                                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                                        if (valueExtension) {
                                            auto value = std::dynamic_pointer_cast<FhirString>(valueExtension->GetValue());
                                            if (value) {
                                                std::string str{value->GetValue()};
                                                if (!str.empty()) {
                                                    recallId = str;
                                                }
                                            }
                                        }
                                    }
                                }
                                if (recallId.empty()) {
                                    recallId = prescriptionId;
                                }
                                if (!recallCode.empty() && !recallId.empty()) {
                                    std::tuple<Code,std::shared_ptr<FhirMedicationStatement>> codeTuple =
                                            {{recallCode, recallDisplay, recallSystem}, medicationStatement};
                                    potentialRecallsWithStatements.insert_or_assign(recallId, codeTuple);
                                    if ((recallCode == "1" || recallCode == "3") && recallId != prescriptionId) {
                                        toPreviousPrescription.insert_or_assign(prescriptionId, recallId);
                                    }
                                }
                            }
                        }
                    } else if (url == "http://ehelse.no/fhir/structuredefinition/sfm-discontinuation") {
                        std::string timeDate{};
                        FhirCodeableConcept reason{};
                        std::string note{};
                        for (const auto &subExtension: extension->GetExtensions()) {
                            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(subExtension);
                            if (!valueExtension) {
                                continue;
                            }
                            auto url = subExtension->GetUrl();
                            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) { return std::tolower(ch); });
                            if (url == "timedate") {
                                auto value = std::dynamic_pointer_cast<FhirDateTimeValue>(valueExtension->GetValue());
                                if (value) {
                                    timeDate = value->GetDateTime();
                                }
                            } else if (url == "reason") {
                                auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                                if (value) {
                                    reason = *value;
                                }
                            } else if (url == "note") {
                                auto value = std::dynamic_pointer_cast<FhirString>(valueExtension->GetValue());
                                if (value) {
                                    note = value->GetValue();
                                }
                            }
                        }
                        if (timeDate.empty()) {
                            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Discontinuation without timedate", "Fatal")));
                        }
                        pllCessation.timeDate = timeDate;
                        auto codings = reason.GetCoding();
                        if (!codings.empty()) {
                            if (!note.empty()) {
                                return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Either reason or note for discontinuation", "Fatal")));
                            }
                            if (codings.size() != 1) {
                                return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Multiple reason codes for discontinuation", "Fatal")));
                            }
                            auto coding = codings[0];
                            if (coding.GetCode().empty()) {
                                return CreateOperationOutcome(CreateIssues(CreateIssue("X", "No reason code supplied for discontinuation", "Fatal")));
                            }
                            if (!reason.GetText().empty()) {
                                return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Reason text should not be set for discontinuation", "Fatal")));
                            }
                            pllCessation.reason.setSystem(coding.GetSystem());
                            pllCessation.reason.setCode(coding.GetCode());
                            pllCessation.reason.setDisplay(coding.GetDisplay());
                        } else {
                            if (note.empty()) {
                                return CreateOperationOutcome(CreateIssues(
                                        CreateIssue("X", "No reason or note supplied for discontinuation", "Fatal")));
                            }
                            pllCessation.reason.setDisplay(note);
                        }
                    } else if (url == "effectivedatetime") {
                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                        if (valueExtension) {
                            auto value = std::dynamic_pointer_cast<FhirDateValue>(valueExtension->GetValue());
                            if (value) {
                                pllEntryData.effectiveDate = value->GetRawValue();
                            }
                        }
                    }
                }
                if (!pllId.empty()) {
                    if (!pllCessation.timeDate.empty()) {
                        potentialPllCessations.insert_or_assign(pllId, pllCessation);
                    }
                    pllEntryDataMap.insert_or_assign(pllId, pllEntryData);
                }
                if (createeresept) {
                    ++medicationStatementIterator;
                } else {
                    if (!prescriptionId.empty()) {
                        if (pllId.empty()) {
                            ePrescriptionIds.emplace_back(prescriptionId);
                        }
                    }
                    medicationStatementIterator = medicationStatements.erase(medicationStatementIterator);
                }
            }
            for (auto &allergyReference : allergySection.GetEntries()) {
                auto iterator = fhirAllergies.find(allergyReference.GetReference());
                if (iterator != fhirAllergies.end()) {
                    allergies.emplace_back(PllAllergy::FromFhir(*(iterator->second), bundle));
                }
            }
        }
        for (const auto &medicationStatement : medicationStatements) {
            Prescription prescription = createPrescriptionService.CreatePrescription(medicationStatement, bundle);
            {
                auto pllId = prescription.GetPllId();
                if (!pllId.empty()) {
                    injectedPllIds.emplace_back(pllId);
                }
            }
            patientId = prescription.GetPatient();
            if (prescription.GetTypeOfPrescription().getCode() != "E") {
                boost::uuids::uuid uuid = boost::uuids::random_generator()();
                std::string uuid_string = to_string(uuid);
                prescription.SetId(uuid_string);
                prescription.SetRfStatus({});
            }
            prescriptions.emplace_back(prescription);
        }
        for (const auto &recallTuple : potentialRecallsWithStatements) {
            auto prescriptionId = recallTuple.first;
            auto code = std::get<0>(recallTuple.second);
            auto medicationStatement = std::get<1>(recallTuple.second);
            auto person = createPrescriptionService.GetPerson(bundle, medicationStatement->GetSubject().GetReference());
            patientId = person.GetId();
            potentialRecalls.insert_or_assign(prescriptionId, code);
        }
        if (!prescriptions.empty() && patientId.empty()) {
            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "No patient", "Fatal")));
        }
        for (const auto &prescription : prescriptions) {
            if (prescription.GetPatient() != patientId) {
                return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Different patient", "Fatal")));
            }
        }
    }
    int prescriptionCount{0};
    int recallCount{0};
    Pll pll{};
    std::vector<std::vector<std::shared_ptr<FhirParameter>>> prescriptionOperationResult{};
    if (!patientId.empty()) {
        PrescriptionStorage prescriptionStorage{};
        auto list = prescriptionStorage.LoadPatientMap(patientId);
        std::map<std::string,std::shared_ptr<Prescription>> filePrescriptionMap{};
        std::vector<std::string> clearPllIdForMove{};
        for (const auto &fileId : list) {
            auto prescription = std::make_shared<Prescription>(prescriptionStorage.Load(patientId, fileId));
            filePrescriptionMap.insert_or_assign(fileId, prescription);
            auto pllId = prescription->GetPllId();
            auto prescriptionId = prescription->GetId();
            for (auto &prescription : prescriptions) {
                auto id = prescription.GetId();
                if (id == prescriptionId) {
                    return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Create prescription without renew", "Fatal")));
                }
                auto prevPresIdIter = toPreviousPrescription.find(id);
            }
            if (createPll && !prescriptionId.empty() && !pllId.empty()) {
                if (std::find(ePrescriptionIds.cbegin(), ePrescriptionIds.cend(), prescriptionId) != ePrescriptionIds.cend() &&
                    std::find(injectedPllIds.cbegin(), injectedPllIds.cend(), pllId) != injectedPllIds.cend()) {
                    return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Existing and kept PLL ID reused on a new prescription", "Fatal")));
                }
                auto iterator = pllToPrescriptionIdMap.find(pllId);
                if (iterator != pllToPrescriptionIdMap.end() && iterator->second != prescriptionId) {
                    clearPllIdForMove.emplace_back(pllId);
                    if (prescription->GetRfStatus().getCode() == "E") {
                        return CreateOperationOutcome(CreateIssues(CreateIssue("X", "PLL is already containing a prescription with rfstatus E", "Fatal")));
                    }
                }
            }
            if (potentialRecalls.find(prescriptionId) != potentialRecalls.end()) {
                auto rfStatus = prescription->GetRfStatus().getCode();
                if (rfStatus != "E" && rfStatus != "U") {
                    return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Recalling inactive prescription", "Fatal")));
                }
            }
        }
        for (auto &prescription : prescriptions) {
            auto id = prescription.GetId();
            if (id.empty()) {
                if (!createPll || prescription.GetPllId().empty()) {
                    return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Requested create prescription without ID", "Fatal")));
                }
                return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Without prescription is not yet supported", "Fatal")));
            }
            auto prevPresIdIter = toPreviousPrescription.find(id);
            if (prevPresIdIter != toPreviousPrescription.end()) {
                prescription.SetPreviousId(prevPresIdIter->second);
            }
        }
        if (createPll) {
            for (const auto &filePrescriptionMapping: filePrescriptionMap) {
                auto fileId = filePrescriptionMapping.first;
                auto prescription = filePrescriptionMapping.second;
                auto pllId = prescription->GetPllId();
                if (!pllId.empty()) {
                    if (std::find(injectedPllIds.cbegin(), injectedPllIds.cend(), pllId) != injectedPllIds.cend()) {
                        prescription->SetPllId("");
                        prescriptionStorage.Replace(patientId, fileId, *prescription);
                    } else if (std::find(clearPllIdForMove.cbegin(), clearPllIdForMove.cend(), pllId) != clearPllIdForMove.cend()) {
                        prescription->SetPllId("");
                        prescriptionStorage.Replace(patientId, fileId, *prescription);
                    } else if (prescription->GetCessationTime().empty()) {
                        auto maybeCeased = potentialPllCessations.find(pllId);
                        if (maybeCeased != potentialPllCessations.end()) {
                            prescription->SetCessationReason(maybeCeased->second.reason);
                            prescription->SetCessationTime(maybeCeased->second.timeDate);
                            prescriptionStorage.Replace(patientId, fileId, *prescription);
                        }
                        auto maybePllData = pllEntryDataMap.find(pllId);
                        if (maybePllData != pllEntryDataMap.end()) {
                            prescription->SetTreatmentStartedTimestamp(maybePllData->second.effectiveDate);
                            prescriptionStorage.Replace(patientId, fileId, *prescription);
                        }
                    }
                }
            }
        }
        for (const auto &filePrescriptionMapping : filePrescriptionMap) {
            auto fileId = filePrescriptionMapping.first;
            auto prescription = filePrescriptionMapping.second;
            auto prescriptionId = prescription->GetId();
            if (createPll && prescription->GetPllId().empty()) {
                auto mapping = std::find_if(pllToPrescriptionIdMap.cbegin(), pllToPrescriptionIdMap.cend(), [prescriptionId] (auto mapping) { return mapping.second == prescriptionId; });
                if (mapping != pllToPrescriptionIdMap.cend()) {
                    prescription->SetPllId(mapping->first);
                    prescriptionStorage.Replace(patientId, fileId, *prescription);
                }
                if (std::find(ePrescriptionIds.cbegin(), ePrescriptionIds.cend(), prescriptionId) !=
                    ePrescriptionIds.cend()) {
                    boost::uuids::uuid uuid = boost::uuids::random_generator()();
                    std::string uuid_string = to_string(uuid);
                    prescription->SetPllId(uuid_string);
                    prescriptionStorage.Replace(patientId, fileId, *prescription);
                }
            }
            auto iterator = potentialRecalls.begin();
            while (iterator != potentialRecalls.end()) {
                auto &potentialRecall = *iterator;
                if (potentialRecall.first == prescriptionId) {
                    auto rfStatusCode = prescription->GetRfStatus().getCode();
                    if (rfStatusCode == "E" || rfStatusCode == "U") {
                        prescription->SetRecallCode(potentialRecall.second);
                        prescription->SetRfStatus(Code("T", "Tilbakekalt", "urn:oid:2.16.578.1.12.4.1.1.7408"));
                        prescriptionStorage.Replace(patientId, fileId, *prescription);
                        std::vector<std::shared_ptr<FhirParameter>> parameters{};
                        parameters.emplace_back(std::make_shared<FhirParameter>("reseptID",
                                                                                std::make_shared<FhirString>(
                                                                                        prescription->GetId())));
                        parameters.emplace_back(std::make_shared<FhirParameter>("resultCode",
                                                                                std::make_shared<FhirCodingValue>(
                                                                                        "http://ehelse.no/fhir/CodeSystem/sfm-kj-rf-error-code",
                                                                                        "1", "Tilbakekalt")));
                        prescriptionOperationResult.emplace_back(parameters);
                        ++recallCount;
                    }
                    iterator = potentialRecalls.erase(iterator);
                    break;
                } else {
                    ++iterator;
                }
            }
        }
        for (auto &prescription : prescriptions) {
            if (createPll) {
                if (prescription.GetPllId().empty()) {
                    boost::uuids::uuid uuid = boost::uuids::random_generator()();
                    std::string uuid_string = to_string(uuid);
                    prescription.SetPllId(uuid_string);
                } else {
                    auto maybePllData = pllEntryDataMap.find(prescription.GetPllId());
                    if (maybePllData != pllEntryDataMap.end()) {
                        prescription.SetTreatmentStartedTimestamp(maybePllData->second.effectiveDate);
                    }
                }
            } else {
                prescription.SetPllId("");
            }
            auto id = prescriptionStorage.Store(patientId, prescription);
            list.emplace_back(id);
            ++prescriptionCount;
            std::vector<std::shared_ptr<FhirParameter>> parameters{};
            parameters.emplace_back(std::make_shared<FhirParameter>("reseptID", std::make_shared<FhirString>(prescription.GetId())));
            parameters.emplace_back(std::make_shared<FhirParameter>("resultCode", std::make_shared<FhirCodingValue>("http://ehelse.no/fhir/CodeSystem/sfm-kj-rf-error-code", "0", "OK")));
            prescriptionOperationResult.emplace_back(parameters);
        }
        prescriptionStorage.StorePatientMap(patientId, list);
        if (createPll) {
            PllStorage pllStorage{};
            {
                boost::uuids::uuid uuid = boost::uuids::random_generator()();
                std::string uuid_string = to_string(uuid);
                pll.SetId(uuid_string);
            }
            {
                auto now = DateTimeOffset::Now();
                pll.SetDateTime(now.to_iso8601());
            }
            pll.SetAllergies(std::move(allergies));
            pllStorage.Store(patientId, pll);
        }
    }
    auto parameters = std::make_shared<FhirParameters>();
    parameters->AddParameter("recallCount", std::make_shared<FhirIntegerValue>(recallCount));
    parameters->AddParameter("prescriptionCount", std::make_shared<FhirIntegerValue>(prescriptionCount));
    for (const auto &por : prescriptionOperationResult) {
        parameters->AddParameter("prescriptionOperationResult", por);
    }
    if (pll.IsValid()) {
        std::vector<std::shared_ptr<FhirParameter>> part{};
        part.emplace_back(std::make_shared<FhirParameter>("PllmessageID", std::make_shared<FhirString>(pll.GetId())));
        part.emplace_back(std::make_shared<FhirParameter>("resultCode", std::make_shared<FhirCodingValue>("http://ehelse.no/fhir/CodeSystem/sfm-kj-rf-error-code", "0", "OK")));
        parameters->AddParameter("PllResult", part);
    }
    return parameters;
}
