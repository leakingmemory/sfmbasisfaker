//
// Created by jeo on 08.12.2023.
//

#ifndef SFMBASISFAKER_BUNDLEENTRY_H
#define SFMBASISFAKER_BUNDLEENTRY_H

#include "fhirobject.h"

class Fhir;

class FhirBundleEntry : public FhirObject {
private:
    std::string fullUrl{};
    std::shared_ptr<Fhir> resource{};
public:
    [[nodiscard]] std::string GetFullUrl() const {
        return fullUrl;
    }
    [[nodiscard]] std::shared_ptr<Fhir> GetResource() const {
        return resource;
    }
    web::json::value ToJson() const;
    static FhirBundleEntry Parse(const web::json::value &obj);
};

#endif //SFMBASISFAKER_BUNDLEENTRY_H
