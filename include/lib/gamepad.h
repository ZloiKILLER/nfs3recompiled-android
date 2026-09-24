#ifndef LIB_GAMEPAD_H_
#define LIB_GAMEPAD_H_

#include <lib/winapp.h>
#include <SDL3/SDL.h>


namespace win32
{

enum InputType
{
    InputType_Keyboard,
    InputType_Mouse,
    InputType_Gamepad
};

class Input : public GenericResource
{
public:
    virtual x86::reg32 getButtonCount() const;
    virtual x86::reg32 getAxesCount() const;
    static void poll();
};

class Keyboard : public Input
{
};

/* Standard c_dfDIMouse object offsets (from the real dinput.h): the game's
 * SetDataFormat() call is not actually parsed (see idirectinputdevice.cpp),
 * but every DirectInput game of this era uses this exact layout for the
 * built-in mouse format, so it is safe to hardcode. */
enum
{
    DIMOFS_X       = 0,
    DIMOFS_Y       = 4,
    DIMOFS_Z       = 8,
    DIMOFS_BUTTON0 = 12,
    DIMOFS_BUTTON1 = 13,
    DIMOFS_BUTTON2 = 14,
    DIMOFS_BUTTON3 = 15,
};

/* Backs both DirectInput mouse read paths.  GetDeviceState() (immediate mode)
 * wants relative motion since the previous read; GetDeviceData() (buffered
 * mode -- confirmed to be what NFS3 actually calls for the mouse, see the
 * NFS2_ASSERT this used to hard-fail in idirectinputdevice.cpp) wants a
 * queue of discrete per-axis/button events with real ordering.  Both are fed
 * by the same SDL mouse event watch (installed lazily on first use, so it
 * only starts capturing once the game has actually created a Mouse device),
 * so a getState() call and the next few drainBuffer() entries agree with
 * each other instead of being two independent samples of the mouse. */
class Mouse : public Input
{
public:
    virtual x86::reg32 getButtonCount() const;
    virtual x86::reg32 getAxesCount() const;

    struct MouseState
    {
        x86::sreg32 dx, dy, dz;
        x86::reg32  buttons;
    };
    /* Consumes and resets the accumulated relative motion/wheel and reports
     * the buttons currently held. */
    static MouseState getState();

    struct BufferedEvent
    {
        x86::reg32  offset;   // DIMOFS_*
        x86::sreg32 value;
    };
    /* Drains up to `max` queued events (oldest first) into `out`, returns how
     * many were written.  Always destructive (DIGDD_PEEK is not honoured). */
    static x86::reg32 drainBuffer(BufferedEvent* out, x86::reg32 max);

    /* For a pointer that knows where it is rather than how far it moved -- a
     * finger on the screen.  Whoever owns that pointer works out the movement
     * from where the game keeps its own cursor and posts it here, so what
     * arrives is the same shape a mouse would have produced and both read
     * paths stay in step.  `button` is 0 to 3, as DIMOFS_BUTTON0 counts them. */
    static void movePointer(x86::sreg32 dx, x86::sreg32 dy);
    static void pressPointer(x86::reg32 button, bool down);
};

struct GamepadState
{
    x86::reg64  buttons;
    x86::sreg16 axes[10];
    x86::reg8   hats[10];
};

class Gamepad : public Input
{
public:
    /* How many devices the game is told about, always, from the first
     * enumeration onwards.  The game asks for the device list exactly once at
     * startup (confirmed on device: one EnumDevices call for the whole
     * session) and stores what it binds by slot number, so a device that only
     * appears when its hardware does could never be bound afterwards.  Two
     * fixed slots cover what the game can actually use on a phone: slot 0 is
     * player one, slot 1 is the second pad for split screen.  The game itself
     * has room for sixteen (its device table at 0x5f1fb8 is indexed 0..15). */
    static const x86::reg32 kSlotCount = 2;
    /* A slot is two axes and nothing else: X steers, Y carries both pedals --
     * up accelerates, down brakes -- the classic joystick layout the game binds
     * on its own.  A pad's buttons never reach the game as buttons: each one
     * sends a key, chosen per slot in the launcher, and the control profile the
     * launcher writes into config.dat binds those keys.  Slot 0 also carries
     * the touch overlay's steering and pedals, so the overlay and the first pad
     * are one player as far as the game is concerned. */
    static const x86::reg32 kAxisSteer = 0;
    static const x86::reg32 kAxisPedals = 1;

    Gamepad(x86::reg32 gamepadIndex);
    ~Gamepad();

    virtual x86::reg32 getButtonCount() const;
    virtual x86::reg32 getAxesCount() const;

    static x86::reg32 getCount();

    /* Which physical pad goes in each slot, as the Android input-device
     * descriptor the launcher resolved; an empty string leaves the slot
     * empty.  Called from the Android UI thread whenever a pad comes or goes
     * (NFS3Activity.nativeSetGamepadSlots) and applied on the next frame.
     * Until it has been called at all -- the desktop build never calls it --
     * slots fill in the order pads are found. */
    static void setSlotDescriptors(const char* first, const char* second);

    bool hasForceFeedback() const;
    /* The parts a device's force feedback is made of, beside the one level it
     * mixes down to: the jolts, and the two textures the game keeps going --
     * its sine, the engine, and its square, the road -- at full weight, each
     * with the rate it runs at.  Only an Android pad uses them: its motor
     * cannot vibrate gently, so GameHaptics plays the textures as taps. */
    struct RumbleDetail
    {
        float impact;
        float road;
        float roadHz;
        float engine;
        float engineHz;
        RumbleDetail() : impact(0), road(0), roadHz(0), engine(0), engineHz(0) {}
        bool operator==(const RumbleDetail& other) const
        {
            return impact == other.impact && road == other.road && roadHz == other.roadHz
                && engine == other.engine && engineHz == other.engineHz;
        }
        bool operator!=(const RumbleDetail& other) const { return !(*this == other); }
    };
    void rumble(float strength);
    void rumble(float strength, const RumbleDetail& detail);
    GamepadState getState() const;
    /* Once a frame, from the renderer: follows pads as they come and go, turns
     * their buttons into the keys the launcher assigned, and runs the
     * force-feedback mixer. */
    static void update();
    /* While set, a pad's buttons send no key but Escape, and keys they already
     * hold are let go.  Set from the game layer, once a frame, for the time the
     * pads' keys would work somebody else's car (nfs3hp_main.cpp). */
    static void muteGameKeys(bool muted);
    /** What is on the screen, which decides what a pad's buttons mean: while a
     *  race is being driven they are the racing set the launcher put on them,
     *  and everywhere else -- the menus, the pause screen, a replay being
     *  watched -- the lower face button confirms, the right one goes back, the
     *  D-pad moves the highlight and the rest are quiet.  The same two states
     *  the on-screen controls have layouts for, from the same signal, set from
     *  the game layer once a frame (nfs3hp_main.cpp). */
    enum class Context { Race, Menu };
    static void context(Context context);
    /* Whether the touch overlay's steering buttons are what steers player
     * one: true from the moment one is held, false once the first pad steers
     * the slot's axis itself (its stick, or a button bound to the axis), and
     * unchanged while nothing steers.  Kept up to date as the game reads the
     * first slot; the game layer gives those buttons the game's own keyboard
     * steering by it (nfs3hp_main.cpp). */
    static bool touchSteering();
    /* About every 40 ms while a race is drawn, from the game layer: how hard
     * player one's car is cornering, 0 to 1, for the phone's own vibration
     * (GameHaptics).  No pad plays it; the desktop build does nothing with it. */
    static void phoneTick(float turn);
private:
    /* Which physical joystick backs this slot is resolved on every read, not
     * captured here: SDL's device order shifts as pads come and go, and a
     * handle taken once at construction would either go stale or, worse,
     * silently start reporting a different pad. */
    SDL_Joystick* joystick() const;

    x86::reg32 m_slot;
};

}

#endif

