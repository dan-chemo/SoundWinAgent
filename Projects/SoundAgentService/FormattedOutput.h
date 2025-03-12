#pragma once

#include <TimeUtils.h>

#include "../SoundAgentDll/ClassDefHelper.h"
#include "../SoundAgentDll/SoundAgentInterface.h"
#include "../SoundAgentLib/DefToString.h"

#include <SpdLogger.h>


class FormattedOutput final
{
private:
	~FormattedOutput() = default;
protected:
	DISALLOW_IMPLICIT_CONSTRUCTORS(FormattedOutput);
public:

    static void PrintEvent(AudioDeviceCollectionEvent event, const std::wstring& devicePnpId)
    {
        std::wostringstream wos; wos << L"Event caught: " << ed::GetDeviceCollectionEventAsString(event) << L"."
            << L" Device PnP id: " << devicePnpId << L'\n';
        SPD_L->info(WString2StringTruncate(wos.str()));
        std::wcout << CurrentLocalTimeAsWideStringWithoutDate << wos.str() << '\n';
    }


    static void PrintDeviceInfo(const SoundDeviceInterface* device)
    {
        const auto idString = device->GetPnpId();
        const std::wstring idAsWideString(idString.begin(), idString.end());
        std::wostringstream wos; wos << idString
            << ", \"" << device->GetName()
            << "\", " << ed::GetFlowAsString(device->GetFlow())
            << ", Volume " << device->GetCurrentRenderVolume()
            << " / " << device->GetCurrentCaptureVolume();
        SPD_L->info(WString2StringTruncate(wos.str()));
		std::wcout << CurrentLocalTimeAsWideStringWithoutDate << wos.str() << '\n';
    }

    static std::string WString2StringTruncate(const std::wstring& str) {
        std::string result;
        result.reserve(str.size());

        std::ranges::for_each(str, [&result](wchar_t wc) {
            result += static_cast<char>(wc);
            });

        return result;
    }

    static std::wostream & CurrentLocalTimeAsWideStringWithoutDate(std::wostream & os)
    {
        const std::wstring currentTime = ed::getLocalTimeAsWideString();
        if
        (
            constexpr int beginOfTimeCountingFromTheEnd = 15;
            currentTime.size() >= beginOfTimeCountingFromTheEnd
        )
        {
            os << currentTime.substr(currentTime.size() - beginOfTimeCountingFromTheEnd, 12) << L" ";
        }
        return os;
    }

    static std::ostream & CurrentLocalTimeWithoutDate(std::ostream & os)
    {
        const std::string currentTime = ed::getLocalTimeAsString();
        if
        (
            constexpr int beginOfTimeCountingFromTheEnd = 15;
            currentTime.size() >= beginOfTimeCountingFromTheEnd
        )
        {
            os << currentTime.substr(currentTime.size() - beginOfTimeCountingFromTheEnd, 12) << " ";
        }
        return os;
    }
};
