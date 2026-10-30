package dev.nfs3hp.port;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;

/** The Advanced screen: one build for players and for testing, and this is what
 *  tells them apart.  With debug mode off the app writes nothing to the log
 *  and the game starts as it always does -- a release in all but name.  With
 *  it on, the log is open and each switch below hands the game the
 *  environment its native side reads as it starts.
 *
 *  Every switch keeps its old launch-extra name, so an adb launch still works
 *  and wins over the screen for that one start:
 *    adb shell am start -n dev.nfs3hp.port/.SplashActivity --ez debug true --ez trace_ticks true */
final class DebugOptions
{
    private DebugOptions()
    {
    }

    static final String ENABLED = "debug_mode";
    /* The depth in the Voodoo's 16-bit steps: "" leaves it to the renderer,
     * which chooses by the depth buffer and the GPU; "ab" takes turns of five
     * seconds with and without, to weigh the GPU's load ([GPU]). */
    static final String DEPTH16 = "depth16";
    static final String[] DEPTH16_VALUES = { "", "off", "on", "ab" };

    /** One switch: its name (the launch extra, and the preference behind
     *  "debug_"), what the game does without it, and the environment it sets
     *  when it is the other way -- names and values in pairs. */
    static final class Option
    {
        final String name;
        final boolean byDefault;
        final String description;
        final String[] environment;

        Option(String name, boolean byDefault, String description, String... environment)
        {
            this.name = name;
            this.byDefault = byDefault;
            this.description = description;
            this.environment = environment;
        }

        String preference()
        {
            return "debug_" + name;
        }
    }

    /* Traces into the log.  The ones that speak only when something changes
     * -- the layout, the pads, the steering, text entry -- need no switch:
     * a handful of lines a session, and they are what makes a report readable. */
    static final Option[] TRACES = {
        new Option("trace_ticks", false, "The game's clock against real time [TICKS]", "NFS_TICK_TRACE", "1"),
        new Option("trace_input", false, "What player one's car is driven with [INPUT]", "NFS_INPUT_TRACE", "1"),
        new Option("trace_pointer", false, "The game's cursor and the touch pointer", "NFS_POINTER_TRACE", "1"),
        // Every car's memory once a second of a race, to cars.bin (with trace_ticks).
        new Option("trace_cars", false, "Every car's memory to cars.bin (with ticks)", "NFS_CAR_TRACE", "1"),
        /* Samples which of the game's functions run, to functions.bin in the
         * game's folder (func_log.cpp); tools/profile_report.py reads it. */
        new Option("live_counter", false,
            "Live functions log: writes which of the game's functions are running to functions.bin as it plays",
            "NFS_FUNC_LOG", "1"),
        new Option("net_trace", false, "EA's comm library, for network races [COMM]", "NFS_NET_TRACE", "1"),
        /* Every win32 call and File::read's byte counts, and SDL's "opening
         * file %s" (SDL_LogDebug, silent by default), so a read burst can be
         * matched back to its path. */
        new Option("trace_api", false, "Every win32 call and file opened (very heavy)",
            "NFS_TRACE_API", "1", "SDL_LOGGING", "app=verbose,error=verbose"),
    };

    /* The natives against the generated code, and the experiments. */
    static final Option[] CODE = {
        new Option("natives", true, "Car specs native (off: generated code)", "NFS_NATIVES", "0"),
        new Option("fe_native", true, "Native front end", "NFS_FE_NATIVE", "0"),
        new Option("native_ai_off", false, "Opponents on their generated code", "NFS_NATIVE_AI", "0"),
        // Listed before native_check, which sets the same variable and wins.
        new Option("load_check", false, "Check the loading natives [NATIVE]", "NFS_NATIVE_CHECK", "load"),
        new Option("native_check", false, "Check every checked native [NATIVE]", "NFS_NATIVE_CHECK", "1"),
        new Option("mad_check", false, "Check the movie decoder [MAD]", "NFS_MAD_CHECK", "1"),
        // Races are single precision, as the original's; this is the Modern Patch's way.
        new Option("fpu_extended_races", false, "Races at extended precision, as the Modern Patch", "NFS_FPU_EXTENDED", "1"),
        new Option("fpu_double", false, "No x87 single-precision rounding", "NFS_FPU_SINGLE", "0"),
        new Option("fpu_float", false, "The whole game in single precision",
            "NFS_FPU_FLOAT", "1", "NFS_FPU_EXTENDED", "0"),
    };

    static final Option[] PICTURE = {
        new Option("mirror_full", true, "Full rear-view mirror (off: the original's)", "NFS_MIRROR_FULL", "0"),
        new Option("car_detail_full", true, "Full car detail", "NFS_CAR_DETAIL_FULL", "0"),
        new Option("textures32", true, "Full-colour textures", "NFS_TEXTURES32", "0"),
        new Option("widescreen", true, "Widescreen modes in the driver", "NFS_WIDESCREEN", "0"),
        new Option("track_neighbours", true, "Extended track neighbours", "NFS_TRACK_NEIGHBOURS", "0"),
        new Option("texcoord_rebase", true, "Texture coordinates brought near the origin", "NFS_TEXCOORD_REBASE", "0"),
        new Option("gl_thread", true, "OpenGL on its own thread", "NFS_GL_THREAD", "0"),
        new Option("perf_hint", true, "Android performance hint", "NFS_PERF_HINT", "0"),
    };

    static final Option[][] ALL = { TRACES, CODE, PICTURE };

    /** Debug mode as the player left it, or as this launch asks. */
    static boolean enabled(SharedPreferences preferences, Intent launch)
    {
        if (launch != null && launch.hasExtra("debug"))
            return launch.getBooleanExtra("debug", false);
        return preferences.getBoolean(ENABLED, false);
    }

    static boolean enabled(Context context)
    {
        return GamePreferences.get(context).getBoolean(ENABLED, false);
    }

    static boolean value(SharedPreferences preferences, Intent launch, Option option)
    {
        if (launch != null && launch.hasExtra(option.name))
            return launch.getBooleanExtra(option.name, option.byDefault);
        return preferences.getBoolean(option.preference(), option.byDefault);
    }

    static String depth16(SharedPreferences preferences, Intent launch)
    {
        String value = launch != null ? launch.getStringExtra(DEPTH16) : null;
        return value != null ? value : preferences.getString("debug_" + DEPTH16, "");
    }

    /** Debug mode off: every switch back where the game has it. */
    static void reset(SharedPreferences preferences)
    {
        SharedPreferences.Editor edit = preferences.edit();
        for (Option[] group : ALL)
            for (Option option : group)
                edit.remove(option.preference());
        edit.remove("debug_" + DEPTH16).apply();
    }
}
