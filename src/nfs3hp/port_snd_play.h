#ifndef NFS3HP_PORT_SND_PLAY_H
#define NFS3HP_PORT_SND_PLAY_H

#include "native_guest.h"
#include "port.h"

/* port_snd_play.cpp's typed core that the menus call as C++ (guest::direct), and how the game's code calls it
 * by address (ByAddress below, for the checks' stand-ins); declared in port_snd.h as well, which holds the
 * game's addresses this one may not (tools/game_checks/generate.py counts a header's numbers as named by every
 * file including it).  nullopt when the game stopped in a call.  Its `esp` is the frame base its adapter hands
 * it: the game's esp on entry, at the return address.  A caller through guest::direct passes cpu.esp - 4 - 4 *
 * N, N the stack words the game's call pushes: the esp the adapter hands it, so the core's frame lies where the
 * game's does; by address the call makes it. */
namespace nfs3hp
{
namespace port
{

// The game's 0x4e8c1c: SNDbankremove of bank `bank`; esp the frame base.
Result removeEveryBank(App* app, CPU& cpu, reg32 esp, int32_t bank);

}  // namespace port

namespace guest
{

// The bank in eax; eax back.
template <>
struct ByAddress<port::removeEveryBank>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t bank)
    {
        return watcomEax(app, cpu, Address, {x86::reg32(bank)});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
