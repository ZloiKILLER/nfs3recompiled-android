#ifndef NFS3HP_NATIVE_AI_RACE_H
#define NFS3HP_NATIVE_AI_RACE_H

#include "native_guest.h"

#include <cstdint>
#include <cstring>
#include <optional>

/* native_ai_race.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Those that take `cpu`
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
// The same as the game's code leaves it in eax: 1 or 0.
x86::reg32 pulledOverOf(win32::WinApplication* app, x86::reg32 car);
// The game's 0x401650: racer stops for cop `index` (its pursuit pulled over, a free cop joining).
bool stopForCopDone(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 racer, x86::reg32 index);
// The same, and the eax its code leaves on each way out.
std::optional<x86::reg32> stopForCopLeaving(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 racer,
                                            x86::reg32 index);
// The game's 0x4018b0: cop joins the pursuit of car (by `kind`; kind 0 keeps its speeding against `speed`); whether it joined.
std::optional<bool> joinPursuitOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 kind, x86::reg32 cop,
                                  x86::reg32 car, float speed);
// The same, and the eax the game's code leaves: the kind where the cop was on a pursuit already, the
// pursuit for a player's cop, else as the radio's word leaves it.
std::optional<x86::reg32> joinPursuitLeaving(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 kind,
                                             x86::reg32 cop, x86::reg32 car, float speed);
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
// The same, and the eax the code leaves; nullopt when the game stopped.
std::optional<x86::reg32> placeCopAtStartLeaving(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop);
// copStopsByDone with the eax the code leaves; nullopt when the game stopped.
std::optional<x86::reg32> copStopsByLeaving(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop);
// The game's 0x402c70: 1, whatever it is asked.
x86::reg32 alwaysOneOf(win32::WinApplication* app);
// The game's 0x4032e0: a cop's driver, each tick, by its step.
bool copDriverDone(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 cop);
// The game's 0x404610: car's class's rule.
x86::reg32 classRuleOf(win32::WinApplication* app, x86::reg32 car);
// The game's 0x404690: a racer's ramming rules by its class's kind.
void setRammingRulesOf(win32::WinApplication* app, x86::reg32 racer);
// The same, and the eax the code leaves: the racer, for a player's, else its place times 5.
x86::reg32 setRammingRulesLeaving(win32::WinApplication* app, x86::reg32 racer);
// The game's 0x404840: racer's rivalry, each tick (its wait, its rival, its lane by it, the chase's sound).
bool rivalTickDone(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 racer);
// The game's 0x404a00: car touched by another (a cop hit by a racer, a racer's ram, the hit's time).
bool carContactDone(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x404b90: whether racer was slowed for its class.
std::optional<bool> slowForClassOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x404c80: the leader among the players' cars, and how many other racers have more laps.
void findLeaderOf(win32::WinApplication* app, x86::reg32 car);

// The game's 0x4015b0: the car pulled over when a cop on it is near; false when the game stopped (eax: the car).
bool carStoppedIfCopNear(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x4047f0: the value of the class of the model.
x86::reg32 classValueOf(win32::WinApplication* app, x86::reg32 model);
}  // namespace airace

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
