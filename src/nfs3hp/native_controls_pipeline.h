#ifndef NFS3HP_NATIVE_CONTROLS_PIPELINE_H
#define NFS3HP_NATIVE_CONTROLS_PIPELINE_H

#include "native_guest.h"

/* native_controls_pipeline.cpp's typed functions that other files call as C++ (guest::direct), and
 * how the game's code calls each by address.  Each may call
 * the game's other functions by address on `cpu`, and is false or nullopt when the game stopped in
 * one. */
namespace nfs3hp
{
namespace controls
{

// The game's 0x48f550: a frame of the race's controls: the joysticks read, each player's four bytes
// made from the table of actions, the function keys into their ring, and handed on; eax as the code
// leaves it (`unchanged`, its eax, where nothing else sets it).
std::optional<x86::reg32> readControls(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 unchanged);

// The game's 0x48f1f0: the players' controls bound to the devices; eax as the code leaves it.
std::optional<x86::reg32> playersBound(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x48f4f0: the race's controls started; eax as the code leaves it.
std::optional<x86::reg32> raceControlsStarted(win32::WinApplication* app, x86::CPU& cpu);

}  // namespace controls
}  // namespace nfs3hp

#endif
