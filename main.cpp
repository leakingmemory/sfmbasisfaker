#include <iostream>
#include <vector>
#include <sfmbasisapi/fhir/parameters.h>
#include "webserver/WebServer.h"
#include "controllers/MedicationController.h"
#include <jjwtid/Jwt.h>
#include "domain/person.h"
#include "service/PersonStorage.h"
#include "webserver/CorsFilter.h"
#include "controllers/PharmacyController.h"
#include "service/DataDirectory.h"

template <typename... Args> class AccessLogFilter : public WebFilter<web::http::http_request &, Args...> {
public:
    std::optional<pplx::task<web::http::http_response>> filter(std::function<std::optional<pplx::task<web::http::http_response>> (web::http::http_request &, Args...)> next, const std::vector<std::string> &path, const std::map<std::string,std::string> &query, web::http::http_request &request, Args... args) override {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::string method = request.method();
        std::string uri = request.request_uri().to_string();
        try {
            auto nextTaskOpt = next(request, args...);
            if (nextTaskOpt) {
                auto &nextTask = *nextTaskOpt;
                return nextTask.then([method, uri, tm] (const pplx::task<web::http::http_response> &task) {
                    try {
                        auto response = task.get();
                        std::ofstream log{};
                        log.open(DataDirectory::Data("sfmbasisfaker").FileName("access.log"), std::ios_base::app);
                        log << std::put_time(&tm, "%d-%m-%Y %H:%M:%S") << " " << method << " " << response.status_code() << " " << uri << "\n";
                        log.close();
                        return response;
                    } catch (...) {
                        std::ofstream log{};
                        log.open(DataDirectory::Data("sfmbasisfaker").FileName("access.log"), std::ios_base::app);
                        log << std::put_time(&tm, "%d-%m-%Y %H:%M:%S") << " " << method << " " << "ASYNC-EXCEPTION " << uri << "\n";
                        log.close();
                        throw;
                    }
                });
            } else {
                std::ofstream log{};
                log.open(DataDirectory::Data("sfmbasisfaker").FileName("access.log"), std::ios_base::app);
                log << std::put_time(&tm, "%d-%m-%Y %H:%M:%S") << " " << method << " " << "NORESP " << uri << "\n";
                log.close();
                return {};
            }
        } catch (...) {
            std::ofstream log{};
            log.open(DataDirectory::Data("sfmbasisfaker").FileName("access.log"), std::ios_base::app);
            log << std::put_time(&tm, "%d-%m-%Y %H:%M:%S") << " " << method << " " << "SYNC-EXCEPTION " << uri << "\n";
            log.close();
            throw;
        }
    }
};

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
    auto pharmacyController = std::make_shared<PharmacyController>();

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

        WebServer webServerInstance(listen);

        auto &webServer = webServerInstance | AccessLogFilter() | CorsFilter(allowedOriginHosts);

        webServer / "health" >> [] web_handler (web::http::http_request &) {
            return handle_web ([] async_web {
                    web::http::http_response response(web::http::status_codes::OK);
                    web::json::value value = web::json::value::object();
                    value["status"] = web::json::value("OK");
                    response.set_body(value);
                    return response;
            });
        };
        webServer / "metadata" >> [] web_handler (web::http::http_request &) {
            return handle_web ([] async_web {
                web::http::http_response response(web::http::status_codes::OK);
                std::string json = "{\n  \"resourceType\": \"CapabilityStatement\",\n  \"url\": \"http://base-fhir.test5.forskrivning.no/metadata\",\n  \"version\": \"1.0.0.0\",\n  \"name\": \"SFM-Basis\",\n  \"title\": \"SFM-Basis Capability statement\",\n  \"status\": \"active\",\n  \"experimental\": false,\n  \"date\": \"2021-05-01\",\n  \"publisher\": \"Norsk Helsenett/SFM\",\n  \"kind\": \"capability\",\n  \"software\": {\n    \"name\": \"SFM Basis API\",\n    \"version\": \"4.10\"\n  },\n  \"fhirVersion\": \"4.0.0\",\n  \"format\": [\n    \"application/fhir+json\",\n    \"json\",\n    \"application/fhir+xml\",\n    \"xml\"\n  ],\n  \"patchFormat\": [\n    \"application/fhir+json\",\n    \"application/json-patch+json\"\n  ],\n  \"rest\": [\n    {\n      \"mode\": \"server\",\n      \"security\": {\n        \"extension\": [\n          {\n            \"extension\": [\n              {\n                \"url\": \"token\",\n                \"valueUri\": \"https://helseid-sts.test.nhn.no/connect/token\"\n              },\n              {\n                \"url\": \"authorize\",\n                \"valueUri\": \"https://helseid-sts.test.nhn.no/connect/authorize\"\n              }\n            ],\n            \"url\": \"http://fhir-registry.smarthealthit.org/StructureDefinition/oauth-uris\"\n          }\n        ],\n        \"service\": [\n          {\n            \"coding\": [\n              {\n                \"system\": \"http://terminology.hl7.org/CodeSystem/restful-security-service\",\n                \"code\": \"OAuth\"\n              }\n            ]\n          }\n        ]\n      },\n      \"resource\": [\n        {\n          \"type\": \"Practitioner\",\n          \"profile\": \"http://ehelse.no/fhir/StructureDefinition/sfm-Practitioner\",\n          \"interaction\": [\n            {\n              \"code\": \"create\"\n            },\n            {\n              \"code\": \"update\"\n            },\n            {\n              \"code\": \"read\"\n            }\n          ],\n          \"versioning\": \"versioned\",\n          \"readHistory\": false,\n          \"updateCreate\": false,\n          \"conditionalCreate\": false,\n          \"conditionalUpdate\": false,\n          \"conditionalDelete\": \"single\",\n          \"searchParam\": [\n            {\n              \"name\": \"identifier\",\n              \"type\": \"token\",\n              \"documentation\": \"A practitioner's Identifier\"\n            },\n            {\n              \"name\": \"given\",\n              \"type\": \"string\",\n              \"documentation\": \"A portion of the given name\"\n            },\n            {\n              \"name\": \"active\",\n              \"type\": \"string\",\n              \"documentation\": \"Is the Practitioner record active\"\n            },\n            {\n              \"name\": \"family\",\n              \"type\": \"string\",\n              \"documentation\": \"A portion of the family name\"\n            }\n          ]\n        },\n        {\n          \"type\": \"Organization\",\n          \"profile\": \"http://ehelse.no/fhir/StructureDefinition/sfm-Organization\",\n          \"interaction\": [\n            {\n              \"code\": \"create\"\n            },\n            {\n              \"code\": \"update\"\n            },\n            {\n              \"code\": \"read\"\n            }\n          ],\n          \"versioning\": \"versioned\",\n          \"readHistory\": false,\n          \"updateCreate\": false,\n          \"conditionalCreate\": false,\n          \"conditionalUpdate\": false,\n          \"conditionalDelete\": \"single\",\n          \"searchParam\": [\n            {\n              \"name\": \"identifier\",\n              \"type\": \"token\",\n              \"documentation\": \"Any identifier for the organization (not the accreditation issuer's identifier)\"\n            },\n            {\n              \"name\": \"partof\",\n              \"type\": \"reference\",\n              \"documentation\": \"An organization of which this organization forms a part\"\n            },\n            {\n              \"name\": \"active\",\n              \"type\": \"string\",\n              \"documentation\": \"Is the Organization record active\"\n            },\n            {\n              \"name\": \"name\",\n              \"type\": \"string\",\n              \"documentation\": \"A portion of the organization's name or alias\"\n            },\n            {\n              \"name\": \"_id\",\n              \"type\": \"token\",\n              \"documentation\": \"The ID of the resource\"\n            }\n          ]\n        },\n        {\n          \"type\": \"Person\",\n          \"profile\": \"http://ehelse.no/fhir/StructureDefinition/sfm-Person\",\n          \"interaction\": [\n            {\n              \"code\": \"create\"\n            },\n            {\n              \"code\": \"update\"\n            },\n            {\n              \"code\": \"read\"\n            }\n          ],\n          \"versioning\": \"versioned\",\n          \"readHistory\": false,\n          \"updateCreate\": false,\n          \"conditionalCreate\": false,\n          \"conditionalUpdate\": false,\n          \"conditionalDelete\": \"single\",\n          \"searchParam\": [\n            {\n              \"name\": \"identifier\",\n              \"type\": \"token\",\n              \"documentation\": \"A person Identifier\"\n            },\n            {\n              \"name\": \"name\",\n              \"type\": \"string\",\n              \"documentation\": \"A server defined search that may match any of the string fields in the HumanName, including family, give, prefix, suffix, suffix, and/or text\"\n            },\n            {\n              \"name\": \"_id\",\n              \"type\": \"token\",\n              \"documentation\": \"The ID of the resource\"\n            }\n          ]\n        },\n        {\n          \"type\": \"Task\",\n          \"profile\": \"http://ehelse.no/fhir/StructureDefinition/sfm-Task\",\n          \"versioning\": \"versioned\",\n          \"readHistory\": false,\n          \"updateCreate\": false,\n          \"conditionalCreate\": false,\n          \"conditionalUpdate\": false,\n          \"conditionalDelete\": \"single\",\n          \"searchParam\": [\n            {\n              \"name\": \"owner\",\n              \"type\": \"reference\",\n              \"documentation\": \"Search by task owner\"\n            },\n            {\n              \"name\": \"code\",\n              \"type\": \"token\",\n              \"documentation\": \"Search by task code\"\n            },\n            {\n              \"name\": \"patientNiN\",\n              \"type\": \"string\",\n              \"documentation\": \"List of patient identifiers\"\n            },\n            {\n              \"name\": \"_lastUpdated\",\n              \"type\": \"date\",\n              \"documentation\": \"Time base for changes\"\n            }\n          ]\n        }\n      ],\n      \"operation\": [\n        {\n          \"name\": \"anonymized-export\",\n          \"definition\": \"http://base-fhir.test5.forskrivning.no/OperationDefinition/anonymized-export\"\n        },\n        {\n          \"name\": \"member-match\",\n          \"definition\": \"http://base-fhir.test5.forskrivning.no/OperationDefinition/member-match\"\n        },\n        {\n          \"name\": \"patient-everything\",\n          \"definition\": \"https://www.hl7.org/fhir/patient-operation-everything.html\"\n        },\n        {\n          \"name\": \"getMedication\",\n          \"definition\": \"http://nhn.no/sfm/fhir/OperationDefinition/SFM-getMedication\"\n        },\n        {\n          \"name\": \"sendMedication\",\n          \"definition\": \"http://nhn.no/sfm/fhir/OperationDefinition/SFM-sendMedication\"\n        },\n        {\n          \"name\": \"registerResponsibility\",\n          \"definition\": \"http://nhn.no/sfm/fhir/OperationDefinition/SFM-registerResponsibility\"\n        },\n        {\n          \"name\": \"deRegisterResponsibility\",\n          \"definition\": \"http://nhn.no/sfm/fhir/OperationDefinition/SFM-deRegisterResponsibility\"\n        },\n        {\n          \"name\": \"validate\",\n          \"definition\": \"http://hl7.org/fhir/OperationDefinition/Resource-validate\"\n        }\n      ]\n    }\n  ]\n}";
                response.set_body(json, "application/fhir+json");
                return response;
            });
        };
        webServer / "patient" / "$getMedication" >> [medicationController] web_handler (const web::http::http_request &req) {
            std::string uri = req.request_uri().to_string();
            const auto &headers = req.headers();
            auto contentType = headers.content_type();
            if (!contentType.starts_with("application/fhir+json") && !contentType.starts_with("application/json")) {
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
                        FhirParameters inputParameterBundle = FhirParameters::ParseJson(json.serialize());
                        for (const auto &inputParameter: inputParameterBundle.GetParameters()) {
                            if (inputParameter.GetName() == "patient") {
                                patient = std::dynamic_pointer_cast<FhirPerson>(inputParameter.GetResource());
                                if (!patient) {
                                    std::cerr << "$getMedication: Invalid patient, not a person object\n";
                                    web::http::http_response response(web::http::status_codes::BadRequest);
                                    return response;
                                }
                            }
                        }
                    }
                    if (!patient) {
                        std::cerr << "$getMedication: No patient in the request\n";
                        web::http::http_response response(web::http::status_codes::BadRequest);
                        return response;
                    }
                    auto outputParameters = medicationController->GetMedication(uri, practitionerPerson, *patient);
                    web::http::http_response response(web::http::status_codes::OK);
                    {
                        auto jsonString = outputParameters.ToJson();
                        response.set_body(jsonString, "application/fhir+json; charset=utf-8");
                    }
                    return response;
                } catch (const std::exception &e) {
                    std::cerr << "$getMedication: std::exception: " << e.what() << "\n";
                    web::http::http_response response(web::http::status_codes::InternalError);
                    return response;
                } catch (...) {
                    std::cerr << "$getMedication: unknown exception\n";
                    web::http::http_response response(web::http::status_codes::InternalError);
                    return response;
                }
            });
        };
        webServer / "patient" / "$sendMedication" >> [medicationController] web_handler (const web::http::http_request &req) {
            const auto &headers = req.headers();
            auto contentType = req.headers().content_type();
            if (!contentType.starts_with("application/fhir+json") && !contentType.starts_with("application/json")) {
                std::cerr << "$sendMedication: wrong content type in request: " << contentType << "\n";
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

            return req.extract_json(true).then([medicationController, practitionerPerson] (const pplx::task<web::json::value> &jsonTask) {
                try {
                    auto json = jsonTask.get();
                    std::shared_ptr<FhirBundle> bundle;
                    {
                        FhirParameters inputParameterBundle = FhirParameters::ParseJson(json.serialize());
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
                    auto outputParameters = medicationController->SendMedication(*bundle, practitionerPerson);
                    web::http::http_response response(web::http::status_codes::OK);
                    {
                        auto jsonString = outputParameters->ToJson();
                        response.set_body(jsonString, "application/fhir+json; charset=utf-8");
                    }
                    return response;
                } catch (...) {
                    web::http::http_response response(web::http::status_codes::InternalError);
                    return response;
                }
            });
        };
        webServer / "pharmacy" / "patients" >> [pharmacyController] web_handler (const web::http::http_request &) {
            return handle_web ( [pharmacyController] async_web {
                auto patients = pharmacyController->GetPatients();
                auto json = web::json::value::array();
                int i = 0;
                for (const auto patient: patients) {
                    auto pat = web::json::value::object();
                    pat["id"] = web::json::value::string(patient.id);
                    pat["firstName"] = web::json::value::string(patient.firstName);
                    pat["lastName"] = web::json::value::string(patient.lastName);
                    json[i++] = pat;
                }
                web::http::http_response response(web::http::status_codes::OK);
                {
                    auto jsonString = json.serialize();
                    response.set_body(jsonString, "application/fhir+json; charset=utf-8");
                }
                return response;
            });
        };
        webServer / "pharmacy" / "patients" / PathVariable<std::string>() / "paperdispatch" >> [pharmacyController] web_handler (const web::http::http_request &request, const std::string &patientId) {
            if (request.method() == "PUT") {
                pplx::task<web::http::http_response> resp = request.extract_json().then(
                        [pharmacyController, patientId](
                                const pplx::task<web::json::value> &jsonTask) -> web::http::http_response {
                            try {
                                auto json = jsonTask.get();
                                PaperDispatchData paperDispatchData{};

                                if (json.has_string_field("prescriptionGroup"))
                                    paperDispatchData.prescriptionGroup = json.at("prescriptionGroup").as_string();
                                if (json.has_string_field("registrationType"))
                                    paperDispatchData.registrationType = json.at("registrationType").as_string();
                                if (json.has_string_field("name"))
                                    paperDispatchData.name = json.at("name").as_string();
                                if (json.has_string_field("nameFormStrength"))
                                    paperDispatchData.nameFormStrength = json.at("nameFormStrength").as_string();
                                if (json.has_string_field("packingSize"))
                                    paperDispatchData.packingSize = json.at("packingSize").as_string();
                                if (json.has_string_field("packingUnitCode"))
                                    paperDispatchData.packingUnitCode = json.at("packingUnitCode").as_string();
                                if (json.has_string_field("packingUnitDisplay"))
                                    paperDispatchData.packingUnitDisplay = json.at("packingUnitDisplay").as_string();
                                if (json.has_string_field("productNumber"))
                                    paperDispatchData.productNumber = json.at("productNumber").as_string();
                                if (json.has_string_field("atcCode"))
                                    paperDispatchData.atcCode = json.at("atcCode").as_string();
                                if (json.has_string_field("atcDisplay"))
                                    paperDispatchData.atcDisplay = json.at("atcDisplay").as_string();
                                if (json.has_string_field("formCode"))
                                    paperDispatchData.formCode = json.at("formCode").as_string();
                                if (json.has_string_field("formDisplay"))
                                    paperDispatchData.formDisplay = json.at("formDisplay").as_string();
                                if (json.has_number_field("amount"))
                                    paperDispatchData.amount = json.at("amount").as_double();
                                if (json.has_string_field("amountUnit"))
                                    paperDispatchData.amountUnit = json.at("amountUnit").as_string();
                                if (json.has_string_field("amountText"))
                                    paperDispatchData.amountText = json.at("amountText").as_string();
                                if (json.has_string_field("dssn"))
                                    paperDispatchData.dssn = json.at("dssn").as_string();
                                if (json.has_number_field("numberOfPackages"))
                                    paperDispatchData.numberOfPackages = json.at("numberOfPackages").as_double();
                                if (json.has_string_field("reit"))
                                    paperDispatchData.reit = json.at("reit").as_string();
                                if (json.has_string_field("itemGroupCode"))
                                    paperDispatchData.itemGroupCode = json.at("itemGroupCode").as_string();
                                if (json.has_string_field("itemGroupDisplay"))
                                    paperDispatchData.itemGroupDisplay = json.at("itemGroupDisplay").as_string();
                                if (json.has_string_field("prescriptionTypeCode"))
                                    paperDispatchData.prescriptionTypeCode = json.at(
                                            "prescriptionTypeCode").as_string();
                                if (json.has_string_field("prescriptionTypeDisplay"))
                                    paperDispatchData.prescriptionTypeDisplay = json.at(
                                            "prescriptionTypeDisplay").as_string();
                                if (json.has_string_field("prescriptionId"))
                                    paperDispatchData.prescriptionId = json.at("prescriptionId").as_string();
                                if (json.has_boolean_field("genericSubstitutionAccepted"))
                                    paperDispatchData.genericSubstitutionAccepted = json.at(
                                            "genericSubstitutionAccepted").as_bool();
                                if (json.has_string_field("prescribedByHpr"))
                                    paperDispatchData.prescribedByHpr = json.at("prescribedByHpr").as_string();
                                if (json.has_string_field("prescribedByGivenName"))
                                    paperDispatchData.prescribedByGivenName = json.at(
                                            "prescribedByGivenName").as_string();
                                if (json.has_string_field("prescribedByFamilyName"))
                                    paperDispatchData.prescribedByFamilyName = json.at(
                                            "prescribedByFamilyName").as_string();
                                if (json.has_string_field("dispatcherHerId"))
                                    paperDispatchData.dispatcherHerId = json.at("dispatcherHerId").as_string();
                                if (json.has_string_field("dispatcherName"))
                                    paperDispatchData.dispatcherName = json.at("dispatcherName").as_string();
                                if (json.has_boolean_field("substitutionReservationCustomer"))
                                    paperDispatchData.substitutionReservationCustomer = json.at(
                                            "substitutionReservationCustomer").as_bool();
                                if (json.has_string_field("dispatchMsgId"))
                                    paperDispatchData.dispatchMsgId = json.at("dispatchMsgId").as_string();
                                if (json.has_number_field("quantity"))
                                    paperDispatchData.quantity = json.at("quantity").as_double();
                                if (json.has_string_field("whenHandedOver"))
                                    paperDispatchData.whenHandedOver = json.at("whenHandedOver").as_string();
                                pharmacyController->PaperDispense(patientId, paperDispatchData);
                                web::http::http_response response(web::http::status_codes::OK);
                                return response;

                            } catch (...) {
                                std::cerr << "Exception in paper dispatch\n";
                                web::http::http_response response(web::http::status_codes::InternalError);
                                return response;
                            }

                        });
                return resp;
            }
            return handle_web ([patientId] async_web {
                auto json = web::json::value::string(patientId);
                web::http::http_response response(web::http::status_codes::OK);
                {
                    auto jsonString = json.serialize();
                    response.set_body(jsonString, "application/json; charset=utf-8");
                }
                return response;
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
