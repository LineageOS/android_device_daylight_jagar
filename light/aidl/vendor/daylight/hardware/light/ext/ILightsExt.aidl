/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.daylight.hardware.light.ext;

import vendor.daylight.hardware.light.ext.Backlight;

@VintfStability
interface ILightsExt {
    /** Returns the detected WHITE and/or AMBER backlight channels. */
    Backlight[] getBacklights();

    /**
     * Sets only the selected backlight to a brightness in [0, 255], scaled to its hardware maximum.
     * Zero turns the channel off.
     *
     * Throws IllegalArgumentException for an invalid selector or a brightness outside [0, 255].
     * Throws UnsupportedOperationException if the selected channel is not present.
     * Throws ServiceSpecificException with ERROR_IO if the sysfs write fails.
     */
    void setBacklightBrightness(Backlight backlight, int brightness);

    const int ERROR_IO = 1;
}
