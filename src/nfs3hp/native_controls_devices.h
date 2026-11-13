#ifndef NFS3HP_NATIVE_CONTROLS_DEVICES_H
#define NFS3HP_NATIVE_CONTROLS_DEVICES_H

#include "native_guest.h"

/* native_controls_devices.cpp's typed functions that other files call as C++ (guest::direct), and
 * how the game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call
 * the game's other functions by address on `cpu`, and is false or nullopt when the game stopped in
 * one. */
namespace nfs3hp
{
namespace controls
{

// The game's 0x4904d0: the players' control sets in `mode`: the two players' on one screen in mode
// 1, else the one player's.
x86::reg32 playersSets(x86::reg32 mode);
// playersSets as guest::direct calls a core, the application first.
inline x86::reg32 controlSets(win32::WinApplication*, x86::reg32 mode)
{
    return playersSets(mode);
}
// The game's 0x4904f0: every player's controls in `mode` as they start: whether it stopped at the
// keyboard's in mode 1.
std::optional<bool> startingSets(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mode);
// startingSets for a caller that reads what the game's code leaves in eax: the players' count, or
// where it stopped at the keyboard's, as the keyboard's copy left it.
inline std::optional<x86::reg32> defaultSets(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mode)
{
    const std::optional<bool> atKeyboard = startingSets(app, cpu, mode);
    if (!atKeyboard)
        return std::nullopt;
    if (*atKeyboard)
        return cpu.eax;
    return x86::reg32(mode == 1 ? 2 : 1);
}

}  // namespace controls

namespace guest
{

// The mode in eax; the sets' address back in eax.
template <>
struct ByAddress<controls::controlSets>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mode)
    {
        return watcomEax(app, cpu, Address, {mode}).value_or(0);
    }
};

// The mode in eax; eax back.
template <>
struct ByAddress<controls::defaultSets>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mode)
    {
        return watcomEax(app, cpu, Address, {mode});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
