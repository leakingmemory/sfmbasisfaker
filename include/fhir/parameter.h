//
// Created by jeo on 08.12.2023.
//

#ifndef SFMBASISFAKER_PARAMETER_H
#define SFMBASISFAKER_PARAMETER_H

#include "fhirobject.h"

class Fhir;
class FhirValue;

class FhirParameter : public FhirObject {
private:
    std::shared_ptr<Fhir> resource;
    std::shared_ptr<FhirValue> value;
    std::string name;
public:
    FhirParameter() : resource(), value(), name() {}
    FhirParameter(const std::string &name, const std::shared_ptr<Fhir> &resource) : resource(resource), value(), name(name) {}
    FhirParameter(const std::string &name, const std::shared_ptr<FhirValue> &value) : resource(), value(value), name(name) {}
    [[nodiscard]] std::shared_ptr<Fhir> GetResource() const { return resource; }
    [[nodiscard]] std::shared_ptr<FhirValue> GetFhirValue() const { return value; }
    [[nodiscard]] std::string GetName() const { return name; }
    [[nodiscard]] web::json::value ToJson() const;
    static FhirParameter Parse(const web::json::value &obj);
};

#endif //SFMBASISFAKER_PARAMETER_H