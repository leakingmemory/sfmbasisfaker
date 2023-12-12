//
// Created by sigsegv on 12/7/23.
//

#ifndef SFMBASISFAKER_FHIREXTENDABLE_H
#define SFMBASISFAKER_FHIREXTENDABLE_H

#include "fhirobject.h"

class FhirExtension;

class FhirExtendable : public FhirObject {
private:
    std::vector<std::shared_ptr<FhirExtension>> extensions{};
protected:
    bool ParseInline(const web::json::value &json);
public:
    constexpr FhirExtendable() {}
    web::json::value ToJson() const;
    virtual ~FhirExtendable() = default;
    [[nodiscard]] std::vector<std::shared_ptr<FhirExtension>> GetExtensions() const {
        return extensions;
    }
};

#endif //SFMBASISFAKER_FHIREXTENDABLE_H
