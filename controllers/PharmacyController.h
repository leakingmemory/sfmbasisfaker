//
// Created by jeo on 5/22/25.
//

#ifndef SFMBASISFAKER_PHARMACYCONTROLLER_H
#define SFMBASISFAKER_PHARMACYCONTROLLER_H

#include <string>
#include <vector>

struct PharmacyPatient {
    std::string id{};
};

class PharmacyController {
public:
    std::vector<PharmacyPatient> GetPatients();
};


#endif //SFMBASISFAKER_PHARMACYCONTROLLER_H
