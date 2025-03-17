#include "stdafx.h"

#include "ServiceObserver.h"

#include "FormattedOutput.h"
#include "AudioDeviceApiClient.h"
#include "HttpRequestProcessor.h"


#include <SpdLogger.h>


ServiceObserver::ServiceObserver(AudioDeviceCollectionInterface& collection,
    std::wstring apiBaseUrl,
    std::wstring universalToken)
    : collection_(collection)
    , apiBaseUrl_(std::move(apiBaseUrl))
    , universalToken_(std::move(universalToken))
    , requestProcessorSmartPtr_(std::make_shared<HttpRequestProcessor>(apiBaseUrl_, universalToken_))
{
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
			AudioDeviceApiClient apiClient(requestProcessorSmartPtr_);
            apiClient.PostDeviceToApi(deviceSmartPtr.get());
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

void ServiceObserver::OnCollectionChanged(AudioDeviceCollectionEvent event, const std::wstring & devicePnpId)
{
    FormattedOutput::PrintEvent(event, devicePnpId);

    if (event == AudioDeviceCollectionEvent::Discovered
        || event == AudioDeviceCollectionEvent::VolumeChanged
    )
    {
        PostAndPrintCollection();
    }
}

void ServiceObserver::OnTrace(const std::wstring & line)
{
    SPD_L->info(FormattedOutput::WString2StringTruncate(line));
}
