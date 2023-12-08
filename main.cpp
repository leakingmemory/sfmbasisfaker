#include <iostream>
#include <vector>
#include "webserver/WebServer.h"


static bool keep_alive = true;

void handle_term_signal(int) {
    keep_alive = false;
}

int main() {
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

        signal(SIGINT, handle_term_signal);
        signal(SIGTERM, handle_term_signal);

        while (keep_alive) {
            sleep(1);
        }
    }
    return 0;
}
