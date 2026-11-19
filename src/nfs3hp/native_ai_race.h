#ifndef NFS3HP_NATIVE_AI_RACE_H
#define NFS3HP_NATIVE_AI_RACE_H

#include "native_guest.h"

#include <cstdint>
#include <cstring>
#include <optional>

/* native_ai_race.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Those that take `ts`
 * may call the game's other functions by address on it, and are false or nullopt when the game
 * stopped in one.  The ...Done ones are for callers that ask only whether the game went on: their
 * cores answer more than what the game's code leaves (the file's own types, kept there). */
namespace nfs3hp
{
namespace airace
{

// The game's 0x401240: whether car's input bit 0 was let go (or key 8 of its input is up).
std::optional<bool> wasReleased(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x4013b0: the gap in nodes from car to the nearest cop, 10000 at most.
std::optional<std::int32_t> nearestCopGapOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x401500: whether car's pursuit has it pulled over.
bool isPulledOver(nfs::App* app, nfs::u32 car);
// The same as the game's code leaves it in eax: 1 or 0.
nfs::u32 pulledOverOf(nfs::App* app, nfs::u32 car);
// The game's 0x401650: racer stops for cop `index` (its pursuit pulled over, a free cop joining).
bool stopForCopDone(nfs::App* app, nfs::ThreadState& ts, nfs::u32 racer, nfs::u32 index);
// The same, and the eax its code leaves on each way out.
std::optional<nfs::u32> stopForCopLeaving(nfs::App* app, nfs::ThreadState& ts, nfs::u32 racer,
                                            nfs::u32 index);
// The game's 0x4018b0: cop joins the pursuit of car (by `kind`; kind 0 keeps its speeding against `speed`); whether it joined.
std::optional<bool> joinPursuitOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 kind, nfs::u32 cop,
                                  nfs::u32 car, float speed);
// The same, and the eax the game's code leaves: the kind where the cop was on a pursuit already, the
// pursuit for a player's cop, else as the radio's word leaves it.
std::optional<nfs::u32> joinPursuitLeaving(nfs::App* app, nfs::ThreadState& ts, nfs::u32 kind,
                                             nfs::u32 cop, nfs::u32 car, float speed);
// joinPursuitOf for a caller that asks only whether the game went on.
inline bool joinPursuitDone(nfs::App* app, nfs::ThreadState& ts, nfs::u32 kind, nfs::u32 cop,
                            nfs::u32 car, float speed)
{
    return joinPursuitOf(app, ts, kind, cop, car, speed).has_value();
}
// The game's 0x401a80: cop gives up its pursuit (its radio's word, off the list, the getaway counted).
bool giveUpPursuitOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 cop);
// The game's 0x401e40: cop at a car pulled over: the nearest of several stops by it, the others drop off.
bool copStopsByDone(nfs::App* app, nfs::ThreadState& ts, nfs::u32 cop);
// The game's 0x401fb0: whether cop is done with a stop (its ticket given at the stop's end).
std::optional<bool> stopOverOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 cop);
// The game's 0x4029c0: cop placed at the race's start.
bool placeCopAtStartOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 cop);
// The same, and the eax the code leaves; nullopt when the game stopped.
std::optional<nfs::u32> placeCopAtStartLeaving(nfs::App* app, nfs::ThreadState& ts, nfs::u32 cop);
// copStopsByDone with the eax the code leaves; nullopt when the game stopped.
std::optional<nfs::u32> copStopsByLeaving(nfs::App* app, nfs::ThreadState& ts, nfs::u32 cop);
// The game's 0x402c70: 1, whatever it is asked.
nfs::u32 alwaysOneOf(nfs::App* app);
// The game's 0x4032e0: a cop's driver, each tick, by its step.
bool copDriverDone(nfs::App* app, nfs::ThreadState& ts, nfs::u32 cop);
// The game's 0x404610: car's class's rule.
nfs::u32 classRuleOf(nfs::App* app, nfs::u32 car);
// The game's 0x404690: a racer's ramming rules by its class's kind.
void setRammingRulesOf(nfs::App* app, nfs::u32 racer);
// The same, and the eax the code leaves: the racer, for a player's, else its place times 5.
nfs::u32 setRammingRulesLeaving(nfs::App* app, nfs::u32 racer);
// The game's 0x404840: racer's rivalry, each tick (its wait, its rival, its lane by it, the chase's sound).
bool rivalTickDone(nfs::App* app, nfs::ThreadState& ts, nfs::u32 racer);
// The game's 0x404a00: car touched by another (a cop hit by a racer, a racer's ram, the hit's time).
bool carContactDone(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x404b90: whether racer was slowed for its class.
std::optional<bool> slowForClassOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x404c80: the leader among the players' cars, and how many other racers have more laps.
void findLeaderOf(nfs::App* app, nfs::u32 car);

// The game's 0x4015b0: the car pulled over when a cop on it is near; false when the game stopped (eax: the car).
bool carStoppedIfCopNear(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car);
// The game's 0x4047f0: the value of the class of the model.
nfs::u32 classValueOf(nfs::App* app, nfs::u32 model);
}  // namespace airace

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
