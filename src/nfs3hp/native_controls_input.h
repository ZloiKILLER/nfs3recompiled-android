#ifndef NFS3HP_NATIVE_CONTROLS_INPUT_H
#define NFS3HP_NATIVE_CONTROLS_INPUT_H

#include "native_guest.h"

/* native_controls_input.cpp's typed functions that other files call as C++ (guest::direct), and how
 * the game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
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
std::optional<bool> startOnce(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 effect);
// What startOnce leaves of effect `effect`: whether its record says it plays.
bool effectStarted(win32::WinApplication* app, x86::reg32 effect);
// The game's 0x436230: effect `effect` (0 or more) stopped.
bool stopOne(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 effect);
// The game's 0x436310: effect `effect` (0 or more) unloaded from its joystick: its HRESULT.
std::optional<x86::reg32> unloadOne(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 effect);
// The game's 0x436330: effect `effect` (0 or more) changed where it is asked to, from the words at
// the addresses, and with `how` started or downloaded: whether anything changed.
std::optional<bool> changeEffect(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 effect, x86::reg32 how,
                                 x86::reg32 directionAt, x86::reg32 durationAt, x86::reg32 gainAt,
                                 x86::reg32 parametersAt, x86::reg32 triggerAt, x86::reg32 envelopeAt);

}  // namespace controls

namespace guest
{

// Nothing handed.
template <>
struct ByAddress<controls::stopInput>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcom(app, cpu, Address, {});
    }
};

// Left in eax, top in edx, right in ebx, bottom in ecx.
template <>
struct ByAddress<controls::boundPointer>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 left, x86::reg32 top, x86::reg32 right,
                     x86::reg32 bottom)
    {
        return watcom(app, cpu, Address, {left, top, right, bottom});
    }
};

// The four addresses in eax, edx, ebx and ecx; eax back.
template <>
struct ByAddress<controls::mouseRead>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buttonsAt,
                                          x86::reg32 xAt, x86::reg32 yAt, x86::reg32 wheelAt)
    {
        return watcomEax(app, cpu, Address, {buttonsAt, xAt, yAt, wheelAt});
    }
};

// Nothing handed; the count back in eax.
template <>
struct ByAddress<controls::lookForJoysticks>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// The joystick in eax; its record back in eax.
template <>
struct ByAddress<controls::describe>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick)
    {
        return watcomEax(app, cpu, Address, {joystick});
    }
};

// The joystick in eax, the state's address in edx; eax back.
template <>
struct ByAddress<controls::readState>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick,
                                          x86::reg32 state)
    {
        return watcomEax(app, cpu, Address, {joystick, state});
    }
};

// The joystick in eax, the axis in edx, which in ebx, the percent in ecx; eax back.
template <>
struct ByAddress<controls::zoneSet>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick,
                                          x86::reg32 axis, x86::reg32 which, x86::reg32 percent)
    {
        return watcomEax(app, cpu, Address, {joystick, axis, which, percent});
    }
};

// The joystick in eax.
template <>
struct ByAddress<controls::actuatorsOnFor>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick)
    {
        return watcom(app, cpu, Address, {joystick});
    }
};

// The joystick in eax.
template <>
struct ByAddress<controls::actuatorsOffFor>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick)
    {
        return watcom(app, cpu, Address, {joystick});
    }
};

// The joystick in eax, the percent in edx.
template <>
struct ByAddress<controls::gain>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick, x86::reg32 percent)
    {
        return watcom(app, cpu, Address, {joystick, percent});
    }
};

// The joystick in eax, the description's address in edx; the effect's number back in eax.
template <>
struct ByAddress<controls::makeEffect>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 joystick,
                                          x86::reg32 description)
    {
        return watcomEax(app, cpu, Address, {joystick, description});
    }
};

// The effect in ax; whether it plays from its record.
template <>
struct ByAddress<controls::startOnce>
{
    template <x86::reg32 Address>
    static std::optional<bool> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 effect)
    {
        if (!watcom(app, cpu, Address, {effect}))
            return std::nullopt;
        return controls::effectStarted(app, effect);
    }
};

// The effect in ax.
template <>
struct ByAddress<controls::stopOne>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 effect)
    {
        return watcom(app, cpu, Address, {effect});
    }
};

// The effect in ax; the HRESULT back in eax.
template <>
struct ByAddress<controls::unloadOne>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 effect)
    {
        return watcomEax(app, cpu, Address, {effect});
    }
};

// The effect in ax, how in edx, directionAt in ebx, durationAt in ecx, the other four addresses
// pushed (stdcall, 4 words); eax 1 if anything changed, else 0.
template <>
struct ByAddress<controls::changeEffect>
{
    template <x86::reg32 Address>
    static std::optional<bool> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 effect, x86::reg32 how,
                                    x86::reg32 directionAt, x86::reg32 durationAt, x86::reg32 gainAt,
                                    x86::reg32 parametersAt, x86::reg32 triggerAt, x86::reg32 envelopeAt)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = effect;
        cpu.edx = how;
        cpu.ebx = directionAt;
        cpu.ecx = durationAt;
        if (!guest::call(app, cpu, Address, {gainAt, parametersAt, triggerAt, envelopeAt}))
            return std::nullopt;
        return cpu.eax != 0;
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
