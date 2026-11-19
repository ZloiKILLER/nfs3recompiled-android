#ifndef NFS3HP_PORT_SND_CALLS_H
#define NFS3HP_PORT_SND_CALLS_H

#include "native_guest.h"
#include "port.h"

/* The SND library's typed cores the game's code calls by address and the rewrites call as C++ (guest::direct):
 * the banks (port_snd_play.cpp), the master volume, the library's end (port_snd_driver.cpp), the effect bus and
 * the mixer's start (port_snd_fx.cpp), a channel's ramp (port_snd_mixer.cpp); and how the game's code calls each
 *.    A Result is nullopt when the game
 * stopped in a call.  A core's `esp` is the frame base its thunk hands it: the game's esp on entry, at the
 * return address; a caller through guest::direct sets ts.sp to the esp the game's code has at the call and
 * passes that - 4 - 4 * N, N the stack words the call pushes. */
namespace nfs3hp
{
namespace port
{

// The game's 0x4e8d20 (SNDbankadd): the bank at `bank` added, its handle into the word at `handleAt`:
// downloadBank's result, -10 before SNDinit.
Result bankAdded(App* app, ThreadState& ts, u32 esp, u32 handleAt, u32 bank);
// The game's 0x4e8db0 (SNDbankheadersize): the size of bank `bank`'s header.
Result bankHeaderSize(App* app, ThreadState& ts, u32 esp, int32_t bank);
// The game's 0x4e8e20 (SNDbankheadercopy): bank `bank`'s header copied to `to`, which the bank is then.
Result copyBankHeader(App* app, ThreadState& ts, u32 esp, u32 to, int32_t bank);
// The game's 0x413750: the voice bank of archive `archive` for speaker `speaker` into slot `slot`: the memory it is
// kept in, nullopt when the game stopped; esp the frame base.
std::optional<u32> voiceBankLoaded(App* app, ThreadState& ts, u32 esp, u32 archive, int32_t speaker, int32_t slot);
// The game's 0x4e2228 (SNDsetrate): the priority offset set from the low word of `rate`.  0.
u32 priorityOffsetSet(App* app, u32 rate);
// The game's 0x4e2630 (SNDinit): the library started for mode `mode` (`second` the verification's second word):
// 0, or the driver's error.
Result libraryStarted(App* app, ThreadState& ts, u32 esp, u32 mode, u32 second);
// The game's 0x4e73d0 (SNDmastervol): the master volume to `volume` (0..127).
Result setMasterVolume(App* app, ThreadState& ts, u32 esp, int32_t volume);
// The game's 0x4e26b4 (SNDrestore): the library ended: the driver's result, -10 before SNDinit.  eax what the
// caller has in eax, which the first restore it calls is handed.
Result endLibrary(App* app, ThreadState& ts, u32 esp, u32 eax);
// The game's 0x4e9c70 (SNDfxinitbus): bus `bus` set to effect `type` with `first` and `second`, its master
// level `level`.
Result initEffectBus(App* app, ThreadState& ts, u32 esp, int32_t bus, u32 type, int32_t first, int32_t second,
                     int32_t level);
// The game's 0x4e9dfc (SNDfxmasterlevel): bus `bus`'s master level to `level`.  0.
Result setBusMaster(App* app, ThreadState& ts, u32 esp, int32_t bus, int32_t level);
// The game's 0x4fff18 (iSNDmixinit): the mixer started for the card's format `format`, `ended` what it calls
// when a channel ends.  0.
Result startMixer(App* app, ThreadState& ts, u32 esp, u32 format, u32 ended);
// The game's 0x5004c8: channel `channel`'s volume ramped over at most `frames` frames: the frames it took.
u32 rampChannel(App* app, ThreadState& ts, u32 esp, u32 channel, u32 frames);
// The game's 0x52d160: the packet player whose voice is voice `voice`, -1 for none.
int32_t packetPlayerOfVoice(App* app, u32 voice);

}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
