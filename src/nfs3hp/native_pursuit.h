#ifndef NFS3HP_NATIVE_PURSUIT_H
#define NFS3HP_NATIVE_PURSUIT_H

#include "native_guest.h"

/* native_pursuit.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one;
 * one without `cpu` calls nothing, and through ByAddress leaves cpu.terminate to say it.  What a
 * radio message's function gives is what its code leaves in eax. */
namespace nfs3hp
{
namespace pursuit
{

/* What radioStopped leaves in eax and edx (it does not keep edx). */
struct StoppedLeft
{
    x86::reg32 eax;
    x86::reg32 edx;
};

// The game's 0x408b30: the target speed of the car pulled over at the roadside.
void stopAtRoadside(win32::WinApplication* app, x86::reg32 car);
// The game's 0x408bd0: 150 frames more to the lanes' time of where the car is.
void addLaneTime(win32::WinApplication* app, x86::reg32 car);
// The game's 0x409660: the lanes' times that have begun, 22 on each.
void growLaneTimesBy(win32::WinApplication* app);
// The game's 0x4096b0: the score of the lane the car will take; eax as the code leaves it.
std::optional<x86::reg32> scoreLaneOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x409c00: the cop's step, by its state; eax as the code leaves it.
std::optional<x86::reg32> copStepOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop);
// The game's 0x40b220: a cop's sighting of the car it chases, on the radio.
std::optional<x86::reg32> radioSpottedOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40b330: a cop's report that it is in pursuit of its chased car.
std::optional<x86::reg32> radioPursuitOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40b480: a cop's report that it lost the car.
std::optional<x86::reg32> radioLostOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40b530: a cop's call for backup on its chased car.
std::optional<x86::reg32> radioBackupOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40b670: a cop on patrol, the car it chases not a cop's.
std::optional<x86::reg32> radioPatrolOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40b720: a roadblock, on the radio.
std::optional<x86::reg32> radioRoadblockOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40b7f0: a message of nothing for the cop: nothing at all.
void radioNothingFor(win32::WinApplication* app, x86::reg32 car);
// The game's 0x40b860: a ticket, on the radio.
std::optional<x86::reg32> radioTicketOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40b910: an arrest, on the radio.
std::optional<x86::reg32> radioArrestOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40b9c0: the car stopped, `how`, on the radio; eax and edx as the code leaves them.
std::optional<StoppedLeft> radioStoppedOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 how);
// The game's 0x40bac0: an ambulance called, on the radio.
std::optional<x86::reg32> radioAmbulanceOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40bba0: spikes laid, on the radio.
std::optional<x86::reg32> radioSpikesOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40bc30: a cop's call for help with the place, on the radio.
std::optional<x86::reg32> radioHelpOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40bca0: the pursuit resumed, on the radio.
std::optional<x86::reg32> radioResumeOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40bd00: the pursuit called off, on the radio.
std::optional<x86::reg32> radioAbortOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40bf50: the cop started again; eax as the code leaves it.
std::optional<x86::reg32> copRestartOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop);
// The game's 0x40bfe0: the traffic's top speed.
x86::Float trafficTopSpeed(win32::WinApplication* app);
// The game's 0x40bff0: the car's target speed by the chased car's; eax as the code leaves it.
x86::reg32 followChasedCar(win32::WinApplication* app, x86::reg32 car);
// The game's 0x40c160: the car's node from the car it follows; eax as the code leaves it (0).
std::optional<x86::reg32> followCarOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40c210: whether the car's countdown runs out this frame.
std::optional<bool> countdownDoneOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40c270: the traffic's car's target speed; eax as the code leaves it.
std::optional<x86::reg32> trafficTargetOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40c2f0: the traffic's car's step; eax as the code leaves it.
std::optional<x86::reg32> trafficStepOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);

/* How the game's code calls those of a car (or a cop) in eax alone, eax back: their ByAddress. */
struct CalledWithCar
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return guest::watcomEax(app, cpu, Address, {car});
    }
};

}  // namespace pursuit

namespace guest
{

template <>
struct ByAddress<pursuit::scoreLaneOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::copStepOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::radioSpottedOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::radioPursuitOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::radioLostOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::radioBackupOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::radioPatrolOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::radioRoadblockOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::radioTicketOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::radioArrestOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::radioAmbulanceOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::radioSpikesOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::radioHelpOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::radioResumeOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::radioAbortOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::copRestartOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::followCarOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::trafficTargetOf> : pursuit::CalledWithCar
{
};
template <>
struct ByAddress<pursuit::trafficStepOf> : pursuit::CalledWithCar
{
};

// The car in eax; every register kept.
template <>
struct ByAddress<pursuit::stopAtRoadside>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
    }
};

// The car in eax.
template <>
struct ByAddress<pursuit::addLaneTime>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
    }
};

template <>
struct ByAddress<pursuit::growLaneTimesBy>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
    }
};

// The cop in eax, as the game's code hands it.
template <>
struct ByAddress<pursuit::radioNothingFor>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
    }
};

// The car in eax, how in edx; eax and edx as it leaves them.
template <>
struct ByAddress<pursuit::radioStoppedOf>
{
    template <x86::reg32 Address>
    static std::optional<pursuit::StoppedLeft> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                                    x86::reg32 how)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = car;
        cpu.edx = how;
        if (!guest::call(app, cpu, Address))
            return std::nullopt;
        return pursuit::StoppedLeft{cpu.eax, cpu.edx};
    }
};

// The top speed left on the x87's stack.
template <>
struct ByAddress<pursuit::trafficTopSpeed>
{
    template <x86::reg32 Address>
    static x86::Float call(win32::WinApplication* app, x86::CPU& cpu)
    {
        const std::optional<x86::Float> speed = watcomFloat(app, cpu, Address, {});
        return speed ? *speed : x86::Float(0);
    }
};

// The car in eax.
template <>
struct ByAddress<pursuit::followChasedCar>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
        return cpu.eax;
    }
};

// The car in eax; 1 or 0 in eax.
template <>
struct ByAddress<pursuit::countdownDoneOf>
{
    template <x86::reg32 Address>
    static std::optional<bool> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        const std::optional<x86::reg32> done = watcomEax(app, cpu, Address, {car});
        if (!done)
            return std::nullopt;
        return *done != 0;
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
