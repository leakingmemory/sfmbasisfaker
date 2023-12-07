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
    std::string GetPropertyName() override;
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

#endif //SFMBASISFAKER_VALUE_H
