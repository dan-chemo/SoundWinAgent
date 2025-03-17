#include "stdafx.h"

#include <SpdLogger.h>

#include <filesystem>
#include <memory>
#include <tchar.h>

#include <Poco/Util/ServerApplication.h>
#include <Poco/UnicodeConverter.h>
#include <vector>

#include "SodiumCrypt.h"
#include "AudioDeviceApiClient.h"
#include "FormattedOutput.h"
#include "ServiceObserver.h"

#include "../SoundAgentLib/CoInitRaiiHelper.h"
#include "../SoundAgentDll/SoundAgentInterface.h"


class AudioDeviceService final : public Poco::Util::ServerApplication {
protected:
    int main(const std::vector<std::string>& args) override {
        try {
            const auto msgStart = "Starting Sound Agent...";
            FormattedOutput::LogAndPrint(msgStart);

            const auto coll(SoundAgent::CreateDeviceCollection(L"", true));
            ServiceObserver serviceObserver(*coll, apiBaseUrl_, universalToken_);
            coll->Subscribe(serviceObserver);

            coll->ResetContent();
            serviceObserver.PostAndPrintCollection();

            waitForTerminationRequest();

            coll->Unsubscribe(serviceObserver);

            const auto msgStop = "Stopping...";
            FormattedOutput::LogAndPrint(msgStop);

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

        if (!config().hasProperty(ApiBaseUrlPropertyKey))
        {
			const auto msg = "No API base URL configured. Skipping API call.";
			FormattedOutput::LogAndPrint(msg);
			return;
        }

        auto narrowVal = config().getString(ApiBaseUrlPropertyKey);
		narrowVal = SodiumDecrypt(narrowVal, "32-characters-long-secure-key-12");

        apiBaseUrl_ = std::wstring(narrowVal.length(), L' ');
        std::ranges::copy(narrowVal, apiBaseUrl_.begin());

        if (!config().hasProperty(UniversalTokenPropertyKey))
        {
			const auto msg = "No universal token configured. Skipping API call.";
            FormattedOutput::LogAndPrint(msg);
            return;
        }

        auto universalTokenNarrow = config().getString(UniversalTokenPropertyKey);
        universalTokenNarrow = SodiumDecrypt(universalTokenNarrow, "32-characters-long-secure-key-12");
        universalToken_ = std::wstring(universalTokenNarrow.length(), L' ');
        std::ranges::copy(universalTokenNarrow, universalToken_.begin());


        setUnixOptions(false);  // Force Windows service behavior
    }
private:
	std::wstring apiBaseUrl_;
	std::wstring universalToken_;
    // bool isService_ = config().getBool("application.runAsService", false);
    static constexpr auto ApiBaseUrlPropertyKey = "custom.apiBaseUrl";
    static constexpr auto UniversalTokenPropertyKey = "custom.universalToken";
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

	// Transform Unicode command line arguments to UTF-8
    std::vector<std::string> args;
    std::vector<char*> charPointers;

    for (int i = 0; i < argc; ++i) {
        std::string utf8Arg;
        Poco::UnicodeConverter::toUTF16(argv[i], utf8Arg);
        args.push_back(utf8Arg);
    }

    for (auto& arg : args) {
        charPointers.push_back(arg.data());
    }

    AudioDeviceService app;
    return app.run(static_cast<int>(charPointers.size()), charPointers.data());
}
