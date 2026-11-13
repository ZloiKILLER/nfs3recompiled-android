#ifndef NFS3HP_NATIVE_FRONTEND_SIMA_H
#define NFS3HP_NATIVE_FRONTEND_SIMA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_sima.cpp's typed functions (the opponents' engine and heading, the race cameras'
 * modes, a car's place on the track), which other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the checks' stand-ins).  Each calls the game's
 * other functions by address on `cpu` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax, and what it leaves on the
 * x87's stack stays there. */
namespace nfs3hp
{
namespace frontend
{
namespace sima
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x40de30: an opponent's lane votes added up and acted on.
reg32 laneSteer(App* app, CPU& cpu, reg32 car);
// The game's 0x4064f0: the pull of an opponent's engine at its speed, onto the x87.
reg32 enginePull(App* app, CPU& cpu, reg32 car);
// The game's 0x4070c0: how far the way the driver wants to go is from the way its car points.
reg32 headingError(App* app, CPU& cpu, reg32 car);
// The game's 0x425060: camera mode 8, beside the road near the car.
reg32 cameraBesideRoad(App* app, CPU& cpu, reg32 camera);
// The game's 0x4251f0: camera mode 18, the nearest of the track's fixed spots.
reg32 cameraNearestSpot(App* app, CPU& cpu, reg32 camera);
// The game's 0x4253d0: camera mode 16, an animated path.
reg32 cameraPlayPath(App* app, CPU& cpu, reg32 camera);
// The game's 0x425640: camera mode 17, the path at a race's end.
reg32 cameraEndPath(App* app, CPU& cpu, reg32 camera);
// The game's 0x425bf0: camera mode 14, beside the road, higher up.
reg32 cameraOverRoad(App* app, CPU& cpu, reg32 camera);
// The game's 0x425d50: camera mode 9, the flyover.
reg32 cameraFlyover(App* app, CPU& cpu, reg32 camera);
// The game's 0x426290: camera mode 20's step: nothing.
reg32 cameraStill(App* app, CPU& cpu, reg32 camera);
// The game's 0x4262a0: camera mode 19, the mode it was told for a while, then the view handed on.
reg32 cameraWaitThenHandOn(App* app, CPU& cpu, reg32 camera);
// The game's 0x426310: a view under mode 19 handed straight on to the mode it was waiting for.
reg32 cameraBackFromWait(App* app, CPU& cpu, reg32 view);
// The game's 0x426450: a mode's camera of a view prepared and stepped.
reg32 cameraStep(App* app, CPU& cpu, reg32 mode, reg32 view);
// The game's 0x426560: the mode a view's car forces now.
reg32 forcedMode(App* app, CPU& cpu, reg32 view);
// The game's 0x426680: every view's camera stepped.
reg32 camerasStep(App* app, CPU& cpu);
// The game's 0x4965b0: the distance between the points at `a` and `b` without a root, onto the x87.
reg32 quickDistance(App* app, CPU& cpu, reg32 a, reg32 b);
// The game's 0x4967e0: a length in the plane without a root, of two floats (their bits), onto the x87.
void planarLength(App* app, CPU& cpu, reg32 a, reg32 b);
// The game's 0x49fde0: whether a car leans over too far: 1 or 0.
reg32 carTiltedOver(App* app, CPU& cpu, reg32 car);
// The game's 0x49fee0: the track's heading at a slice, as a fixed angle.
reg32 sliceHeading(App* app, CPU& cpu, reg32 index);
// The game's 0x49ffa0: a car in the race given the track's frame where it is, and its heading.
reg32 carTrackFrame(App* app, CPU& cpu, reg32 car);
// The game's 0x4a00c0: how many nodes from `node` to `from` (0x40e120).
reg32 nodeGap(App* app, CPU& cpu, reg32 node, reg32 from);
// The game's 0x4a0140: a car moved back onto the road.
reg32 carOntoRoad(App* app, CPU& cpu, reg32 car);
// The game's 0x4a0580: how far a car is from the players, and what it does about it.
reg32 carNearPlayers(App* app, CPU& cpu, reg32 car);
// The game's 0x4a08c0: the frame's work on a car's place on the track.
reg32 carTrackEnvironment(App* app, CPU& cpu, reg32 car);

}  // namespace sima
}  // namespace frontend

namespace guest
{

/* Each by address as the game's code calls it: its arguments in eax and edx, or on the stack. */
template <>
struct ByAddress<frontend::sima::laneSteer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::sima::enginePull>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::sima::headingError>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::sima::cameraBesideRoad>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ camera }));
    }
};

template <>
struct ByAddress<frontend::sima::cameraNearestSpot>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ camera }));
    }
};

template <>
struct ByAddress<frontend::sima::cameraPlayPath>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ camera }));
    }
};

template <>
struct ByAddress<frontend::sima::cameraEndPath>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ camera }));
    }
};

template <>
struct ByAddress<frontend::sima::cameraOverRoad>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ camera }));
    }
};

template <>
struct ByAddress<frontend::sima::cameraFlyover>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ camera }));
    }
};

template <>
struct ByAddress<frontend::sima::cameraStill>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ camera }));
    }
};

template <>
struct ByAddress<frontend::sima::cameraWaitThenHandOn>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 camera)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ camera }));
    }
};

template <>
struct ByAddress<frontend::sima::cameraBackFromWait>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

// The mode in eax, the view in edx.
template <>
struct ByAddress<frontend::sima::cameraStep>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mode, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ mode, view }));
    }
};

template <>
struct ByAddress<frontend::sima::forcedMode>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

template <>
struct ByAddress<frontend::sima::camerasStep>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::sima::quickDistance>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ a, b }));
    }
};

// The two floats on the stack (ret 8).
template <>
struct ByAddress<frontend::sima::planarLength>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b)
    {
        fe::callByAddress(app, cpu, Address, fe::args({}, { a, b }));
    }
};

template <>
struct ByAddress<frontend::sima::carTiltedOver>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::sima::sliceHeading>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index }));
    }
};

template <>
struct ByAddress<frontend::sima::carTrackFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::sima::nodeGap>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 node, x86::reg32 from)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ node, from }));
    }
};

template <>
struct ByAddress<frontend::sima::carOntoRoad>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::sima::carNearPlayers>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::sima::carTrackEnvironment>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
