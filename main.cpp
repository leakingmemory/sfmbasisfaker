#include <iostream>
#include <vector>
#include <sfmbasisapi/fhir/parameters.h>
#include "webserver/WebServer.h"
#include "controllers/MedicationController.h"


static bool keep_alive = true;

void handle_term_signal(int) {
    keep_alive = false;
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
            auto contentType = req.headers().content_type();
            if (!contentType.starts_with("application/fhir+json")) {
                std::cerr << "$getMedication: wrong content type in request: " << contentType << "\n";
                return pplx::task<web::http::http_response>([] () {
                    web::http::http_response response(web::http::status_codes::BadRequest);
                    return response;
                });
            }
            return req.extract_json(true).then([medicationController, uri] (const pplx::task<web::json::value> &jsonTask) {
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
                    auto outputParameters = medicationController->GetMedication(uri, *patient);
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
