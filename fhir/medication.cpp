//
// Created by jeo on 12/7/23.
//

#include <fhir/medication.h>

FhirIngredient FhirIngredient::Parse(const web::json::value &obj) {
    FhirIngredient ingredient{};
    if (!ingredient.ParseInline(obj)) {
        throw std::exception();
    }
    if (obj.has_object_field("itemReference")) {
        ingredient.itemReference = FhirReference::Parse(obj.at("itemReference"));
    }
    if (obj.has_boolean_field("isActive")) {
        ingredient.isActive = obj.at("isActive").as_bool();
    }
    if (obj.has_object_field("strength")) {
        ingredient.strength = FhirRatio::Parse(obj.at("strength"));
    }
    return ingredient;
}

web::json::value FhirIngredient::ToJson() const {
    web::json::value val = FhirExtendable::ToJson();
    if (itemReference.IsSet()) {
        val["itemReference"] = itemReference.ToJson();
    }
    val["isActive"] = isActive;
    if (strength.IsSet()) {
        val["strength"] = strength.ToJson();
    }
    return val;
}

FhirMedication FhirMedication::Parse(const web::json::value &obj) {
    FhirMedication medication{};
    if (!medication.ParseInline(obj)) {
        throw std::exception();
    }
    if (obj.has_object_field("code")) {
        medication.code = FhirCodeableConcept::Parse(obj.at("code"));
    }
    if (obj.has_object_field("form")) {
        medication.form = FhirCodeableConcept::Parse(obj.at("form"));
    }
    if (obj.has_object_field("amount")) {
        medication.amount = FhirRatio::Parse(obj.at("amount"));
    }
    if (obj.has_array_field("ingredient")) {
        for (const auto &ingredient : obj.at("ingredient").as_array()) {
            medication.ingredients.emplace_back(FhirIngredient::Parse(ingredient));
        }
        medication.hasIngredients = true;
    }
    return medication;
}

web::json::value FhirMedication::ToJson() const {
    web::json::value val = Fhir::ToJson();
    if (code.IsSet()) {
        val["code"] = code.ToJson();
    }
    if (form.IsSet()) {
        val["form"] = form.ToJson();
    }
    if (amount.IsSet()) {
        val["amount"] = amount.ToJson();
    }
    if (hasIngredients) {
        web::json::value ingArr = web::json::value::array(ingredients.size());
        for (size_t i = 0; i < ingredients.size(); i++) {
            ingArr[i] = ingredients[i].ToJson();
        }
        val["ingredient"] = ingArr;
    }
    return val;
}