#ifndef NFS3HP_NATIVE_FRONTEND_SIMB_H
#define NFS3HP_NATIVE_FRONTEND_SIMB_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_simb.cpp's typed functions (a car on the track: put on the road, its probes and
 * the road ahead, what it goes into, its downforce and drag, its numbers on the road, a turn by
 * three angles), which other files call as C++ (guest::direct), and how the game's code calls each
 * by address (ByAddress, for the checks' stand-ins).  Each calls the game's other functions by
 * address on `cpu` and throws fe::Terminated when the game stopped in one, as Guest::call does; its
 * result is what the game's function leaves in eax, and what it leaves on the x87's stack stays
 * there. */
namespace nfs3hp
{
namespace frontend
{
namespace simb
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4a0aa0: the car put on node `node`, `offset` (three 16.16 words) from it, facing back or not.
reg32 placeOnRoad(App* app, CPU& cpu, reg32 car, reg32 node, reg32 offset, reg32 facesBack);
// The game's 0x4a1210: eight points round the car's box against the track's walls; 1 when one hit.
reg32 probeWalls(App* app, CPU& cpu, reg32 car);
// The game's 0x4a1610: the road a step ahead and what it does to the car, its up brought to the
// three floats the game's code pushes -- here at `wanted`, which it writes over as the game writes
// over its stack arguments.
reg32 predictContact(App* app, CPU& cpu, reg32 car, reg32 wanted);
// The game's 0x4a1ce0: what going into the surface of normal `normal` does to the car.
reg32 reactToSurface(App* app, CPU& cpu, reg32 car, reg32 normal);
// The game's 0x4a21a0: a car in a tunnel that has reached its node's roof.
reg32 hitTunnelRoof(App* app, CPU& cpu, reg32 car);
// The game's 0x4a2260: the downforce and the drag.
reg32 downforceAndDrag(App* app, CPU& cpu, reg32 car);
// The game's 0x4a2690: an active car's numbers as it stands on the road.
reg32 settle(App* app, CPU& cpu, reg32 car);
// The game's 0x4a2bc0: how far across the road the car is at its node, onto the x87.
reg32 offsetAcross(App* app, CPU& cpu, reg32 car);
// The game's 0x4a2cc0: a turn by three angles (floats' bits) as one matrix at `out`, their
// magnitudes added to the float at `total`; 1 when one of them is not nothing.
reg32 turnMatrix(App* app, CPU& cpu, reg32 out, reg32 angleX, reg32 angleY, reg32 angleZ, reg32 total);
// The game's 0x4a2f10: a thirty-second of the car's spin put on its frame.
reg32 spinThirtySecond(App* app, CPU& cpu, reg32 car);
// The game's 0x4a2f30: a sixty-fourth of it.
reg32 spinSixtyFourth(App* app, CPU& cpu, reg32 car);

}  // namespace simb
}  // namespace frontend

namespace guest
{

/* Each by address as the game's code calls it: its arguments in eax, edx, ebx and ecx, the rest
 * on the stack. */
template <>
struct ByAddress<frontend::simb::placeOnRoad>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 node,
                           x86::reg32 offset, x86::reg32 facesBack)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car, node, offset, facesBack }));
    }
};

template <>
struct ByAddress<frontend::simb::probeWalls>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

// The car in eax, the three words at `wanted` pushed (ret 12).
template <>
struct ByAddress<frontend::simb::predictContact>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 wanted)
    {
        return fe::callByAddress(app, cpu, Address,
                                 fe::args({ car }, { guest::at(app, wanted), guest::at(app, wanted + 4),
                                                     guest::at(app, wanted + 8) }));
    }
};

template <>
struct ByAddress<frontend::simb::reactToSurface>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 normal)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car, normal }));
    }
};

template <>
struct ByAddress<frontend::simb::hitTunnelRoof>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::simb::downforceAndDrag>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::simb::settle>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::simb::offsetAcross>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

// `out` in eax, the three angles and `total` on the stack (ret 16).
template <>
struct ByAddress<frontend::simb::turnMatrix>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 out, x86::reg32 angleX,
                           x86::reg32 angleY, x86::reg32 angleZ, x86::reg32 total)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ out }, { angleX, angleY, angleZ, total }));
    }
};

template <>
struct ByAddress<frontend::simb::spinThirtySecond>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

template <>
struct ByAddress<frontend::simb::spinSixtyFourth>
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
