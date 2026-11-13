#ifndef NFS3HP_NATIVE_CONTROLS_RACE_H
#define NFS3HP_NATIVE_CONTROLS_RACE_H

#include "native_guest.h"

/* native_controls_race.cpp's typed functions that other files call as C++ (guest::direct), and how
 * the game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one.
 * Their frames lie on the guest's stack at the code's depth, the registers the caller holds pushed
 * into them as the code pushes them. */
namespace nfs3hp
{
namespace controls
{

/* The registers a frame saves, as its caller holds them when it calls: what
 * the code pushes into it. */
struct Saved
{
    x86::reg32 ebx, ecx, edx, esi, edi, ebp;
};

/* What bump leaves in eax, ebx, ecx and edx, as the code leaves them. */
struct BumpLeft
{
    x86::reg32 answer;
    x86::reg32 ebxLeft;
    x86::reg32 ecxLeft;
    x86::reg32 edxLeft;
};

// The game's 0x471270: the jolt of `car` hit, the blow's place (three floats) and its strength (a
// float) the stack's words above the return address's slot; eax as the code leaves it.
std::optional<x86::reg32> crash(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// crash for a caller with the blow's place and strength in hand (their bits), through guest::direct:
// pushed as the game's code pushes them -- the strength, its first push, in the return address's slot
// direct takes under the caller's esp -- the return address's slot below them, as its call has them.
inline std::optional<x86::reg32> crashWith(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 x,
                                           x86::reg32 y, x86::reg32 z, x86::reg32 strength)
{
    const x86::reg32 esp = cpu.esp;
    guest::put(app, esp - 12, x);
    guest::put(app, esp - 8, y);
    guest::put(app, esp - 4, z);
    guest::put(app, esp, strength);
    cpu.esp = esp - 16;
    const std::optional<x86::reg32> jolt = crash(app, cpu, car);
    cpu.esp = esp;
    return jolt;
}
// The game's 0x471440: the jolt of `car` bumped, from the right for `side` above 0, softer for
// `softness` 0 and 2.
std::optional<BumpLeft> bump(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 side,
                             x86::reg32 softness);
// The game's 0x4714f0: the kick of `car`'s gear changed; eax as the code leaves it.
std::optional<x86::reg32> gearKick(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x471dc0: whether `car` is the one the race's forces are played from.
bool forceCar(win32::WinApplication* app, x86::reg32 car);
// The game's 0x471e10: a frame of the race's forces, `entry` the registers its caller holds; eax as
// the code leaves it.
std::optional<x86::reg32> forcesFrame(win32::WinApplication* app, x86::CPU& cpu, const Saved& entry);
// The game's 0x471fb0: every force-feedback device's actuators off, if a race has a force
// controller; eax as the code leaves it (`unchanged`, its eax, without one).
std::optional<x86::reg32> actuatorsOff(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 unchanged);
// The game's 0x472b60: the controllers found compared with config.dat's, the effects made, the menus
// told; eax as the code leaves it.
std::optional<x86::reg32> loadDevices(win32::WinApplication* app, x86::CPU& cpu);

}  // namespace controls

namespace guest
{

// The car in eax, the place and the strength pushed (stdcall, 4 words); eax back.
template <>
struct ByAddress<controls::crashWith>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 x,
                                          x86::reg32 y, x86::reg32 z, x86::reg32 strength)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = car;
        if (!guest::call(app, cpu, Address, {x, y, z, strength}))
            return std::nullopt;
        return cpu.eax;
    }
};

// The car in eax, the side in ebx, the softness in edx; eax, ebx, ecx and edx back.
template <>
struct ByAddress<controls::bump>
{
    template <x86::reg32 Address>
    static std::optional<controls::BumpLeft> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                                  x86::reg32 side, x86::reg32 softness)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = car;
        cpu.edx = softness;
        cpu.ebx = side;
        if (!guest::call(app, cpu, Address))
            return std::nullopt;
        return controls::BumpLeft{cpu.eax, cpu.ebx, cpu.ecx, cpu.edx};
    }
};

// The car in eax; eax back.
template <>
struct ByAddress<controls::gearKick>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcomEax(app, cpu, Address, {car});
    }
};

// The car in eax; eax 1 if so, else 0.
template <>
struct ByAddress<controls::forceCar>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        const std::optional<x86::reg32> is = watcomEax(app, cpu, Address, {car});
        return is && *is != 0;
    }
};

// Nothing in eax; the registers but eax `entry`; eax back.
template <>
struct ByAddress<controls::forcesFrame>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, const controls::Saved& entry)
    {
        const KeptRegisters kept(cpu);
        cpu.ebx = entry.ebx;
        cpu.ecx = entry.ecx;
        cpu.edx = entry.edx;
        cpu.esi = entry.esi;
        cpu.edi = entry.edi;
        cpu.ebp = entry.ebp;
        if (!guest::call(app, cpu, Address))
            return std::nullopt;
        return cpu.eax;
    }
};

// `unchanged` in eax; eax back.
template <>
struct ByAddress<controls::actuatorsOff>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 unchanged)
    {
        return watcomEax(app, cpu, Address, {unchanged});
    }
};

// Nothing handed; eax back.
template <>
struct ByAddress<controls::loadDevices>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
