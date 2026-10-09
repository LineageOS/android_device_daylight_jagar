#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

# Inherit from those products. Most specific first.
$(call inherit-product, $(SRC_TARGET_DIR)/product/core_64_bit.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base.mk)

# Inherit from device makefile.
$(call inherit-product, device/daylight/jagar/device.mk)

# Inherit some common LineageOS stuff.
$(call inherit-product, vendor/lineage/config/common_full_tablet_wifionly.mk)

PRODUCT_NAME := lineage_jagar
PRODUCT_DEVICE := jagar
PRODUCT_MANUFACTURER := Daylight
PRODUCT_BRAND := Daylight
PRODUCT_MODEL := Daylight DC-1

PRODUCT_BUILD_PROP_OVERRIDES += \
    BuildFingerprint=Daylight/jagar/jagar:13/TP1A.220624.014/2602101745:user/release-keys \
    DeviceProduct=jagar
