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
    [[nodiscard]] bool IsSet() const {
        return !coding.empty() || !text.empty();
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

class FhirQuantity : public FhirObject {
private:
    std::string unit;
    long milliValue;
    bool isSet;
public:
    FhirQuantity() : unit(), milliValue(0), isSet(false) {}
    FhirQuantity(double value, const std::string &unit) : unit(unit), milliValue((long) floor(value * ((double)1000.0))), isSet(true) {}
    [[nodiscard]] std::string GetUnit() const {
        return unit;
    }
    [[nodiscard]] double GetValue() const {
        return ((double) milliValue) / ((double)1000.0);
    }
    bool IsSet() const {
        return isSet;
    }
    web::json::value ToJson() const;
    static FhirQuantity Parse(const web::json::value &obj);
};

class FhirRatio : public FhirObject {
private:
    FhirQuantity numerator;
    FhirQuantity denominator;
public:
    FhirRatio() : numerator(), denominator() {}
    FhirRatio(const FhirQuantity &numerator, const FhirQuantity &denominator)
            : numerator(numerator), denominator(denominator) {}
    FhirRatio(FhirQuantity &&numerator, FhirQuantity &&denominator)
            : numerator(std::move(numerator)), denominator(std::move(denominator)) {}
    [[nodiscard]] FhirQuantity GetNumerator() const {
        return numerator;
    }
    [[nodiscard]] FhirQuantity GetDenominator() const {
        return denominator;
    }
    bool IsSet() const {
        return numerator.IsSet() || denominator.IsSet();
    }
    web::json::value ToJson() const;
    static FhirRatio Parse(const web::json::value &obj);
};

class FhirReference : public FhirObject {
private:
    std::string reference;
    std::string type;
    std::string display;
public:
    FhirReference() : reference(), type(), display() {}
    FhirReference(const std::string &reference, const std::string &type, const std::string &display)
            : reference(reference), type(type), display(display) {}
    FhirReference(std::string &&reference, std::string &&type, std::string &&display)
            : reference(std::move(reference)), type(std::move(type)), display(std::move(display)) {}

    [[nodiscard]] std::string GetReference() const {
        return reference;
    }
    [[nodiscard]] std::string GetType() const {
        return type;
    }
    [[nodiscard]] std::string GetDisplay() const {
        return display;
    }
    [[nodiscard]] bool IsSet() const {
        return !reference.empty() || !type.empty() || !display.empty();
    }

    web::json::value ToJson() const;
    static FhirReference Parse(const web::json::value &obj);
};

class FhirIdentifier : public FhirObject {
private:
    FhirCodeableConcept type;
    std::string use;
    std::string value;
public:
    FhirIdentifier() : type(), use(), value() {}
    FhirIdentifier(const FhirCodeableConcept &type, const std::string &use, const std::string &value)
        : type(type), use(use), value(value) {}
    FhirIdentifier(FhirCodeableConcept &&type, std::string &&use, std::string &&value)
        : type(std::move(type)), use(std::move(use)), value(std::move(value)) {}
    [[nodiscard]] std::string GetUse() const {
        return use;
    }
    [[nodiscard]] FhirCodeableConcept GetType() const {
        return type;
    }
    [[nodiscard]] std::string GetValue() const {
        return value;
    }
    bool IsSet() const {
        return !use.empty() || type.IsSet() || !value.empty();
    }
    web::json::value ToJson() const;
    static FhirIdentifier Parse(const web::json::value &obj);
};

#endif //SFMBASISFAKER_VALUE_H
