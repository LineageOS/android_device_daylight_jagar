/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <devices/LedDevice.h>

#define LOG_TAG "LedDevice"

#include <Utils.h>

#include <android-base/logging.h>
#include <fstream>
#include <thread>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

static const uint32_t kDefaultMaxBrightness = 255;

static const std::string kBaseLedsPath = "/sys/class/leds/";

static const std::string kBrightnessNode = "brightness";
static const std::string kMaxBrightnessNode = "max_brightness";

static const std::string kTriggerNode = "trigger";
static const std::string kDelayOffNode = "delay_off";
static const std::string kDelayOnNode = "delay_on";

LedDevice::LedDevice(std::string name)
    : mName(name), mBasePath(kBaseLedsPath + name + "/") {
    if (!readFromFile(mBasePath + kMaxBrightnessNode, mMaxBrightness)) {
        mMaxBrightness = kDefaultMaxBrightness;
    }

    mSupportsTimedUpstream = false;
    std::ifstream triggerNode(mBasePath + kTriggerNode);
    if (triggerNode.good()) {
        std::string line;
        while (std::getline(triggerNode, line)) {
            if (line.find("timer") != std::string::npos) {
                mSupportsTimedUpstream = true;
                break;
            }
        }
    }
}

bool LedDevice::isOk() const {
    return std::ifstream(mBasePath + kBrightnessNode).good();
}

bool LedDevice::setState(const State& state) {
    LightMode mode = LightMode::STATIC;
    uint32_t flashOnMs = 0, flashOffMs = 0;
    if (state.effect.type == Effect::Type::TIMED && supportsMode(LightMode::TIMED_UPSTREAM)) {
        mode = LightMode::TIMED_UPSTREAM;
        flashOnMs = state.effect.timed.onMs;
        flashOffMs = state.effect.timed.offMs;
    }

    return setBrightness(state.color.toBrightness(), mode, flashOnMs, flashOffMs);
}

std::string LedDevice::getName() const {
    return mName;
}

bool LedDevice::supportsMode(LightMode mode) const {
    switch (mode) {
        case LightMode::STATIC:
            return true;
        case LightMode::TIMED_UPSTREAM:
            return mSupportsTimedUpstream;
    }

    return false;
}

bool LedDevice::setBrightness(uint8_t value, LightMode mode, uint32_t flashOnMs,
                              uint32_t flashOffMs) {
    // Disable current blinking
    if (mode != LightMode::TIMED_UPSTREAM && supportsMode(LightMode::TIMED_UPSTREAM)) {
        writeToFile(mBasePath + kTriggerNode, "none");
    }
    if (mode != LightMode::STATIC && supportsMode(LightMode::STATIC)) {
        writeToFile(mBasePath + kBrightnessNode, 0);
    }

    switch (mode) {
        case LightMode::TIMED_UPSTREAM: {
            bool ok = writeToFile(mBasePath + kTriggerNode, value > 0 ? "timer" : "none");
            if (ok && value > 0) {
                using namespace std::chrono_literals;
                auto retries = 20;
                while (retries--) {
                    std::this_thread::sleep_for(2ms);

                    ok = writeToFile(mBasePath + kDelayOffNode, flashOffMs);
                    if (!ok) continue;

                    ok = writeToFile(mBasePath + kDelayOnNode, flashOnMs);
                    if (ok) break;
                }
            }
            return ok;
        }
        case LightMode::STATIC: {
            return writeToFile(mBasePath + kBrightnessNode, scaleBrightness(value, mMaxBrightness));
        }
        default: {
            LOG(ERROR) << "Unknown mode: " << mode;
            return false;
        }
    }
}

void LedDevice::dump(int fd) const {
    dprintf(fd, "Name: %s", mName.c_str());
    dprintf(fd, ", is ok: %d", isOk());
    dprintf(fd, ", base path: %s", mBasePath.c_str());
    dprintf(fd, ", max brightness: %u", mMaxBrightness);
    dprintf(fd, ", supports timed upstream: %d",
            supportsMode(LedDevice::LightMode::TIMED_UPSTREAM));
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
