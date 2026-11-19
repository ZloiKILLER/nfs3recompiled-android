#ifndef NFS3HP_NATIVE_CONTROLS_RACE_H
#define NFS3HP_NATIVE_CONTROLS_RACE_H

#include "native_guest.h"

/* native_controls_race.cpp's typed functions that other files call as C++ (guest::direct), and how
 * the game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is false or nullopt when the game stopped in one.
 * Their frames lie on the guest's stack at the code's depth. */
namespace nfs3hp
{
namespace controls
{

// The game's 0x471270: the jolt of `car` hit, the blow's place (three floats) and its strength (a
// float) the stack's words above the return address's slot; eax as the code leaves it.
std::optional<nfs::u32> crash(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// crash for a caller with the blow's place and strength in hand (their bits), through guest::direct:
// pushed as the game's code pushes them -- the strength, its first push, in the return address's slot
// direct takes under the caller's esp -- the return address's slot below them, as its call has them.
inline std::optional<nfs::u32> crashWith(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car, nfs::u32 x,
                                           nfs::u32 y, nfs::u32 z, nfs::u32 strength)
{
    const nfs::u32 esp = ts.sp;
    guest::put(app, esp - 12, x);
    guest::put(app, esp - 8, y);
    guest::put(app, esp - 4, z);
    guest::put(app, esp, strength);
    ts.sp = esp - 16;
    const std::optional<nfs::u32> jolt = crash(app, ts, car);
    ts.sp = esp;
    return jolt;
}
// The game's 0x471440: the jolt of `car` bumped, from the right for `side` above 0, softer for
// `softness` 0 and 2.
std::optional<nfs::u32> bump(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car, nfs::u32 side,
                             nfs::u32 softness);
// The game's 0x4714f0: the kick of `car`'s gear changed; eax as the code leaves it.
std::optional<nfs::u32> gearKick(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x471dc0: whether `car` is the one the race's forces are played from.
bool forceCar(nfs::App* app, nfs::u32 car);
// The game's 0x471e10: a frame of the race's forces; eax as the code leaves it.
std::optional<nfs::u32> forcesFrame(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x471fb0: every force-feedback device's actuators off, if a race has a force
// controller; eax as the code leaves it (`unchanged`, its eax, without one).
std::optional<nfs::u32> actuatorsOff(nfs::App* app, nfs::ThreadState& ts, nfs::u32 unchanged);
// The game's 0x472b60: the controllers found compared with config.dat's, the effects made, the menus
// told; eax as the code leaves it.
std::optional<nfs::u32> loadDevices(nfs::App* app, nfs::ThreadState& ts);

// The game's 0x470f40, 0x470f80 and 0x470fc0: every force effect stopped, if a race has a force controller;
// eax as the code leaves it (`unchanged`, its eax, without one).
std::optional<nfs::u32> forcesStopped(nfs::App* app, nfs::ThreadState& ts, nfs::u32 unchanged);
// The game's 0x471000: the forces stopped, or a Microsoft device's actuators on; eax as the code leaves it.
std::optional<nfs::u32> forcesStoppedOrActuatorsOn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 unchanged);
// The game's 0x471070: the forces stopped, or a Microsoft device's actuators off; eax as the code leaves it.
std::optional<nfs::u32> forcesStoppedOrActuatorsOff(nfs::App* app, nfs::ThreadState& ts, nfs::u32 unchanged);
// The game's 0x4710f0: the car the force controller's player drives; eax as the code leaves it.
std::optional<nfs::u32> forceCarFound(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4711e0: the race's forces started; eax as the code leaves it (`unchanged` without a controller).
std::optional<nfs::u32> raceForcesStarted(nfs::App* app, nfs::ThreadState& ts, nfs::u32 unchanged);

}  // namespace controls
}  // namespace nfs3hp

#endif
