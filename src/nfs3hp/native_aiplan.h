#ifndef NFS3HP_NATIVE_AIPLAN_H
#define NFS3HP_NATIVE_AIPLAN_H

#include "native_guest.h"

#include <cstdint>
#include <cstring>
#include <optional>

/* native_aiplan.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address (ByAddress, for the check's stand-ins).  Those that take `cpu`
 * may call the game's other functions by address on it, and are false or nullopt when the game
 * stopped in one. */
namespace nfs3hp
{
namespace aiplan
{

// The game's 0x40c590: traffic car set going (its way along the track, the player car it keeps near), then placed.
bool placeTraffic(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40c9f0: a traffic car's driver, each tick (its loading steps, then its state's).
bool driveTraffic(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40cb70: a traffic car reset (its record cleared, then linked).
bool resetTraffic(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40cd80: car put on the track at its node and target place across, its velocity along its nose.
bool putOnTrack(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40ce70: a pulled-over car brought to a stop once on the ground and gripping; whether it was.
std::optional<bool> stopPulledOver(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// stopPulledOver for a caller that asks only whether the game went on.
inline bool brakeToStopDone(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
{
    return stopPulledOver(app, cpu, car).has_value();
}
// The game's 0x40d720: car's lanes scored against the parked obstacles ahead.
void scoreObstacles(win32::WinApplication* app, x86::reg32 car);
// The game's 0x40d800: car's lanes scored against the cars sensed ahead in each.
bool scoreCars(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40d9f0: a small cut to the outer lanes' votes for a car sensed close ahead in them.
void penaliseCloseCars(win32::WinApplication* app);
// The game's 0x40dd60: the lane planner started for car (votes cleared, its slices, its lane, the traffic sensed).
bool beginPlanning(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40ddb0: car picks its lane (the edge, the way there, the road; the obstacles and cars but for a player's).
bool pickLane(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40de50: car's horn sounded for a while, its tone on.
void soundHorn(win32::WinApplication* app, x86::reg32 car);
// The game's 0x40deb0: car's horn ended once its time is up.
void endHorn(win32::WinApplication* app, x86::reg32 car);
// The game's 0x40df10: a traffic car's loading steps; 0 when it is not on the road, else 1.
std::optional<x86::reg32> stepLoading(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car);
// The game's 0x40e080: the computer cars' target speed at slice.
float targetSpeed(win32::WinApplication* app, x86::reg32 slice);
// The game's 0x40e0c0: the road's kind at slice.
x86::reg32 roadKind(win32::WinApplication* app, x86::reg32 slice);
// The game's 0x40e120: node less node `from`, wrapped round the track.
std::int32_t nodesBetween(win32::WinApplication* app, x86::reg32 node, x86::reg32 from);
// The game's 0x40e160: the node `steps` on from car's in its direction of travel.
x86::reg32 nodeAhead(win32::WinApplication* app, x86::reg32 car, x86::reg32 steps);
// The game's 0x40e1b0: the node car will reach by its look-ahead times `scale`, within `least` and `most`.
x86::reg32 nodeAheadBySpeed(win32::WinApplication* app, x86::reg32 car, float scale, std::int32_t least,
                            std::int32_t most);
// The game's 0x40e2b0: the road slice (0..13) `offset` across the road falls in at node.
x86::reg32 roadSlice(win32::WinApplication* app, x86::reg32 node, float offset);
// The game's 0x40e350: car's three road slices (its middle's and its sides') written to it.
void findCarSlices(win32::WinApplication* app, x86::reg32 car);
// The game's 0x40eca0: whether the track's road is wide.
x86::reg32 trackRoadWide(win32::WinApplication* app);
// The game's 0x40ed40: the traffic's speed at node.
float trafficSpeed(win32::WinApplication* app, x86::reg32 node);
// The game's 0x40ee80: the track's tuning speed.
float trackTuningSpeed(win32::WinApplication* app);
// The game's 0x40eed0: the track's tuning distance, more in the weather and at night.
x86::reg32 trackTuningDistance(win32::WinApplication* app);
// The game's 0x40ef10: the first word of the track's tuning record.
x86::reg32 trackTuningFirst(win32::WinApplication* app);
// The game's 0x40ef70: the dot product of nodes a's and b's forward directions.
x86::Float directionDot(win32::WinApplication* app, x86::reg32 a, x86::reg32 b);
// The game's 0x40f000: whether node lies in the run of `run` nodes from node `from`.
bool nodeInRun(win32::WinApplication* app, x86::reg32 node, x86::reg32 from, x86::reg32 run);
// The game's 0x40f050: whether car is within reach of `distance` along the track.
bool carInReach(win32::WinApplication* app, x86::reg32 car, x86::reg32 distance);
// The game's 0x40f100: 1, whatever it is asked.
x86::reg32 answerOne(win32::WinApplication* app);
// The game's 0x40f110: whether car is boxed in off its node's edge slices.
bool boxedIn(win32::WinApplication* app, x86::reg32 car);
// The game's 0x40f330: the seven animation paths freed and forgotten.
bool freePaths(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x40f3b0: path started in a free animation slot; the slot, or -1.
x86::reg32 playAnimation(win32::WinApplication* app, x86::reg32 path);
// The game's 0x40f430: an animation slot's mode, flags and third word set.
void setAnimation(win32::WinApplication* app, x86::reg32 slot, x86::reg32 mode, x86::reg32 flags, x86::reg32 third);
// The game's 0x40f460: an animation slot stopped; 1, or -1 when it was not playing.
x86::reg32 endAnimation(win32::WinApplication* app, x86::reg32 slot);
// The game's 0x40f4b0: path's pose at `time` in `mode` made a matrix at `matrix`; 1.
std::optional<x86::reg32> poseAt(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path, x86::reg32 mode,
                                 std::int32_t time, x86::reg32 blendWord, x86::reg32 matrix);
// The game's 0x40f800: an animation slot's frame and spans written out; the spans.
x86::reg32 animationSpans(win32::WinApplication* app, x86::reg32 slot, x86::reg32 frameOut, x86::reg32 spansOut);
// The game's 0x40f940: an animation slot stepped, its pose to `pose`; 1, 0 when it ended, -1 when not playing.
std::optional<x86::reg32> stepAnimation(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slot,
                                        x86::reg32 blendWord, x86::reg32 pose);

}  // namespace aiplan

namespace guest
{

// The car in eax (so for each below with a car, a slice, a node, a slot, a path: in eax).
template <>
struct ByAddress<aiplan::placeTraffic>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcom(app, cpu, Address, {car});
    }
};

template <>
struct ByAddress<aiplan::driveTraffic>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcom(app, cpu, Address, {car});
    }
};

template <>
struct ByAddress<aiplan::resetTraffic>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcom(app, cpu, Address, {car});
    }
};

template <>
struct ByAddress<aiplan::putOnTrack>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcom(app, cpu, Address, {car});
    }
};

template <>
struct ByAddress<aiplan::brakeToStopDone>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcom(app, cpu, Address, {car});
    }
};

template <>
struct ByAddress<aiplan::scoreObstacles>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
    }
};

template <>
struct ByAddress<aiplan::scoreCars>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcom(app, cpu, Address, {car});
    }
};

// Nothing handed over.
template <>
struct ByAddress<aiplan::penaliseCloseCars>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
    }
};

template <>
struct ByAddress<aiplan::beginPlanning>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcom(app, cpu, Address, {car});
    }
};

template <>
struct ByAddress<aiplan::pickLane>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcom(app, cpu, Address, {car});
    }
};

template <>
struct ByAddress<aiplan::soundHorn>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
    }
};

template <>
struct ByAddress<aiplan::endHorn>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
    }
};

// The step's answer in eax.
template <>
struct ByAddress<aiplan::stepLoading>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        return watcomEax(app, cpu, Address, {car});
    }
};

// The speed left on the x87's stack.
template <>
struct ByAddress<aiplan::targetSpeed>
{
    template <x86::reg32 Address>
    static float call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slice)
    {
        return float(watcomFloat(app, cpu, Address, {slice}).value_or(x86::Float(0.0)));
    }
};

// The kind in eax.
template <>
struct ByAddress<aiplan::roadKind>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slice)
    {
        watcom(app, cpu, Address, {slice});
        return cpu.eax;
    }
};

// The node in eax, `from` in edx; the gap in eax.
template <>
struct ByAddress<aiplan::nodesBetween>
{
    template <x86::reg32 Address>
    static std::int32_t call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 node, x86::reg32 from)
    {
        watcom(app, cpu, Address, {node, from});
        return std::int32_t(cpu.eax);
    }
};

// The steps in edx; the node in eax.
template <>
struct ByAddress<aiplan::nodeAhead>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 steps)
    {
        watcom(app, cpu, Address, {car, steps});
        return cpu.eax;
    }
};

// Stdcall: the scale's bits, the least and the most pushed (in that order up the stack); the node in eax.
template <>
struct ByAddress<aiplan::nodeAheadBySpeed>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, float scale, std::int32_t least,
                           std::int32_t most)
    {
        x86::reg32 bits;
        std::memcpy(&bits, &scale, sizeof bits);
        const KeptRegisters kept(cpu);
        cpu.eax = car;
        guest::call(app, cpu, Address, {bits, x86::reg32(least), x86::reg32(most)});
        return cpu.eax;
    }
};

// Stdcall: the offset's bits pushed; the slice in eax.
template <>
struct ByAddress<aiplan::roadSlice>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 node, float offset)
    {
        x86::reg32 bits;
        std::memcpy(&bits, &offset, sizeof bits);
        const KeptRegisters kept(cpu);
        cpu.eax = node;
        guest::call(app, cpu, Address, {bits});
        return cpu.eax;
    }
};

template <>
struct ByAddress<aiplan::findCarSlices>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
    }
};

// Nothing handed over; the answer in eax.
template <>
struct ByAddress<aiplan::trackRoadWide>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
        return cpu.eax;
    }
};

// The speed left on the x87's stack.
template <>
struct ByAddress<aiplan::trafficSpeed>
{
    template <x86::reg32 Address>
    static float call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 node)
    {
        return float(watcomFloat(app, cpu, Address, {node}).value_or(x86::Float(0.0)));
    }
};

// Nothing handed over; the speed left on the x87's stack.
template <>
struct ByAddress<aiplan::trackTuningSpeed>
{
    template <x86::reg32 Address>
    static float call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return float(watcomFloat(app, cpu, Address, {}).value_or(x86::Float(0.0)));
    }
};

// Nothing handed over; the distance in eax.
template <>
struct ByAddress<aiplan::trackTuningDistance>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
        return cpu.eax;
    }
};

// Nothing handed over; the word in eax.
template <>
struct ByAddress<aiplan::trackTuningFirst>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
        return cpu.eax;
    }
};

// Node a in eax, b in edx; the product left on the x87's stack.
template <>
struct ByAddress<aiplan::directionDot>
{
    template <x86::reg32 Address>
    static x86::Float call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b)
    {
        return watcomFloat(app, cpu, Address, {a, b}).value_or(x86::Float(0.0));
    }
};

// `from` in edx, `run` in ebx; 1 or 0 in eax.
template <>
struct ByAddress<aiplan::nodeInRun>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 node, x86::reg32 from, x86::reg32 run)
    {
        watcom(app, cpu, Address, {node, from, run});
        return cpu.eax != 0;
    }
};

// The distance in edx; 1 or 0 in eax.
template <>
struct ByAddress<aiplan::carInReach>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 distance)
    {
        watcom(app, cpu, Address, {car, distance});
        return cpu.eax != 0;
    }
};

// Nothing handed over; the answer in eax.
template <>
struct ByAddress<aiplan::answerOne>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
        return cpu.eax;
    }
};

// 1 or 0 in eax.
template <>
struct ByAddress<aiplan::boxedIn>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car)
    {
        watcom(app, cpu, Address, {car});
        return cpu.eax != 0;
    }
};

// Nothing handed over.
template <>
struct ByAddress<aiplan::freePaths>
{
    template <x86::reg32 Address>
    static bool call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return watcom(app, cpu, Address, {});
    }
};

// The slot in eax.
template <>
struct ByAddress<aiplan::playAnimation>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path)
    {
        watcom(app, cpu, Address, {path});
        return cpu.eax;
    }
};

// The mode in edx, the flags in ebx, the third word in ecx.
template <>
struct ByAddress<aiplan::setAnimation>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slot, x86::reg32 mode, x86::reg32 flags,
                     x86::reg32 third)
    {
        watcom(app, cpu, Address, {slot, mode, flags, third});
    }
};

// The answer in eax.
template <>
struct ByAddress<aiplan::endAnimation>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slot)
    {
        watcom(app, cpu, Address, {slot});
        return cpu.eax;
    }
};

// Stdcall: the mode in edx, the time in ebx, `blendWord` in ecx, the matrix's address pushed; the answer in eax.
template <>
struct ByAddress<aiplan::poseAt>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 path, x86::reg32 mode,
                                          std::int32_t time, x86::reg32 blendWord, x86::reg32 matrix)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = path;
        cpu.edx = mode;
        cpu.ebx = x86::reg32(time);
        cpu.ecx = blendWord;
        if (!guest::call(app, cpu, Address, {matrix}))
            return std::nullopt;
        return cpu.eax;
    }
};

// The frame's address in edx, the spans' in ebx; the spans in eax.
template <>
struct ByAddress<aiplan::animationSpans>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slot, x86::reg32 frameOut,
                           x86::reg32 spansOut)
    {
        watcom(app, cpu, Address, {slot, frameOut, spansOut});
        return cpu.eax;
    }
};

// `blendWord` in edx, the pose's address in ebx; the answer in eax.
template <>
struct ByAddress<aiplan::stepAnimation>
{
    template <x86::reg32 Address>
    static std::optional<x86::reg32> call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slot,
                                          x86::reg32 blendWord, x86::reg32 pose)
    {
        return watcomEax(app, cpu, Address, {slot, blendWord, pose});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
