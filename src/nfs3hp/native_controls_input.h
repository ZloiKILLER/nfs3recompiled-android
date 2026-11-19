#ifndef NFS3HP_NATIVE_CONTROLS_INPUT_H
#define NFS3HP_NATIVE_CONTROLS_INPUT_H

#include "native_guest.h"

/* native_controls_input.cpp's typed functions that other files call as C++ (guest::direct), and how
 * the game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is false or nullopt when the game stopped in one.
 * A joystick's number 16 is all of them, 19 every device. */
namespace nfs3hp
{
namespace controls
{

/* An axis's dead zone or saturation in percent, and where DirectInput
 * answered, the remainder of its hundredths. */
struct Zone
{
    nfs::u32 percent;
    std::optional<nfs::u32> remainder;
};

// The game's 0x434fd0: DirectInput stopped: the mouse let go and released, the keyboard, each
// joystick, then DirectInput itself.
bool stopInput(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4352d0: the pointer's bounds, and the pointer put back inside them, where there is a
// mouse.
bool boundPointer(nfs::App* app, nfs::ThreadState& ts, nfs::u32 left, nfs::u32 top, nfs::u32 right,
                  nfs::u32 bottom);
// The game's 0x435310: the mouse read and handed out to [buttonsAt], [xAt], [yAt] and [wheelAt]; eax
// as the code leaves it.
std::optional<nfs::u32> mouseRead(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buttonsAt, nfs::u32 xAt,
                                    nfs::u32 yAt, nfs::u32 wheelAt);
// The game's 0x4350a0: the mouse opened, DirectInput started first if it is not: 1, or 0 when it fails.
std::optional<nfs::u32> openMouseCore(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x434f20: DirectInput started, where it is not, and the mouse opened.
bool startInputCore(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x435600: the joysticks, looked for once, and acquired: how many there are.
std::optional<nfs::u32> lookForJoysticks(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x435690: joystick `joystick` described, where it is not yet: its record, or 0.
std::optional<nfs::u32> describe(nfs::App* app, nfs::ThreadState& ts, nfs::u32 joystick);
// The game's 0x4358f0: joystick `joystick` read into the DIJOYSTATE at `state`: `state`, or 0.
std::optional<nfs::u32> readState(nfs::App* app, nfs::ThreadState& ts, nfs::u32 joystick, nfs::u32 state);
// The game's 0x435b40: joystick `joystick`'s axis `axis`'s dead zone (`which` 0) or saturation set to
// `percent`.
std::optional<Zone> setZone(nfs::App* app, nfs::ThreadState& ts, nfs::u32 joystick, nfs::u32 axis,
                            nfs::u32 which, nfs::u32 percent);
// setZone for a caller that reads what the game's code leaves in eax: the percent, or -1.
inline std::optional<nfs::u32> zoneSet(nfs::App* app, nfs::ThreadState& ts, nfs::u32 joystick,
                                         nfs::u32 axis, nfs::u32 which, nfs::u32 percent)
{
    const std::optional<Zone> zone = setZone(app, ts, joystick, axis, which, percent);
    if (!zone)
        return std::nullopt;
    return zone->percent;
}
// The game's 0x435d20: joystick `joystick`'s effects stopped and its actuators on, where it has
// force feedback.
bool actuatorsOnFor(nfs::App* app, nfs::ThreadState& ts, nfs::u32 joystick);
// The game's 0x435d90: joystick `joystick`'s actuators off, where it has force feedback.
bool actuatorsOffFor(nfs::App* app, nfs::ThreadState& ts, nfs::u32 joystick);
// The game's 0x435df0: joystick `joystick`'s force-feedback gain set to `percent`.
bool gain(nfs::App* app, nfs::ThreadState& ts, nfs::u32 joystick, nfs::u32 percent);
// The game's 0x435f10: an effect made on joystick `joystick` from the description at `description`:
// its number, or -1.
std::optional<nfs::u32> makeEffect(nfs::App* app, nfs::ThreadState& ts, nfs::u32 joystick,
                                     nfs::u32 description);
// The game's 0x4361e0: effect `effect` (0 or more) started, once: whether it plays.
std::optional<nfs::u32> startOnce(nfs::App* app, nfs::ThreadState& ts, nfs::u32 word);
// What startOnce leaves of effect `effect`: whether its record says it plays.
bool effectStarted(nfs::App* app, nfs::u32 effect);
// The game's 0x436230: effect `effect` (0 or more) stopped.
std::optional<nfs::u32> stopOne(nfs::App* app, nfs::ThreadState& ts, nfs::u32 word);
// The game's 0x436310: effect `effect` (0 or more) unloaded from its joystick: its HRESULT.
std::optional<nfs::u32> unloadOne(nfs::App* app, nfs::ThreadState& ts, nfs::u32 word);
// The game's 0x436330: effect `effect` (0 or more) changed where it is asked to, from the words at
// the addresses, and with `how` started or downloaded: whether anything changed.
std::optional<bool> changeEffect(nfs::App* app, nfs::ThreadState& ts, nfs::u32 effect, nfs::u32 how,
                                 nfs::u32 directionAt, nfs::u32 durationAt, nfs::u32 gainAt,
                                 nfs::u32 parametersAt, nfs::u32 triggerAt, nfs::u32 envelopeAt);

}  // namespace controls

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
