#ifndef NFS3HP_NATIVE_PHYSICS_H
#define NFS3HP_NATIVE_PHYSICS_H

#include "native_guest.h"

/* native_physics.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and is nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace physics
{

// The game's 0x4ae4c1 (a car's +0x608): car's physics for the frame; what the code leaves in eax.
std::optional<x86::reg32> stepCar(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);

}  // namespace physics

namespace guest
{

// The car in eax; eax as it leaves it.
template <>
struct ByAddress<physics::stepCar>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcomEax(app, cpu, Address, {car});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
