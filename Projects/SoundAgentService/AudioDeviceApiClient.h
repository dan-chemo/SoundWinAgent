#pragma once

#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <nlohmann/json.hpp>
#include <string>
#include <sstream>

#include <TimeUtils.h>

#include "FormattedOutput.h"

using namespace web::http;  // NOLINT(clang-diagnostic-header-hygiene)
using namespace web::http::client;  // NOLINT(clang-diagnostic-header-hygiene)

class AudioDeviceApiClient {
public:
    explicit AudioDeviceApiClient(std::wstring apiBaseUrl)
        : apiBaseUrl_(std::move(apiBaseUrl))
    {
    }

    // Post device data to the REST API
    void PostDeviceToApi(const std::wstring& pnpId, const std::wstring& name, int volume, const std::wstring& hostName) const
    {
        const std::string pnpIdUtf8 = utility::conversions::to_utf8string(pnpId);
        const std::string nameUtf8 = utility::conversions::to_utf8string(name);
        const std::string hostNameUtf8 = utility::conversions::to_utf8string(hostName);

        auto localTimeAsString = ed::getLocalTimeAsString("T");
        localTimeAsString = localTimeAsString.substr(0, localTimeAsString.length() - 7);

        const nlohmann::json payload = {
            {"pnpId", pnpIdUtf8},
            {"name", nameUtf8},
            {"volume", volume},
            {"lastSeen", localTimeAsString},
            {"hostName", hostNameUtf8}
        };

        // Convert nlohmann::json to cpprestsdk::json::value
        const web::json::value jsonPayload = web::json::value::parse(payload.dump());

        // Create HTTP client and request
        http_client client(apiBaseUrl_);
        http_request request(methods::POST);
        request.set_body(jsonPayload);
        request.headers().set_content_type(U("application/json"));

        SPD_L->info("Sending request...");
        // Send request and handle response
        const pplx::task<http_response> responseTask = client.request(request);
        // ReSharper disable once CppExpressionWithoutSideEffects
        responseTask.then([](const http_response& response) {
            if (response.status_code() == status_codes::Created ||
                response.status_code() == status_codes::OK ||
                response.status_code() == status_codes::NoContent)
            {
                const auto msg = "Device data posted successfully!"; std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msg << '\n';
                SPD_L->info(msg);
            }
            else {
                const auto statusCode = response.status_code();
                const auto msg = "Failed to post device data. Status code: " + std::to_string(statusCode); std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msg << '\n';
                SPD_L->error(msg);
            }
            }).wait(); // Blocking call for simplicity
        SPD_L->info("...Request sent.");
    }

private:
    std::wstring apiBaseUrl_;
};