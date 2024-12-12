//
// Created by sigsegv on 8/9/24.
//

#include "../domain/pll.h"
#include <cpprest/json.h>

std::string PllAllergy::Serialize() const {
    auto obj = web::json::value::object();
    obj["code"] = code.Serialize();
    obj["sourceOfInformation"] = sourceOfInformation.Serialize();
    obj["typeOfReaction"] = typeOfReaction.Serialize();
    obj["verificationStatus"] = verificationStatus.Serialize();
    obj["recordedBy"] = web::json::value::string(recordedBy);
    obj["updatedTime"] = web::json::value::string(updatedTime);
    obj["recordedDate"] = web::json::value::string(recordedDate);
    obj["inactiveIngredient"] = web::json::value::boolean(inactiveIngredient);
    return obj.serialize();
}

PllAllergy PllAllergy::Parse(const std::string &json) {
    PllAllergy allergy{};
    auto obj = web::json::value::parse(json);
    if (obj.has_object_field("code")) {
        allergy.code = Code::Parse(obj.at("code"));
    }
    if (obj.has_object_field("sourceOfInformation")) {
        allergy.sourceOfInformation = Code::Parse(obj.at("sourceOfInformation"));
    }
    if (obj.has_object_field("typeOfReaction")) {
        allergy.typeOfReaction = Code::Parse(obj.at("typeOfReaction"));
    }
    if (obj.has_object_field("verificationStatus")) {
        allergy.verificationStatus = Code::Parse(obj.at("verificationStatus"));
    }
    if (obj.has_string_field("recordedBy")) {
        allergy.recordedBy = obj.at("recordedBy").as_string();
    }
    if (obj.has_string_field("updatedTime")) {
        allergy.updatedTime = obj.at("updatedTime").as_string();
    }
    if (obj.has_string_field("recordedDate")) {
        allergy.recordedDate = obj.at("recordedDate").as_string();
    }
    if (obj.has_boolean_field("inactiveIngredient")) {
        allergy.inactiveIngredient = obj.at("inactiveIngredient").as_bool();
    }
    return allergy;
}

std::string Pll::Serialize() const {
    auto obj = web::json::value::object();
    obj["id"] = web::json::value::string(id);
    obj["datetime"] = web::json::value::string(dateTime);
    auto allergiesArr = web::json::value::array(allergies.size());
    for (typeof(allergies.size()) i = 0; i < allergies.size(); i++) {
        allergiesArr[i] = web::json::value::parse(allergies[i].Serialize());
    }
    obj["allergies"] = allergiesArr;
    return obj.serialize();
}

Pll Pll::Parse(const std::string &json) {
    Pll pll{};
    auto obj = web::json::value::parse(json);
    if (obj.has_string_field("id")) {
        pll.id = obj.at("id").as_string();
    }
    if (obj.has_string_field("datetime")) {
        pll.dateTime = obj.at("datetime").as_string();
    }
    if (obj.has_array_field("allergies")) {
        for (const auto &allergyObj : obj.at("allergies").as_array()) {
            if (!allergyObj.is_object()) {
                continue;
            }
            pll.allergies.emplace_back(PllAllergy::Parse(allergyObj.serialize()));
        }
    }
    return pll;
}
