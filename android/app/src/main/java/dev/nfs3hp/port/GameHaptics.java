package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.view.InputDevice;
import java.util.HashSet;

/** Routes finite effects to the selected input device, never to both outputs. */
final class GameHaptics {
    private final SharedPreferences preferences;
    private final Vibrator phone;
    private final HashSet<Vibrator> active = new HashSet<>();
    private int controllerId=-1;
    private long lastPulse;
    private boolean enabled;

    void resume() { enabled=true; }
    void pause() { enabled=false;stop(); }
    void usePhone() { if(controllerId!=-1){stop();controllerId=-1;} }
    void useController(InputDevice device) {
        if(device!=null&&(device.supportsSource(InputDevice.SOURCE_GAMEPAD)||device.supportsSource(InputDevice.SOURCE_JOYSTICK))) {
            if(controllerId!=device.getId()){stop();controllerId=device.getId();}
        }
    }
    // Called only by the native Force Feedback mixer. Input events select an output, never create effects.
    void setLevel(float level) {
        if(!enabled)return;
        if(!(level>0)){stop();return;}
        long now=android.os.SystemClock.uptimeMillis();
        if(now-lastPulse<40)return;
        lastPulse=now;
        int amplitude=Math.max(1,Math.min(255,Math.round(level*255)));
        if(controllerId==-1)phonePulse(100,amplitude);
        else controllerPulse(InputDevice.getDevice(controllerId),100,amplitude);
    }

    GameHaptics(Context context) {
        preferences = GamePreferences.get(context);
        phone = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
    }

    boolean phonePulse(long duration, int amplitude) {
        return preferences.getBoolean(GamePreferences.TOUCH_VIBRATION, false)
            && pulse(phone, duration, scaledAmplitude(amplitude, GamePreferences.TOUCH_VIBRATION_STRENGTH));
    }

    int scaledAmplitude(int amplitude, String setting) {
        int percent=Math.max(0,Math.min(100,preferences.getInt(setting,100)));
        return Math.round(amplitude*percent/100f);
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
    }
}
