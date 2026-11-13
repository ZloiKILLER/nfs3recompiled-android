#ifndef NFS3HP_PORT_SND_VOICES_H
#define NFS3HP_PORT_SND_VOICES_H

#include "native_guest.h"
#include "port.h"

/* port_snd_voices.cpp's typed cores that the menus call as C++ (guest::direct), and how the game's code calls
 * each by address (ByAddress below, for the checks' stand-ins); declared in port_snd.h as well, which holds the
 * game's addresses this one may not (a header's numbers count as named by every file including it:
 * tools/game_checks/generate.py).  A Result is nullopt when the game stopped in a call.  A core's `esp` is the
 * frame base its adapter hands it: the game's esp on entry, at the return address.  A caller through
 * guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the adapter hands
 * it, so the core's frame lies where the game's does; by address the call makes it. */
namespace nfs3hp
{
namespace port
{

// The game's 0x4e9420 (SNDautovol): the sound of `handle` to volume `target` over `ticks`: 0, -8 for no such sound,
// -10 before SNDinit; esp the frame base.
Result fadeSound(App* app, CPU& cpu, reg32 esp, reg32 handle, int32_t ticks, int32_t target);
// The game's 0x4e95fc (SNDvol): the sound of `handle` at volume `volume` (0..127); esp the frame base.
Result setVolume(App* app, CPU& cpu, reg32 esp, reg32 handle, int32_t volume);

}  // namespace port

namespace guest
{

// The handle in eax, the ticks in edx, the volume in ebx; eax back.
template <>
struct ByAddress<port::fadeSound>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 handle,
                             std::int32_t ticks, std::int32_t target)
    {
        return watcomEax(app, cpu, Address, {handle, x86::reg32(ticks), x86::reg32(target)});
    }
};

// The handle in eax, the volume in edx; eax back.
template <>
struct ByAddress<port::setVolume>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 handle,
                             std::int32_t volume)
    {
        return watcomEax(app, cpu, Address, {handle, x86::reg32(volume)});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
