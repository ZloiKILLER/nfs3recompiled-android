package dev.nfs3hp.port;

import android.content.SharedPreferences;
import android.view.InputDevice;

import java.util.ArrayList;
import java.util.List;

/**
 * Which physical pad is Gamepad 1 and which is Gamepad 2.
 *
 * The game always sees two DirectInput devices, and what a pad's buttons do is
 * set per slot (GamepadButtons). The one thing neither can know is which of the
 * pads Android has attached belongs to which player, and that is all this
 * decides.
 *
 * Pads are remembered by their Android input-device descriptor: the device id
 * changes every time a Bluetooth pad reconnects and the name is the same for
 * two pads of one model, but the descriptor survives reconnects and reboots and
 * tells identical pads apart. The native side recognises the same descriptor
 * through the fingerprint SDL keeps of it in the joystick GUID.
 *
 * Assignments describe hardware on this particular phone, so they are left out
 * of the launcher-settings export on purpose, like the data-set keys.
 */
final class GamepadSlots
{
    static final int COUNT = 2;

    private static final String[] DESCRIPTOR = { "gamepad_slot_1", "gamepad_slot_2" };
    /* Kept beside the descriptor only to name a pad that is not connected
     * right now; the descriptor itself means nothing to a player. */
    private static final String[] NAME = { "gamepad_slot_1_name", "gamepad_slot_2_name" };

    private GamepadSlots() {}

    /** A pad as the resolver needs it -- nothing Android-specific, so the
     *  rules below can be checked without real hardware. */
    static final class Pad
    {
        final int id;
        final String descriptor;
        final String name;

        Pad(int id, String descriptor, String name)
        {
            this.id = id;
            this.descriptor = descriptor;
            this.name = name;
        }
    }

    /** "" while the slot is automatic. */
    static String assigned(SharedPreferences preferences, int slot)
    {
        return preferences.getString(DESCRIPTOR[slot], "");
    }

    static String assignedName(SharedPreferences preferences, int slot)
    {
        return preferences.getString(NAME[slot], "");
    }

    /** Pins a pad to a slot. A pad belongs to one player at a time, so if the
     *  other slot had it, that slot goes back to automatic. */
    static void assign(SharedPreferences preferences, int slot, String descriptor, String name)
    {
        SharedPreferences.Editor edit = preferences.edit();
        for (int other = 0; other < COUNT; ++other)
        {
            if (other != slot && descriptor.equals(assigned(preferences, other)))
                edit.remove(DESCRIPTOR[other]).remove(NAME[other]);
        }
        edit.putString(DESCRIPTOR[slot], descriptor).putString(NAME[slot], name).apply();
    }

    static void useAutomatic(SharedPreferences preferences, int slot)
    {
        preferences.edit().remove(DESCRIPTOR[slot]).remove(NAME[slot]).apply();
    }

    static boolean isGamepad(InputDevice device)
    {
        if (device == null || device.isVirtual())
            return false;
        String descriptor = device.getDescriptor();
        return descriptor != null && !descriptor.isEmpty()
            && (device.supportsSource(InputDevice.SOURCE_GAMEPAD)
                || device.supportsSource(InputDevice.SOURCE_JOYSTICK));
    }

    /** Physical gamepads attached right now, in Android device-id order --
     *  which is the order Android handed them out, oldest connection first. */
    static List<Pad> attached()
    {
        int[] ids = InputDevice.getDeviceIds();
        java.util.Arrays.sort(ids);
        List<Pad> pads = new ArrayList<>();
        for (int id : ids)
        {
            InputDevice device = InputDevice.getDevice(id);
            if (isGamepad(device))
                pads.add(new Pad(id, device.getDescriptor(), device.getName()));
        }
        return pads;
    }

    /**
     * The pad each slot gets. Pinned slots go first and take only their own
     * pad: a pinned slot whose pad is missing stays empty rather than grabbing
     * another one, so if the second player's pad drops out mid-race the first
     * player's pad is not handed over to them. Automatic slots then share what
     * is left, in connection order.
     */
    static Pad[] resolve(String[] assigned, List<Pad> attached)
    {
        Pad[] slots = new Pad[COUNT];
        boolean[] taken = new boolean[attached.size()];
        for (int slot = 0; slot < COUNT; ++slot)
        {
            if (assigned[slot].isEmpty())
                continue;
            for (int i = 0; i < attached.size(); ++i)
            {
                if (!taken[i] && attached.get(i).descriptor.equals(assigned[slot]))
                {
                    slots[slot] = attached.get(i);
                    taken[i] = true;
                    break;
                }
            }
        }
        int next = 0;
        for (int slot = 0; slot < COUNT; ++slot)
        {
            if (!assigned[slot].isEmpty())
                continue;
            while (next < attached.size() && taken[next])
                ++next;
            if (next < attached.size())
            {
                slots[slot] = attached.get(next);
                taken[next] = true;
                ++next;
            }
        }
        return slots;
    }

    static Pad[] resolve(SharedPreferences preferences)
    {
        String[] assigned = new String[COUNT];
        for (int slot = 0; slot < COUNT; ++slot)
            assigned[slot] = assigned(preferences, slot);
        return resolve(assigned, attached());
    }

    static InputDevice device(Pad pad)
    {
        return pad == null ? null : InputDevice.getDevice(pad.id);
    }
}
