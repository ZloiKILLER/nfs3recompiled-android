package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.view.KeyEvent;

import java.util.Map;

final class GamePreferences
{
    static void setTouchLayout(SharedPreferences preferences,String layout) {
        String old=preferences.getString(TOUCH_LAYOUT,TOUCH_LAYOUT_STANDARD);
        if(old.equals(layout)) return;
        SharedPreferences.Editor edit=preferences.edit().putString(TOUCH_LAYOUT,layout);
        boolean mirrored=TOUCH_LAYOUT_MIRRORED.equals(layout);
        if(mirrored!=TOUCH_LAYOUT_MIRRORED.equals(old)) {
            /* A placed control goes to the other side: the edge it keeps its
             * distance from, or the fraction of an older layout, flips. */
            for(Map.Entry<String,?> entry:preferences.getAll().entrySet()) {
                if(!entry.getKey().startsWith("touch_position_")) continue;
                if(entry.getKey().endsWith("_x")&&entry.getValue() instanceof Float)
                    edit.putFloat(entry.getKey(),1f-(Float)entry.getValue());
                else if(entry.getKey().endsWith("_from_right")&&entry.getValue() instanceof Boolean)
                    edit.putBoolean(entry.getKey(),!(Boolean)entry.getValue());
            }
        }
        edit.apply();
    }

    static final String FILE_NAME = "launcher_settings";

    static final String ACTIVE_DATA_SET_ID = "active_data_set_id";
    /* Launcher language. "system" follows the device; anything else is a BCP-47
     * tag applied by LocaleHelper, written by the launcher's language picker. */
    static final String UI_LANGUAGE = "ui_language";
    static final String UI_LANGUAGE_SYSTEM = "system";
    static final String DATA_SET_NAMES = "data_set_names";
    static final String ORIENTATION = "screen_orientation";
    static final String FPS_CAP = "fps_cap";
    /* Display gamma as a percentage: 100 leaves the picture exactly as the game
     * drew it, higher lifts the dark end.  Stored as an int because that is
     * what a SeekBar deals in. */
    static final String GAMMA = "display_gamma";
    /* Brightness and contrast join gamma on the Screen adjustment screen and
     * are applied in the same final blit.  Percentages for the same reason
     * gamma is one -- a SeekBar deals in ints -- with 100 meaning "leave the
     * picture as the game drew it" for all three. */
    static final String BRIGHTNESS = "display_brightness";
    static final String CONTRAST = "display_contrast";
    /* Off, touching the screen does nothing in the game: no on-screen controls
     * and no touch mouse.  Gamepads and a real mouse are not touch and keep
     * working. */
    static final String TOUCH_ENABLED = "touch_enabled";
    static final String TOUCH_MODE = "touch_mode";
    static final String TOUCH_LAYOUT = "touch_layout";
    /* How a finger works the game's pointer in the menus: TAP puts the cursor
     * where it lands and clicks on a tap (TouchPointer), TOUCHPAD moves it by as
     * far as the finger slides (TouchpadPointer).  Read as the game starts. */
    static final String TOUCH_POINTER = "touch_pointer";
    static final String TOUCH_POINTER_TAP = "tap";
    static final String TOUCH_POINTER_TOUCHPAD = "touchpad";
    static final String TOUCH_OPACITY = "touch_opacity";
    static final String TOUCH_SIZE = "touch_size";
    static final String TOUCH_AUTO_HIDE = "touch_auto_hide";
    static final String TOUCH_EDGE = "touch_edge_spacing";
    /* Whether the save archive carries the game's own settings.  Remembered
     * rather than read off the checkbox: the data screen is rebuilt every time
     * the system file picker returns, which used to blank the box and make a
     * working export look like it had ignored the choice. */
    static final String SAVES_INCLUDE_SETTINGS = "saves_include_settings";
    /* The phone's vibration as 0.76 and before kept it, a switch of its own
     * under Controls -> Touch; read now only to carry that choice over to
     * FORCE_FEEDBACK, and still taken from a settings file exported then. */
    static final String TOUCH_VIBRATION = "touch_vibration";
    static final String TOUCH_HIDE_SECONDS = "touch_hide_seconds";
    /* Whether a control dragged in the layout editor lands on its grid
     * (TouchLayout.GRID).  On unless switched off. */
    static final String TOUCH_SNAP = "touch_editor_snap";
    /* The area, in pixels, the touch controls had in the game the last time it
     * ran, for the launcher's previews to lay them out in exactly as the game
     * does (TouchPreviewFrame).  Written by the game's overlay; this device's
     * alone, so never carried in a settings file. */
    static final String TOUCH_GAME_WIDTH = "touch_game_area_width";
    static final String TOUCH_GAME_HEIGHT = "touch_game_area_height";
    /* The pad's vibration switch as 0.76 and before kept it, under Controls ->
     * Gamepads; read now only to carry that choice over to FORCE_FEEDBACK. */
    static final String GAMEPAD_VIBRATION = "gamepad_force_feedback";
    /* Where the game's Force Feedback effects are felt: nowhere, on the phone,
     * or on the gamepad -- one of them, never both at once.  Chosen under
     * Controls -> Force Feedback; what each one plays, and how, is GameHaptics'
     * own, and how strong a pad plays stays the game's Force Feedback menu's to
     * say.  Read as the game starts. */
    static final String FORCE_FEEDBACK = "force_feedback_output";
    static final String FORCE_FEEDBACK_OFF = "off";
    static final String FORCE_FEEDBACK_PHONE = "phone";
    static final String FORCE_FEEDBACK_GAMEPAD = "gamepad";
    static final String[] FORCE_FEEDBACK_OUTPUTS = {
        FORCE_FEEDBACK_OFF, FORCE_FEEDBACK_PHONE, FORCE_FEEDBACK_GAMEPAD,
    };
    /* Per pad, whether it drives a race as a keyboard does -- the D-pad steers,
     * accelerates and brakes and the triggers work the pedals, all of them as
     * keys -- rather than on its stick and triggers as axes.  Off: Analog.
     * GamepadButtons resolves what that makes each button send, ControlProfile
     * what it makes the game's settings bind. */
    static final String[] GAMEPAD_DIGITAL = { "gamepad1_digital", "gamepad2_digital" };
    /* The TCP/IP port races are hosted and joined on: 9803 as the Modern Patch
     * has it, or 1030, the original game's.  Every player of a race needs the
     * same; the game reads it as it starts (NFS_NET_PORT). */
    static final String NETWORK_PORT = "network_port";
    static final int NETWORK_PORT_MODERN = 9803;
    static final int NETWORK_PORT_ORIGINAL = 1030;

    static final String ORIENTATION_AUTO = "auto";
    static final String ORIENTATION_LANDSCAPE = "landscape";
    /* The other way round.  Both fixed choices have to be spelled out: which
     * one is "right way up" depends on how the phone is held, and there is no
     * way to guess it. */
    static final String ORIENTATION_LANDSCAPE_REVERSE = "landscape_reverse";
    static final String TOUCH_MODE_BUTTONS = "buttons";
    static final String TOUCH_MODE_WHEEL = "wheel";
    static final String TOUCH_MODE_TILT = "tilt";
    static final String TOUCH_LAYOUT_STANDARD = "standard";
    static final String TOUCH_LAYOUT_MIRRORED = "mirrored";
    static final String TOUCH_LAYOUT_COMPACT = "compact";

    static final String[] ACTION_IDS = {
        "steer_left", "steer_right", "accelerate", "brake", "confirm",
        "handbrake", "camera", "look_behind", "horn", "spike_strip", "pause", "headlights", "recover", "gear_up", "gear_down",
    };

    static final int[] DEFAULT_KEYS = {
        KeyEvent.KEYCODE_DPAD_LEFT,
        KeyEvent.KEYCODE_DPAD_RIGHT,
        KeyEvent.KEYCODE_DPAD_UP,
        KeyEvent.KEYCODE_DPAD_DOWN,
        KeyEvent.KEYCODE_ENTER,
        KeyEvent.KEYCODE_SPACE,
        KeyEvent.KEYCODE_C,
        KeyEvent.KEYCODE_B,
        KeyEvent.KEYCODE_H,
        KeyEvent.KEYCODE_S,
        KeyEvent.KEYCODE_ESCAPE,
        KeyEvent.KEYCODE_L,
        KeyEvent.KEYCODE_R,
        // Stock EXE defaults at VA 0x490444 / 0x490448: DIK_A / DIK_Z.
        // Menu dispatch at 0x43c8c8 maps these slots to text entries 75 / 76 (Shift up / down).
        KeyEvent.KEYCODE_A,
        KeyEvent.KEYCODE_Z,
    };

    static final int[] KEY_VALUES = {
        KeyEvent.KEYCODE_UNKNOWN,
        KeyEvent.KEYCODE_DPAD_UP,
        KeyEvent.KEYCODE_DPAD_DOWN,
        KeyEvent.KEYCODE_DPAD_LEFT,
        KeyEvent.KEYCODE_DPAD_RIGHT,
        KeyEvent.KEYCODE_ENTER,
        KeyEvent.KEYCODE_ESCAPE,
        KeyEvent.KEYCODE_SPACE,
        KeyEvent.KEYCODE_C,
        KeyEvent.KEYCODE_B,
        KeyEvent.KEYCODE_H,
        KeyEvent.KEYCODE_S,
        KeyEvent.KEYCODE_L,
        KeyEvent.KEYCODE_R,
        KeyEvent.KEYCODE_A,
        KeyEvent.KEYCODE_Z,
    };

    /* Display labels live in res/values/mapping_labels.xml, index-parallel to the
     * id arrays above.  Only the ids are ever stored, so a translated label can
     * never change or invalidate a saved mapping. */
    static String[] actionLabels(Context context)
    {
        return context.getResources().getStringArray(R.array.action_labels);
    }

    static String[] keyLabels(Context context)
    {
        return context.getResources().getStringArray(R.array.key_labels);
    }

    private GamePreferences() {}

    static SharedPreferences get(Context context)
    {
        return context.getSharedPreferences(FILE_NAME, Context.MODE_PRIVATE);
    }

    /** Where the game's effects are felt (FORCE_FEEDBACK_OUTPUTS).  Until the
     *  player picks, it is what the two switches it replaced said: the pad if
     *  its switch was on -- the one of the two a player had to go and find --
     *  otherwise the phone if its switch was, otherwise nowhere. */
    static String forceFeedback(SharedPreferences preferences)
    {
        String output = preferences.getString(FORCE_FEEDBACK, null);
        for (String known : FORCE_FEEDBACK_OUTPUTS)
            if (known.equals(output))
                return known;
        if (preferences.getBoolean(GAMEPAD_VIBRATION, false))
            return FORCE_FEEDBACK_GAMEPAD;
        if (preferences.getBoolean(TOUCH_VIBRATION, false))
            return FORCE_FEEDBACK_PHONE;
        return FORCE_FEEDBACK_OFF;
    }

    /** The old switches go with the choice, so neither can speak again. */
    static void setForceFeedback(SharedPreferences preferences, String output)
    {
        preferences.edit().putString(FORCE_FEEDBACK, output)
            .remove(TOUCH_VIBRATION).remove(GAMEPAD_VIBRATION).apply();
    }

    static String touchKey(String actionId)
    {
        return "mapping_touch_key_" + actionId;
    }

    static int actionIndex(String actionId)
    {
        for (int i = 0; i < ACTION_IDS.length; ++i)
        {
            if (ACTION_IDS[i].equals(actionId))
                return i;
        }
        return -1;
    }

    static int getTouchKey(SharedPreferences preferences, String actionId)
    {
        int index = actionIndex(actionId);
        if (index < 0)
            return KeyEvent.KEYCODE_UNKNOWN;
        return preferences.getInt(touchKey(actionId), DEFAULT_KEYS[index]);
    }

    static String keyName(int keyCode)
    {
        switch (keyCode)
        {
        case KeyEvent.KEYCODE_DPAD_UP: return "up";
        case KeyEvent.KEYCODE_DPAD_DOWN: return "down";
        case KeyEvent.KEYCODE_DPAD_LEFT: return "left";
        case KeyEvent.KEYCODE_DPAD_RIGHT: return "right";
        case KeyEvent.KEYCODE_ENTER: return "return";
        case KeyEvent.KEYCODE_ESCAPE: return "escape";
        case KeyEvent.KEYCODE_SPACE: return "space";
        case KeyEvent.KEYCODE_C: return "c";
        case KeyEvent.KEYCODE_B: return "b";
        case KeyEvent.KEYCODE_H: return "h";
        case KeyEvent.KEYCODE_S: return "s";
        case KeyEvent.KEYCODE_L: return "l";
        case KeyEvent.KEYCODE_R: return "r";
        case KeyEvent.KEYCODE_A: return "a";
        case KeyEvent.KEYCODE_Z: return "z";
        default: return "unknown";
        }
    }

    static int indexOf(String[] values, String value)
    {
        for (int i = 0; i < values.length; ++i)
        {
            if (values[i].equals(value))
                return i;
        }
        return 0;
    }

    static int indexOf(int[] values, int value)
    {
        for (int i = 0; i < values.length; ++i)
        {
            if (values[i] == value)
                return i;
        }
        return 0;
    }
}
