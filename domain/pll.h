//
// Created by sigsegv on 8/9/24.
//

#ifndef SFMBASISFAKER_PLL_H
#define SFMBASISFAKER_PLL_H

#include "code.h"
#include <string>
#include <vector>
#include <memory>

class FhirAllergyIntolerance;
class FhirBundleEntry;
class FhirBundle;

class PllAllergy {
private:
    Code code{};
    Code sourceOfInformation{};
    Code typeOfReaction{};
    Code verificationStatus{};
    std::string recordedBy{};
    std::string updatedTime{};
    std::string recordedDate{};
    std::string id{};
    bool inactiveIngredient{false};

public:
    constexpr PllAllergy() noexcept = default;
    constexpr PllAllergy(const PllAllergy &) = default;
    constexpr PllAllergy(PllAllergy &&) noexcept = default;
    constexpr PllAllergy &operator = (const PllAllergy &) = default;
    constexpr PllAllergy &operator = (PllAllergy &&) noexcept = default;
    [[nodiscard]] constexpr Code GetCode() const {
        return code;
    }
    constexpr void SetCode(Code code) {
        this->code = code;
    }
    [[nodiscard]] constexpr Code GetSourceOfInformation() const {
        return sourceOfInformation;
    }
    constexpr void SetSourceOfInformation(Code sourceOfInformation) {
        this->sourceOfInformation = sourceOfInformation;
    }
    [[nodiscard]] constexpr Code GetTypeOfReaction() const {
        return typeOfReaction;
    }
    constexpr void SetTypeOfReaction(Code typeOfReaction) {
        this->typeOfReaction = typeOfReaction;
    }
    [[nodiscard]] constexpr Code GetVerificationStatus() const {
        return verificationStatus;
    }
    constexpr void SetVerificationStatus(Code verificationStatus) {
        this->verificationStatus = verificationStatus;
    }
    [[nodiscard]] constexpr std::string GetRecordedBy() const {
        return recordedBy;
    }
    constexpr void SetRecordedBy(const std::string &recordedBy) {
        this->recordedBy = recordedBy;
    }
    [[nodiscard]] constexpr std::string GetUpdatedTime() const {
        return updatedTime;
    }
    constexpr void SetUpdatedTime(const std::string &updatedTime) {
        this->updatedTime = updatedTime;
    }
    [[nodiscard]] constexpr std::string GetRecordedDate() const {
        return recordedDate;
    }
    constexpr void SetRecordedDate(const std::string &recordedDate) {
        this->recordedDate = recordedDate;
    }
    [[nodiscard]] constexpr std::string GetId() const {
        return id;
    }
    constexpr void SetId(const std::string &id) {
        this->id = id;
    }
    [[nodiscard]] constexpr bool IsInactiveIngredient() const {
        return inactiveIngredient;
    }
    constexpr void SetInactiveIngredient(bool inactiveIngredient) {
        this->inactiveIngredient = inactiveIngredient;
    }
    std::string Serialize() const;
    static PllAllergy Parse(const std::string &json);
    std::shared_ptr<FhirAllergyIntolerance> ToFhir(std::vector<FhirBundleEntry> &practitioners) const;
    static PllAllergy FromFhir(const FhirAllergyIntolerance &fhir, const FhirBundle &bundle);
};

class Pll {
private:
    std::string id;
    std::string dateTime;
    std::vector<PllAllergy> allergies;
public:
    constexpr Pll() noexcept = default;
    constexpr Pll(const Pll &) = default;
    constexpr Pll(Pll &&) noexcept = default;
    constexpr Pll &operator = (const Pll &) = default;
    constexpr Pll &operator = (Pll &&) noexcept = default;
    [[nodiscard]] constexpr std::string GetId() const {
        return id;
    }

    constexpr void SetId(const std::string &id) {
        this->id = id;
    }

    [[nodiscard]] constexpr std::string GetDateTime() const {
        return dateTime;
    }

    constexpr void SetDateTime(const std::string &dateTime) {
        this->dateTime = dateTime;
    }

    [[nodiscard]] constexpr std::vector<PllAllergy> GetAllergies() const {
        return allergies;
    }

    constexpr void SetAllergies(const std::vector<PllAllergy> &allergies) {
        this->allergies = allergies;
    }

    constexpr void SetAllergies(std::vector<PllAllergy> &&allergies) {
        this->allergies = allergies;
    }

    constexpr bool IsValid() const {
        return (!id.empty()) && (!dateTime.empty());
    }

    std::string Serialize() const;
    static Pll Parse(const std::string &json);
};

#endif //SFMBASISFAKER_PLL_H
