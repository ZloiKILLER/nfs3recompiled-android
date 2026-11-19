#ifndef NFS3HP_NATIVE_CARSTATE_H
#define NFS3HP_NATIVE_CARSTATE_H

#include "native_guest.h"

#include <cstring>

/* native_carstate.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  None of these calls
 * the game's other functions. */
namespace nfs3hp
{
namespace carstate
{

// The game's 0x4217e0: the track's node player's camera keeps to.
nfs::u32 nodeOfCamera(nfs::App* app, nfs::u32 player);
// The game's 0x4227e0: for a view of mode 0 its player's camera's zoom's bits, where that is not 0.
std::optional<nfs::u32> ownZoom(nfs::App* app, nfs::u32 view);
// ownZoom as the game's code hands it on: the zoom, or 1.0.
inline float zoomOfView(nfs::App* app, nfs::u32 view)
{
    const std::optional<nfs::u32> own = ownZoom(app, view);
    if (!own)
        return 1.0f;
    float zoom;
    std::memcpy(&zoom, &*own, sizeof zoom);
    return zoom;
}
// The game's 0x422aa0: whether player sees from inside its car.
bool insideView(nfs::App* app, nfs::u32 player);
// The game's 0x422ba0: the car player's camera rides with.
nfs::u32 carOfCamera(nfs::App* app, nfs::u32 player);
// The game's 0x422bd0: the car player's camera looks at.
nfs::u32 targetOfCamera(nfs::App* app, nfs::u32 player);
// The game's 0x423b00: the mode of player's camera.
nfs::u32 modeOfCamera(nfs::App* app, nfs::u32 player);
// The game's 0x423fb0: the float with the bits `value`, scaled where the record's +4 is set.
float scaledWhereSet(nfs::App* app, nfs::u32 record, nfs::u32 value);

/* What a view's copy leaves: the camera its place came from and that camera's mode. */
struct ViewCopy
{
    nfs::u32 camera;
    nfs::u32 mode;
};

// The game's 0x421450: the view kept (0x552ec4) and set the way `how` says (0 by its camera, 2 as a screen's); eax.
std::optional<nfs::u32> cameraViewChosen(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 how);
// The game's 0x421490: player's chase camera's node ahead, picked and kept; the node.
nfs::u32 chaseNodePicked(nfs::App* app, nfs::u32 player);
// The game's 0x4215c0: player's cameras put on car; eax as the code leaves it.
std::optional<nfs::u32> camerasAttached(nfs::App* app, nfs::ThreadState& ts, nfs::u32 player, nfs::u32 car);
// The game's 0x422290: the camera made to look at its target from the place; eax.
std::optional<nfs::u32> cameraLookedAt(nfs::App* app, nfs::ThreadState& ts, nfs::u32 camera, nfs::u32 place);
// The game's 0x422610: player's camera switched to the mode, blending; eax.
std::optional<nfs::u32> cameraSwitched(nfs::App* app, nfs::ThreadState& ts, nfs::u32 mode, nfs::u32 player);
// The game's 0x4229c0: player's view copied to `to`; the camera its place came from and its mode.
ViewCopy viewCopied(nfs::App* app, nfs::u32 player, nfs::u32 to);
// The game's 0x422cf0: the race's cameras set up; eax.
std::optional<nfs::u32> camerasSetUp(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4238b0: player's camera put to the mode; eax.
std::optional<nfs::u32> cameraModePut(nfs::App* app, nfs::ThreadState& ts, nfs::u32 player, nfs::u32 mode);
// The game's 0x423960: player's overriding camera put to the mode (0 none); eax.
std::optional<nfs::u32> overrideModePut(nfs::App* app, nfs::ThreadState& ts, nfs::u32 player, nfs::u32 mode);
// The game's 0x423a60: the cycle of camera modes built (it leaves eax 0).
void cycleBuilt(nfs::App* app);
// The game's 0x423ab0: whether the mode is in the cycle (eax 1 or 0).
bool modeInTheCycle(nfs::App* app, nfs::u32 mode);
// The game's 0x423ae0: the mode kept as the player's last if it is in the cycle; whether it is (eax 1 or 0).
bool modeRemembered(nfs::App* app, nfs::u32 player, nfs::u32 mode);
// The game's 0x423b80: player's camera brought back to what the player chose; eax.
std::optional<nfs::u32> cameraSettled(nfs::App* app, nfs::ThreadState& ts, nfs::u32 player);
// The game's 0x423d90: the spot nearest player's car, or -1.
std::optional<nfs::u32> spotNearestTo(nfs::App* app, nfs::ThreadState& ts, nfs::u32 player);
// The game's 0x423fe0: whether the road's flags are not a single run of bits (eax 1 or 0).
bool roadBitsGone(nfs::u32 flags);
// The game's 0x424030: the vector damped by the weight through the car's slip; eax as the code leaves it.
nfs::u32 vectorDamped(nfs::App* app, nfs::u32 car, nfs::u32 weight, nfs::u32 vector);

}  // namespace carstate

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
