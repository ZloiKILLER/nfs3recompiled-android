package dev.nfs3hp.port;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/**
 * The control set the launcher writes into the game's own settings file,
 * fedata/config/config.dat.
 *
 * The game binds exactly one input to each function, in each of three tables:
 * one player, and the two players of split screen.  The port needs those
 * inputs to be what its gamepad and touch layers send, whatever game image the
 * player installed and whatever was bound in it before, so it writes them.
 * config.dat is a plain 8432-byte copy of the game's settings block with no
 * checksum.
 *
 * The one thing the game verifies is the device list kept in the same file.
 * At every start it compares the saved list with the devices DirectInput
 * reports -- count, type, button, hat and axis masks, name -- and on any
 * difference it regenerates all three tables.  So the list is written too,
 * describing exactly the two slots the port reports, and the tables survive.
 */
final class ControlProfile
{
    enum Kind { GAMEPADS, KEYBOARD }

    /** There is no settings file to write into: the game creates it the first
     *  time it runs. */
    static final class NoSettingsException extends IOException
    {
        NoSettingsException() { super("config.dat not found"); }
    }

    static final int FILE_SIZE = 0x20F0;

    private static final int DEVICE_RECORDS = 0xE30;
    private static final int DEVICE_RECORD_SIZE = 0x88;
    private static final int DEVICE_COUNT = 0x16C4;
    private static final int FORCE_FEEDBACK_DEVICES = 0x16C8;
    /** One player, split screen player 1, split screen player 2. */
    static final int[] TABLES = { 0x16CC, 0x1700, 0x1734 };
    static final int FUNCTIONS = 13;

    /* Where each function sits in a table. */
    static final int STEER_RIGHT = 0, STEER_LEFT = 1, ACCELERATE = 2, BRAKE = 3,
        HANDBRAKE = 4, GEAR_UP = 5, GEAR_DOWN = 6, CAMERA = 7, HORN = 8,
        LOOK_BEHIND = 9, RECOVER = 10, SPIKE_STRIP = 11, HEADLIGHTS = 12;

    /* A slot as the game records it: a plain joystick (type 1) with force
     * feedback (flag 8), no buttons and no hats, axes X and Y -- what
     * IDirectInputDevice::GetCapabilities reports for either slot. */
    static final int PAD_TYPE = 1 | 8;
    static final int PAD_AXES = 0x3;
    private static final int NAME_OFFSET = 0x04, NAME_LENGTH = 0x34;
    private static final int BUTTONS_OFFSET = 0x38, HATS_OFFSET = 0x3C, AXES_OFFSET = 0x40;
    /* What the game's own menus change about a device: force-feedback strength
     * and each axis's settings.  Kept when the record already describes the
     * slot; otherwise the values the game gives a device it has just found. */
    static final int GAIN_OFFSET = 0x44, AXIS_SETTINGS_OFFSET = 0x48;
    static final int DEFAULT_GAIN = 75;
    private static final int[] DEFAULT_AXIS_SETTINGS = { 0, 100, 0, 100 };

    /** A keyboard key as a table stores it -- DirectInput scan code, and the
     *  character if it has one -- and as SDL names it for the pad layer. */
    static final class Key
    {
        final int scanCode;
        final int character;
        final String sdlName;

        Key(int scanCode, int character, String sdlName)
        {
            this.scanCode = scanCode;
            this.character = character;
            this.sdlName = sdlName;
        }

        int record()
        {
            return 4 | scanCode << 8 | character << 24;
        }
    }

    private static Key letter(char name, int scanCode)
    {
        return new Key(scanCode, name, String.valueOf(name));
    }

    private static Key special(int scanCode, String sdlName)
    {
        return new Key(scanCode, 0, sdlName);
    }

    static final Key SPACE = new Key(0x39, ' ', "Space");
    static final Key RIGHT = special(0x4D, "Right"), LEFT = special(0x4B, "Left"),
        UP = special(0x48, "Up"), DOWN = special(0x50, "Down");
    static final Key A = letter('A', 0x1E), B = letter('B', 0x30), C = letter('C', 0x2E),
        D = letter('D', 0x20), E = letter('E', 0x12), F = letter('F', 0x21),
        G = letter('G', 0x22), H = letter('H', 0x23), K = letter('K', 0x25),
        L = letter('L', 0x26), M = letter('M', 0x32), P = letter('P', 0x19),
        Q = letter('Q', 0x10), R = letter('R', 0x13), S = letter('S', 0x1F),
        W = letter('W', 0x11), X = letter('X', 0x2D), Y = letter('Y', 0x15),
        Z = letter('Z', 0x2C);
    /* The game's split-screen defaults put player one on the numeric keypad. */
    private static final Key KEYPAD_0 = special(0x52, "Keypad 0"), KEYPAD_1 = special(0x4F, "Keypad 1"),
        KEYPAD_3 = special(0x51, "Keypad 3"), KEYPAD_7 = special(0x47, "Keypad 7"),
        KEYPAD_9 = special(0x49, "Keypad 9");

    /* The key behind each of the nine functions that stay keys, HANDBRAKE to
     * HEADLIGHTS, per player.  Player one keeps the game's own defaults, which
     * are also the keys the touch overlay sends out of the box.  Player two
     * gets letters no player-one function uses, every one of them a key the
     * game itself hands out in its split-screen defaults, so none is special
     * to it. */
    private static final Key[][] PLAYER_KEYS = {
        { SPACE, A, Z, C, H, B, R, S, L },
        { D, F, G, Q, W, E, X, P, Y },
    };

    private ControlProfile() {}

    static Key playerKey(int player, int function)
    {
        return PLAYER_KEYS[player][function - HANDBRAKE];
    }

    /* An axis as a table stores it: which half of which axis of which device.
     * The middle two bytes are the half's end and start on a 0..255 scale --
     * FF 80 is the upper half, 00 7F the lower. */
    private static int axis(int device, int axis, boolean upperHalf)
    {
        return 1 | (upperHalf ? 0xFF << 8 | 0x80 << 16 : 0x7F << 16) | (device << 4 | axis) << 24;
    }

    private static int[] gamepadTable(int device, int player)
    {
        int[] table = new int[FUNCTIONS];
        // X grows to the right; Y grows towards the player, so up is its lower half.
        table[STEER_RIGHT] = axis(device, 0, true);
        table[STEER_LEFT] = axis(device, 0, false);
        table[ACCELERATE] = axis(device, 1, false);
        table[BRAKE] = axis(device, 1, true);
        for (int function = HANDBRAKE; function < FUNCTIONS; ++function)
            table[function] = playerKey(player, function).record();
        return table;
    }

    private static int[] keys(Key... keys)
    {
        int[] table = new int[keys.length];
        for (int i = 0; i < keys.length; ++i)
            table[i] = keys[i].record();
        return table;
    }

    /** The three tables of a set, as the four-byte records the file holds. */
    static int[][] tables(Kind kind)
    {
        if (kind == Kind.GAMEPADS)
        {
            /* Split screen player one is exactly the one-player set, so the
             * first player's controls do not change between the two modes. */
            return new int[][] { gamepadTable(0, 0), gamepadTable(0, 0), gamepadTable(1, 1) };
        }
        // The game's own keyboard defaults, as nfs3.exe carries them from 0x490430.
        return new int[][] {
            keys(RIGHT, LEFT, UP, DOWN, SPACE, A, Z, C, H, B, R, S, L),
            keys(RIGHT, LEFT, UP, DOWN, KEYPAD_0, KEYPAD_9, KEYPAD_3, K, M, KEYPAD_7, KEYPAD_1, P, L),
            keys(G, D, R, F, SPACE, A, Z, Q, W, E, X, S, Y),
        };
    }

    static boolean isSettingsFile(byte[] config)
    {
        return config.length == FILE_SIZE
            && ByteBuffer.wrap(config).order(ByteOrder.LITTLE_ENDIAN).getInt(0) == FILE_SIZE;
    }

    /** Writes the set into a settings file held in memory. */
    static void apply(byte[] config, Kind kind)
    {
        if (!isSettingsFile(config))
            throw new IllegalArgumentException("not a config.dat");
        ByteBuffer buffer = ByteBuffer.wrap(config).order(ByteOrder.LITTLE_ENDIAN);
        for (int slot = 0; slot < GamepadSlots.COUNT; ++slot)
            describeSlot(buffer, slot);
        buffer.putInt(DEVICE_COUNT, GamepadSlots.COUNT);
        buffer.putInt(FORCE_FEEDBACK_DEVICES, GamepadSlots.COUNT);
        int[][] tables = tables(kind);
        for (int table = 0; table < TABLES.length; ++table)
            for (int function = 0; function < FUNCTIONS; ++function)
                buffer.putInt(TABLES[table] + 4 * function, tables[table][function]);
    }

    /* The records of devices 2 and up are left alone: the game clears them
     * itself when it finds no such device, and never compares them. */
    private static void describeSlot(ByteBuffer config, int slot)
    {
        int base = DEVICE_RECORDS + slot * DEVICE_RECORD_SIZE;
        byte[] name = ("NFS Gamepad " + (slot + 1)).getBytes(StandardCharsets.US_ASCII);
        boolean described = config.getInt(base) == PAD_TYPE
            && config.getInt(base + BUTTONS_OFFSET) == 0
            && config.getInt(base + HATS_OFFSET) == 0
            && config.getInt(base + AXES_OFFSET) == PAD_AXES
            && nameIs(config, base + NAME_OFFSET, name);
        int gain = described ? config.getInt(base + GAIN_OFFSET) : DEFAULT_GAIN;
        int[] axisSettings = new int[DEFAULT_AXIS_SETTINGS.length];
        for (int i = 0; i < axisSettings.length; ++i)
            axisSettings[i] = described ? config.getInt(base + AXIS_SETTINGS_OFFSET + 4 * i)
                                        : DEFAULT_AXIS_SETTINGS[i];

        for (int i = 0; i < DEVICE_RECORD_SIZE; ++i)
            config.put(base + i, (byte) 0);
        config.putInt(base, PAD_TYPE);
        for (int i = 0; i < name.length; ++i)
            config.put(base + NAME_OFFSET + i, name[i]);
        config.putInt(base + AXES_OFFSET, PAD_AXES);
        config.putInt(base + GAIN_OFFSET, gain);
        for (int i = 0; i < axisSettings.length; ++i)
            config.putInt(base + AXIS_SETTINGS_OFFSET + 4 * i, axisSettings[i]);
    }

    // Compared as the game compares it: as a C string.
    private static boolean nameIs(ByteBuffer config, int offset, byte[] name)
    {
        for (int i = 0; i < name.length; ++i)
            if (config.get(offset + i) != name[i])
                return false;
        return name.length < NAME_LENGTH && config.get(offset + name.length) == 0;
    }

    /** Writes the set into the active game data and returns the name of the
     *  backup taken of the file first. */
    static String writeToGame(File dataRoot, Kind kind) throws IOException
    {
        File file = settingsFile(dataRoot);
        if (file == null)
            throw new NoSettingsException();
        byte[] config = Files.readAllBytes(file.toPath());
        if (!isSettingsFile(config))
            throw new NoSettingsException();
        String backup = backup(dataRoot, config);
        apply(config, kind);
        /* Written beside the file and moved over it, so a failure half way
         * leaves the old settings rather than a torn file. */
        File partial = new File(file.getParentFile(), file.getName() + ".part");
        try (FileOutputStream out = new FileOutputStream(partial))
        {
            out.write(config);
            out.getFD().sync();
        }
        if (!partial.renameTo(file))
        {
            if (!partial.delete())
                partial.deleteOnExit();
            throw new IOException("Could not replace " + file);
        }
        return backup;
    }

    private static File settingsFile(File dataRoot)
    {
        File fedata = child(dataRoot, "fedata");
        File config = fedata == null ? null : child(fedata, "config");
        File file = config == null ? null : child(config, "config.dat");
        return file != null && file.isFile() ? file : null;
    }

    // Game data comes off a disc image, where names can be in any case.
    private static File child(File directory, String name)
    {
        String[] names = directory.list();
        if (names == null)
            return null;
        for (String candidate : names)
            if (candidate.equalsIgnoreCase(name))
                return new File(directory, candidate);
        return null;
    }

    private static String backup(File dataRoot, byte[] config) throws IOException
    {
        File folder = new File(dataRoot, ".launcher-backups");
        if (!folder.isDirectory() && !folder.mkdirs())
            throw new IOException("Could not create " + folder);
        String stamp = new SimpleDateFormat("yyyyMMdd-HHmmss", Locale.ROOT).format(new Date());
        File file = new File(folder, "config-" + stamp + ".dat");
        for (int suffix = 1; file.exists(); ++suffix)
            file = new File(folder, "config-" + stamp + "-" + suffix + ".dat");
        try (FileOutputStream out = new FileOutputStream(file))
        {
            out.write(config);
        }
        return file.getName();
    }
}
