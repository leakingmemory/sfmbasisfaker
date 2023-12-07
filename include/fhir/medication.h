//
// Created by jeo on 12/7/23.
//

#ifndef SFMBASISFAKER_MEDICATION_H
#define SFMBASISFAKER_MEDICATION_H

#include "value.h"

class FhirIngredient : public FhirExtendable {
private:
    FhirReference itemReference{};
    FhirRatio strength{};
    bool isActive{false};
public:
    static FhirIngredient Parse(const web::json::value &);
    web::json::value ToJson() const;
    [[nodiscard]] FhirReference GetItemReference() const {
        return itemReference;
    }
    [[nodiscard]] FhirRatio GetStrength() const {
        return strength;
    }
    bool IsActive() const {
        return isActive;
    }
};

class FhirMedication : public Fhir {
    FhirCodeableConcept code{};
    FhirCodeableConcept form{};
    FhirRatio amount{};
    std::vector<FhirIngredient> ingredients{};
    bool hasIngredients{false};
public:
    static FhirMedication Parse(const web::json::value &);
    web::json::value ToJson() const;
    [[nodiscard]] FhirCodeableConcept GetCode() const {
        return code;
    }
    [[nodiscard]] FhirCodeableConcept GetForm() const {
        return form;
    }
    [[nodiscard]] FhirRatio GetAmount() const {
        return amount;
    }
    [[nodiscard]] std::vector<FhirIngredient> GetIngredients() const {
        return ingredients;
    }
};

#endif //SFMBASISFAKER_MEDICATION_H
