//
// Created by sigsegv on 8/9/24.
//

#include "../domain/pll.h"
#include <cpprest/json.h>

std::string Pll::Serialize() const {
    auto obj = web::json::value::object();
    obj["id"] = web::json::value::string(id);
    obj["datetime"] = web::json::value::string(dateTime);
    return obj.serialize();
}

Pll Pll::Parse(const std::string &json) {
    Pll pll{};
    auto obj = web::json::value::parse(json);
    if (obj.has_string_field("id")) {
        pll.id = obj.at("id").as_string();
    }
    if (obj.has_string_field("datetime")) {
        pll.dateTime = obj.at("datetime").as_string();
    }
    return pll;
}
