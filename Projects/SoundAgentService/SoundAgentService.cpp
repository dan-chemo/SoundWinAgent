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
#include "../SoundAgentLib/CoInitRaiiHelper.h"
#include "../SoundAgentDll/SoundAgentInterface.h"
#include "../SoundAgentLib/DefToString.h"

class Observer final : public AudioDeviceCollectionObserverInterface {
public:
    explicit Observer(AudioDeviceCollectionInterface& collection)
        : collection_(collection)
    {
    }

    DISALLOW_COPY_MOVE(Observer);
    ~Observer() override = default;

public:
    void OnCollectionChanged(AudioDeviceCollectionEvent event, const std::wstring& devicePnpId) override
	{
		std::wstring eventStr = ed::GetDeviceCollectionEventAsString(event);
//		SPD_L->info(L"Event: " + eventStr + L" PnpId: " + devicePnpId);
		if (event == AudioDeviceCollectionEvent::Discovered)
        {
            AudioDeviceApiClient apiClient(L"https://your-api-endpoint.com");

            // Example device data
            const std::string pnpId = "USB\\VID_1234&PID_5678";
            const std::string name = "Speakers (High Definition Audio Device)";
            constexpr int volume = 75;
            const std::string hostName = "My-PC";

            // Post device data to the API
            apiClient.PostDeviceToApi(pnpId, name, volume, hostName);
        }
    }

    void OnTrace(const std::wstring& line) override
    {
        std::string result; result.reserve(line.size());
        std::ranges::for_each(line, [&result](const auto p)
            {
                result += static_cast<char>(p);
            });

        SPD_L->info(result);
    }

    void OnTraceDebug(const std::wstring& line) override
    {
        OnTrace(line);
    }

private:
    AudioDeviceCollectionInterface& collection_;
};



class AudioDeviceService final : public Poco::Util::ServerApplication {
protected:
    int main(const std::vector<std::string>& args) override {
        try {
            // Service initialization
            SPD_L->info("Starting Audio Device Service");

            const auto coll(SoundAgent::CreateDeviceCollection(L"", true));
            Observer o(*coll);
            coll->Subscribe(o);

            waitForTerminationRequest();

            coll->Unsubscribe(o);

            SPD_L->info("Stopping service");
            return Application::EXIT_OK;
        }
        catch (const Poco::Exception& ex) {
			SPD_L-> error(ex.displayText());
            return EXIT_SOFTWARE;
        }
    }

    void initialize(Application& self) override {
        loadConfiguration();  // Load service config from XML if needed
        ServerApplication::initialize(self);

        // Windows service registration
        setUnixOptions(false);  // Force Windows service behavior
    }
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
