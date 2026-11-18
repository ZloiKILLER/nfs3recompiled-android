#ifndef NFS3HP_NATIVE_CONTROLS_INPUT_H
#define NFS3HP_NATIVE_CONTROLS_INPUT_H

#include "native_guest.h"

/* native_controls_input.cpp's typed functions that other files call as C++ (guest::direct), and how
 * the game's code calls each by address.  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one.
 * A joystick's number 16 is all of them, 19 every device. */
namespace nfs3hp
{
namespace controls
{

/* An axis's dead zone or saturation in percent, and where DirectInput
 * answered, the remainder of its hundredths. */
struct Zone
{
    x86::reg32 percent;
    std::optional<x86::reg32> remainder;
};

// The game's 0x434fd0: DirectInput stopped: the mouse let go and released, the keyboard, each
// joystick, then DirectInput itself.
bool stopInput(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4352d0: the pointer's bounds, and the pointer put back inside them, where there is a
// mouse.
bool boundPointer(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 left, x86::reg32 top, x86::reg32 right,
                  x86::reg32 bottom);
// The game's 0x435310: the mouse read and handed out to [buttonsAt], [xAt], [yAt] and [wheelAt]; eax
// as the code leaves it.
std::optional<x86::reg32> mouseRead(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buttonsAt, x86::reg32 xAt,
                                    x86::reg32 yAt, x86::reg32 wheelAt);
// The game's 0x4350a0: the mouse opened, DirectInput started first if it is not: 1, or 0 when it fails.
std::optional<x86::reg32> openMouseCore(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x434f20: DirectInput started, where it is not, and the mouse opened.
bool startInputCore(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x435600: the joysticks, looked for once, and acquired: how many there are.
std::optional<x86::reg32> lookForJoysticks(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x435690: joystick `joystick` described, where it is not yet: its record, or 0.
std::optional<x86::reg32> describe(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick);
// The game's 0x4358f0: joystick `joystick` read into the DIJOYSTATE at `state`: `state`, or 0.
std::optional<x86::reg32> readState(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick, x86::reg32 state);
// The game's 0x435b40: joystick `joystick`'s axis `axis`'s dead zone (`which` 0) or saturation set to
// `percent`.
std::optional<Zone> setZone(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick, x86::reg32 axis,
                            x86::reg32 which, x86::reg32 percent);
// setZone for a caller that reads what the game's code leaves in eax: the percent, or -1.
inline std::optional<x86::reg32> zoneSet(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick,
                                         x86::reg32 axis, x86::reg32 which, x86::reg32 percent)
{
    const std::optional<Zone> zone = setZone(app, cpu, joystick, axis, which, percent);
    if (!zone)
        return std::nullopt;
    return zone->percent;
}
// The game's 0x435d20: joystick `joystick`'s effects stopped and its actuators on, where it has
// force feedback.
bool actuatorsOnFor(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick);
// The game's 0x435d90: joystick `joystick`'s actuators off, where it has force feedback.
bool actuatorsOffFor(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick);
// The game's 0x435df0: joystick `joystick`'s force-feedback gain set to `percent`.
bool gain(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick, x86::reg32 percent);
// The game's 0x435f10: an effect made on joystick `joystick` from the description at `description`:
// its number, or -1.
std::optional<x86::reg32> makeEffect(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick,
                                     x86::reg32 description);
// The game's 0x4361e0: effect `effect` (0 or more) started, once: whether it plays.
std::optional<x86::reg32> startOnce(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 word);
// What startOnce leaves of effect `effect`: whether its record says it plays.
bool effectStarted(win32::WinApplication* app, x86::reg32 effect);
// The game's 0x436230: effect `effect` (0 or more) stopped.
std::optional<x86::reg32> stopOne(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 word);
// The game's 0x436310: effect `effect` (0 or more) unloaded from its joystick: its HRESULT.
std::optional<x86::reg32> unloadOne(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 word);
// The game's 0x436330: effect `effect` (0 or more) changed where it is asked to, from the words at
// the addresses, and with `how` started or downloaded: whether anything changed.
std::optional<bool> changeEffect(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 effect, x86::reg32 how,
                                 x86::reg32 directionAt, x86::reg32 durationAt, x86::reg32 gainAt,
                                 x86::reg32 parametersAt, x86::reg32 triggerAt, x86::reg32 envelopeAt);

}  // namespace controls

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
