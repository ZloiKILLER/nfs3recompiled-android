#ifndef NFS3HP_NATIVE_AI_RACE_H
#define NFS3HP_NATIVE_AI_RACE_H

#include "native_guest.h"

#include <cstdint>
#include <cstring>
#include <optional>

/* native_ai_race.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Those that take `cpu`
 * may call the game's other functions by address on it, and are false or nullopt when the game
 * stopped in one.  The ...Done ones are for callers that ask only whether the game went on: their
 * cores answer more than what the game's code leaves (the file's own types, kept there). */
namespace nfs3hp
{
namespace airace
{

// The game's 0x401240: whether car's input bit 0 was let go (or key 8 of its input is up).
std::optional<bool> wasReleased(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x4013b0: the gap in nodes from car to the nearest cop, 10000 at most.
std::optional<std::int32_t> nearestCopGapOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x401500: whether car's pursuit has it pulled over.
bool isPulledOver(win32::WinApplication* app, x86::reg32 car);
// The game's 0x401650: racer stops for cop `index` (its pursuit pulled over, a free cop joining).
bool stopForCopDone(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 racer, x86::reg32 index);
// The game's 0x4018b0: cop joins the pursuit of car (by `kind`; kind 0 keeps its speeding against `speed`); whether it joined.
std::optional<bool> joinPursuitOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 kind, x86::reg32 cop,
                                  x86::reg32 car, float speed);
// joinPursuitOf for a caller that asks only whether the game went on.
inline bool joinPursuitDone(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 kind, x86::reg32 cop,
                            x86::reg32 car, float speed)
{
    return joinPursuitOf(app, cpu, kind, cop, car, speed).has_value();
}
// The game's 0x401a80: cop gives up its pursuit (its radio's word, off the list, the getaway counted).
bool giveUpPursuitOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop);
// The game's 0x401e40: cop at a car pulled over: the nearest of several stops by it, the others drop off.
bool copStopsByDone(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop);
// The game's 0x401fb0: whether cop is done with a stop (its ticket given at the stop's end).
std::optional<bool> stopOverOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop);
// The game's 0x4029c0: cop placed at the race's start.
bool placeCopAtStartOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop);
// The game's 0x402c70: 1, whatever it is asked.
x86::reg32 alwaysOneOf(win32::WinApplication* app);
// The game's 0x4032e0: a cop's driver, each tick, by its step.
bool copDriverDone(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop);
// The game's 0x404610: car's class's rule.
x86::reg32 classRuleOf(win32::WinApplication* app, x86::reg32 car);
// The game's 0x404690: a racer's ramming rules by its class's kind.
void setRammingRulesOf(win32::WinApplication* app, x86::reg32 racer);
// The game's 0x404840: racer's rivalry, each tick (its wait, its rival, its lane by it, the chase's sound).
bool rivalTickDone(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 racer);
// The game's 0x404a00: car touched by another (a cop hit by a racer, a racer's ram, the hit's time).
bool carContactDone(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x404b90: whether racer was slowed for its class.
std::optional<bool> slowForClassOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x404c80: the leader among the players' cars, and how many other racers have more laps.
void findLeaderOf(win32::WinApplication* app, x86::reg32 car);

}  // namespace airace

namespace guest
{

// The car in eax; 1 or 0 in eax.
template <>
struct ByAddress<airace::wasReleased>
{
    template <x86::reg32 Address>
    static std::optional<bool> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        if (!watcom(app, cpu, Address, {car}))
            return std::nullopt;
        return cpu.eax != 0;
    }
};

// The car in eax; the gap in eax.
template <>
struct ByAddress<airace::nearestCopGapOf>
{
    template <x86::reg32 Address>
    static std::optional<std::int32_t> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        const std::optional<x86::reg32> gap = watcomEax(app, cpu, Address, {car});
        if (!gap)
            return std::nullopt;
        return std::int32_t(*gap);
    }
};

// The car in eax; 1 or 0 in eax.
template <>
struct ByAddress<airace::isPulledOver>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
        return cpu.eax != 0;
    }
};

// The racer in eax, the cop's index in edx.
template <>
struct ByAddress<airace::stopForCopDone>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 racer, x86::reg32 index)
    {
        return watcom(app, cpu, Address, {racer, index});
    }
};

// Stdcall: the kind in eax, the cop in edx, the car in ebx, the speed's bits pushed.
template <>
struct ByAddress<airace::joinPursuitDone>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 kind, x86::reg32 cop, x86::reg32 car,
                     float speed)
    {
        x86::reg32 bits;
        std::memcpy(&bits, &speed, sizeof bits);
        const KeptRegisters kept(cpu);
        cpu.eax = kind;
        cpu.edx = cop;
        cpu.ebx = car;
        return guest::call(app, cpu, Address, {bits});
    }
};

// The cop in eax.
template <>
struct ByAddress<airace::giveUpPursuitOf>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop)
    {
        return watcom(app, cpu, Address, {cop});
    }
};

template <>
struct ByAddress<airace::copStopsByDone>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop)
    {
        return watcom(app, cpu, Address, {cop});
    }
};

// The cop in eax; 1 or 0 in eax.
template <>
struct ByAddress<airace::stopOverOf>
{
    template <x86::reg32 Address>
    static std::optional<bool> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop)
    {
        if (!watcom(app, cpu, Address, {cop}))
            return std::nullopt;
        return cpu.eax != 0;
    }
};

// The cop in eax.
template <>
struct ByAddress<airace::placeCopAtStartOf>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop)
    {
        return watcom(app, cpu, Address, {cop});
    }
};

// Nothing handed over; the answer in eax.
template <>
struct ByAddress<airace::alwaysOneOf>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
        return cpu.eax;
    }
};

// The cop in eax.
template <>
struct ByAddress<airace::copDriverDone>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop)
    {
        return watcom(app, cpu, Address, {cop});
    }
};

// The car in eax; the rule in eax.
template <>
struct ByAddress<airace::classRuleOf>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
        return cpu.eax;
    }
};

// The racer in eax.
template <>
struct ByAddress<airace::setRammingRulesOf>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 racer)
    {
        watcom(app, cpu, Address, {racer});
    }
};

template <>
struct ByAddress<airace::rivalTickDone>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 racer)
    {
        return watcom(app, cpu, Address, {racer});
    }
};

// The car in eax.
template <>
struct ByAddress<airace::carContactDone>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcom(app, cpu, Address, {car});
    }
};

// The racer in eax; 1 or 0 in eax.
template <>
struct ByAddress<airace::slowForClassOf>
{
    template <x86::reg32 Address>
    static std::optional<bool> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        if (!watcom(app, cpu, Address, {car}))
            return std::nullopt;
        return cpu.eax != 0;
    }
};

// The car in eax.
template <>
struct ByAddress<airace::findLeaderOf>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
