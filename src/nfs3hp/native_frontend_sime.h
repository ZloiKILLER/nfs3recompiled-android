#ifndef NFS3HP_NATIVE_FRONTEND_SIME_H
#define NFS3HP_NATIVE_FRONTEND_SIME_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_sime.cpp's typed functions (the cars' numbered folders, the fog at a camera, the
 * root table and a quick normal, the race's effects: skid strips, wheel marks, particles, sparks,
 * puffs, drops; the views resized, the track's pieces loaded), which other files call as C++
 * (guest::direct), and how the game's code calls each by address (ByAddress, for the checks'
 * stand-ins).  Each calls the game's other functions by address on `cpu` and throws fe::Terminated
 * when the game stopped in one, as Guest::call does; its result is what the game's function leaves
 * in eax, and what it leaves in the registers it does not save stays there.  A function handed a
 * record on the stack takes the address of the words the game's code pushes (`on`, `parent`): a
 * C++ caller puts them where those pushes put them. */
namespace nfs3hp
{
namespace frontend
{
namespace sime
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x43bcf0: the cars' folders named by a number removed.
reg32 removeNumberedFolders(App* app, CPU& cpu);
// The game's 0x478e10: the fog's colour and distance at a view's camera.
reg32 fogAtCamera(App* app, CPU& cpu, reg32 view, reg32 colourOut, reg32 distanceOut);
// The game's 0x496310: the table of reciprocal roots made.
reg32 makeRootTable(App* app, CPU& cpu);
// The game's 0x497500: the vector made about one long without a root, to `out`; eax the vector.
reg32 quickNormal(App* app, CPU& cpu, reg32 vector, reg32 out);
// The game's 0x4bc8b0: the race's effects reset.
reg32 effectsReset(App* app, CPU& cpu);
// The game's 0x4bc950: the race's effects freed.
reg32 effectsFree(App* app, CPU& cpu);
// The game's 0x4be970: the views made larger or smaller.
reg32 viewsResize(App* app, CPU& cpu, reg32 direction);
// The game's 0x4bea40: a view made larger or smaller by `change`.
reg32 viewResize(App* app, CPU& cpu, reg32 view, reg32 change);
// The game's 0x4c6c70: a skid strip's segment put.
reg32 skidSegmentPut(App* app, CPU& cpu, reg32 corners, reg32 alphas, reg32 from, reg32 to, reg32 alphaFrom, reg32 alphaTo);
// The game's 0x4c70f0: the skid strips' memory taken.
reg32 skidStripsTake(App* app, CPU& cpu);
// The game's 0x4c7130: the skid strips reset.
reg32 skidStripsReset(App* app, CPU& cpu);
// The game's 0x4c7150: the skid strips' memory freed.
reg32 skidStripsFree(App* app, CPU& cpu);
// The game's 0x4c7b20: a race's end: nothing, eax as it came.
reg32 effectsEndEmpty(App* app, CPU& cpu, reg32 eax);
// The game's 0x4ca160: the strip under a car's wheels.
reg32 stripUnderWheels(App* app, CPU& cpu, reg32 car, reg32 out);
// The game's 0x4ca4b0: a slot for a new particle.
reg32 particleSlot(App* app, CPU& cpu);
// The game's 0x4ca500: the rain's drops on a player's view.
reg32 dropsOnView(App* app, CPU& cpu, reg32 player, reg32 on);
// The game's 0x4ca850: a particle of a kind where the words at `on` say; 0 or the particle.
reg32 spawnParticle(App* app, CPU& cpu, reg32 kind, reg32 on);
// The game's 0x4cab60: a dying particle's children: the particle a copy at `parent`.
reg32 particleChildren(App* app, CPU& cpu, reg32 parent);
// The game's 0x4cac20: sparks off a car where the words at `on` say.
reg32 sparks(App* app, CPU& cpu, reg32 car, reg32 on);
// The game's 0x4cae00: a puff of a kind where the words at `on` say.
reg32 crashPuff(App* app, CPU& cpu, reg32 kind, reg32 on);
// The game's 0x4caea0: the sparkle round a car as it is put back.
reg32 respawnSparkle(App* app, CPU& cpu, reg32 car, reg32 on);
// The game's 0x4cb010: a wheel's dust of a kind off a car.
reg32 wheelParticles(App* app, CPU& cpu, reg32 car, reg32 wheel, reg32 on);
// The game's 0x4d5730: a race's end (0x4d56b0), every register handed on.
reg32 effectsEndNothing(App* app, CPU& cpu);
// The game's 0x4db860: a wheel's mark cut.
reg32 wheelMarkCut(App* app, CPU& cpu, reg32 car, reg32 wheel);
// The game's 0x4dbba0: the wheel marks reset as the race loads.
reg32 wheelMarksResetAtLoad(App* app, CPU& cpu);
// The game's 0x4dbbc0: the wheel marks reset.
reg32 wheelMarksReset(App* app, CPU& cpu);
// The game's 0x4dbbe0: a race's end for the marks: nothing, eax as it came.
reg32 wheelMarksEndEmpty(App* app, CPU& cpu, reg32 eax);
// The game's 0x4dbbf0: the track's pieces loaded.
reg32 loadTrackPieces(App* app, CPU& cpu, reg32 track);

}  // namespace sime
}  // namespace frontend

namespace guest
{

template <>
struct ByAddress<frontend::sime::removeNumberedFolders>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// `view` in eax; `colourOut` in edx; `distanceOut` in ebx.
template <>
struct ByAddress<frontend::sime::fogAtCamera>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 colourOut, x86::reg32 distanceOut)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view, colourOut, distanceOut }));
    }
};

template <>
struct ByAddress<frontend::sime::makeRootTable>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// `vector` in eax; `out` in edx.
template <>
struct ByAddress<frontend::sime::quickNormal>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 vector, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ vector, out }));
    }
};

template <>
struct ByAddress<frontend::sime::effectsReset>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::sime::effectsFree>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// `direction` in eax.
template <>
struct ByAddress<frontend::sime::viewsResize>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 direction)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ direction }));
    }
};

// `view` in eax; `change` in edx.
template <>
struct ByAddress<frontend::sime::viewResize>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 change)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ view, change }));
    }
};

// `corners` in eax; `alphas` in edx; `from` in ebx; `to` in ecx; `alphaFrom`, `alphaTo` on the stack (ret 8).
template <>
struct ByAddress<frontend::sime::skidSegmentPut>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 corners, x86::reg32 alphas, x86::reg32 from, x86::reg32 to, x86::reg32 alphaFrom, x86::reg32 alphaTo)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ corners, alphas, from, to }, { alphaFrom, alphaTo }));
    }
};

template <>
struct ByAddress<frontend::sime::skidStripsTake>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::sime::skidStripsReset>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::sime::skidStripsFree>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// `eax` in eax.
template <>
struct ByAddress<frontend::sime::effectsEndEmpty>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ eax }));
    }
};

// `car` in eax; `out` in edx.
template <>
struct ByAddress<frontend::sime::stripUnderWheels>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car, out }));
    }
};

template <>
struct ByAddress<frontend::sime::particleSlot>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// `player` in eax; the 4 words at `on` on the stack (ret 16).
template <>
struct ByAddress<frontend::sime::dropsOnView>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player, x86::reg32 on)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ player }, { guest::at(app, on), guest::at(app, on + 4), guest::at(app, on + 8), guest::at(app, on + 12) }));
    }
};

// `kind` in eax; the 13 words at `on` on the stack (ret 52).
template <>
struct ByAddress<frontend::sime::spawnParticle>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 kind, x86::reg32 on)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ kind }, { guest::at(app, on), guest::at(app, on + 4), guest::at(app, on + 8), guest::at(app, on + 12), guest::at(app, on + 16), guest::at(app, on + 20), guest::at(app, on + 24), guest::at(app, on + 28), guest::at(app, on + 32), guest::at(app, on + 36), guest::at(app, on + 40), guest::at(app, on + 44), guest::at(app, on + 48) }));
    }
};

// The 32 words at `parent` on the stack (ret 128): more than fe::Arguments holds, put there here.
template <>
struct ByAddress<frontend::sime::particleChildren>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 parent)
    {
        const x86::reg32 esp = cpu.esp;
        cpu.esp -= 32 * 4;
        for (x86::reg32 i = 0; i < 32; ++i)
            guest::put(app, cpu.esp + 4 * i, guest::at(app, parent + 4 * i));
        fe::Guest guest(app, cpu);
        const x86::reg32 eax = guest.call(Address);
        cpu.esp = esp;
        return eax;
    }
};

// `car` in eax; the 4 words at `on` on the stack (ret 16).
template <>
struct ByAddress<frontend::sime::sparks>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 on)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }, { guest::at(app, on), guest::at(app, on + 4), guest::at(app, on + 8), guest::at(app, on + 12) }));
    }
};

// `kind` in eax; the 3 words at `on` on the stack (ret 12).
template <>
struct ByAddress<frontend::sime::crashPuff>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 kind, x86::reg32 on)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ kind }, { guest::at(app, on), guest::at(app, on + 4), guest::at(app, on + 8) }));
    }
};

// `car` in eax; the 3 words at `on` on the stack (ret 12).
template <>
struct ByAddress<frontend::sime::respawnSparkle>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 on)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car }, { guest::at(app, on), guest::at(app, on + 4), guest::at(app, on + 8) }));
    }
};

// `car` in eax; `wheel` in edx; the 4 words at `on` on the stack (ret 16).
template <>
struct ByAddress<frontend::sime::wheelParticles>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 wheel, x86::reg32 on)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car, wheel }, { guest::at(app, on), guest::at(app, on + 4), guest::at(app, on + 8), guest::at(app, on + 12) }));
    }
};

template <>
struct ByAddress<frontend::sime::effectsEndNothing>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// `car` in eax; `wheel` in edx.
template <>
struct ByAddress<frontend::sime::wheelMarkCut>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 wheel)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ car, wheel }));
    }
};

template <>
struct ByAddress<frontend::sime::wheelMarksResetAtLoad>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::sime::wheelMarksReset>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// `eax` in eax.
template <>
struct ByAddress<frontend::sime::wheelMarksEndEmpty>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 eax)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ eax }));
    }
};

// `track` in eax.
template <>
struct ByAddress<frontend::sime::loadTrackPieces>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 track)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ track }));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
