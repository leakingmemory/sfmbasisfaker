#include <iostream>
#include <vector>
#include <sfmbasisapi/fhir/parameters.h>
#include "webserver/WebServer.h"
#include "controllers/MedicationController.h"
#include <jjwtid/Jwt.h>
#include "domain/person.h"
#include "service/PersonStorage.h"


static bool keep_alive = true;

void handle_term_signal(int) {
    keep_alive = false;
}

Person GetPersonFromAuthorization(const web::http::http_headers &headers) {
    std::string authorization{};
    auto authorizationFind = headers.find("Authorization");
    if (authorizationFind != headers.end()) {
        authorization = authorizationFind->second;
    }
    if (authorization.empty() || !authorization.starts_with("Bearer")) {
        return {};
    }
    authorization.erase(0, 6);
    if (authorization[0] != ' ' && authorization[0] != '\t') {
        return {};
    }
    do {
        authorization.erase(0, 1);
    } while (authorization[0] == ' ' || authorization[0] == '\t');
    Jwt jwt{authorization};
    JwtPart jwtBody{jwt.GetUnverifiedBody()};
    std::string pid = jwtBody.GetString("helseid://claims/identity/pid");
    std::string hpr = jwtBody.GetString("helseid://claims/hpr/hpr_number");
    // "helseid://claims/client/claims/orgnr_parent"
    std::string dateOfBirth{};
    bool female{true};
    if (pid.size() == 11) {
        std::string sd = pid.substr(0, 2);
        std::string sm = pid.substr(2, 2);
        std::string sy2 = pid.substr(4, 2);
        std::string sc = pid.substr(6, 1);
        std::string sg = pid.substr(8, 1);
        std::size_t ccd;
        std::size_t ccm;
        std::size_t ccy;
        std::size_t ccc;
        std::size_t ccg;
        auto d = std::stoi(sd, &ccd);
        auto m = std::stoi(sm, &ccm);
        auto y = std::stoi(sy2, &ccy);
        auto c = std::stoi(sc, &ccc);
        auto g = std::stoi(sg, &ccg);
        if (ccd == 2 && ccm == 2 && ccy == 2 && ccc == 1 && ccg == 1 && y >= 0 && m > 0 && d > 0) {
            if (c <= 4) {
                if (y < 40) {
                    y += 2000;
                } else {
                    y += 1900;
                }
            } else if (c == 8) {
                y += 2000;
            } else if (c < 8) {
                if (y < 55) {
                    y += 2000;
                } else {
                    y += 1800;
                }
            } else {
                if (y < 40) {
                    y += 1900;
                } else {
                    y += 2000;
                }
            }
        }
        if ((g & 1) == 1) {
            female = false;
        }
        std::stringstream dob{};
        dob << y << "-";
        if (m < 10) {
            dob << "0";
        }
        dob << m << "-";
        if ( d < 10) {
            dob << "0";
        }
        dob << d;
        dateOfBirth = dob.str();
    }
    Person person{};
    PersonStorage personStorage{};
    person = personStorage.GetByFodselsnummer(pid);
    if (!person.GetId().empty()) {
        bool modified{false};
        if (person.GetHpr() != hpr) {
            person.SetHpr(hpr);
            modified = true;
        }
        if (person.GetDateOfBirth() != dateOfBirth) {
            person.SetDateOfBirth(dateOfBirth);
            modified = true;
        }
        if (person.GetGender() == PersonGender::FEMALE && !female) {
            person.SetGender(PersonGender::MALE);
            modified = true;
        } else if (person.GetGender() == PersonGender::MALE && female) {
            person.SetGender(PersonGender::FEMALE);
            modified = true;
        }
        if (modified) {
            personStorage.Store(person);
        }
    }
    person.SetFodselsnummer(pid);
    person.SetHpr(hpr);
    person.SetDateOfBirth(dateOfBirth);
    person.SetGender(female ? PersonGender::FEMALE : PersonGender::MALE);
    personStorage.Store(person);
    return person;
}

int main() {
    auto medicationController = std::make_shared<MedicationController>();

    std::vector<std::string> allowedOriginHosts{};
    {
        const char *allow_cors = getenv("ALLOW_CORS");
        if (allow_cors != NULL) {
            std::string cors{allow_cors};
            while (!cors.empty()) {
                std::string::size_type comma = cors.find(',');
                std::string host{};
                if (comma > cors.length()) {
                    host = cors;
                    cors = "";
                } else {
                    host = cors.substr(0, comma);
                    cors = cors.substr(comma + 1);
                }
                boost::trim(host);
                if (!host.empty()) {
                    allowedOriginHosts.push_back(host);
                }
            }
        }
    }
    std::string listen{};
    {
        const char *listen = getenv("LISTEN");
        if (listen == NULL) {
            listen = "http://0.0.0.0:8080";
        }

        WebServer webServer(listen);

        webServer / "health" >> [] web_handler (web::http::http_request &) {
            return handle_web ([] async_web {
                    web::http::http_response response(web::http::status_codes::OK);
                    web::json::value value = web::json::value::object();
                    value["status"] = web::json::value("OK");
                    response.set_body(value);
                    return response;
            });
        };
        webServer / "patient" / "$getMedication" >> [medicationController] web_handler (const web::http::http_request &req) {
            std::string uri = req.request_uri().to_string();
            const auto &headers = req.headers();
            auto contentType = headers.content_type();
            if (!contentType.starts_with("application/fhir+json")) {
                std::cerr << "$getMedication: wrong content type in request: " << contentType << "\n";
                return pplx::task<web::http::http_response>([] () {
                    web::http::http_response response(web::http::status_codes::BadRequest);
                    return response;
                });
            }

            auto practitionerPerson = GetPersonFromAuthorization(headers);
            if (practitionerPerson.GetFodselsnummer().empty() ||
                practitionerPerson.GetHpr().empty()) {
                return pplx::task<web::http::http_response>([] () {
                    web::http::http_response response(web::http::status_codes::Unauthorized);
                    return response;
                });
            }

            return req.extract_json(true).then([medicationController, practitionerPerson, uri] (const pplx::task<web::json::value> &jsonTask) {
                try {
                    auto json = jsonTask.get();
                    std::shared_ptr<FhirPerson> patient;
                    {
                        FhirParameters inputParameterBundle = FhirParameters::Parse(json);
                        for (const auto &inputParameter : inputParameterBundle.GetParameters()) {
                            if (inputParameter.GetName() != "patient") {
                                web::http::http_response response(web::http::status_codes::BadRequest);
                                return response;
                            }
                            patient = std::dynamic_pointer_cast<FhirPerson>(inputParameter.GetResource());
                            if (!patient) {
                                web::http::http_response response(web::http::status_codes::BadRequest);
                                return response;
                            }
                        }
                    }
                    if (!patient) {
                        web::http::http_response response(web::http::status_codes::BadRequest);
                        return response;
                    }
                    auto outputParameters = medicationController->GetMedication(uri, practitionerPerson, *patient);
                    web::http::http_response response(web::http::status_codes::OK);
                    {
                        auto json = outputParameters.ToJson();
                        auto jsonString = json.serialize();
                        response.set_body(jsonString, "application/fhir+json; charset=utf-8");
                    }
                    return response;
                } catch (...) {
                    web::http::http_response response(web::http::status_codes::InternalError);
                    return response;
                }
            });
        };
        webServer / "patient" / "$sendMedication" >> [medicationController] web_handler (const web::http::http_request &req) {
            auto contentType = req.headers().content_type();
            if (!contentType.starts_with("application/fhir+json")) {
                std::cerr << "$getMedication: wrong content type in request: " << contentType << "\n";
                return pplx::task<web::http::http_response>([] () {
                    web::http::http_response response(web::http::status_codes::BadRequest);
                    return response;
                });
            }
            return req.extract_json(true).then([medicationController] (const pplx::task<web::json::value> &jsonTask) {
                try {
                    auto json = jsonTask.get();
                    std::shared_ptr<FhirBundle> bundle;
                    {
                        FhirParameters inputParameterBundle = FhirParameters::Parse(json);
                        for (const auto &inputParameter : inputParameterBundle.GetParameters()) {
                            if (inputParameter.GetName() != "medication") {
                                web::http::http_response response(web::http::status_codes::BadRequest);
                                return response;
                            }
                            bundle = std::dynamic_pointer_cast<FhirBundle>(inputParameter.GetResource());
                            if (!bundle) {
                                web::http::http_response response(web::http::status_codes::BadRequest);
                                return response;
                            }
                        }
                    }
                    if (!bundle) {
                        web::http::http_response response(web::http::status_codes::BadRequest);
                        return response;
                    }
                    auto outputParameters = medicationController->SendMedication(*bundle);
                    web::http::http_response response(web::http::status_codes::OK);
                    {
                        auto json = outputParameters.ToJson();
                        auto jsonString = json.serialize();
                        response.set_body(jsonString, "application/fhir+json; charset=utf-8");
                    }
                    return response;
                } catch (...) {
                    web::http::http_response response(web::http::status_codes::InternalError);
                    return response;
                }
            });
        };

        signal(SIGINT, handle_term_signal);
        signal(SIGTERM, handle_term_signal);

        while (keep_alive) {
            sleep(1);
        }
    }
    return 0;
}
