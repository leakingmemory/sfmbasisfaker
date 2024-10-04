//
// Created by sigsegv on 2/22/24.
//

#include "CreatePrescriptionService.h"
#include <sfmbasisapi/fhir/medstatement.h>
#include <sfmbasisapi/fhir/bundle.h>
#include <sfmbasisapi/fhir/bundleentry.h>
#include <sfmbasisapi/fhir/medication.h>
#include <sfmbasisapi/fhir/person.h>
#include <map>
#include <boost/uuid/uuid_generators.hpp> // for random_generator
#include <boost/uuid/uuid_io.hpp> // for to_string
#include "PersonStorage.h"
#include "../domain/person.h"

Person CreatePrescriptionService::GetPerson(const FhirPerson &fhir) const {
    std::string dateOfBirth = fhir.GetBirthDate();
    PersonGender gender = fhir.GetGender() == "female" ? PersonGender::FEMALE : PersonGender::MALE;
    std::string givenName{};
    std::string familyName{};
    {
        auto names = fhir.GetName();
        for (const auto &name : names) {
            auto g = name.GetGiven();
            auto f = name.GetFamily();
            if (!g.empty()) {
                givenName = g;
            }
            if (!f.empty()) {
                familyName = f;
            }
        }
    }
    std::string pid{};
    std::string hpr{};
    {
        auto identifiers = fhir.GetIdentifiers();
        for (const auto &identifier: identifiers) {
            auto system = identifier.GetSystem();
            if (system == "urn:oid:2.16.578.1.12.4.1.4.1") {
                pid = identifier.GetValue();
            } else if (system == "urn:oid:2.16.578.1.12.4.1.4.4") {
                hpr = identifier.GetValue();
            }
        }
    }
    PersonStorage personStorage{};
    Person person{};
    if (!pid.empty()) {
        person = personStorage.GetByFodselsnummer(pid);
    }
    bool modified{false};
    if (!pid.empty() && person.GetFodselsnummer() != pid) {
        person.SetFodselsnummer(pid);
        modified = true;
    }
    if (!hpr.empty() && person.GetHpr() != hpr) {
        person.SetHpr(hpr);
        modified = true;
    }
    if (!dateOfBirth.empty()) {
        person.SetDateOfBirth(dateOfBirth);
        modified = true;
    }
    if (!givenName.empty() || !familyName.empty()) {
        if (givenName != person.GetGivenName() || familyName != person.GetFamilyName()) {
            person.SetGivenName(givenName);
            person.SetFamilyName(familyName);
        }
        modified = true;
    }
    if (modified || person.GetId().empty()) {
        personStorage.Store(person);
    }
    return person;
}

Person CreatePrescriptionService::GetPerson(const FhirBundle &bundle, const std::string &fullUrl) const {
    for (const auto &entry : bundle.GetEntries()) {
        if (entry.GetFullUrl() == fullUrl) {
            auto resource = std::dynamic_pointer_cast<FhirPerson>(entry.GetResource());
            if (resource) {
                return GetPerson(*resource);
            }
        }
    }
    return {};
}

static bool SetCommonValues(Medication &medicationObject, const std::string &atc, const std::string &atcDisplay, const FhirMedication &medication, std::vector<std::shared_ptr<FhirValueExtension>> &otherValueExtensions, std::vector<std::shared_ptr<FhirExtension>> &otherExtensions) {
    medicationObject.SetAtc(atc, atcDisplay);
    FhirCoding form{};
    {
        auto formCodings = medication.GetForm().GetCoding();
        if (formCodings.empty()) {
            return false;
        }
        form = formCodings[0];
    }
    medicationObject.SetForm({form.GetCode(), form.GetDisplay(), form.GetSystem()});
    auto extensions = medication.GetExtensions();
    for (const auto &extension : extensions) {
        auto url = extension->GetUrl();
        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
        std::shared_ptr<FhirValue> value{};
        if (valueExtension) {
            value = valueExtension->GetValue();
        }
        if (url == "http://hl7.no/fhir/StructureDefinition/no-basis-prescriptiongroup" && !value.operator bool()) {
            auto subexts = extension->GetExtensions();
            if (subexts.size() != 1) {
                otherExtensions.emplace_back(extension);
                continue;
            }
            auto subext = std::dynamic_pointer_cast<FhirValueExtension>(subexts[0]);
            if (!subext || subext->GetUrl() != "prescriptiongroup") {
                otherExtensions.emplace_back(extension);
                continue;
            }
            value = subext->GetValue();
            auto codeableConceptValue = std::dynamic_pointer_cast<FhirCodeableConceptValue>(value);
            if (codeableConceptValue) {
                auto codings = codeableConceptValue->GetCoding();
                if (!codings.empty()) {
                    auto coding = codings[0];
                    medicationObject.SetPrescriptionGroup({coding.GetCode(), coding.GetDisplay(), coding.GetSystem()});
                }
            } else {
                otherExtensions.emplace_back(extension);
            }
            continue;
        } else if (valueExtension) {
            otherValueExtensions.emplace_back(valueExtension);
        } else {
            otherExtensions.emplace_back(extension);
        }
    }
    return true;
}

std::shared_ptr<Medication>
CreatePrescriptionService::CreateMedication(const FhirReference &medicationReference, const FhirBundle &bundle) const {
    std::shared_ptr<FhirMedication> medication{};
    {
        auto entries = bundle.GetEntries();
        auto medicationIterator = entries.begin();
        while (medicationIterator != entries.end()) {
            if (medicationIterator->GetFullUrl() == medicationReference.GetReference()) {
                break;
            }
            ++medicationIterator;
        }
        if (medicationIterator == entries.end()) {
            return {};
        }
        medication = std::dynamic_pointer_cast<FhirMedication>(medicationIterator->GetResource());
        if (!medication) {
            return {};
        }
    }
    FhirCoding code{};
    std::string atc{};
    std::string atcDisplay{};
    std::string festCode{};
    std::string festDisplay{};
    {
        auto codeCoding = medication->GetCode().GetCoding();
        if (codeCoding.empty()) {
            return {};
        }
        for (const auto &c : codeCoding) {
            if (c.GetSystem() == "urn:oid:2.16.578.1.12.4.1.1.7424") {
                if (code.GetCode().empty()) {
                    code = c;
                } else {
                    return {};
                }
            }
            if (c.GetSystem() == "http://www.whocc.no/atc") {
                atc = c.GetCode();
                atcDisplay = c.GetDisplay();
            }
            if (c.GetSystem() == "http://ehelse.no/fhir/CodeSystem/FEST") {
                festCode = c.GetCode();
                festDisplay = c.GetDisplay();
            }
        }
    }
    if (code.GetSystem() == "urn:oid:2.16.578.1.12.4.1.1.7424" && code.GetCode() == "10") /* Magistrell */ {
        auto medicationObject = std::make_shared<MagistralMedication>();
        std::vector<std::shared_ptr<FhirValueExtension>> otherValueExtensions{};
        std::vector<std::shared_ptr<FhirExtension>> otherExtensions{};
        if (!SetCommonValues(*medicationObject, atc, atcDisplay, *medication, otherValueExtensions, otherExtensions)) {
            return {};
        }
        auto amount = medication->GetAmount();
        auto amountNumerator = amount.GetNumerator();
        auto amountDenomerator = amount.GetDenominator();
        if (!amountNumerator.IsSet()) {
            return {};
        }
        if (amountDenomerator.IsSet()) {
            return {};
        }
        medicationObject->SetAmount(amountNumerator.GetValue());
        medicationObject->SetAmountUnit(amountNumerator.GetUnit());
        auto extensions = medication->GetExtensions();
        for (const auto &valueExtension : otherValueExtensions) {
            auto url = valueExtension->GetUrl();
            std::shared_ptr<FhirValue> value{};
            if (valueExtension) {
                value = valueExtension->GetValue();
            }
            if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-name" && value.operator bool()) {
                auto stringValue = std::dynamic_pointer_cast<FhirString>(value);
                if (stringValue) {
                    medicationObject->SetName(stringValue->GetValue());
                }
            } else if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-recipe" && value.operator bool()) {
                auto stringValue = std::dynamic_pointer_cast<FhirString>(value);
                if (stringValue) {
                    medicationObject->SetRecipe(stringValue->GetValue());
                }
            }
        }
        return medicationObject;
    } else if (!festCode.empty()) {
        std::shared_ptr<FhirExtension> medicationDetails{};
        {
            auto extensions = medication->GetExtensions();
            for (const auto &extension: extensions) {
                if (extension->GetUrl() == "http://ehelse.no/fhir/StructureDefinition/sfm-medicationdetails") {
                    if (medicationDetails) {
                        return {};
                    }
                    medicationDetails = extension;
                }
            }
        }
        if (!medicationDetails) {
            return {};
        }
        std::string registreringstypeCode{};
        std::vector<std::shared_ptr<FhirExtension>> medicationDetailsExtensions{};
        {
            FhirCoding registreringstypeCoding{};
            {
                FhirCodeableConcept registreringstype{};
                {
                    auto extensions = medicationDetails->GetExtensions();
                    for (const auto &extension: extensions) {
                        if (extension->GetUrl() == "registreringstype") {
                            auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            if (valueExt) {
                                auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExt->GetValue());
                                if (value) {
                                    registreringstype = *value;
                                }
                            }
                        } else {
                            medicationDetailsExtensions.emplace_back(extension);
                        }
                    }
                }
                auto registreringstypeCodings = registreringstype.GetCoding();
                if (registreringstypeCodings.empty()) {
                    return {};
                }
                registreringstypeCoding = registreringstypeCodings[0];
            }
            if (registreringstypeCoding.GetSystem() != "http://ehelse.no/fhir/CodeSystem/sfm-festregistrationtype") {
                return {};
            }
            registreringstypeCode = registreringstypeCoding.GetCode();
        }
        if (registreringstypeCode == "1") {
            auto medicationObject = std::make_shared<GenericMedication>(festCode, festDisplay);
            std::vector<std::shared_ptr<FhirValueExtension>> otherValueExtensions{};
            std::vector<std::shared_ptr<FhirExtension>> otherExtensions{};
            if (!SetCommonValues(*medicationObject, atc, atcDisplay, *medication, otherValueExtensions, otherExtensions)) {
                return {};
            }
            return medicationObject;
        } else if (registreringstypeCode == "2") {
            auto medicationObject = std::make_shared<BrandNameMedication>(festCode, festDisplay);
            std::vector<std::shared_ptr<FhirValueExtension>> otherValueExtensions{};
            std::vector<std::shared_ptr<FhirExtension>> otherExtensions{};
            if (!SetCommonValues(*medicationObject, atc, atcDisplay, *medication, otherValueExtensions, otherExtensions)) {
                return {};
            }
            return medicationObject;
        } else if (registreringstypeCode == "3") {
            auto medicationObject = std::make_shared<PackageMedication>(festCode, festDisplay);
            std::vector<std::shared_ptr<FhirValueExtension>> otherValueExtensions{};
            std::vector<std::shared_ptr<FhirExtension>> otherExtensions{};
            if (!SetCommonValues(*medicationObject, atc, atcDisplay, *medication, otherValueExtensions, otherExtensions)) {
                return {};
            }
            std::vector<PackingInfoPrescription> packingInfoPrescription{};
            for (const auto &extension : medicationDetailsExtensions) {
                if (extension->GetUrl() == "packinginfoprescription") {
                    auto &pi = packingInfoPrescription.emplace_back();
                    auto subext = extension->GetExtensions();
                    for (const auto &sube : subext) {
                        auto valext = std::dynamic_pointer_cast<FhirValueExtension>(sube);
                        if (!valext) {
                            continue;
                        }
                        auto key = sube->GetUrl();
                        if (key == "name") {
                            auto str = std::dynamic_pointer_cast<FhirString>(valext->GetValue());
                            if (!str) {
                                continue;
                            }
                            pi.SetName(str->GetValue());
                        } else if (key == "packingsize") {
                            auto str = std::dynamic_pointer_cast<FhirString>(valext->GetValue());
                            if (!str) {
                                continue;
                            }
                            pi.SetPackingSize(str->GetValue());
                        } else if (key == "packingunit") {
                            auto codeable = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valext->GetValue());
                            if (!codeable) {
                                continue;
                            }
                            auto codings = codeable->GetCoding();
                            if (codings.size() != 1) {
                                continue;
                            }
                            auto coding = codings[0];
                            pi.SetPackingUnit(Code(coding.GetCode(), coding.GetDisplay(), coding.GetSystem()));
                        }
                    }
                }
            }
            medicationObject->SetPackageInfoPrescription(packingInfoPrescription);
            return medicationObject;
        } else {
            return {};
        }
    } else {
        return {};
    }
}

Prescription
CreatePrescriptionService::CreatePrescription(const std::shared_ptr<FhirMedicationStatement> &medicationStatement,
                                              const FhirBundle &bundle) const {
    Prescription prescription{};
    {
        auto medication = CreateMedication(medicationStatement->GetMedicationReference(), bundle);
        prescription.SetMedication(medication);
    }
    {
        auto dosages = medicationStatement->GetDosage();
        if (dosages.size() != 1) {
            return {};
        }
        auto dosage = dosages[0];
        prescription.SetDosingText(dosage.GetText());
        auto extensions = dosage.GetExtensions();
        for (const auto &extension : extensions) {
            auto url = extension->GetUrl();
            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) { return std::tolower(ch); });
            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
            std::shared_ptr<FhirValue> value{};
            if (valueExtension) {
                value = valueExtension->GetValue();
            }
            if (url == "http://ehelse.no/fhir/structuredefinition/sfm-use") {
                auto codeableConcept = std::dynamic_pointer_cast<FhirCodeableConceptValue>(value);
                if (codeableConcept) {
                    auto codings = codeableConcept->GetCoding();
                    if (!codings.empty()) {
                        auto coding = codings[0];
                        prescription.SetUse({coding.GetCode(), coding.GetDisplay(), coding.GetSystem()});
                    }
                }
            } else if (url == "http://ehelse.no/fhir/structuredefinition/sfm-application-area") {
                std::map<std::string,std::shared_ptr<FhirValue>> submap{};
                {
                    auto subextensions = extension->GetExtensions();
                    for (const auto &subext: subextensions) {
                        auto subvalext = std::dynamic_pointer_cast<FhirValueExtension>(subext);
                        if (subvalext) {
                            submap.insert_or_assign(subvalext->GetUrl(), subvalext->GetValue());
                        }
                    }
                }
                auto findText = submap.find("text");
                if (findText != submap.end()) {
                    auto str = std::dynamic_pointer_cast<FhirString>(findText->second);
                    if (str) {
                        prescription.SetApplicationArea(str->GetValue());
                    }
                }
            } else if (url == "http://ehelse.no/fhir/structuredefinition/sfm-shortdosage") {
                auto codeableConcept = std::dynamic_pointer_cast<FhirCodeableConceptValue>(value);
                if (codeableConcept) {
                    auto codings = codeableConcept->GetCoding();
                    if (!codings.empty()) {
                        auto coding = codings[0];
                        prescription.SetShortDose({coding.GetCode(), coding.GetDisplay(), coding.GetSystem()});
                    }
                }
            }
        }
    }
    auto identifiers = medicationStatement->GetIdentifiers();
    for (const auto &identifier : identifiers) {
        auto typeCodeable = identifier.GetType();
        auto typeCodings = typeCodeable.GetCoding();
        if (!typeCodings.empty()) {
            continue;
        }
        auto typeText = typeCodeable.GetText();
        std::transform(typeText.cbegin(), typeText.cend(), typeText.begin(), [] (char ch) { return std::tolower(ch); });
        if (typeText == "reseptid") {
            prescription.SetId(identifier.GetValue());
        } else if (typeText == "pll") {
            prescription.SetPllId(identifier.GetValue());
        }
    }
    {
        auto person = GetPerson(bundle, medicationStatement->GetSubject().GetReference());
        prescription.SetPatient(person.GetId());
        if (prescription.GetPatient().empty()) {
            return {};
        }
    }
    auto statementExtensions = medicationStatement->GetExtensions();
    for (const auto &statementExtension : statementExtensions) {
        auto statementExtensionUrl = statementExtension->GetUrl();
        std::transform(statementExtensionUrl.cbegin(), statementExtensionUrl.cend(), statementExtensionUrl.begin(), [] (char ch) { return std::tolower(ch); });
        if (statementExtensionUrl == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
            auto extensions = statementExtension->GetExtensions();
            for (const auto &extension : extensions) {
                auto url = extension->GetUrl();
                auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                std::shared_ptr<FhirValue> value{};
                if (valueExtension) {
                    value = valueExtension->GetValue();
                }
                if (url == "reseptdate") {
                    auto dateval = std::dynamic_pointer_cast<FhirDateValue>(value);
                    if (dateval) {
                        prescription.SetPrescriptionDate(dateval->GetRawValue());
                    }
                } else if (url == "expirationdate") {
                    auto dateval = std::dynamic_pointer_cast<FhirDateValue>(value);
                    if (dateval) {
                        prescription.SetExpirationDate(dateval->GetRawValue());
                    }
                } else if (url == "festUpdate") {
                    auto dateTimeVal = std::dynamic_pointer_cast<FhirDateTimeValue>(value);
                    if (dateTimeVal) {
                        prescription.SetFestUpdate(dateTimeVal->GetDateTime());
                    }
                } else if (url == "dssn") {
                    auto stringVal = std::dynamic_pointer_cast<FhirString>(value);
                    if (stringVal) {
                        prescription.SetDssn(stringVal->GetValue());
                    }
                } else if (url == "amount") {
                    auto quantityValue = std::dynamic_pointer_cast<FhirQuantityValue>(value);
                    if (quantityValue) {
                        prescription.SetAmountUnit(quantityValue->GetUnit());
                        prescription.SetAmount(quantityValue->GetValue());
                    }
                } else if (url == "numberofpackages") {
                    auto decimalValue = std::dynamic_pointer_cast<FhirDecimalValue>(value);
                    if (decimalValue) {
                        prescription.SetNumberOfPackages(decimalValue->GetValue());
                    }
                } else if (url == "reit") {
                    auto stringVal = std::dynamic_pointer_cast<FhirString>(value);
                    if (stringVal) {
                        prescription.SetReit(stringVal->GetValue());
                    }
                } else if (url == "itemgroup") {
                    auto codeableValue = std::dynamic_pointer_cast<FhirCodeableConceptValue>(value);
                    if (codeableValue) {
                        auto codings = codeableValue->GetCoding();
                        if (!codings.empty()) {
                            auto coding = codings[0];
                            prescription.SetItemGroup({coding.GetCode(), coding.GetDisplay(), coding.GetSystem()});
                        }
                    }
                } else if (url == "rfstatus") {
                    auto subextensions = extension->GetExtensions();
                    for (const auto &subext : subextensions) {
                        auto suburl = subext->GetUrl();
                        auto subvaluext = std::dynamic_pointer_cast<FhirValueExtension>(subext);
                        if (suburl == "status" && subvaluext) {
                            auto subval = std::dynamic_pointer_cast<FhirCodeableConceptValue>(subvaluext->GetValue());
                            if (subval) {
                                auto codings = subval->GetCoding();
                                if (!codings.empty()) {
                                    auto coding = codings[0];
                                    prescription.SetRfStatus({coding.GetCode(), coding.GetDisplay(), coding.GetSystem()});
                                }
                            }
                        }
                    }
                } else if (url == "lastchanged") {
                    auto dateTimeVal = std::dynamic_pointer_cast<FhirDateTimeValue>(value);
                    if (dateTimeVal) {
                        prescription.SetLastChanged(dateTimeVal->GetDateTime());
                    }
                } else if (url == "typeresept") {
                    auto codeableValue = std::dynamic_pointer_cast<FhirCodeableConceptValue>(value);
                    if (codeableValue) {
                        auto codings = codeableValue->GetCoding();
                        if (!codings.empty()) {
                            auto coding = codings[0];
                            prescription.SetTypeOfPrescription({coding.GetCode(), coding.GetDisplay(), coding.GetSystem()});
                        }
                    }
                }
            }
        } else if (statementExtensionUrl == "http://ehelse.no/fhir/structuredefinition/sfm-reginfo") {
            Code status{};
            Code type{};
            std::string provider{};
            std::string timestamp{};
            {
                auto extensions = statementExtension->GetExtensions();
                for (const auto &extension : extensions) {
                    auto url = extension->GetUrl();
                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                    std::shared_ptr<FhirValue> value{};
                    if (valueExtension) {
                        value = valueExtension->GetValue();
                    }
                    if (url == "status") {
                        auto codeableValue = std::dynamic_pointer_cast<FhirCodeableConceptValue>(value);
                        if (codeableValue) {
                            auto codings = codeableValue->GetCoding();
                            if (!codings.empty()) {
                               auto coding = codings[0];
                               status = {coding.GetCode(), coding.GetDisplay(), coding.GetSystem()};
                            }
                        }
                    } else if (url == "type") {
                        auto codeableValue = std::dynamic_pointer_cast<FhirCodeableConceptValue>(value);
                        if (codeableValue) {
                            auto codings = codeableValue->GetCoding();
                            if (!codings.empty()) {
                                auto coding = codings[0];
                                type = {coding.GetCode(), coding.GetDisplay(), coding.GetSystem()};
                            }
                        }
                    } else if (url == "provider") {
                        auto referenceValue = std::dynamic_pointer_cast<FhirReference>(value);
                        if (referenceValue) {
                            auto person = GetPerson(bundle, referenceValue->GetReference());
                            provider = person.GetId();
                        }
                    } else if (url == "timestamp") {
                        auto dateTimeValue = std::dynamic_pointer_cast<FhirDateTimeValue>(value);
                        if (dateTimeValue) {
                            timestamp = dateTimeValue->GetDateTime();
                        }
                    }
                }
            }
            if (status.getCode() == "3"/*Godkjent*/ && type.getCode() == "1"/*Forskrevet av*/) {
                prescription.SetPrescribedBy(provider);
                prescription.SetPrescribedTimestamp(timestamp);
            }
        } else if (statementExtensionUrl == "http://ehelse.no/fhir/structuredefinition/sfm-generic-substitution") {
            auto extensions = statementExtension->GetExtensions();
            for (const auto &extension : extensions) {
                if (extension->GetUrl() == "genericSubstitutionAccepted") {
                    auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                    if (valueExt) {
                        auto value = std::dynamic_pointer_cast<FhirBooleanValue>(valueExt->GetValue());
                        if (value) {
                            prescription.SetGenericSubstitutionAccepted(value->IsTrue());
                        }
                    }
                }
            }
        } else if (statementExtensionUrl == "http://ehelse.no/fhir/structuredefinition/sfm-discontinuation") {
            std::string timeDate{};
            FhirCodeableConcept reason{};
            std::string note{};
            for (const auto &subExtension: statementExtension->GetExtensions()) {
                auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(subExtension);
                if (!valueExtension) {
                    continue;
                }
                auto url = subExtension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) { return std::tolower(ch); });
                if (url == "timedate") {
                    auto value = std::dynamic_pointer_cast<FhirString>(valueExtension->GetValue());
                    if (value) {
                        timeDate = value->GetValue();
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
            prescription.SetCessationTime(timeDate);
            auto codings = reason.GetCoding();
            if (!codings.empty()) {
                if (codings.size() == 1) {
                    auto coding = codings[0];
                    Code code{coding.GetCode(), coding.GetDisplay(), coding.GetSystem()};
                    prescription.SetCessationReason(code);
                }
            } else {
                Code code{"", note, ""};
                prescription.SetCessationReason(code);
            }
        }
    }
    return prescription;
}

std::vector<FhirBundleEntry>
CreatePrescriptionService::CreateBundleEntryFromMedication(const std::shared_ptr<FhirMedication> &medication) const {
    std::string medicationFullUrl{"urn:uuid:"};
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        auto id = boost::uuids::to_string(randomUUID);
        medicationFullUrl.append(id);
    }
    FhirBundleEntry bundleEntry{std::move(medicationFullUrl), medication};
    return {bundleEntry};
}

void CreatePrescriptionService::SetCommonFhirMedication(FhirMedication &medication, const Medication &source) const {
    {
        auto form = source.GetForm();
        FhirCodeableConcept codeable{form.getSystem(), form.getCode(), form.getDisplay()};
        medication.SetForm(codeable);
    }
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        auto id = boost::uuids::to_string(randomUUID);
        medication.SetId(id);
    }
    {
        auto prescriptionGroup = source.GetPrescriptionGroup();
        FhirCodeableConcept codeable{prescriptionGroup.getSystem(), prescriptionGroup.getCode(), prescriptionGroup.getDisplay()};
        medication.AddExtension(std::make_shared<FhirValueExtension>(
                "http://hl7.no/fhir/StructureDefinition/no-basis-prescriptiongroup",
                std::make_shared<FhirCodeableConceptValue>(codeable)
        ));
    }
}

std::vector<FhirBundleEntry> CreatePrescriptionService::CreateFhirMedicationFromMagistral(
        const std::shared_ptr<MagistralMedication> &magistral) const {
    auto medication = std::make_shared<FhirMedication>();
    SetCommonFhirMedication(*medication, *magistral);
    medication->SetAmount({{magistral->GetAmount(), magistral->GetAmountUnit()}, {}});
    {
        FhirCodeableConcept code{"urn:oid:2.16.578.1.12.4.1.1.7424", "10", "Magistrell"};
        medication->SetCode(code);
    }
    medication->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Magistrell-Medication");
    medication->SetStatus(FhirStatus::ACTIVE);
    {
        auto name = magistral->GetName();
        medication->SetName(name);
        medication->AddExtension(std::make_shared<FhirValueExtension>(
                "http://ehelse.no/fhir/StructureDefinition/sfm-name",
                std::make_shared<FhirString>(name)
        ));
    }
    medication->AddExtension(std::make_shared<FhirValueExtension>(
        "http://ehelse.no/fhir/StructureDefinition/sfm-recipe",
        std::make_shared<FhirString>(magistral->GetRecipe())
    ));
    return CreateBundleEntryFromMedication(medication);
}

std::vector<FhirBundleEntry>
CreatePrescriptionService::CreateFhirMedicationFromPackage(const std::shared_ptr<PackageMedication> &package) const {
    auto medication = std::make_shared<FhirMedication>();
    SetCommonFhirMedication(*medication, *package);
    {
        std::vector<FhirCoding> codings{};
        {
            auto code = package->GetCode();
            auto value = code.getCode();
            if (!value.empty()) {
                auto display = code.getDisplay();
                codings.emplace_back("http://ehelse.no/fhir/CodeSystem/FEST", value, display);
                medication->SetName(display);
            }
        }
        {
            auto atc = package->GetAtc();
            if (!atc.empty()) {
                codings.emplace_back("http://www.whocc.no/atc", atc, package->GetAtcDisplay());
            }
        }
        FhirCodeableConcept code{codings};
        medication->SetCode(code);
    }
    {
        auto medicationDetails = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-medicationdetails");
        medicationDetails->AddExtension(std::make_shared<FhirValueExtension>("registreringstype", std::make_shared<FhirCodeableConceptValue>(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-festregistrationtype", "3", "Legemiddelpakning"))));
        {
            auto pkgInfos = package->GetPackageInfoPrescription();
            for (const auto &pi : pkgInfos) {
                auto info = std::make_shared<FhirExtension>("packinginfoprescription");
                info->AddExtension(std::make_shared<FhirValueExtension>("name", std::make_shared<FhirString>(pi.GetName())));
                info->AddExtension(std::make_shared<FhirValueExtension>("packingsize", std::make_shared<FhirString>(pi.GetPackingSize())));
                auto unit = pi.GetPackingUnit();
                info->AddExtension(std::make_shared<FhirValueExtension>("packingunit", std::make_shared<FhirCodeableConceptValue>(FhirCodeableConcept(unit.getSystem(), unit.getCode(), unit.getDisplay()))));
                medicationDetails->AddExtension(info);
            }
        }
        medication->AddExtension(medicationDetails);
    }
    medication->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Medication");
    medication->SetStatus(FhirStatus::ACTIVE);
    return CreateBundleEntryFromMedication(medication);
}

std::vector<FhirBundleEntry> CreatePrescriptionService::CreateFhirMedicationFromBrandName(
        const std::shared_ptr<BrandNameMedication> &brand) const {
    auto medication = std::make_shared<FhirMedication>();
    SetCommonFhirMedication(*medication, *brand);
    {
        std::vector<FhirCoding> codings{};
        {
            auto code = brand->GetCode();
            auto value = code.getCode();
            if (!value.empty()) {
                auto display = code.getDisplay();
                codings.emplace_back("http://ehelse.no/fhir/CodeSystem/FEST", value, display);
                medication->SetName(display);
            }
        }
        {
            auto atc = brand->GetAtc();
            if (!atc.empty()) {
                codings.emplace_back("http://www.whocc.no/atc", atc, brand->GetAtcDisplay());
            }
        }
        FhirCodeableConcept code{codings};
        medication->SetCode(code);
    }
    {
        auto medicationDetails = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-medicationdetails");
        medicationDetails->AddExtension(std::make_shared<FhirValueExtension>("registreringstype", std::make_shared<FhirCodeableConceptValue>(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-festregistrationtype", "2", "Legemiddelmerkevare"))));
        medication->AddExtension(medicationDetails);
    }
    medication->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Medication");
    medication->SetStatus(FhirStatus::ACTIVE);
    return CreateBundleEntryFromMedication(medication);
}

std::vector<FhirBundleEntry> CreatePrescriptionService::CreateFhirMedicationFromGeneric(
        const std::shared_ptr<GenericMedication> &generic) const {
    auto medication = std::make_shared<FhirMedication>();
    SetCommonFhirMedication(*medication, *generic);
    {
        std::vector<FhirCoding> codings{};
        {
            auto code = generic->GetCode();
            auto value = code.getCode();
            if (!value.empty()) {
                auto display = code.getDisplay();
                codings.emplace_back("http://ehelse.no/fhir/CodeSystem/FEST", value, display);
                medication->SetName(display);
            }
        }
        {
            auto atc = generic->GetAtc();
            if (!atc.empty()) {
                codings.emplace_back("http://www.whocc.no/atc", atc, generic->GetAtcDisplay());
            }
        }
        FhirCodeableConcept code{codings};
        medication->SetCode(code);
    }
    {
        auto medicationDetails = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-medicationdetails");
        medicationDetails->AddExtension(std::make_shared<FhirValueExtension>("registreringstype", std::make_shared<FhirCodeableConceptValue>(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-festregistrationtype", "2", "Legemiddelmerkevare"))));
        medication->AddExtension(medicationDetails);
    }
    medication->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Medication");
    medication->SetStatus(FhirStatus::ACTIVE);
    return CreateBundleEntryFromMedication(medication);
}

std::vector<FhirBundleEntry> CreatePrescriptionService::CreateFhirMedication(const std::shared_ptr<Medication> &medication) const {
    {
        auto magistralMedication = std::dynamic_pointer_cast<MagistralMedication>(medication);
        if (magistralMedication) {
            return CreateFhirMedicationFromMagistral(magistralMedication);
        }
        auto packageMedication = std::dynamic_pointer_cast<PackageMedication>(medication);
        if (packageMedication) {
            return CreateFhirMedicationFromPackage(packageMedication);
        }
        auto brandNameMedication = std::dynamic_pointer_cast<BrandNameMedication>(medication);
        if (brandNameMedication) {
            return CreateFhirMedicationFromBrandName(brandNameMedication);
        }
        auto genericMedication = std::dynamic_pointer_cast<GenericMedication>(medication);
        if (genericMedication) {
            return CreateFhirMedicationFromGeneric(genericMedication);
        }
    }
    return {};
}

FhirBundleEntry CreatePrescriptionService::CreateFhirMedicationStatement(const Prescription &prescription, std::vector<FhirBundleEntry> &practitioners) {
    std::string prescribedByReference{};
    std::string prescribedByDisplay{};
    {
        Person prescribedBy{};
        {
            PersonStorage personStorage{};
            auto prescribedById = prescription.GetPrescribedBy();
            if (prescribedById.empty()) {
                return {};
            }
            prescribedBy = personStorage.GetById(prescribedById);
            if (prescribedBy.GetId().empty() || prescribedBy.GetFodselsnummer().empty()) {
                return {};
            }
        }
        for (const auto &practitionerEntry : practitioners) {
            auto practitioner = std::dynamic_pointer_cast<FhirPerson>(practitionerEntry.GetResource());
            if (practitioner) {
                bool matching{false};
                for (const auto &identifier : practitioner->GetIdentifiers()) {
                    if (identifier.GetSystem() == "urn:oid:2.16.578.1.12.4.1.4.1" &&
                        identifier.GetValue() == prescribedBy.GetFodselsnummer()) {
                        matching = true;
                        break;
                    }
                }
                if (matching) {
                    prescribedByReference = practitionerEntry.GetFullUrl();
                    prescribedByDisplay = practitioner->GetDisplay();
                }
            }
        }
        if (prescribedByReference.empty()) {
            auto practitioner = std::make_shared<FhirPractitioner>();
            practitioner->SetActive(true);
            practitioner->SetBirthDate(prescribedBy.GetDateOfBirth());
            practitioner->SetGender(prescribedBy.GetGender() == PersonGender::FEMALE ? "female" : "male");
            {
                boost::uuids::random_generator generator;
                boost::uuids::uuid randomUUID = generator();
                auto id = boost::uuids::to_string(randomUUID);
                practitioner->SetId(id);
            }
            {
                std::vector<FhirIdentifier> identifiers{};
                auto hpr = prescribedBy.GetHpr();
                if (!hpr.empty()) {
                    FhirCodeableConcept type{"http://hl7.no/fhir/NamingSystem/HPR", "HPR-nummer", ""};
                    identifiers.emplace_back(type, "official", "urn:oid:2.16.578.1.12.4.1.4.4", hpr);
                }
                auto fnr = prescribedBy.GetFodselsnummer();
                if (!fnr.empty()) {
                    FhirCodeableConcept type{"http://hl7.no/fhir/NamingSystem/FNR", "FNR-nummer", ""};
                    identifiers.emplace_back(type, "official", "urn:oid:2.16.578.1.12.4.1.4.1", fnr);
                }
                practitioner->SetIdentifiers(identifiers);
            }
            practitioner->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Practitioner");
            {
                FhirName name{"official", prescribedBy.GetFamilyName(), prescribedBy.GetGivenName()};
                practitioner->SetName({name});
            }
            std::string fullUrl{"urn:uuid:"};
            {
                boost::uuids::random_generator generator;
                boost::uuids::uuid randomUUID = generator();
                auto id = boost::uuids::to_string(randomUUID);
                fullUrl.append(id);
            }
            practitioners.emplace_back(fullUrl, practitioner);
            prescribedByReference = std::move(fullUrl);
            prescribedByDisplay = practitioner->GetDisplay();
        }
    }
    auto medicationStatement = std::make_shared<FhirMedicationStatement>();
    {
        std::string dosingText{prescription.GetDosingText()};
        if (dosingText.empty()) {
            dosingText = prescription.GetDssn();
        }
        FhirDosage dosage{dosingText, 1};
        {
            auto use = prescription.GetUse();
            FhirCodeableConcept codeable{use.getSystem(), use.getCode(), use.getDisplay()};
            dosage.AddExtension(std::make_shared<FhirValueExtension>(
                "http://ehelse.no/fhir/StructureDefinition/sfm-use",
                std::make_shared<FhirCodeableConceptValue>(codeable)
            ));
        }
        {
            auto ext = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-application-area");
            ext->AddExtension(std::make_shared<FhirValueExtension>("text", std::make_shared<FhirString>(prescription.GetApplicationArea())));
            dosage.AddExtension(ext);
        }
        auto shortDose = prescription.GetShortDose();
        if (!shortDose.getCode().empty()) {
            FhirCodeableConcept codeable{shortDose.getSystem(), shortDose.getCode(), shortDose.getDisplay()};
            dosage.AddExtension(std::make_shared<FhirValueExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-shortdosage", std::make_shared<FhirCodeableConceptValue>(codeable)));
        }
        medicationStatement->AddDosage(dosage);
    }
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        auto id = boost::uuids::to_string(randomUUID);
        medicationStatement->SetId(id);
    }
    {
        auto pllId = prescription.GetPllId();
        if (!pllId.empty()) {
            FhirCodeableConcept type{"PLL"};
            FhirIdentifier identifier{type, "usual", pllId};
            medicationStatement->AddIdentifier(identifier);
        }
    }
    if (prescription.GetTypeOfPrescription().getCode() == "E") {
        FhirCodeableConcept type{"ReseptId"};
        FhirIdentifier identifier{type, "usual", prescription.GetId()};
        medicationStatement->AddIdentifier(identifier);
    }
    medicationStatement->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-MedicationStatement");
    medicationStatement->SetStatus(FhirStatus::ACTIVE);
    {
        auto reseptAmendment = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-reseptamendment");
        {
            auto value = prescription.GetPrescriptionDate();
            if (!value.empty()) {
                reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                        "reseptdate",
                        std::make_shared<FhirString>(value)
                ));
            }
        }
        {
            auto value = prescription.GetExpirationDate();
            if (!value.empty()) {
                reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                        "expirationdate",
                        std::make_shared<FhirString>(value)
                ));
            }
        }
        {
            auto value = prescription.GetFestUpdate();
            if (!value.empty()) {
                reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                        "festUpdate",
                        std::make_shared<FhirString>(value)
                ));
            }
        }
        // TODO
        reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                "guardiantransparencyreservation",
                std::make_shared<FhirBooleanValue>(false)
        ));
        // TODO
        reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                "indoctorsname",
                std::make_shared<FhirBooleanValue>(false)
        ));
        {
            auto value = prescription.GetDssn();
            if (!value.empty()) {
                reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                        "dssn",
                        std::make_shared<FhirString>(value)
                ));
            }
        }
        // TODO
        reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                "lockedresept",
                std::make_shared<FhirBooleanValue>(false)
        ));
        {
            auto amount = prescription.GetAmount();
            if (amount > 0.001) {
                auto amountUnit = prescription.GetAmountUnit();
                reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                        "amount",
                        std::make_shared<FhirQuantityValue>(FhirQuantity(amount, amountUnit))
                ));
            }
        }
        {
            auto value = prescription.GetNumberOfPackages();
            if (value > 0.001) {
                reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                        "numberofpackages",
                        std::make_shared<FhirDecimalValue>(value)
                ));
            }
        }
        {
            auto value = prescription.GetReit();
            if (!value.empty()) {
                reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                        "reit",
                        std::make_shared<FhirString>(value)
                ));
            }
        }
        {
            auto value = prescription.GetItemGroup();
            FhirCodeableConcept codeable{value.getSystem(), value.getCode(), value.getDisplay()};
            if (codeable.IsSet()) {
                reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                        "itemgroup",
                        std::make_shared<FhirCodeableConceptValue>(codeable)
                ));
            }
        }
        {
            // TODO
            Code value{};
            FhirCodeableConcept codeable{value.getSystem(), value.getCode(), value.getDisplay()};
            if (codeable.IsSet()) {
                reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                        "reimbursementparagraph",
                        std::make_shared<FhirCodeableConceptValue>(codeable)
                ));
            }
        }
        {
            auto value = prescription.GetRfStatus();
            FhirCodeableConcept codeable{value.getSystem(), value.getCode(), value.getDisplay()};
            if (codeable.IsSet()) {
                auto ext = std::make_shared<FhirExtension>("rfstatus");
                ext->AddExtension(std::make_shared<FhirValueExtension>(
                        "status",
                        std::make_shared<FhirCodeableConceptValue>(codeable)
                ));
                reseptAmendment->AddExtension(ext);
            }
        }
        {
            auto value = prescription.GetTypeOfPrescription();
            FhirCodeableConcept codeable{value.getSystem(), value.getCode(), value.getDisplay()};
            if (codeable.IsSet()) {
                reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                        "typeresept",
                        std::make_shared<FhirCodeableConceptValue>(codeable)
                ));
            }
        }
        {
            auto recallCode = prescription.GetRecallCode();
            if (!recallCode.getCode().empty()) {
                auto recallinfo = std::make_shared<FhirExtension>("recallinfo");
                recallinfo->AddExtension(std::make_shared<FhirValueExtension>("recallcode", std::make_shared<FhirCodeableConceptValue>(FhirCodeableConcept(recallCode.getSystem(), recallCode.getCode(), recallCode.getDisplay()))));
                reseptAmendment->AddExtension(recallinfo);
            }
        }
        medicationStatement->AddExtension(reseptAmendment);
    }
    {
        auto cessationTime = prescription.GetCessationTime();
        auto cessationReason = prescription.GetCessationReason();
        if (!cessationTime.empty()) {
            auto discontinuation = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-discontinuation");
            discontinuation->AddExtension(std::make_shared<FhirValueExtension>("timeDate", std::make_shared<FhirDateTimeValue>(cessationTime)));
            if (!cessationReason.getCode().empty()) {
                FhirCodeableConcept codeable{cessationReason.getSystem(), cessationReason.getCode(), cessationReason.getDisplay()};
                discontinuation->AddExtension(std::make_shared<FhirValueExtension>("reason", std::make_shared<FhirCodeableConceptValue>(codeable)));
            } else {
                discontinuation->AddExtension(std::make_shared<FhirValueExtension>("note", std::make_shared<FhirString>(cessationReason.getDisplay())));
            }
            medicationStatement->AddExtension(discontinuation);
        }
    }
    {
        auto regInfo = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-regInfo");
        {
            {
                FhirCodeableConcept codeable{
                        "http://ehelse.no/fhir/CodeSystem/sfm-medicationstatement-registration-status", "3",
                        "Godkjent"};
                regInfo->AddExtension(std::make_shared<FhirValueExtension>(
                        "status",
                        std::make_shared<FhirCodeableConceptValue>(codeable)
                ));
            }
            {
                FhirCodeableConcept codeable{"http://ehelse.no/fhir/CodeSystem/sfm-performer-roles", "1",
                                             "Forskrevet av"};
                regInfo->AddExtension(std::make_shared<FhirValueExtension>(
                        "type",
                        std::make_shared<FhirCodeableConceptValue>(codeable)
                ));
            }
            regInfo->AddExtension(std::make_shared<FhirValueExtension>(
                    "provider",
                    std::make_shared<FhirReference>(prescribedByReference, "http://ehelse.no/fhir/StructureDefinition/sfm-Practitioner", prescribedByDisplay)
            ));
            auto timestamp = prescription.GetPrescribedTimestamp();
            if (!timestamp.empty()) {
                regInfo->AddExtension(std::make_shared<FhirValueExtension>(
                        "timestamp",
                        std::make_shared<FhirDateTimeValue>(timestamp)
                ));
            }
        }
        medicationStatement->AddExtension(regInfo);
    }
    {
        auto ext = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-generic-substitution");
        ext->AddExtension(std::make_shared<FhirValueExtension>(
                "genericSubstitutionAccepted",
                std::make_shared<FhirBooleanValue>(prescription.IsGenericSubstitutionAcceptd())
        ));
        medicationStatement->AddExtension(ext);
    }
    std::string fullUrl{"urn:uuid:"};
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        auto id = boost::uuids::to_string(randomUUID);
        fullUrl.append(id);
    }
    return {fullUrl, medicationStatement};
}
