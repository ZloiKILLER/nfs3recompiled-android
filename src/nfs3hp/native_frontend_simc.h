#ifndef NFS3HP_NATIVE_FRONTEND_SIMC_H
#define NFS3HP_NATIVE_FRONTEND_SIMC_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_simc.cpp's typed functions (quaternions and matrices, a view's pass, the replay's
 * panel, the television director, small helpers, the view widget), which other files call as C++
 * (guest::direct), and how the game's code calls each by address (ByAddress, for the checks'
 * stand-ins).  Each calls the game's other functions by address on `cpu` and throws fe::Terminated
 * when the game stopped in one, as Guest::call does; its result is what the game's function leaves
 * in eax, and what it leaves on the x87's stack and in the registers it does not save stays there.
 * A stack argument the game's code hands by its address (`...At`) is taken as that address. */
namespace nfs3hp
{
namespace frontend
{
namespace simc
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4b70f0: a pose mixed between two by the float `t`.
reg32 poseBlend(App* app, CPU& cpu, reg32 quatA, reg32 quatB, reg32 placeA, reg32 placeB, reg32 t, reg32 quatOut, reg32 placeOut);
// The game's 0x4b71e0: quaternion slerp by the float `t`.
reg32 quatSlerp(App* app, CPU& cpu, reg32 from, reg32 to, reg32 t, reg32 out);
// The game's 0x4b7300: the quaternion at `b` negated if its negative is nearer `a`.
reg32 quatNearer(App* app, CPU& cpu, reg32 a, reg32 b);
// The game's 0x4b73c0: the 3x3 matrix made a quaternion.
reg32 quatFromMatrix(App* app, CPU& cpu, reg32 matrix, reg32 out);
// The game's 0x4b7590: the quaternion made a 3x3 matrix.
reg32 matrixFromQuat(App* app, CPU& cpu, reg32 quat, reg32 out);
// The game's 0x4b7680: two 3x3 matrices slerped through quaternions.
reg32 matrixSlerp(App* app, CPU& cpu, reg32 from, reg32 to, reg32 t, reg32 out);
// The game's 0x4b7720: the 3x3 matrix `out` is `a` minus `b`.
reg32 matrixSubtract(App* app, CPU& cpu, reg32 a, reg32 b, reg32 out);
// The game's 0x4b7780: the 3x3 matrix `out` is `a` plus `b`.
reg32 matrixAdd(App* app, CPU& cpu, reg32 a, reg32 b, reg32 out);
// The game's 0x4b77e0: the 3x3 matrix times the float `factor`.
reg32 matrixScale(App* app, CPU& cpu, reg32 matrix, reg32 factor, reg32 out);
// The game's 0x4b7840: the 3x3 matrix made orthonormal again.
reg32 matrixOrthonormalise(App* app, CPU& cpu, reg32 matrix, reg32 terms);
// The game's 0x4b79c0: the 3x3 matrix turned a quarter.
reg32 matrixQuarterTurn(App* app, CPU& cpu, reg32 matrix);
// The game's 0x4ea680: the 3x3 matrix turned about its first row by the angle at `angleAt`.
reg32 turnAboutFirstRow(App* app, CPU& cpu, reg32 matrix, reg32 angleAt, reg32 out);
// The game's 0x4ea820: the 3x3 matrix turned about its last row by the angle at `angleAt`.
reg32 turnAboutLastRow(App* app, CPU& cpu, reg32 matrix, reg32 angleAt, reg32 out);
// The game's 0x4ea920: the square root of the float `value`, onto the x87.
void squareRoot(App* app, CPU& cpu, reg32 value);
// The game's 0x4bc470: a view's pass: the view's record filled, the frame drawn through it.
reg32 viewPass(App* app, CPU& cpu, reg32 pass, reg32 record4, reg32 record68, reg32 record6c, reg32 record70);
// The game's 0x4dbde0: the projection of the view.
reg32 setView(App* app, CPU& cpu, reg32 view);
// The game's 0x4d5720: the race frame's end (0x434a90).
reg32 raceFrameEnd(App* app, CPU& cpu);
// The game's 0x4bd4d0: the pointer on the replay's panel: the buttons' word of a press, else 0.
reg32 panelPointer(App* app, CPU& cpu);
// The game's 0x4bd640: the replay's panel each frame.
reg32 panelKeys(App* app, CPU& cpu);
// The game's 0x4bdb60: player's camera made `camera`, a new shot begun.
reg32 tvSetCamera(App* app, CPU& cpu, reg32 player, reg32 camera);
// The game's 0x4bdbd0: player's next shot.
reg32 tvNextCamera(App* app, CPU& cpu, reg32 player);
// The game's 0x4bdc70: 1 when player's shot has run its length.
reg32 tvShotOver(App* app, CPU& cpu, reg32 player);
// The game's 0x4bdcc0: player's shot begins now, as long as camera's are.
reg32 tvStartShot(App* app, CPU& cpu, reg32 player, reg32 camera);
// The game's 0x4bdd10: the largest of `count` rows of a column of the replay's table; edx and ebx left where they stop.
reg32 tvBusiest(App* app, CPU& cpu, reg32 column, reg32 row, reg32 count);
// The game's 0x4bdd70: the director's choice for player each frame.
reg32 tvDirect(App* app, CPU& cpu, reg32 player);
// The game's 0x4be000: al 1 for a digit or a sign, 0 for anything else.
reg32 isNumberCharacter(App* app, CPU& cpu, reg32 character);
// The game's 0x4be1a0: an off beat of the clock: 0 or 1.
reg32 offBeat(App* app, CPU& cpu, reg32 rate, reg32 offset);
// The game's 0x4c9dd0: siren `index` on at a node.
reg32 sirenOn(App* app, CPU& cpu, reg32 index, reg32 node, reg32 side, reg32 clock);
// The game's 0x4c9f50: siren `index` off.
reg32 sirenOff(App* app, CPU& cpu, reg32 index);
// The game's 0x4ea570: the angle of (x, y) in 16-bit fixed point.
reg32 angle(App* app, CPU& cpu, reg32 y, reg32 x);
// The game's 0x4ea654: how many entries the pack has.
reg32 packCount(App* app, CPU& cpu, reg32 pack);
// The game's 0x4ea658: entry `entry` of the pack, or 0 past its end.
reg32 packEntry(App* app, CPU& cpu, reg32 pack, reg32 entry);
// The game's 0x43bcc0: where on the car's interior picture its map starts.
reg32 interiorMapX(App* app, CPU& cpu, reg32 car);
// The game's 0x465320: the view widget's item placed.
reg32 viewLaidOut(App* app, CPU& cpu, reg32 item);
// The game's 0x465360: the view widget's empty entry: eax as it came.
reg32 viewNoEntry(App* app, CPU& cpu, reg32 eax);
// The game's 0x465370: the showcase's car's interior picture loaded and the view set back.
reg32 viewLoadCar(App* app, CPU& cpu);

}  // namespace simc
}  // namespace frontend

namespace guest
{

// `quatA` in eax; `quatB` in edx; `placeA` in ebx; `placeB` in ecx; `t`, `quatOut`, `placeOut` on the stack (ret 12).
template <>
struct ByAddress<frontend::simc::poseBlend>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 quatA, x86::reg32 quatB, x86::reg32 placeA, x86::reg32 placeB, x86::reg32 t, x86::reg32 quatOut, x86::reg32 placeOut)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ quatA, quatB, placeA, placeB }, { t, quatOut, placeOut }));
    }
};

// `from` in eax; `to` in edx; `t`, `out` on the stack (ret 8).
template <>
struct ByAddress<frontend::simc::quatSlerp>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 from, x86::reg32 to, x86::reg32 t, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ from, to }, { t, out }));
    }
};

// `a` in eax; `b` in edx.
template <>
struct ByAddress<frontend::simc::quatNearer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ a, b }));
    }
};

// `matrix` in eax; `out` in edx.
template <>
struct ByAddress<frontend::simc::quatFromMatrix>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 matrix, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ matrix, out }));
    }
};

// `quat` in eax; `out` in edx.
template <>
struct ByAddress<frontend::simc::matrixFromQuat>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 quat, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ quat, out }));
    }
};

// `from` in eax; `to` in edx; `t`, `out` on the stack (ret 8).
template <>
struct ByAddress<frontend::simc::matrixSlerp>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 from, x86::reg32 to, x86::reg32 t, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ from, to }, { t, out }));
    }
};

// `a` in eax; `b` in edx; `out` in ebx.
template <>
struct ByAddress<frontend::simc::matrixSubtract>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ a, b, out }));
    }
};

// `a` in eax; `b` in edx; `out` in ebx.
template <>
struct ByAddress<frontend::simc::matrixAdd>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ a, b, out }));
    }
};

// `matrix` in eax; `factor`, `out` on the stack (ret 8).
template <>
struct ByAddress<frontend::simc::matrixScale>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 matrix, x86::reg32 factor, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ matrix }, { factor, out }));
    }
};

// `matrix` in eax; `terms` in edx.
template <>
struct ByAddress<frontend::simc::matrixOrthonormalise>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 matrix, x86::reg32 terms)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ matrix, terms }));
    }
};

// `matrix` in eax.
template <>
struct ByAddress<frontend::simc::matrixQuarterTurn>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 matrix)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ matrix }));
    }
};

// `matrix` in eax; the 1 words at `angleAt`, `out` on the stack (ret 8).
template <>
struct ByAddress<frontend::simc::turnAboutFirstRow>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 matrix, x86::reg32 angleAt, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ matrix }, { guest::at(app, angleAt), out }));
    }
};

// `matrix` in eax; the 1 words at `angleAt`, `out` on the stack (ret 8).
template <>
struct ByAddress<frontend::simc::turnAboutLastRow>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 matrix, x86::reg32 angleAt, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ matrix }, { guest::at(app, angleAt), out }));
    }
};

// `value` on the stack (ret 4).
template <>
struct ByAddress<frontend::simc::squareRoot>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 value)
    {
        fe::callByAddress(app, cpu, Address, fe::args({}, { value }));
    }
};

// `pass` in eax; `record4` in edx; `record68` in ebx; `record6c` in ecx; `record70` on the stack (ret 4).
template <>
struct ByAddress<frontend::simc::viewPass>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 pass, x86::reg32 record4, x86::reg32 record68, x86::reg32 record6c, x86::reg32 record70)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ pass, record4, record68, record6c }, { record70 }));
    }
};

// `view` in eax.
template <>
struct ByAddress<frontend::simc::setView>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view }));
    }
};

template <>
struct ByAddress<frontend::simc::raceFrameEnd>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::simc::panelPointer>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::simc::panelKeys>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// `player` in eax; `camera` in edx.
template <>
struct ByAddress<frontend::simc::tvSetCamera>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player, x86::reg32 camera)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ player, camera }));
    }
};

// `player` in eax.
template <>
struct ByAddress<frontend::simc::tvNextCamera>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ player }));
    }
};

// `player` in eax.
template <>
struct ByAddress<frontend::simc::tvShotOver>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ player }));
    }
};

// `player` in eax; `camera` in edx.
template <>
struct ByAddress<frontend::simc::tvStartShot>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player, x86::reg32 camera)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ player, camera }));
    }
};

// `column` in eax; `row` in edx; `count` in ebx.
template <>
struct ByAddress<frontend::simc::tvBusiest>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 column, x86::reg32 row, x86::reg32 count)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ column, row, count }));
    }
};

// `player` in eax.
template <>
struct ByAddress<frontend::simc::tvDirect>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ player }));
    }
};

// `character` in eax.
template <>
struct ByAddress<frontend::simc::isNumberCharacter>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 character)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ character }));
    }
};

// `rate` in eax; `offset` in edx.
template <>
struct ByAddress<frontend::simc::offBeat>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 rate, x86::reg32 offset)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ rate, offset }));
    }
};

// `index` in eax; `node` in edx; `side` in ebx; `clock` in ecx.
template <>
struct ByAddress<frontend::simc::sirenOn>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index, x86::reg32 node, x86::reg32 side, x86::reg32 clock)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index, node, side, clock }));
    }
};

// `index` in eax.
template <>
struct ByAddress<frontend::simc::sirenOff>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 index)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ index }));
    }
};

// `y` in eax; `x` in edx.
template <>
struct ByAddress<frontend::simc::angle>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 y, x86::reg32 x)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ y, x }));
    }
};

// `pack` in eax.
template <>
struct ByAddress<frontend::simc::packCount>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 pack)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ pack }));
    }
};

// `pack` in eax; `entry` in edx.
template <>
struct ByAddress<frontend::simc::packEntry>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 pack, x86::reg32 entry)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ pack, entry }));
    }
};

// `car` in eax.
template <>
struct ByAddress<frontend::simc::interiorMapX>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }));
    }
};

// `item` in eax.
template <>
struct ByAddress<frontend::simc::viewLaidOut>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

// `eax` in eax.
template <>
struct ByAddress<frontend::simc::viewNoEntry>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ eax }));
    }
};

template <>
struct ByAddress<frontend::simc::viewLoadCar>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
