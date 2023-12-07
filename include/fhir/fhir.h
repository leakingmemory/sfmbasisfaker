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
    static std::shared_ptr<FhirValue> Parse(const std::string &propertyName, const web::json::value &property);
};

class FhirExtension : public FhirObject {
private:
    std::string url;
public:
    FhirExtension() : url() {}
    explicit FhirExtension(const std::string &url) : url(url) {}
    explicit FhirExtension(std::string &&url) : url(std::move(url)) {}
    virtual ~FhirExtension() = default;
    virtual web::json::value ToJson() const override;
    static std::shared_ptr<FhirExtension> Parse(const web::json::value &);
    [[nodiscard]] std::string GetUrl() const {
        return url;
    }
    void SetUrl(const std::string &url) {
        this->url = url;
    }
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
