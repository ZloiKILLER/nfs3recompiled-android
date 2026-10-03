package dev.nfs3hp.port;

import android.util.Log;

/** The app's log: android.util.Log in a debug build, silent in a release,
 *  which writes nothing to the log. */
final class AppLog
{
    private AppLog()
    {
    }

    static void v(String tag, String message)
    {
        if (BuildConfig.DEBUG)
            Log.v(tag, message);
    }

    static void d(String tag, String message)
    {
        if (BuildConfig.DEBUG)
            Log.d(tag, message);
    }

    static void i(String tag, String message)
    {
        if (BuildConfig.DEBUG)
            Log.i(tag, message);
    }

    static void w(String tag, String message)
    {
        if (BuildConfig.DEBUG)
            Log.w(tag, message);
    }

    static void w(String tag, String message, Throwable error)
    {
        if (BuildConfig.DEBUG)
            Log.w(tag, message, error);
    }

    static void e(String tag, String message)
    {
        if (BuildConfig.DEBUG)
            Log.e(tag, message);
    }

    static void e(String tag, String message, Throwable error)
    {
        if (BuildConfig.DEBUG)
            Log.e(tag, message, error);
    }
}
