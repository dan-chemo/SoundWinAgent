#include "stdafx.h"

#include "ServiceObserver.h"

#include "FormattedOutput.h"
#include "AudioDeviceApiClient.h"
#include "HttpRequestProcessor.h"

#include <SpdLogger.h>

ServiceObserver::ServiceObserver(SoundDeviceCollectionInterface& collection,
    std::wstring apiBaseUrl,
    std::wstring universalToken,
    std::wstring codespaceName) // Added codespaceName parameter
    : collection_(collection)
    , apiBaseUrl_(std::move(apiBaseUrl))
    , universalToken_(std::move(universalToken))
    , codespaceName_(std::move(codespaceName)) // Initialize new member
    , requestProcessorSmartPtr_(std::make_shared<HttpRequestProcessor>(apiBaseUrl_, universalToken_, codespaceName_))
{
}

void ServiceObserver::PostToApi(const SoundDeviceEventType messageType, const SoundDeviceInterface* devicePtr) const
{
    const AudioDeviceApiClient apiClient(requestProcessorSmartPtr_);
    apiClient.PostDeviceToApi(messageType, devicePtr);
    apiClient.PostDeviceToApi(messageType, devicePtr, " (copy)");
}

void ServiceObserver::PostAndPrintCollection() const
{
    std::string message("Processing device collection...");
    FormattedOutput::LogAndPrint(message);

    for (size_t i = 0; i < collection_.GetSize(); ++i)
    {
        const std::unique_ptr deviceSmartPtr(collection_.CreateItem(i));

        FormattedOutput::PrintDeviceInfo(deviceSmartPtr.get());
        if (!apiBaseUrl_.empty())
        {
            PostToApi(SoundDeviceEventType::None, deviceSmartPtr.get());
        }
        else
        {
            SPD_L->info("No API base URL configured. Skipping API call.");
        }
    }
    message = "...Processing device collection finished.";
    FormattedOutput::LogAndPrint(message);
    std::cout
        << '\n'
        << FormattedOutput::CurrentLocalTimeWithoutDate << "Press Ctrl-C to stop and finish the application\n"
        << FormattedOutput::CurrentLocalTimeWithoutDate << "-----------------------------------------------\n";
}

void ServiceObserver::OnCollectionChanged(SoundDeviceEventType event, const std::wstring & devicePnpId)
{
    FormattedOutput::PrintEvent(event, devicePnpId);

    if (event == SoundDeviceEventType::Discovered
		|| event == SoundDeviceEventType::VolumeRenderChanged) // TODO: separate processing, below
    {
        const auto soundDeviceInterface = collection_.CreateItem(devicePnpId);
        PostToApi(event, soundDeviceInterface.get());
    }
    // TODO: other events
    /*
    else if (event == SoundDeviceEventType::VolumeRenderChanged)
    {
        const auto soundDeviceInterface = collection_.CreateItem(devicePnpId);
        VolumeChangeToApi(devicePnpId, soundDeviceInterface->GetCurrentRenderVolume());
    }
    else if (event == SoundDeviceEventType::Removed)
    {
        RemoveToApi(devicePnpId);
    }
    else
	{
		SPD_L->warn("Unknown event type: {}", static_cast<int>(event));
	}
	*/

}

void ServiceObserver::OnTrace(const std::wstring & line)
{
    SPD_L->info(FormattedOutput::WString2StringTruncate(line));
}
