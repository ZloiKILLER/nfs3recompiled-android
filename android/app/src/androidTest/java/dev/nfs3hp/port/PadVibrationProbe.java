package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.SystemClock;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.util.Log;
import android.view.InputDevice;
import java.util.Arrays;
import java.util.Locale;

/** What a pad's rumble can express, felt by the person holding it: one pattern
 *  per run at six steps, so every run asks a single question.
 *    am instrument -w -e probe pad_vibration -e pattern NAME \
 *      dev.nfs3hp.port.android10.test/dev.nfs3hp.port.TouchOverlayInstrumentation
 *  Each step plays for 2 s followed by 1.5 s of silence, logged under PadProbe.
 *   game:   the port's real output path -- GameHaptics.setLevel on slot 0 every
 *           16 ms, as the native mixer calls it -- at 0.03 0.07 0.15 0.3 0.5 0.8
 *           (engine and road effects sit around 0.03-0.07, collisions 0.4-0.8);
 *   steady: one uninterrupted effect on both motors at the same strengths;
 *   weak:   the same on the motor listed last;
 *   strong: the same on the motor listed first;
 *   low:    both motors at the very bottom of the scale, amplitude 1 2 3 4 6 8;
 *   tick:   amplitude 1 for 3 6 10 15 25 40 ms of every 100 ms -- whether a
 *           motor that cannot run gently can still tap gently;
 *   road:   8 ms taps at amplitude 1, 5 7 9 11 14 17 times a second -- the
 *           game's road effect runs at 5-17 Hz, faster with speed;
 *   engine: 4 ms taps at amplitude 1, 13 16 19 22 25 29 times a second -- the
 *           engine effect runs at revs / 375 + 10 Hz;
 *   side:   first motor, second motor, first, second with 8 ms taps at 10 Hz,
 *           then the two motors at amplitude 128 -- which side each one is;
 *   pulse:  full strength switched on for 5 to 40 ms of every 50 ms.
 *  Android reports amplitude control for every input-device vibrator, whatever
 *  the driver behind it does with the value, so only a person can tell. */
final class PadVibrationProbe {
    private static final String TAG = "PadProbe";
    private static final long ON_MS = 2000;
    private static final long PAUSE_MS = 1500;
    private static final float[] LEVELS = {0.03f, 0.07f, 0.15f, 0.3f, 0.5f, 0.8f};
    private static final int[] LOW_AMPLITUDES = {1, 2, 3, 4, 6, 8};
    private static final int[] TICK_ON_MS = {3, 6, 10, 15, 25, 40};
    private static final int[] PULSE_ON_MS = {5, 10, 15, 20, 30, 40};
    private static final int[] ROAD_HZ = {5, 7, 9, 11, 14, 17};
    private static final int[] ENGINE_HZ = {13, 16, 19, 22, 25, 29};

    /* Set by TouchOverlayInstrumentation.onCreate from -e pattern. */
    static String pattern;
    static Context context;

    static String run() {
        InputDevice pad = null;
        for (int id : InputDevice.getDeviceIds()) {
            InputDevice device = InputDevice.getDevice(id);
            if (GamepadSlots.isGamepad(device)) { pad = device; break; }
        }
        if (pad == null) return "FAIL: no gamepad connected";
        Vibrator both = Build.VERSION.SDK_INT >= 31
            ? pad.getVibratorManager().getDefaultVibrator() : pad.getVibrator();
        if (both == null || !both.hasVibrator()) return "FAIL: " + pad.getName() + " has no vibrator";
        int[] ids = Build.VERSION.SDK_INT >= 31 ? pad.getVibratorManager().getVibratorIds() : new int[0];
        String name = pattern == null ? "" : pattern;
        Log.i(TAG, "pad " + pad.getName() + ", motors " + Arrays.toString(ids) + ", pattern " + name);

        Vibrator target;
        switch (name) {
            case "game": case "steady": case "low": case "tick": case "pulse": case "road": case "engine":
                target = both;
                break;
            case "weak": case "strong": case "side":
                if (ids.length < 2) return "FAIL: " + pad.getName() + " has no separate motors";
                target = pad.getVibratorManager().getVibrator(name.equals("strong") ? ids[0] : ids[ids.length - 1]);
                break;
            default:
                return "FAIL: -e pattern game, steady, weak, strong, low, tick, road, engine, side or pulse";
        }
        SystemClock.sleep(1500);
        switch (name) {
            case "game": game(); break;
            case "low": steady(target, LOW_AMPLITUDES); break;
            case "tick": repeating(target, TICK_ON_MS, 100, 1, "tick"); break;
            case "pulse": repeating(target, PULSE_ON_MS, 50, 255, "pulse"); break;
            case "road": rates(target, ROAD_HZ, 8, "road"); break;
            case "engine": rates(target, ENGINE_HZ, 4, "engine"); break;
            case "side": side(pad.getVibratorManager().getVibrator(ids[0]), ids[0],
                              pad.getVibratorManager().getVibrator(ids[1]), ids[1]); break;
            default: steady(target, levelAmplitudes()); break;
        }
        Log.i(TAG, "done");
        return "PROBE DONE: " + name + " on " + pad.getName() + ", motors " + Arrays.toString(ids);
    }

    private static int[] levelAmplitudes() {
        int[] amplitudes = new int[LEVELS.length];
        for (int i = 0; i < LEVELS.length; i++) amplitudes[i] = Math.max(1, Math.round(LEVELS[i] * 255));
        return amplitudes;
    }

    private static void mark(int step, String what) {
        Log.i(TAG, String.format(Locale.ROOT, "%d/6 %s", step, what));
    }

    private static void game() {
        SharedPreferences preferences = GamePreferences.get(context);
        boolean present = preferences.contains(GamePreferences.TOUCH_VIBRATION);
        boolean saved = preferences.getBoolean(GamePreferences.TOUCH_VIBRATION, false);
        boolean padPresent = preferences.contains(GamePreferences.GAMEPAD_VIBRATION);
        boolean padSaved = preferences.getBoolean(GamePreferences.GAMEPAD_VIBRATION, false);
        // The phone would join in on slot 0; this run is about the pad alone, switched on.
        preferences.edit().putBoolean(GamePreferences.TOUCH_VIBRATION, false)
            .putBoolean(GamePreferences.GAMEPAD_VIBRATION, true).commit();
        GameHaptics haptics = new GameHaptics(context);
        haptics.resume();
        try {
            for (int i = 0; i < LEVELS.length; i++) {
                mark(i + 1, String.format(Locale.ROOT, "game level %.2f", LEVELS[i]));
                long end = SystemClock.uptimeMillis() + ON_MS;
                while (SystemClock.uptimeMillis() < end) {
                    haptics.setLevel(0, LEVELS[i]);
                    SystemClock.sleep(16);
                }
                haptics.setLevel(0, 0f);
                SystemClock.sleep(PAUSE_MS);
            }
        } finally {
            haptics.pause();
            SharedPreferences.Editor edit = preferences.edit();
            if (present) edit.putBoolean(GamePreferences.TOUCH_VIBRATION, saved);
            else edit.remove(GamePreferences.TOUCH_VIBRATION);
            if (padPresent) edit.putBoolean(GamePreferences.GAMEPAD_VIBRATION, padSaved);
            else edit.remove(GamePreferences.GAMEPAD_VIBRATION);
            edit.commit();
        }
    }

    private static void steady(Vibrator vibrator, int[] amplitudes) {
        for (int i = 0; i < amplitudes.length; i++) {
            mark(i + 1, "steady amplitude " + amplitudes[i]);
            play(vibrator, VibrationEffect.createOneShot(ON_MS, amplitudes[i]));
        }
    }

    /* On for onMs[i] of every periodMs at one amplitude, repeating for ON_MS. */
    private static void repeating(Vibrator vibrator, int[] onMs, int periodMs, int amplitude, String label) {
        for (int i = 0; i < onMs.length; i++) {
            mark(i + 1, label + " amplitude " + amplitude + " on " + onMs[i] + " ms of " + periodMs);
            play(vibrator, taps(onMs[i], periodMs, amplitude));
        }
    }

    /* Taps of one length at amplitude 1, hz[i] times a second. */
    private static void rates(Vibrator vibrator, int[] hz, int onMs, String label) {
        for (int i = 0; i < hz.length; i++) {
            int period = Math.round(1000f / hz[i]);
            mark(i + 1, label + " " + hz[i] + " Hz, " + onMs + " ms taps every " + period + " ms");
            play(vibrator, taps(onMs, period, 1));
        }
    }

    private static void side(Vibrator first, int firstId, Vibrator second, int secondId) {
        mark(1, "taps on the first motor (id " + firstId + ")");
        play(first, taps(8, 100, 1));
        mark(2, "taps on the second motor (id " + secondId + ")");
        play(second, taps(8, 100, 1));
        mark(3, "taps on the first motor (id " + firstId + ")");
        play(first, taps(8, 100, 1));
        mark(4, "taps on the second motor (id " + secondId + ")");
        play(second, taps(8, 100, 1));
        mark(5, "amplitude 128 on the first motor (id " + firstId + ")");
        play(first, VibrationEffect.createOneShot(ON_MS, 128));
        mark(6, "amplitude 128 on the second motor (id " + secondId + ")");
        play(second, VibrationEffect.createOneShot(ON_MS, 128));
    }

    private static VibrationEffect taps(int onMs, int periodMs, int amplitude) {
        return VibrationEffect.createWaveform(
            new long[] {onMs, Math.max(1, periodMs - onMs)}, new int[] {amplitude, 0}, 0);
    }

    private static void play(Vibrator vibrator, VibrationEffect effect) {
        vibrator.vibrate(effect);
        SystemClock.sleep(ON_MS);
        vibrator.cancel();
        SystemClock.sleep(PAUSE_MS);
    }
}
