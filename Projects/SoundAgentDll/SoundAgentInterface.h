// ReSharper disable CppClangTidyModernizeUseNodiscard
#pragma once

#ifdef AC_EXPORTS
#define AC_EXPORT_IMPORT_DECL __declspec(dllexport)
#else
#define AC_EXPORT_IMPORT_DECL __declspec(dllimport)
#endif

#include <memory>

#include "ClassDefHelper.h"

class AudioDeviceCollectionInterface;
class DeviceCollectionObserver;
class SoundDeviceInterface;
class AudioDeviceCollectionObserverInterface;

enum class AC_EXPORT_IMPORT_DECL AudioDeviceCollectionEvent : uint8_t {
    None = 0,
    Discovered,
    Detached,
    VolumeChanged
};

enum class AC_EXPORT_IMPORT_DECL DeviceFlowEnum : uint8_t {
    None = 0,
    Render,
    Capture,
    RenderAndCapture
};

class AC_EXPORT_IMPORT_DECL SoundAgent {
public:
    static std::unique_ptr<AudioDeviceCollectionInterface> CreateDeviceCollection(
        const std::wstring & nameFilter, bool bothHeadsetAndMicro = false);

    DISALLOW_COPY_MOVE(SoundAgent);
    SoundAgent() = delete;
    ~SoundAgent() = delete;
};

class AC_EXPORT_IMPORT_DECL AudioDeviceCollectionInterface {
public:
    virtual size_t GetSize() const = 0;
    virtual std::unique_ptr<SoundDeviceInterface> CreateItem(size_t deviceNumber) const = 0;

    virtual void Subscribe(AudioDeviceCollectionObserverInterface & observer) = 0;
    virtual void Unsubscribe(AudioDeviceCollectionObserverInterface & observer) = 0;

    virtual void ResetContent() = 0;

    AS_INTERFACE(AudioDeviceCollectionInterface);
    DISALLOW_COPY_MOVE(AudioDeviceCollectionInterface);
};

class AC_EXPORT_IMPORT_DECL AudioDeviceCollectionObserverInterface {
public:
    virtual void OnCollectionChanged(AudioDeviceCollectionEvent event, const std::wstring & devicePnpId) = 0;
    virtual void OnTrace(const std::wstring & line) = 0;
    virtual void OnTraceDebug(const std::wstring & line) = 0;

    AS_INTERFACE(AudioDeviceCollectionObserverInterface);
    DISALLOW_COPY_MOVE(AudioDeviceCollectionObserverInterface);
};


class AC_EXPORT_IMPORT_DECL SoundDeviceInterface {
public:
    virtual std::wstring GetName() const = 0;
    virtual std::wstring GetPnpId() const = 0;
    virtual DeviceFlowEnum GetFlow() const = 0;
    virtual uint16_t GetCurrentRenderVolume() const = 0;
    virtual uint16_t GetCurrentCaptureVolume() const = 0;

    AS_INTERFACE(SoundDeviceInterface);
    DISALLOW_COPY_MOVE(SoundDeviceInterface);
};
