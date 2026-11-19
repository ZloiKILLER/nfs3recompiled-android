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
        /* Samples which of the game's functions run, to functions.bin in the
         * game's folder (func_log.cpp); tools/profile_report.py reads it. */
        new Option("live_counter", false,
            "Live functions log: writes which of the game's functions are running to functions.bin as it plays",
            "NFS_FUNC_LOG", "1"),
        // Every car at every physics step, to compare two builds by a replay (tools/replay_compare.py).
        new Option("replay_trace", false, "Every car at every physics step, to logs/trace-*.bin",
            "NFS_REPLAY_TRACE", "1"),
        new Option("replay_fast", false, "The game's time 8 times as fast, no frame cap (for checking replays)",
            "NFS_TIME_SCALE", "8", "NFS_FPS_CAP", "0"),
        new Option("net_trace", false, "EA's comm library, for network races [COMM]", "NFS_NET_TRACE", "1"),
        /* Every call the game makes through a pointer (its tables, callbacks,
         * imports) by name, File::read's byte counts, and SDL's "opening file
         * %s" (SDL_LogDebug, silent by default), so a read burst can be
         * matched back to its path. */
        new Option("trace_api", false, "Every call through a pointer and file opened (very heavy)",
            "NFS_TRACE_API", "1", "SDL_LOGGING", "app=verbose,error=verbose"),
    };

    static final Option[][] ALL = { TRACES };

    /* Switches of earlier builds, gone with what they switched: their preferences cleared with the
     * rest.  The car-specs native against the generated code and the natives' checks; the empty
     * Performance page and the cars' memory, both settled; the picture's and the renderer's own,
     * which only turned the port's proven changes off; the depth in 16-bit steps, the renderer's
     * own choice now. */
    static final String[] RETIRED = {
        "debug_natives", "debug_native_check", "debug_perf_trace", "debug_trace_cars",
        "debug_mirror_full", "debug_car_detail_full", "debug_textures32", "debug_widescreen",
        "debug_track_neighbours", "debug_texcoord_rebase", "debug_gl_thread", "debug_perf_hint",
        "debug_depth16",
    };

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

    /** Debug mode off: every switch back where the game has it. */
    static void reset(SharedPreferences preferences)
    {
        SharedPreferences.Editor edit = preferences.edit();
        for (Option[] group : ALL)
            for (Option option : group)
                edit.remove(option.preference());
        for (String retired : RETIRED)
            edit.remove(retired);
        edit.apply();
    }
}
