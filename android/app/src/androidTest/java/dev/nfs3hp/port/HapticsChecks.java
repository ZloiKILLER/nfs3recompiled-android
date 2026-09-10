package dev.nfs3hp.port;

import android.app.Instrumentation;
import android.content.SharedPreferences;
import java.lang.reflect.Field;
import java.util.Set;

final class HapticsChecks {
    static void run(Instrumentation test) throws Exception {
        SharedPreferences prefs=GamePreferences.get(test.getTargetContext());
        boolean present=prefs.contains(GamePreferences.TOUCH_VIBRATION);
        boolean saved=prefs.getBoolean(GamePreferences.TOUCH_VIBRATION,false);
        String[] strengthKeys={GamePreferences.TOUCH_VIBRATION_STRENGTH,GamePreferences.GAMEPAD_VIBRATION_STRENGTH};
        boolean[] strengthPresent={prefs.contains(strengthKeys[0]),prefs.contains(strengthKeys[1])};
        int[] strengthSaved={prefs.getInt(strengthKeys[0],100),prefs.getInt(strengthKeys[1],100)};
        Throwable[] failure={null};
        test.runOnMainSync(()->{
            GameHaptics h=new GameHaptics(test.getTargetContext());
            try {
                Field field=GameHaptics.class.getDeclaredField("active");field.setAccessible(true);
                Set<?> active=(Set<?>)field.get(h);
                prefs.edit().putInt(strengthKeys[0],50).putInt(strengthKeys[1],25).commit();
                // Same raw amplitude, different scales: the phone quarters it, the pad takes the plain percentage.
                if(h.scaledAmplitude(200,strengthKeys[0])!=25||h.scaledAmplitude(200,strengthKeys[1])!=50)
                    throw new AssertionError("phone and controller strengths must be independent");
                prefs.edit().putInt(strengthKeys[0],100).commit();
                if(h.scaledAmplitude(200,strengthKeys[0])!=50)throw new AssertionError("touch 100 percent is a quarter of the raw amplitude");
                prefs.edit().putInt(strengthKeys[0],0).commit();
                if(h.phonePulse(40,200)||!active.isEmpty())throw new AssertionError("zero intensity must stay silent");
                prefs.edit().putInt(strengthKeys[0],100).commit();
                prefs.edit().putBoolean(GamePreferences.TOUCH_VIBRATION,false).commit();
                h.resume();h.setLevel(.5f);
                if(!active.isEmpty())throw new AssertionError("disabled phone must stay silent");
                prefs.edit().putBoolean(GamePreferences.TOUCH_VIBRATION,true).commit();
                h.pause();h.setLevel(.5f);
                if(!active.isEmpty())throw new AssertionError("background must stay silent");
                h.resume();h.usePhone();
                if(!active.isEmpty())throw new AssertionError("selecting touch must not vibrate");
                boolean available=((android.os.Vibrator)test.getTargetContext().getSystemService(android.content.Context.VIBRATOR_SERVICE)).hasVibrator();
                if(available&&!h.phonePulse(40,64))throw new AssertionError("available phone accepts effect");
                /* The rate limit must not swallow a collision.  A drift inside
                 * the window waits for it; a sharp rise goes through early. */
                Field pulseField=GameHaptics.class.getDeclaredField("lastPulse");pulseField.setAccessible(true);
                h.stop();Thread.sleep(45);
                h.setLevel(.2f);long first=(long)pulseField.get(h);
                h.setLevel(.25f);
                if((long)pulseField.get(h)!=first)throw new AssertionError("a drift inside the window must wait");
                Thread.sleep(20);
                h.setLevel(.9f);
                if((long)pulseField.get(h)==first)throw new AssertionError("a sharp rise must pass the rate limit early");
                h.setLevel(0);
                if(!active.isEmpty())throw new AssertionError("zero effect cancels output");
                if(h.controllerPulse(null,40,64))throw new AssertionError("missing controller must not fall back to phone");
            }catch(Throwable e){failure[0]=e;}
            finally {
                h.pause();SharedPreferences.Editor edit=prefs.edit();
                if(present)edit.putBoolean(GamePreferences.TOUCH_VIBRATION,saved);else edit.remove(GamePreferences.TOUCH_VIBRATION);
                for(int i=0;i<strengthKeys.length;i++) {
                    if(strengthPresent[i])edit.putInt(strengthKeys[i],strengthSaved[i]);else edit.remove(strengthKeys[i]);
                }
                edit.commit();
            }
        });
        if(failure[0]!=null)throw new AssertionError(failure[0]);
    }
}
