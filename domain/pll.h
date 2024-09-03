//
// Created by sigsegv on 8/9/24.
//

#ifndef SFMBASISFAKER_PLL_H
#define SFMBASISFAKER_PLL_H

#include <string>

class Pll {
private:
    std::string id;
    std::string dateTime;
public:
    [[nodiscard]] std::string GetId() const {
        return id;
    }

    void SetId(const std::string &id) {
        this->id = id;
    }

    [[nodiscard]] std::string GetDateTime() const {
        return dateTime;
    }

    void SetDateTime(const std::string &dateTime) {
        this->dateTime = dateTime;
    }

    bool IsValid() const {
        return (!id.empty()) && (!dateTime.empty());
    }

    std::string Serialize() const;
    static Pll Parse(const std::string &json);
};

#endif //SFMBASISFAKER_PLL_H
