package dev.nfs3hp.port;

import android.content.SharedPreferences;

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
public final class ControlProfile
{
    /** GAMEPADS: the pads drive, steering and pedals on their axes, and the
     *  on-screen controls reach those axes through the port.  TOUCH: the
     *  on-screen controls drive, as the keyboard they really are.  KEYBOARD:
     *  the game's own defaults, for a player who wants them. */
    public enum Kind { GAMEPADS, TOUCH, KEYBOARD }

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

    /** What the on-screen controls steer and work the pedals with when they are
     *  what drives: the four keys they send, as the player set them in
     *  Controls -> Touch -> Keys, in the order the tables want them. */
    static Key[] touchDriving(SharedPreferences preferences)
    {
        return new Key[] {
            touchDrivingKey(preferences, "steer_right", RIGHT),
            touchDrivingKey(preferences, "steer_left", LEFT),
            touchDrivingKey(preferences, "accelerate", UP),
            touchDrivingKey(preferences, "brake", DOWN),
        };
    }

    private static Key touchDrivingKey(SharedPreferences preferences, String action, Key fallback)
    {
        Key key = androidKey(GamePreferences.getTouchKey(preferences, action));
        return key == null ? fallback : key;
    }

    /* The launcher keeps a control's key as Android names it; a table keeps the
     * scan code the game knows.  Only the keys the launcher offers appear here
     * (GamePreferences.KEY_VALUES); Enter and Escape are not among them,
     * because the game gives neither to a car. */
    private static Key androidKey(int code)
    {
        switch (code)
        {
        case android.view.KeyEvent.KEYCODE_DPAD_UP: return UP;
        case android.view.KeyEvent.KEYCODE_DPAD_DOWN: return DOWN;
        case android.view.KeyEvent.KEYCODE_DPAD_LEFT: return LEFT;
        case android.view.KeyEvent.KEYCODE_DPAD_RIGHT: return RIGHT;
        case android.view.KeyEvent.KEYCODE_SPACE: return SPACE;
        case android.view.KeyEvent.KEYCODE_A: return A;
        case android.view.KeyEvent.KEYCODE_B: return B;
        case android.view.KeyEvent.KEYCODE_C: return C;
        case android.view.KeyEvent.KEYCODE_H: return H;
        case android.view.KeyEvent.KEYCODE_L: return L;
        case android.view.KeyEvent.KEYCODE_R: return R;
        case android.view.KeyEvent.KEYCODE_S: return S;
        case android.view.KeyEvent.KEYCODE_Z: return Z;
        default: return null;
        }
    }

    /** The three tables of a set, as the four-byte records the file holds. */
    static int[][] tables(Kind kind)
    {
        return tables(kind, new Key[] { RIGHT, LEFT, UP, DOWN });
    }

    static int[][] tables(Kind kind, Key[] touchDriving)
    {
        if (kind == Kind.TOUCH)
        {
            /* The on-screen controls as the keyboard they are: the four that
             * drive are bound to the keys those buttons send, so the game
             * steers them the way it steers a keyboard -- turning towards the
             * lock rather than snapping to it -- because it can see that is
             * what they are.  Nothing needs telling it afterwards, and a race
             * driven on them is a race the game can replay, which one driven
             * through a pad's axes by a port holding a flag up is not.  The
             * nine that are not driving stay the player's keys, which the
             * buttons send as they are.  Split screen's second player keeps
             * the second pad: two players on one screen means a pad. */
            int[] one = gamepadTable(0, 0);
            one[STEER_RIGHT] = touchDriving[0].record();
            one[STEER_LEFT] = touchDriving[1].record();
            one[ACCELERATE] = touchDriving[2].record();
            one[BRAKE] = touchDriving[3].record();
            return new int[][] { one, one.clone(), gamepadTable(1, 1) };
        }
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

    /** Which of the sets the game's settings file holds: null for any other
     *  bindings -- the player's own, made in the game's Controls screen -- and
     *  for no settings file at all. */
    static Kind installedKind(File dataRoot)
    {
        return installedKind(dataRoot, new Key[] { RIGHT, LEFT, UP, DOWN });
    }

    static Kind installedKind(File dataRoot, Key[] touchDriving)
    {
        try
        {
            File file = settingsFile(dataRoot);
            if (file == null)
                return null;
            byte[] config = Files.readAllBytes(file.toPath());
            if (!isSettingsFile(config))
                return null;
            ByteBuffer buffer = ByteBuffer.wrap(config).order(ByteOrder.LITTLE_ENDIAN);
            for (Kind kind : Kind.values())
            {
                int[][] tables = tables(kind, touchDriving);
                boolean same = true;
                for (int table = 0; table < TABLES.length && same; ++table)
                    for (int function = 0; function < FUNCTIONS && same; ++function)
                        same = buffer.getInt(TABLES[table] + 4 * function) == tables[table][function];
                if (same)
                    return kind;
            }
        }
        catch (IOException ignored)
        {
            // Unreadable is as good as unknown here.
        }
        return null;
    }

    static boolean isSettingsFile(byte[] config)
    {
        return config.length == FILE_SIZE
            && ByteBuffer.wrap(config).order(ByteOrder.LITTLE_ENDIAN).getInt(0) == FILE_SIZE;
    }

    /** Writes the set into a settings file held in memory. */
    static void apply(byte[] config, Kind kind)
    {
        apply(config, kind, new Key[] { RIGHT, LEFT, UP, DOWN });
    }

    static void apply(byte[] config, Kind kind, Key[] touchDriving)
    {
        if (!isSettingsFile(config))
            throw new IllegalArgumentException("not a config.dat");
        ByteBuffer buffer = ByteBuffer.wrap(config).order(ByteOrder.LITTLE_ENDIAN);
        for (int slot = 0; slot < GamepadSlots.COUNT; ++slot)
            describeSlot(buffer, slot);
        buffer.putInt(DEVICE_COUNT, GamepadSlots.COUNT);
        buffer.putInt(FORCE_FEEDBACK_DEVICES, GamepadSlots.COUNT);
        int[][] tables = tables(kind, touchDriving);
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
        return writeToGame(dataRoot, kind, new Key[] { RIGHT, LEFT, UP, DOWN });
    }

    static String writeToGame(File dataRoot, Kind kind, Key[] touchDriving) throws IOException
    {
        File file = settingsFile(dataRoot);
        if (file == null)
            throw new NoSettingsException();
        byte[] config = Files.readAllBytes(file.toPath());
        if (!isSettingsFile(config))
            throw new NoSettingsException();
        String backup = backup(dataRoot, config);
        apply(config, kind, touchDriving);
        replace(file, config);
        return backup;
    }

    /** The set that belongs in the settings file for how the player is about to
     *  drive, written there if it is not what the file already holds.  A pad
     *  assigned to player one steers on its axes (GAMEPADS); with no pad, the
     *  on-screen controls drive as the keyboard they are (TOUCH), which is what
     *  lets the game record and replay a race driven on them.  Settings the
     *  player made themselves, in the game's own Controls screen, are not one of
     *  the sets and are left exactly as they are.  Returns the set in force. */
    static Kind driveWith(File dataRoot, SharedPreferences preferences, boolean padForPlayerOne)
    {
        Key[] touchDriving = touchDriving(preferences);
        Kind installed = installedKind(dataRoot, touchDriving);
        if (installed == null || installed == Kind.KEYBOARD)
            return installed;
        Kind wanted = padForPlayerOne ? Kind.GAMEPADS : Kind.TOUCH;
        if (installed == wanted)
            return wanted;
        try
        {
            writeToGame(dataRoot, wanted, touchDriving);
        }
        catch (IOException couldNotWrite)
        {
            android.util.Log.w("ControlProfile", "Could not set the controls up for this race", couldNotWrite);
            return installed;
        }
        return wanted;
    }

    /* View Distance, Full to Close as 0 to 3 ([0x6fbc28] in the game). */
    static final int VIEW_DISTANCE = 0xE8;
    /* Screen Size, kept as a width alone ([0x6fbc18]): at start the game takes
     * the screen mode nearest it and finds its place in the menu's list itself
     * (sub_4730d0), so the list index beside it (+0xD4) need not be written. */
    static final int SCREEN_WIDTH = 0xD8;
    static final int IMPORT_SCREEN_WIDTH = 1280;

    /** What a freshly imported game starts with: the gamepad set, which is what
     *  the pads and the on-screen controls send, View Distance at Full, the
     *  whole track out to the horizon, a 1280x720 screen for races, which
     *  any phone draws smoothly -- a player after more picks it in the game's
     *  Graphics menu -- a HUD arranged for a phone (hudDefaults) and the cop's
     *  map as a minimap in split screen too (copMinimap).  Written once, as the last step of an
     *  import, into the settings file that came with the data -- and never
     *  again: from then on the file is the player's, whatever they change in
     *  the game or write from the Controls screen.  Last, because the game
     *  throws away a settings file older than its executable, and the
     *  executable is in place before any import starts.  A file that is not a
     *  settings file is left for the game, which starts it over itself --
     *  and then gets the same from firstStart. */
    static void writeImportDefaults(File dataRoot) throws IOException
    {
        File file = settingsFile(dataRoot);
        if (file == null)
            return;
        byte[] config = Files.readAllBytes(file.toPath());
        if (!isSettingsFile(config))
            return;
        newGame(config, Kind.GAMEPADS, new Key[] { RIGHT, LEFT, UP, DOWN });
        replace(file, config);
        // Done as part of the import: the launcher's one-off pass leaves it be.
        copMinimapDone(file.getParentFile());
    }

    /** The same for a game with no settings file of its own -- data copied
     *  straight off the disc, which has none: the game makes a new player's
     *  settings as it first starts and chooses what suits the machine
     *  (sub_472d10), and this goes over them before the game saves them
     *  (NFS3Activity.onFirstSettings).  The controls are the set for how player
     *  one is about to drive, as the launch decided.  Returns whether the block
     *  was a settings block to change at all. */
    static boolean firstStart(byte[] config, Kind kind, Key[] touchDriving, File dataRoot)
    {
        if (!isSettingsFile(config))
            return false;
        newGame(config, kind, touchDriving);
        File fedata = dataRoot == null ? null : child(dataRoot, "fedata");
        File folder = fedata == null ? null : child(fedata, "config");
        if (folder != null)
            copMinimapDone(folder);
        return true;
    }

    /* What a new game starts with, into a settings block held in memory. */
    private static void newGame(byte[] config, Kind kind, Key[] touchDriving)
    {
        apply(config, kind, touchDriving);
        ByteBuffer buffer = ByteBuffer.wrap(config).order(ByteOrder.LITTLE_ENDIAN);
        buffer.putInt(VIEW_DISTANCE, 0);
        buffer.putInt(SCREEN_WIDTH, IMPORT_SCREEN_WIDTH);
        hudDefaults(config);
        copMinimap(config);
    }

    /** Whether the game will find settings of its own to start with: a
     *  settings file it can read, where the game keeps it. */
    static boolean hasSettings(File dataRoot)
    {
        try
        {
            File file = settingsFile(dataRoot);
            return file != null && isSettingsFile(Files.readAllBytes(file.toPath()));
        }
        catch (IOException unreadable)
        {
            return false;
        }
    }

    private static void copMinimapDone(File folder)
    {
        File done = new File(folder, COP_MINIMAP_DONE);
        try
        {
            if (!done.exists() && !done.createNewFile())
                android.util.Log.w("ControlProfile", "Could not mark the cop's minimap as done");
        }
        catch (IOException e)
        {
            android.util.Log.w("ControlProfile", "Could not mark the cop's minimap as done", e);
        }
    }

    /* The HUD layouts in the settings file: from 0x6fbc50 in the game, a slot
     * for one player and one for each of split screen's two, HUD_SLOT apart,
     * each holding a racer's layout and then a cop's, HUD_LAYOUT apart.  In a
     * layout: the map's scale (float), its mode, the 23 elements from
     * HUD_ELEMENTS (top, left, bottom, right as fractions of the screen), and
     * after them the small map's place. */
    private static final int HUD_LAYOUTS = 0x6fbc50 - 0x6fbb40;
    private static final int HUD_SLOT = 0x348, HUD_LAYOUT = 0x1a4;
    private static final int HUD_MAP_SCALE = 0x10, HUD_MAP_MODE = 0x20, HUD_ELEMENTS = 0x24, HUD_SMALL_MAP = 0x194;
    private static final int COP_MAP = 22;
    /* The Map list in the game's HUD menu, as kept: 0 off, 1 rotating, 2 stationary. */
    private static final int MAP_ROTATING = 1, MAP_STATIONARY = 2;
    /* Beside config.dat once the cop's map has been set up, so it happens once. */
    private static final String COP_MINIMAP_DONE = ".port-cop-minimap";

    /* The HUD an import hands the player, arranged on a phone in the game's own
     * HUD screen.  The game's arrangement is a 640x480 monitor's: the standings
     * and the map lie along the bottom, where a phone's thumbs and the
     * on-screen controls are, and the cop's map covers the road entirely.  Here
     * the standings and the map move to the top corners, the cone stats sit
     * under the standings, the cop's table of speeders is a block on the left
     * and both maps are corner minimaps.  The Modern Patch likewise ships a HUD
     * of its own rather than the game's.  Only the single player's two layouts
     * are set; split screen keeps the game's, with its cop map brought into the
     * corner by copMinimap.
     *
     * A row is one element's top, left, bottom and right as fractions of the
     * screen, in the game's own order -- gamedata/dashhud/def.pos names them and
     * holds the arrangement these started from.  A row of zeroes is an element
     * the layout does not show: the cop has no standings, the racer no radar. */
    private static final float[] RACER_HUD = {
        0.0080f, 0.1370f, 0.0600f, 0.3420f, /* DigitalSpeedometer */
        0.0021f, 0.2141f, 0.1521f, 0.3297f, /* AnalogSpeedometer */
        0.0042f, 0.6500f, 0.1000f, 0.8406f, /* DigitalTachometer */
        0.0000f, 0.6672f, 0.1500f, 0.7813f, /* AnalogTachometer */
        0.0463f, -0.0234f, 0.0870f, 0.1214f, /* LapCounter */
        0.0065f, -0.0250f, 0.0444f, 0.1042f, /* Time */
        0.1970f, 0.4625f, 0.2970f, 0.5375f, /* TutorIcon */
        0.3056f, -0.0516f, 0.3796f, 0.2432f, /* ConeStats */
        0.9100f, 0.3500f, 0.9900f, 0.6500f, /* ReplayHUD */
        0.0100f, 0.3500f, 0.1430f, 0.6500f, /* RearView */
        0.0454f, 0.9187f, 0.0833f, 1.0589f, /* Position */
        0.0056f, 0.8859f, 0.0417f, 1.0245f, /* SplitTime */
        0.4102f, 0.8016f, 0.4509f, 1.0698f, /* OpponentName */
        0.1000f, 0.0000f, 0.2963f, 0.1495f, /* OpponentList */
        0.3704f, 0.8302f, 0.4167f, 1.0422f, /* PlayerTickets */
        0.4000f, 0.3880f, 0.4400f, 0.6120f, /* WrongWayIndicator */
        0.1500f, 0.3438f, 0.1750f, 0.6563f, /* RadarDetector */
        0.1000f, 0.8500f, 0.3741f, 0.9995f, /* RacerMap */
        0.0000f, 0.0000f, 0.0000f, 0.0000f, /* Radar */
        0.0000f, 0.0000f, 0.0000f, 0.0000f, /* CopTickets */
        0.0000f, 0.0000f, 0.0000f, 0.0000f, /* RadarSpeeds */
        0.0000f, 0.0000f, 0.0000f, 0.0000f, /* SpikeBeltIndicator */
        0.0000f, 0.0000f, 0.0000f, 0.0000f, /* CopMap */
    };
    private static final float[] RACER_SMALL_MAP = { 0.1000f, 0.8500f, 0.3741f, 0.9995f };
    private static final float[] COP_HUD = {
        0.0080f, 0.1370f, 0.0600f, 0.3420f, /* DigitalSpeedometer */
        0.0000f, 0.2281f, 0.1500f, 0.3406f, /* AnalogSpeedometer */
        0.0060f, 0.6600f, 0.1020f, 0.8510f, /* DigitalTachometer */
        0.0000f, 0.6562f, 0.1500f, 0.7719f, /* AnalogTachometer */
        0.0080f, 0.8500f, 0.0500f, 0.9950f, /* LapCounter */
        0.0120f, 0.0000f, 0.0540f, 0.1310f, /* Time */
        0.1560f, 0.4625f, 0.2560f, 0.5375f, /* TutorIcon */
        0.1542f, 0.7031f, 0.2292f, 0.9937f, /* ConeStats */
        0.9125f, 0.3656f, 0.9958f, 0.6656f, /* ReplayHUD */
        0.0125f, 0.3469f, 0.1458f, 0.6469f, /* RearView */
        0.0000f, 0.0000f, 0.0000f, 0.0000f, /* Position */
        0.0000f, 0.0000f, 0.0000f, 0.0000f, /* SplitTime */
        0.0000f, 0.0000f, 0.0000f, 0.0000f, /* OpponentName */
        0.0000f, 0.0000f, 0.0000f, 0.0000f, /* OpponentList */
        0.0000f, 0.0000f, 0.0000f, 0.0000f, /* PlayerTickets */
        0.0000f, 0.0000f, 0.0000f, 0.0000f, /* WrongWayIndicator */
        0.0000f, 0.0000f, 0.0000f, 0.0000f, /* RadarDetector */
        0.0000f, 0.0000f, 0.0000f, 0.0000f, /* RacerMap */
        0.1740f, 0.0250f, 0.5080f, 0.2750f, /* Radar */
        0.1167f, 0.7937f, 0.1646f, 0.9969f, /* CopTickets */
        0.5269f, 0.0000f, 0.7565f, 0.3260f, /* RadarSpeeds */
        0.0680f, 0.8560f, 0.0921f, 0.9960f, /* SpikeBeltIndicator */
        0.6479f, 0.8484f, 0.9167f, 0.9984f, /* CopMap */
    };
    private static final float[] COP_SMALL_MAP = { 0.6479f, 0.8484f, 0.9167f, 0.9984f };

    /* How far the maps are zoomed in, as the arrangement has them. */
    private static final float MAP_ZOOM = 4f;

    /** The arrangement above, into a settings file held in memory: the single
     *  player's racer layout and cop layout, each with the small map the map
     *  key shrinks to and the map's zoom and mode. */
    static void hudDefaults(byte[] config)
    {
        if (!isSettingsFile(config))
            return;
        ByteBuffer buffer = ByteBuffer.wrap(config).order(ByteOrder.LITTLE_ENDIAN);
        hudLayout(buffer, 0, RACER_HUD, RACER_SMALL_MAP);
        hudLayout(buffer, 1, COP_HUD, COP_SMALL_MAP);
    }

    private static void hudLayout(ByteBuffer buffer, int variant, float[] elements, float[] smallMap)
    {
        int layout = HUD_LAYOUTS + variant * HUD_LAYOUT;
        for (int i = 0; i < elements.length; ++i)
            buffer.putFloat(layout + HUD_ELEMENTS + 4 * i, elements[i]);
        for (int i = 0; i < smallMap.length; ++i)
            buffer.putFloat(layout + HUD_SMALL_MAP + 4 * i, smallMap[i]);
        buffer.putFloat(layout + HUD_MAP_SCALE, MAP_ZOOM);
        buffer.putInt(layout + HUD_MAP_MODE, MAP_ROTATING);
    }

    /** The cop's map as a minimap in the corner, as the Modern Patch sets it up
     *  when it is installed.  The game's own default is the whole track across
     *  the whole screen -- DASHHUD/DEF.POS has CopCopMap at (0,0)-(1,1), the Map
     *  setting at Stationary and the scale at 1 -- which on a phone lies over
     *  the road and the controls.  The corner is the game's own: the small map
     *  its map key shrinks the cop's to (sub_48e790), kept after the layout's
     *  elements.  Rotating, and zoomed as far as the racer's map, as the Modern
     *  Patch does; the HUD menu and the map key still offer the rest.  Only a
     *  map still across the whole width at Stationary and scale 1 is changed.
     *  Returns whether anything was. */
    static boolean copMinimap(byte[] config)
    {
        if (!isSettingsFile(config))
            return false;
        ByteBuffer buffer = ByteBuffer.wrap(config).order(ByteOrder.LITTLE_ENDIAN);
        boolean changed = false;
        for (int slot = 0; slot < 3; ++slot)
        {
            int racer = HUD_LAYOUTS + slot * HUD_SLOT, cop = racer + HUD_LAYOUT;
            int map = cop + HUD_ELEMENTS + COP_MAP * 16;
            boolean asTheGameHasIt = buffer.getInt(cop + HUD_MAP_MODE) == MAP_STATIONARY
                && buffer.getFloat(cop + HUD_MAP_SCALE) == 1f
                && buffer.getFloat(map + 4) == 0f && buffer.getFloat(map + 12) == 1f;
            if (!asTheGameHasIt)
                continue;
            for (int edge = 0; edge < 16; edge += 4)
                buffer.putInt(map + edge, buffer.getInt(cop + HUD_SMALL_MAP + edge));
            buffer.putInt(cop + HUD_MAP_MODE, MAP_ROTATING);
            buffer.putFloat(cop + HUD_MAP_SCALE, buffer.getFloat(racer + HUD_MAP_SCALE));
            changed = true;
        }
        return changed;
    }

    /** The cop's minimap for game data imported before it was part of an
     *  import: once per data set -- a player who puts the big map back later
     *  keeps it -- and never while the game runs, which would write its own
     *  settings over it on the way out. */
    static void copMinimapOnce(File dataRoot)
    {
        try
        {
            File file = settingsFile(dataRoot);
            if (file == null)
                return;
            File done = new File(file.getParentFile(), COP_MINIMAP_DONE);
            if (done.exists())
                return;
            byte[] config = Files.readAllBytes(file.toPath());
            if (copMinimap(config))
                replace(file, config);
            if (!done.createNewFile())
                android.util.Log.w("ControlProfile", "Could not mark the cop's minimap as done");
        }
        catch (IOException e)
        {
            android.util.Log.w("ControlProfile", "Could not set up the cop's minimap", e);
        }
    }

    /* Written beside the file and moved over it, so a failure half way leaves
     * the old settings rather than a torn file. */
    private static void replace(File file, byte[] config) throws IOException
    {
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
