#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixups,
    lib_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'device/daylight/jagar',
    'hardware/mediatek',
    'hardware/mediatek/libion_mtk',
    'hardware/mediatek/libmtkperf_client',
]


def lib_fixup_vendor_suffix(lib: str, partition: str, *args, **kwargs):
    return f'{lib}_{partition}' if partition == 'vendor' else None


lib_fixups: lib_fixups_user_type = {
    **lib_fixups,
}

blob_fixups: blob_fixups_user_type = {
    (
        'vendor/lib/libcodec2_mtk_venc.so',
        'vendor/lib/libcodec2_mtk_vdec.so',
        'vendor/lib64/libcodec2_mtk_venc.so',
        'vendor/lib64/libcodec2_mtk_vdec.so',
    ): blob_fixup()
        .replace_needed('libformatter.so', 'libformatter_mtk.so'),
    (
        'vendor/lib/libformatter_mtk.so',
        'vendor/lib64/libformatter_mtk.so',
    ): blob_fixup()
        .fix_soname(),
    (
        'vendor/lib64/android.hardware.power-service-mediatek.so'
    ): blob_fixup()
        .replace_needed('android.hardware.power-V2-ndk_platform.so', 'android.hardware.power-V2-ndk.so'),
    (
        'vendor/lib64/hw/hwcomposer.mtk_common.so'
    ): blob_fixup()
        .add_needed('libprocessgroup_shim.so'),
    (
        'vendor/lib/libneuralnetworks_sl_driver_mtk_prebuilt.so',
        'vendor/lib64/libneuralnetworks_sl_driver_mtk_prebuilt.so',
    ): blob_fixup()
        .clear_symbol_version('AHardwareBuffer_allocate')
        .clear_symbol_version('AHardwareBuffer_describe')
        .clear_symbol_version('AHardwareBuffer_lock')
        .clear_symbol_version('AHardwareBuffer_release')
        .clear_symbol_version('AHardwareBuffer_unlock')
        .clear_symbol_version('AHardwareBuffer_createFromHandle')
        .clear_symbol_version('AHardwareBuffer_getNativeHandle'),
    (
        'vendor/lib/libnvram.so',
        'vendor/lib64/libnvram.so',
        'vendor/lib/libsysenv.so',
        'vendor/lib64/libsysenv.so',
    ): blob_fixup()
        .add_needed('libbase_shim.so'),
    (
        'vendor/lib/libspeech_enh_lib.so',
        'vendor/lib64/libspeech_enh_lib.so',
        'vendor/lib64/libwifi-hal-mtk.so',
        'vendor/lib64/hw/sensors.mt6789.so',
        'vendor/lib/hw/audio.primary.mt6789.so',
        'vendor/lib64/hw/audio.primary.mt6789.so',
        'vendor/lib/hw/audio.r_submix.mt6789.so',
        'vendor/lib64/hw/audio.r_submix.mt6789.so',
    ): blob_fixup()
        .fix_soname(),
    (
        'vendor/bin/hw/android.hardware.media.c2@1.2-mediatek',
        'vendor/bin/hw/android.hardware.media.c2@1.2-mediatek-64b',
    ): blob_fixup()
        .replace_needed('libavservices_minijail_vendor.so', 'libavservices_minijail.so')
        .add_needed('libstagefright_foundation-v33.so'),
    (
        'vendor/etc/sensors/hals.conf'
    ): blob_fixup()
        .regex_replace('android.hardware.sensors@2.X-subhal-mediatek.so', 'android.hardware.sensors@2.0-subhal-impl-1.0.so'),
}  # fmt: skip

module = ExtractUtilsModule(
    'jagar',
    'daylight',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
    add_firmware_proprietary_file=True,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
