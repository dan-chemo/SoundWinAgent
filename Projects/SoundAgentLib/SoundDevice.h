#pragma once

#include <string>

#include "../SoundAgentDll/SoundAgentInterface.h"

namespace ed::audio {
class SoundDevice final : public SoundDeviceInterface {
public:
    ~SoundDevice() override;

public:
    SoundDevice();
    SoundDevice(std::wstring pnpGuid, std::wstring name, DeviceFlowEnum flow, uint16_t renderVolume, uint16_t captureVolume);
    SoundDevice(const SoundDevice & toCopy);
    SoundDevice(SoundDevice && toMove) noexcept;
    SoundDevice & operator=(const SoundDevice & toCopy);
    SoundDevice & operator=(SoundDevice && toMove) noexcept;

public:
    [[nodiscard]] std::wstring GetName() const override;
    [[nodiscard]] std::wstring GetPnpId() const override;
    [[nodiscard]] DeviceFlowEnum GetFlow() const override;
    [[nodiscard]] uint16_t GetCurrentRenderVolume() const override;
    [[nodiscard]] uint16_t GetCurrentCaptureVolume() const override;
    void SetCurrentRenderVolume(uint16_t volume);
    void SetCurrentCaptureVolume(uint16_t volume);

private:
    std::wstring pnpGuid_;
    std::wstring name_;
    DeviceFlowEnum flow_;
    uint16_t renderVolume_;
    uint16_t captureVolume_;
};
}
