#ifndef NFS3HP_PORT_SOUND_RACESETUP_H
#define NFS3HP_PORT_SOUND_RACESETUP_H

#include "native_guest.h"
#include "port.h"

/* port_sound_racesetup.cpp's typed core that the menus call as C++ (guest::direct), and how the game's code
 * calls it by address; nullopt when the game stopped in a call.
 * Its `esp` is the frame base its thunk hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * thunk hands it, so the core's frame lies where the game's does; by address the call makes it. */
namespace nfs3hp
{
namespace port
{

// The game's 0x4136e0: the race's first bank loaded into slot 0: its handle; esp the frame base.
Result raceBankLoadOf(App* app, CPU& cpu, reg32 esp);
// The game's 0x4129c0: the race's sound set up (the slots emptied, the engines started, the speech reset, the cars'
// sounds' state cleared, the master volume set); esp the frame base.
Result raceSoundInitOf(App* app, CPU& cpu, reg32 esp);
// The game's 0x412b50: the race's sound ended (the master volume faded out, everything stopped and freed, the
// music stopped, the menus' volumes back): the music's setting kept; esp the frame base.
Result raceSoundEndOf(App* app, CPU& cpu, reg32 esp);

}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
