#ifndef NFS3HP_PORT_SOUND_EFFECTS_H
#define NFS3HP_PORT_SOUND_EFFECTS_H

#include "native_guest.h"
#include "port.h"

/* port_sound_effects.cpp's typed cores that the menus call as C++ (guest::direct), and how the game's code
 * calls each by address (ByAddress below, for the checks' stand-ins); bankEffect is declared in port_snd.h as
 * well, which holds the game's addresses this one may not (a header's numbers count as named by every file
 * including it: tools/game_checks/generate.py).  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its adapter hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * adapter hands it, so the core's frame lies where the game's does; by address the call makes it. */
namespace nfs3hp
{
namespace port
{

// The game's 0x4181d0: menu effect `effect` of the first bank at `volume`: SNDplay's handle (`effect` with the
// sounds off); esp the frame base.
Result playMenuEffectOf(App* app, CPU& cpu, reg32 esp, int32_t effect, int32_t volume);
// The game's 0x4182b0: effect `effect` of bank `bank` at `volume`, pan `pan`, pitch `pitch`: the sound's handle, -1
// with the effects off; esp the frame base.
Result bankEffect(App* app, CPU& cpu, reg32 esp, int32_t bank, int32_t effect, int32_t volume, uint32_t pan,
                  uint8_t pitch);

}  // namespace port

namespace guest
{

// The effect in eax, the volume in edx; the handle in eax.
template <>
struct ByAddress<port::playMenuEffectOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t effect,
                             std::int32_t volume)
    {
        return watcomEax(app, cpu, Address, {x86::reg32(effect), x86::reg32(volume)});
    }
};

// Stdcall: the bank in eax, the effect in edx, the pan in ebx, the volume in ecx; the pitch pushed (ret 4); the
// handle in eax.
template <>
struct ByAddress<port::bankEffect>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t bank,
                             std::int32_t effect, std::int32_t volume, std::uint32_t pan, std::uint8_t pitch)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = x86::reg32(bank);
        cpu.edx = x86::reg32(effect);
        cpu.ebx = pan;
        cpu.ecx = x86::reg32(volume);
        if (!guest::call(app, cpu, Address, {pitch}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
