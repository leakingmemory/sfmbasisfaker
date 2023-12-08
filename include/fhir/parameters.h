//
// Created by jeo on 08.12.2023.
//

#ifndef SFMBASISFAKER_PARAMETERS_H
#define SFMBASISFAKER_PARAMETERS_H

#include "parameter.h"
#include "fhir.h"

class FhirParameters : public Fhir {
private:
    std::vector<FhirParameter> parameters{};
public:
    [[nodiscard]] std::vector<FhirParameter> GetParameters() const {
        return parameters;
    }
    [[nodiscard]] web::json::value ToJson() const;
    static FhirParameters Parse(const web::json::value &obj);
};

#endif //SFMBASISFAKER_PARAMETERS_H
