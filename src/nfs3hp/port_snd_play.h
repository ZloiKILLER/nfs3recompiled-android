#ifndef NFS3HP_PORT_SND_PLAY_H
#define NFS3HP_PORT_SND_PLAY_H

#include "native_guest.h"
#include "port.h"

/* port_snd_play.cpp's typed core that the menus call as C++ (guest::direct), and how the game's code calls it
 * by address; declared in port_snd.h as well.  nullopt when the game stopped in a call.  Its `esp` is the frame base its thunk hands
 * it: the game's esp on entry, at the return address.  A caller through guest::direct passes ts.sp - 4 - 4 *
 * N, N the stack words the game's call pushes: the esp the thunk hands it, so the core's frame lies where the
 * game's does; by address the call makes it. */
namespace nfs3hp
{
namespace port
{

// The game's 0x4e8c1c: SNDbankremove of bank `bank`; esp the frame base.
Result removeEveryBank(App* app, ThreadState& ts, u32 esp, int32_t bank);

}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
