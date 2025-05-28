//
// Created by sigsegv on 3/1/24.
//

#ifndef SFMBASISFAKER_PRESCRIPTIONSTORAGE_H
#define SFMBASISFAKER_PRESCRIPTIONSTORAGE_H

#include <string>
#include <vector>

class Prescription;
class PaperDispatch;

class PrescriptionStorage {
public:
    static std::string Store(const std::string &patient, const Prescription &prescription);
    static std::string Store(const std::string &patient, const PaperDispatch &paperDispatch);
    static void Replace(const std::string &patient, const std::string &fileId, const Prescription &prescription);
    static Prescription Load(const std::string &patient, const std::string &prescriptionId);
    static PaperDispatch LoadPaperDispatch(const std::string &patient, const std::string &prescriptionId);
    static void StorePatientMap(const std::string &patient, const std::vector<std::string> &prescriptions);
    static std::vector<std::string> LoadPatientMap(const std::string &patient);
    static void StorePaperDispatchMap(const std::string &patient, const std::vector<std::string> &prescriptions);
    static std::vector<std::string> LoadPaperDispatchMap(const std::string &patient);
};


#endif //SFMBASISFAKER_PRESCRIPTIONSTORAGE_H
