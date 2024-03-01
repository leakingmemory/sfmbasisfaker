//
// Created by sigsegv on 2/29/24.
//

#include "../domain/code.h"
#include <cpprest/json.h>

web::json::value Code::Serialize() const {
    auto obj = web::json::value::object();
    obj["code"] = web::json::value::string(code);
    obj["display"] = web::json::value::string(display);
    obj["system"] = web::json::value::string(system);
    return obj;
}

Code Code::Parse(const web::json::value &json) {
    std::string code{};
    std::string display{};
    std::string system{};
    if (json.has_string_field("code")) {
        code = json.at("code").as_string();
    }
    if (json.has_string_field("display")) {
        display = json.at("display").as_string();
    }
    if (json.has_string_field("system")) {
        system = json.at("system").as_string();
    }
    return {code, display, system};
}
