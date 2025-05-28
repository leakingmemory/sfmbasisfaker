//
// Created by jeo on 5/22/25.
//

#include "PharmacyController.h"
#include "../service/PersonStorage.h"
#include "../domain/person.h"
#include "../domain/prescription.h"
#include "../service/PrescriptionStorage.h"
#include <exception>
#include <iostream>

std::vector<PharmacyPatient> PharmacyController::GetPatients() {
    std::vector<PharmacyPatient> patients{};
    PersonStorage personStorage{};
    for (auto person : personStorage.GetPersons()) {
        PharmacyPatient pharmacyPatient{.id = person.GetId(), .firstName = person.GetGivenName(), .lastName = person.GetFamilyName()};
        patients.emplace_back(pharmacyPatient);
    }
    return patients;
}

void PharmacyController::PaperDispense(const std::string &patientId, const PaperDispatchData &paperDispatchData) {
    PersonStorage personStorage{};
    auto patient = personStorage.GetById(patientId);
    if (patient.GetDateOfBirth().empty() && patient.GetGivenName().empty() && patient.GetFamilyName().empty()) {
        std::cerr << "Patient not found\n";
        throw std::exception();
    }
    std::shared_ptr<Medication> medication{};
    if (paperDispatchData.registrationType == "3") {
        auto packageMedication = std::make_shared<PackageMedication>(paperDispatchData.productNumber, paperDispatchData.nameFormStrength);
        PackingInfoPrescription packingInfoPrescription;
        packingInfoPrescription.SetName(paperDispatchData.name);
        packingInfoPrescription.SetPackingSize(paperDispatchData.packingSize);
        packingInfoPrescription.SetPackingUnit({paperDispatchData.packingUnitCode, paperDispatchData.packingUnitDisplay, "urn:oid:2.16.578.1.12.4.1.1.7452"});
        packageMedication->SetPackageInfoPrescription({packingInfoPrescription});
        medication = packageMedication;
    } else {
        std::cerr << "Unsupported registration type\n";
        throw std::exception();
    }
    {
        std::string display;
        if (paperDispatchData.prescriptionGroup == "A") {
            display = "Reseptgruppe A";
        } else if (paperDispatchData.prescriptionGroup == "B") {
            display = "Reseptgruppe B";
        } else if (paperDispatchData.prescriptionGroup == "C") {
            display = "Reseptgruppe C";
        } else if (paperDispatchData.prescriptionGroup == "CF") {
            display = "Reseptgruppe CF";
        } else if (paperDispatchData.prescriptionGroup == "F") {
            display = "Reseptgruppe F";
        } else if (paperDispatchData.prescriptionGroup == "K") {
            display = "Kosttilskudd";
        } else {
            std::cerr << "Invalid prescription group code\n";
            throw std::exception();
        }
        medication->SetPrescriptionGroup({paperDispatchData.prescriptionGroup, display, "urn:oid:2.16.578.1.12.4.1.1.7421"});
    }
    medication->SetForm(Code{paperDispatchData.formCode, paperDispatchData.formDisplay, "urn:oid:2.16.578.1.12.4.1.1.7448"});
    medication->SetAtc(paperDispatchData.atcCode, paperDispatchData.atcDisplay);
    medication->SetAmount(paperDispatchData.amount);
    medication->SetAmountUnit(paperDispatchData.amountUnit);
    medication->SetAmountText(paperDispatchData.amountText);

    PaperDispatch paperDispatch{};
    paperDispatch.SetId(paperDispatchData.prescriptionId);
    paperDispatch.SetMedication(medication);
    paperDispatch.SetDssn(paperDispatchData.dssn);
    paperDispatch.SetNumberOfPackages(paperDispatchData.numberOfPackages);
    paperDispatch.SetReit(paperDispatchData.reit);
    paperDispatch.SetItemGroupCode(paperDispatchData.itemGroupCode);
    paperDispatch.SetItemGroupDisplay(paperDispatchData.itemGroupDisplay);
    paperDispatch.SetPrescriptionTypeCode(paperDispatchData.prescriptionTypeCode);
    paperDispatch.SetPrescriptionTypeDisplay(paperDispatchData.prescriptionTypeDisplay);
    paperDispatch.SetPrescriptionId(paperDispatchData.prescriptionId);
    paperDispatch.SetGenericSubstitutionAccepted(paperDispatchData.genericSubstitutionAccepted);
    paperDispatch.SetPrescribedByHpr(paperDispatchData.prescribedByHpr);
    paperDispatch.SetPrescribedByGivenName(paperDispatchData.prescribedByGivenName);
    paperDispatch.SetPrescribedByFamilyName(paperDispatchData.prescribedByFamilyName);
    paperDispatch.SetDispatcherHerId(paperDispatchData.dispatcherHerId);
    paperDispatch.SetDispatcherName(paperDispatchData.dispatcherName);
    paperDispatch.SetSubstitutionReservationCustomer(paperDispatchData.substitutionReservationCustomer);
    paperDispatch.SetDispatchMsgId(paperDispatchData.dispatchMsgId);
    paperDispatch.SetQuantity(paperDispatchData.quantity);
    paperDispatch.SetWhenHandedOver(paperDispatchData.whenHandedOver);

    auto paperDispatchMap = PrescriptionStorage::LoadPaperDispatchMap(patientId);
    auto id = PrescriptionStorage::Store(patientId, paperDispatch);
    paperDispatchMap.emplace_back(std::move(id));
    PrescriptionStorage::StorePaperDispatchMap(patientId, paperDispatchMap);
}