#ifndef NFS3HP_NATIVE_CONTROLS_RACE_H
#define NFS3HP_NATIVE_CONTROLS_RACE_H

#include "native_guest.h"

/* native_controls_race.cpp's typed functions that other files call as C++ (guest::direct), and how
 * the game's code calls each by address.  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one.
 * Their frames lie on the guest's stack at the code's depth. */
namespace nfs3hp
{
namespace controls
{

// The game's 0x471270: the jolt of `car` hit, the blow's place (three floats) and its strength (a
// float) the stack's words above the return address's slot; eax as the code leaves it.
std::optional<x86::reg32> crash(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// crash for a caller with the blow's place and strength in hand (their bits), through guest::direct:
// pushed as the game's code pushes them -- the strength, its first push, in the return address's slot
// direct takes under the caller's esp -- the return address's slot below them, as its call has them.
inline std::optional<x86::reg32> crashWith(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 x,
                                           x86::reg32 y, x86::reg32 z, x86::reg32 strength)
{
    const x86::reg32 esp = cpu.esp;
    guest::put(app, esp - 12, x);
    guest::put(app, esp - 8, y);
    guest::put(app, esp - 4, z);
    guest::put(app, esp, strength);
    cpu.esp = esp - 16;
    const std::optional<x86::reg32> jolt = crash(app, cpu, car);
    cpu.esp = esp;
    return jolt;
}
// The game's 0x471440: the jolt of `car` bumped, from the right for `side` above 0, softer for
// `softness` 0 and 2.
std::optional<x86::reg32> bump(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 side,
                             x86::reg32 softness);
// The game's 0x4714f0: the kick of `car`'s gear changed; eax as the code leaves it.
std::optional<x86::reg32> gearKick(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x471dc0: whether `car` is the one the race's forces are played from.
bool forceCar(win32::WinApplication* app, x86::reg32 car);
// The game's 0x471e10: a frame of the race's forces; eax as the code leaves it.
std::optional<x86::reg32> forcesFrame(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x471fb0: every force-feedback device's actuators off, if a race has a force
// controller; eax as the code leaves it (`unchanged`, its eax, without one).
std::optional<x86::reg32> actuatorsOff(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 unchanged);
// The game's 0x472b60: the controllers found compared with config.dat's, the effects made, the menus
// told; eax as the code leaves it.
std::optional<x86::reg32> loadDevices(win32::WinApplication* app, x86::CPU& cpu);

// The game's 0x470f40, 0x470f80 and 0x470fc0: every force effect stopped, if a race has a force controller;
// eax as the code leaves it (`unchanged`, its eax, without one).
std::optional<x86::reg32> forcesStopped(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 unchanged);
// The game's 0x471000: the forces stopped, or a Microsoft device's actuators on; eax as the code leaves it.
std::optional<x86::reg32> forcesStoppedOrActuatorsOn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 unchanged);
// The game's 0x471070: the forces stopped, or a Microsoft device's actuators off; eax as the code leaves it.
std::optional<x86::reg32> forcesStoppedOrActuatorsOff(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 unchanged);
// The game's 0x4710f0: the car the force controller's player drives; eax as the code leaves it.
std::optional<x86::reg32> forceCarFound(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4711e0: the race's forces started; eax as the code leaves it (`unchanged` without a controller).
std::optional<x86::reg32> raceForcesStarted(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 unchanged);

}  // namespace controls
}  // namespace nfs3hp

#endif
