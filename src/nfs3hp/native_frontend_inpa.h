#ifndef NFS3HP_NATIVE_FRONTEND_INPA_H
#define NFS3HP_NATIVE_FRONTEND_INPA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_inpa.cpp's typed functions (the race's main loop and what hangs off it: a frame's
 * keys, a network race's hold, the set-up, the controls' queues and their locks), which other files
 * call as C++ (guest::direct), and how the game's code calls each by address (ByAddress, for the
 * checks' stand-ins).  Each calls the game's other functions by address on `cpu` and throws
 * fe::Terminated when the game stopped in one, as Guest::call does; its result is what the game's
 * function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace inpa
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4c4ac0: the developer's keys while paused: the freeze, single steps and their way.
reg32 raceDebugKeys(App* app, CPU& cpu);
// The game's 0x4c4be0: the pause menu's turn in a frame: back to the race, again, or out of it.
reg32 pauseMenuStep(App* app, CPU& cpu);
// The game's 0x4c4e40: the race, every pass of it, until it is over.
reg32 runRaceLoop(App* app, CPU& cpu);
// The game's 0x4c5520: the network's lock made.
reg32 networkLockMake(App* app, CPU& cpu);
// The game's 0x4c5540: a network race's hold forgotten.
reg32 holdClear(App* app, CPU& cpu);
// The game's 0x4c5560: the network's lock freed, the connection put back when the game is over here.
reg32 networkLockFree(App* app, CPU& cpu);
// The game's 0x4c55a0: a network race's hold (asked for when `mayAsk`): 1 once the race goes on.
reg32 networkHold(App* app, CPU& cpu, reg32 mayAsk);
// The game's 0x4c62a0: what a race needs before its first frame; 1 when it is given up.
reg32 raceSetUp(App* app, CPU& cpu);
// The game's 0x4c63c0: the input's locks made, the queue held stopped; not for a replay.
reg32 inputLocksMake(App* app, CPU& cpu);
// The game's 0x4c63f0: the controls' queues back to their start.
reg32 inputReset(App* app, CPU& cpu);
// The game's 0x4c64e0: the input's locks freed, unless the race was given up or is a replay.
reg32 inputLocksFree(App* app, CPU& cpu);
// The game's 0x4c7b30: the queue's lock made, once.
reg32 queueLockMake(App* app, CPU& cpu);
// The game's 0x4c7bb0: the queue's lock freed.
reg32 queueLockFree(App* app, CPU& cpu);

}  // namespace inpa
}  // namespace frontend

namespace guest
{

template <>
struct ByAddress<frontend::inpa::raceDebugKeys>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::inpa::pauseMenuStep>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::inpa::runRaceLoop>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::inpa::networkLockMake>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::inpa::holdClear>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::inpa::networkLockFree>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

// Whether it may be asked for in eax.
template <>
struct ByAddress<frontend::inpa::networkHold>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mayAsk)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ mayAsk }));
    }
};

template <>
struct ByAddress<frontend::inpa::raceSetUp>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::inpa::inputLocksMake>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::inpa::inputReset>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::inpa::inputLocksFree>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::inpa::queueLockMake>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::inpa::queueLockFree>
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
