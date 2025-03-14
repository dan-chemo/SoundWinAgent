#pragma once

#include <cpprest/http_client.h>
#include <string>

class HttpRequestProcessor;
class SoundDeviceInterface;

class AudioDeviceApiClient {
public:
    explicit AudioDeviceApiClient(std::shared_ptr<HttpRequestProcessor> processor);

    // Post device data to the REST API using a SoundDeviceInterface
    void PostDeviceToApi(const SoundDeviceInterface* device) const;

private:
    std::shared_ptr<HttpRequestProcessor> requestProcessor_;
};
