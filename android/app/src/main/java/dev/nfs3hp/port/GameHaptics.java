package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.os.Build;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.util.Log;
import android.view.InputDevice;
import java.util.HashSet;
import java.util.Locale;

/** Routes finite effects to the selected input device, never to both outputs. */
final class GameHaptics {
    private static final String TAG = "GameHaptics";

    private final SharedPreferences preferences;
    private final Vibrator phone;
    private final HashSet<Vibrator> active = new HashSet<>();
    private int controllerId=-1;
    private long lastPulse;
    /* What the last pulse actually played, so a rise can be told from a drift. */
    private float lastLevel;
    private boolean enabled;

    /* Diagnostics for the intensity slider, debug builds only.  Two things decide
     * whether it can work at all and neither is visible from the outside: what
     * level the native mixer actually hands over, and whether the motor honours
     * amplitude -- without hasAmplitudeControl() every value vibrates at full. */
    private final boolean verbose;
    private long lastTrace;
    private int droppedSinceTrace;
    private int reportedControllerId = Integer.MIN_VALUE;

    void resume() { enabled=true; }
    void pause() { enabled=false;stop(); }
    void usePhone() { if(controllerId!=-1){stop();controllerId=-1;reportPhone();} }
    void useController(InputDevice device) {
        if(device!=null&&(device.supportsSource(InputDevice.SOURCE_GAMEPAD)||device.supportsSource(InputDevice.SOURCE_JOYSTICK))) {
            if(controllerId!=device.getId()){stop();controllerId=device.getId();reportController(device);}
        }
    }
    // Called only by the native Force Feedback mixer. Input events select an output, never create effects.
    void setLevel(float level) {
        if(!enabled)return;
        if(!(level>0)){stop();return;}
        long now=android.os.SystemClock.uptimeMillis();
        /* The rate limit keeps the vibrator API from being hammered, but a
         * collision landing inside the window used to be swallowed whole -- and
         * a sharp rise is exactly the event worth feeling.  Anything clearly
         * louder than the last pulse passes early, down to a floor that still
         * protects the API. */
        long since=now-lastPulse;
        if(since<15||(since<40&&level<lastLevel+.25f)){droppedSinceTrace++;return;}
        lastPulse=now;lastLevel=level;
        int amplitude=Math.max(1,Math.min(255,Math.round(level*255)));
        if(controllerId==-1) {
            trace("phone level=%.3f raw=%d percent=%d scaled=%d dropped=%d", level, amplitude,
                percent(GamePreferences.TOUCH_VIBRATION_STRENGTH),
                scaledAmplitude(amplitude, GamePreferences.TOUCH_VIBRATION_STRENGTH), droppedSinceTrace);
            phonePulse(100,amplitude);
        } else {
            trace("pad %d level=%.3f raw=%d percent=%d scaled=%d dropped=%d", controllerId, level, amplitude,
                percent(GamePreferences.GAMEPAD_VIBRATION_STRENGTH),
                scaledAmplitude(amplitude, GamePreferences.GAMEPAD_VIBRATION_STRENGTH), droppedSinceTrace);
            controllerPulse(InputDevice.getDevice(controllerId),100,amplitude);
        }
    }

    GameHaptics(Context context) {
        preferences = GamePreferences.get(context);
        phone = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
        verbose = (context.getApplicationInfo().flags & ApplicationInfo.FLAG_DEBUGGABLE) != 0;
        reportPhone();
    }

    private int percent(String setting) {
        return Math.max(0, Math.min(100, preferences.getInt(setting, 100)));
    }

    /** Rate-limited so the log itself cannot change the timing it is measuring. */
    private void trace(String format, Object... args) {
        if (!verbose) return;
        long now = android.os.SystemClock.uptimeMillis();
        if (now - lastTrace < 500) return;
        lastTrace = now;
        droppedSinceTrace = 0;
        Log.i(TAG, String.format(Locale.ROOT, format, args));
    }

    private void reportPhone() {
        if (!verbose) return;
        Log.i(TAG, "output -> phone, vibrator=" + (phone != null && phone.hasVibrator())
            + " amplitudeControl=" + (phone != null && phone.hasAmplitudeControl()));
    }

    private void reportController(InputDevice device) {
        if (!verbose || device == null || device.getId() == reportedControllerId) return;
        reportedControllerId = device.getId();
        try {
            Vibrator vibrator = Build.VERSION.SDK_INT >= 31
                ? device.getVibratorManager().getDefaultVibrator() : device.getVibrator();
            Log.i(TAG, "output -> pad " + device.getId() + " " + device.getName()
                + ", vibrator=" + (vibrator != null && vibrator.hasVibrator())
                + " amplitudeControl=" + (vibrator != null && vibrator.hasAmplitudeControl()));
        } catch (RuntimeException disconnected) {
            Log.i(TAG, "output -> pad " + device.getId() + ", vibrator lookup failed");
        }
    }

    boolean phonePulse(long duration, int amplitude) {
        return preferences.getBoolean(GamePreferences.TOUCH_VIBRATION, false)
            && pulse(phone, duration, scaledAmplitude(amplitude, GamePreferences.TOUCH_VIBRATION_STRENGTH));
    }

    int scaledAmplitude(int amplitude, String setting) {
        /* The phone motor is much stronger than the reference gamepad effect,
         * and the top of its range is not worth having: what used to be 50% on
         * the slider is now 100%, so the whole scale lands where the motor is
         * still expressive.  Two halvings, hence 400 rather than the gamepad's
         * plain percentage. */
        float divisor=GamePreferences.TOUCH_VIBRATION_STRENGTH.equals(setting)?400f:100f;
        return Math.round(amplitude * percent(setting) / divisor);
    }

    boolean controllerPulse(InputDevice device, long duration, int amplitude) {
        if (!preferences.getBoolean(GamePreferences.GAMEPAD_VIBRATION, false)
                || device == null || device.isVirtual()
                || !(device.supportsSource(InputDevice.SOURCE_GAMEPAD)
                    || device.supportsSource(InputDevice.SOURCE_JOYSTICK))) return false;
        try {
            Vibrator vibrator = Build.VERSION.SDK_INT >= 31
                ? device.getVibratorManager().getDefaultVibrator() : device.getVibrator();
            return pulse(vibrator, duration, scaledAmplitude(amplitude, GamePreferences.GAMEPAD_VIBRATION_STRENGTH));
        } catch (RuntimeException disconnected) { return false; }
    }

    private boolean pulse(Vibrator vibrator, long duration, int amplitude) {
        try {
            if (vibrator == null || !vibrator.hasVibrator()) return false;
            if(amplitude<=0) { vibrator.cancel();active.remove(vibrator);return false; }
            vibrator.vibrate(VibrationEffect.createOneShot(
                Math.max(1, Math.min(500, duration)), Math.max(1, Math.min(255, amplitude))));
            active.add(vibrator);
            return true;
        } catch (RuntimeException unavailable) {
            // A controller can disconnect between capability lookup and vibration.
            return false;
        }
    }

    void stop() {
        for (Vibrator vibrator : active) {
            try { vibrator.cancel(); } catch (RuntimeException disconnected) { }
        }
        active.clear();
        // Nothing is playing any more, so the next effect of any size is a rise.
        lastLevel = 0;
    }
}
