/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Lights.h"
#include "LightsExt.h"

#define LOG_TAG "android.hardware.light-service.jagar"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

using ::aidl::android::hardware::light::Lights;
using ::aidl::vendor::daylight::hardware::light::ext::LightsExt;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    std::shared_ptr<Lights> lights = ndk::SharedRefBase::make<Lights>();
    ndk::SpAIBinder binder = lights->asBinder();
    auto extension = ndk::SharedRefBase::make<LightsExt>(lights);
    CHECK(AIBinder_setExtension(binder.get(), extension->asBinder().get()) == STATUS_OK);

    const std::string instance = std::string() + Lights::descriptor + "/default";
    binder_status_t status = AServiceManager_addService(binder.get(), instance.c_str());
    CHECK(status == STATUS_OK);

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
