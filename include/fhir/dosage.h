//
// Created by sigsegv on 12/8/23.
//

#ifndef SFMBASISFAKER_DOSAGE_H
#define SFMBASISFAKER_DOSAGE_H

#include "fhirextendable.h"

class FhirDosage : public FhirExtendable {
private:
    std::string text;
    int sequence;
public:
    FhirDosage() : text(""), sequence(0) {}
    explicit FhirDosage(const std::string& text, int sequence)
        : text(text), sequence(sequence) {}

    std::string GetText() const { return text; }
    int GetSequence() const { return sequence; }

    web::json::value ToJson() const;
    static FhirDosage Parse(const web::json::value &obj);

};

#endif //SFMBASISFAKER_DOSAGE_H
