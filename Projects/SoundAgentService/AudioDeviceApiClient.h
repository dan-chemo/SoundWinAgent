#pragma once

#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <nlohmann/json.hpp>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace web::http;  // NOLINT(clang-diagnostic-header-hygiene)
using namespace web::http::client;  // NOLINT(clang-diagnostic-header-hygiene)

class AudioDeviceApiClient {
public:
    explicit AudioDeviceApiClient(std::wstring apiBaseUrl)
        : apiBaseUrl_(std::move(apiBaseUrl))
    {
    }

    // Post device data to the REST API
    void PostDeviceToApi(const std::string& pnpId, const std::string& name, int volume, const std::string& hostName) const
    {
        // Create JSON payload
        const nlohmann::json payload = {
            {"pnpId", pnpId},
            {"name", name},
            {"volume", volume},
            {"lastSeen", get_current_timestamp()},
            {"hostName", hostName}
        };

        // Convert nlohmann::json to cpprestsdk::json::value
        const web::json::value jsonPayload = web::json::value::parse(payload.dump());

        // Create HTTP client and request
        http_client client(apiBaseUrl_);
        http_request request(methods::POST);
        request.set_body(jsonPayload);
        request.headers().set_content_type(U("application/json"));

        // Send request and handle response
        // ReSharper disable once CppExpressionWithoutSideEffects
        client.request(request)
              .then([](const http_response & response) {
                  if (response.status_code() == status_codes::NoContent) {
                      std::cout << "Device data posted successfully!\n";
                  }
                  else {
                      std::cerr << "Failed to post device data. Status code: " << response.status_code() << "\n";
                  }
              })
              .wait(); // Blocking call for simplicity (use .then() for async)
    }

private:
    std::wstring apiBaseUrl_;

    // Helper function to get current timestamp in ISO 8601 format  
    static std::string get_current_timestamp() {  
           auto now = std::chrono::system_clock::now();  
           auto in_time_t = std::chrono::system_clock::to_time_t(now);  

           std::stringstream ss;  
           std::tm tm;  
           gmtime_s(&tm, &in_time_t);  
           ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");  
           return ss.str();  
       }
};