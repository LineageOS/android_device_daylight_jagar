/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <devices/LedDevice.h>
#include <models/IDumpable.h>
#include <models/State.h>

#include <vector>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

class Devices : public IDumpable {
  public:
    Devices();

    void dump(int fd) const override;

    bool hasBacklightDevices() const;

    void setBacklightState(const State& state);

    LedDevice* getBacklightDevice(const std::string& name);

  private:
    // Backlight
    std::vector<LedDevice> mBacklightLedDevices;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
