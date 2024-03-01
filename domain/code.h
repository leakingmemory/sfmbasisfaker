//
// Created by sigsegv on 2/23/24.
//

#ifndef SFMBASISFAKER_CODE_H
#define SFMBASISFAKER_CODE_H

#include <string>

namespace web::json {
    class value;
}

class Code {
private:
    std::string code{};
    std::string display{};
    std::string system{};
public:
    Code() {}
    Code(const std::string &code, const std::string &display, const std::string &system) : code(code), display(display), system(system) {}

    // Getters
    std::string getCode() const { return code; }
    std::string getDisplay() const { return display; }
    std::string getSystem() const { return system; }

    // Setters
    void setCode(const std::string& code) { this->code = code; }
    void setDisplay(const std::string& display) { this->display = display; }
    void setSystem(const std::string& system) { this->system = system; }

    [[nodiscard]] web::json::value Serialize() const;
    static Code Parse(const web::json::value &json);
};

#endif //SFMBASISFAKER_CODE_H