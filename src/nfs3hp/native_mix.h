#ifndef NFS3HP_NATIVE_MIX_H
#define NFS3HP_NATIVE_MIX_H

#include "native_guest.h"

/* native_mix.cpp's typed functions that the mixer's rewrites call as C++ (guest::direct), and how the game's
 * code calls each by address (cdecl: the words pushed, the caller pops them). */
namespace nfs3hp
{
namespace mix
{

/* An interpolation's words, in the order the caller pushes them: the frames, the source, the output, where the
 * whole position's dword and the fraction's are, the step's whole and its fraction. */
struct Interpolate
{
    x86::reg32 count, source, target, wholeAt, fractionAt, stepWhole, stepFraction;
};

// The game's 0x5270f0, 0x527170, 0x527210 and 0x527290: 8-bit mono, 8-bit stereo, 16-bit mono and 16-bit
// stereo frames interpolated; both positions written back.
void interpolatedMono8(win32::WinApplication* app, const Interpolate& a);
void interpolatedStereo8(win32::WinApplication* app, const Interpolate& a);
void interpolatedMono16(win32::WinApplication* app, const Interpolate& a);
void interpolatedStereo16(win32::WinApplication* app, const Interpolate& a);
// The game's 0x52d1a0 and 0x52d250: the XA decoder's state at `state` decoded, mono and stereo.
void xaMonoDecoded(win32::WinApplication* app, x86::reg32 state);
void xaStereoDecoded(win32::WinApplication* app, x86::reg32 state);

}  // namespace mix

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
