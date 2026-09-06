#include <lib/gamepad.h>
#include <SDL3/SDL.h>
#include <cstring>
#include <string>


namespace win32
{

static x86::reg32 s_kbPoll = 3;

static Gamepad* s_gp1;

void Input::poll()
{
    s_kbPoll = 3;
    SDL_UpdateJoysticks();
}

bool Gamepad::isPolledByGame()
{
    return s_gp1 != nullptr && s_kbPoll != 0;
}

/* ----------------------------------------------------------------------
 * Menu/system controls: translate a physical gamepad into real keyboard
 * events, independent of whatever the game itself has bound as a
 * DirectInput steering device.
 *
 * This cannot go through SDL's SDL_EVENT_GAMEPAD_* event stream, even
 * though window.cpp briefly tried that.  win32::Gamepad's constructor below
 * calls SDL_SetJoystickEventsEnabled(false) -- and the game triggers that
 * constructor itself by enumerating DirectInput devices at startup, well
 * before any menu is shown.  SDL3 turns raw joystick state into gamepad
 * events through an internal event watcher (SDL_GamepadEventWatcher) that
 * only runs *inside* SDL_PushEvent when a joystick event is actually
 * pushed; with joystick events disabled, SDL_SendJoystickButton/Axis/Hat
 * skip that push entirely, so the watcher never runs and no
 * SDL_EVENT_GAMEPAD_BUTTON_DOWN/AXIS_MOTION is ever generated for the rest
 * of the process's life.  (SDL_EVENT_GAMEPAD_ADDED/REMOVED still fire --
 * those are sent directly, not through the watcher -- which is what made
 * this look like a button-mapping bug rather than a dead event pipeline:
 * the pad shows up as connected, then goes silent.)
 *
 * Polling SDL_GetGamepadButton/Axis instead sidesteps all of that: they
 * read live joystick state, which SDL keeps updated regardless of whether
 * events are enabled. */

static SDL_Gamepad* s_padDevice = nullptr;

static void ensurePadOpen()
{
    if (s_padDevice)
    {
        if (SDL_GamepadConnected(s_padDevice))
            return;
        SDL_CloseGamepad(s_padDevice);
        s_padDevice = nullptr;
    }
    int count = 0;
    SDL_JoystickID* ids = SDL_GetGamepads(&count);
    if (ids && count > 0)
    {
        s_padDevice = SDL_OpenGamepad(ids[0]);
        if (s_padDevice)
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "gamepad connected: %s",
                        SDL_GetGamepadName(s_padDevice));
    }
    SDL_free(ids);
}

static void sendKey(bool down, SDL_Keycode key, SDL_Scancode scancode)
{
    if (key == SDLK_UNKNOWN) return;
    SDL_Event event;
    SDL_zero(event);
    event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    event.key.timestamp = SDL_GetTicksNS();
    event.key.scancode = scancode;
    event.key.key = key;
    event.key.down = down;
    SDL_PushEvent(&event);
}

/* One threshold to press, a lower one to release, so a stick resting near
 * the edge does not machine-gun the menu. */
static const Sint16 STICK_PRESS   = 20000;
static const Sint16 STICK_RELEASE = 12000;

static void updateAxisAsKeys(Sint16 value, int& state,
                             SDL_Keycode negKey, SDL_Scancode negScan,
                             SDL_Keycode posKey, SDL_Scancode posScan)
{
    const int wanted = value >  STICK_PRESS ?  1
                     : value < -STICK_PRESS ? -1
                     : (value < STICK_RELEASE && value > -STICK_RELEASE) ? 0
                     : state;
    if (wanted == state)
        return;
    if (state ==  1) sendKey(false, posKey, posScan);
    if (state == -1) sendKey(false, negKey, negScan);
    if (wanted ==  1) sendKey(true, posKey, posScan);
    if (wanted == -1) sendKey(true, negKey, negScan);
    state = wanted;
}

/* ----------------------------------------------------------------------
 * NFS_GAMEPAD_MAPPING: optional remapping of the system-button/D-pad tables
 * below, set by the Android launcher's Controls screen (GamePreferences.
 * gamepadMappingEnvironment() in the Java side) via Os.setenv() before the
 * native thread starts.  Format is comma-separated physicalButton=keyName
 * pairs, e.g. "south=return,west=space,dpad_left=left" -- both sides use
 * fixed short names rather than raw SDL enum/keycode values so the env var
 * stays readable in logcat and stable if SDL's own enum values ever change.
 * Unset (the common case, no launcher involved) or a name this cannot parse
 * leaves the compiled-in default for that button untouched. */
struct KeyBinding
{
    SDL_Keycode  key;
    SDL_Scancode scancode;
};

static bool keyBindingFromName(const char* name, KeyBinding& out)
{
    struct NamedKey { const char* name; SDL_Keycode key; SDL_Scancode scancode; };
    static const NamedKey s_namedKeys[] =
    {
        { "up",     SDLK_UP,     SDL_SCANCODE_UP     },
        { "down",   SDLK_DOWN,   SDL_SCANCODE_DOWN   },
        { "left",   SDLK_LEFT,   SDL_SCANCODE_LEFT   },
        { "right",  SDLK_RIGHT,  SDL_SCANCODE_RIGHT  },
        { "return", SDLK_RETURN, SDL_SCANCODE_RETURN },
        { "escape", SDLK_ESCAPE, SDL_SCANCODE_ESCAPE },
        { "space",  SDLK_SPACE,  SDL_SCANCODE_SPACE  },
        { "c",      SDLK_C,      SDL_SCANCODE_C      },
        { "b",      SDLK_B,      SDL_SCANCODE_B      },
        { "h",      SDLK_H,      SDL_SCANCODE_H      },
        { "l",      SDLK_L,      SDL_SCANCODE_L      },
        { "r",      SDLK_R,      SDL_SCANCODE_R      },
        { "a",      SDLK_A,      SDL_SCANCODE_A      },
        { "z",      SDLK_Z,      SDL_SCANCODE_Z      },
        { "unknown", SDLK_UNKNOWN, SDL_SCANCODE_UNKNOWN },
    };
    for (size_t i = 0; i < SDL_arraysize(s_namedKeys); ++i)
    {
        if (SDL_strcasecmp(name, s_namedKeys[i].name) == 0)
        {
            out.key = s_namedKeys[i].key;
            out.scancode = s_namedKeys[i].scancode;
            return true;
        }
    }
    return false;
}

static bool gamepadButtonFromName(const char* name, SDL_GamepadButton& out)
{
    struct NamedButton { const char* name; SDL_GamepadButton button; };
    static const NamedButton s_namedButtons[] =
    {
        { "back",           SDL_GAMEPAD_BUTTON_BACK           },
        { "start",          SDL_GAMEPAD_BUTTON_START          },
        { "south",          SDL_GAMEPAD_BUTTON_SOUTH          },
        { "east",           SDL_GAMEPAD_BUTTON_EAST           },
        { "west",           SDL_GAMEPAD_BUTTON_WEST           },
        { "north",          SDL_GAMEPAD_BUTTON_NORTH          },
        { "left_shoulder",  SDL_GAMEPAD_BUTTON_LEFT_SHOULDER  },
        { "right_shoulder", SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER },
        { "dpad_up",        SDL_GAMEPAD_BUTTON_DPAD_UP        },
        { "dpad_down",      SDL_GAMEPAD_BUTTON_DPAD_DOWN      },
        { "dpad_left",      SDL_GAMEPAD_BUTTON_DPAD_LEFT      },
        { "dpad_right",     SDL_GAMEPAD_BUTTON_DPAD_RIGHT     },
        { "left_stick",     SDL_GAMEPAD_BUTTON_LEFT_STICK     },
        { "right_stick",    SDL_GAMEPAD_BUTTON_RIGHT_STICK    },
    };
    for (size_t i = 0; i < SDL_arraysize(s_namedButtons); ++i)
    {
        if (SDL_strcasecmp(name, s_namedButtons[i].name) == 0)
        {
            out = s_namedButtons[i].button;
            return true;
        }
    }
    return false;
}

/* Parsed once (the env var never changes at runtime) into a flat table
 * indexed by SDL_GamepadButton -- small and fixed-size, so no map/allocation
 * needed.  s_overrideSet[button] is false for every button NFS_GAMEPAD_MAPPING
 * did not mention, which is every button whenever the variable is unset. */
static bool        s_overrideSet[SDL_GAMEPAD_BUTTON_COUNT];
static KeyBinding  s_override[SDL_GAMEPAD_BUTTON_COUNT];

static void parseGamepadMapping()
{
    const char* env = SDL_getenv("NFS_GAMEPAD_MAPPING");
    if (!env || !*env)
        return;
    std::string mapping(env);
    size_t pos = 0;
    while (pos <= mapping.size())
    {
        size_t comma = mapping.find(',', pos);
        const std::string pair = mapping.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
        const size_t eq = pair.find('=');
        if (eq != std::string::npos)
        {
            SDL_GamepadButton button = SDL_GAMEPAD_BUTTON_INVALID;
            KeyBinding binding = { SDLK_UNKNOWN, SDL_SCANCODE_UNKNOWN };
            if (gamepadButtonFromName(pair.substr(0, eq).c_str(), button) &&
                keyBindingFromName(pair.substr(eq + 1).c_str(), binding))
            {
                s_overrideSet[button] = true;
                s_override[button] = binding;
            }
            else
            {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                            "NFS_GAMEPAD_MAPPING: unrecognised entry \"%s\"", pair.c_str());
            }
        }
        if (comma == std::string::npos)
            break;
        pos = comma + 1;
    }
}

/* Resolves the effective key for `button`, applying the parsed override (if
 * any) on top of the compiled-in default. */
static void resolveKey(SDL_GamepadButton button, SDL_Keycode defaultKey, SDL_Scancode defaultScancode,
                       SDL_Keycode& key, SDL_Scancode& scancode)
{
    static bool s_parsed = (parseGamepadMapping(), true);
    NFS2_USE(s_parsed);
    if (s_overrideSet[button])
    {
        key = s_override[button].key;
        scancode = s_override[button].scancode;
    }
    else
    {
        key = defaultKey;
        scancode = defaultScancode;
    }
}

static void pollMenuGamepad()
{
    ensurePadOpen();
    if (!s_padDevice)
        return;

    /* key2/scancode2 is SDLK_UNKNOWN when a button only ever sends one key.
     * SOUTH is the one exception: it has to keep confirming menus (Return)
     * and also accelerate once a race is running, and those two states are
     * mutually exclusive in the game itself, so sending both keys on every
     * press/release is safe -- nothing ever reads both at once. */
    struct ButtonMap
    {
        SDL_GamepadButton button;
        SDL_Keycode  key;      SDL_Scancode scancode;
        SDL_Keycode  key2;     SDL_Scancode scancode2;
    };
    /* Back is View on Xbox, Create on DualSense and Minus on a Switch pad --
     * SDL's mapping database makes this one entry cover every controller,
     * and is what actually fixes "Escape is not emulated by the joystick".
     * Circle (East) used to be Escape too, which left nothing on the button
     * PlayStation players instinctively reach for to change camera; Back and
     * Start alone are still enough ways to reach Escape. */
    static const ButtonMap s_systemButtons[] =
    {
        { SDL_GAMEPAD_BUTTON_BACK,           SDLK_ESCAPE, SDL_SCANCODE_ESCAPE, SDLK_UNKNOWN, SDL_SCANCODE_UNKNOWN },
        { SDL_GAMEPAD_BUTTON_START,          SDLK_ESCAPE, SDL_SCANCODE_ESCAPE, SDLK_UNKNOWN, SDL_SCANCODE_UNKNOWN },
        /* SOUTH used to also send Up (for Accelerate) on every press, but Up
         * is exactly what list-style menus (Camera/Graphics/HUD, all
         * buttontype=blist) use to move the selection -- so confirming would
         * also silently move to the next item first and confirm THAT one
         * instead.  Reverted to Return only; Accelerate stays on D-pad/stick
         * Up until there is a way to send it without going through the same
         * key path menu navigation uses. */
        { SDL_GAMEPAD_BUTTON_SOUTH,          SDLK_RETURN, SDL_SCANCODE_RETURN, SDLK_UNKNOWN, SDL_SCANCODE_UNKNOWN }, // Confirm
        { SDL_GAMEPAD_BUTTON_EAST,           SDLK_C,      SDL_SCANCODE_C,      SDLK_UNKNOWN, SDL_SCANCODE_UNKNOWN }, // Camera view
        { SDL_GAMEPAD_BUTTON_WEST,           SDLK_SPACE,  SDL_SCANCODE_SPACE,  SDLK_UNKNOWN, SDL_SCANCODE_UNKNOWN }, // Handbrake
        { SDL_GAMEPAD_BUTTON_NORTH,          SDLK_C,      SDL_SCANCODE_C,      SDLK_UNKNOWN, SDL_SCANCODE_UNKNOWN }, // Camera view (same as Circle -- unconfirmed which key NFS3 actually binds this to, so both try it)
        { SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,  SDLK_B,      SDL_SCANCODE_B,      SDLK_UNKNOWN, SDL_SCANCODE_UNKNOWN }, // Look behind
        { SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, SDLK_H,      SDL_SCANCODE_H,      SDLK_UNKNOWN, SDL_SCANCODE_UNKNOWN }, // Horn
        // No default synthetic key: preserve existing in-game DirectInput bindings.
        { SDL_GAMEPAD_BUTTON_LEFT_STICK,     SDLK_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDLK_UNKNOWN, SDL_SCANCODE_UNKNOWN },
        { SDL_GAMEPAD_BUTTON_RIGHT_STICK,    SDLK_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDLK_UNKNOWN, SDL_SCANCODE_UNKNOWN },
    };
    static bool s_buttonDown[SDL_arraysize(s_systemButtons)];
    for (size_t i = 0; i < SDL_arraysize(s_systemButtons); ++i)
    {
        const bool down = SDL_GetGamepadButton(s_padDevice, s_systemButtons[i].button);
        if (down != s_buttonDown[i])
        {
            /* NFS_GAMEPAD_MAPPING (Android launcher Controls screen) can
             * replace the table's compiled-in key for this button; key2 has
             * no launcher equivalent and always stays tied to the default. */
            SDL_Keycode key; SDL_Scancode scancode;
            resolveKey(s_systemButtons[i].button, s_systemButtons[i].key, s_systemButtons[i].scancode, key, scancode);
            sendKey(down, key, scancode);
            if (s_systemButtons[i].key2 != SDLK_UNKNOWN)
            {
                sendKey(down, s_systemButtons[i].key2, s_systemButtons[i].scancode2);
            }
            s_buttonDown[i] = down;
        }
    }

    /* D-pad and the left stick drive menu navigation, and in-race steering
     * when nothing has bound the pad as a DirectInput device.  Skipped once
     * the game *is* reading this pad as DirectInput, so a race does not see
     * both a real analog axis and a synthetic arrow key for the same push. */
    if (Gamepad::isPolledByGame())
        return;

    struct DpadMap { SDL_GamepadButton button; SDL_Keycode key; SDL_Scancode scancode; };
    static const DpadMap s_dpad[] =
    {
        { SDL_GAMEPAD_BUTTON_DPAD_UP,    SDLK_UP,    SDL_SCANCODE_UP    },
        { SDL_GAMEPAD_BUTTON_DPAD_DOWN,  SDLK_DOWN,  SDL_SCANCODE_DOWN  },
        { SDL_GAMEPAD_BUTTON_DPAD_LEFT,  SDLK_LEFT,  SDL_SCANCODE_LEFT  },
        { SDL_GAMEPAD_BUTTON_DPAD_RIGHT, SDLK_RIGHT, SDL_SCANCODE_RIGHT },
    };
    static bool s_dpadDown[SDL_arraysize(s_dpad)];
    for (size_t i = 0; i < SDL_arraysize(s_dpad); ++i)
    {
        const bool down = SDL_GetGamepadButton(s_padDevice, s_dpad[i].button);
        if (down != s_dpadDown[i])
        {
            SDL_Keycode key; SDL_Scancode scancode;
            resolveKey(s_dpad[i].button, s_dpad[i].key, s_dpad[i].scancode, key, scancode);
            sendKey(down, key, scancode);
            s_dpadDown[i] = down;
        }
    }

    static int s_stickX = 0, s_stickY = 0;
    updateAxisAsKeys(SDL_GetGamepadAxis(s_padDevice, SDL_GAMEPAD_AXIS_LEFTX), s_stickX,
                      SDLK_LEFT, SDL_SCANCODE_LEFT, SDLK_RIGHT, SDL_SCANCODE_RIGHT);
    updateAxisAsKeys(SDL_GetGamepadAxis(s_padDevice, SDL_GAMEPAD_AXIS_LEFTY), s_stickY,
                      SDLK_UP, SDL_SCANCODE_UP, SDLK_DOWN, SDL_SCANCODE_DOWN);
}

void Gamepad::updateKeys()
{
    pollMenuGamepad();
    /* s_kbPoll is reset to 3 by Input::poll() every time the game actually
     * reads this pad as a DirectInput device (idirectinputdevice.cpp).  It
     * has to be counted back down here, once per frame, or isPolledByGame()
     * would see the very first DirectInput read at boot, latch permanently
     * true, and disable D-pad/stick menu navigation for good -- which is
     * exactly what happened before this line existed. */
    if (s_kbPoll != 0)
        --s_kbPoll;
}

x86::reg32 Input::getButtonCount() const
{
    return 0;
}

x86::reg32 Input::getAxesCount() const
{
    return 0;
}

/* ----------------------------------------------------------------------
 * Mouse.  Fed entirely by an SDL event watch rather than SDL_GetRelativeMouseState():
 * a watch runs synchronously inside SDL_PushEvent regardless of which thread
 * generated the event and regardless of whether anything is polling
 * SDL_PollEvent/WaitEvent right now, so it can back the buffered
 * (GetDeviceData) read path with real per-event ordering, not just an
 * immediate-mode accumulator.  getState() (GetDeviceState) is served from the
 * same accumulator the watch fills, so both read paths agree with each
 * other.  Installed lazily from mouseImpl()'s first call -- always after
 * SDL_Init(), since nothing reaches Mouse::getState()/drainBuffer() before
 * the game creates a DirectInput mouse device from guest code. */
namespace
{

bool SDLCALL mouseEventWatch(void* userdata, SDL_Event* event);

struct MouseImpl
{
    SDL_Mutex*  mutex;
    float       accumDx, accumDy, accumDz;
    x86::reg32  buttons;

    static const x86::reg32 kBufferCapacity = 64;
    Mouse::BufferedEvent buffer[kBufferCapacity];
    x86::reg32  bufferHead, bufferCount;

    MouseImpl()
        :   mutex(SDL_CreateMutex())
        ,   accumDx(0.f), accumDy(0.f), accumDz(0.f)
        ,   buttons(0)
        ,   bufferHead(0), bufferCount(0)
    {
        /* Registered exactly once, thanks to mouseImpl()'s function-local
         * static -- see the comment above this namespace for why a watch
         * instead of SDL_GetRelativeMouseState(). */
        SDL_AddEventWatch(mouseEventWatch, nullptr);
    }

    /* Caller holds mutex.  Drops the oldest entry once full -- DirectInput
     * would report a buffer-overflow flag instead, but nothing here reads
     * that flag back. */
    void pushEvent(x86::reg32 offset, x86::sreg32 value)
    {
        const x86::reg32 idx = (bufferHead + bufferCount) % kBufferCapacity;
        if (bufferCount == kBufferCapacity)
            bufferHead = (bufferHead + 1) % kBufferCapacity;
        else
            ++bufferCount;
        buffer[idx].offset = offset;
        buffer[idx].value  = value;
    }
};

MouseImpl& mouseImpl()
{
    static MouseImpl s_impl;
    return s_impl;
}

bool SDLCALL mouseEventWatch(void* /*userdata*/, SDL_Event* event)
{
    MouseImpl& m = mouseImpl();
    switch (event->type)
    {
    case SDL_EVENT_MOUSE_MOTION:
        SDL_LockMutex(m.mutex);
        m.accumDx += event->motion.xrel;
        m.accumDy += event->motion.yrel;
        if (event->motion.xrel != 0.f)
            m.pushEvent(DIMOFS_X, x86::sreg32(event->motion.xrel));
        if (event->motion.yrel != 0.f)
            m.pushEvent(DIMOFS_Y, x86::sreg32(event->motion.yrel));
        SDL_UnlockMutex(m.mutex);
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
    {
        /* Only the first 4 buttons exist in the classic DIMOUSESTATE
         * layout this port hardcodes (see DIMOFS_* in gamepad.h). */
        int bit = -1;
        switch (event->button.button)
        {
        case SDL_BUTTON_LEFT:   bit = 0; break;
        case SDL_BUTTON_RIGHT:  bit = 1; break;
        case SDL_BUTTON_MIDDLE: bit = 2; break;
        case SDL_BUTTON_X1:     bit = 3; break;
        default: break;
        }
        if (bit >= 0)
        {
            const bool down = event->type == SDL_EVENT_MOUSE_BUTTON_DOWN;
            SDL_LockMutex(m.mutex);
            if (down) m.buttons |= (x86::reg32(1) << bit);
            else      m.buttons &= ~(x86::reg32(1) << bit);
            m.pushEvent(DIMOFS_BUTTON0 + bit, down ? 0x80 : 0x00);
            SDL_UnlockMutex(m.mutex);
        }
        break;
    }
    case SDL_EVENT_MOUSE_WHEEL:
        SDL_LockMutex(m.mutex);
        m.accumDz += event->wheel.y * 120.f;
        m.pushEvent(DIMOFS_Z, x86::sreg32(event->wheel.y * 120.f));
        SDL_UnlockMutex(m.mutex);
        break;
    default:
        break;
    }
    /* Let the event continue on to the normal queue -- window.cpp's message
     * pump still sees (and currently ignores) it too. */
    return true;
}

}

Mouse::MouseState Mouse::getState()
{
    /* mouseImpl() installs the event watch on its first call ever (from
     * either this function or drainBuffer()) -- motion before the game asks
     * for mouse state the first time is simply never captured, which is
     * fine, since nothing would have consumed it anyway. */
    MouseImpl& m = mouseImpl();

    MouseState result;
    SDL_LockMutex(m.mutex);
    result.dx = x86::sreg32(m.accumDx);
    result.dy = x86::sreg32(m.accumDy);
    result.dz = x86::sreg32(m.accumDz);
    /* Keep the fractional remainder so slow, sub-pixel motion accumulates
     * across several reads instead of being truncated away every time. */
    m.accumDx -= float(result.dx);
    m.accumDy -= float(result.dy);
    m.accumDz -= float(result.dz);
    result.buttons = m.buttons;
    SDL_UnlockMutex(m.mutex);
    return result;
}

x86::reg32 Mouse::drainBuffer(BufferedEvent* out, x86::reg32 max)
{
    MouseImpl& m = mouseImpl();
    SDL_LockMutex(m.mutex);
    const x86::reg32 n = m.bufferCount < max ? m.bufferCount : max;
    for (x86::reg32 i = 0; i < n; ++i)
        out[i] = m.buffer[(m.bufferHead + i) % MouseImpl::kBufferCapacity];
    m.bufferHead = (m.bufferHead + n) % MouseImpl::kBufferCapacity;
    m.bufferCount -= n;
    SDL_UnlockMutex(m.mutex);
    return n;
}

x86::reg32 Mouse::getButtonCount() const
{
    return 4;
}

x86::reg32 Mouse::getAxesCount() const
{
    return 3;
}

Gamepad::Gamepad(x86::reg32 gamepadIndex)
    :   m_joystick(nullptr)
{
    int count = 0;
    SDL_JoystickID *ids = SDL_GetJoysticks(&count);
    if (ids && (int)gamepadIndex < count)
        m_joystick = SDL_OpenJoystick(ids[gamepadIndex]);
    SDL_free(ids);
    SDL_SetJoystickEventsEnabled(false);
    if (!s_gp1)
        s_gp1 = this;
}

Gamepad::~Gamepad()
{
    if (s_gp1)
        s_gp1 = nullptr;
    SDL_CloseJoystick(m_joystick);
}

x86::reg32 Gamepad::getCount()
{
    int count = 0;
    SDL_JoystickID *ids = SDL_GetJoysticks(&count);
    SDL_free(ids);
    return (x86::reg32)count;
}

x86::reg32 Gamepad::getButtonCount() const
{
    return SDL_GetNumJoystickButtons(m_joystick);
}

x86::reg32 Gamepad::getAxesCount() const
{
    return SDL_GetNumJoystickAxes(m_joystick);
}

GamepadState Gamepad::getState() const
{
    GamepadState result;
    memset(&result, 0, sizeof(result));
    for (x86::reg32 button = 0; button < getButtonCount(); ++button)
    {
        result.buttons |= (SDL_GetJoystickButton(m_joystick, button) ? 1 : 0) << button;
    }
    for (x86::reg32 axis = 0; axis < getAxesCount(); ++axis)
    {
        result.axes[axis] = SDL_GetJoystickAxis(m_joystick, axis);
    }
    const int hatCount = SDL_GetNumJoystickHats(m_joystick);
    for (int hat = 0; hat < hatCount && hat < 10; ++hat)
    {
        result.hats[hat] = SDL_GetJoystickHat(m_joystick, hat);
    }
    return result;
}


}

