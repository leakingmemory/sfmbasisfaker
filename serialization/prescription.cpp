//
// Created by sigsegv on 2/29/24.
//

#include "../domain/prescription.h"
#include <cpprest/json.h>

web::json::value Medication::Serialize() const {
    auto obj = web::json::value::object();
    obj["code"] = code.Serialize();
    obj["prescriptionGroup"] = prescriptionGroup.Serialize();
    obj["form"] = form.Serialize();
    obj["atc"] = web::json::value::string(atc);
    obj["atcDisplay"] = web::json::value::string(atcDisplay);
    return obj;
}

void Medication::ParseInline(const web::json::value &json) {
    if (json.has_object_field("prescriptionGroup")) {
        prescriptionGroup = Code::Parse(json.at("prescriptionGroup"));
    }
    if (json.has_object_field("form")) {
        form = Code::Parse(json.at("form"));
    }
    if (json.has_string_field("atc")) {
        atc = json.at("atc").as_string();
    }
    if (json.has_string_field("atcDisplay")) {
        atcDisplay = json.at("atcDisplay").as_string();
    }
}

std::shared_ptr<Medication> Medication::Parse(const web::json::value &json) {
    std::shared_ptr<Medication> medication{};
    if (json.has_object_field("code")) {
        auto code = Code::Parse(json.at("code"));
        auto systemValue = code.getSystem();
        auto codeValue = code.getCode();
        if (systemValue == "urn:oid:2.16.578.1.12.4.1.1.7424" && codeValue == "10") {
            medication = std::make_shared<MagistralMedication>();
        } else if (systemValue == "Varenummer") {
            medication = std::make_shared<PackageMedication>(codeValue, code.getDisplay());
        } else if (systemValue == "BrandName") {
            medication = std::make_shared<BrandNameMedication>(codeValue, code.getDisplay());
        } else if (systemValue == "Generic") {
            medication = std::make_shared<GenericMedication>(codeValue, code.getDisplay());
        }
        if (medication) {
            medication->ParseInline(json);
        }
    }
    return medication;
}

web::json::value MagistralMedication::Serialize() const {
    auto obj = Medication::Serialize();
    obj["amount"] = web::json::value::number(amount);
    obj["amountUnit"] = web::json::value::string(amountUnit);
    obj["name"] = web::json::value::string(name);
    obj["recipe"] = web::json::value::string(recipe);
    return obj;
}

void MagistralMedication::ParseInline(const web::json::value &json) {
    Medication::ParseInline(json);
    if (json.has_number_field("amount")) {
        amount = json.at("amount").as_double();
    }
    if (json.has_string_field("amountUnit")) {
        amountUnit = json.at("amountUnit").as_string();
    }
    if (json.has_string_field("name")) {
        name = json.at("name").as_string();
    }
    if (json.has_string_field("recipe")) {
        recipe = json.at("recipe").as_string();
    }
}

web::json::value PackingInfoPrescription::Serialize() const {
    auto obj = web::json::value::object();
    obj["name"] = web::json::value::string(name);
    obj["packingsize"] = web::json::value::string(packingSize);
    obj["packingunit"] = packingUnit.Serialize();
    return obj;
}

void PackingInfoPrescription::ParseInline(const web::json::value &json) {
    if (json.has_string_field("name")) {
        name = json.at("name").as_string();
    }
    if (json.has_string_field("packingsize")) {
        packingSize = json.at("packingsize").as_string();
    }
    if (json.has_object_field("packingunit")) {
        packingUnit = Code::Parse(json.at("packingunit"));
    }
}

web::json::value PackageMedication::Serialize() const {
    auto obj = Medication::Serialize();
    auto infoArr = web::json::value::array(packageInfoPrescription.size());
    typeof(packageInfoPrescription.size()) i = 0;
    for (const auto &pi : packageInfoPrescription) {
        infoArr[i++] = pi.Serialize();
    }
    obj["packinginfoprescription"] = infoArr;
    return obj;
}

void PackageMedication::ParseInline(const web::json::value &json) {
    Medication::ParseInline(json);
    if (json.has_array_field("packinginfoprescription")) {
        auto arr = json.at("packinginfoprescription").as_array();
        for (const auto &item : arr) {
            auto &pi = packageInfoPrescription.emplace_back();
            pi.ParseInline(item);
        }
    }
}

std::string Prescription::Serialize() const {
    auto obj = web::json::value::object();
    obj["id"] = web::json::value::string(id);
    if (medication) {
        obj["medication"] = medication->Serialize();
    }
    obj["use"] = use.Serialize();
    obj["applicationArea"] = web::json::value::string(applicationArea);
    obj["prescriptionDate"] = web::json::value::string(prescriptionDate);
    obj["expirationDate"] = web::json::value::string(expirationDate);
    obj["festUpdate"] = web::json::value::string(festUpdate);
    obj["dssn"] = web::json::value::string(dssn);
    obj["amount"] = web::json::value::number(amount);
    obj["amountUnit"] = web::json::value::string(amountUnit);
    obj["numberOfPackages"] = web::json::value::number(numberOfPackages);
    obj["reit"] = web::json::value::string(reit);
    obj["itemGroup"] = itemGroup.Serialize();
    obj["rfStatus"] = rfStatus.Serialize();
    obj["lastChanged"] = web::json::value::string(lastChanged);
    obj["typeOfPrescription"] = typeOfPrescription.Serialize();
    obj["prescribedBy"] = web::json::value::string(prescribedBy);
    obj["prescribedTimestamp"] = web::json::value::string(prescribedTimestamp);
    obj["patient"] = web::json::value::string(patient);
    obj["genericSubstitutionAccepted"] = web::json::value::boolean(genericSubstitutionAccepted);
    obj["recallCode"] = recallCode.Serialize();
    return obj.serialize();
}

Prescription Prescription::Parse(const std::string &json) {
    Prescription prescription{};
    auto obj = web::json::value::parse(json);
    if (obj.has_string_field("id")) {
        prescription.id = obj.at("id").as_string();
    }
    if (obj.has_object_field("medication")) {
        prescription.medication = Medication::Parse(obj.at("medication"));
    }
    if (obj.has_object_field("use")) {
        prescription.use = Code::Parse(obj.at("use"));
    }
    if (obj.has_string_field("applicationArea")) {
        prescription.applicationArea = obj.at("applicationArea").as_string();
    }
    if (obj.has_string_field("prescriptionDate")) {
        prescription.prescriptionDate = obj.at("prescriptionDate").as_string();
    }
    if (obj.has_string_field("expirationDate")) {
        prescription.expirationDate = obj.at("expirationDate").as_string();
    }
    if (obj.has_string_field("festUpdate")) {
        prescription.festUpdate = obj.at("festUpdate").as_string();
    }
    if (obj.has_string_field("dssn")) {
        prescription.dssn = obj.at("dssn").as_string();
    }
    if (obj.has_string_field("amountUnit")) {
        prescription.amountUnit = obj.at("amountUnit").as_string();
    }
    if (obj.has_number_field("amount")) {
        prescription.amount = obj.at("amount").as_double();
    }
    if (obj.has_number_field("numberOfPackages")) {
        prescription.numberOfPackages = obj.at("numberOfPackages").as_double();
    }
    if (obj.has_string_field("reit")) {
        prescription.reit = obj.at("reit").as_string();
    }
    if (obj.has_object_field("itemGroup")) {
        prescription.itemGroup = Code::Parse(obj.at("itemGroup"));
    }
    if (obj.has_object_field("rfStatus")) {
        prescription.rfStatus = Code::Parse(obj.at("rfStatus"));
    }
    if (obj.has_string_field("lastChanged")) {
        prescription.lastChanged = obj.at("lastChanged").as_string();
    }
    if (obj.has_object_field("typeOfPrescription")) {
        prescription.typeOfPrescription = Code::Parse(obj.at("typeOfPrescription"));
    }
    if (obj.has_string_field("prescribedBy")) {
        prescription.prescribedBy = obj.at("prescribedBy").as_string();
    }
    if (obj.has_string_field("prescribedTimestamp")) {
        prescription.prescribedTimestamp = obj.at("prescribedTimestamp").as_string();
    }
    if (obj.has_string_field("patient")) {
        prescription.patient = obj.at("patient").as_string();
    }
    if (obj.has_boolean_field("genericSubstitutionAccepted")) {
        prescription.genericSubstitutionAccepted = obj.at("genericSubstitutionAccepted").as_bool();
    }
    if (obj.has_object_field("recallCode")) {
        prescription.recallCode = Code::Parse(obj.at("recallCode"));
    }
    return prescription;
}
