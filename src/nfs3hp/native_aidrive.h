#ifndef NFS3HP_NATIVE_AIDRIVE_H
#define NFS3HP_NATIVE_AIDRIVE_H

#include "native_guest.h"

#include <optional>

/* native_aidrive.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Those that take `ts`
 * may call the game's other functions by address on it, and are false or nullopt when the game
 * stopped in one.  Where one answers with "what the code leaves in eax", that is the eax its
 * thunk leaves. */
namespace nfs3hp
{
namespace aidrive
{

// The game's 0x404d30: a cop's start in Hot Pursuit: the slice behind it to [sliceOut], its place to [placeOut].
bool placeCopAtStart(nfs::App* app, nfs::ThreadState& ts, nfs::u32 cop, nfs::u32 sliceOut,
                     nfs::u32 placeOut);
// The game's 0x405220: cop put back behind the race's leader (on the road where `putBack` asks); what the code leaves in eax.
std::optional<nfs::u32> jumpBehind(nfs::App* app, nfs::ThreadState& ts, nfs::u32 cop, bool putBack);
// The game's 0x405740: a cop's driver, by its state; what the code leaves in eax.
std::optional<nfs::u32> driveCop(nfs::App* app, nfs::ThreadState& ts, nfs::u32 cop);
// The game's 0x405c80: the first human's car within 50 slices of slice, or 0.
std::optional<nfs::u32> humanNearSliceOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 slice);
// The game's 0x405cd0: whether a traffic car on the road is out of every human's sight.
bool unwatched(nfs::App* app, nfs::u32 car);
// The game's 0x405d40: an extra car brought onto the road, its life a random share of traffic's; its life.
std::optional<nfs::u32> bringOn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// bringOn for a caller that asks only whether the game went on.
inline bool trafficStartDone(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car)
{
    return bringOn(app, ts, car).has_value();
}
// The game's 0x405dc0: an extra car taken off the road (a traffic car's and a cop's own leaving); what the code leaves in eax.
std::optional<nfs::u32> takeOff(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x405e20: an extra car's step (a traffic car's, a cop's, 0 for any other); what it gives.
std::optional<nfs::u32> stepExtraCar(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x405fd0: car's reversing set by `how` for `frames`; what its code leaves in edx.
nfs::u32 setReversing(nfs::App* app, nfs::u32 car, nfs::u32 how, nfs::u32 frames);
// The game's 0x406120: car, moving and slower than 10, turns its reversing over; whether it did.
bool reverseIfSlow(nfs::App* app, nfs::u32 car);
// reverseIfSlow for a caller that asks only whether the game went on (it cannot stop it).
inline bool reverseWhenSlowDone(nfs::App* app, nfs::u32 car)
{
    reverseIfSlow(app, car);
    return true;
}
// The game's 0x406160: as reverseIfSlow, where its nose also points down along the vector at `down`.
bool reverseIfTipped(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car, nfs::u32 down);
// The game's 0x4088f0: the computer's car's step, each frame; what the code leaves in eax.
std::optional<nfs::u32> stepCar(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);

}  // namespace aidrive

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
