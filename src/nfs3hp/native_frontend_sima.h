#ifndef NFS3HP_NATIVE_FRONTEND_SIMA_H
#define NFS3HP_NATIVE_FRONTEND_SIMA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_sima.cpp's typed functions (the opponents' engine and heading, the race cameras'
 * modes, a car's place on the track), which other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each calls the game's
 * other functions by address on `ts` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax, and what it leaves on the
 * x87's stack stays there. */
namespace nfs3hp
{
namespace frontend
{
namespace sima
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x40de30: an opponent's lane votes added up and acted on.
u32 laneSteer(App* app, ThreadState& ts, u32 car);
// What a function that leaves a float on the x87's stack and a word in eax gives back as C++.
struct FloatLeft
{
    double value;
    u32 eax;
};
// enginePull's core: the pull and the eax it leaves.
FloatLeft enginePullOf(App* app, ThreadState& ts, u32 car);
// The game's 0x4070c0: how far the way the driver wants to go is from the way its car points.
u32 headingError(App* app, ThreadState& ts, u32 car);
// The game's 0x425060: camera mode 8, beside the road near the car.
u32 cameraBesideRoad(App* app, ThreadState& ts, u32 camera);
// The game's 0x4251f0: camera mode 18, the nearest of the track's fixed spots.
u32 cameraNearestSpot(App* app, ThreadState& ts, u32 camera);
// The game's 0x4253d0: camera mode 16, an animated path.
u32 cameraPlayPath(App* app, ThreadState& ts, u32 camera);
// The game's 0x425640: camera mode 17, the path at a race's end.
u32 cameraEndPath(App* app, ThreadState& ts, u32 camera);
// The game's 0x425bf0: camera mode 14, beside the road, higher up.
u32 cameraOverRoad(App* app, ThreadState& ts, u32 camera);
// The game's 0x425d50: camera mode 9, the flyover.
u32 cameraFlyover(App* app, ThreadState& ts, u32 camera);
// The game's 0x426290: camera mode 20's step: nothing.
u32 cameraStill(App* app, ThreadState& ts, u32 camera);
// The game's 0x4262a0: camera mode 19, the mode it was told for a while, then the view handed on.
u32 cameraWaitThenHandOn(App* app, ThreadState& ts, u32 camera);
// The game's 0x426310: a view under mode 19 handed straight on to the mode it was waiting for.
u32 cameraBackFromWait(App* app, ThreadState& ts, u32 view);
// The game's 0x426450: a mode's camera of a view prepared and stepped.
u32 cameraStep(App* app, ThreadState& ts, u32 mode, u32 view);
// The game's 0x426560: the mode a view's car forces now.
u32 forcedMode(App* app, ThreadState& ts, u32 view);
// The game's 0x426680: every view's camera stepped.
u32 camerasStep(App* app, ThreadState& ts);
// quickDistance's core: the distance and the eax it leaves.
FloatLeft quickDistanceOf(App* app, ThreadState& ts, u32 a, u32 b);
// planarLength's core: the length.
double planarLengthOf(App* app, ThreadState& ts, u32 a, u32 b);
// The game's 0x49fde0: whether a car leans over too far: 1 or 0.
u32 carTiltedOver(App* app, ThreadState& ts, u32 car);
// The game's 0x49fee0: the track's heading at a slice, as a fixed angle.
u32 sliceHeading(App* app, ThreadState& ts, u32 index);
// The game's 0x49ffa0: a car in the race given the track's frame where it is, and its heading.
u32 carTrackFrame(App* app, ThreadState& ts, u32 car);
// The game's 0x4a00c0: how many nodes from `node` to `from` (0x40e120).
u32 nodeGap(App* app, ThreadState& ts, u32 node, u32 from);
// The game's 0x4a0140: a car moved back onto the road.
u32 carOntoRoad(App* app, ThreadState& ts, u32 car);
// The game's 0x4a0580: how far a car is from the players, and what it does about it.
u32 carNearPlayers(App* app, ThreadState& ts, u32 car);
// The game's 0x4a08c0: the frame's work on a car's place on the track.
u32 carTrackEnvironment(App* app, ThreadState& ts, u32 car);

}  // namespace sima
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
