//
// Created by jeo on 12/7/23.
//

#ifndef SFMBASISFAKER_FHIR_H
#define SFMBASISFAKER_FHIR_H

#include <string>
#include <vector>
#include <memory>
#include "extension.h"

enum class FhirStatus {
    ACTIVE
};

class FhirValue : public FhirObject {
public:
    virtual std::string GetPropertyName() const = 0;
    static std::shared_ptr<FhirValue> Parse(const std::string &propertyName, const web::json::value &property);
};

class FhirValueExtension : public FhirExtension {
private:
    std::shared_ptr<FhirValue> value;
public:
    FhirValueExtension() = default;
    FhirValueExtension(const std::string &url, const std::shared_ptr<FhirValue> &value) : FhirExtension(url), value(value) {}
    FhirValueExtension(std::string &&url, std::shared_ptr<FhirValue> &&value) : FhirExtension(std::move(url)), value(std::move(value)) {}
    [[nodiscard]] std::shared_ptr<FhirValue> GetValue() const {
        return value;
    }
    web::json::value ToJson() const override;
};

class FhirGenericExtension : public FhirExtension {
private:
    web::json::value json;
public:
    FhirGenericExtension() : json() {}
    explicit FhirGenericExtension(const web::json::value &json);
    web::json::value ToJson() const override;
};

class Fhir : public FhirExtendable {
private:
    std::string resourceType{};
    std::string id{};
    std::vector<std::string> profile{};
    FhirStatus status{};
protected:
    bool ParseInline(const web::json::value &json);
public:
    web::json::value ToJson() const;
    virtual ~Fhir() = default;
    [[nodiscard]] std::string GetResourceType() const {
        return resourceType;
    }
    [[nodiscard]] std::string GetId() const {
        return id;
    }
    [[nodiscard]] std::vector<std::string> GetProfile() const {
        return profile;
    }
    [[nodiscard]] FhirStatus GetStatus() const {
        return status;
    }
};

#endif //SFMBASISFAKER_FHIR_H
