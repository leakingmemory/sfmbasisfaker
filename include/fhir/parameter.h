//
// Created by jeo on 08.12.2023.
//

#ifndef SFMBASISFAKER_PARAMETER_H
#define SFMBASISFAKER_PARAMETER_H

#include "fhirobject.h"

class Fhir;

class FhirParameter : public FhirObject {
private:
    std::shared_ptr<Fhir> resource;
    std::string name;
public:
    [[nodiscard]] std::shared_ptr<Fhir> GetResource() const { return resource; }
    [[nodiscard]] std::string GetName() const { return name; }
    [[nodiscard]] web::json::value ToJson() const;
    static FhirParameter Parse(const web::json::value &obj);
};

#endif //SFMBASISFAKER_PARAMETER_H