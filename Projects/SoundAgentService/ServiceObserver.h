﻿#pragma once

#include <public/SoundAgentInterface.h>

class HttpRequestProcessor;

class ServiceObserver final : public SoundDeviceObserverInterface {
public:
    ServiceObserver(SoundDeviceCollectionInterface& collection,
        std::wstring apiBaseUrl,
        std::wstring universalToken,
        std::wstring codespaceName); // Added codespaceName parameter

    void PostToApi(SoundDeviceEventType messageType, const SoundDeviceInterface* devicePtr) const;

    DISALLOW_COPY_MOVE(ServiceObserver);
    ~ServiceObserver() override = default;

public:
    void PostAndPrintCollection() const;

    void OnCollectionChanged(SoundDeviceEventType event, const std::wstring& devicePnpId) override;

    void OnTrace(const std::wstring& line) override;

    void OnTraceDebug(const std::wstring& line) override
    {
        OnTrace(line);
    }

private:
    SoundDeviceCollectionInterface& collection_;
    std::wstring apiBaseUrl_;
    std::wstring universalToken_;
    std::wstring codespaceName_; // Newly added member for codespaceName
    std::shared_ptr<HttpRequestProcessor> requestProcessorSmartPtr_;
};
