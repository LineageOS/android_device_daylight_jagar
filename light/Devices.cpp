/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <Devices.h>

#define LOG_TAG "Devices"

#include <android-base/logging.h>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

static const std::string kLedBacklightDevices[] = {
        "lcd-backlight",
        "lcd-backlight-amber",
};

static std::vector<LedDevice> getBacklightLedDevices() {
    std::vector<LedDevice> devices;

    for (const auto& device : kLedBacklightDevices) {
        LedDevice backlight(device);
        if (backlight.isOk()) {
            LOG(INFO) << "Found backlight LED device: " << backlight.getName();
            devices.push_back(backlight);
        }
    }

    return devices;
}

Devices::Devices()
    : mBacklightLedDevices(getBacklightLedDevices()) {
    if (!hasBacklightDevices()) {
        LOG(INFO) << "No backlight devices found";
    }
}

bool Devices::hasBacklightDevices() const {
    return !mBacklightLedDevices.empty();
}

void Devices::setBacklightState(const State& state) {
    for (auto& device : mBacklightLedDevices) {
        device.setState(state);
    }
}

LedDevice* Devices::getBacklightDevice(const std::string& name) {
    for (auto& device : mBacklightLedDevices) {
        if (device.getName() == name) {
            return &device;
        }
    }
    return nullptr;
}

void Devices::dump(int fd) const {
    dprintf(fd, "Backlight LED devices:\n");
    for (const auto& device : mBacklightLedDevices) {
        dprintf(fd, "- ");
        device.dump(fd);
        dprintf(fd, "\n");
    }

    return;
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
