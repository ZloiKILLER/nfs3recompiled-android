#ifndef NFS3HP_PORT_SND_MIXER_H
#define NFS3HP_PORT_SND_MIXER_H

#include "port_snd.h"
#include "port_sound_calls.h"

/* The mixer of EA's SND library and what starts it, rewritten (port.h says
 * how): the mixer's set-ups for a channel's decoder, volume and resampler, and
 * the tables of them (port_snd_mixsetup.cpp); the effect buses, the reverb's
 * start and the mixer's start (port_snd_fx.cpp); the mix itself, a block of
 * frames of every channel's decoded, resampled and added sound
 * (port_snd_mixer.cpp).  The game's entries are in each file's end. */
namespace nfs3hp
{
namespace port
{

/* The tables of the decoders' set-ups the mixer's start fills, by whether the
 * CPU has MMX (port_snd_mixsetup.cpp).  Without it the first table calls a
 * routine of the plain set (0x522100), which the port does not have. */
Result pickPcmFirstSetups(App* app, ThreadState& ts, u32 esp);
void pickPcmSecondSetups(App* app);
void pickXaSetups(App* app);
void pickLastSetups(App* app);

/* The mix (port_snd_mixer.cpp): `frames` frames mixed into the output at
 * `target`, a block of at most 256 at a time, the game's calls made from the
 * stack at `esp`; the bytes of the last block (`eax` where there was none),
 * as the code leaves them in eax.  And a channel's volume ramped towards its
 * target over at most `frames` frames, each mixed by itself: the frames it
 * took. */
u32 mixFrames(App* app, ThreadState& ts, u32 esp, u32 target, int32_t frames, u32 eax);
u32 rampChannel(App* app, ThreadState& ts, u32 esp, u32 channel, u32 frames);

/* The game's conversions of a sound's samples (cdecl, every register kept): `count` 16-bit samples at 0x5008a0,
 * 8-bit at 0x500b10 (by a table), each to a float; `frames` stereo float frames to 16-bit samples at 0x500910
 * (where the loop stopped); the XA state at `state` decoded by the x87 at 0x500990 (stereo) and 0x500b90 (mono). */
void samples16Converted(App* app, int32_t count, u32 from, u32 to);
void samples8Converted(App* app, int32_t count, u32 from, u32 to);
u32 floatsPacked(App* app, uint32_t frames, u32 from, u32 to);
void xaStereoConverted(App* app, u32 state);
void xaMonoConverted(App* app, u32 state);

}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
