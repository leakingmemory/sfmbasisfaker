//
// Created by jeo on 5/22/25.
//

#ifndef SFMBASISFAKER_PHARMACYCONTROLLER_H
#define SFMBASISFAKER_PHARMACYCONTROLLER_H

#include <string>
#include <vector>
#include <optional>

struct PharmacyPatient {
    std::string id{};
    std::string firstName{};
    std::string lastName{};
};

struct PaperDispatchData {
    std::string prescriptionGroup;
    std::string registrationType;
    std::string name;
    std::string nameFormStrength;
    std::string packingSize;
    std::string packingUnitCode;
    std::string packingUnitDisplay;
    std::string productNumber;
    std::string atcCode;
    std::string atcDisplay;
    std::string formCode;
    std::string formDisplay;
    std::optional<double> amount;
    std::string amountUnit;
    std::string amountText;
    /* */
    std::string dssn;
    std::optional<double> numberOfPackages;
    std::string reit;
    std::string itemGroupCode;
    std::string itemGroupDisplay;
    std::string prescriptionTypeCode;
    std::string prescriptionTypeDisplay;
    std::string prescriptionId;
    bool genericSubstitutionAccepted;
    /* */
    std::string prescribedByHpr;
    std::string prescribedByGivenName;
    std::string prescribedByFamilyName;
    /* */
    std::string dispatcherHerId;
    std::string dispatcherName;
    /* */
    bool substitutionReservationCustomer;
    std::string dispatchMsgId;
    double quantity;
    std::string whenHandedOver;
};

class PharmacyController {
public:
    std::vector<PharmacyPatient> GetPatients();
    void PaperDispense(const std::string &patientId, const PaperDispatchData &paperDispatch);
};


#endif //SFMBASISFAKER_PHARMACYCONTROLLER_H
