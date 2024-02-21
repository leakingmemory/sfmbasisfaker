//
// Created by sigsegv on 2/20/24.
//

#ifndef SFMBASISFAKER_M_PERSON_H
#define SFMBASISFAKER_M_PERSON_H

#include <string>

enum class PersonGender {
    FEMALE,
    MALE
};

class Person {
private:
    std::string id;
    std::string fodselsnummer;
    std::string familyName;
    std::string givenName;
    PersonGender gender;
    std::string dateOfBirth;
    std::string homePostalCode;
    std::string homeCity;
    std::string hpr;
public:
    std::string GetId() const {
        return id;
    }

    void SetId(const std::string &id) {
        Person::id = id;
    }

    std::string GetFodselsnummer() const {
        return fodselsnummer;
    }

    void SetFodselsnummer(const std::string &fodselsnummer) {
        Person::fodselsnummer = fodselsnummer;
    }

    std::string GetFamilyName() const {
        return familyName;
    }

    void SetFamilyName(const std::string &familyName) {
        Person::familyName = familyName;
    }

    std::string GetGivenName() const {
        return givenName;
    }

    void SetGivenName(const std::string &givenName) {
        Person::givenName = givenName;
    }

    PersonGender GetGender() const {
        return gender;
    }

    void SetGender(PersonGender gender) {
        Person::gender = gender;
    }

    std::string GetDateOfBirth() const {
        return dateOfBirth;
    }

    void SetDateOfBirth(const std::string &dateOfBirth) {
        Person::dateOfBirth = dateOfBirth;
    }

    std::string GetHomePostalCode() const {
        return homePostalCode;
    }

    void SetHomePostalCode(const std::string &homePostalCode) {
        Person::homePostalCode = homePostalCode;
    }

    std::string GetHomeCity() const {
        return homeCity;
    }

    void SetHomeCity(const std::string &homeCity) {
        Person::homeCity = homeCity;
    }

    std::string GetHpr() const {
        return hpr;
    }

    void SetHpr(const std::string &hpr) {
        this->hpr = hpr;
    }
};

#endif //SFMBASISFAKER_M_PERSON_H