#ifndef NFS3HP_NATIVE_PHYSICS_H
#define NFS3HP_NATIVE_PHYSICS_H

#include "native_guest.h"

/* native_physics.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace physics
{

// The game's 0x4ae4c1 (a car's +0x608): car's physics for the frame; what the code leaves in eax.
std::optional<nfs::u32> stepCar(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);

}  // namespace physics

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
