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
x86::reg32 nodeOfCamera(win32::WinApplication* app, x86::reg32 player);
// The game's 0x4227e0: for a view of mode 0 its player's camera's zoom's bits, where that is not 0.
std::optional<x86::reg32> ownZoom(win32::WinApplication* app, x86::reg32 view);
// ownZoom as the game's code hands it on: the zoom, or 1.0.
inline float zoomOfView(win32::WinApplication* app, x86::reg32 view)
{
    const std::optional<x86::reg32> own = ownZoom(app, view);
    if (!own)
        return 1.0f;
    float zoom;
    std::memcpy(&zoom, &*own, sizeof zoom);
    return zoom;
}
// The game's 0x422aa0: whether player sees from inside its car.
bool insideView(win32::WinApplication* app, x86::reg32 player);
// The game's 0x422ba0: the car player's camera rides with.
x86::reg32 carOfCamera(win32::WinApplication* app, x86::reg32 player);
// The game's 0x422bd0: the car player's camera looks at.
x86::reg32 targetOfCamera(win32::WinApplication* app, x86::reg32 player);
// The game's 0x423b00: the mode of player's camera.
x86::reg32 modeOfCamera(win32::WinApplication* app, x86::reg32 player);
// The game's 0x423fb0: the float with the bits `value`, scaled where the record's +4 is set.
float scaledWhereSet(win32::WinApplication* app, x86::reg32 record, x86::reg32 value);

/* What a view's copy leaves: the camera its place came from and that camera's mode. */
struct ViewCopy
{
    x86::reg32 camera;
    x86::reg32 mode;
};

// The game's 0x421450: the view kept (0x552ec4) and set the way `how` says (0 by its camera, 2 as a screen's); eax.
std::optional<x86::reg32> cameraViewChosen(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 how);
// The game's 0x421490: player's chase camera's node ahead, picked and kept; the node.
x86::reg32 chaseNodePicked(win32::WinApplication* app, x86::reg32 player);
// The game's 0x4215c0: player's cameras put on car; eax as the code leaves it.
std::optional<x86::reg32> camerasAttached(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player, x86::reg32 car);
// The game's 0x422290: the camera made to look at its target from the place; eax.
std::optional<x86::reg32> cameraLookedAt(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera, x86::reg32 place);
// The game's 0x422610: player's camera switched to the mode, blending; eax.
std::optional<x86::reg32> cameraSwitched(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mode, x86::reg32 player);
// The game's 0x4229c0: player's view copied to `to`; the camera its place came from and its mode.
ViewCopy viewCopied(win32::WinApplication* app, x86::reg32 player, x86::reg32 to);
// The game's 0x422cf0: the race's cameras set up; eax.
std::optional<x86::reg32> camerasSetUp(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4238b0: player's camera put to the mode; eax.
std::optional<x86::reg32> cameraModePut(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player, x86::reg32 mode);
// The game's 0x423960: player's overriding camera put to the mode (0 none); eax.
std::optional<x86::reg32> overrideModePut(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player, x86::reg32 mode);
// The game's 0x423a60: the cycle of camera modes built (it leaves eax 0).
void cycleBuilt(win32::WinApplication* app);
// The game's 0x423ab0: whether the mode is in the cycle (eax 1 or 0).
bool modeInTheCycle(win32::WinApplication* app, x86::reg32 mode);
// The game's 0x423ae0: the mode kept as the player's last if it is in the cycle; whether it is (eax 1 or 0).
bool modeRemembered(win32::WinApplication* app, x86::reg32 player, x86::reg32 mode);
// The game's 0x423b80: player's camera brought back to what the player chose; eax.
std::optional<x86::reg32> cameraSettled(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player);
// The game's 0x423d90: the spot nearest player's car, or -1.
std::optional<x86::reg32> spotNearestTo(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player);
// The game's 0x423fe0: whether the road's flags are not a single run of bits (eax 1 or 0).
bool roadBitsGone(x86::reg32 flags);
// The game's 0x424030: the vector damped by the weight through the car's slip; eax as the code leaves it.
x86::reg32 vectorDamped(win32::WinApplication* app, x86::reg32 car, x86::reg32 weight, x86::reg32 vector);

}  // namespace carstate

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
