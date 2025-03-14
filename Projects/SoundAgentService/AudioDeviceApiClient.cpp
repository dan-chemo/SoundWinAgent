#include "stdafx.h"
// ReSharper disable CppExpressionWithoutSideEffects

#include "AudioDeviceApiClient.h"

#include "../SoundAgentDll/SoundAgentInterface.h"

#include <SpdLogger.h>

#include <string>
#include <sstream>
#include <nlohmann/json.hpp>


#include <TimeUtils.h>
#include "FormattedOutput.h"
#include "HttpRequestProcessor.h"


// ReSharper disable once CppPassValueParameterByConstReference
AudioDeviceApiClient::AudioDeviceApiClient(std::shared_ptr<HttpRequestProcessor> processor)  // NOLINT(performance-unnecessary-value-param, modernize-pass-by-value)
    : requestProcessor_(processor)  // NOLINT(performance-unnecessary-value-param)
{
}

void AudioDeviceApiClient::PostDeviceToApi(const SoundDeviceInterface * device) const
{
    if (!device)
    {
        const auto msg = "Cannot post device data: nullptr provided";
        std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msg << '\n';
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
    web::http::http_request request(web::http::methods::POST);
    request.set_body(jsonPayload);
    request.headers().set_content_type(U("application/json"));

    SPD_L->info("Enqueueing request for device: {}...", pnpIdUtf8);

    // Instead of sending directly, enqueue the request in the processor
    if(requestProcessor_->EnqueueRequest(request, pnpIdUtf8))
    {
		FormattedOutput::LogAndPrint("Device data enqueued for: " + pnpIdUtf8);
	}
	else
	{
		FormattedOutput::LogAndPrint("Device data enqueuing skipped for: " + pnpIdUtf8);
	}
}
