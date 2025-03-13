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
#include "ServiceObserver.h"

#include "../SoundAgentLib/CoInitRaiiHelper.h"
#include "../SoundAgentDll/SoundAgentInterface.h"


class AudioDeviceService final : public Poco::Util::ServerApplication {
protected:
    int main(const std::vector<std::string>& args) override {
        try {
            const auto msgStart = "Starting Sound Agent..."; std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msgStart << '\n';
            SPD_L->info(msgStart);

            const auto coll(SoundAgent::CreateDeviceCollection(L"", true));
            ServiceObserver o(*coll, apiBaseUrl_);
            coll->Subscribe(o);

            coll->ResetContent();
            o.PostAndPrintCollection();

            waitForTerminationRequest();

            coll->Unsubscribe(o);

            const auto msgStop = "Stopping..."; std::cout << FormattedOutput::CurrentLocalTimeWithoutDate << msgStop << '\n';
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

        setUnixOptions(false);  // Force Windows service behavior
    }
private:
	std::wstring apiBaseUrl_;
    // bool isService_ = config().getBool("application.runAsService", false);
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
