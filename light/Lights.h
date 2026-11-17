/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <Devices.h>
#include <models/State.h>

#include <aidl/android/hardware/light/BnLights.h>
#include <aidl/vendor/daylight/hardware/light/ext/Backlight.h>
#include <mutex>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

class Lights : public BnLights {
  public:
    Lights();

    ndk::ScopedAStatus setLightState(int32_t id, const HwLightState& hwLightState) override;
    ndk::ScopedAStatus getLights(std::vector<HwLight>* _aidl_return) override;

    ndk::ScopedAStatus getBacklights(
            std::vector<::aidl::vendor::daylight::hardware::light::ext::Backlight>* backlights);
    ndk::ScopedAStatus setBacklightBrightness(
            ::aidl::vendor::daylight::hardware::light::ext::Backlight backlight, int32_t brightness);

    binder_status_t dump(int fd, const char** args, uint32_t numArgs) override;

  private:
    std::vector<HwLight> mLights;

    Devices mDevices;
    std::mutex mBacklightMutex;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
