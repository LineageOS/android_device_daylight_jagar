/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LightsExt.h"

#include <utility>

namespace aidl::vendor::daylight::hardware::light::ext {

LightsExt::LightsExt(std::shared_ptr<::aidl::android::hardware::light::Lights> lights)
    : mLights(std::move(lights)) {}

ndk::ScopedAStatus LightsExt::getBacklights(std::vector<Backlight>* backlights) {
    return mLights->getBacklights(backlights);
}

ndk::ScopedAStatus LightsExt::setBacklightBrightness(Backlight backlight, int32_t brightness) {
    return mLights->setBacklightBrightness(backlight, brightness);
}

}  // namespace aidl::vendor::daylight::hardware::light::ext
