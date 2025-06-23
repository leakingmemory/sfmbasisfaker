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
#include <sfmbasisapi/fhir/meddispense.h>
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
    std::vector<FhirBundleEntry> organizations{};
    std::vector<FhirBundleEntry> practitioners{};
    practitioners.emplace_back(practitionerEntry);
    std::vector<FhirBundleEntry> medicamentEntries{};
    std::vector<FhirBundleEntry> medicationStatementEntries{};
    std::vector<FhirReference> medicationSectionEntries{};
    std::vector<FhirReference> bandaSectionEntries{};
    std::vector<FhirBundleEntry> medicationDispenseEntries{};
    std::vector<FhirReference> medicationDispenseSectionEntries{};
    std::map<std::string,std::variant<std::shared_ptr<FhirMedicationStatement>,std::shared_ptr<FhirBasic>>> urlToMedicationStatementMap{};
    std::map<std::string,FhirReference> urlToStatementReferenceMap{};
    std::map<std::string,FhirReference> urlToBandaReferenceMap{};
    std::map<std::string,std::string> prescriptionIdToUrlMap{};
    std::map<std::string,std::string> prescriptionIdToPreviousIdMap{};
    PllStorage pllStorage{};
    auto pll = pllStorage.Load(patient.GetId());
    {
        PrescriptionStorage prescriptionStorage{};
        CreatePrescriptionService createPrescriptionService{};
        std::vector<std::string> ePrescriptionIdsObserved{};
        std::vector<std::string> pllIdsObserved{};
        auto lookupList = prescriptionStorage.LoadPatientMap(patient.GetId());
        for (const auto &lookup : lookupList) {
            struct {
                Prescription prescription;
                CreatePrescriptionService &createPrescriptionService;
                std::vector<std::string> &ePrescriptionIdsObserved;
                std::vector<std::string> &pllIdsObserved;
                std::map<std::string,std::variant<std::shared_ptr<FhirMedicationStatement>,std::shared_ptr<FhirBasic>>> &urlToMedicationStatementMap;
                std::map<std::string,FhirReference> &urlToStatementReferenceMap;
                std::map<std::string,FhirReference> &urlToBandaReferenceMap;
                std::map<std::string,std::string> &prescriptionIdToUrlMap;
                std::map<std::string,std::string> &prescriptionIdToPreviousIdMap;
                std::vector<FhirBundleEntry> &medicamentEntries;
                FhirBundleEntry &patientEntry;
                std::vector<FhirBundleEntry> &medicationStatementEntries;
                std::vector<FhirReference> &medicationSectionEntries;
                std::vector<FhirReference> &bandaSectionEntries;
                std::vector<FhirBundleEntry> &practitioners;

                void operator () (const std::shared_ptr<Medication> &medication) {
                    auto medications = createPrescriptionService.CreateFhirMedication(medication);
                    if (medications.

                            empty()

                            ) {
                        return;
                    }
                    auto medicationStatement = createPrescriptionService.CreateFhirMedicationStatement(prescription,
                                                                                                       practitioners);
                    std::string medicationStatementRef = medicationStatement.GetFullUrl();
                    if (medicationStatementRef.

                            empty()

                            ) {
                        return;
                    }
                    auto medicationStatementResource = std::dynamic_pointer_cast<FhirMedicationStatement>(
                            medicationStatement.GetResource());
                    if (!medicationStatementResource) {
                        return;
                    }
                    if (!prescription.

                                    GetPllId()

                            .

                                    empty()

                            ) {
                        pllIdsObserved.emplace_back(prescription.GetPllId());
                    }
                    if (!prescription.

                                    GetId()

                            .

                                    empty()

                            ) {
                        ePrescriptionIdsObserved.emplace_back(prescription.GetId());
                    }
                    urlToMedicationStatementMap.
                            insert_or_assign(medicationStatementRef, medicationStatementResource
                    );
                    std::string medicationStatementDisplay = medicationStatementResource->GetDisplay();
                    FhirReference medicationStatementReferenceObject{medicationStatementRef,
                                                                     "http://ehelse.no/fhir/StructureDefinition/sfm-MedicationStatement",
                                                                     medicationStatementDisplay};
                    urlToStatementReferenceMap.
                            insert_or_assign(medicationStatementRef, medicationStatementReferenceObject
                    );
                    auto prescriptionId = prescription.GetId();
                    if (!prescriptionId.

                            empty()

                            ) {
                        prescriptionIdToUrlMap.insert_or_assign(prescriptionId, medicationStatementRef);
                    }
                    auto previousPrescriptionId = prescription.GetPreviousId();
                    if (!previousPrescriptionId.

                            empty()

                            ) {
                        prescriptionIdToPreviousIdMap.insert_or_assign(prescriptionId, previousPrescriptionId);
                    }
                    std::string medicationRef{};
                    std::string medicationType{};
                    std::string medicationDisplay{};
                    {
                        const FhirBundleEntry *lastEntry;
                        for (const auto &entry: medications) {
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
                        FhirReference subjectReference{patientEntry.GetFullUrl(),
                                                       "http://ehelse.no/fhir/StructureDefinition/sfm-Patient",
                                                       patientResource->GetDisplay()};
                        medicationStatementResource->SetSubject(subjectReference);
                    }
                    medicationStatementEntries.
                            emplace_back(medicationStatement);
                    medicationSectionEntries.
                            emplace_back(medicationStatementReferenceObject);
                }
                void operator() (const std::shared_ptr<Banda> &banda) {
                    if (!banda) {
                        return;
                    }
                    auto basicBundleEntry = createPrescriptionService.CreateFhirBasic(prescription,
                                                                                      practitioners);
                    std::string basicBundleEntryRef = basicBundleEntry.GetFullUrl();
                    if (basicBundleEntryRef.

                            empty()

                            ) {
                        return;
                    }
                    auto basicBundleEntryResource = std::dynamic_pointer_cast<FhirBasic>(
                            basicBundleEntry.GetResource());
                    if (!basicBundleEntryResource) {
                        return;
                    }
                    if (!prescription.

                                    GetId()

                            .

                                    empty()

                            ) {
                        ePrescriptionIdsObserved.emplace_back(prescription.GetId());
                    }
                    urlToMedicationStatementMap.
                            insert_or_assign(basicBundleEntryRef, basicBundleEntryResource
                    );
                    std::string medicationStatementDisplay = basicBundleEntryResource->GetDisplay();
                    FhirReference medicationStatementReferenceObject{basicBundleEntryRef,
                                                                     "http://ehelse.no/fhir/StructureDefinition/sfm-BandaPrescription",
                                                                     medicationStatementDisplay};
                    urlToBandaReferenceMap.
                            insert_or_assign(basicBundleEntryRef, medicationStatementReferenceObject
                    );
                    auto prescriptionId = prescription.GetId();
                    if (!prescriptionId.

                            empty()

                            ) {
                        prescriptionIdToUrlMap.insert_or_assign(prescriptionId, basicBundleEntryRef);
                    }
                    auto previousPrescriptionId = prescription.GetPreviousId();
                    if (!previousPrescriptionId.

                            empty()

                            ) {
                        prescriptionIdToPreviousIdMap.insert_or_assign(prescriptionId, previousPrescriptionId);
                    }
                    {
                        auto patientResource = patientEntry.GetResource();
                        FhirReference subjectReference{patientEntry.GetFullUrl(),
                                                       "http://ehelse.no/fhir/StructureDefinition/sfm-Patient",
                                                       patientResource->GetDisplay()};
                        basicBundleEntryResource->SetSubject(subjectReference);
                    }
                    medicationStatementEntries.
                            emplace_back(basicBundleEntry);
                    bandaSectionEntries.
                            emplace_back(medicationStatementReferenceObject);
                }
            } visitor{
                .prescription = prescriptionStorage.Load(patient.GetId(), lookup),
                .createPrescriptionService = createPrescriptionService,
                .ePrescriptionIdsObserved = ePrescriptionIdsObserved,
                .pllIdsObserved = pllIdsObserved,
                .urlToMedicationStatementMap = urlToMedicationStatementMap,
                .urlToStatementReferenceMap = urlToStatementReferenceMap,
                .urlToBandaReferenceMap = urlToBandaReferenceMap,
                .prescriptionIdToUrlMap = prescriptionIdToUrlMap,
                .prescriptionIdToPreviousIdMap = prescriptionIdToPreviousIdMap,
                .medicamentEntries = medicamentEntries,
                .patientEntry = patientEntry,
                .medicationStatementEntries = medicationStatementEntries,
                .medicationSectionEntries = medicationSectionEntries,
                .bandaSectionEntries = bandaSectionEntries,
                .practitioners = practitioners
            };
            std::visit(visitor, visitor.prescription.GetProduct());
        }
        auto pllEntryList = pll.GetPrescriptions();
        for (const auto &prescription : pllEntryList) {
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
            auto prescriptionId = prescription.GetId();
            if (!prescription.GetPllId().empty()) {
                if (std::find(pllIdsObserved.cbegin(), pllIdsObserved.cend(), prescription.GetPllId()) != pllIdsObserved.cend()) {
                    continue;
                }
                pllIdsObserved.emplace_back(prescription.GetPllId());
            }
            if (!prescriptionId.empty()) {
                if (std::find(ePrescriptionIdsObserved.cbegin(), ePrescriptionIdsObserved.cend(), prescriptionId) != ePrescriptionIdsObserved.cend()) {
                    continue;
                }
                ePrescriptionIdsObserved.emplace_back(prescription.GetId());
            }
            urlToMedicationStatementMap.insert_or_assign(medicationStatementRef,medicationStatementResource);
            std::string medicationStatementDisplay = medicationStatementResource->GetDisplay();
            FhirReference medicationStatementReferenceObject{medicationStatementRef, "http://ehelse.no/fhir/StructureDefinition/sfm-MedicationStatement", medicationStatementDisplay};
            urlToStatementReferenceMap.insert_or_assign(medicationStatementRef,medicationStatementReferenceObject);
            if (!prescriptionId.empty()) {
                prescriptionIdToUrlMap.insert_or_assign(prescriptionId,medicationStatementRef);
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
        auto paperDispatchList = prescriptionStorage.LoadPaperDispatchMap(patient.GetId());
        for (const auto &paperDispatchId : paperDispatchList) {
            auto paperDispatch = prescriptionStorage.LoadPaperDispatch(patient.GetId(), paperDispatchId);
            auto medications = createPrescriptionService.CreateFhirMedication(paperDispatch.GetMedication());
            if (medications.empty()) {
                continue;
            }
            auto medicationStatement = createPrescriptionService.CreateFhirMedicationStatement(paperDispatch, practitioners);
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
            FhirReference medicationReference{medicationRef, medicationType, medicationDisplay};
            medicationStatementResource->SetMedicationReference(medicationReference);
            {
                auto patientResource = patientEntry.GetResource();
                FhirReference subjectReference{patientEntry.GetFullUrl(), "http://ehelse.no/fhir/StructureDefinition/sfm-Patient", patientResource->GetDisplay()};
                medicationStatementResource->SetSubject(subjectReference);
            }
            medicationStatementEntries.emplace_back(medicationStatement);
            medicationSectionEntries.emplace_back(medicationStatementReferenceObject);
            FhirBundleEntry organizationEntry;
            {
                auto organization = std::make_shared<FhirOrganization>();
                {
                    boost::uuids::uuid uuid = boost::uuids::random_generator()();
                    std::string uuid_string = to_string(uuid);
                    organization->SetId(uuid_string);
                }
                organization->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Organization");
                organization->SetIdentifiers({FhirIdentifier("official", "urn:oid:2.16.578.1.12.4.1.2",
                                                             paperDispatch.GetDispatcherHerId())});
                organization->SetName(paperDispatch.GetDispatcherName());
                std::string fullUrl{"urn:uuid:"};
                fullUrl.append(organization->GetId());
                organizationEntry = {fullUrl, organization};
            }
            organizations.emplace_back(organizationEntry);
            FhirBundleEntry dispatchEntry;
            {
                auto dispatch = std::make_shared<FhirMedicationDispense>();
                {
                    boost::uuids::uuid uuid = boost::uuids::random_generator()();
                    std::string uuid_string = to_string(uuid);
                    dispatch->SetId(uuid_string);
                }
                dispatch->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-MedicationDispense");
                dispatch->SetMedicationReference(medicationReference);
                {
                    auto dispatchInfo = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-dispenseinfo");
                    dispatchInfo->AddExtension(std::make_shared<FhirValueExtension>("cancellation", std::make_shared<FhirBooleanValue>(false)));
                    dispatchInfo->AddExtension(std::make_shared<FhirValueExtension>("concluded", std::make_shared<FhirBooleanValue>(true)));
                    dispatchInfo->AddExtension(std::make_shared<FhirValueExtension>("substitutionreservationcustomer", std::make_shared<FhirBooleanValue>(paperDispatch.IsSubstitutionReservationCustomer())));
                    dispatchInfo->AddExtension(std::make_shared<FhirValueExtension>("prescriptionid", std::make_shared<FhirString>(paperDispatch.GetId())));
                    dispatch->AddExtension(dispatchInfo);
                }
                dispatch->SetIdentifiers({FhirIdentifier("official", "M10id", paperDispatch.GetDispatchMsgId())});
                dispatch->SetStatus(FhirStatus::COMPLETED);
                {
                    auto patientResource = patientEntry.GetResource();
                    FhirReference subjectReference{patientEntry.GetFullUrl(), "http://ehelse.no/fhir/StructureDefinition/sfm-Patient", patientResource->GetDisplay()};
                    dispatch->SetSubject(subjectReference);
                }
                dispatch->SetPerformer({{.actor = {organizationEntry.GetFullUrl(), "http://ehelse.no/fhir/StructureDefinition/sfm-Organization", organizationEntry.GetResource()->GetDisplay()}}});
                dispatch->SetAuthorizingPrescription({medicationStatementReferenceObject});
                dispatch->SetQuantity(FhirQuantity(paperDispatch.GetQuantity(), ""));
                dispatch->SetDosageInstruction({FhirDosage(paperDispatch.GetDssn(), 0)});
                dispatch->SetWhenHandedOver(paperDispatch.GetWhenHandedOver());
                std::string fullUrl{"urn:uuid:"};
                fullUrl.append(dispatch->GetId());
                dispatchEntry = {fullUrl, dispatch};
            }
            medicationDispenseEntries.emplace_back(dispatchEntry);
            medicationDispenseSectionEntries.emplace_back(dispatchEntry.CreateReference("http://ehelse.no/fhir/StructureDefinition/sfm-MedicationDispense"));
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
                struct {
                    FhirReference reference;
                    void operator () (const std::shared_ptr<FhirMedicationStatement> &statement) {
                        statement->SetBasedOn({reference});
                    }
                    void operator () (const std::shared_ptr<FhirBasic> &) {
                    }
                } visitor{.reference = prevRef->second};
                std::visit(visitor, statement->second);
            }
            auto nextRef = urlToStatementReferenceMap.find(prescriptionUrl->second);
            if (nextRef != urlToStatementReferenceMap.end()) {
                struct {
                    FhirReference reference;
                    void operator () (const std::shared_ptr<FhirMedicationStatement> &statement) {
                        statement->SetPartOf({reference});
                    }
                    void operator () (const std::shared_ptr<FhirBasic> &) {
                    }
                } visitor{.reference = nextRef->second};
                std::visit(visitor, prevStatement->second);
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
    FhirCompositionSection otherPrescriptionsSection{};
    otherPrescriptionsSection.SetTitle("Other Prescriptions");
    otherPrescriptionsSection.SetCode(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-section-types", "sectionOtherPrescriptions", "List of non medical prescriptions"));
    otherPrescriptionsSection.SetTextStatus("generated");
    otherPrescriptionsSection.SetTextXhtml("<xhtml:div xmlns:xhtml=\"http://www.w3.org/1999/xhtml\">List of non medical prescriptions</xhtml:div>");
    if (bandaSectionEntries.empty()) {
        otherPrescriptionsSection.SetEmptyReason(FhirCodeableConcept("http://terminology.hl7.org/CodeSystem/list-empty-reason", "unavailable",
                                                                     "Unavailable"));
    } else {
        otherPrescriptionsSection.SetEntries(bandaSectionEntries);
    }
    std::vector<FhirBundleEntry> allergyEntries{};
    std::vector<FhirReference> allergyReferences{};
    std::shared_ptr<FhirBundleEntry> m251Entry{};
    {
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
            auto pllInfoExt = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-pllInformation");
            auto pllDate = pll.GetDateTime();
            if (!pllDate.empty()) {
                pllInfoExt->AddExtension(std::make_shared<FhirValueExtension>("PLLdate", std::make_shared<FhirDateTimeValue>(pllDate)));
            }
            m251->AddExtension(pllInfoExt);
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
    }
    pllInfoSection.SetTextStatus("generated");
    pllInfoSection.SetTextXhtml("<xhtml:div xmlns:xhtml=\"http://www.w3.org/1999/xhtml\"></xhtml:div>");
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
    FhirCompositionSection medicationDispenseSection{};
    medicationDispenseSection.SetTitle("sectionDispense");
    medicationDispenseSection.SetCode(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-section-types", "sectionDispense", "Section dispense"));
    medicationDispenseSection.SetTextStatus("generated");
    medicationDispenseSection.SetTextXhtml("<xhtml:div xmlns:xhtml=\"http://www.w3.org/1999/xhtml\"></xhtml:div>");
    if (!medicationDispenseEntries.empty()) {
        medicationDispenseSection.SetEntries(medicationDispenseSectionEntries);
    } else {
        medicationDispenseSection.SetEmptyReason(FhirCodeableConcept("http://terminology.hl7.org/CodeSystem/list-empty-reason", "unavailable", "Unavailable"));
    }
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
        if (!medicationDispenseSectionEntries.empty()) {
            resource.AddSection(medicationDispenseSection);
        }
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
    for (const auto &entry : organizations) {
        bundle->AddEntry(entry);
    }
    for (const auto &entry : practitioners) {
        bundle->AddEntry(entry);
    }
    for (const auto &entry : medicamentEntries) {
        bundle->AddEntry(entry);
    }
    for (const auto &entry : medicationStatementEntries) {
        bundle->AddEntry(entry);
    }
    for (const auto &entry : medicationDispenseEntries) {
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

std::shared_ptr<Fhir> MedicationController::SendMedication(const FhirBundle &bundle, const Person &practitioner) {
    bool createPll{false};
    std::string patientId{};
    std::vector<std::string> ePrescriptionIds{};
    std::vector<std::string> injectedPllIds{};
    std::vector<Prescription> prescriptions{};
    std::vector<Prescription> prescriptionsForPll{};
    std::vector<PllAllergy> allergies{};
    std::map<std::string,Code> potentialRecalls{};
    std::map<std::string,PllCessation> potentialPllCessations{};
    std::map<std::string,PllEntryData> pllEntryDataMap{};
    std::map<std::string,std::string> toPreviousPrescription{};
    std::map<std::string,std::string> pllToPrescriptionIdMap{};
    {
        std::vector<std::variant<std::shared_ptr<FhirMedicationStatement>,std::shared_ptr<FhirBasic>>> medicationStatements{};
        std::vector<std::shared_ptr<FhirMedicationStatement>> medicationStatementsForPll{};
        std::map<std::string,std::shared_ptr<FhirBasic>> fhirBasics{};
        std::map<std::string,std::shared_ptr<FhirAllergyIntolerance>> fhirAllergies{};
        std::map<std::string,std::tuple<Code,std::variant<std::shared_ptr<FhirMedicationStatement>,std::shared_ptr<FhirBasic>>>> potentialRecallsWithStatements{};
        CreatePrescriptionService createPrescriptionService{};
        {
            std::shared_ptr<FhirComposition> composition{};
            for (const auto &entry: bundle.GetEntries()) {
                auto compositionResource = std::dynamic_pointer_cast<FhirComposition>(entry.GetResource());
                if (compositionResource) {
                    if (composition) {
                        std::cerr << "SendMedication: Multiple composition entries\n";
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
                std::cerr << "SendMedication: No composition entries\n";
                return CreateOperationOutcome(CreateIssues(CreateIssue("X", "No composition entries", "Fatal")));
            }
            FhirCompositionSection medicationSection{};
            FhirCompositionSection otherPrescriptionsSection{};
            FhirCompositionSection pllSection{};
            FhirCompositionSection allergySection{};
            {
                bool foundMedication{false};
                bool foundOtherPrescriptions{false};
                bool foundPll{false};
                bool foundAllergies{false};
                for (const auto &section: composition->GetSections()) {
                    bool isMedication{false};
                    bool isOtherPrescriptions{false};
                    bool isPllInfo{false};
                    bool isAllergies{false};
                    for (const auto &coding: section.GetCode().GetCoding()) {
                        auto code = coding.GetCode();
                        std::transform(code.cbegin(), code.cend(), code.begin(), [] (char ch) { return std::tolower(ch); });
                        if (code == "sectionmedication") {
                            isMedication = true;
                            break;
                        }
                        if (code == "sectionotherprescriptions") {
                            isOtherPrescriptions = true;
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
                        if (isPllInfo || isAllergies || isOtherPrescriptions) {
                            std::cerr << "SendMedication: Ambigous section\n";
                            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Ambiguous section", "Fatal")));
                        }
                        if (foundMedication) {
                            std::cerr << "SendMedication: Multiple medication sections\n";
                            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Multiple medication sections", "Fatal")));
                        }
                        medicationSection = section;
                        foundMedication = true;
                    }
                    if (isOtherPrescriptions) {
                        if (isPllInfo || isAllergies) {
                            std::cerr << "SendMedication: Ambigous section\n";
                            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Ambiguous section", "Fatal")));
                        }
                        if (foundOtherPrescriptions) {
                            std::cerr << "SendMedication: Multiple other prescriptions sections\n";
                            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Multiple other prescriptions sections", "Fatal")));
                        }
                        otherPrescriptionsSection = section;
                        foundOtherPrescriptions = true;
                    }
                    if (isPllInfo) {
                        if (isAllergies) {
                            std::cerr << "SendMedication: Ambigous section\n";
                            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Ambiguous section", "Fatal")));
                        }
                        if (foundPll) {
                            std::cerr << "SendMedication: Multiple PLL info sections\n";
                            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Multiple PLL info sections", "Fatal")));
                        }
                        pllSection = section;
                        foundPll = true;
                    }
                    if (isAllergies) {
                        if (foundAllergies) {
                            std::cerr << "SendMedication: Multiple allergies sections\n";
                            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Multiple allergies sections", "Fatal")));
                        }
                        allergySection = section;
                        foundAllergies = true;
                    }
                }
                if (!foundMedication) {
                    std::cerr << "SendMedication: No medication section\n";
                    return CreateOperationOutcome(CreateIssues(CreateIssue("X", "No medication section", "Fatal")));
                }
                if (!foundOtherPrescriptions) {
                    std::cerr << "SendMedication: No other prescriptions section\n";
                    return CreateOperationOutcome(CreateIssues(CreateIssue("X", "No other prescriptions section", "Fatal")));
                }
                if (!foundPll) {
                    std::cerr << "SendMedication: No PLL info section\n";
                    return CreateOperationOutcome(CreateIssues(CreateIssue("X", "No PLL info section", "Fatal")));
                }
                if (!foundAllergies) {
                    std::cerr << "SendMedication: No allergy section\n";
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
                std::vector<std::string> basicReferences{};
                for (const auto &entryReference: medicationSection.GetEntries()) {
                    medicationStatementReferences.emplace_back(entryReference.GetReference());
                }
                for (const auto &entryReference: otherPrescriptionsSection.GetEntries()) {
                    basicReferences.emplace_back(entryReference.GetReference());
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
                    } else {
                        iterator = std::find(basicReferences.begin(), basicReferences.end(), fullUrl);
                        if (iterator != basicReferences.end()) {
                            auto basic = std::dynamic_pointer_cast<FhirBasic>(entry.GetResource());
                            if (basic) {
                                medicationStatements.emplace_back(basic);
                            }
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
                struct {
                    std::vector<FhirIdentifier> operator () (const std::shared_ptr<FhirMedicationStatement> &medicationStatement) {
                        return medicationStatement->GetIdentifiers();
                    }
                    std::vector<FhirIdentifier> operator () (const std::shared_ptr<FhirBasic> &basic) {
                        std::vector<FhirIdentifier> identifiers{};
                        for (const auto &identifier : basic->GetIdentifiers()) {
                            auto type = identifier.GetType().GetText();
                            std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) { return std::tolower(ch); });
                            if (type == "pll") {
                                continue;
                            }
                            identifiers.emplace_back(identifier);
                        }
                        return identifiers;
                    }
                } visitor;
                auto identifiers = std::visit(visitor, medicationStatement);
                for (const auto &identifier : identifiers) {
                    auto type = identifier.GetType().GetText();
                    std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) { return std::tolower(ch); });
                    if (type == "reseptid") {
                        prescriptionId = identifier.GetValue();
                    } else if (type == "pll") {
                        pllId = identifier.GetValue();
                    }
                }
                if (createPll && pllId.empty()) {
                    struct {
                        std::vector<FhirIdentifier> &identifiers;
                        std::string &pllId;
                        void operator () (const std::shared_ptr<FhirMedicationStatement> &medicationStatement) {
                            boost::uuids::uuid uuid = boost::uuids::random_generator()();
                            pllId = to_string(uuid);
                            identifiers.emplace_back(FhirCodeableConcept("pll"), "official", pllId);
                            medicationStatement->SetIdentifiers(identifiers);
                        }
                        void operator () (const std::shared_ptr<FhirBasic> &basic) {
                        }
                    } visitor{.identifiers = identifiers, .pllId = pllId};;
                    std::visit(visitor, medicationStatement);
                }
                if (createPll && !prescriptionId.empty() && !pllId.empty()) {
                    pllToPrescriptionIdMap.insert_or_assign(pllId, prescriptionId);
                }
                PllCessation pllCessation{};
                PllEntryData pllEntryData{};
                struct {
                    std::vector<std::shared_ptr<FhirExtension>> operator () (const std::shared_ptr<FhirMedicationStatement> &medicationStatement) {
                        return medicationStatement->GetExtensions();
                    }
                    std::vector<std::shared_ptr<FhirExtension>> operator () (const std::shared_ptr<FhirBasic> &basic) {
                        return basic->GetExtensions();
                    }
                } GetExtensionsVisitor;
                for (const auto &extension: std::visit(GetExtensionsVisitor, medicationStatement)) {
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
                                    std::tuple<Code,std::variant<std::shared_ptr<FhirMedicationStatement>,std::shared_ptr<FhirBasic>>> codeTuple =
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
                            std::cerr << "SendMedication: Discontinuation without timedate\n";
                            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Discontinuation without timedate", "Fatal")));
                        }
                        pllCessation.timeDate = timeDate;
                        auto codings = reason.GetCoding();
                        if (!codings.empty()) {
                            if (!note.empty()) {
                                std::cerr << "SendMedication: Either reason or note for discontinuation\n";
                                return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Either reason or note for discontinuation", "Fatal")));
                            }
                            if (codings.size() != 1) {
                                std::cerr << "SendMedication: Multiple reason codes for discontinuation\n";
                                return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Multiple reason codes for discontinuation", "Fatal")));
                            }
                            auto coding = codings[0];
                            if (coding.GetCode().empty()) {
                                std::cerr << "SendMedication: No reason code supplied for discontinuation\n";
                                return CreateOperationOutcome(CreateIssues(CreateIssue("X", "No reason code supplied for discontinuation", "Fatal")));
                            }
                            if (!reason.GetText().empty()) {
                                std::cerr << "SendMedication: Reason text should not be set for discontinuation\n";
                                return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Reason text should not be set for discontinuation", "Fatal")));
                            }
                            pllCessation.reason.setSystem(coding.GetSystem());
                            pllCessation.reason.setCode(coding.GetCode());
                            pllCessation.reason.setDisplay(coding.GetDisplay());
                        } else {
                            if (note.empty()) {
                                std::cerr << "SendMedication: No reason or note supplied for discontinuation\n";
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
                    } else {
                        struct {
                            std::vector<std::shared_ptr<FhirMedicationStatement>> &medicationStatementsForPll;
                            void operator () (const std::shared_ptr<FhirMedicationStatement> &medicationStatement) {
                                medicationStatementsForPll.emplace_back(medicationStatement);
                            }
                            void operator () (const std::shared_ptr<FhirBasic> &) {
                            }
                        } visitor{
                            .medicationStatementsForPll = medicationStatementsForPll
                        };
                        std::visit(visitor, medicationStatement);
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
            struct {
                CreatePrescriptionService &createPrescriptionService;
                const FhirBundle &bundle;
                Prescription operator () (const std::shared_ptr<FhirMedicationStatement> &medicationStatement) {
                    return createPrescriptionService.CreatePrescription(medicationStatement, bundle);
                }
                Prescription operator () (const std::shared_ptr<FhirBasic> &basic) {
                    return createPrescriptionService.CreatePrescription(basic, bundle);
                }
            } visitor{.createPrescriptionService = createPrescriptionService, .bundle = bundle};
            Prescription prescription = std::visit(visitor, medicationStatement);
            {
                auto pllId = prescription.GetPllId();
                if (!pllId.empty()) {
                    injectedPllIds.emplace_back(pllId);
                }
            }
            if (!practitioner.GetId().empty()) {
                prescription.SetPrescribedBy(practitioner.GetId());
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
        for (const auto &medicationStatement : medicationStatementsForPll) {
            Prescription prescription = createPrescriptionService.CreatePrescription(medicationStatement, bundle);
            {
                auto pllId = prescription.GetPllId();
                if (!pllId.empty()) {
                    injectedPllIds.emplace_back(pllId);
                }
            }
            if (!practitioner.GetId().empty()) {
                prescription.SetPrescribedBy(practitioner.GetId());
            }
            patientId = prescription.GetPatient();
            if (prescription.GetTypeOfPrescription().getCode() != "E") {
                boost::uuids::uuid uuid = boost::uuids::random_generator()();
                std::string uuid_string = to_string(uuid);
                prescription.SetId(uuid_string);
                prescription.SetRfStatus({});
            }
            prescriptionsForPll.emplace_back(prescription);
        }
        for (const auto &recallTuple : potentialRecallsWithStatements) {
            auto prescriptionId = recallTuple.first;
            auto code = std::get<0>(recallTuple.second);
            auto medicationStatement = std::get<1>(recallTuple.second);
            struct {
                FhirReference operator () (const std::shared_ptr<FhirMedicationStatement> &medicationStatement) {
                    return medicationStatement->GetSubject();
                }
                FhirReference operator () (const std::shared_ptr<FhirBasic> &basic) {
                    return basic->GetSubject();
                }
            } visitor;
            auto person = createPrescriptionService.GetPerson(bundle, std::visit(visitor, medicationStatement).GetReference());
            patientId = person.GetId();
            potentialRecalls.insert_or_assign(prescriptionId, code);
        }
        if (!prescriptions.empty() && patientId.empty()) {
            std::cerr << "SendMedication: No patient\n";
            return CreateOperationOutcome(CreateIssues(CreateIssue("X", "No patient", "Fatal")));
        }
        for (const auto &prescription : prescriptions) {
            if (prescription.GetPatient() != patientId) {
                std::cerr << "SendMedication: Different patient\n";
                return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Different patient", "Fatal")));
            }
        }
        for (const auto &prescription : prescriptionsForPll) {
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
                    std::cerr << "SendMedication: Create prescription without renew\n";
                    return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Create prescription without renew", "Fatal")));
                }
                auto prevPresIdIter = toPreviousPrescription.find(id);
            }
            if (potentialRecalls.find(prescriptionId) != potentialRecalls.end()) {
                auto rfStatus = prescription->GetRfStatus().getCode();
                if (rfStatus != "E" && rfStatus != "U") {
                    std::cerr << "SendMedication: Recalling inactive prescription\n";
                    return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Recalling inactive prescription", "Fatal")));
                }
            } else if (createPll && !prescriptionId.empty() && !pllId.empty()) {
                if (std::find(ePrescriptionIds.cbegin(), ePrescriptionIds.cend(), prescriptionId) != ePrescriptionIds.cend() &&
                    std::find(injectedPllIds.cbegin(), injectedPllIds.cend(), pllId) != injectedPllIds.cend()) {
                    std::cerr << "SendMedication: Existing and kept PLL ID reused on a new prescription\n";
                    return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Existing and kept PLL ID reused on a new prescription", "Fatal")));
                }
                auto iterator = pllToPrescriptionIdMap.find(pllId);
                if (iterator != pllToPrescriptionIdMap.end() && iterator->second != prescriptionId) {
                    clearPllIdForMove.emplace_back(pllId);
                    if (prescription->GetRfStatus().getCode() == "E") {
                        std::cerr << "SendMedication: PLL is already containing a prescription with rfstatus E\n";
                        return CreateOperationOutcome(CreateIssues(CreateIssue("X", "PLL is already containing a prescription with rfstatus E", "Fatal")));
                    }
                }
            }
        }
        for (auto &prescription : prescriptions) {
            auto id = prescription.GetId();
            if (id.empty()) {
                if (!createPll || prescription.GetPllId().empty()) {
                    std::cerr << "SendMedication: Requested create prescription without ID\n";
                    return CreateOperationOutcome(CreateIssues(CreateIssue("X", "Requested create prescription without ID", "Fatal")));
                }
                std::cerr << "SendMedication: Without prescription is not yet supported\n";
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
            pll.SetPrescriptions(prescriptionsForPll);
            pllStorage.Store(patientId, pll);
            prescriptionStorage.StorePaperDispatchMap(patientId, {});
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
