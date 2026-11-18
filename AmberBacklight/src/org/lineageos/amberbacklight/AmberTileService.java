/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.amberbacklight;

import android.app.Dialog;
import android.content.SharedPreferences;
import android.hardware.display.DisplayManager;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.service.quicksettings.Tile;
import android.service.quicksettings.TileService;
import android.util.Log;
import android.view.Display;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.accessibility.AccessibilityManager;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import vendor.daylight.hardware.light.ext.Backlight;
import vendor.daylight.hardware.light.ext.ILightsExt;

public final class AmberTileService extends TileService {
    private static final String TAG = "AmberBacklight";
    private static final String LIGHTS = "android.hardware.light.ILights/default";
    private static final String BRIGHTNESS = "brightness";
    private static final int MAX_BRIGHTNESS = 255;
    private static final int DISMISS_TIMEOUT_MS = 3000;
    private final Handler mMain = new Handler(Looper.getMainLooper());
    private HandlerThread mThread;
    private Handler mWorker;
    private ILightsExt mLights;
    private SharedPreferences mPreferences;
    private Dialog mDialog;
    private SeekBar mBrightnessSlider;
    private TextView mBrightnessValue;
    private View mOffButton;
    private boolean mDestroyed;
    private boolean mAvailable;
    private int mBrightness;
    private Runnable mPendingWrite;
    private final Runnable mDismiss = () -> mDialog.dismiss();

    @Override
    public void onCreate() {
        super.onCreate();
        mPreferences = getSharedPreferences("amber", MODE_PRIVATE);
        int brightness;
        if (mPreferences.contains(BRIGHTNESS)) {
            brightness = mPreferences.getInt(BRIGHTNESS, 0);
        } else {
            Display display =
                    getSystemService(DisplayManager.class).getDisplay(Display.DEFAULT_DISPLAY);
            // The extension uses 0-255 brightness instead of the display's 0-1 range.
            brightness = Math.round(display.getBrightnessDefault() * MAX_BRIGHTNESS);
        }
        mBrightness = Math.max(0, Math.min(MAX_BRIGHTNESS, brightness));
        mThread = new HandlerThread(TAG);
        mThread.start();
        mWorker = new Handler(mThread.getLooper());
        mDialog = createDialog();
    }

    private ILightsExt getLights() throws RemoteException {
        if (mLights != null && mLights.asBinder().isBinderAlive()) {
            return mLights;
        }
        IBinder binder = ServiceManager.checkService(LIGHTS);
        if (binder == null) {
            throw new IllegalStateException("Light HAL unavailable");
        }
        ILightsExt lights = ILightsExt.Stub.asInterface(binder.getExtension());
        if (lights != null) {
            for (int backlight : lights.getBacklights()) {
                if (backlight == Backlight.AMBER) {
                    mLights = lights;
                    return lights;
                }
            }
        }
        throw new IllegalStateException("Amber extension unavailable");
    }

    @Override
    public void onStartListening() {
        super.onStartListening();
        mWorker.post(this::checkAvailability);
    }

    private void checkAvailability() {
        try {
            getLights();
            mMain.post(() -> onAvailabilityChanged(true));
        } catch (RemoteException | RuntimeException e) {
            mLights = null;
            Log.w(TAG, "Cannot connect to amber backlight", e);
            mMain.post(() -> onAvailabilityChanged(false));
        }
    }

    private void onAvailabilityChanged(boolean available) {
        if (mDestroyed) {
            return;
        }
        mAvailable = available;
        updateUi();
    }

    private void updateUi() {
        int state;
        if (!mAvailable) {
            state = Tile.STATE_UNAVAILABLE;
        } else if (mBrightness == 0) {
            state = Tile.STATE_INACTIVE;
        } else {
            state = Tile.STATE_ACTIVE;
        }

        String status;
        if (mAvailable) {
            int percent = Math.round(mBrightness * 100f / MAX_BRIGHTNESS);
            status = getString(R.string.last_set, percent);
        } else {
            status = getString(R.string.unavailable);
        }

        Tile tile = getQsTile();
        if (tile != null) {
            tile.setState(state);
            tile.setSubtitle(status);
            tile.updateTile();
        }
        mBrightnessSlider.setEnabled(mAvailable);
        mOffButton.setEnabled(mAvailable);
        mBrightnessValue.setText(status);
    }

    @Override
    public void onClick() {
        super.onClick();
        if (mDialog.isShowing()) {
            scheduleDismiss();
            return;
        }
        mBrightnessSlider.setProgress(mBrightness);
        showDialog(mDialog);
        int width = Math.min(getResources().getDisplayMetrics().widthPixels - dp(32), dp(400));
        mDialog.getWindow().setLayout(width, WindowManager.LayoutParams.WRAP_CONTENT);
        mWorker.post(this::checkAvailability);
        scheduleDismiss();
    }

    private Dialog createDialog() {
        Dialog dialog =
                new Dialog(this) {
                    @Override
                    public boolean dispatchTouchEvent(MotionEvent event) {
                        if (event.getActionMasked() == MotionEvent.ACTION_DOWN) {
                            mMain.removeCallbacks(mDismiss);
                        }
                        boolean handled = super.dispatchTouchEvent(event);
                        if (event.getActionMasked() == MotionEvent.ACTION_UP
                                || event.getActionMasked() == MotionEvent.ACTION_CANCEL) {
                            scheduleDismiss();
                        }
                        return handled;
                    }
                };
        dialog.setContentView(R.layout.brightness_dialog);
        dialog.setTitle(R.string.amber_backlight);
        dialog.setCanceledOnTouchOutside(true);
        Window window = dialog.getWindow();
        window.setGravity(Gravity.TOP | Gravity.CENTER_HORIZONTAL);
        window.clearFlags(WindowManager.LayoutParams.FLAG_DIM_BEHIND);
        mBrightnessSlider = dialog.findViewById(R.id.brightness);
        mBrightnessValue = dialog.findViewById(R.id.brightness_value);
        mOffButton = dialog.findViewById(R.id.off);
        mBrightnessSlider.setOnSeekBarChangeListener(
                new SeekBar.OnSeekBarChangeListener() {
                    @Override
                    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                        if (fromUser) {
                            setBrightness(progress);
                        }
                    }

                    @Override
                    public void onStartTrackingTouch(SeekBar seekBar) {
                        mMain.removeCallbacks(mDismiss);
                    }

                    @Override
                    public void onStopTrackingTouch(SeekBar seekBar) {
                        scheduleDismiss();
                    }
                });
        mOffButton.setOnClickListener(
                view -> {
                    mBrightnessSlider.setProgress(0);
                    setBrightness(0);
                    scheduleDismiss();
                });
        dialog.setOnDismissListener(
                ignored -> {
                    mMain.removeCallbacks(mDismiss);
                    if (!mDestroyed) {
                        mDialog = createDialog();
                        updateUi();
                    }
                });
        return dialog;
    }

    private int dp(int value) {
        return Math.round(value * getResources().getDisplayMetrics().density);
    }

    private void scheduleDismiss() {
        mMain.removeCallbacks(mDismiss);
        AccessibilityManager accessibility = getSystemService(AccessibilityManager.class);
        int timeout =
                accessibility.getRecommendedTimeoutMillis(
                        DISMISS_TIMEOUT_MS,
                        AccessibilityManager.FLAG_CONTENT_CONTROLS
                                | AccessibilityManager.FLAG_CONTENT_TEXT);
        mMain.postDelayed(mDismiss, timeout);
    }

    private void setBrightness(int brightness) {
        if (mPendingWrite != null) {
            mWorker.removeCallbacks(mPendingWrite);
        }
        mPendingWrite = () -> writeBrightness(brightness);
        mWorker.post(mPendingWrite);
    }

    private void writeBrightness(int brightness) {
        try {
            getLights().setBacklightBrightness(Backlight.AMBER, brightness);
            mPreferences.edit().putInt(BRIGHTNESS, brightness).apply();
            mMain.post(() -> onBrightnessWritten(brightness));
        } catch (RemoteException | RuntimeException e) {
            mLights = null;
            Log.w(TAG, "Cannot set amber brightness", e);
            mMain.post(this::onWriteFailed);
        }
    }

    private void onBrightnessWritten(int brightness) {
        if (mDestroyed) {
            return;
        }
        mBrightness = brightness;
        mAvailable = true;
        updateUi();
    }

    private void onWriteFailed() {
        if (mDestroyed) {
            return;
        }
        mAvailable = false;
        updateUi();
        mDialog.dismiss();
        Toast.makeText(this, R.string.write_failed, Toast.LENGTH_SHORT).show();
    }

    @Override
    public void onDestroy() {
        mDestroyed = true;
        mDialog.dismiss();
        mMain.removeCallbacksAndMessages(null);
        mThread.quitSafely();
        super.onDestroy();
    }
}
