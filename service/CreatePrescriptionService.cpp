//
// Created by sigsegv on 2/22/24.
//

#include "CreatePrescriptionService.h"
#include <sfmbasisapi/fhir/medstatement.h>
#include <sfmbasisapi/fhir/bundle.h>
#include <sfmbasisapi/fhir/medication.h>
#include <sfmbasisapi/fhir/person.h>
#include <map>
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
    {
        auto codeCoding = medication->GetCode().GetCoding();
        if (codeCoding.empty()) {
            return {};
        }
        code = codeCoding[0];
    }
    if (code.GetCode() == "10") /* Magistrell */ {
        auto medicationObject = std::make_shared<MagistralMedication>();
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
        FhirCoding form{};
        {
            auto formCodings = medication->GetForm().GetCoding();
            if (formCodings.empty()) {
                return {};
            }
            form = formCodings[0];
        }
        medicationObject->SetForm({form.GetCode(), form.GetDisplay(), form.GetSystem()});
        auto extensions = medication->GetExtensions();
        for (const auto &extension : extensions) {
            auto url = extension->GetUrl();
            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
            std::shared_ptr<FhirValue> value{};
            if (valueExtension) {
                value = valueExtension->GetValue();
            }
            if (url == "http://hl7.no/fhir/StructureDefinition/no-basis-prescriptiongroup" && value.operator bool()) {
                auto codeableConceptValue = std::dynamic_pointer_cast<FhirCodeableConceptValue>(value);
                if (codeableConceptValue) {
                    auto codings = codeableConceptValue->GetCoding();
                    if (!codings.empty()) {
                        auto coding = codings[0];
                        medicationObject->SetPrescriptionGroup({coding.GetCode(), coding.GetDisplay(), coding.GetSystem()});
                    }
                }
            } else if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-name" && value.operator bool()) {
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
        auto extensions = dosage.GetExtensions();
        for (const auto &extension : extensions) {
            auto url = extension->GetUrl();
            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
            std::shared_ptr<FhirValue> value{};
            if (valueExtension) {
                value = valueExtension->GetValue();
            }
            if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-use") {
                auto codeableConcept = std::dynamic_pointer_cast<FhirCodeableConceptValue>(value);
                if (codeableConcept) {
                    auto codings = codeableConcept->GetCoding();
                    if (!codings.empty()) {
                        auto coding = codings[0];
                        prescription.SetUse({coding.GetCode(), coding.GetDisplay(), coding.GetSystem()});
                    }
                }
            } else if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-application-area") {
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
        if (typeText == "ReseptId") {
            prescription.SetId(identifier.GetValue());
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
        if (statementExtensionUrl == "http://ehelse.no/fhir/StructureDefinition/sfm-reseptamendment") {
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
        } else if (statementExtensionUrl == "http://ehelse.no/fhir/StructureDefinition/sfm-regInfo") {
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
        } else if (statementExtensionUrl == "http://ehelse.no/fhir/StructureDefinition/sfm-generic-substitution") {
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
        }
    }
    return prescription;
}