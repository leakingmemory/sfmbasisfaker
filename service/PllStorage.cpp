//
// Created by sigsegv on 8/9/24.
//

#include "PllStorage.h"
#include "DataDirectory.h"

void PllStorage::Store(const std::string &patient, const Pll &pll) {
    if (patient.empty()) {
        throw PllStorageError();
    }
    auto dir = DataDirectory::Data("sfmbasisfaker").Sub("prescription").Sub(patient);
    dir.WriteFile("PLL", pll.Serialize());
}

Pll PllStorage::Load(const std::string &patient) {
    if (patient.empty()) {
        return {};
    }
    auto dir = DataDirectory::Data("sfmbasisfaker").Sub("prescription").Sub(patient);
    auto json = dir.ReadFile("PLL");
    if (json.empty()) {
        return {};
    }
    return Pll::Parse(json);
}
