//
// Created by sigsegv on 8/9/24.
//

#ifndef SFMBASISFAKER_PLLSTORAGE_H
#define SFMBASISFAKER_PLLSTORAGE_H

#include "../domain/pll.h"

class PllStorageError : public std::exception {
};

class PllStorage {
public:
    void Store(const std::string &patient, const Pll &);
    Pll Load(const std::string &patient);
};


#endif //SFMBASISFAKER_PLLSTORAGE_H
