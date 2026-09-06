package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.view.KeyEvent;

import java.util.LinkedHashMap;
import java.util.Map;

final class GamePreferences
{
    static final String FILE_NAME = "launcher_settings";

    static final String ACTIVE_DATA_SET_ID = "active_data_set_id";
    static final String DATA_SET_NAMES = "data_set_names";
    static final String ORIENTATION = "screen_orientation";
    static final String FPS_CAP = "fps_cap";
    static final String TOUCH_MODE = "touch_mode";
    static final String TOUCH_LAYOUT = "touch_layout";
    static final String TOUCH_OPACITY = "touch_opacity";
    static final String TOUCH_SIZE = "touch_size";
    static final String TOUCH_AUTO_HIDE = "touch_auto_hide";
    static final String TOUCH_EDGE = "touch_edge_spacing";
    static final String TOUCH_RAISE = "touch_raise_controls";
    static final String TOUCH_SEPARATE = "touch_separate_layouts";
    static final String TOUCH_GEARS = "touch_show_gears";
    static final String TOUCH_HIDE_SECONDS = "touch_hide_seconds";
    static final String TOUCH_HIDE_FULL = "touch_hide_full";

    static final String ORIENTATION_AUTO = "auto";
    static final String ORIENTATION_LANDSCAPE = "landscape";
    static final String TOUCH_MODE_BUTTONS = "buttons";
    static final String TOUCH_MODE_WHEEL = "wheel";
    static final String TOUCH_MODE_TILT = "tilt";
    static final String TOUCH_LAYOUT_STANDARD = "standard";
    static final String TOUCH_LAYOUT_MIRRORED = "mirrored";
    static final String TOUCH_LAYOUT_COMPACT = "compact";

    static final String[] ACTION_IDS = {
        "steer_left", "steer_right", "accelerate", "brake", "confirm",
        "handbrake", "camera", "look_behind", "horn", "pause", "headlights", "recover", "gear_up", "gear_down",
    };

    static final String[] ACTION_LABELS = {
        "Steer left", "Steer right", "Accelerate / menu up", "Brake / menu down", "Confirm / OK",
        "Handbrake", "Camera view", "Look behind", "Horn", "Escape / pause", "Headlights", "Return to track", "Shift up", "Shift down",
    };

    static final String[] DEFAULT_PHYSICAL_BUTTONS = {
        "dpad_left", "dpad_right", "dpad_up", "dpad_down", "south",
        "west", "east", "left_shoulder", "right_shoulder", "start", "none", "none", "none", "none",
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
    };

    static final String[] PHYSICAL_BUTTON_LABELS = {
        "Unassigned", "Back / View / Minus", "Start / Options / Plus",
        "South (A / Cross)", "East (B / Circle)", "West (X / Square)",
        "North (Y / Triangle)", "Left shoulder", "Right shoulder",
        "D-pad up", "D-pad down", "D-pad left", "D-pad right", "Left stick click", "Right stick click",
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
        KeyEvent.KEYCODE_L,
        KeyEvent.KEYCODE_R,
        KeyEvent.KEYCODE_A,
        KeyEvent.KEYCODE_Z,
    };

    static final String[] KEY_LABELS = {
        "Unassigned", "Up arrow", "Down arrow", "Left arrow", "Right arrow",
        "Return / Enter", "Escape", "Space", "C", "B", "H", "L", "R", "A", "Z",
    };

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
        for(String button:PHYSICAL_BUTTON_VALUES) if(!"none".equals(button)) mappings.put(button,"unknown");
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
