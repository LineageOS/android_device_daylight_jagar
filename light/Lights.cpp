/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <Lights.h>

#define LOG_TAG "Lights"

#include <Utils.h>

#include <android-base/logging.h>
#include <aidl/vendor/daylight/hardware/light/ext/ILightsExt.h>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

using ::aidl::vendor::daylight::hardware::light::ext::Backlight;

static const char* backlightDeviceName(Backlight backlight) {
    switch (backlight) {
        case Backlight::WHITE:
            return "lcd-backlight";
        case Backlight::AMBER:
            return "lcd-backlight-amber";
        default:
            return nullptr;
    }
}

#define AutoHwLight(light) {.id = static_cast<int32_t>(light), .ordinal = 0, .type = light}

Lights::Lights() {
    if (mDevices.hasBacklightDevices()) {
        mLights.push_back(AutoHwLight(LightType::BACKLIGHT));
    }
}

ndk::ScopedAStatus Lights::setLightState(int32_t id, const HwLightState& hwLightState) {
    std::lock_guard<std::mutex> lock(mBacklightMutex);
    State state = fromAidl(hwLightState);

    LightType type = static_cast<LightType>(id);
    switch (type) {
        case LightType::BACKLIGHT:
            if (mDevices.hasBacklightDevices()) {
                mDevices.setBacklightState(state);
            } else {
                return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
            }
            break;
        case LightType::KEYBOARD:
        case LightType::BUTTONS:
        case LightType::BATTERY:
        case LightType::NOTIFICATIONS:
        case LightType::ATTENTION:
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
        case LightType::MICROPHONE:
        case LightType::CAMERA:
            break;
        default:
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Lights::getLights(std::vector<HwLight>* _aidl_return) {
    for (const auto& light : mLights) {
        _aidl_return->push_back(light);
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Lights::getBacklights(std::vector<Backlight>* backlights) {
    std::lock_guard<std::mutex> lock(mBacklightMutex);
    backlights->clear();
    for (Backlight backlight : {Backlight::WHITE, Backlight::AMBER}) {
        if (mDevices.getBacklightDevice(backlightDeviceName(backlight)) != nullptr) {
            backlights->push_back(backlight);
        }
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Lights::setBacklightBrightness(Backlight backlight, int32_t brightness) {
    std::lock_guard<std::mutex> lock(mBacklightMutex);
    if (brightness < 0 || brightness > 255) {
        return ndk::ScopedAStatus::fromExceptionCodeWithMessage(
                EX_ILLEGAL_ARGUMENT, "Brightness must be in [0, 255]");
    }
    const char* name = backlightDeviceName(backlight);
    if (name == nullptr) {
        return ndk::ScopedAStatus::fromExceptionCodeWithMessage(
                EX_ILLEGAL_ARGUMENT, "Invalid backlight selector");
    }
    auto* device = mDevices.getBacklightDevice(name);
    if (device == nullptr) {
        return ndk::ScopedAStatus::fromExceptionCodeWithMessage(
                EX_UNSUPPORTED_OPERATION, "Backlight channel is not present");
    }
    if (!device->setBrightness(static_cast<uint8_t>(brightness))) {
        return ndk::ScopedAStatus::fromServiceSpecificErrorWithMessage(
                ::aidl::vendor::daylight::hardware::light::ext::ILightsExt::ERROR_IO,
                "Failed to write backlight brightness");
    }
    return ndk::ScopedAStatus::ok();
}

binder_status_t Lights::dump(int fd, const char** /*args*/, uint32_t /*numArgs*/) {
    std::lock_guard<std::mutex> lock(mBacklightMutex);
    dprintf(fd, "Lights AIDL:\n");
    dprintf(fd, "\n");

    dprintf(fd, "Lights:\n");
    for (const auto& light : mLights) {
        dprintf(fd, "- %d: LightType::%s\n", light.id, toString(light.type).c_str());
    }
    dprintf(fd, "\n");

    dprintf(fd, "Devices:\n");
    mDevices.dump(fd);
    dprintf(fd, "\n");

    return STATUS_OK;
}


}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
