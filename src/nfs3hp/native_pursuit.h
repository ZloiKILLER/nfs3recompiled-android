#ifndef NFS3HP_NATIVE_PURSUIT_H
#define NFS3HP_NATIVE_PURSUIT_H

#include "native_guest.h"

/* native_pursuit.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each may call the
 * game's other functions by address on `ts`, and is false or nullopt when the game stopped in one;
 * one without `ts` calls nothing.  What a
 * radio message's function gives is what its code leaves in eax. */
namespace nfs3hp
{
namespace pursuit
{

/* What radioStopped leaves in eax and edx (it does not keep edx). */
struct StoppedLeft
{
    nfs::u32 eax;
    nfs::u32 edx;
};

// The game's 0x408b30: the target speed of the car pulled over at the roadside.
void stopAtRoadside(nfs::App* app, nfs::u32 car);
// The game's 0x408bd0: 150 frames more to the lanes' time of where the car is.
void addLaneTime(nfs::App* app, nfs::u32 car);
// The game's 0x409660: the lanes' times that have begun, 22 on each.
void growLaneTimesBy(nfs::App* app);
// The game's 0x4096b0: the score of the lane the car will take; eax as the code leaves it.
std::optional<nfs::u32> scoreLaneOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x409c00: the cop's step, by its state; eax as the code leaves it.
std::optional<nfs::u32> copStepOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 cop);
// The game's 0x40b220: a cop's sighting of the car it chases, on the radio.
std::optional<nfs::u32> radioSpottedOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40b330: a cop's report that it is in pursuit of its chased car.
std::optional<nfs::u32> radioPursuitOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40b480: a cop's report that it lost the car.
std::optional<nfs::u32> radioLostOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40b530: a cop's call for backup on its chased car.
std::optional<nfs::u32> radioBackupOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40b670: a cop on patrol, the car it chases not a cop's.
std::optional<nfs::u32> radioPatrolOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40b720: a roadblock, on the radio.
std::optional<nfs::u32> radioRoadblockOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40b7f0: a message of nothing for the cop: nothing at all.
void radioNothingFor(nfs::App* app, nfs::u32 car);
// The game's 0x40b860: a ticket, on the radio.
std::optional<nfs::u32> radioTicketOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40b910: an arrest, on the radio.
std::optional<nfs::u32> radioArrestOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40b9c0: the car stopped, `how`, on the radio; eax and edx as the code leaves them.
std::optional<StoppedLeft> radioStoppedOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car, nfs::u32 how);
// The game's 0x40bac0: an ambulance called, on the radio.
std::optional<nfs::u32> radioAmbulanceOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40bba0: spikes laid, on the radio.
std::optional<nfs::u32> radioSpikesOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40bc30: a cop's call for help with the place, on the radio.
std::optional<nfs::u32> radioHelpOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40bca0: the pursuit resumed, on the radio.
std::optional<nfs::u32> radioResumeOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40bd00: the pursuit called off, on the radio.
std::optional<nfs::u32> radioAbortOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40bf50: the cop started again; eax as the code leaves it.
std::optional<nfs::u32> copRestartOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 cop);
// The game's 0x40bfe0: the traffic's top speed.
nfs::Float trafficTopSpeed(nfs::App* app);
// The game's 0x40bff0: the car's target speed by the chased car's; eax as the code leaves it.
nfs::u32 followChasedCar(nfs::App* app, nfs::u32 car);
// The game's 0x40c160: the car's node from the car it follows; eax as the code leaves it (0).
std::optional<nfs::u32> followCarOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40c210: whether the car's countdown runs out this frame.
std::optional<bool> countdownDoneOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40c270: the traffic's car's target speed; eax as the code leaves it.
std::optional<nfs::u32> trafficTargetOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40c2f0: the traffic's car's step; eax as the code leaves it.
std::optional<nfs::u32> trafficStepOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);

/* How the game's code calls those of a car (or a cop) in eax alone, eax back. */
struct CalledWithCar
{
    template <nfs::u32 Address>
    static std::optional<nfs::u32> call(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car)
    {
        return guest::pointerCall(app, ts, Address, {car});
    }
};

// The game's 0x40a330: the radio's frames counted on.
void tickRadioFrames(nfs::App* app);
}  // namespace pursuit

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
