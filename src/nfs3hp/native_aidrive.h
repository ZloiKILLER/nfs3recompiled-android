#ifndef NFS3HP_NATIVE_AIDRIVE_H
#define NFS3HP_NATIVE_AIDRIVE_H

#include "native_guest.h"

#include <optional>

/* native_aidrive.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Those that take `cpu`
 * may call the game's other functions by address on it, and are false or nullopt when the game
 * stopped in one.  Where one answers with "what the code leaves in eax", that is the eax its
 * thunk leaves. */
namespace nfs3hp
{
namespace aidrive
{

// The game's 0x404d30: a cop's start in Hot Pursuit: the slice behind it to [sliceOut], its place to [placeOut].
bool placeCopAtStart(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop, x86::reg32 sliceOut,
                     x86::reg32 placeOut);
// The game's 0x405220: cop put back behind the race's leader (on the road where `putBack` asks); what the code leaves in eax.
std::optional<x86::reg32> jumpBehind(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop, bool putBack);
// The game's 0x405740: a cop's driver, by its state; what the code leaves in eax.
std::optional<x86::reg32> driveCop(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop);
// The game's 0x405c80: the first human's car within 50 slices of slice, or 0.
std::optional<x86::reg32> humanNearSliceOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slice);
// The game's 0x405cd0: whether a traffic car on the road is out of every human's sight.
bool unwatched(win32::WinApplication* app, x86::reg32 car);
// The game's 0x405d40: an extra car brought onto the road, its life a random share of traffic's; its life.
std::optional<x86::reg32> bringOn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// bringOn for a caller that asks only whether the game went on.
inline bool trafficStartDone(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
{
    return bringOn(app, cpu, car).has_value();
}
// The game's 0x405dc0: an extra car taken off the road (a traffic car's and a cop's own leaving); what the code leaves in eax.
std::optional<x86::reg32> takeOff(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x405e20: an extra car's step (a traffic car's, a cop's, 0 for any other); what it gives.
std::optional<x86::reg32> stepExtraCar(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x405fd0: car's reversing set by `how` for `frames`; what its code leaves in edx.
x86::reg32 setReversing(win32::WinApplication* app, x86::reg32 car, x86::reg32 how, x86::reg32 frames);
// The game's 0x406120: car, moving and slower than 10, turns its reversing over; whether it did.
bool reverseIfSlow(win32::WinApplication* app, x86::reg32 car);
// reverseIfSlow for a caller that asks only whether the game went on (it cannot stop it).
inline bool reverseWhenSlowDone(win32::WinApplication* app, x86::reg32 car)
{
    reverseIfSlow(app, car);
    return true;
}
// The game's 0x406160: as reverseIfSlow, where its nose also points down along the vector at `down`.
bool reverseIfTipped(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 down);
// The game's 0x4088f0: the computer's car's step, each frame; what the code leaves in eax.
std::optional<x86::reg32> stepCar(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);

}  // namespace aidrive

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
