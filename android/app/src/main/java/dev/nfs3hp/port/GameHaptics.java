package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.os.Build;
import android.os.SystemClock;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.util.Log;
import android.view.InputDevice;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Locale;
import java.util.Objects;

/** Routes finite effects to the motors of the device slot they were created on:
 *  slot 0 is player one -- the phone and the first pad, both of which can play
 *  at once -- and slot 1 is the second pad.  The phone and a pad each have a
 *  switch in the launcher, and each plays in its own way: a pad what the game
 *  sends, as strong as the game's Force Feedback menu says; the phone only
 *  jolts, the engine's revs and cornering. */
final class GameHaptics {
    private static final String TAG = "GameHaptics";
    /** Matches win32::Gamepad::kSlotCount on the native side. */
    static final int SLOTS = 2;

    /* How long one effect is asked to run.  A new amplitude replaces it, and the
     * mixer sends zero the moment the game's effects stop, so this only bounds
     * how long a motor could keep going if updates stopped arriving. */
    static final long HOLD_MS = 3000;
    /* An effect still playing the right thing is sent again this long before
     * it would run out. */
    private static final long REFRESH_MS = 400;
    /* A drift -- a jolt fading out -- is applied at most this often, and only
     * once it has moved by a fifth of the amplitude playing (at least STEP).  A
     * rise to double or more is a collision or a landing and goes at once. */
    static final long SETTLE_MS = 150;
    static final int STEP = 4;

    /* A pad's weakest steady vibration already feels medium-strong -- measured on
     * a DualSense through Android's input vibrator, all the way down to amplitude
     * 1 -- so the game's steady textures, the road and the engine, play as taps:
     * the lowest amplitude for a few milliseconds, too short for the motor to
     * reach full speed, at the rate the effect itself runs.  Both motor ids of
     * that path drive the same motor, so the two textures share one loop, told
     * apart by length: the road taps longer, the engine finer.  A jolt still
     * plays steadily, as strong as it is, over whatever taps were playing. */
    private static final int TAP_AMPLITUDE = 1;
    /* Android refuses an input device any waveform of more steps than this. */
    static final int MAX_STEPS = 100;
    /* With both textures playing, a loop is about this long. */
    static final long LOOP_MS = 400;
    /* A loop is replaced at most this often while the revs or the speed sweep:
     * every replacement starts it over. */
    private static final long TAPS_SETTLE_MS = 250;
    /* Below these a texture or a jolt is left out rather than played. */
    static final float TEXTURE_FLOOR = 0.02f;
    static final float IMPACT_FLOOR = 0.02f;
    /* The game's road runs at 5-17 Hz, a rate made for a wheel; on a pad twice
     * that reads as a road passing underneath rather than a slow knock. */
    static final float ROAD_RATE = 2f;

    /* The phone, apart from any pad, plays three things and nothing else.
     * - Jolts -- collisions, landings, gear changes -- as hits that fade with
     *   the game's own envelope.
     * - Cornering, as a texture while the car leans into a turn (setTurn, read
     *   from the car itself), only while the engine runs: it runs only in a race
     *   that is not paused, so a car left leaning under the pause menu does not
     *   keep the phone going.
     * - The engine, as light ticks that follow the revs and are left out below
     *   them.
     * The road rumble and the engine's steady hum are left to a pad: sent to the
     * phone as one level they made a single buzz of the whole race, even at the
     * weakest amplitude.  Amplitudes are on Android's 0-255 scale. */
    static final int PHONE_HIT_MIN = 24, PHONE_HIT_MAX = 64;
    static final int PHONE_TURN_MIN = 10, PHONE_TURN_MAX = 36;
    static final int PHONE_TICK_MIN = 18, PHONE_TICK_MAX = 40;
    static final float PHONE_HIT_FLOOR = 0.02f, PHONE_TURN_FLOOR = 0.2f;
    /* The engine effect runs at 10 Hz plus one for every 375 rpm (sub_4719f0):
     * the ticks start at 16 Hz, 2 250 rpm, and are strongest from 30 Hz, 7 500.
     * Near the limiter the game also raises the effect's magnitude, to about 2.9
     * times what it is below 5 000 rpm, and that counts as full revs too. */
    static final float PHONE_ENGINE_FROM_HZ = 16f, PHONE_ENGINE_TO_HZ = 30f, PHONE_LIMITER_RISE = 1.92f;
    private static final long PHONE_PULSE_MS = 60, PHONE_REPEAT_MS = 40, PHONE_MIN_GAP_MS = 15, PHONE_TICK_MS = 10;

    /** The taps one pad plays: a period and a tap length for the road and for
     *  the engine, in milliseconds, zero where that texture is silent. */
    static final class Taps {
        final int roadPeriod, roadOn, enginePeriod, engineOn;
        Taps(int roadPeriod, int roadOn, int enginePeriod, int engineOn) {
            this.roadPeriod = roadPeriod; this.roadOn = roadOn;
            this.enginePeriod = enginePeriod; this.engineOn = engineOn;
        }
        @Override public boolean equals(Object other) {
            if (!(other instanceof Taps)) return false;
            Taps t = (Taps) other;
            return roadPeriod == t.roadPeriod && roadOn == t.roadOn
                && enginePeriod == t.enginePeriod && engineOn == t.engineOn;
        }
        @Override public int hashCode() { return Objects.hash(roadPeriod, roadOn, enginePeriod, engineOn); }
    }

    /** What one motor was last told.  Kept per motor -- the phone, and each
     *  slot's pad -- so two players' effects cannot hold each other back. */
    private static final class Output {
        Vibrator target;
        int amplitude;
        Taps taps;
        long started;
        long until;
        int sends;
    }

    private final SharedPreferences preferences;
    private final Vibrator phone;
    private final HashSet<Vibrator> active = new HashSet<>();
    private final Output phoneOutput = new Output();
    private final Output[] padOutputs = new Output[SLOTS];
    private boolean enabled;

    /* What the phone last heard of player one's effects and car. */
    private float phoneImpact, phoneEngine, phoneEngineHz, phoneEngineBase, phoneTurn;
    private long phoneTickDue;

    /* Diagnostics, debug builds only: what the native mixer hands over, what
     * the phone gets of it, and how many times a motor was actually sent
     * something -- whether the motor honours amplitude cannot be seen from the
     * outside, so the pad is reported once as well. */
    private final boolean verbose;
    private long lastTrace;
    private int sends;
    private int sendsAtTrace;
    private int reportedControllerId = Integer.MIN_VALUE;
    private float traceLevel, traceRoad, traceRoadHz;

    void resume() { enabled=true; }
    void pause() { enabled=false;stop(); }

    /** The pad in a device slot, or null while the slot is empty.  Resolved by
     *  the same rules NFS3Activity hands to the native side, so the pad that
     *  vibrates is always the pad the game is reading. */
    private InputDevice gamepadForSlot(int slot) {
        return GamepadSlots.device(GamepadSlots.resolve(preferences)[slot]);
    }

    /** One mixed level and nothing else: both a pad and the phone play it as a
     *  jolt. */
    void setLevel(int slot,float level) {
        setEffects(slot,level,level,0,0,0,0);
    }

    /** Called by the native Force Feedback mixer, which names the slot: the
     *  mixed level, and the jolts, road and engine it is made of
     *  (win32::Gamepad::RumbleDetail). */
    void setEffects(int slot,float level,float impact,float road,float roadHz,float engine,float engineHz) {
        if(!enabled||slot<0||slot>=SLOTS)return;
        final long now=SystemClock.uptimeMillis();
        InputDevice pad=gamepadForSlot(slot);
        if(pad!=null)reportController(pad);
        /* Both outputs, not one: the phone belongs to player one whether or not
         * a pad is also in their hands.  The phone answers to its own switch in
         * the launcher and picks out what suits it (drivePhone).  A pad answers
         * to a switch of its own, in Controls -> Gamepads and off until the
         * player turns it on, and plays what the game sends: how strong is the
         * game's Force Feedback menu's to decide.  Switched off, a pad that was
         * vibrating is let go at once. */
        if(slot==0) {
            phoneImpact=impact;
            phoneEngine=engine;
            phoneEngineHz=engine>0?engineHz:0;
            phoneEngineBase=engine>0?(phoneEngineBase>0?Math.min(phoneEngineBase,engine):engine):0;
            traceLevel=level;traceRoad=road;traceRoadHz=roadHz;
            drivePhone(now);
        }
        final boolean padOn=preferences.getBoolean(GamePreferences.GAMEPAD_VIBRATION,false);
        drivePad(padOutputs[slot],padOn&&pad!=null&&GamepadSlots.isGamepad(pad)?vibratorOf(pad):null,
                 impact,road,roadHz,engine,engineHz,now);
        if(level>0)trace(slot,level,impact,road,roadHz,engine,engineHz);
    }

    /** How hard player one's car corners, 0 to 1, about every 40 ms while a race
     *  is drawn (NFS3Activity.onPhoneTick): the phone's cornering, and the clock
     *  its engine ticks keep time by. */
    void setTurn(float turn) {
        if(!enabled)return;
        phoneTurn=turn>0?Math.min(1f,turn):0;
        drivePhone(SystemClock.uptimeMillis());
        if(phoneTurn>0||phoneEngineHz>0)trace(0,traceLevel,phoneImpact,traceRoad,traceRoadHz,phoneEngine,phoneEngineHz);
    }

    /* The phone's steady part is re-sent as a short effect about every 40 ms
     * while it lasts, the way 0.72 played everything: each send restarts the
     * motor, which is what makes a lasting level a texture instead of a flat
     * hum.  A clearly harder hit goes out early, but never sooner than 15 ms
     * after the last send.  With nothing steady to play, the engine ticks. */
    private void drivePhone(long now) {
        if (phone == null) return;
        if (!preferences.getBoolean(GamePreferences.TOUCH_VIBRATION, false)) {
            if (phoneOutput.amplitude > 0) cancel(phone);
            phoneOutput.amplitude = 0;
            return;
        }
        final int hold = phoneHold();
        if (hold > 0) {
            final long since = now - phoneOutput.started;
            if (phoneOutput.amplitude > 0
                && (since < PHONE_MIN_GAP_MS || (since < PHONE_REPEAT_MS && hold < phoneOutput.amplitude + 16))) return;
            if (pulse(phone, PHONE_PULSE_MS, hold)) {
                phoneOutput.amplitude = hold;
                phoneOutput.started = now;
                phoneOutput.until = now + PHONE_PULSE_MS;
                phoneOutput.sends++;
                sends++;
            }
            return;
        }
        if (phoneOutput.amplitude > 0) {
            cancel(phone);
            phoneOutput.amplitude = 0;
        }
        final float revs = engineRevs();
        if (revs <= 0 || now < phoneTickDue) return;
        if (pulse(phone, PHONE_TICK_MS, Math.round(PHONE_TICK_MIN + (PHONE_TICK_MAX - PHONE_TICK_MIN) * revs))) {
            phoneOutput.started = now;
            phoneOutput.until = now + PHONE_TICK_MS;
            phoneOutput.sends++;
            sends++;
            // A tick every third engine cycle: 4 to 12 a second.
            phoneTickDue = now + Math.round(1000f / Math.max(4f, Math.min(12f, phoneEngineHz / 3f)));
        }
    }

    /** The phone's steady part right now, 0 for none: a jolt, or cornering
     *  while the engine runs, whichever is stronger. */
    int phoneHold() {
        int hold = 0;
        if (phoneImpact > PHONE_HIT_FLOOR)
            hold = Math.round(PHONE_HIT_MIN + (PHONE_HIT_MAX - PHONE_HIT_MIN) * Math.min(1f, phoneImpact));
        if (phoneEngineHz > 0 && phoneTurn > PHONE_TURN_FLOOR)
            hold = Math.max(hold, Math.round(PHONE_TURN_MIN + (PHONE_TURN_MAX - PHONE_TURN_MIN)
                * Math.min(1f, (phoneTurn - PHONE_TURN_FLOOR) / (1f - PHONE_TURN_FLOOR))));
        return hold;
    }

    /** How far up its revs the engine is, 0 to 1: by the rate its effect runs
     *  at, or full once the game raises its magnitude at the limiter. */
    float engineRevs() {
        if (phoneEngineHz <= 0) return 0;
        final float rate = (phoneEngineHz - PHONE_ENGINE_FROM_HZ) / (PHONE_ENGINE_TO_HZ - PHONE_ENGINE_FROM_HZ);
        final float limiter = phoneEngineBase > 0 ? (phoneEngine / phoneEngineBase - 1f) / PHONE_LIMITER_RISE : 0;
        return Math.max(0f, Math.min(1f, Math.max(rate, limiter)));
    }

    /** A different motor for this output -- the slot's pad changed or went away:
     *  stop whatever the old one was playing. */
    private void retarget(Output output, Vibrator vibrator) {
        if (output.target == vibrator) return;
        if (output.target != null && (output.amplitude > 0 || output.taps != null)) cancel(output.target);
        output.target = vibrator;
        output.amplitude = 0;
        output.taps = null;
    }

    /** Plays an amplitude on one motor, sending it only when that changes
     *  something.  Every send stops the motor and starts it again, and on a
     *  DualSense the restart is a kick strong enough to drown out any light
     *  effect: a level sent again every 40 ms, which is how this used to work,
     *  turned the engine and the road -- and the game's sliders for them -- into
     *  one and the same buzz, while collisions, strong enough to rise above the
     *  kicks, were all that still felt right. */
    private void drive(Output output, Vibrator vibrator, int amplitude, long now) {
        retarget(output, vibrator);
        if (vibrator == null) return;
        if (amplitude <= 0) {
            if (output.amplitude > 0 || output.taps != null) cancel(vibrator);
            output.amplitude = 0;
            output.taps = null;
            return;
        }
        if (output.taps == null && output.amplitude > 0 && now < output.until - REFRESH_MS) {
            final int change = amplitude - output.amplitude;
            if (change == 0) return;
            final boolean rise = change >= Math.max(STEP, output.amplitude);
            if (!rise && (Math.abs(change) < Math.max(STEP, output.amplitude / 5)
                          || now - output.started < SETTLE_MS)) return;
        }
        if (pulse(vibrator, HOLD_MS, amplitude)) {
            output.amplitude = amplitude;
            output.taps = null;
            output.started = now;
            output.until = now + HOLD_MS;
            output.sends++;
            sends++;
        }
    }

    /** A pad: a jolt plays steadily, as strong as it is; otherwise the road and
     *  the engine play as taps; with neither, the motor stops. */
    private void drivePad(Output output, Vibrator vibrator, float impact, float road, float roadHz,
                          float engine, float engineHz, long now) {
        if (impact > IMPACT_FLOOR) {
            drive(output, vibrator, Math.max(1, Math.min(255, Math.round(impact * 255))), now);
            return;
        }
        final Taps taps = tapsFor(road, roadHz, engine, engineHz);
        if (taps == null) {
            drive(output, vibrator, 0, now);
            return;
        }
        retarget(output, vibrator);
        if (vibrator == null) return;
        // The loop repeats until it is replaced, so only a change is sent.
        if (output.taps != null && (output.taps.equals(taps) || now - output.started < TAPS_SETTLE_MS)) return;
        if (playTaps(vibrator, taps)) {
            output.taps = taps;
            output.amplitude = 0;
            output.started = now;
            output.until = Long.MAX_VALUE;
            output.sends++;
            sends++;
        }
    }

    /** The taps for a pad's textures, or null when both are below the floor.
     *  The road taps 5-12 ms, longer on rougher ground (its magnitude), the
     *  engine 3-6 ms, longer toward the rev limiter; the game's sliders for
     *  both scale those magnitudes, and with them the taps. */
    static Taps tapsFor(float road, float roadHz, float engine, float engineHz) {
        int roadPeriod = 0, roadOn = 0, enginePeriod = 0, engineOn = 0;
        if (road > TEXTURE_FLOOR && roadHz > 0) {
            final float rate = Math.max(4f, Math.min(40f, roadHz * ROAD_RATE));
            roadPeriod = Math.round(1000f / rate / 2f) * 2;
            roadOn = Math.max(5, Math.min(12, Math.round(4 + road * 32)));
        }
        if (engine > TEXTURE_FLOOR && engineHz > 0) {
            final float rate = Math.max(8f, Math.min(40f, engineHz));
            enginePeriod = Math.round(1000f / rate / 2f) * 2;
            engineOn = Math.max(3, Math.min(6, Math.round(2 + engine * 16)));
        }
        return roadPeriod == 0 && enginePeriod == 0 ? null : new Taps(roadPeriod, roadOn, enginePeriod, engineOn);
    }

    /** One loop of taps as waveform timings -- a tap, a silence, a tap and so on
     *  -- for the input reader to play on repeat.  A single texture is one tap a
     *  period.  Both together make a loop of about LOOP_MS: a whole number of
     *  road taps, and as many engine taps as fit it evenly, set half a road
     *  period in so the two interleave; taps that meet merge.  Either way the
     *  loop starts on a tap, and stays far below MAX_STEPS: at most forty taps a
     *  second each. */
    static long[] tapTimings(Taps taps) {
        final long loop = taps.roadPeriod > 0
            ? taps.roadPeriod * (taps.enginePeriod > 0 ? Math.max(1, Math.round((float) LOOP_MS / taps.roadPeriod)) : 1)
            : taps.enginePeriod;
        final ArrayList<long[]> on = new ArrayList<>();
        if (taps.roadPeriod > 0)
            for (long t = 0; t < loop; t += taps.roadPeriod) on.add(new long[] {t, t + taps.roadOn});
        if (taps.enginePeriod > 0) {
            final int count = Math.max(1, Math.round((float) loop / taps.enginePeriod));
            final long offset = taps.roadPeriod > 0 ? taps.roadPeriod / 2 : 0;
            for (int i = 0; i < count; i++) {
                final long t = Math.min(loop - taps.engineOn, (offset + Math.round((double) i * loop / count)) % loop);
                on.add(new long[] {t, t + taps.engineOn});
            }
        }
        on.sort((a, b) -> Long.compare(a[0], b[0]));
        final ArrayList<Long> timings = new ArrayList<>();
        long start = on.get(0)[0], end = on.get(0)[1];
        for (int i = 1; i < on.size(); i++) {
            final long[] tap = on.get(i);
            if (tap[0] <= end) { end = Math.max(end, tap[1]); continue; }
            timings.add(end - start);
            timings.add(tap[0] - end);
            start = tap[0];
            end = tap[1];
        }
        timings.add(end - start);
        if (loop > end) timings.add(loop - end);
        final long[] result = new long[timings.size()];
        for (int i = 0; i < result.length; i++) result[i] = timings.get(i);
        return result;
    }

    private boolean playTaps(Vibrator vibrator, Taps taps) {
        try {
            if (vibrator == null || !vibrator.hasVibrator()) return false;
            final long[] timings = tapTimings(taps);
            if (timings.length > MAX_STEPS) return false;
            final int[] amplitudes = new int[timings.length];
            for (int i = 0; i < amplitudes.length; i += 2) amplitudes[i] = TAP_AMPLITUDE;
            vibrator.vibrate(VibrationEffect.createWaveform(timings, amplitudes, 0));
            active.add(vibrator);
            return true;
        } catch (RuntimeException unavailable) {
            // A controller can disconnect between capability lookup and vibration.
            return false;
        }
    }

    GameHaptics(Context context) {
        preferences = GamePreferences.get(context);
        phone = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
        verbose = (context.getApplicationInfo().flags & ApplicationInfo.FLAG_DEBUGGABLE) != 0;
        for (int slot = 0; slot < SLOTS; slot++) padOutputs[slot] = new Output();
        reportPhone();
    }

    /** Rate-limited so the log itself cannot change the timing it is measuring. */
    private void trace(int slot, float level, float impact, float road, float roadHz, float engine, float engineHz) {
        if (!verbose) return;
        long now = SystemClock.uptimeMillis();
        if (now - lastTrace < 500) return;
        lastTrace = now;
        final Output pad = padOutputs[slot];
        final String playing = pad.taps != null
            ? String.format(Locale.ROOT, "taps road=%d/%d engine=%d/%d",
                pad.taps.roadOn, pad.taps.roadPeriod, pad.taps.engineOn, pad.taps.enginePeriod)
            : pad.amplitude > 0 ? "steady " + pad.amplitude : "silent";
        final String phonePlaying = slot != 0 ? "-"
            : phoneHold() > 0 ? "hold " + phoneHold()
            : engineRevs() > 0 ? String.format(Locale.ROOT, "ticks revs=%.2f", engineRevs()) : "silent";
        Log.i(TAG, String.format(Locale.ROOT,
            "slot %d level=%.3f impact=%.3f road=%.3f@%.2fHz engine=%.3f@%.2fHz turn=%.2f phone=%s pad=%s sends=%d",
            slot, level, impact, road, roadHz, engine, engineHz, slot == 0 ? phoneTurn : 0f, phonePlaying,
            pad.target == null ? "none" : playing, sends - sendsAtTrace));
        sendsAtTrace = sends;
    }

    private void reportPhone() {
        if (!verbose) return;
        Log.i(TAG, "output -> phone, vibrator=" + (phone != null && phone.hasVibrator())
            + " amplitudeControl=" + (phone != null && phone.hasAmplitudeControl()));
    }

    /* One Vibrator per pad, kept.  `active` tracks vibrators by identity, and
     * nothing promises that looking a pad's vibrator up twice yields the same
     * object -- a slot silenced through a second lookup would otherwise leave
     * the first one listed as still playing. */
    private final java.util.HashMap<Integer, Vibrator> padVibrators = new java.util.HashMap<>();

    /** A controller can disconnect between this lookup and the vibration, and
     *  the vibrator manager is the only route on API 31 and up. */
    private Vibrator vibratorOf(InputDevice device) {
        Vibrator known = padVibrators.get(device.getId());
        if (known != null) return known;
        try {
            Vibrator vibrator = Build.VERSION.SDK_INT >= 31
                ? device.getVibratorManager().getDefaultVibrator() : device.getVibrator();
            if (vibrator != null) padVibrators.put(device.getId(), vibrator);
            return vibrator;
        } catch (RuntimeException disconnected) { return null; }
    }

    private void cancel(Vibrator vibrator) {
        if (vibrator == null) return;
        try { vibrator.cancel(); } catch (RuntimeException disconnected) { }
        active.remove(vibrator);
    }

    private void reportController(InputDevice device) {
        if (!verbose || device == null || device.getId() == reportedControllerId) return;
        reportedControllerId = device.getId();
        Vibrator vibrator = vibratorOf(device);
        Log.i(TAG, "output -> pad " + device.getId() + " " + device.getName()
            + ", vibrator=" + (vibrator != null && vibrator.hasVibrator())
            + " amplitudeControl=" + (vibrator != null && vibrator.hasAmplitudeControl()));
    }

    /** One effect on the phone, while its switch is on. */
    boolean phonePulse(long duration, int amplitude) {
        return preferences.getBoolean(GamePreferences.TOUCH_VIBRATION, false)
            && pulse(phone, duration, amplitude);
    }

    /** A pad plays the game's level as it arrives.  How strong is the game's to
     *  decide -- Stick Volume and five kinds of effect in its Force Feedback
     *  menu. */
    boolean controllerPulse(InputDevice device, long duration, int amplitude) {
        if (!GamepadSlots.isGamepad(device)) return false;
        return pulse(vibratorOf(device), duration, amplitude);
    }

    private boolean pulse(Vibrator vibrator, long duration, int amplitude) {
        try {
            if (vibrator == null || !vibrator.hasVibrator()) return false;
            if(amplitude<=0) { vibrator.cancel();active.remove(vibrator);return false; }
            vibrator.vibrate(VibrationEffect.createOneShot(
                Math.max(1, Math.min(HOLD_MS, duration)), Math.max(1, Math.min(255, amplitude))));
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
        // Nothing is playing any more, so the next level of any size is sent.
        phoneOutput.amplitude = 0;
        phoneOutput.taps = null;
        phoneImpact = phoneEngine = phoneEngineHz = phoneEngineBase = phoneTurn = 0;
        phoneTickDue = 0;
        for (Output output : padOutputs) { output.amplitude = 0; output.taps = null; }
    }
}
