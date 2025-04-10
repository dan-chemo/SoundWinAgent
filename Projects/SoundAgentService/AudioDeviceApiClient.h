﻿#pragma once

#include <cpprest/http_client.h>
#include <memory>


class HttpRequestProcessor;
class SoundDeviceInterface;


class AudioDeviceApiClient {
public:
    explicit AudioDeviceApiClient(std::shared_ptr<HttpRequestProcessor> processor);

    void PostDeviceToApi(SoundDeviceEventType eventType, const SoundDeviceInterface* device, const std::string & hint = "") const;
private:
    static std::string GetHostNameHash();
    static std::string ShortHash(const std::string& input);
private:
    std::shared_ptr<HttpRequestProcessor> requestProcessor_;
};
