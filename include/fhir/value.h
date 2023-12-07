//
// Created by jeo on 12/7/23.
//

#ifndef SFMBASISFAKER_VALUE_H
#define SFMBASISFAKER_VALUE_H

#include "fhir.h"

class FhirString : public FhirValue {
private:
    std::string value;
public:
    constexpr FhirString() noexcept : value() {}
    explicit constexpr FhirString(const std::string &value) noexcept : value(value) {}
    explicit constexpr FhirString(std::string &&value) noexcept : value(std::move(value)) {}
    static std::string PropertyName() {
        return "valueString";
    }
    std::string GetPropertyName() const override;
    web::json::value ToJson() const override;
    static std::shared_ptr<FhirString> Parse(web::json::value value);
    std::string GetValue() const {
        return value;
    }
};

class FhirCoding : public FhirObject {
private:
    std::string system;
    std::string code;
    std::string display;
public:
    FhirCoding(const std::string &system, const std::string &code, const std::string &display) :
        system(system), code(code), display(display) {}
    web::json::value ToJson() const override;
    static FhirCoding Parse(const web::json::value &obj);
    std::string GetSystem() const {
        return system;
    }
    std::string GetCode() const {
        return code;
    }
    std::string GetDisplay() const {
        return display;
    }
};

class FhirCodeableConcept : public FhirObject {
private:
    std::vector<FhirCoding> coding;
    std::string text;
public:
    FhirCodeableConcept() : coding(), text() {}
    explicit FhirCodeableConcept(const std::vector<FhirCoding> &coding) : coding(coding), text() {}
    explicit FhirCodeableConcept(std::vector<FhirCoding> &&coding) : coding(std::move(coding)), text() {}
    explicit FhirCodeableConcept(std::vector<FhirCoding> &&coding, std::string &&text) : coding(std::move(coding)), text(std::move(text)) {}
    explicit FhirCodeableConcept(const std::string &text) : coding(), text(text) {}
    [[nodiscard]] web::json::value ToJson() const override;
    static FhirCodeableConcept Parse(const web::json::value &obj);
    [[nodiscard]] std::vector<FhirCoding> GetCoding() const {
        return coding;
    }
    [[nodiscard]] std::string GetText() const {
        return text;
    }
};

class FhirCodeableConceptValue : public FhirValue, public FhirCodeableConcept {
public:
    FhirCodeableConceptValue() {}
    explicit FhirCodeableConceptValue(const std::vector<FhirCoding> &coding) : FhirCodeableConcept(coding) {}
    explicit FhirCodeableConceptValue(std::vector<FhirCoding> &&coding) : FhirCodeableConcept(coding) {}
    explicit FhirCodeableConceptValue(const std::string &text) : FhirCodeableConcept(text) {}
    explicit FhirCodeableConceptValue(const FhirCodeableConcept &cc) : FhirCodeableConcept(cc) {}
    explicit FhirCodeableConceptValue(FhirCodeableConcept &&cc) : FhirCodeableConcept(std::move(cc)) {}
    static std::string PropertyName() {
        return "valueCodeableConcept";
    }
    std::string GetPropertyName() const override;
    [[nodiscard]] web::json::value ToJson() const override;
    static std::shared_ptr<FhirCodeableConceptValue> Parse(const web::json::value &obj);
};

#endif //SFMBASISFAKER_VALUE_H
