//
// Created by sigsegv on 3/1/24.
//

#ifndef SFMBASISFAKER_PRESCRIPTIONSTORAGE_H
#define SFMBASISFAKER_PRESCRIPTIONSTORAGE_H

#include <string>
#include <vector>

class Prescription;

class PrescriptionStorage {
public:
    [[nodiscard]] std::string Store(const std::string &patient, const Prescription &prescription) const;
    [[nodiscard]] Prescription Load(const std::string &patient, const std::string &prescriptionId) const;
    void StorePatientMap(const std::string &patient, const std::vector<std::string> &prescriptions);
    [[nodiscard]] std::vector<std::string> LoadPatientMap(const std::string &patient) const;
};


#endif //SFMBASISFAKER_PRESCRIPTIONSTORAGE_H
