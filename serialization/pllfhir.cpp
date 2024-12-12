//
// Created by sigsegv on 12/6/24.
//

#include "../domain/pll.h"
#include "../domain/person.h"
#include <sfmbasisapi/fhir/allergy.h>
#include "../service/CreatePrescriptionService.h"

std::shared_ptr<FhirAllergyIntolerance> PllAllergy::ToFhir(std::vector<FhirBundleEntry> &practitioners) const {
    CreatePrescriptionService createPrescriptionService{};
    auto recordedBy = createPrescriptionService.GetPractitionerRef(this->recordedBy, practitioners);
    if (recordedBy.id.empty()) {
        return {};
    }
    auto allergy = std::make_shared<FhirAllergyIntolerance>();
    allergy->SetCategories({"medication"});
    allergy->SetCriticality("high");
    allergy->SetProfile("http://nhn.no/kj/fhir/StructureDefinition/KjAllergyIntolerance");
    {
        FhirCodeableConcept codeableConcept{code.getSystem(), code.getCode(), code.getDisplay()};
        if (inactiveIngredient) {
            codeableConcept.AddExtension(std::make_shared<FhirValueExtension>("http://nhn.no/kj/fhir/StructureDefinition/KjInactiveIngredient", std::make_shared<FhirBooleanValue>(true)));
        }
        allergy->SetCode(codeableConcept);
    }
    {
        FhirCoding coding{sourceOfInformation.getSystem(), sourceOfInformation.getCode(), sourceOfInformation.getDisplay()};
        allergy->AddExtension(
                std::make_shared<FhirValueExtension>("http://nhn.no/kj/fhir/StructureDefinition/KjSourceOfInformation",
                                                     std::make_shared<FhirCodingValue>(coding)));
    }
    {
        FhirCodeableConcept codeableConcept{typeOfReaction.getSystem(), typeOfReaction.getCode(), typeOfReaction.getDisplay()};
        std::vector<FhirReaction> reactions{};
        reactions.emplace_back().SetManifestation({codeableConcept});
        allergy->SetReactions(reactions);
    }
    {
        {
            FhirCodeableConcept codeableConcept{verificationStatus.getSystem(), verificationStatus.getCode(),
                                                verificationStatus.getDisplay()};
            allergy->SetVerificationStatus(codeableConcept);
        }
        auto active = verificationStatus.getCode() != "refuted" && verificationStatus.getCode() != "entered-in-error";
        FhirCodeableConcept codeableConcept{"http://terminology.hl7.org/CodeSystem/allergyintolerance-clinical", active ? "active" : "inactive", active ? "Active" : "Inactive"};
        allergy->SetClinicalStatus(codeableConcept);
    }
    allergy->SetRecorder(FhirReference(recordedBy.id, "http://ehelse.no/fhir/StructureDefinition/sfm-Practitioner", recordedBy.name));
    allergy->AddExtension(std::make_shared<FhirValueExtension>("http://nhn.no/kj/fhir/StructureDefinition/KjUpdatedDateTime", std::make_shared<FhirDateTimeValue>(updatedTime)));
    allergy->SetRecordedDate(recordedDate);
    allergy->SetId(id);
    allergy->SetIdentifiers({FhirIdentifier("official", id)});
    return allergy;
}

PllAllergy PllAllergy::FromFhir(const FhirAllergyIntolerance &fhir, const FhirBundle &bundle) {
    CreatePrescriptionService createPrescriptionService{};
    PllAllergy allergy{};
    {
        auto codeable = fhir.GetCode();
        {
            auto codings = codeable.GetCoding();
            if (codings.empty()) {
                return {};
            }
            allergy.code = {codings[0].GetCode(), codings[0].GetDisplay(), codings[0].GetSystem()};
        }
        bool inactiveSubstance{false};
        for (const auto &extension : codeable.GetExtensions()) {
            auto url = extension->GetUrl();
            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
            if (url == "http://nhn.no/kj/fhir/structuredefinition/kjinactiveingredient") {
                auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                if (valueExtension) {
                    auto value = std::dynamic_pointer_cast<FhirBooleanValue>(valueExtension->GetValue());
                    if (value) {
                        inactiveSubstance = value->IsTrue();
                    }
                }
            }
        }
        allergy.inactiveIngredient = inactiveSubstance;
    }
    for (const auto &extension : fhir.GetExtensions()) {
        auto url = extension->GetUrl();
        std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
        if (url == "http://nhn.no/kj/fhir/structuredefinition/kjsourceofinformation") {
            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
            if (valueExtension) {
                auto value = std::dynamic_pointer_cast<FhirCodingValue>(valueExtension->GetValue());
                if (value) {
                    allergy.sourceOfInformation = {value->GetCode(), value->GetDisplay(), value->GetSystem()};
                }
            }
            continue;
        }
        if (url == "http://nhn.no/kj/fhir/structuredefinition/kjupdateddatetime") {
            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
            if (valueExtension) {
                auto value = std::dynamic_pointer_cast<FhirDateTimeValue>(valueExtension->GetValue());
                if (value) {
                    allergy.updatedTime = value->GetDateTime();
                }
            }
        }
    }
    {
        auto reactions = fhir.GetReactions();
        if (!reactions.empty()) {
            auto manifestations = reactions[0].GetManifestations();
            if (!manifestations.empty()) {
                auto codings = manifestations[0].GetCoding();
                if (!codings.empty()) {
                    allergy.typeOfReaction = {codings[0].GetCode(), codings[0].GetDisplay(), codings[0].GetSystem()};
                }
            }
        }
    }
    {
        auto codings = fhir.GetVerificationStatus().GetCoding();
        if (!codings.empty()) {
            allergy.verificationStatus = {codings[0].GetCode(), codings[0].GetDisplay(), codings[0].GetSystem()};
        }
    }
    {
        auto person = createPrescriptionService.GetPerson(bundle, fhir.GetRecorder().GetReference());
        allergy.recordedBy = person.GetId();
    }
    allergy.recordedDate = fhir.GetRecordedDate();
    allergy.id = fhir.GetId();
    return allergy;
}