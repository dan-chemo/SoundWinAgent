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
#include "../SoundAgentLib/DefToString.h"

#include "FormattedOutput.h"

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
        FormattedOutput::PrintEvent(event, devicePnpId);
        FormattedOutput::PrintCollection(collection_);

        const std::shared_ptr foundDevice = FindDevice(devicePnpId);
		if (!foundDevice)
		{
			const std::wstring line = L"Device with PnP id " + devicePnpId + L" not found in the collection.";
            SPD_L->error(FormattedOutput::WString2StringTruncate(line));
            std::wcout << FormattedOutput::CurrentLocalTimeAsWideStringWithoutDate << line << '\n';
            return;
		}

		if (event == AudioDeviceCollectionEvent::Discovered)
        {
            const AudioDeviceApiClient apiClient(L"http://localhost:5027/api/AudioDevices");

            // Example device data
            const std::wstring pnpId = foundDevice->GetPnpId();
            const std::wstring name = foundDevice->GetName();
            const int volume = foundDevice->GetCurrentRenderVolume();
            const std::wstring hostName = foundDevice->GetName();

            // Post device data to the API
            apiClient.PostDeviceToApi(pnpId, name, volume, hostName);
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
    [[nodiscard]] std::shared_ptr<SoundDeviceInterface> FindDevice(const std::wstring & pnpId) const
    {
        const size_t deviceCount = collection_.GetSize();
        for (size_t i = 0; i < deviceCount; ++i)
        {
            if (std::unique_ptr device(collection_.CreateItem(i))
              ; device && device->GetPnpId() == pnpId
            )
            {
                return std::shared_ptr<SoundDeviceInterface>(device.release());
            }
        }
        return nullptr;
    }
private:
    AudioDeviceCollectionInterface& collection_;
};



class AudioDeviceService final : public Poco::Util::ServerApplication {
protected:
    int main(const std::vector<std::string>& args) override {
        try {
            const auto msgStart = "Starting Sound Agent Service..."; std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msgStart << '\n';
            SPD_L->info(msgStart);

            const auto coll(SoundAgent::CreateDeviceCollection(L"", true));
            Observer o(*coll);
            coll->Subscribe(o);

            waitForTerminationRequest();

            coll->Unsubscribe(o);

            const auto msgStop = "Stopping service..."; std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msgStop << '\n';
            SPD_L->info(msgStop);
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

        // Set service name (if not provided in config)
        if (!config().hasProperty("application.name")) {
            config().setString("application.name", "SoundAgentService");
        }

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
