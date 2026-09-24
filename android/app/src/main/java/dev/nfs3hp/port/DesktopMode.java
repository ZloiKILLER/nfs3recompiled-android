package dev.nfs3hp.port;

import android.content.Context;
import android.content.res.Configuration;

/**
 * Whether the app is running on a desktop rather than a phone's own screen:
 * Samsung DeX, where it is a window on a monitor, with a keyboard and a mouse.
 *
 * <p>Samsung says so through a field of its own on {@link Configuration} --
 * {@code semDesktopModeEnabled}, which equals the class's {@code
 * SEM_DESKTOP_MODE_ENABLED} while DeX is on. Neither is in the public SDK, so
 * they are read by reflection and simply absent on every other phone, which is
 * the answer that belongs there anyway.
 *
 * <p>The game asks because of what it does about it: a landscape lock is right
 * for a phone and wrong for a window, where it leaves the system handing out
 * one shape and letterboxing the rest of what the player drags the window to.
 */
final class DesktopMode
{
    private DesktopMode() {}

    static boolean active(Context context)
    {
        if (context == null)
            return false;
        Configuration configuration = context.getResources().getConfiguration();
        try
        {
            Class<?> type = configuration.getClass();
            return type.getField("SEM_DESKTOP_MODE_ENABLED").getInt(type)
                == type.getField("semDesktopModeEnabled").getInt(configuration);
        }
        catch (ReflectiveOperationException | IllegalArgumentException notADesktop)
        {
            return false;
        }
    }
}
