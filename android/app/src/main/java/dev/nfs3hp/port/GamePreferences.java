package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.view.KeyEvent;

import java.util.LinkedHashMap;
import java.util.Map;

final class GamePreferences
{
    static void setTouchLayout(SharedPreferences preferences,String layout) {
        String old=preferences.getString(TOUCH_LAYOUT,TOUCH_LAYOUT_STANDARD);
        if(old.equals(layout)) return;
        SharedPreferences.Editor edit=preferences.edit().putString(TOUCH_LAYOUT,layout);
        boolean mirrored=TOUCH_LAYOUT_MIRRORED.equals(layout);
        if(mirrored!=TOUCH_LAYOUT_MIRRORED.equals(old)) {
            for(Map.Entry<String,?> entry:preferences.getAll().entrySet())
                if(entry.getKey().startsWith("touch_position_")&&entry.getKey().endsWith("_x")&&entry.getValue() instanceof Float)
                    edit.putFloat(entry.getKey(),1f-(Float)entry.getValue());
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
    static final String TOUCH_MODE = "touch_mode";
    static final String TOUCH_LAYOUT = "touch_layout";
    static final String TOUCH_OPACITY = "touch_opacity";
    static final String TOUCH_SIZE = "touch_size";
    static final String TOUCH_AUTO_HIDE = "touch_auto_hide";
    static final String TOUCH_EDGE = "touch_edge_spacing";
    static final String TOUCH_RAISE = "touch_raise_controls";
    static final String TOUCH_SEPARATE = "touch_separate_layouts";
    /* Whether the save archive carries the game's own settings.  Remembered
     * rather than read off the checkbox: the data screen is rebuilt every time
     * the system file picker returns, which used to blank the box and make a
     * working export look like it had ignored the choice. */
    static final String SAVES_INCLUDE_SETTINGS = "saves_include_settings";
    static final String TOUCH_VIBRATION = "touch_vibration";
    static final String GAMEPAD_VIBRATION = "gamepad_vibration";
    static final String TOUCH_VIBRATION_STRENGTH = "touch_vibration_strength";
    static final String GAMEPAD_VIBRATION_STRENGTH = "gamepad_vibration_strength";
    static final String TOUCH_HIDE_SECONDS = "touch_hide_seconds";
    static final String TOUCH_HIDE_FULL = "touch_hide_full";

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

    static final String[] DEFAULT_PHYSICAL_BUTTONS = {
        "left_stick_left", "left_stick_right", "right_trigger", "left_trigger", "south",
        "west", "east", "left_shoulder", "right_shoulder", "none", "start", "none", "none", "none", "none",
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

    static final String[] PHYSICAL_BUTTON_VALUES = {
        "none", "back", "start", "south", "east", "west", "north",
        "left_shoulder", "right_shoulder", "dpad_up", "dpad_down",
        "dpad_left", "dpad_right", "left_stick", "right_stick",
        "left_trigger", "right_trigger", "left_stick_up", "left_stick_down",
        "left_stick_left", "left_stick_right", "right_stick_up", "right_stick_down",
        "right_stick_left", "right_stick_right",
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

    static String[] physicalButtonLabels(Context context)
    {
        return context.getResources().getStringArray(R.array.physical_button_labels);
    }

    private GamePreferences() {}

    static SharedPreferences get(Context context)
    {
        return context.getSharedPreferences(FILE_NAME, Context.MODE_PRIVATE);
    }

    static String physicalKey(String actionId)
    {
        return "mapping_physical_" + actionId;
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

    static String getPhysicalButton(SharedPreferences preferences, int actionIndex)
    {
        return preferences.getString(physicalKey(ACTION_IDS[actionIndex]),
            DEFAULT_PHYSICAL_BUTTONS[actionIndex]);
    }

    /** Format consumed by the native gamepad translation layer: button=SDL-key pairs. */
    static String gamepadMappingEnvironment(SharedPreferences preferences)
    {
        Map<String, String> mappings = new LinkedHashMap<>();
        // Explicit unassignment must override native defaults too.
        for(String button:PHYSICAL_BUTTON_VALUES)
            if(!"none".equals(button)&&!button.startsWith("dpad_"))mappings.put(button,"unknown");
        for (int i = 0; i < ACTION_IDS.length; ++i)
        {
            String button = getPhysicalButton(preferences, i);
            int keyCode = getTouchKey(preferences, ACTION_IDS[i]);
            if (!"none".equals(button) && keyCode != KeyEvent.KEYCODE_UNKNOWN)
                mappings.put(button, keyName(keyCode));
        }

        StringBuilder result = new StringBuilder();
        for (Map.Entry<String, String> entry : mappings.entrySet())
        {
            if (result.length() != 0)
                result.append(',');
            result.append(entry.getKey()).append('=').append(entry.getValue());
        }
        return result.toString();
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
