#ifndef NFS3HP_NATIVE_AIPLAN_H
#define NFS3HP_NATIVE_AIPLAN_H

#include "native_guest.h"

#include <cstdint>
#include <cstring>
#include <optional>

/* native_aiplan.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Those that take `ts`
 * may call the game's other functions by address on it, and are false or nullopt when the game
 * stopped in one. */
namespace nfs3hp
{
namespace aiplan
{

// The game's 0x40c590: traffic car set going (its way along the track, the player car it keeps near), then placed.
std::optional<nfs::u32> placeTraffic(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40c9f0: a traffic car's driver, each tick (its loading steps, then its state's).
bool driveTraffic(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40cb70: a traffic car reset (its record cleared, then linked).
bool resetTraffic(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40cd80: car put on the track at its node and target place across, its velocity along its nose.
bool putOnTrack(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40ce70: a pulled-over car brought to a stop once on the ground and gripping; whether it was.
std::optional<bool> stopPulledOver(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// stopPulledOver with the eax the code leaves: the car, with the grip's compare's status word in ax where that
// ended it, or the last scaling's; nullopt when the game stopped.
std::optional<nfs::u32> stopPulledOverLeaving(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// stopPulledOver for a caller that asks only whether the game went on.
inline bool brakeToStopDone(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car)
{
    return stopPulledOver(app, ts, car).has_value();
}
// The game's 0x40d720: car's lanes scored against the parked obstacles ahead.
void scoreObstacles(nfs::App* app, nfs::u32 car);
// The game's 0x40d800: car's lanes scored against the cars sensed ahead in each.
bool scoreCars(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40d9f0: a small cut to the outer lanes' votes for a car sensed close ahead in them.
void penaliseCloseCars(nfs::App* app);
// The game's 0x40dd60: the lane planner started for car (votes cleared, its slices, its lane, the traffic sensed).
bool beginPlanning(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40ddb0: car picks its lane (the edge, the way there, the road; the obstacles and cars but for a player's).
bool pickLane(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40de50: car's horn sounded for a while, its tone on.
void soundHorn(nfs::App* app, nfs::u32 car);
// The game's 0x40deb0: car's horn ended once its time is up.
void endHorn(nfs::App* app, nfs::u32 car);
// The game's 0x40df10: a traffic car's loading steps; 0 when it is not on the road, else 1.
std::optional<nfs::u32> stepLoading(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40e080: the computer cars' target speed at slice.
float targetSpeed(nfs::App* app, nfs::u32 slice);
// The game's 0x40e0c0: the road's kind at slice.
nfs::u32 roadKind(nfs::App* app, nfs::u32 slice);
// The game's 0x40e120: node less node `from`, wrapped round the track.
std::int32_t nodesBetween(nfs::App* app, nfs::u32 node, nfs::u32 from);
// The game's 0x40e160: the node `steps` on from car's in its direction of travel.
nfs::u32 nodeAhead(nfs::App* app, nfs::u32 car, nfs::u32 steps);
// The game's 0x40e1b0: the node car will reach by its look-ahead times `scale`, within `least` and `most`.
nfs::u32 nodeAheadBySpeed(nfs::App* app, nfs::u32 car, float scale, std::int32_t least,
                            std::int32_t most);
// The game's 0x40e2b0: the road slice (0..13) `offset` across the road falls in at node.
nfs::u32 roadSlice(nfs::App* app, nfs::u32 node, float offset);
// The game's 0x40e350: car's three road slices (its middle's and its sides') written to it.
void findCarSlices(nfs::App* app, nfs::u32 car);
// The same, and the eax the code leaves: the last slice.
nfs::u32 findCarSlicesLeaving(nfs::App* app, nfs::u32 car);
// The game's 0x40eca0: whether the track's road is wide.
nfs::u32 trackRoadWide(nfs::App* app);
// The game's 0x40ed40: the traffic's speed at node.
float trafficSpeed(nfs::App* app, nfs::u32 node);
// The game's 0x40ee80: the track's tuning speed.
float trackTuningSpeed(nfs::App* app);
// The game's 0x40eed0: the track's tuning distance, more in the weather and at night.
nfs::u32 trackTuningDistance(nfs::App* app);
// The game's 0x40ef10: the first word of the track's tuning record.
nfs::u32 trackTuningFirst(nfs::App* app);
// The game's 0x40ef70: the dot product of nodes a's and b's forward directions.
nfs::Float directionDot(nfs::App* app, nfs::u32 a, nfs::u32 b);
// The game's 0x40f000: whether node lies in the run of `run` nodes from node `from`.
bool nodeInRun(nfs::App* app, nfs::u32 node, nfs::u32 from, nfs::u32 run);
// The game's 0x40f050: whether car is within reach of `distance` along the track.
bool carInReach(nfs::App* app, nfs::u32 car, nfs::u32 distance);
// The game's 0x40f100: 1, whatever it is asked.
nfs::u32 answerOne(nfs::App* app);
// The game's 0x40f110: whether car is boxed in off its node's edge slices.
bool boxedIn(nfs::App* app, nfs::u32 car);
// The game's 0x40f330: the seven animation paths freed and forgotten.
bool freePaths(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x40f3b0: path started in a free animation slot; the slot, or -1.
nfs::u32 playAnimation(nfs::App* app, nfs::u32 path);
// The game's 0x40f430: an animation slot's mode, flags and third word set.
void setAnimation(nfs::App* app, nfs::u32 slot, nfs::u32 mode, nfs::u32 flags, nfs::u32 third);
// The game's 0x40f460: an animation slot stopped; 1, or -1 when it was not playing.
nfs::u32 endAnimation(nfs::App* app, nfs::u32 slot);
// The game's 0x40f4b0: path's pose at `time` in `mode` made a matrix at `matrix`; 1.
std::optional<nfs::u32> poseAt(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path, nfs::u32 mode,
                                 std::int32_t time, nfs::u32 blendWord, nfs::u32 matrix);
// The game's 0x40f800: an animation slot's frame and spans written out; the spans.
nfs::u32 animationSpans(nfs::App* app, nfs::u32 slot, nfs::u32 frameOut, nfs::u32 spansOut);
// The game's 0x40f940: an animation slot stepped, its pose to `pose`; 1, 0 when it ended, -1 when not playing.
std::optional<nfs::u32> stepAnimation(nfs::App* app, nfs::ThreadState& ts, nfs::u32 slot,
                                        nfs::u32 blendWord, nfs::u32 pose);

// resetTraffic with the eax the code leaves (0x404690's); nullopt when the game stopped.
std::optional<nfs::u32> resetTrafficEax(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x40da30: car's lane votes added up and the lane taken; eax as the code leaves it is no one's to read.
void takeLane(nfs::App* app, nfs::u32 car);
// The game's 0x40daf0: car steered for its lane; eax as the code leaves it is no one's to read.
void steerForLane(nfs::App* app, nfs::u32 car);
// The game's 0x40e220: the lane bits of `node` in slice `slice` (eax; 0 for none).
nfs::u32 hasLane(nfs::App* app, nfs::u32 node, nfs::u32 slice);
// The game's 0x40ec40: the speeds table freed; eax, or `eax` as it came where there was none to free.
std::optional<nfs::u32> freeSpeeds(nfs::App* app, nfs::ThreadState& ts, nfs::u32 eax);
// The game's 0x40f6d0: path's place at `time` (mode bit 1: played once), the time within the path to
// `timeOut` and its length to `lengthOut`; 1.
std::optional<nfs::u32> placeAt(nfs::App* app, nfs::ThreadState& ts, nfs::u32 path, nfs::u32 mode,
                                  std::int32_t time, nfs::u32 blendWord, nfs::u32 timeOut, nfs::u32 lengthOut);
// The game's 0x40f8d0: the end pose of path `index` by `flags`, its place to `placeOut`, its matrix to `pose`.
bool endPose(nfs::App* app, nfs::ThreadState& ts, nfs::u32 index, nfs::u32 flags, nfs::u32 placeOut,
             nfs::u32 pose);
// The game's 0x40fac0: whether slot is playing.
bool animationOn(nfs::App* app, nfs::u32 slot);
// The game's 0x40fb10: the sound brought up at the race's start; `callerEdx` as its library start tests it.
bool bringUpSound(nfs::App* app, nfs::ThreadState& ts, nfs::u32 callerEdx);
// The game's 0x40fcf0: the sound restarted for `setup`; `callerEdx` as its library start tests it.
bool restartLibrary(nfs::App* app, nfs::ThreadState& ts, nfs::u32 setup, nfs::u32 callerEdx);
}  // namespace aiplan

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
