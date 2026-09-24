package dev.nfs3hp.port;

import android.app.ActivityManager;
import android.content.Context;

import java.util.List;

/**
 * Whether the game is up, asked from the launcher.
 *
 * <p>The game has a process of its own (":game" in the manifest), so the flag
 * NFS3Activity keeps is in the wrong process to read: the launcher asks Android
 * instead. Since Android 5 an app is told about its own processes and no
 * others, which is exactly the question here.
 *
 * <p>The launcher asks before it writes the game's settings file. The game
 * reads that file at its start, keeps it in memory and writes the whole of it
 * back as it leaves, so anything written underneath a running game is lost
 * when it goes.
 */
final class GameProcess
{
    static final String SUFFIX = ":game";

    private GameProcess() {}

    static boolean running(Context context)
    {
        ActivityManager activities = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        if (activities == null)
            return false;
        List<ActivityManager.RunningAppProcessInfo> processes = activities.getRunningAppProcesses();
        if (processes == null)
            return false;
        String name = context.getPackageName() + SUFFIX;
        for (ActivityManager.RunningAppProcessInfo process : processes)
            if (name.equals(process.processName))
                return true;
        return false;
    }
}
