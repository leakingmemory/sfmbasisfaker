//
// Created by sigsegv on 2/22/24.
//

#ifndef SFMBASISFAKER_PRESCRIPTION_H
#define SFMBASISFAKER_PRESCRIPTION_H

#include "code.h"
#include <string>
#include <memory>
#include <vector>

namespace web::json {
    class value;
}

class Medication {
private:
    Code code;
    Code prescriptionGroup;
    Code form;
    std::string atc;
    std::string atcDisplay;
public:
    Medication(const Code &code) : code(code) {}
    virtual ~Medication() = default;
    [[nodiscard]] Code GetCode() const {
        return code;
    }
    [[nodiscard]] Code GetPrescriptionGroup() const {
        return prescriptionGroup;
    }
    void SetPrescriptionGroup(const Code &prescriptionGroup) {
        this->prescriptionGroup = prescriptionGroup;
    }
    [[nodiscard]] Code GetForm() const {
        return form;
    }
    void SetForm(const Code &form) {
        this->form = form;
    }
    [[nodiscard]] std::string GetAtc() const {
        return atc;
    }
    [[nodiscard]] std::string GetAtcDisplay() const {
        return atcDisplay;
    }
    void SetAtc(const std::string &code, const std::string &display) {
        atc = code;
        atcDisplay = display;
    }

    virtual web::json::value Serialize() const;
    virtual void ParseInline(const web::json::value &json);
    static std::shared_ptr<Medication> Parse(const web::json::value &json);
};

class MagistralMedication : public Medication {
private:
    double amount;
    std::string amountUnit;
    std::string name;
    std::string recipe;
public:
    MagistralMedication() : Medication({"10", "Magistrell", "urn:oid:2.16.578.1.12.4.1.1.7424"}) {}
    double GetAmount() const {
        return amount;
    }
    void SetAmount(double amount) {
        this->amount = amount;
    }
    [[nodiscard]] std::string GetAmountUnit() const {
        return amountUnit;
    }
    void SetAmountUnit(const std::string &amountUnit) {
        this->amountUnit = amountUnit;
    }
    [[nodiscard]] std::string GetName() const {
        return name;
    }
    void SetName(const std::string &name) {
        this->name = name;
    }
    [[nodiscard]] std::string GetRecipe() const {
        return recipe;
    }
    void SetRecipe(const std::string &recipe) {
        this->recipe = recipe;
    }

    web::json::value Serialize() const override;
    void ParseInline(const web::json::value &json) override;
};

class PackingInfoPrescription {
private:
    std::string name{};
    std::string packingSize{};
    Code packingUnit{};
public:

    [[nodiscard]] std::string GetName() const {
        return name;
    }

    void SetName(const std::string &name) {
        this->name = name;
    }

    [[nodiscard]] std::string GetPackingSize() const {
        return packingSize;
    }

    void SetPackingSize(const std::string &packingSize) {
        this->packingSize = packingSize;
    }

    [[nodiscard]] Code GetPackingUnit() const {
        return packingUnit;
    }

    void SetPackingUnit(const Code &packingUnit) {
        this->packingUnit = packingUnit;
    }
    web::json::value Serialize() const;
    void ParseInline(const web::json::value &json);
};

class PackageMedication : public Medication {
private:
    std::vector<PackingInfoPrescription> packageInfoPrescription{};
public:
    PackageMedication(const std::string &varenr, const std::string &display) : Medication({varenr, display, "Varenummer"}) {}
    void SetPackageInfoPrescription(const std::vector<PackingInfoPrescription> &packageInfoPrescription) {
        this->packageInfoPrescription = packageInfoPrescription;
    }
    [[nodiscard]] std::vector<PackingInfoPrescription> GetPackageInfoPrescription() const {
        return packageInfoPrescription;
    }
    web::json::value Serialize() const override;
    void ParseInline(const web::json::value &json) override;
};

class BrandNameMedication : public Medication {
public:
    BrandNameMedication(const std::string &id, const std::string &display) : Medication({id, display, "BrandName"}) {}
};

class GenericMedication : public Medication {
public:
    GenericMedication(const std::string &id, const std::string &display) : Medication({id, display, "Generic"}) {}
};

class Prescription {
private:
    std::string id{};
    std::string pllId{};
    std::string previousId{};
    std::shared_ptr<Medication> medication{};
    Code use{};
    std::string applicationArea{};
    std::string prescriptionDate{};
    std::string expirationDate{};
    std::string festUpdate{};
    std::string dssn{};
    std::string amountUnit{};
    double amount{0.00000};
    double numberOfPackages{0.00000};
    std::string reit{};
    Code itemGroup{};
    Code rfStatus{};
    std::string lastChanged{};
    Code typeOfPrescription{};
    std::string prescribedBy{};
    std::string prescribedTimestamp{};
    std::string treatmentStartedBy{};
    std::string treatmentStartedTimestamp{};
    std::string patient{};
    Code recallCode{};
    bool genericSubstitutionAccepted{false};
public:
    [[nodiscard]] std::string GetId() const {
        return id;
    }
    void SetId(const std::string &id) {
        this->id = id;
    }
    [[nodiscard]] std::string GetPllId() const {
        return pllId;
    }
    void SetPllId(const std::string &pllId) {
        this->pllId = pllId;
    }
    [[nodiscard]] std::string GetPreviousId() const {
        return previousId;
    }
    void SetPreviousId(const std::string &prevId) {
        this->previousId = prevId;
    }
    [[nodiscard]] std::shared_ptr<Medication> GetMedication() const {
        return medication;
    }
    void SetMedication(const std::shared_ptr<Medication> &medication) {
        this->medication = medication;
    }
    [[nodiscard]] Code GetUse() const {
        return use;
    }
    void SetUse(const Code &use) {
        this->use = use;
    }
    [[nodiscard]] std::string GetApplicationArea() const {
        return applicationArea;
    }
    void SetApplicationArea(const std::string &applicationArea) {
        this->applicationArea = applicationArea;
    }
    [[nodiscard]] std::string GetPrescriptionDate() const {
        return prescriptionDate;
    }
    void SetPrescriptionDate(const std::string &prescriptionDate) {
        this->prescriptionDate = prescriptionDate;
    }
    [[nodiscard]] std::string GetExpirationDate() const {
        return expirationDate;
    }
    void SetExpirationDate(const std::string &expirationDate) {
        this->expirationDate = expirationDate;
    }
    [[nodiscard]] std::string GetFestUpdate() const {
        return festUpdate;
    }
    void SetFestUpdate(const std::string &festUpdate) {
        this->festUpdate = festUpdate;
    }
    [[nodiscard]] std::string GetDssn() const {
        return dssn;
    }
    void SetDssn(const std::string &dssn) {
        this->dssn = dssn;
    }
    [[nodiscard]] std::string GetAmountUnit() const {
        return amountUnit;
    }
    void SetAmountUnit(const std::string &unit) {
        amountUnit = unit;
    }
    [[nodiscard]] double GetAmount() const {
        return amount;
    }
    void SetAmount(double a) {
        amount = a;
    }
    [[nodiscard]] double GetNumberOfPackages() const {
        return numberOfPackages;
    }
    void SetNumberOfPackages(double numberOfPackages) {
        this->numberOfPackages = numberOfPackages;
    }
    [[nodiscard]] std::string GetReit() const {
        return reit;
    }
    void SetReit(const std::string &reit) {
        this->reit = reit;
    }
    [[nodiscard]] Code GetItemGroup() const {
        return itemGroup;
    }
    void SetItemGroup(const Code &itemGroup) {
        this->itemGroup = itemGroup;
    }
    [[nodiscard]] Code GetRfStatus() const {
        return rfStatus;
    }
    void SetRfStatus(const Code &rfStatus) {
        this->rfStatus = rfStatus;
    }
    [[nodiscard]] std::string GetLastChanged() const {
        return lastChanged;
    }
    void SetLastChanged(const std::string &lastChanged) {
        this->lastChanged = lastChanged;
    }
    [[nodiscard]] Code GetTypeOfPrescription() const {
        return typeOfPrescription;
    }
    void SetTypeOfPrescription(const Code &typeOfPrescription) {
        this->typeOfPrescription = typeOfPrescription;
    }
    [[nodiscard]] std::string GetPrescribedBy() const {
        return prescribedBy;
    }
    void SetPrescribedBy(const std::string &prescribedBy) {
        this->prescribedBy = prescribedBy;
    }
    [[nodiscard]] std::string GetPrescribedTimestamp() const {
        return prescribedTimestamp;
    }
    void SetPrescribedTimestamp(const std::string &prescribedTimestamp) {
        this->prescribedTimestamp = prescribedTimestamp;
    }
    [[nodiscard]] std::string GetTreatmentStartedBy() const {
        return prescribedBy;
    }
    void SetTreatmentStartedBy(const std::string &prescribedBy) {
        this->prescribedBy = prescribedBy;
    }
    [[nodiscard]] std::string GetTreatmentStartedTimestamp() const {
        return prescribedTimestamp;
    }
    void SetTreatmentStartedTimestamp(const std::string &prescribedTimestamp) {
        this->prescribedTimestamp = prescribedTimestamp;
    }
    [[nodiscard]] std::string GetPatient() const {
        return patient;
    }
    void SetPatient(const std::string &patient) {
        this->patient = patient;
    }
    bool IsGenericSubstitutionAcceptd() const {
        return genericSubstitutionAccepted;
    }
    void SetGenericSubstitutionAccepted(bool genericSubstitutionAccepted) {
        this->genericSubstitutionAccepted = genericSubstitutionAccepted;
    }
    [[nodiscard]] Code GetRecallCode() const {
        return recallCode;
    }
    void SetRecallCode(const Code &recallCode) {
        this->recallCode = recallCode;
    }

    std::string Serialize() const;
    static Prescription Parse(const std::string &json);
};

#endif //SFMBASISFAKER_PRESCRIPTION_H