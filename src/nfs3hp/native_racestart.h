#ifndef NFS3HP_NATIVE_RACESTART_H
#define NFS3HP_NATIVE_RACESTART_H

#include "native_guest.h"

/* native_racestart.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is false or nullopt when the game stopped in one;
 * one without `ts` calls nothing. */
namespace nfs3hp
{
namespace racestart
{

/* What a function's code leaves in eax and edx, which it does not keep: a caller of this
 * file's passes them on where its code does. */
struct Left
{
    nfs::u32 eax, edx;
};

// The game's 0x429100: the car put back on the road when the player asks; eax as the code leaves it.
std::optional<nfs::u32> resetCarOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x429b40: what wheel `wheel` (0 to 3) of the car is on.
std::optional<nfs::u32> wheelSurfaceOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car,
                                         nfs::u32 wheel);
// The game's 0x42b900: the car's frame of race state between the physics'; eax as the code leaves it.
std::optional<nfs::u32> updateCarOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x42bb80: the start's grid, from the track's start.dat or worked out; eax as the code leaves it.
std::optional<nfs::u32> loadStartGridOf(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x42bc70: grid place `place`, its slice to [slice] and its place to `where`.
Left gridPlaceOf(nfs::App* app, nfs::u32 place, nfs::u32 slice, nfs::u32 where);
// The game's 0x42bcb0: the car put on its grid place (`number` its handling's), its race state the start's.
bool placeOnGridOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car, nfs::u32 number);

// The game's 0x4294c0: the car's slice to be put back on and the frames since, cleared (eax: the car).
void clearRespawnOf(nfs::App* app, nfs::u32 car);
// The game's 0x4296c0: the car's little shake started; eax as the code leaves it, nullopt when the game stopped.
std::optional<nfs::u32> startIdleShakeOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
}  // namespace racestart

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
