#pragma once

#include <cpprest/http_client.h>
#include <string>

class HttpRequestProcessor;
class SoundDeviceInterface;

class AudioDeviceApiClient {
public:
    explicit AudioDeviceApiClient(std::wstring apiBaseUrl, std::shared_ptr<HttpRequestProcessor> processor = nullptr);

    // Post device data to the REST API using a SoundDeviceInterface
    void PostDeviceToApi(const SoundDeviceInterface* device) const;

private:
    std::wstring apiBaseUrl_;
    std::shared_ptr<HttpRequestProcessor> requestProcessor_;
};
