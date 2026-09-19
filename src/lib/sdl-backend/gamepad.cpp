#include <lib/gamepad.h>
#include <winapi/dinput/idirectinputeffect.h>
#ifdef __ANDROID__
#include <jni.h>
#endif
#include <SDL3/SDL.h>
#include <array>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <utility>
#include <vector>


namespace win32
{

/* ----------------------------------------------------------------------
 * Slots.  A slot is a stable identity the game binds its controls to; the
 * physical joystick behind it comes and goes.  Slots remember an
 * SDL_JoystickID rather than a position in SDL's device array, because that
 * position is not stable: unplug the first of two pads and the second one
 * slides down to index 0.
 *
 * Hotplug works even though the Gamepad constructor turns SDL's joystick
 * events off: SDL's Android backend appends to its device list and bumps the
 * count *before* the event push it then skips, and the three-second device
 * scan (ANDROID_JoystickDetect) hangs off SDL_UpdateJoysticks, which
 * SDL_PumpEvents calls every frame regardless of whether joystick events are
 * enabled.  Only SDL_HINT_AUTO_UPDATE_JOYSTICKS could switch that off, and
 * nothing here touches it. */
struct Slot
{
    SDL_JoystickID id = 0;
    SDL_Joystick*  handle = nullptr;
    /* The same pad through SDL's gamepad layer, whenever SDL has a mapping
     * for it -- on Android that is every pad, since SDL builds one from what
     * the device reports. */
    SDL_Gamepad*   gamepad = nullptr;
};
static Slot s_slots[Gamepad::kSlotCount];

/* Which physical pad belongs in which slot is the launcher's decision, not
 * SDL's: the player picks them in Controls -> Gamepad, and NFS3Activity
 * resolves that choice against the pads Android has attached and hands the
 * result down here whenever a pad comes or goes.  What arrives is each pad's
 * Android input-device descriptor -- the one identifier Android keeps across
 * reconnects and reboots, and which differs between two pads of one model.
 *
 * SDL never exposes that descriptor, but it keeps a fingerprint of it: the
 * Android joystick driver builds each GUID with the descriptor as the name
 * SDL_CreateJoystickGUID runs through SDL_crc16, into bytes 2-3, and nothing
 * afterwards touches those two bytes (the capability bits it adds go into
 * bytes 12-15).  So a pad is recognised by hashing the descriptor the same way
 * and comparing. */
static std::mutex  s_assignmentMutex;
static std::string s_assignment[Gamepad::kSlotCount];
static bool        s_assignmentKnown = false;
static bool        s_assignmentChanged = false;

void Gamepad::setSlotDescriptors(const char* first, const char* second)
{
    std::lock_guard<std::mutex> lock(s_assignmentMutex);
    s_assignment[0] = first ? first : "";
    s_assignment[1] = second ? second : "";
    s_assignmentKnown = true;
    s_assignmentChanged = true;
}

static bool assignmentChanged()
{
    std::lock_guard<std::mutex> lock(s_assignmentMutex);
    return s_assignmentChanged;
}

static bool joystickHasDescriptor(SDL_JoystickID id, const std::string& descriptor)
{
    if (descriptor.empty())
        return false;
    const SDL_GUID guid = SDL_GetJoystickGUIDForID(id);
    const Uint16 fingerprint = Uint16(guid.data[2] | (guid.data[3] << 8));
    return fingerprint == SDL_crc16(0, descriptor.data(), descriptor.size());
}

static void placeInSlot(x86::reg32 slot, SDL_JoystickID id)
{
    if (s_slots[slot].id == id)
        return;
    if (s_slots[slot].gamepad)
        SDL_CloseGamepad(s_slots[slot].gamepad);
    if (s_slots[slot].handle)
        SDL_CloseJoystick(s_slots[slot].handle);
    s_slots[slot] = Slot();
    if (!id)
        return;
    SDL_Joystick* handle = SDL_OpenJoystick(id);
    if (!handle)
        return;
    s_slots[slot].id = id;
    s_slots[slot].handle = handle;
    if (SDL_IsGamepad(id))
        s_slots[slot].gamepad = SDL_OpenGamepad(id);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "gamepad slot %u <- %s",
                unsigned(slot), SDL_GetJoystickName(handle));
}

static void refreshSlots()
{
    int count = 0;
    SDL_JoystickID* ids = SDL_GetJoysticks(&count);

    std::string wanted[Gamepad::kSlotCount];
    bool known;
    {
        std::lock_guard<std::mutex> lock(s_assignmentMutex);
        known = s_assignmentKnown;
        for (x86::reg32 slot = 0; slot < Gamepad::kSlotCount; ++slot)
            wanted[slot] = s_assignment[slot];
        s_assignmentChanged = false;
    }

    if (known)
    {
        /* An empty slot stays empty: the launcher had no pad for it, and
         * guessing one here would undo exactly the choice it made. */
        for (x86::reg32 slot = 0; slot < Gamepad::kSlotCount; ++slot)
        {
            SDL_JoystickID match = 0;
            for (int i = 0; i < count && !match; ++i)
                if (joystickHasDescriptor(ids[i], wanted[slot]))
                    match = ids[i];
            placeInSlot(slot, match);
        }
        SDL_free(ids);
        return;
    }

    /* Nobody has said which pad goes where -- the desktop build, where there
     * is no launcher -- so slots fill in the order pads were found. */
    for (x86::reg32 slot = 0; slot < Gamepad::kSlotCount; ++slot)
    {
        if (!s_slots[slot].id)
            continue;
        bool stillThere = false;
        for (int i = 0; i < count; ++i)
            if (ids[i] == s_slots[slot].id) { stillThere = true; break; }
        if (!stillThere)
            placeInSlot(slot, 0);
    }
    for (int i = 0; i < count; ++i)
    {
        bool held = false;
        for (x86::reg32 slot = 0; slot < Gamepad::kSlotCount; ++slot)
            if (s_slots[slot].id == ids[i]) { held = true; break; }
        for (x86::reg32 slot = 0; slot < Gamepad::kSlotCount && !held; ++slot)
            if (!s_slots[slot].id) { placeInSlot(slot, ids[i]); held = true; }
    }
    SDL_free(ids);
}

void Input::poll()
{
    SDL_UpdateJoysticks();
    refreshSlots();
}

SDL_Joystick* Gamepad::joystick() const
{
    return m_slot < kSlotCount ? s_slots[m_slot].handle : nullptr;
}

namespace
{
void updateButtonKeys();
}

void Gamepad::update()
{
    /* The only place slots are rescanned while the game is running.
     * IDirectInputDevice::Poll would be the natural home for it, but the game
     * never calls it -- a full traced session on device shows not one Poll --
     * whereas this runs once per frame from the renderer.  Once a second is
     * quick enough to notice a pad being plugged in and keeps SDL_GetJoysticks'
     * allocation out of the frame loop; an assignment sent from the launcher
     * side is applied on the very next frame instead of waiting it out. */
    static Uint64 s_lastSlotScan = 0;
    const Uint64 now = SDL_GetTicks();
    if (assignmentChanged() || now - s_lastSlotScan >= 1000)
    {
        s_lastSlotScan = now;
        refreshSlots();
    }
    updateButtonKeys();
    dinput::IDirectInputEffect::update();
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

void Mouse::movePointer(x86::sreg32 dx, x86::sreg32 dy)
{
    if (!dx && !dy)
        return;
    MouseImpl& m = mouseImpl();
    SDL_LockMutex(m.mutex);
    m.accumDx += float(dx);
    m.accumDy += float(dy);
    if (dx)
        m.pushEvent(DIMOFS_X, dx);
    if (dy)
        m.pushEvent(DIMOFS_Y, dy);
    SDL_UnlockMutex(m.mutex);
}

void Mouse::pressPointer(x86::reg32 button, bool down)
{
    if (button > 3)
        return;
    MouseImpl& m = mouseImpl();
    SDL_LockMutex(m.mutex);
    if (down) m.buttons |= (x86::reg32(1) << button);
    else      m.buttons &= ~(x86::reg32(1) << button);
    m.pushEvent(DIMOFS_BUTTON0 + button, down ? 0x80 : 0x00);
    SDL_UnlockMutex(m.mutex);
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

/* ----------------------------------------------------------------------
 * The touch overlay's half of slot 0: steering and the pedals.  The overlay
 * speaks to the game through keys, and the launcher owns which key each
 * on-screen control sends (Controls -> Touch -> Keys), handed down as
 * NFS_TOUCH_<ACTION>.  The control profile binds steering and the pedals to
 * the slot's axes rather than to keys, so what the overlay is doing with
 * those four is read back off the keyboard state and folded into the axes.
 * Its other controls are keys the profile binds as they are.  Resolved once --
 * the environment is written before the native thread starts and never
 * changes afterwards. */
enum TouchAxisAction
{
    TOUCH_STEER_LEFT, TOUCH_STEER_RIGHT, TOUCH_ACCELERATE, TOUCH_BRAKE, TOUCH_AXIS_ACTIONS
};

static const SDL_Scancode* touchAxisKeys()
{
    static const auto s_keys = []() {
        struct Action { const char* variable; SDL_Scancode fallback; };
        static const Action actions[TOUCH_AXIS_ACTIONS] =
        {
            { "NFS_TOUCH_STEER_LEFT",  SDL_SCANCODE_LEFT  },
            { "NFS_TOUCH_STEER_RIGHT", SDL_SCANCODE_RIGHT },
            { "NFS_TOUCH_ACCELERATE",  SDL_SCANCODE_UP    },
            { "NFS_TOUCH_BRAKE",       SDL_SCANCODE_DOWN  },
        };
        std::array<SDL_Scancode, TOUCH_AXIS_ACTIONS> resolved;
        for (int i = 0; i < TOUCH_AXIS_ACTIONS; ++i)
        {
            const char* name = SDL_getenv(actions[i].variable);
            resolved[i] = name && *name
                ? SDL_GetScancodeFromKey(SDL_GetKeyFromName(name), nullptr)
                : actions[i].fallback;
        }
        return resolved;
    }();
    return s_keys.data();
}

/* Whether the touch overlay's steering buttons are what steers player one, for
 * Gamepad::touchSteering(): set while one of them is held, cleared once the
 * pad steers the slot's axis itself, left alone while nothing steers -- so the
 * wheel a button let go of still comes back to the centre the same way. */
static bool s_touchSteering = false;
/* How far off the centre the pad has to steer to count, a quarter of the
 * range: a stick resting a little off-centre must not take steering back. */
static const int kPadSteerThreshold = 8192;

/* Larger magnitude wins rather than a sum, so a stick resting off-centre
 * cannot cancel a finger or a button asking for full lock, and no two sources
 * can push an axis past its range. */
static void blendAxis(GamepadState& state, x86::reg32 axis, int value)
{
    if (std::abs(value) > std::abs(int(state.axes[axis])))
        state.axes[axis] = x86::sreg16(value);
}

/* ----------------------------------------------------------------------
 * Buttons.  A slot has none as far as the game is concerned; each of a pad's
 * buttons sends a keyboard key instead, picked per slot in the launcher
 * (Controls -> Gamepad -> Buttons) and handed down as NFS_GAMEPAD<n>_<BUTTON>
 * -- player one's keys for the first pad, player two's for the second, the
 * keys the control profile in config.dat binds.  So a button means the same
 * in a race, in split screen and in the menus, with no mode to keep track of.
 * A value "axis:<action>" pushes the slot's own axis instead, for a player who
 * wants to steer or accelerate with a button; an empty value sends nothing.
 *
 * The keys go out as SDL events, not through SDL's keyboard state: window.cpp
 * turns them into the game's key messages exactly as it does a real
 * keyboard's, while the touch blend in getState() -- which reads that state --
 * can never take the second pad's D-pad for the first player's steering. */
namespace
{

struct ButtonBinding
{
    SDL_Keycode  key = SDLK_UNKNOWN;
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    int          axis = -1;         // Gamepad::kAxisSteer or kAxisPedals
    int          direction = 0;     // -1 or +1 along it
};

struct PadButton
{
    const char*       name;         // the <BUTTON> in NFS_GAMEPAD<n>_<BUTTON>
    SDL_GamepadButton button;
    /* Only for a variable that is not set at all -- a build without the
     * launcher.  The launcher's own defaults, GamepadButtons.java. */
    const char*       defaults[Gamepad::kSlotCount];
};

const PadButton s_padButtons[] =
{
    { "SOUTH",          SDL_GAMEPAD_BUTTON_SOUTH,          { "Space",  "D"      } },
    { "EAST",           SDL_GAMEPAD_BUTTON_EAST,           { "S",      "P"      } },
    { "WEST",           SDL_GAMEPAD_BUTTON_WEST,           { "R",      "X"      } },
    { "NORTH",          SDL_GAMEPAD_BUTTON_NORTH,          { "C",      "Q"      } },
    { "LEFT_SHOULDER",  SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,  { "Z",      "G"      } },
    { "RIGHT_SHOULDER", SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, { "A",      "F"      } },
    { "LEFT_STICK",     SDL_GAMEPAD_BUTTON_LEFT_STICK,     { "H",      "W"      } },
    { "RIGHT_STICK",    SDL_GAMEPAD_BUTTON_RIGHT_STICK,    { "L",      "Y"      } },
    { "BACK",           SDL_GAMEPAD_BUTTON_BACK,           { "Escape", "Escape" } },
    { "START",          SDL_GAMEPAD_BUTTON_START,          { "Return", "Return" } },
    { "DPAD_UP",        SDL_GAMEPAD_BUTTON_DPAD_UP,        { "Up",     "Up"     } },
    { "DPAD_DOWN",      SDL_GAMEPAD_BUTTON_DPAD_DOWN,      { "Down",   "Down"   } },
    { "DPAD_LEFT",      SDL_GAMEPAD_BUTTON_DPAD_LEFT,      { "Left",   "Left"   } },
    { "DPAD_RIGHT",     SDL_GAMEPAD_BUTTON_DPAD_RIGHT,     { "Right",  "Right"  } },
};
constexpr size_t kPadButtons = SDL_arraysize(s_padButtons);

ButtonBinding parseBinding(const char* value)
{
    ButtonBinding binding;
    if (!value || !*value)
        return binding;
    static const struct { const char* name; int axis; int direction; } s_axisActions[] =
    {
        { "axis:steer_left",  int(Gamepad::kAxisSteer),  -1 },
        { "axis:steer_right", int(Gamepad::kAxisSteer),  +1 },
        { "axis:accelerate",  int(Gamepad::kAxisPedals), -1 },
        { "axis:brake",       int(Gamepad::kAxisPedals), +1 },
    };
    for (const auto& action : s_axisActions)
    {
        if (SDL_strcasecmp(value, action.name) == 0)
        {
            binding.axis = action.axis;
            binding.direction = action.direction;
            return binding;
        }
    }
    binding.key = SDL_GetKeyFromName(value);
    if (binding.key != SDLK_UNKNOWN)
        binding.scancode = SDL_GetScancodeFromKey(binding.key, nullptr);
    if (binding.scancode == SDL_SCANCODE_UNKNOWN)
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "gamepad: no key called \"%s\"", value);
        binding.key = SDLK_UNKNOWN;
    }
    return binding;
}

const ButtonBinding& buttonBinding(x86::reg32 slot, size_t button)
{
    static const auto s_bindings = []() {
        std::array<std::array<ButtonBinding, kPadButtons>, Gamepad::kSlotCount> table;
        for (x86::reg32 slot = 0; slot < Gamepad::kSlotCount; ++slot)
        {
            std::string summary;
            for (size_t i = 0; i < kPadButtons; ++i)
            {
                char variable[64];
                SDL_snprintf(variable, sizeof(variable), "NFS_GAMEPAD%u_%s",
                             unsigned(slot) + 1, s_padButtons[i].name);
                // Unset means the default; set but empty means nothing.
                const char* value = SDL_getenv(variable);
                if (!value)
                    value = s_padButtons[i].defaults[slot];
                table[slot][i] = parseBinding(value);
                summary += ' ';
                summary += s_padButtons[i].name;
                summary += '=';
                summary += value;
            }
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "gamepad slot %u buttons:%s",
                        unsigned(slot), summary.c_str());
        }
        return table;
    }();
    return s_bindings[slot][button];
}

/* How many held buttons, across both pads, are keeping each key down.  Two
 * pads can share a key -- both D-pads send the arrows -- and the key must stay
 * down until the last of them lets go. */
Uint8 s_keyHolders[SDL_SCANCODE_COUNT];

void sendKey(const ButtonBinding& binding, bool down)
{
    if (binding.scancode == SDL_SCANCODE_UNKNOWN)
        return;
    Uint8& holders = s_keyHolders[binding.scancode];
    if (down ? holders++ != 0 : (holders == 0 || --holders != 0))
        return;
    SDL_Event event;
    SDL_zero(event);
    event.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    event.key.timestamp = SDL_GetTicksNS();
    event.key.scancode = binding.scancode;
    event.key.key = binding.key;
    event.key.down = down;
    SDL_PushEvent(&event);
}

struct SlotButtons
{
    SDL_JoystickID pad = 0;
    bool           held[kPadButtons] = {};
    bool           sent[kPadButtons] = {};    // its key went down and has not come up
};
SlotButtons s_slotButtons[Gamepad::kSlotCount];

std::atomic<bool> s_gameKeysMuted{false};

/* A key a muted pad keeps quiet: all of them but Escape, which the game never
 * binds to a car -- it is the pause menu, and a pad keeps its way into that. */
bool muteable(const ButtonBinding& binding)
{
    return binding.scancode != SDL_SCANCODE_UNKNOWN && binding.scancode != SDL_SCANCODE_ESCAPE;
}

void updateButtonKeys()
{
    // The first frame resolves and logs every binding, before anything is pressed.
    buttonBinding(0, 0);
    const bool muted = s_gameKeysMuted.load(std::memory_order_relaxed);
    for (x86::reg32 slot = 0; slot < Gamepad::kSlotCount; ++slot)
    {
        SlotButtons& buttons = s_slotButtons[slot];
        const Slot& source = s_slots[slot];
        /* A pad that leaves its slot -- unplugged, or handed to the other
         * player -- lets go of whatever it held, or the key would stay down. */
        if (buttons.pad != source.id)
        {
            for (size_t i = 0; i < kPadButtons; ++i)
            {
                if (buttons.sent[i])
                    sendKey(buttonBinding(slot, i), false);
                buttons.held[i] = buttons.sent[i] = false;
            }
            buttons.pad = source.id;
        }
        if (!source.gamepad)
            continue;
        for (size_t i = 0; i < kPadButtons; ++i)
        {
            const ButtonBinding& binding = buttonBinding(slot, i);
            if (muted && buttons.sent[i] && muteable(binding))
            {
                sendKey(binding, false);
                buttons.sent[i] = false;
            }
            const bool down = SDL_GetGamepadButton(source.gamepad, s_padButtons[i].button);
            if (down == buttons.held[i])
                continue;
            buttons.held[i] = down;
            if (down && !(muted && muteable(binding)))
            {
                sendKey(binding, true);
                buttons.sent[i] = true;
            }
            else if (!down && buttons.sent[i])
            {
                sendKey(binding, false);
                buttons.sent[i] = false;
            }
        }
    }
}

}

void Gamepad::muteGameKeys(bool muted)
{
    s_gameKeysMuted.store(muted, std::memory_order_relaxed);
}

Gamepad::Gamepad(x86::reg32 gamepadIndex)
    :   m_slot(gamepadIndex)
{
    refreshSlots();
    SDL_SetJoystickEventsEnabled(false);
}

Gamepad::~Gamepad()
{
    dinput::IDirectInputEffect::detach(this);
    /* The slot outlives the device object: the joystick handle belongs to the
     * slot table, which survives the game releasing and recreating its
     * DirectInput devices. */
}

x86::reg32 Gamepad::getCount()
{
    return kSlotCount;
}

/* The shape a slot reports never changes, whatever is or is not plugged in.
 * The game keeps a description of every device in config.dat and compares it
 * with what it finds at each start; on any difference it throws its control
 * bindings away and generates new ones.  So a slot is always the same two
 * axes and no buttons -- which is also exactly what the launcher's control
 * profile describes when it writes that file (ControlProfile.java). */
x86::reg32 Gamepad::getButtonCount() const
{
    return 0;
}

x86::reg32 Gamepad::getAxesCount() const
{
    return 2;
}

bool Gamepad::touchSteering()
{
    return s_touchSteering;
}

GamepadState Gamepad::getState() const
{
    GamepadState result;
    memset(&result, 0, sizeof(result));

    /* A pad is read by polling its live state.  SDL's joystick and gamepad
     * *events* are no use here: the constructor turns joystick events off
     * (the game enumerates its DirectInput devices before a frame is drawn),
     * and SDL3 derives gamepad events from joystick events, so neither ever
     * arrives -- but the state queried below stays current regardless. */
    const Slot* slot = m_slot < kSlotCount ? &s_slots[m_slot] : nullptr;
    if (slot && slot->gamepad)
    {
        result.axes[kAxisSteer] = SDL_GetGamepadAxis(slot->gamepad, SDL_GAMEPAD_AXIS_LEFTX);
        /* Both triggers share one axis, the way combined pedals did on wheels
         * of the time: the right trigger pulls it up, towards accelerate, the
         * left one down, towards brake.  SDL's standard layout rests a trigger
         * at 0 and takes it to 32767, so the difference always fits -- and
         * pressing both cancels out, as those pedals did. */
        result.axes[kAxisPedals] = x86::sreg16(
              int(SDL_GetGamepadAxis(slot->gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER))
            - int(SDL_GetGamepadAxis(slot->gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)));
        for (size_t i = 0; i < kPadButtons; ++i)
        {
            const ButtonBinding& binding = buttonBinding(m_slot, i);
            if (binding.axis >= 0 && SDL_GetGamepadButton(slot->gamepad, s_padButtons[i].button))
                blendAxis(result, x86::reg32(binding.axis), binding.direction * 32767);
        }
    }
    else if (SDL_Joystick* pad = joystick())
    {
        /* A device SDL has no gamepad mapping for: its first two axes, where
         * a plain joystick keeps them. */
        const int axes = SDL_GetNumJoystickAxes(pad);
        if (axes > 0)
            result.axes[kAxisSteer] = SDL_GetJoystickAxis(pad, 0);
        if (axes > 1)
            result.axes[kAxisPedals] = SDL_GetJoystickAxis(pad, 1);
    }

    if (m_slot == 0)
    {
        const bool* keys = SDL_GetKeyboardState(nullptr);
        const SDL_Scancode* touch = touchAxisKeys();
        auto held = [keys, touch](int action) {
            return touch[action] != SDL_SCANCODE_UNKNOWN && keys[touch[action]];
        };
        const int touchSteer = int(held(TOUCH_STEER_RIGHT)) - int(held(TOUCH_STEER_LEFT));
        if (touchSteer != 0)
            s_touchSteering = true;
        else if (std::abs(int(result.axes[kAxisSteer])) > kPadSteerThreshold)
            s_touchSteering = false;
        blendAxis(result, kAxisSteer, 32767 * touchSteer);
        blendAxis(result, kAxisPedals,
                  32767 * (int(held(TOUCH_BRAKE)) - int(held(TOUCH_ACCELERATE))));
    }
    return result;
}


}


#ifdef __ANDROID__
namespace {
// SDL supplies an attached JNIEnv on every calling thread. No global Activity refs.
void androidForceFeedback(x86::reg32 slot,float strength,const win32::Gamepad::RumbleDetail& detail) {
    JNIEnv* env=static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
    jobject activity=static_cast<jobject>(SDL_GetAndroidActivity());
    if(!env||!activity)return;
    jclass cls=env->GetObjectClass(activity);
    jmethodID method=env->GetMethodID(cls,"onForceFeedback","(IFFFFFF)V");
    if(method)env->CallVoidMethod(activity,method,jint(slot),jfloat(strength),jfloat(detail.impact),
                                  jfloat(detail.road),jfloat(detail.roadHz),jfloat(detail.engine),jfloat(detail.engineHz));
    if(env->ExceptionCheck())env->ExceptionClear();
    env->DeleteLocalRef(cls);env->DeleteLocalRef(activity);
}
}

#endif
bool win32::Gamepad::hasForceFeedback() const {
#ifdef __ANDROID__
    /* Yes, for both slots, whatever is plugged in.  A pad's vibration belongs
     * to the game now -- Options -> Controllers -> Force Feedback, with Stick
     * Volume at zero as the off switch -- and the game asks this once, at
     * startup: an answer that followed the hardware would leave a pad
     * connected later without force feedback for the whole session.  The
     * phone's motor keeps its own switch and strength in the launcher, applied
     * in GameHaptics, and an effect with no motor behind it is dropped there. */
    return true;
#else
    SDL_Joystick* pad=joystick();
    return pad&&SDL_GetBooleanProperty(SDL_GetJoystickProperties(pad),SDL_PROP_JOYSTICK_CAP_RUMBLE_BOOLEAN,false);
#endif
}
void win32::Gamepad::rumble(float strength) {
    rumble(strength,RumbleDetail());
}
void win32::Gamepad::rumble(float strength,const RumbleDetail& detail) {
#ifdef __ANDROID__
    /* Which motors a slot drives is decided in GameHaptics, which owns both
     * the enable switches and the two strength sliders: slot 0 reaches the
     * phone and the first pad, slot 1 the second pad. */
    androidForceFeedback(m_slot,strength,detail);
#else
    (void)detail;
    SDL_Joystick* pad=joystick();
    if(pad)SDL_RumbleJoystick(pad,Uint16(strength*65535),Uint16(strength*65535),100);
#endif
}
void win32::Gamepad::phoneTick(float turn) {
#ifdef __ANDROID__
    JNIEnv* env=static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
    jobject activity=static_cast<jobject>(SDL_GetAndroidActivity());
    if(!env||!activity)return;
    jclass cls=env->GetObjectClass(activity);
    jmethodID method=env->GetMethodID(cls,"onPhoneTick","(F)V");
    if(method)env->CallVoidMethod(activity,method,jfloat(turn));
    if(env->ExceptionCheck())env->ExceptionClear();
    env->DeleteLocalRef(cls);env->DeleteLocalRef(activity);
#else
    (void)turn;
#endif
}
