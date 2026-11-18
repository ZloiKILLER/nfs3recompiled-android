#ifndef NFS3HP_NATIVE_DRIVETRAIN_H
#define NFS3HP_NATIVE_DRIVETRAIN_H

#include "native_guest.h"

/* native_drivetrain.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace drivetrain
{

// The game's 0x4aaed1: the engine's force at car's wheels.
std::optional<float> forceAtWheels(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x4a9bc4: car's driver's inputs read into it.
bool readInputs(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x4aab41: the grip of car's wheels.
bool setWheelGrip(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);

}  // namespace drivetrain

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
