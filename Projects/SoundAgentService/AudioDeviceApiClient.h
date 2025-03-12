// ReSharper disable CppExpressionWithoutSideEffects
#pragma once

#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <nlohmann/json.hpp>
#include <string>
#include <sstream>

#include <TimeUtils.h>
#include "../SoundAgentDll/SoundAgentInterface.h"
#include "FormattedOutput.h"

using namespace web::http;  // NOLINT(clang-diagnostic-header-hygiene)
using namespace web::http::client;  // NOLINT(clang-diagnostic-header-hygiene)

class AudioDeviceApiClient {
public:
    explicit AudioDeviceApiClient(std::wstring apiBaseUrl)
        : apiBaseUrl_(std::move(apiBaseUrl))
    {
    }

    // Post device data to the REST API using a SoundDeviceInterface
    void PostDeviceToApi(const SoundDeviceInterface* device) const
    {
        if (!device)
        {
            const auto msg = "Cannot post device data: nullptr provided"; std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msg << '\n';
            SPD_L->error(msg);
            return;
        }

        // Use system hostname for network identification
        wchar_t hostNameBuffer[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD bufferSize = std::size(hostNameBuffer);
        GetComputerNameW(hostNameBuffer, &bufferSize);
        const std::wstring hostName(hostNameBuffer);

        // Convert wstring parameters to UTF-8 strings for JSON
        const std::string pnpIdUtf8 = utility::conversions::to_utf8string(device->GetPnpId());
        const std::string nameUtf8 = utility::conversions::to_utf8string(device->GetName());
        const std::string hostNameUtf8 = utility::conversions::to_utf8string(hostName);

        auto localTimeAsString = ed::getLocalTimeAsString("T");
        localTimeAsString = localTimeAsString.substr(0, localTimeAsString.length() - 7);

        const nlohmann::json payload = {
            {"pnpId", pnpIdUtf8},
            {"name", nameUtf8},
            {"volume", static_cast<const int>(device->GetCurrentRenderVolume())},
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

        SPD_L->info("Sending request for device: {}", pnpIdUtf8);
        try {
            // Send request and handle response
            const pplx::task<http_response> responseTask = client.request(request);
            responseTask.then([pnpIdUtf8](const http_response& response) {
                if (response.status_code() == status_codes::Created ||
                    response.status_code() == status_codes::OK ||
                    response.status_code() == status_codes::NoContent)
                {
                    const auto msg = "Device data posted successfully for: " + pnpIdUtf8;
                    std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msg << '\n';
                    SPD_L->info(msg);
                }
				else
                {
                    const auto statusCode = response.status_code();
                    const auto msg = "Failed to post device data for: " + pnpIdUtf8 +
                                    " - Status code: " + std::to_string(statusCode); 
                    std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msg << '\n';
                    SPD_L->error(msg);
                }
            }).wait(); // Blocking call for simplicity
        }
        catch (const http_exception& ex) {
            const auto msg = "HTTP exception for device " + pnpIdUtf8 + ": " + std::string(ex.what()); std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msg << '\n';
            SPD_L->error(msg);
        }
        catch (const std::exception& ex) {
            const auto msg = "Common exception while sending HTTP request for device " + pnpIdUtf8 + ": " + std::string(ex.what()); std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msg << '\n';
            SPD_L->error(msg);
        }
        catch (...) {
            const auto msg = "Unspecified exception while sending HTTP request for device " + pnpIdUtf8; std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msg << '\n';
            SPD_L->error(msg);
        }
        SPD_L->info("...Request handled for device: {}", pnpIdUtf8);
    }

private:
    std::wstring apiBaseUrl_;
};