package dev.nfs3hp.port;

import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.Map;

/** Import/export of the launcher's own configuration as a single JSON file.
 *
 *  What travels is everything that describes how the player set the launcher up
 *  -- the touch layout down to each control's position, the control mappings,
 *  vibration, orientation and the frame cap.  What does not travel is anything
 *  that describes *this device*: the active data set and the names of the data
 *  sets installed on it would point a different phone at things it does not
 *  have.  The interface language is left out on purpose too.
 *
 *  Control positions are stored as fractions of the play area rather than
 *  pixels, so a layout built on one screen lands correctly on another -- which
 *  is what makes carrying them worth doing at all. */
final class LauncherSettings {
    private LauncherSettings() {}

    /** Bumped only if the meaning of the contents changes, not their spelling. */
    private static final int FORMAT = 1;
    private static final String FORMAT_KEY = "format", SETTINGS_KEY = "settings";
    private static final String TYPE = "type", VALUE = "value";

    /** Settings carried one by one. */
    private static final String[] KEYS = {
        GamePreferences.ORIENTATION, GamePreferences.FPS_CAP, GamePreferences.GAMMA,
        GamePreferences.TOUCH_MODE, GamePreferences.TOUCH_LAYOUT,
        GamePreferences.TOUCH_OPACITY, GamePreferences.TOUCH_SIZE,
        GamePreferences.TOUCH_AUTO_HIDE, GamePreferences.TOUCH_EDGE,
        GamePreferences.TOUCH_RAISE, GamePreferences.TOUCH_SEPARATE,
        GamePreferences.TOUCH_HIDE_SECONDS, GamePreferences.TOUCH_HIDE_FULL,
        GamePreferences.TOUCH_VIBRATION, GamePreferences.GAMEPAD_VIBRATION,
        GamePreferences.TOUCH_VIBRATION_STRENGTH, GamePreferences.GAMEPAD_VIBRATION_STRENGTH,
        GamePreferences.SAVES_INCLUDE_SETTINGS,
    };

    /** Families whose every member travels: control geometry and mappings. */
    private static final String[] PREFIXES = {
        "touch_position_", "mapping_physical_", "mapping_touch_key_",
    };

    private static boolean carried(String key) {
        for (String known : KEYS) if (known.equals(key)) return true;
        for (String prefix : PREFIXES) if (key.startsWith(prefix)) return true;
        return false;
    }

    static int export(Context context, Uri uri) throws IOException {
        OutputStream raw = context.getContentResolver().openOutputStream(uri, "wt");
        if (raw == null) throw new IOException(context.getString(R.string.saves_open_failed));
        try (OutputStream out = raw) { return export(context, out); }
    }

    static int export(Context context, OutputStream out) throws IOException {
        JSONObject settings = new JSONObject();
        int count = 0;
        try {
            for (Map.Entry<String, ?> entry : GamePreferences.get(context).getAll().entrySet()) {
                if (!carried(entry.getKey())) continue;
                JSONObject typed = describe(entry.getValue());
                if (typed == null) continue;
                settings.put(entry.getKey(), typed);
                count++;
            }
            JSONObject root = new JSONObject();
            root.put(FORMAT_KEY, FORMAT);
            root.put(SETTINGS_KEY, settings);
            out.write(root.toString(2).getBytes(StandardCharsets.UTF_8));
        } catch (JSONException malformed) {
            throw new IOException(malformed);
        }
        if (count == 0) throw new IOException(context.getString(R.string.launcher_nothing_to_export));
        return count;
    }

    /** SharedPreferences keeps types, and JSON on its own cannot tell an int
     *  from a float, so each value carries its own. */
    private static JSONObject describe(Object value) throws JSONException {
        JSONObject typed = new JSONObject();
        if (value instanceof Boolean) typed.put(TYPE, "boolean").put(VALUE, value);
        else if (value instanceof Integer) typed.put(TYPE, "int").put(VALUE, value);
        else if (value instanceof Long) typed.put(TYPE, "long").put(VALUE, value);
        else if (value instanceof Float) typed.put(TYPE, "float").put(VALUE, ((Float) value).doubleValue());
        else if (value instanceof String) typed.put(TYPE, "string").put(VALUE, value);
        else return null;
        return typed;
    }

    static String importFrom(Context context, Uri uri, File dataRoot) throws IOException {
        InputStream raw = context.getContentResolver().openInputStream(uri);
        if (raw == null) throw new IOException(context.getString(R.string.saves_open_failed));
        try (InputStream in = raw) { return importFrom(context, in, dataRoot); }
    }

    static String importFrom(Context context, InputStream raw, File dataRoot) throws IOException {
        JSONObject settings;
        try {
            JSONObject root = new JSONObject(readAll(raw));
            if (root.optInt(FORMAT_KEY, -1) != FORMAT)
                throw new IOException(context.getString(R.string.launcher_wrong_format));
            settings = root.optJSONObject(SETTINGS_KEY);
        } catch (JSONException malformed) {
            throw new IOException(context.getString(R.string.launcher_wrong_format));
        }
        if (settings == null || settings.length() == 0)
            throw new IOException(context.getString(R.string.launcher_wrong_format));

        SharedPreferences preferences = GamePreferences.get(context);
        SharedPreferences.Editor edit = preferences.edit();
        /* Replace rather than merge: what the device carries afterwards is
         * exactly what the file describes, so everything carried is cleared
         * first.  Keys outside the whitelist -- the active data set, the
         * language -- are never touched either way. */
        for (String key : preferences.getAll().keySet())
            if (carried(key)) edit.remove(key);

        int applied = 0;
        for (java.util.Iterator<String> keys = settings.keys(); keys.hasNext(); ) {
            String key = keys.next();
            // An unknown or device-specific key in the file is skipped, not trusted.
            if (!carried(key)) continue;
            JSONObject typed = settings.optJSONObject(key);
            if (typed != null && apply(edit, key, typed)) applied++;
        }
        if (applied == 0) throw new IOException(context.getString(R.string.launcher_wrong_format));

        String backup = writeBackup(context, dataRoot);
        edit.apply();
        return backup;
    }

    /** Returns whether the entry was understood; a wrong type is dropped rather
     *  than stored, since it would otherwise surface as a ClassCastException far
     *  from here, the next time something read that setting. */
    private static boolean apply(SharedPreferences.Editor edit, String key, JSONObject typed) {
        String type = typed.optString(TYPE, "");
        switch (type) {
        case "boolean": edit.putBoolean(key, typed.optBoolean(VALUE)); return true;
        case "int":     edit.putInt(key, typed.optInt(VALUE)); return true;
        case "long":    edit.putLong(key, typed.optLong(VALUE)); return true;
        case "float":   edit.putFloat(key, clamp(key, (float) typed.optDouble(VALUE, 0))); return true;
        case "string":
            String value = typed.optString(VALUE, null);
            if (value == null) return false;
            edit.putString(key, value); return true;
        default: return false;
        }
    }

    /** Control geometry is bounded by construction -- positions are fractions of
     *  the play area and sizes are multipliers -- so a file claiming otherwise is
     *  corrected rather than allowed to put a control off screen. */
    private static float clamp(String key, float value) {
        if (Float.isNaN(value)) return 0;
        if (key.endsWith("_x") || key.endsWith("_y")) return Math.max(0f, Math.min(1f, value));
        if (key.endsWith("_size") || key.endsWith("_hit")) return Math.max(.1f, Math.min(10f, value));
        return value;
    }

    /** The launcher is also the tool used to undo a bad import, so the previous
     *  settings are written out before anything is replaced. */
    private static String writeBackup(Context context, File dataRoot) throws IOException {
        File folder = new File(dataRoot, ".launcher-backups");
        if (!folder.isDirectory() && !folder.mkdirs())
            throw new IOException("Could not create " + folder);
        String stamp = new SimpleDateFormat("yyyyMMdd-HHmmss", Locale.ROOT).format(new Date());
        File file = new File(folder, stamp + ".json");
        for (int suffix = 1; file.exists(); ++suffix) file = new File(folder, stamp + "-" + suffix + ".json");
        try (OutputStream out = new java.io.FileOutputStream(file)) { export(context, out); }
        catch (IOException empty) {
            /* Nothing configured yet, so there is nothing to lose -- and no
             * point leaving the half-written file behind either. */
            if (!file.delete()) file.deleteOnExit();
            return "";
        }
        return file.getName();
    }

    private static String readAll(InputStream in) throws IOException {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        byte[] chunk = new byte[16 * 1024];
        int n, total = 0;
        while ((n = in.read(chunk)) >= 0) {
            total += n;
            // A settings file is a few kilobytes; anything vastly larger is not one.
            if (total > 4 * 1024 * 1024) throw new IOException("Settings file is too large");
            buffer.write(chunk, 0, n);
        }
        return new String(buffer.toByteArray(), StandardCharsets.UTF_8);
    }
}
