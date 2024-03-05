//
// Created by sigsegv on 2/20/24.
//

#include "PersonStorage.h"
#include "DataDirectory.h"
#include "../domain/person.h"
#include <cpprest/json.h>
#include <boost/uuid/uuid_generators.hpp> // for random_generator
#include <boost/uuid/uuid_io.hpp> // for to_string

void PersonStorage::Store(const Person &person) {
    std::string id{person.GetId()};
    if (id.empty()) {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        id = boost::uuids::to_string(randomUUID);
    }
    auto personDir = DataDirectory::Data("sfmbasisfaker").Sub("person");
    {
        std::string clob{};
        {
            web::json::value p = web::json::value::object();
            p["family"] = web::json::value::string(person.GetFamilyName());
            p["given"] = web::json::value::string(person.GetGivenName());
            p["gender"] = web::json::value::string(person.GetGender() == PersonGender::FEMALE ? "female" : "male");
            p["dob"] = web::json::value::string(person.GetDateOfBirth());
            p["postcode"] = web::json::value::string(person.GetHomePostalCode());
            p["city"] = web::json::value::string(person.GetHomeCity());
            clob = p.serialize();
        }
        personDir.WriteFile(id, clob);
    }
    {
        auto fodselsnummer = person.GetFodselsnummer();
        if (!fodselsnummer.empty()) {
            std::string clob = personDir.ReadFile("fnr.json");
            {
                web::json::value fnr{};
                if (!clob.empty()) {
                    fnr = web::json::value::parse(clob);
                } else {
                    fnr = web::json::value::object();
                }
                fnr[fodselsnummer] = web::json::value::string(id);
                clob = fnr.serialize();
            }
            personDir.WriteFile("fnr.json", clob);
        }
    }
    auto hpr = person.GetHpr();
    if (!hpr.empty()) {
        std::string clob = personDir.ReadFile("hpr.json");
        {
            web::json::value fnr{};
            if (!clob.empty()) {
                fnr = web::json::value::parse(clob);
            } else {
                fnr = web::json::value::object();
            }
            fnr[hpr] = web::json::value::string(id);
            clob = fnr.serialize();
        }
        personDir.WriteFile("hpr.json", clob);
    }
}

Person PersonStorage::GetRawById(const std::string &id) const {
    std::string clob = DataDirectory::Data("sfmbasisfaker").Sub("person").ReadFile(id);
    if (clob.empty()) {
        return {};
    }
    Person person{};
    auto p = web::json::value::parse(clob);
    person.SetId(id);
    person.SetFamilyName(p.at("family").as_string());
    person.SetGivenName(p.at("given").as_string());
    person.SetGender(p.at("gender").as_string() == "female" ? PersonGender::FEMALE : PersonGender::MALE);
    person.SetDateOfBirth(p.at("dob").as_string());
    person.SetHomePostalCode(p.at("postcode").as_string());
    person.SetHomeCity(p.at("city").as_string());
    return person;
}

void PersonStorage::AddFnr(Person &person) const {
    auto id = person.GetId();
    if (id.empty()) {
        return;
    }
    std::string clob = DataDirectory::Data("sfmbasisfaker").Sub("person").ReadFile("fnr.json");
    if (clob.empty()) {
        return;
    }
    auto map = web::json::value::parse(clob);
    for (const auto &pair : map.as_object()) {
        if (pair.second.as_string() == id) {
            person.SetFodselsnummer(pair.first);
            return;
        }
    }
}

void PersonStorage::AddHpr(Person &person) const {
    auto id = person.GetId();
    if (id.empty()) {
        return;
    }
    std::string clob = DataDirectory::Data("sfmbasisfaker").Sub("person").ReadFile("hpr.json");
    if (clob.empty()) {
        return;
    }
    auto map = web::json::value::parse(clob);
    for (const auto &pair : map.as_object()) {
        if (pair.second.as_string() == id) {
            person.SetHpr(pair.first);
            return;
        }
    }
}

Person PersonStorage::GetByFodselsnummer(const std::string &fodselsnummer) const {
    Person person{};
    {
        web::json::value fnr{};
        {
            std::string clob = DataDirectory::Data("sfmbasisfaker").Sub("person").ReadFile("fnr.json");
            if (clob.empty()) {
                return {};
            }
            fnr = web::json::value::parse(clob);
        }
        if (!fnr.has_string_field(fodselsnummer)) {
            return {};
        }
        person = GetRawById(fnr.at(fodselsnummer).as_string());
    }
    person.SetFodselsnummer(fodselsnummer);
    AddHpr(person);
    return person;
}

Person PersonStorage::GetById(const std::string &id) const {
    Person person = GetRawById(id);
    if (!id.empty()) {
        AddHpr(person);
        AddFnr(person);
    }
    return person;
}
