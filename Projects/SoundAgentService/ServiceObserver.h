#pragma once

#include "../SoundAgentDll/SoundAgentInterface.h"

class HttpRequestProcessor;

class ServiceObserver final : public AudioDeviceCollectionObserverInterface {
public:
    explicit ServiceObserver(AudioDeviceCollectionInterface& collection, std::wstring apiBaseUrl);

    DISALLOW_COPY_MOVE(ServiceObserver);
    ~ServiceObserver() override = default;

public:
    void PostAndPrintCollection() const;

    void OnCollectionChanged(AudioDeviceCollectionEvent event, const std::wstring& devicePnpId) override;

    void OnTrace(const std::wstring& line) override;

    void OnTraceDebug(const std::wstring& line) override
    {
        OnTrace(line);
    }

private:
    AudioDeviceCollectionInterface& collection_;
    std::wstring apiBaseUrl_;
    std::shared_ptr<HttpRequestProcessor> requestProcessorSmartPtr_;
};

