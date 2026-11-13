#ifndef NFS3HP_NATIVE_RACELOAD_H
#define NFS3HP_NATIVE_RACELOAD_H

#include "native_guest.h"

#include <cstdint>

/* native_raceload.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Each may call the
 * game's other functions by address on `cpu`, and is false or nullopt when the game stopped in one. */
namespace nfs3hp
{
namespace raceload
{

// The game's 0x42ce10: car's distance into the race (+8) from its laps and node.
void setRaceDistance(win32::WinApplication* app, x86::reg32 car);
// The game's 0x42d160: car's shadow and lights' frame placed; false when it has no shadow.
std::optional<bool> placeShadow(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x42d500: a hook that does nothing.
void noHook(win32::WinApplication* app);
// The game's 0x42d520: another hook that does nothing.
void noHook2(win32::WinApplication* app);
// The game's 0x42d7f0: the message box closed and the message lines with it.
bool closeAllMessages(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x42dd00: the race controls' tick taken from the game's timer, if it was given.
bool stopTick(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x42dd60: whether the corner (the three signs) of other's box is inside box's.
bool cornerIn(win32::WinApplication* app, x86::reg32 box, x86::reg32 other, std::int32_t firstSign,
              std::int32_t secondSign, std::int32_t thirdSign);
// The game's 0x42e010: cornerIn the other way round, box's corner in other's box.
bool cornerInBack(win32::WinApplication* app, x86::reg32 box, x86::reg32 other, std::int32_t firstSign,
                  std::int32_t secondSign, std::int32_t thirdSign);
// The game's 0x42e2c0: cornerIn with the corner from other's axes worked out the other way about.
bool cornerIn2(win32::WinApplication* app, x86::reg32 box, x86::reg32 other, std::int32_t firstSign,
               std::int32_t secondSign, std::int32_t thirdSign);
// The game's 0x42e4f0: cornerInBack worked out as cornerIn2 is.
bool cornerInBack2(win32::WinApplication* app, x86::reg32 box, x86::reg32 other, std::int32_t firstSign,
                   std::int32_t secondSign, std::int32_t thirdSign);
// The game's 0x42ee10: car hits the wall (its normal, its point) at point; false when it moved away.
std::optional<bool> hit(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 point,
                        x86::reg32 normal, x86::reg32 wall);
// The game's 0x42efe0: car's box against the wall of normal through point within the float at
// `reach` (the address of the word the code pushes); what hit gave, or 0.
std::optional<x86::reg32> boxHits(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 normal,
                                  x86::reg32 point, x86::reg32 reach);
// The game's 0x42f280: the cars a and b's collision at point answered.
bool respond(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 point);

}  // namespace raceload

namespace guest
{

// The car in eax.
template <>
struct ByAddress<raceload::setRaceDistance>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
    }
};

// The car in eax; eax the car back when it has no shadow, else an address in the code's frame.
template <>
struct ByAddress<raceload::placeShadow>
{
    template <x86::reg32 Address>
    static std::optional<bool> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        const std::optional<x86::reg32> left = watcomEax(app, cpu, Address, {car});
        if (!left)
            return std::nullopt;
        return *left != car;
    }
};

template <>
struct ByAddress<raceload::noHook>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
    }
};

template <>
struct ByAddress<raceload::noHook2>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
    }
};

template <>
struct ByAddress<raceload::closeAllMessages>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcom(app, cpu, Address, {});
    }
};

template <>
struct ByAddress<raceload::stopTick>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcom(app, cpu, Address, {});
    }
};

// The corner tests': box in eax, other in edx, the first two signs in ebx and ecx, the third on
// the stack (ret 4); eax 1 inside, else 0.
struct RaceloadCornerTest
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 box, x86::reg32 other,
                     std::int32_t firstSign, std::int32_t secondSign, std::int32_t thirdSign)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = box;
        cpu.edx = other;
        cpu.ebx = x86::reg32(firstSign);
        cpu.ecx = x86::reg32(secondSign);
        guest::call(app, cpu, Address, {x86::reg32(thirdSign)});
        return cpu.eax != 0;
    }
};

template <>
struct ByAddress<raceload::cornerIn> : RaceloadCornerTest
{
};

template <>
struct ByAddress<raceload::cornerInBack> : RaceloadCornerTest
{
};

template <>
struct ByAddress<raceload::cornerIn2> : RaceloadCornerTest
{
};

template <>
struct ByAddress<raceload::cornerInBack2> : RaceloadCornerTest
{
};

// The car in eax, the point in edx, the normal in ebx, the wall's point in ecx; eax 1, or 0.
template <>
struct ByAddress<raceload::hit>
{
    template <x86::reg32 Address>
    static std::optional<bool> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 point,
                                    x86::reg32 normal, x86::reg32 wall)
    {
        const std::optional<x86::reg32> hitIt = watcomEax(app, cpu, Address, {car, point, normal, wall});
        if (!hitIt)
            return std::nullopt;
        return *hitIt != 0;
    }
};

// The car in eax, the normal in edx, the point in ebx, the reach's word on the stack (ret 4).
template <>
struct ByAddress<raceload::boxHits>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car,
                                          x86::reg32 normal, x86::reg32 point, x86::reg32 reach)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = car;
        cpu.edx = normal;
        cpu.ebx = point;
        if (!guest::call(app, cpu, Address, {at(app, reach)}))
            return std::nullopt;
        return cpu.eax;
    }
};

// The cars in eax and edx, the point in ebx.
template <>
struct ByAddress<raceload::respond>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 point)
    {
        return watcom(app, cpu, Address, {a, b, point});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
