#ifndef NFS3HP_PORT_SND_VOICES_H
#define NFS3HP_PORT_SND_VOICES_H

#include "native_guest.h"
#include "port.h"

/* port_snd_voices.cpp's typed cores that the menus call as C++ (guest::direct), and how the game's code calls
 * each by address; declared in port_snd.h as well, which holds the
 * game's addresses this one may not.  A Result is nullopt when the game stopped in a call.  A core's `esp` is the
 * frame base its thunk hands it: the game's esp on entry, at the return address.  A caller through
 * guest::direct passes ts.sp - 4 - 4 * N, N the stack words the game's call pushes: the esp the thunk hands
 * it, so the core's frame lies where the game's does; by address the call makes it. */
namespace nfs3hp
{
namespace port
{

// The game's 0x4e9420 (SNDautovol): the sound of `handle` to volume `target` over `ticks`: 0, -8 for no such sound,
// -10 before SNDinit; esp the frame base.
Result fadeSound(App* app, ThreadState& ts, u32 esp, u32 handle, int32_t ticks, int32_t target);
// The game's 0x4e95fc (SNDvol): the sound of `handle` at volume `volume` (0..127); esp the frame base.
Result setVolume(App* app, ThreadState& ts, u32 esp, u32 handle, int32_t volume);
// The game's 0x4e8fd0 (SNDstatus): 0 while the sound of `handle` plays, 1 when it no longer does, -10 before
// SNDinit; esp the frame base.
Result soundStatus(App* app, ThreadState& ts, u32 esp, u32 handle);
// The game's 0x4e9010 (SNDstopall): every sound stopped; esp the frame base.
Result stopAllSounds(App* app, ThreadState& ts, u32 esp);
// The game's 0x4e97e4 (SNDpitchbend): the sound of `handle` bent by `bend` (0..127; the registers its
// error text reads 0); esp the frame base.
Result setBend(App* app, ThreadState& ts, u32 esp, u32 handle, int32_t bend);
// The game's 0x4e9950 (SNDpan): the sound of `handle` panned to `pan` (0..127): 0, -8 for no such
// sound, -10 before SNDinit; esp the frame base.
Result panSound(App* app, ThreadState& ts, u32 esp, u32 handle, int32_t pan);

}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
