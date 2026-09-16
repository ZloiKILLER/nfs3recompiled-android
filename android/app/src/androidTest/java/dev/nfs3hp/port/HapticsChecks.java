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
        boolean padPresent=prefs.contains(GamePreferences.GAMEPAD_VIBRATION);
        boolean padSaved=prefs.getBoolean(GamePreferences.GAMEPAD_VIBRATION,false);
        Throwable[] failure={null};
        test.runOnMainSync(()->{
            GameHaptics h=new GameHaptics(test.getTargetContext());
            try {
                Field field=GameHaptics.class.getDeclaredField("active");field.setAccessible(true);
                Set<?> active=(Set<?>)field.get(h);
                /* The phone's own vibrator, for the checks that are about the
                 * phone alone: a pad attached to the test device plays slot 0's
                 * effects too once its vibration is switched on. */
                Field phoneField=GameHaptics.class.getDeclaredField("phone");phoneField.setAccessible(true);
                Object phone=phoneField.get(h);
                Field outputField=GameHaptics.class.getDeclaredField("phoneOutput");outputField.setAccessible(true);
                Object output=outputField.get(h);
                Field sendsField=output.getClass().getDeclaredField("sends");sendsField.setAccessible(true);
                prefs.edit().putBoolean(GamePreferences.TOUCH_VIBRATION,false).commit();
                h.resume();h.setLevel(0,.5f);h.setEffects(0,.1f,0,0,0,.1f,28f);h.setTurn(.9f);
                if(active.contains(phone))throw new AssertionError("disabled phone must stay silent");
                prefs.edit().putBoolean(GamePreferences.TOUCH_VIBRATION,true).commit();
                h.pause();h.setLevel(0,.5f);h.setTurn(.9f);
                if(!active.isEmpty())throw new AssertionError("background must stay silent");
                h.resume();
                boolean available=((android.os.Vibrator)test.getTargetContext().getSystemService(android.content.Context.VIBRATOR_SERVICE)).hasVibrator();
                if(available&&!h.phonePulse(40,64))throw new AssertionError("available phone accepts effect");
                h.stop();
                if(available) {
                    /* A jolt is sent at once and again about every 40 ms while it
                     * lasts -- each send restarts the motor, which is the texture --
                     * never more often than every 15 ms, and a clearly harder hit
                     * goes out early. */
                    h.setLevel(0,.4f);int first=sendsField.getInt(output);
                    if(!active.contains(phone))throw new AssertionError("a jolt plays on the phone");
                    h.setLevel(0,.4f);
                    if(sendsField.getInt(output)!=first)throw new AssertionError("a jolt repeated at once is not sent again");
                    android.os.SystemClock.sleep(20);
                    h.setLevel(0,.42f);
                    if(sendsField.getInt(output)!=first)throw new AssertionError("a small change waits for the 40 ms cadence");
                    h.setLevel(0,.9f);
                    if(sendsField.getInt(output)!=first+1)throw new AssertionError("a harder hit goes out early");
                    android.os.SystemClock.sleep(45);
                    h.setLevel(0,.9f);
                    if(sendsField.getInt(output)!=first+2)throw new AssertionError("a lasting jolt is sent again after 40 ms");
                }
                h.setLevel(0,0);
                if(!active.isEmpty())throw new AssertionError("zero effect cancels output");
                if(available) {
                    // The road is a pad's: on its own it leaves the phone alone.
                    h.stop();int before=sendsField.getInt(output);
                    h.setEffects(0,.3f,0,.8f,12f,0,0);h.setTurn(0);
                    if(sendsField.getInt(output)!=before)throw new AssertionError("the road never reaches the phone");
                    // The engine ticks only up its revs, not at idle.
                    h.setEffects(0,.03f,0,0,0,.086f,12f);h.setTurn(0);
                    if(sendsField.getInt(output)!=before)throw new AssertionError("an idling engine leaves the phone alone");
                    h.setEffects(0,.03f,0,0,0,.086f,28f);
                    if(sendsField.getInt(output)!=before+1)throw new AssertionError("high revs tick");
                    /* Cornering plays only while the engine runs -- in a race that
                     * is not paused -- and lets go on a gentle curve. */
                    h.stop();before=sendsField.getInt(output);
                    h.setTurn(.9f);
                    if(sendsField.getInt(output)!=before)throw new AssertionError("no cornering without a running engine");
                    h.setEffects(0,.03f,0,0,0,.086f,12f);h.setTurn(.9f);
                    if(sendsField.getInt(output)!=before+1||!active.contains(phone))throw new AssertionError("cornering plays on the phone");
                    h.setTurn(.1f);
                    if(active.contains(phone))throw new AssertionError("a gentle curve lets the phone go");
                    h.stop();
                }
                if(h.controllerPulse(null,40,64))throw new AssertionError("missing controller must not fall back to phone");
                /* The phone belongs to player one only.  Slot 1 exists for the
                 * second pad in split screen, and an effect created on it must
                 * never come out of the phone in the first player's hands. */
                h.stop();
                int phoneSends=sendsField.getInt(output);
                h.setLevel(1,.9f);
                if(active.contains(phone)||sendsField.getInt(output)!=phoneSends)throw new AssertionError("the second slot must not reach the phone");
                /* Each motor keeps its own state, so a busy second player cannot
                 * hold back the first. */
                Field padsField=GameHaptics.class.getDeclaredField("padOutputs");padsField.setAccessible(true);
                if(((Object[])padsField.get(h)).length!=GameHaptics.SLOTS)throw new AssertionError("one output per pad slot");
                h.setLevel(1,0);
                /* A pad vibrates only once its switch in Controls -> Gamepads is on:
                 * off, it is never so much as picked as a target. */
                prefs.edit().putBoolean(GamePreferences.GAMEPAD_VIBRATION,false).commit();
                h.setLevel(0,.9f);
                Object firstPad=((Object[])padsField.get(h))[0];
                Field targetField=firstPad.getClass().getDeclaredField("target");targetField.setAccessible(true);
                if(targetField.get(firstPad)!=null)throw new AssertionError("a pad stays silent until its vibration is switched on");
                h.setLevel(0,0);
                /* A pad's textures are taps on one repeating loop: nothing below
                 * the floor, every step a real length, and never more steps than
                 * Android plays for an input device -- the busiest road and
                 * engine together included. */
                if(GameHaptics.tapsFor(0,0,0,0)!=null)throw new AssertionError("no texture, no taps");
                if(GameHaptics.tapsFor(.01f,10,.01f,20)!=null)throw new AssertionError("textures below the floor stay silent");
                long[] roadTaps=GameHaptics.tapTimings(GameHaptics.tapsFor(.2f,10,0,0));
                if(roadTaps.length!=2||roadTaps[0]+roadTaps[1]!=50)throw new AssertionError("the road alone taps once a period, at twice the game's rate");
                long[] bothTaps=GameHaptics.tapTimings(GameHaptics.tapsFor(.2f,10,.2f,21));
                long[] busiest=GameHaptics.tapTimings(GameHaptics.tapsFor(1,100,1,100));
                for(long[] timings:new long[][]{roadTaps,bothTaps,busiest}) {
                    if(timings.length>GameHaptics.MAX_STEPS)throw new AssertionError("a loop Android would refuse: "+timings.length+" steps");
                    for(long step:timings)if(step<=0)throw new AssertionError("every step has a real length");
                }
                if(bothTaps.length<=roadTaps.length)throw new AssertionError("the engine adds taps of its own");
            }catch(Throwable e){failure[0]=e;}
            finally {
                h.pause();SharedPreferences.Editor edit=prefs.edit();
                if(present)edit.putBoolean(GamePreferences.TOUCH_VIBRATION,saved);else edit.remove(GamePreferences.TOUCH_VIBRATION);
                if(padPresent)edit.putBoolean(GamePreferences.GAMEPAD_VIBRATION,padSaved);else edit.remove(GamePreferences.GAMEPAD_VIBRATION);
                edit.commit();
            }
        });
        if(failure[0]!=null)throw new AssertionError(failure[0]);
    }
}
