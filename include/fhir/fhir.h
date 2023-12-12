//
// Created by jeo on 12/7/23.
//

#ifndef SFMBASISFAKER_FHIR_H
#define SFMBASISFAKER_FHIR_H

#include <string>
#include <vector>
#include <memory>
#include "extension.h"
#include "fhirconcepts.h"

enum class FhirStatus {
    NOT_SET,
    ACTIVE,
    FINAL
};

class FhirValue : public FhirObject {
public:
    constexpr FhirValue() {}
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
    std::string resourceType;
    std::string id{};
    std::string lastUpdated{};
    std::string source{};
    std::vector<std::string> profile{};
    std::string timestamp{};
    std::string date{};
    FhirStatus status{FhirStatus::NOT_SET};
protected:
    bool ParseInline(const web::json::value &json);
public:
    constexpr Fhir() : resourceType() {}
    constexpr explicit Fhir(const std::string &resourceType) : resourceType(resourceType) {}
    constexpr explicit Fhir(std::string &&resourceType) : resourceType(std::move(resourceType)) {}
    web::json::value ToJson() const;
    static std::shared_ptr<Fhir> Parse(const web::json::value &obj);
    virtual std::string GetDisplay() const;
    virtual ~Fhir() = default;
    [[nodiscard]] std::string GetResourceType() const {
        return resourceType;
    }
    [[nodiscard]] std::string GetId() const {
        return id;
    }
    [[nodiscard]] std::string GetLastUpdated() const {
        return lastUpdated;
    }
    [[nodiscard]] std::string GetSource() const {
        return source;
    }
    [[nodiscard]] std::vector<std::string> GetProfile() const {
        return profile;
    }
    [[nodiscard]] std::string GetTimestamp() const {
        return timestamp;
    }
    [[nodiscard]] std::string GetDate() const {
        return date;
    }
    [[nodiscard]] FhirStatus GetStatus() const {
        return status;
    }
    void SetId(const std::string &id) {
        this->id = id;
    }
    void SetLastUpdated(const std::string &lastUpdated) {
        this->lastUpdated = lastUpdated;
    }
    void SetSource(const std::string &source) {
        this->source = source;
    }
    void SetProfile(const std::string &profile) {
        this->profile = { profile };
    }
    void SetTimestamp(const std::string ts) {
        timestamp = ts;
    }
    void SetDate(const std::string &d) {
        date = d;
    }
    void SetStatus(FhirStatus status) {
        this->status = status;
    }
};

#endif //SFMBASISFAKER_FHIR_H
