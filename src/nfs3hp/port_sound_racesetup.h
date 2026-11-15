#ifndef NFS3HP_PORT_SOUND_RACESETUP_H
#define NFS3HP_PORT_SOUND_RACESETUP_H

#include "native_guest.h"
#include "port.h"

/* port_sound_racesetup.cpp's typed core that the menus call as C++ (guest::direct), and how the game's code
 * calls it by address (ByAddress below, for the checks' stand-ins); nullopt when the game stopped in a call.
 * Its `esp` is the frame base its adapter hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * adapter hands it, so the core's frame lies where the game's does; by address the call makes it. */
namespace nfs3hp
{
namespace port
{

// The game's 0x4136e0: the race's first bank loaded into slot 0: its handle; esp the frame base.
Result raceBankLoadOf(App* app, CPU& cpu, reg32 esp);
// The game's 0x412b50: the race's sound ended (the master volume faded out, everything stopped and freed, the
// music stopped, the menus' volumes back): the music's setting kept; esp the frame base.
Result raceSoundEndOf(App* app, CPU& cpu, reg32 esp);

}  // namespace port

namespace guest
{

// Nothing handed over; the handle in eax.
template <>
struct ByAddress<port::raceBankLoadOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// Nothing handed over; eax back.
template <>
struct ByAddress<port::raceSoundEndOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
