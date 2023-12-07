//
// Created by jeo on 12/7/23.
//

#ifndef SFMBASISFAKER_FHIR_H
#define SFMBASISFAKER_FHIR_H

#include <string>
#include <vector>
#include <memory>
#include <cpprest/json.h>

enum class FhirStatus {
    ACTIVE
};

class FhirObject {
public:
    virtual ~FhirObject() = default;
    virtual web::json::value ToJson() const;
};

class FhirValue : public FhirObject {
public:
    virtual std::string GetPropertyName() const = 0;
    static std::shared_ptr<FhirValue> Parse(const web::json::value &propertyName, const web::json::value &property);
};

class FhirExtension : public FhirObject {
private:
    std::string url;
public:
    virtual ~FhirExtension() = default;
    virtual web::json::value ToJson() const override;
};

class Fhir : public FhirObject {
private:
    std::string resourceType;
    std::string id;
    std::string profile;
    FhirStatus status;
    std::vector<std::shared_ptr<FhirExtension>> extensions;
public:
    virtual ~Fhir() = default;
};

#endif //SFMBASISFAKER_FHIR_H
