//
// Created by sigsegv on 2/20/24.
//

#ifndef SFMBASISFAKER_PERSONSTORAGE_H
#define SFMBASISFAKER_PERSONSTORAGE_H

#include <string>

class Person;

class PersonStorage {
public:
    void Store(const Person &);
private:
    [[nodiscard]] Person GetRawById(const std::string &) const ;
    void AddFnr(Person &person) const ;
    void AddHpr(Person &person) const ;
public:
    [[nodiscard]] Person GetByFodselsnummer(const std::string &) const ;
    [[nodiscard]] Person GetById(const std::string &) const ;
};

#endif //SFMBASISFAKER_PERSONSTORAGE_H
