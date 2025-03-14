#include "stdafx.h"

#include <SpdLogger.h>

#include <filesystem>
#include <iomanip>
#include <memory>
#include <tchar.h>

#include <Poco/Util/ServerApplication.h>
#include <Poco/UnicodeConverter.h>
#include <vector>

#include "AudioDeviceApiClient.h"
#include "FormattedOutput.h"
#include "../SoundAgentLib/CoInitRaiiHelper.h"
#include "../SoundAgentDll/SoundAgentInterface.h"

class Observer final : public AudioDeviceCollectionObserverInterface {
public:
    explicit Observer(AudioDeviceCollectionInterface & collection, std::wstring apiBaseUrl)
        : collection_(collection)
        , apiBaseUrl_(std::move(apiBaseUrl))
    {
    }

    DISALLOW_COPY_MOVE(Observer);
    ~Observer() override = default;

public:
    void PostAndPrintCollection() const
    {
        const std::string message1("Posting device collection:..."); std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << message1 << '\n';
        SPD_L->info(message1);
        for (size_t i = 0; i < collection_.GetSize(); ++i)
        {
            const std::unique_ptr deviceSmartPtr(collection_.CreateItem(i));
            FormattedOutput::PrintDeviceInfo(deviceSmartPtr.get());
            AudioDeviceApiClient(apiBaseUrl_).PostDeviceToApi(deviceSmartPtr.get());
        }
        const std::string message2("...Posting device collection finished."); std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << message2 << '\n';
        SPD_L->info(message2);
    }

    void OnCollectionChanged(AudioDeviceCollectionEvent event, const std::wstring& devicePnpId) override
	{
        FormattedOutput::PrintEvent(event, devicePnpId);

        if (event == AudioDeviceCollectionEvent::Discovered
            || event == AudioDeviceCollectionEvent::VolumeChanged
        )
        {
			PostAndPrintCollection();
        }
    }

    void OnTrace(const std::wstring& line) override
    {
        SPD_L->info(FormattedOutput::WString2StringTruncate(line));
    }

    void OnTraceDebug(const std::wstring& line) override
    {
        OnTrace(line);
    }

private:
    AudioDeviceCollectionInterface& collection_;
    std::wstring apiBaseUrl_;
};



class AudioDeviceService final : public Poco::Util::ServerApplication {
protected:
    int main(const std::vector<std::string>& args) override {
        try {
            const auto msgStart = "Starting Sound Agent Service..."; std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msgStart << '\n';
            SPD_L->info(msgStart);

            const auto coll(SoundAgent::CreateDeviceCollection(L"", true));
            Observer o(*coll, apiBaseUrl_);
            coll->Subscribe(o);

            coll->ResetContent();
            o.PostAndPrintCollection();

            waitForTerminationRequest();

            coll->Unsubscribe(o);

            const auto msgStop = "Stopping service..."; std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msgStop << '\n';
            SPD_L->info(msgStop);
            return EXIT_OK;
        }
        catch (const Poco::Exception& ex) {
			SPD_L-> error(ex.displayText());
            return EXIT_SOFTWARE;
        }
    }

    void initialize(Application& self) override {
        loadConfiguration();
        ServerApplication::initialize(self);

        if (config().hasProperty(ApiBaseUrlPropertyKey))
        {
            const auto narrowVal = config().getString(ApiBaseUrlPropertyKey);
            apiBaseUrl_ = std::wstring(narrowVal.length(), L' ');
            std::ranges::copy(narrowVal, apiBaseUrl_.begin());
        }

        // Windows service registration
        setUnixOptions(false);  // Force Windows service behavior
    }
private:
	std::wstring apiBaseUrl_ = L"http://localhost:5027/api/AudioDevices";
    static constexpr auto ApiBaseUrlPropertyKey = "custom.apiBaseUrl";
};

int _tmain(int argc, _TCHAR * argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);

    if (std::filesystem::path logFile;
        ed::utility::AppPath::GetAndValidateLogFileInProgramData(
            logFile, RESOURCE_FILENAME_ATTRIBUTE)
        )
    {
        ed::model::Logger::Inst().SetPathName(logFile);
    }

    ed::CoInitRaiiHelper coInitHelper;

    std::vector<std::string> args;
    std::vector<char*> argPtrs;

    for (int i = 0; i < argc; ++i) {
        std::string utf8Arg;
        Poco::UnicodeConverter::toUTF16(argv[i], utf8Arg); // MSBuild uses UTF-16
        args.push_back(utf8Arg);
    }

    for (auto& arg : args) {
        argPtrs.push_back(arg.data());
    }

    AudioDeviceService app;
    return app.run(static_cast<int>(argPtrs.size()), argPtrs.data());
}
