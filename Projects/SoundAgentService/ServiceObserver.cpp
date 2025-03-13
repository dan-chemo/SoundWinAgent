#include "stdafx.h"

#include "ServiceObserver.h"

#include "FormattedOutput.h"
#include "AudioDeviceApiClient.h"

#include <SpdLogger.h>


void ServiceObserver::PostAndPrintCollection() const
{
    std::string message("Posting device collection:...");
	FormattedOutput::LogAndPrint(message);

    for (size_t i = 0; i < collection_.GetSize(); ++i)
    {
        const std::unique_ptr deviceSmartPtr(collection_.CreateItem(i));
        FormattedOutput::PrintDeviceInfo(deviceSmartPtr.get());
        AudioDeviceApiClient(apiBaseUrl_).PostDeviceToApi(deviceSmartPtr.get());
    }
    message = "...Posting device collection finished.";
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
