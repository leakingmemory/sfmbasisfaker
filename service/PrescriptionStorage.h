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
    static std::string Store(const std::string &patient, const Prescription &prescription);
    static void Replace(const std::string &patient, const std::string &fileId, const Prescription &prescription);
    static Prescription Load(const std::string &patient, const std::string &prescriptionId);
    static void StorePatientMap(const std::string &patient, const std::vector<std::string> &prescriptions);
    static std::vector<std::string> LoadPatientMap(const std::string &patient);
};


#endif //SFMBASISFAKER_PRESCRIPTIONSTORAGE_H
