//
// Created by jeo on 08.12.2023.
//

#ifndef SFMBASISFAKER_BUNDLE_H
#define SFMBASISFAKER_BUNDLE_H

#include "value.h"
#include "bundleentry.h"

class FhirBundle : public Fhir {
private:
    std::vector<FhirLink> link{};
    std::vector<FhirBundleEntry> entries{};
    std::string type{};
    int total{0};
public:
    constexpr FhirBundle() : Fhir("Bundle") {}
    void SetType(const std::string &type) {
        this->type = type;
    }
    void AddLink(const std::string &relation, const std::string &url);
    void AddLink(std::string &&relation, std::string &&url);
    void AddEntry(const FhirBundleEntry &entry);
    void AddEntry(FhirBundleEntry &&entry);
    [[nodiscard]] std::vector<FhirLink> GetLink() const { return link; }
    [[nodiscard]] std::vector<FhirBundleEntry> GetEntries() const { return entries; }
    [[nodiscard]] std::string GetType() const {
        return type;
    }
    int GetTotal() const { return total; }

    web::json::value ToJson() const;
    static FhirBundle Parse(const web::json::value &obj);

};

#endif //SFMBASISFAKER_BUNDLE_H
