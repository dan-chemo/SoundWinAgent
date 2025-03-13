#pragma once

#include <cpprest/http_client.h>
#include <string>

class SoundDeviceInterface;

class AudioDeviceApiClient {
public:
    explicit AudioDeviceApiClient(std::wstring apiBaseUrl);

    // Post device data to the REST API using a SoundDeviceInterface
    void PostDeviceToApi(const SoundDeviceInterface* device) const;

private:
    std::wstring apiBaseUrl_;
};
