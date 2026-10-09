package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;

import java.util.Locale;

/**
 * What each button of each pad does.
 *
 * A pad's sticks and triggers are the game's axes; its buttons are keys.  A
 * button is set to an action rather than to a key, and the key behind the
 * action depends on the player: the first pad sends player one's keys, the
 * second pad player two's -- the keys ControlProfile writes into the game's
 * settings.  That is what lets both pads share one list of actions and one
 * set of defaults.
 *
 * A pad set to Digital drives a race as a keyboard does instead: its D-pad
 * steers, accelerates and brakes and its triggers work the pedals, each one
 * sending its player's key for that (ControlProfile.drivingKeys), whatever the
 * D-pad is set to here.  Only in a race: the menus are the same either way.
 *
 * Resolved here into final values for the native side, which reads them as
 * NFS_GAMEPAD<n>_<BUTTON> (s_padButtons in sdl-backend/gamepad.cpp).
 */
final class GamepadButtons
{
    /* Order and names match the native table. */
    static final String[] BUTTON_IDS = {
        "south", "east", "west", "north", "left_shoulder", "right_shoulder",
        "left_stick", "right_stick", "back", "start",
        "dpad_up", "dpad_down", "dpad_left", "dpad_right",
    };

    /* What the D-pad does in a race on a pad set to Digital, parallel to
     * BUTTON_IDS: null for a button that keeps what it is set to. */
    private static final String[] DIGITAL_ACTIONS = {
        null, null, null, null, null, null,
        null, null, null, null,
        "accelerate", "brake", "steer_left", "steer_right",
    };

    /* The triggers, which the native table lists after the buttons.  They are
     * the pedals' axis and no button -- except on a pad set to Digital, where
     * in a race they press its player's brake and accelerate keys. */
    static final String[] TRIGGER_IDS = { "left_trigger", "right_trigger" };
    private static final String[] TRIGGER_ACTIONS = { "brake", "accelerate" };

    /* The four driving actions in the order ControlProfile keeps their keys. */
    private static final String[] DRIVING_ACTIONS = { "steer_right", "steer_left", "accelerate", "brake" };

    static final String NONE = "none";

    static final String[] ACTION_IDS = {
        NONE, "steer_left", "steer_right", "accelerate", "brake",
        "handbrake", "gear_up", "gear_down", "camera", "horn", "look_behind",
        "recover", "spike_strip", "headlights",
        "menu_up", "menu_down", "menu_left", "menu_right", "confirm", "pause",
    };

    /* Parallel to BUTTON_IDS, and for a race: the face buttons where a driving
     * hand expects them -- the handbrake under the thumb on the right, the
     * spikes beside it, look behind where a hand rests, the camera on top --
     * Start for the pause menu, and Share for the one thing a race asks for
     * when it has gone wrong, putting the car back on the road.  Every action
     * now has a button; none of them is the one the menus need, because a
     * button means something else there (MENU_KEYS). */
    static final String[] DEFAULT_ACTIONS = {
        "look_behind", "handbrake", "spike_strip", "camera", "gear_down", "gear_up",
        "horn", "headlights", "recover", "pause",
        "menu_up", "menu_down", "menu_left", "menu_right",
    };

    /* What the same buttons send outside a race, parallel to BUTTON_IDS: the
     * face buttons a console player reaches for without looking -- the lower
     * one confirms, the right one goes back -- Start confirms as well, Share
     * goes back, and the D-pad moves the highlight.  The rest are quiet: a
     * menu has nothing for a handbrake, and a replay being watched has nothing
     * for any of them.  Fixed rather than set per button, so the screen that
     * sets up a pad stays one screen about driving. */
    private static final String[] MENU_KEYS = {
        "Return", "Escape", "", "", "", "",
        "", "", "Escape", "Return",
        "Up", "Down", "Left", "Right",
    };

    /* What the buttons were set to before they knew about menus: pause on
     * Share and confirm on Start, which a race has no use for either way, and
     * the handbrake, the spikes and the reset one button to the left of where
     * they are now.  A pad still on exactly that set is moved to the new one;
     * a pad the player has arranged themselves is left alone. */
    private static final String[] SUPERSEDED_ACTIONS = {
        "handbrake", "spike_strip", "recover", "camera", "gear_down", "gear_up",
        "horn", "headlights", "pause", "confirm",
        "menu_up", "menu_down", "menu_left", "menu_right",
    };

    static void migrateDefaults(SharedPreferences preferences)
    {
        for (int slot = 0; slot < GamepadSlots.COUNT; ++slot)
        {
            boolean asShipped = true;
            for (int button = 0; button < BUTTON_IDS.length && asShipped; ++button)
            {
                String saved = preferences.getString(preferenceKey(slot, BUTTON_IDS[button]), null);
                asShipped = saved == null || saved.equals(SUPERSEDED_ACTIONS[button]);
            }
            if (asShipped)
                reset(preferences, slot);
        }
    }

    private GamepadButtons() {}

    static String preferenceKey(int slot, String buttonId)
    {
        return "gamepad" + (slot + 1) + "_button_" + buttonId;
    }

    static String action(SharedPreferences preferences, int slot, int button)
    {
        String action = preferences.getString(preferenceKey(slot, BUTTON_IDS[button]),
            DEFAULT_ACTIONS[button]);
        for (String known : ACTION_IDS)
            if (known.equals(action))
                return action;
        return DEFAULT_ACTIONS[button];
    }

    static void setAction(SharedPreferences preferences, int slot, int button, String action)
    {
        preferences.edit().putString(preferenceKey(slot, BUTTON_IDS[button]), action).apply();
    }

    static void reset(SharedPreferences preferences, int slot)
    {
        SharedPreferences.Editor edit = preferences.edit();
        for (String button : BUTTON_IDS)
            edit.remove(preferenceKey(slot, button));
        edit.apply();
    }

    /** Whether the pad in this slot drives a race on keys (Digital) rather than
     *  on its stick and triggers (Analog, the default). */
    static boolean digital(SharedPreferences preferences, int slot)
    {
        return preferences.getBoolean(GamePreferences.GAMEPAD_DIGITAL[slot], false);
    }

    static void setDigital(SharedPreferences preferences, int slot, boolean digital)
    {
        preferences.edit().putBoolean(GamePreferences.GAMEPAD_DIGITAL[slot], digital).apply();
    }

    /** What a button does in a race on a pad set to Digital, in place of what
     *  it is set to; null for a button that keeps its own. */
    static String digitalAction(int button)
    {
        return DIGITAL_ACTIONS[button];
    }

    /** What a button sends while a race is driven (NFS_GAMEPAD&lt;n&gt;_&lt;BUTTON&gt;):
     *  the action set for it, as its player's key -- or, on a pad set to
     *  Digital, the D-pad's driving.  Digital, steering and the pedals are
     *  keys wherever they are set, never the pad's axes.
     *
     *  The second pad never sends a key player one drives with.  Its D-pad's
     *  menu actions are the arrows, and the arrows steer player one whenever
     *  player one drives on keys -- on the touch controls, or with a first pad
     *  set to Digital: sent in split screen they would steer the other car. */
    static String raceValue(SharedPreferences preferences, int slot, int button)
    {
        String action = action(preferences, slot, button);
        if (digital(preferences, slot) && DIGITAL_ACTIONS[button] != null)
            action = DIGITAL_ACTIONS[button];
        return forPlayer(preferences, slot, drivingValue(preferences, slot, action));
    }

    /** What a trigger sends in a race: its pedal's key on a pad set to Digital,
     *  nothing otherwise -- the trigger then works the pedals' axis. */
    static String triggerRaceValue(SharedPreferences preferences, int slot, int trigger)
    {
        if (!digital(preferences, slot))
            return "";
        return forPlayer(preferences, slot, drivingValue(preferences, slot, TRIGGER_ACTIONS[trigger]));
    }

    /* Steering and the pedals as keys on a pad set to Digital, and every other
     * action as it always is. */
    private static String drivingValue(SharedPreferences preferences, int slot, String action)
    {
        if (digital(preferences, slot))
            for (int i = 0; i < DRIVING_ACTIONS.length; ++i)
                if (DRIVING_ACTIONS[i].equals(action))
                    return ControlProfile.drivingKeys(preferences, slot)[i].sdlName;
        return environmentValue(slot, action);
    }

    private static String forPlayer(SharedPreferences preferences, int slot, String value)
    {
        if (slot != 0)
            for (ControlProfile.Key key : ControlProfile.drivingKeys(preferences, 0))
                if (key.sdlName.equals(value))
                    return "";
        return value;
    }

    static String environmentName(int slot, String buttonId)
    {
        return "NFS_GAMEPAD" + (slot + 1) + "_" + buttonId.toUpperCase(Locale.ROOT);
    }

    /** The same button outside a race: the native side reads this one while a
     *  menu or a replay is on the screen (NFS_GAMEPAD&lt;n&gt;_&lt;BUTTON&gt;_UI). */
    static String menuEnvironmentName(int slot, String buttonId)
    {
        return environmentName(slot, buttonId) + "_UI";
    }

    static String menuEnvironmentValue(int button)
    {
        return MENU_KEYS[button];
    }

    /** An SDL key name; "axis:" and a steering or pedal action; or "" for a
     *  button that does nothing. */
    static String environmentValue(int slot, String action)
    {
        switch (action)
        {
        case "steer_left":
        case "steer_right":
        case "accelerate":
        case "brake":
            return "axis:" + action;
        case "handbrake":   return key(slot, ControlProfile.HANDBRAKE);
        case "gear_up":     return key(slot, ControlProfile.GEAR_UP);
        case "gear_down":   return key(slot, ControlProfile.GEAR_DOWN);
        case "camera":      return key(slot, ControlProfile.CAMERA);
        case "horn":        return key(slot, ControlProfile.HORN);
        case "look_behind": return key(slot, ControlProfile.LOOK_BEHIND);
        case "recover":     return key(slot, ControlProfile.RECOVER);
        case "spike_strip": return key(slot, ControlProfile.SPIKE_STRIP);
        case "headlights":  return key(slot, ControlProfile.HEADLIGHTS);
        case "menu_up":     return ControlProfile.UP.sdlName;
        case "menu_down":   return ControlProfile.DOWN.sdlName;
        case "menu_left":   return ControlProfile.LEFT.sdlName;
        case "menu_right":  return ControlProfile.RIGHT.sdlName;
        case "confirm":     return "Return";
        case "pause":       return "Escape";
        default:            return "";
        }
    }

    private static String key(int slot, int function)
    {
        return ControlProfile.playerKey(slot, function).sdlName;
    }

    /* Labels live in res/values/mapping_labels.xml, index-parallel to the id
     * arrays above; only the ids are ever stored. */
    static String[] buttonLabels(Context context)
    {
        return context.getResources().getStringArray(R.array.pad_button_labels);
    }

    static String[] actionLabels(Context context)
    {
        return context.getResources().getStringArray(R.array.pad_action_labels);
    }
}
