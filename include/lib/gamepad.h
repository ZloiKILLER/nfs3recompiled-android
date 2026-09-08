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
    Gamepad(x86::reg32 gamepadIndex);
    ~Gamepad();

    virtual x86::reg32 getButtonCount() const;
    virtual x86::reg32 getAxesCount() const;

    static x86::reg32 getCount();

    bool hasForceFeedback() const;
    void markInputRead() const;
    void rumble(float strength);
    GamepadState getState() const;
    static void updateKeys();
    /* True while the game is actively reading this pad as a DirectInput
     * device (i.e. it has been bound as the steering input in Controls and
     * a race is running).  Gamepad::updateKeys() uses this to stop turning
     * D-pad/stick motion into arrow keys once the game is already reading
     * the same stick as a real analog axis, so the car is not driven by
     * both at once. */
    static bool isPolledByGame();
private:
    SDL_Joystick* m_joystick;
};

}

#endif

