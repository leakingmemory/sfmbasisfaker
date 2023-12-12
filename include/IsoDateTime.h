//
// Created by sigsegv on 21.12.2020.
//

#ifndef ELPRICE_ISODATETIME_H
#define ELPRICE_ISODATETIME_H

#include <string>
#include <iostream>
#include <sstream>
#include <locale>
#include <iomanip>
#include <ctime>
#include <math.h>
#include <cstdint>

inline int64_t isoDateTime(const std::string &str) {
    std::string main{};
    std::string frac{};
    auto pos = str.find(".");
    if (pos >= str.length()) {
        pos = str.find("+");
        if (pos >= str.length()) {
            auto tpos = str.find("T");
            if (tpos < str.length()) {
                pos = str.find("-", tpos);
                if (pos >= str.length()) {
                    pos = str.find("Z");
                }
            }
        }
    }
    if (pos < str.length()) {
        main = str.substr(0, pos);
        frac = str.substr(pos);
    } else {
        main = str;
        frac = "";
    }
    std::tm t = {};
    {
        std::istringstream ss(main);
        ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S");
        if (ss.fail()) {
            throw std::exception();
        }
    }
    int64_t millis = timegm(&t);
    millis *= 1000;
    if (!frac.empty()) {
        std::string::size_type sz;
        if (frac[0] == '.') {
            double tmfrac = std::stod(frac, &sz);
            tmfrac = tmfrac * 1000.0;
            tmfrac = round(tmfrac);
            millis += (uint64_t) tmfrac;
        } else {
            sz = 0;
        }
        if (sz != frac.length()) {
            std::string zone = frac.substr(sz);
            if (zone != "Z") {
                if (zone[0] == '+' || zone[0] == '-') {
                    bool pos = zone[0] == '+';
                    zone = zone.substr(1);
                    int separators = 0;
                    for (char ch : zone) {
                        if (ch == ':') {
                            separators++;
                        }
                    }
                    t = {};
                    if (separators == 0) {
                        if (zone.length() != 4) {
                            throw std::exception();
                        }
                        std::istringstream ss(zone);
                        ss >> std::get_time(&t, "%H%M");
                        if (ss.fail()) {
                            throw std::exception();
                        }
                    } else if (separators == 1) {
                        std::istringstream ss(zone);
                        ss >> std::get_time(&t, "%H:%M");
                        if (ss.fail()) {
                            throw std::exception();
                        }
                    } else if (separators == 2) {
                        std::istringstream ss(zone);
                        ss >> std::get_time(&t, "%H:%M:%S");
                        if (ss.fail()) {
                            throw std::exception();
                        }
                    } else {
                        throw std::exception();
                    }
                    int32_t zmillis = t.tm_hour;
                    zmillis *= 60;
                    zmillis += t.tm_min;
                    zmillis *= 60;
                    zmillis += t.tm_sec;
                    zmillis *= 1000;
                    if (!pos) {
                        zmillis = 0 - zmillis;
                    }
                    millis += (int64_t) zmillis;
                } else {
                    throw std::exception();
                }
            }
        }
    }
    return millis;
}

inline std::string digits(int num, int digits) {
    std::string str{std::to_string(num)};
    std::string zeros{};
    int n_zeros = digits - str.length();
    while (n_zeros-- > 0) {
        zeros.append("0");
    }
    std::string fn = zeros + str;
    return fn;
}

inline std::string toIsoDateTime(int64_t millis) {
    std::tm t = {};
    int32_t mill{};
    {
        time_t tmc{};
        {
            int64_t mill_64 = millis % 1000;
            int64_t tmc_64 = millis / 1000;
            mill = mill_64;
            tmc = tmc_64;
        }
        gmtime_r(&tmc, &t);
    }
    std::string str{};
    str.append(digits(t.tm_year + 1900, 4));
    str.append("-");
    str.append(digits(t.tm_mon + 1, 2));
    str.append("-");
    str.append(digits(t.tm_mday, 2));
    str.append("T");
    str.append(digits(t.tm_hour, 2));
    str.append(":");
    str.append(digits(t.tm_min, 2));
    str.append(":");
    str.append(digits(t.tm_sec, 2));
    if (mill > 0) {
        float frac = mill;
        frac /= 1000;
        std::string fstr = std::to_string(frac);
        while (!fstr.empty() && fstr[0] == '0') {
            fstr = fstr.substr(1);
        }
        if (!fstr.empty() && fstr[0] != '.') {
            throw std::exception();
        }
        str.append(fstr);
    }
    str.append("Z");
    return str;
}

inline std::string toIsoDate(time_t tmc) {
    std::tm t = {};
    gmtime_r(&tmc, &t);
    std::string str{};
    str.append(digits(t.tm_year + 1900, 4));
    str.append("-");
    str.append(digits(t.tm_mon + 1, 2));
    str.append("-");
    str.append(digits(t.tm_mday, 2));
    return str;
}

inline std::string durationMillisToString(uint32_t seconds) {
    std::string str{"P"};
    if (seconds > (24*3600)) {
        str.append(std::to_string((int) (seconds / (24*3600))));
        str.append("D");
        seconds = seconds % (24*3600);
    }
    str.append("T");
    if (seconds > 3600) {
        str.append(std::to_string((int) (seconds / 3600)));
        str.append("H");
        seconds = seconds % 3600;
    }
    if (seconds > 60) {
        str.append(std::to_string((int) (seconds / 60)));
        str.append("M");
        seconds = seconds % 60;
    }
    if (seconds > 0) {
        str.append(std::to_string((int) seconds));
        str.append("S");
    }
    return str;
}

inline uint64_t isoDurationToMillis(const std::string &str) {
    auto iterator = str.begin();
    if (iterator == str.end() || *iterator != 'P') {
        throw std::exception();
    }
    ++iterator;
    uint64_t millis = 0;
    while (iterator != str.end() && *iterator != 'T') {
        int num = 0;
        while (iterator != str.end() && *iterator >= '0' && *iterator <= '9') {
            num *= 10;
            num += *iterator - '0';
            ++iterator;
        }
        if (iterator == str.end()) {
            throw std::exception();
        }
        switch (*iterator) {
            case 'W':
                millis += ((uint64_t) num) * 7*24*3600000;
                break;
            case 'D':
                millis += ((uint64_t) num) * 24*3600000;
                break;
            default:
                throw std::exception();
        }
        ++iterator;
    }
    if (iterator == str.end()) {
        return millis;
    }
    if (*iterator != 'T') {
        throw std::exception();
    }
    ++iterator;
    while (iterator != str.end()) {
        int num = 0;
        while (iterator != str.end() && *iterator >= '0' && *iterator <= '9') {
            num *= 10;
            num += *iterator - '0';
            ++iterator;
        }
        if (iterator == str.end()) {
            throw std::exception();
        }
        switch (*iterator) {
            case 'S':
                millis += ((uint64_t) num) * 1000;
                break;
            case 'M':
                millis += ((uint64_t) num) * 60000;
                break;
            case 'H':
                millis += ((uint64_t) num) * 3600000;
                break;
            default:
                throw std::exception();
        }
        ++iterator;
    }
    return millis;
}

#endif //ELPRICE_ISODATETIME_H
