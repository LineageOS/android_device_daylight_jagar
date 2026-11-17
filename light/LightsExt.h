/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "Lights.h"

#include <aidl/vendor/daylight/hardware/light/ext/BnLightsExt.h>

namespace aidl::vendor::daylight::hardware::light::ext {

class LightsExt : public BnLightsExt {
  public:
    explicit LightsExt(std::shared_ptr<::aidl::android::hardware::light::Lights> lights);

    ndk::ScopedAStatus getBacklights(std::vector<Backlight>* backlights) override;
    ndk::ScopedAStatus setBacklightBrightness(Backlight backlight, int32_t brightness) override;

  private:
    const std::shared_ptr<::aidl::android::hardware::light::Lights> mLights;
};

}  // namespace aidl::vendor::daylight::hardware::light::ext
