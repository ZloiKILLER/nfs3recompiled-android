package org.libsdl.app;

import android.util.Log;

/** SDL's Java log, changed for the NFS III port: android.util.Log in a debug
 *  build, silent in a release, which writes nothing to the log. */
final class SDLLog
{
    private static final boolean ON = dev.nfs3hp.port.BuildConfig.DEBUG;

    private SDLLog()
    {
    }

    static int v(String tag, String message) { return ON ? Log.v(tag, message) : 0; }
    static int v(String tag, String message, Throwable error) { return ON ? Log.v(tag, message, error) : 0; }
    static int d(String tag, String message) { return ON ? Log.d(tag, message) : 0; }
    static int d(String tag, String message, Throwable error) { return ON ? Log.d(tag, message, error) : 0; }
    static int i(String tag, String message) { return ON ? Log.i(tag, message) : 0; }
    static int i(String tag, String message, Throwable error) { return ON ? Log.i(tag, message, error) : 0; }
    static int w(String tag, String message) { return ON ? Log.w(tag, message) : 0; }
    static int w(String tag, String message, Throwable error) { return ON ? Log.w(tag, message, error) : 0; }
    static int e(String tag, String message) { return ON ? Log.e(tag, message) : 0; }
    static int e(String tag, String message, Throwable error) { return ON ? Log.e(tag, message, error) : 0; }
}
