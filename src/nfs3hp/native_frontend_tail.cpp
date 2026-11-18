#include "native_frontend_tail.h"
#include "native_collide.h"
#include "native_trackdata.h"
#include "native_game.h"
#include "native_carstate.h"
#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_hudx.h"
#include "native_frontend_loada.h"
#include "native_frontend_loopb.h"
#include "native_frontend_rendd.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_load_saves.h"
#include "native_vertices.h"
#include "native_windows.h"
#include "host_threads.h"
#include "port_snd_stream_calls.h"
#include "port_sound_effects.h"
#include "native_render.h"
#include <nfs3hp.h>
#include <fpu.h>
#include <lib/winapp.h>
#include <winapi/types.h>
#include <cstdint>
#include <cstring>
#include <optional>
#include "native_guest.h"

namespace nfs3hp
{
namespace
{

/* A call through a pointer or to a function of the game's that has no typed core: Watcom's register
 * arguments (eax, edx, ebx, ecx in turn) and the stack words as `arguments` have them, through
 * guest::pointerCall; its eax, and Terminated when the game stopped in it. */
x86::reg32 callAt(fe::Guest& guest, x86::reg32 target, const fe::Arguments& arguments = fe::Arguments())
{
    x86::CPU& cpu = guest.cpu();
    x86::Args registers;
    x86::reg32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    const x86::reg32 esp = cpu.esp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        cpu.esp -= 4;
        guest.setDword(cpu.esp, arguments.stack[i]);
    }
    const std::optional<x86::reg32> result = guest::pointerCall(guest.app(), cpu, target, registers);
    if (!result)
        throw fe::Terminated();
    cpu.esp = esp;
    return *result;
}

}  // namespace
}  // namespace nfs3hp

/* The last of the game's own logic, decompiled:
 *
 *   a frame        sub_4c48d0: a tick of the race: the countdown's four steps, the task
 *                  lists, the standings (sub_4be3a0 runs a list, sub_4be240 frees a block);
 *   the queues     sub_4c7b50: the controls' queues emptied under their lock;
 *   places         sub_4c8550 and sub_4c8470: the named place a node of the track is in, or
 *                  the one nearest it, for the dispatcher's speech;
 *   a lane         sub_4c9a80: how far across the road a lane is at a node;
 *   an angle       sub_4eac60: sine and cosine of a 10-bit angle, by the quarter table;
 *   the weather    sub_4dcdf0: the rain's or the snow's set-up; sub_4dece0: its pass over a
 *                  view; under it how much falls at a node (sub_4dd3d0) and at this time of
 *                  the race (sub_4dd330), how it follows a new camera (sub_4dd230)
 *                  and a change of view (sub_4dd510), its drops moved and put back around
 *                  the view (sub_4dd5e0), how heavy it is seen to be (sub_4dd4c0).
 *
 * The weather's take the race's random numbers, so every sum is the x87's in the
 * original's order: each fp:: call is an instruction's, its operands in the order the
 * game's code has them, a value that went through a float in memory goes through a
 * float here.  All the game's state stays in guest memory.  What is handed to the game
 * by address lives in the frame, where the game's function had it.
 *
 * Plan step 5: each function a typed core in frontend::tail (native_frontend_tail.h), in a frame
 * of its own where it calls anything (fe::Frame: what it calls runs at the stack addresses it did),
 * and a thunk under the game's name for the calls by address.  A core of this file calls another
 * as C++ (guest::direct), and the typed cores of other files too (fe::direct); the rest of the game
 * still by address. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using x86::Float;
using x86::reg16;
using x86::reg32;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;

// ---------------------------------------------------------------------------
// The x87 as these functions read it
// ---------------------------------------------------------------------------

/* What a jump after fcom; fnstsw ax; sahf sees of fp::compare's word. */
bool jumpsAbove(reg16 status) { return (status & 0x4100) == 0; }  // ja, not jbe; false unordered
bool jumpsBelow(reg16 status) { return (status & 0x0100) != 0; }  // jb, not jae; true unordered

/* eax after `fnstsw ax`. */
reg32 withStatus(reg32 eax, reg16 status) { return (eax & 0xffff0000u) | status; }

reg32 bitsOf(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* fld dword */
Float floatFrom(const Guest& guest, reg32 address) { return Float(guest.floatAt(address)); }

/* A double of the game's constants, as fmul qword reads it. */
Float doubleAt(const Guest& guest, reg32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* Another thread's turn with the game, where the game's code offers it. */
void safepoint(win32::WinApplication* app, x86::CPU& cpu)
{
    if (app->contextWanted())
        app->yieldContext(cpu);
}

// ---------------------------------------------------------------------------
// The game's functions, by address
// ---------------------------------------------------------------------------

constexpr reg32 kFillZero = 0x4e070c;          // eax where, edx the bytes
constexpr reg32 kClearCarList = 0x430cd0;
constexpr reg32 kRaceSoundTick = 0x411410;
// eax the count, edx the vectors, ebx the one added, ecx the result.  By address: the code goes on
// with the eax it leaves.
constexpr reg32 kAddVectors = 0x4dffb0;
constexpr reg32 kCarFacingWay = 0x420d60;      // eax a view: nonzero when its car drives against the track

// ---------------------------------------------------------------------------
// The race's data
// ---------------------------------------------------------------------------

constexpr reg32 kClock = 0x7d3684;             // the race's ticks
constexpr reg32 kCarNode = 0x1c;
constexpr reg32 kViewPlayer = 0x04;            // a view's player
constexpr reg32 kViewPosition = 0x08;          // its camera: three floats

// ---------------------------------------------------------------------------
// A frame of the race
// ---------------------------------------------------------------------------

constexpr reg32 kRaceUnderWay = 0x7d3680;
constexpr reg32 kCountdownStep = 0x552858;     // a byte: 1, 2, 3 and at the go 0
constexpr reg32 kSelectedLook = 0x55b088;      // which look the setters work on
constexpr reg32 kCountdownBank = 0x57116c;
constexpr reg32 kCountdownText = 0x791b44;
constexpr reg32 kEarlyTasks = 0x7d3690, kLateTasks = 0x7d368c;  // task lists
constexpr reg32 kPaused = 0x7d36a0;            // the pause menu is up

constexpr reg32 kTickGo = 0x200;               // the ticks at which each step of the countdown comes
constexpr reg32 kTickOne = 0x1c0;
constexpr reg32 kTickTwo = 0x180;
constexpr reg32 kTickThree = 0x140;
constexpr std::uint32_t kColourGo = 0xff007f40u, kColourOne = 0xffcfcf00u, kColourTwo = 0xffcf7700u, kColourThree = 0xffcf0000u;
constexpr reg32 kCountdownVolume = 0x7f, kCountdownPan = 0x40;

/* A step of the countdown come: its sound, its number, the text's colour.  sub_4182b0
 * (port_sound_effects.cpp's) is handed 0 in ebx, which it takes for the pan: what the branches
 * before it have left there, all of them; and kCountdownPan pushed, which it takes for the pitch. */
void countdownStep(Guest& guest, x86::CPU& cpu, reg32 effect, std::uint8_t step, reg32 colour)
{
    fe::direct<0x4182b0, port::bankEffect>(guest.app(), cpu, cpu.esp - 8, std::int32_t(guest.dword(kCountdownBank)),
                                           std::int32_t(effect), std::int32_t(kCountdownVolume), 0u,
                                           std::uint8_t(kCountdownPan));
    guest.setByte(kCountdownStep, step);
    guest.setDword(kSelectedLook, 3);
    fe::direct<0x477170, frontend::hudx::setDirectional>(guest.app(), cpu, colour, guest.dword(kCountdownText));
}

}  // namespace

/* sub_4c48d0: a tick of the race.  The lists of tasks, the race's end and the sound only
 * every second tick, on the even ones. */
x86::reg32 frontend::tail::raceFrame(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    if (gameSettings(guest).replayMode == 0)
    {
        const reg32 clock = guest.dword(kClock);
        if (sreg32(clock) >= sreg32(kTickGo))
            gameSettings(guest).lastTickRaced = clock;
    }
    if (guest.dword(kRaceUnderWay) == 0)
    {
        const sreg32 clock = sreg32(guest.dword(kClock));
        if (clock >= sreg32(kTickGo))
        {
            const std::uint8_t step = guest.byte(kCountdownStep);
            guest.setDword(kRaceUnderWay, 1);
            if (step != 0)
                countdownStep(guest, cpu, 3, 0, kColourGo);
        }
        else if (clock >= sreg32(kTickOne))
        {
            if (guest.byte(kCountdownStep) != 1)
                countdownStep(guest, cpu, 2, 1, kColourOne);
        }
        else if (clock >= sreg32(kTickTwo))
        {
            if (guest.byte(kCountdownStep) != 2)
                countdownStep(guest, cpu, 1, 2, kColourTwo);
        }
        else if (clock >= sreg32(kTickThree))
        {
            if (guest.byte(kCountdownStep) != 3)
                countdownStep(guest, cpu, 0, 3, kColourThree);
        }
    }
    fe::direct<kClearCarList, collide::carListCleared>(guest.app(), guest.cpu());
    if ((guest.byte(kClock) & 1) == 0)
    {
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, reg32(0));
        guest::direct<0x4be3a0, runTaskList>(app, cpu, guest.dword(kEarlyTasks));
        if (guest.dword(kPaused) == 0)
            fe::direct<0x4ceae0, frontend::loopb::checkRaceEnd>(app, cpu);
    }
    guest::direct<0x4be3a0, runTaskList>(app, cpu, guest.dword(kLateTasks));
    guest.setDword(kClock, guest.dword(kClock) + 1);
    // What the last call leaves in eax: the sound's, or the standings' on the odd ticks.
    const reg32 standings = fe::direct<0x4cde00, frontend::loopb::standingsUpdate>(app, cpu);
    if ((guest.byte(kClock) & 1) == 0)
        return callAt(guest, kRaceSoundTick);
    return standings;
}

namespace
{

// A list of tasks: its count, and from the start 16 bytes a task.
constexpr reg32 kTaskListCount = 0x04;
constexpr reg32 kTaskFunction = 0x0c, kTaskArgument = 0x10;
constexpr reg32 kTaskShift = 4;

}  // namespace

/* sub_4be3a0: each task of the list that has a function called, with its argument.  The
 * registers it finds are the loop's as the original has them; the tasks keep ecx and edx
 * as Watcom's functions do, the count read again each time.  eax is left the last
 * answer, or the last task itself if it had no function, or the list if it has no task. */
x86::reg32 frontend::tail::runTaskList(App* app, CPU& cpu, reg32 list)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    reg32 eax = list;
    for (reg32 i = 0;; ++i)
    {
        safepoint(app, cpu);
        if (sreg32(i) >= sreg32(guest.dword(list + kTaskListCount)))
            return eax;
        const reg32 task = list + (i << kTaskShift);
        eax = task;
        const reg32 function = guest.dword(task + kTaskFunction);
        if (function != 0)
            eax = callAt(guest, function, args({ guest.dword(task + kTaskArgument), i, task, list }));
    }
}

namespace
{

// ---------------------------------------------------------------------------
// The controls' queues
// ---------------------------------------------------------------------------

constexpr reg32 kEnterCriticalSection = 0x5344b8, kLeaveCriticalSection = 0x534588;  // import slots
constexpr reg32 kQueueLock = 0x560114;

/* A function of Windows', its words handed: the port's own, called as C++ (native_windows.h).  Terminated when the game stopped in it, as Guest::call. */
template <reg32 Slot, auto Function, typename... Words>
reg32 windows(Guest& guest, x86::CPU& cpu, Words... words)
{
    const std::optional<reg32> eax = guest::windows<Slot, Function>(guest.app(), cpu, words...);
    if (!eax)
        throw fe::Terminated();
    return *eax;
}

}  // namespace

/* sub_4c7b50: the four tables of the queues zeroed under their lock. */
x86::reg32 frontend::tail::queuesClear(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    windows<kEnterCriticalSection, &host::enterLock>(guest, cpu, guest.dword(kQueueLock));
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), 0x7dd070, 0xa0);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), 0x7dd010, 0x60);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), 0x7dd2a0, 0x8c);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), 0x7dd110, 0x190);
    return windows<kLeaveCriticalSection, &host::leaveLock>(guest, cpu, guest.dword(kQueueLock));
}

namespace
{

// ---------------------------------------------------------------------------
// The track's named places
// ---------------------------------------------------------------------------

constexpr reg32 kPlacesOfTrack = 0x560aa0;     // a list by track, or 0
// A place: from one node up to another, its phrase and its kind; the list ends with from -1.
constexpr reg32 kPlaceFrom = 0x00, kPlaceTo = 0x04, kPlaceKind = 0x08, kPlacePhrase = 0x0c;
constexpr reg32 kPlaceBytes = 0x10;
constexpr reg32 kPlacesEnd = reg32(-1);
// What is handed back: the phrase, how far off, the kind.
constexpr reg32 kFoundPhrase = 0x00, kFoundDistance = 0x04, kFoundKind = 0x08;

reg32 placesOfTrack(const Guest& guest)
{
    return guest.dword(kPlacesOfTrack + gameSettings(guest).track * 4);
}

}  // namespace

/* sub_4c8550: the place node `node` is in, to `out`, and 0; -1 if none. */
x86::reg32 frontend::tail::placeAtNode(App* app, CPU& cpu, reg32 node, reg32 out)
{
    Guest guest(app, cpu);
    reg32 place = placesOfTrack(guest);
    if (place == 0)
        return reg32(-1);
    for (;; place += kPlaceBytes)
    {
        safepoint(app, cpu);
        const sreg32 from = sreg32(guest.dword(place + kPlaceFrom));
        if (from <= sreg32(kPlacesEnd))
            return reg32(-1);
        if (sreg32(node) >= from && sreg32(node) < sreg32(guest.dword(place + kPlaceTo)))
        {
            guest.setDword(out + kFoundPhrase, guest.dword(place + kPlacePhrase));
            guest.setDword(out + kFoundKind, guest.dword(place + kPlaceKind));
            return 0;
        }
    }
}

namespace
{

/* The place at `place` as the one nearest, the distance from its end. */
reg32 foundPastEnd(Guest& guest, reg32 place, reg32 node, reg32 out)
{
    guest.setDword(out + kFoundPhrase, guest.dword(place + kPlacePhrase));
    guest.setDword(out + kFoundDistance, node - guest.dword(place + kPlaceTo));
    guest.setDword(out + kFoundKind, guest.dword(place + kPlaceKind));
    return 1;
}

}  // namespace

/* sub_4c8470: the place nearest node `node`, which is in none, to `out`, and 1: the first
 * if it is before that, the nearer of the two it is between, the last if it is past
 * that; 0 if it is in one after all, or the track has none. */
x86::reg32 frontend::tail::placeNearNode(App* app, CPU& cpu, reg32 node, reg32 out)
{
    Guest guest(app, cpu);
    reg32 place = placesOfTrack(guest);
    if (place == 0)
        return 0;
    if (sreg32(node) < sreg32(guest.dword(place + kPlaceFrom)))
    {
        guest.setDword(out + kFoundPhrase, guest.dword(place + kPlacePhrase));
        guest.setDword(out + kFoundDistance, guest.dword(place + kPlaceFrom) - node);
        guest.setDword(out + kFoundKind, guest.dword(place + kPlaceKind));
        return 1;
    }
    for (;; place += kPlaceBytes)
    {
        safepoint(app, cpu);
        if (guest.dword(place + kPlaceFrom) == kPlacesEnd)
            return 0;
        const reg32 next = place + kPlaceBytes;
        const sreg32 nextFrom = sreg32(guest.dword(next + kPlaceFrom));
        if (nextFrom == sreg32(kPlacesEnd))
        {
            if (sreg32(node) > sreg32(guest.dword(place + kPlaceTo)))
                return foundPastEnd(guest, place, node, out);
            return 0;
        }
        const sreg32 to = sreg32(guest.dword(place + kPlaceTo));
        if (sreg32(node) > to && sreg32(node) < nextFrom)
        {
            const reg32 toNext = guest.dword(next + kPlaceFrom) - node;
            const reg32 sinceEnd = node - reg32(to);
            if (sreg32(sinceEnd) < sreg32(toNext))
                return foundPastEnd(guest, place, node, out);
            guest.setDword(out + kFoundPhrase, guest.dword(next + kPlacePhrase));
            guest.setDword(out + kFoundDistance, guest.dword(next + kPlaceFrom) - node);
            guest.setDword(out + kFoundKind, guest.dword(next + kPlaceKind));
            return 1;
        }
    }
}

namespace
{

// ---------------------------------------------------------------------------
// A lane
// ---------------------------------------------------------------------------

constexpr reg32 kTrackNodes = 0x5dd948;        // the nodes: 36 bytes each
constexpr reg32 kNodeBytes = 36;
constexpr reg32 kNodeLanes = 0x13;             // a byte: the lanes each side, left in its top half
constexpr reg32 kNodeLeftWidth = 0x17, kNodeRightWidth = 0x1b;  // bytes: a lane's width each side, in 1/2 of a unit
constexpr reg32 kNodeLeftEdge = 0x1c, kNodeRightEdge = 0x20;    // 16.16: the road's edges
constexpr reg32 kMiddleLane = 7;
constexpr reg32 kEdgeMargin = 0x20000;         // 16.16: what stays between a lane and the edge

}  // namespace

/* sub_4c9a80: how far from the middle of the road, 16.16, lane `lane` is at node `node`:
 * by its number and the width of a lane on that side, its edge's lane a little off the
 * edge, the others on their middles, the whole within the road.  The original then
 * writes the nodes' pointer back as it read it: no change.  edx is left the near limit. */
x86::reg32 frontend::tail::laneOffset(App* app, CPU& cpu, reg32 node, reg32 lane)
{
    Guest guest(app, cpu);
    const reg32 nodes = guest.dword(kTrackNodes);
    const reg32 record = nodes + node * kNodeBytes;
    const reg32 lanes = guest.byte(record + kNodeLanes);
    const bool atEdge = lane == 6 - ((lanes >> 4) & 0xf) || lane == (lanes & 0xf) + 7;
    const bool left = sreg32(lane) < sreg32(kMiddleLane);
    const reg32 width = reg32(guest.byte(record + (left ? kNodeLeftWidth : kNodeRightWidth))) << 15;
    const reg32 lanesOut = left ? 0 - (kMiddleLane - lane - 1) : lane - kMiddleLane;
    reg32 offset;
    if (atEdge)
        offset = lanesOut * width + (left ? 0 - kEdgeMargin : kEdgeMargin);
    else
        offset = lanesOut * width + (left ? 0 - (width >> 1) : width >> 1);

    const reg32 most = guest.dword(record + kNodeRightEdge) - kEdgeMargin;
    if (sreg32(offset) > sreg32(most))
        offset = most;
    const reg32 least = 0 - guest.dword(record + kNodeLeftEdge) + kEdgeMargin;
    if (sreg32(offset) < sreg32(least))
        offset = least;
    return offset;
}

namespace
{

// ---------------------------------------------------------------------------
// An angle
// ---------------------------------------------------------------------------

constexpr reg32 kQuarterSine = 0x5672ac;       // 257 of them, 0 to a quarter turn

reg32 quarterSine(const Guest& guest, reg32 at) { return guest.dword(kQuarterSine + at * 4); }

}  // namespace

/* sub_4eac60: the sine of a 10-bit angle to edx's address and its cosine to ebx's, by the
 * quarter it is in.  The original writes a value and then its negative over it; eax and
 * edx are left as it leaves them, the table's values before they are turned. */
x86::reg32 frontend::tail::angleParts(App* app, CPU& cpu, reg32 angle, reg32 sineOut, reg32 cosineOut)
{
    Guest guest(app, cpu);
    const reg32 quarter = (reg32(sreg32(angle) >> 8)) & 3;
    const reg32 at = angle & 0xff;
    const reg32 rest = 0x100 - at;
    switch (quarter)
    {
    case 0:
    {
        guest.setDword(sineOut, quarterSine(guest, at));
        const reg32 cosine = quarterSine(guest, rest);
        guest.setDword(cosineOut, cosine);
        return cosine;
    }
    case 1:
    {
        guest.setDword(sineOut, quarterSine(guest, rest));
        const reg32 cosine = quarterSine(guest, at);
        guest.setDword(cosineOut, 0 - cosine);
        return cosine;
    }
    case 2:
    {
        guest.setDword(sineOut, 0 - quarterSine(guest, at));
        const reg32 cosine = quarterSine(guest, rest);
        guest.setDword(cosineOut, 0 - cosine);
        return cosine;
    }
    default:
    {
        guest.setDword(sineOut, 0 - quarterSine(guest, rest));
        const reg32 cosine = quarterSine(guest, at);
        guest.setDword(cosineOut, cosine);
        return cosine;
    }
    }
}

namespace
{

// ---------------------------------------------------------------------------
// The weather
// ---------------------------------------------------------------------------

constexpr reg32 kWeatherChoice = 0x8ca1e8;     // a byte, the track's: by 3, the kind
constexpr reg32 kWeatherFirstNodeIn = 0x8ca1ea, kWeatherLastNodeIn = 0x8ca1ec, kWeatherFadeNodesIn = 0x8ca1ee;  // int16
constexpr reg32 kFadeInFrom = 0x8ca1f0, kFadeInTo = 0x8ca1f4, kFadeOutFrom = 0x8ca1f8, kFadeOutTo = 0x8ca1fc;   // ticks

constexpr reg32 kWeather = 0x8ca22c;           // what is set up, 0x5c bytes
constexpr reg32 kWeatherBytes = 0x5c;
constexpr reg32 kWeatherKind = 0x8ca22c;       // 0 none, 1 rain, 2 snow
constexpr reg32 kWeatherSize = 0x8ca230;       // float: a flake's size
constexpr reg32 kWeatherCount = 0x8ca234;      // the drops there are, a player
constexpr reg32 kWeatherGrey = 0x8ca238;
constexpr reg32 kWeatherFirstNode = 0x8ca23c, kWeatherLastNode = 0x8ca23e, kWeatherFadeNodes = 0x8ca240;  // int16
constexpr reg32 kWeatherNodeShift = 0x8ca244;
constexpr reg32 kWeatherFadeStart = 0x8ca248;  // the tick its time began
constexpr reg32 kWeatherReach = 0x8ca24c;      // how far from the view drops are drawn
constexpr reg32 kWeatherOneOverReach = 0x8ca250;  // float
constexpr reg32 kWeatherHeightInt = 0x8ca254, kWeatherDepthInt = 0x8ca258;  // above and below the view
constexpr reg32 kWeatherHalfWidth = 0x8ca25c;  // float, the reach: how far either way a drop stays
constexpr reg32 kWeatherLow = 0x8ca260, kWeatherHigh = 0x8ca264;  // floats: the box's bottom and top
constexpr reg32 kWeatherSpan = 0x8ca268;       // float: its height
constexpr reg32 kWeatherDrift = 0x8ca26c;      // three floats: what a drop moves by each step
constexpr reg32 kWeatherDriftY = 0x8ca270, kWeatherDriftZ = 0x8ca274;
constexpr reg32 kWeatherSplashes = 0x8ca278;   // how many a player
constexpr reg32 kSplashFullLife = 0x8ca27c, kSplashScale = 0x8ca280, kSplashGrey = 0x8ca284;
constexpr reg32 kWeatherLastView = 0x8ca2b4;   // by player: what sub_4dd510 last found

// A kind, 52 bytes in the table at 0x564290.
constexpr reg32 kKinds = 0x564290;
constexpr reg32 kKindBytes = 52;
constexpr reg32 kKindFall = 0x00;              // 16.16, ten times: how fast down
constexpr reg32 kKindSize = 0x04;
constexpr reg32 kKindDensity = 0x08;           // per cent of all the drops
constexpr reg32 kKindReach = 0x0c;
constexpr reg32 kKindHeight = 0x10, kKindDepth = 0x14;
constexpr reg32 kKindAngle = 0x18;             // degrees: the wind's way
constexpr reg32 kKindSpeed = 0x1c;             // tenths: the wind's speed
constexpr reg32 kKindGrey = 0x20, kKindSplashLife = 0x24, kKindSplashScale = 0x28, kKindSplashGrey = 0x2c;
constexpr reg32 kKindSplashes = 0x30;          // per cent of them all

constexpr reg32 kAllDrops = 800, kAllSplashes = 20;
constexpr reg32 kAPlayersDrops = 0x2580;       // 800 of 12 bytes
constexpr reg32 kDropBytes = 12;

// A drop's state, a player's after the other's.
constexpr reg32 kDrops = 0x8c0be8;
constexpr reg32 kDropsBytes = 0x4b00;
constexpr reg32 kDropOld = 0x00;               // two floats: where it was seen
constexpr reg32 kDropOldY = 0x04;
constexpr reg32 kDropFacing = 0x08;            // a byte: the car's way when it was put back
constexpr reg32 kDropSeen = 0x09;              // a byte: it has a place it was seen at
constexpr reg32 kNoPlace = 0xbf800000u;        // -1.0
// Its place.
constexpr reg32 kDropPlaces = 0x8c56e8;

// The splashes: 20 bytes each, 400 a player, how long each has left at +8.
constexpr reg32 kSplashes = 0x8c08c0;
constexpr reg32 kSplashesBytes = 0x320;
constexpr reg32 kSplashLives = 0x8c08c8;
constexpr reg32 kSplashBytes = 20;
constexpr reg32 kAPlayersSplashes = 400;

constexpr reg32 kSharesSeen = 0x564288;        // by player: how many of the drops were seen, per cent
constexpr reg32 kLastStep = 0x8c0be0;          // by player: the tick its drops last moved
constexpr reg32 kCameraChanged = 0x552ff4;     // by player: its camera has just changed
constexpr reg32 kPlayerOut = 0x79c4a0;         // by player
constexpr reg32 kCameraOverride = 0x5e10b8, kCamera = 0x5e10b0;  // by player: a camera's record, its mode first

constexpr reg32 kFixedToFloat = 0x5497b4;      // a double, 1 / 65536
constexpr reg32 kFixedToFloatB = 0x5497c4;     // the same
constexpr reg32 kTwoAtStart = 0x5497bc, kTwoAtCut = 0x5497c0, kTwoAtReturn = 0x5497cc;  // floats, 2 each
constexpr reg32 kNodeCount = 0x5dd940;

/* What sub_4eb230 leaves on the x87. */
Float randomFraction(Guest& guest, x86::CPU& cpu)
{
    return *fe::direct<0x4eb230, ea::randomFractionOf>(guest.app(), cpu);
}

/* From minus the half width to plus it, a random fraction of it: `two` the 2 of the
 * game's own at that place. */
float randomAcross(Guest& guest, x86::CPU& cpu, reg32 two)
{
    const Float scaled = fp::mul(randomFraction(guest, cpu), floatFrom(guest, kWeatherHalfWidth));
    return float(fp::sub(fp::mul(scaled, floatFrom(guest, two)), floatFrom(guest, kWeatherHalfWidth)));
}

/* From the box's bottom to its top. */
float randomHeight(Guest& guest, x86::CPU& cpu)
{
    return float(fp::add(fp::mul(randomFraction(guest, cpu), floatFrom(guest, kWeatherSpan)), floatFrom(guest, kWeatherLow)));
}

/* A fixed-point value turned by `times` and back, cut towards zero as the original's
 * shift and sbb cut it. */
reg32 byShareOf65536(reg32 value, reg32 times)
{
    return reg32(sreg32(value * times) / 65536);
}

}  // namespace

/* sub_4dcdf0: the weather set up as a race loads: the kind by the track's byte, what that
 * kind says, the drops scattered in their box, for each player a place and a place it
 * was last seen.  eax is left what the last loop left, or the byte by 3 if none falls. */
x86::reg32 frontend::tail::weatherStart(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0xc);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kWeather, kWeatherBytes);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kDrops, kDropsBytes);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kSplashes, kSplashesBytes);
    guest.setDword(kSharesSeen, 0);
    guest.setDword(kSharesSeen + 4, 0);

    const reg32 choice = guest.byte(kWeatherChoice);
    const reg32 kind = choice % 3;
    guest.setDword(kWeatherKind, kind);
    if (kind == 0)
        return choice / 3;

    guest.setWord(kWeatherFirstNode, guest.word(kWeatherFirstNodeIn));
    guest.setWord(kWeatherLastNode, guest.word(kWeatherLastNodeIn));
    guest.setWord(kWeatherFadeNodes, guest.word(kWeatherFadeNodesIn));
    const reg32 row = kKinds + kind * kKindBytes;
    guest.setDword(kWeatherFadeStart, guest.dword(kClock));
    const sreg32 density = sreg32(guest.dword(row + kKindDensity) << 16) / 100;
    guest.setDword(kWeatherSize, guest.dword(row + kKindSize));
    guest.setDword(kWeatherCount, byShareOf65536(reg32(density), kAllDrops));
    const sreg32 reachAsGiven = sreg32(guest.dword(row + kKindReach));
    const sreg32 reach = reachAsGiven > 1 ? reachAsGiven : 1;
    guest.setDword(kWeatherReach, reg32(reach));
    guest.setDword(kWeatherHeightInt, guest.dword(row + kKindHeight));
    guest.setDword(kWeatherDepthInt, guest.dword(row + kKindDepth));

    const sreg32 angleFixed = sreg32(guest.dword(row + kKindAngle) << 16) / 360;
    const Float share = fp::div(1.0, Float(reach));
    const sreg32 speedFixed = sreg32(guest.dword(row + kKindSpeed) << 16) / 10;
    const Float angle = fp::mul(Float(angleFixed), doubleAt(guest, kFixedToFloat));
    const Float speed = fp::mul(Float(speedFixed), doubleAt(guest, kFixedToFloat));
    guest.setFloat(kWeatherOneOverReach, float(share));

    // The angle by address, where the original has it, and the wind its sine and cosine times the speed.
    const reg32 angleAt = guest.frame();
    guest.setFloat(angleAt, float(angle));
    fe::direct<0x4ea970, frontend::simd::sineCosine>(app, cpu, angleAt, kWeatherDrift, kWeatherDriftZ);
    guest.setDword(kWeatherDriftY, 0);
    fe::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, std::int32_t(1), kWeatherDrift, float(speed), kWeatherDrift);

    const sreg32 fall = sreg32(guest.dword(row + kKindFall)) / 10;
    guest.setDword(kWeatherGrey, guest.dword(row + kKindGrey));
    guest.setDword(kLastStep, guest.dword(kClock));
    guest.setDword(kLastStep + 4, guest.dword(kClock));
    const sreg32 streams = sreg32(guest.dword(row + kKindSplashes) << 16) / 100;
    const Float fallPerStep = fp::mul(Float(fall), doubleAt(guest, kFixedToFloat));
    guest.setDword(kWeatherNodeShift, 0);
    guest.setFloat(kWeatherHalfWidth, float(Float(sreg32(guest.dword(kWeatherReach)))));
    guest.setFloat(kWeatherHigh, float(Float(sreg32(guest.dword(kWeatherHeightInt)))));
    guest.setDword(kWeatherSplashes, byShareOf65536(reg32(streams), kAllSplashes));
    guest.setDword(kSplashFullLife, guest.dword(row + kKindSplashLife));
    const Float high = floatFrom(guest, kWeatherHigh);
    guest.setDword(kSplashGrey, guest.dword(row + kKindSplashGrey));
    guest.setFloat(kSplashScale, float(floatFrom(guest, row + kKindSplashScale)));
    guest.setFloat(kWeatherDriftY, float(-fallPerStep));
    guest.setFloat(kWeatherLow, float(Float(sreg32(0 - guest.dword(kWeatherDepthInt)))));
    guest.setFloat(kWeatherSpan, float(fp::sub(high, floatFrom(guest, kWeatherLow))));

    // The original tests the kind again here; nothing since has changed it.
    fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
    for (reg32 i = 0; i < kAllDrops; ++i)
    {
        safepoint(app, cpu);
        const reg32 at = i * kDropBytes;
        guest.setFloat(kDropPlaces + at, randomAcross(guest, cpu, kTwoAtStart));
        guest.setFloat(kDropPlaces + at + 4, randomHeight(guest, cpu));
        guest.setFloat(kDropPlaces + at + 8, randomAcross(guest, cpu, kTwoAtStart));
        guest.setFloat(kDropPlaces + kAPlayersDrops + at, randomAcross(guest, cpu, kTwoAtStart));
        guest.setFloat(kDropPlaces + kAPlayersDrops + at + 4, randomHeight(guest, cpu));
        // Between the call and what it answers, the original writes the rest of the drop.
        const Float fraction = randomFraction(guest, cpu);
        guest.setDword(kDrops + at + kDropOld, kNoPlace);
        guest.setDword(kDrops + at + kDropOldY, kNoPlace);
        guest.setDword(kDrops + kAPlayersDrops + at + kDropOld, kNoPlace);
        guest.setDword(kDrops + kAPlayersDrops + at + kDropOldY, kNoPlace);
        guest.setByte(kDrops + at + kDropSeen, 0);
        guest.setByte(kDrops + kAPlayersDrops + at + kDropSeen, 0);
        guest.setByte(kDrops + at + kDropFacing, 0);
        guest.setByte(kDrops + kAPlayersDrops + at + kDropFacing, 0);
        const Float scaled = fp::mul(fraction, floatFrom(guest, kWeatherHalfWidth));
        guest.setFloat(kDropPlaces + kAPlayersDrops + at + 8,
                       float(fp::sub(fp::mul(scaled, floatFrom(guest, kTwoAtStart)), floatFrom(guest, kWeatherHalfWidth))));
    }
    fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
    for (reg32 j = 0; j < kAllSplashes; ++j)
    {
        safepoint(app, cpu);
        guest.setDword(kSplashLives + j * kSplashBytes, 0);
        guest.setDword(kSplashLives + kAPlayersSplashes + j * kSplashBytes, 0);
    }
    return (kAllSplashes - 1) * 5;
}


/* sub_4dd3d0: how many drops fall at node `node`: none outside the weather's stretch of the
 * track, all inside it, fewer over its first and last nodes.  A stretch that wraps past
 * the track's end has the wrap taken out of it first. */
x86::reg32 frontend::tail::weatherFallAtNode(App* app, CPU& cpu, reg32 node)
{
    Guest guest(app, cpu);
    const reg32 shift = guest.dword(kWeatherNodeShift);
    const reg32 first = reg32(sreg32(std::int16_t(guest.word(kWeatherFirstNode)))) + shift;
    reg32 last = reg32(sreg32(std::int16_t(guest.word(kWeatherLastNode)))) + guest.dword(kWeatherNodeShift);
    const sreg32 fade = sreg32(std::int16_t(guest.word(kWeatherFadeNodes)));
    const reg32 fadedIn = first + reg32(fade);
    reg32 fadingOut = last - reg32(fade);

    if (first == last && fade == 0)
        return guest.dword(kWeatherCount);
    if (sreg32(first) > sreg32(last))
    {
        if (sreg32(node) < sreg32(last))
            node += guest.dword(kNodeCount);
        const reg32 nodes = guest.dword(kNodeCount);
        fadingOut += nodes;
        last += nodes;
    }
    if (sreg32(node) < sreg32(first) || sreg32(node) > sreg32(last))
        return 0;
    if (fade == 0)
        return guest.dword(kWeatherCount);
    if (sreg32(node) > sreg32(fadedIn) && sreg32(node) < sreg32(fadingOut))
        return guest.dword(kWeatherCount);
    const reg32 count = guest.dword(kWeatherCount);
    if (sreg32(node) < sreg32(fadedIn))
        return count - reg32(sreg32(count * (fadedIn - node)) / fade);
    if (sreg32(node) > sreg32(fadingOut))
        return count - reg32(sreg32(count * (node - fadingOut)) / fade);
    return count;
}


/* sub_4dd330: `amount`, by the time of the race: as it is with no times or before they
 * begin, less and less to nothing, nothing, more and more back to all of it; past them
 * the times start again. */
x86::reg32 frontend::tail::weatherFadeInTime(App* app, CPU& cpu, reg32 amount)
{
    Guest guest(app, cpu);
    const sreg32 inFrom = sreg32(guest.dword(kFadeInFrom)), inTo = sreg32(guest.dword(kFadeInTo));
    const sreg32 outFrom = sreg32(guest.dword(kFadeOutFrom)), outTo = sreg32(guest.dword(kFadeOutTo));
    const sreg32 time = sreg32(guest.dword(kClock) - guest.dword(kWeatherFadeStart));
    if (inFrom == 0 || inTo == 0 || outFrom == 0 || outTo == 0 || time < inFrom)
        return amount;
    if (time < inTo)
        return reg32(sreg32((reg32(inTo) - reg32(time)) * amount) / sreg32(reg32(inTo) - reg32(inFrom)));
    if (time < outFrom)
        return 0;
    if (time < outTo)
        return amount - reg32(sreg32((reg32(outTo) - reg32(time)) * amount) / sreg32(reg32(outTo) - reg32(outFrom)));
    guest.setDword(kWeatherFadeStart, guest.dword(kClock));
    return amount;
}


/* sub_4dd230: the camera changed: the player's drops, if the car is going the track's way, all
 * put around the view again; its splashes over. */
x86::reg32 frontend::tail::weatherScatter(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    const reg32 player = guest.dword(view + kViewPlayer);
    if (guest.dword(kWeatherKind) == 0)
        return view;
    reg32 eax = *fe::direct<kCarFacingWay, trackdata::carAgainstTrack>(guest.app(), guest.cpu(), view);
    if (eax == 0)
    {
        for (reg32 i = 0;; ++i)
        {
            safepoint(app, cpu);
            if (sreg32(i) >= sreg32(guest.dword(kWeatherCount)))
                break;
            const reg32 at = player * kAPlayersDrops + i * kDropBytes;
            eax = (i * kDropBytes) & 0xffff00ffu;  // xor ah, ah: the byte's 0
            guest.setByte(kDrops + at + kDropSeen, 0);
            const Float x = fp::mul(randomFraction(guest, cpu), floatFrom(guest, kWeatherHalfWidth));
            const Float x2 = fp::sub(fp::mul(x, floatFrom(guest, kTwoAtCut)), floatFrom(guest, kWeatherHalfWidth));
            guest.setFloat(kDropPlaces + at, float(fp::add(x2, floatFrom(guest, view + kViewPosition))));
            const Float y = fp::add(fp::mul(randomFraction(guest, cpu), floatFrom(guest, kWeatherSpan)), floatFrom(guest, kWeatherLow));
            guest.setFloat(kDropPlaces + at + 4, float(fp::add(y, floatFrom(guest, view + kViewPosition + 4))));
            const Float z = fp::mul(randomFraction(guest, cpu), floatFrom(guest, kWeatherHalfWidth));
            const Float z2 = fp::sub(fp::mul(z, floatFrom(guest, kTwoAtCut)), floatFrom(guest, kWeatherHalfWidth));
            guest.setFloat(kDropPlaces + at + 8, float(fp::add(z2, floatFrom(guest, view + kViewPosition + 8))));
        }
    }
    for (reg32 j = 0;; ++j)
    {
        safepoint(app, cpu);
        if (sreg32(j) >= sreg32(guest.dword(kWeatherSplashes)))
            break;
        eax = j * 5;
        guest.setDword(kSplashLives + player * kAPlayersSplashes + j * kSplashBytes, 0);
    }
    return eax;
}

namespace
{

/* The player's camera's mode: the one overriding it if there is one, or 0 for neither. */
sreg32 cameraMode(const Guest& guest, reg32 player)
{
    const reg32 over = guest.dword(kCameraOverride + player * 4);
    if (over != 0)
        return sreg32(guest.dword(over));
    const reg32 under = guest.dword(kCamera + player * 4);
    return under != 0 ? sreg32(guest.dword(under)) : 0;
}

}  // namespace

/* sub_4dd510: whether this player's camera is of modes 2 to 4 with kPlayerOut set.  When
 * that changes, or the camera does, its drops have no place they were seen.  eax is left
 * what its last sums left. */
x86::reg32 frontend::tail::weatherViewChange(App* app, CPU& cpu, reg32 player)
{
    Guest guest(app, cpu);
    constexpr sreg32 kLeastMode = 2, kMostMode = 4;
    reg32 covered = 0;
    const sreg32 mode = cameraMode(guest, player);
    if (mode <= kMostMode && mode >= kLeastMode && guest.dword(kPlayerOut + player * 4) != 0)
        covered = 1;
    reg32 eax = player * 4;
    if (covered != guest.dword(kWeatherLastView + player * 4) || guest.dword(kCameraChanged + player * 4) != 0)
    {
        for (reg32 i = 0;; ++i)
        {
            safepoint(app, cpu);
            if (sreg32(i) >= sreg32(guest.dword(kWeatherCount)))
                break;
            eax = i * 3;
            guest.setByte(kDrops + player * kAPlayersDrops + i * kDropBytes + kDropSeen, 0);
        }
    }
    guest.setDword(kWeatherLastView + player * 4, covered);
    return eax;
}


/* sub_4dd4c0: how many of the drops were seen, as a share of all of them, per cent, for
 * the player: 0 of none.  edx is left the division's remainder when there was one. */
x86::reg32 frontend::tail::weatherShare(App* app, CPU& cpu, reg32 seen, reg32 player)
{
    Guest guest(app, cpu);
    const reg32 count = guest.dword(kWeatherCount);
    if (count == 0 || seen == 0)
    {
        guest.setDword(kSharesSeen + player * 4, 0);
        return seen;
    }
    const sreg32 hundredTimes = sreg32(seen * 100);
    const reg32 share = reg32(hundredTimes / sreg32(count));
    guest.setDword(kSharesSeen + player * 4, share);
    return share;
}


/* sub_4dd5e0: a player's drops moved, if two ticks or more have gone by, with what falls; and
 * of the first `wanted` of them what has left the box round the view put back at the
 * side it left by, what snow is not yet on the ground swaying first.  eax and edx are left
 * the last drop's, which is to say the last comparison's, as the original has them. */
x86::reg32 frontend::tail::weatherDropsStep(App* app, CPU& cpu, reg32 view, reg32 wanted)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x2c);
    constexpr reg32 kSnow = 2;
    const reg32 player = guest.dword(view + kViewPlayer);
    const reg32 sinceLast = guest.dword(kClock) - guest.dword(kLastStep + player * 4);
    if (sreg32(sinceLast) <= 1)
    {
        return sinceLast;
    }
    const reg32 facing = *fe::direct<kCarFacingWay, trackdata::carAgainstTrack>(guest.app(), guest.cpu(), view);
    guest.setDword(kLastStep + player * 4, guest.dword(kClock));
    for (reg32 j = 0;; ++j)
    {
        safepoint(app, cpu);
        if (sreg32(j) >= sreg32(guest.dword(kWeatherSplashes)))
            break;
        const reg32 timer = kSplashLives + player * kAPlayersSplashes + j * kSplashBytes;
        const reg32 left = guest.dword(timer);
        if (left != 0)
            guest.setDword(timer, left - 1);
    }
    const reg32 places = kDropPlaces + player * kAPlayersDrops;
    guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, std::int32_t(guest.dword(kWeatherCount)), places,
                                                               kWeatherDrift, places);

    reg32 eax = 0;  // what the vectors' addition left; the upper bits come back only when no drop is walked
    const auto width = [&] { return floatFrom(guest, kWeatherHalfWidth); };
    // fcomp and fnstsw: the word is left in ax.
    const auto compared = [&](const Float& a, const Float& b) {
        const reg16 status = fp::compare(a, b);
        eax = withStatus(eax, status);
        return status;
    };
    // Off to a side by the size of `value`, as the original takes it: fldz; fcomp, its sign
    // changed only below 0, so that -0 stays -0 and a NaN as it is.
    const auto farOff = [&](float value) {
        float size;
        if (jumpsAbove(compared(0.0, Float(value))))
        {
            size = float(-Float(value));
        }
        else
        {
            eax = bitsOf(value);
            size = value;
        }
        return jumpsAbove(compared(Float(size), width()));
    };

    for (reg32 i = 0;; ++i)
    {
        safepoint(app, cpu);
        if (sreg32(i) >= sreg32(wanted))
            break;
        const reg32 at = player * kAPlayersDrops + i * kDropBytes;
        const reg32 drop = kDrops + at;
        const reg32 place = kDropPlaces + at;
        eax = guest.dword(kWeatherKind);
        if (eax == kSnow && guest.byte(drop + kDropFacing) == 0)
        {
            const reg32 angle = (i << 8) + (guest.dword(kClock) << 8);
            const sreg32 sideways = sreg32(fe::direct<0x4f2380, load::sineOf>(app, cpu, angle)) >> 4;
            const Float x = fp::mul(Float(sideways), doubleAt(guest, kFixedToFloatB));
            guest.setFloat(place, float(fp::add(x, floatFrom(guest, place))));
            const sreg32 onwards = sreg32(fe::direct<0x4f23f0, load::cosineOf>(app, cpu, angle)) >> 4;
            eax = reg32(onwards);
            const Float z = fp::mul(Float(onwards), doubleAt(guest, kFixedToFloatB));
            guest.setFloat(place + 8, float(fp::add(z, floatFrom(guest, place + 8))));
        }

        float offX = float(fp::sub(floatFrom(guest, place), floatFrom(guest, view + kViewPosition)));
        float offY = float(fp::sub(floatFrom(guest, place + 4), floatFrom(guest, view + kViewPosition + 4)));
        float offZ = float(fp::sub(floatFrom(guest, place + 8), floatFrom(guest, view + kViewPosition + 8)));

        bool outside = jumpsBelow(compared(Float(offY), floatFrom(guest, kWeatherLow)));
        outside = outside || jumpsAbove(compared(Float(offY), floatFrom(guest, kWeatherHigh)));
        outside = outside || farOff(offX);
        outside = outside || farOff(offZ);
        if (!outside)
            continue;

        const reg32 facingByte = facing & 0xff;
        eax = (eax & 0xffffff00u) | facingByte;
        guest.setByte(drop + kDropFacing, std::uint8_t(facingByte));

        // Past the side either way: at the other side, anywhere along it.
        bool wrapped = false;
        if (jumpsAbove(compared(Float(offX), width())))
        {
            guest.setByte(drop + kDropSeen, 0);
            offX = float(-width());
            wrapped = true;
        }
        else if (jumpsAbove(compared(-width(), Float(offX))))
        {
            guest.setByte(drop + kDropSeen, 0);
            eax = guest.dword(kWeatherHalfWidth);
            offX = guest.floatAt(kWeatherHalfWidth);
            wrapped = true;
        }
        if (wrapped)
        {
            offY = randomHeight(guest, cpu);
            offZ = randomAcross(guest, cpu, kTwoAtReturn);
        }
        wrapped = false;
        if (jumpsAbove(compared(Float(offZ), width())))
        {
            guest.setByte(drop + kDropSeen, 0);
            wrapped = true;
        }
        else if (jumpsAbove(compared(-width(), Float(offZ))))
        {
            guest.setByte(drop + kDropSeen, 0);
            eax = guest.dword(kWeatherHalfWidth);
            wrapped = true;
        }
        if (wrapped)
        {
            offY = randomHeight(guest, cpu);
            offZ = randomAcross(guest, cpu, kTwoAtReturn);
        }
        wrapped = false;
        if (jumpsAbove(compared(Float(offY), floatFrom(guest, kWeatherHigh))))
        {
            guest.setByte(drop + kDropSeen, 0);
            eax = guest.dword(kWeatherLow);
            offY = guest.floatAt(kWeatherLow);
            wrapped = true;
        }
        else if (jumpsBelow(compared(Float(offY), floatFrom(guest, kWeatherLow))))
        {
            guest.setByte(drop + kDropSeen, 0);
            eax = guest.dword(kWeatherHigh);
            offY = guest.floatAt(kWeatherHigh);
            wrapped = true;
        }
        if (wrapped)
        {
            offX = randomAcross(guest, cpu, kTwoAtReturn);
            offZ = randomAcross(guest, cpu, kTwoAtReturn);
        }
        guest.setFloat(place, float(fp::add(floatFrom(guest, view + kViewPosition), Float(offX))));
        guest.setFloat(place + 4, float(fp::add(floatFrom(guest, view + kViewPosition + 4), Float(offY))));
        guest.setFloat(place + 8, float(fp::add(floatFrom(guest, view + kViewPosition + 8), Float(offZ))));
    }
    return eax;
}

namespace
{

// The frame of sub_4dece0: what is handed to the game by address.
constexpr reg32 kLocalList = 0x08;             // what is to draw: two dwords
constexpr reg32 kLocalSeen = 0x10;

}  // namespace

/* sub_4dece0: the weather's pass over a view: esi a list for the caller, eax the view.  How
 * much falls here and now, less on the lower effects; the drops moved; rain drawn in
 * streaks and how many were seen noted, snow in flakes.  The list goes back through the
 * caller's pointer, in eax, and esi is left on the frame's, as the original leaves it. */
x86::reg32 frontend::tail::weatherOnView(App* app, CPU& cpu, reg32 view, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x14);
    const reg32 list = guest.frame() + kLocalList;
    const reg32 seen = guest.frame() + kLocalSeen;
    guest.setDword(seen, 0);
    fe::direct<0x4bbe70, frontend::rendd::emptyList>(app, cpu, list);
    if (guest.dword(view) == 0 && guest.dword(kWeatherKind) != 0)
    {
        if (guest.dword(kCameraChanged + guest.dword(view + kViewPlayer) * 4) != 0)
            guest::direct<0x4dd230, weatherScatter>(app, cpu, view);
        const reg32 car = fe::direct<0x422ba0, carstate::carOfCamera>(app, cpu, guest.dword(view + kViewPlayer));
        const reg32 falling = guest::direct<0x4dd3d0, weatherFallAtNode>(app, cpu, guest.dword(car + kCarNode));
        sreg32 count = sreg32(guest::direct<0x4dd330, weatherFadeInTime>(app, cpu, falling));
        const reg32 level = gameSettings(guest).effectsLevel;  // 0 all, 1 half the weather, 2 a quarter of it
        if (level == 1)
            count /= 2;
        if (level == 2)
            count /= 4;
        guest::direct<0x4dd5e0, weatherDropsStep>(app, cpu, view, reg32(count));
        fe::direct<0x4dbda0, frontend::simd::viewProjection>(app, cpu, view);
        const reg32 kind = guest.dword(kWeatherKind);
        if (kind == 1)
        {
            guest::direct<0x4dd510, weatherViewChange>(app, cpu, guest.dword(view + kViewPlayer));
            if (count != 0)
            {
                fe::direct<0x4de700, render::streaksDrawn>(app, cpu, view, reg32(count), seen, list);
            }
            guest::direct<0x4dd4c0, weatherShare>(app, cpu, guest.dword(seen), guest.dword(view + kViewPlayer));
        }
        else if (kind == 2 && count != 0)
        {
            fe::direct<0x4de070, render::smokeDrawn>(app, cpu, view, reg32(count), list);
        }
        fe::direct<0x4dbde0, frontend::simc::setView>(app, cpu, view);
    }
    guest.setDword(out, guest.dword(list));
    guest.setDword(out + 4, guest.dword(list + 4));
    return out;
}

// ---------------------------------------------------------------------------
// The functions the game calls
// ---------------------------------------------------------------------------

/* sub_4be240: a block let go, if there is one. */
x86::reg32 frontend::tail::blockFree(App* app, CPU& cpu, reg32 block)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    return block != 0 ? *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block) : block;
}

// ---------------------------------------------------------------------------
// The thunks under the game's names: the arguments from the registers, the core in the frame it
// makes
// ---------------------------------------------------------------------------

x86::reg32 frontend::taskBlockFree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4be240", 0, 0, nullptr, 0, 0 };
    const reg32 block = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::blockFree(app, cpu, block); });
}

x86::reg32 frontend::taskListRun(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4be3a0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 list = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::runTaskList(app, cpu, list); });
}

x86::reg32 frontend::raceFrameRun(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c48d0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::raceFrame(app, cpu); });
}

x86::reg32 frontend::raceQueuesClear(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c7b50", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::queuesClear(app, cpu); });
}

x86::reg32 frontend::trackPlaceNear(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c8470", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 node = args.eax, out = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::placeNearNode(app, cpu, node, out); });
}

x86::reg32 frontend::trackPlaceAt(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c8550", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const reg32 node = args.eax, out = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::placeAtNode(app, cpu, node, out); });
}

x86::reg32 frontend::trackLaneOffset(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c9a80", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 node = args.eax, lane = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::laneOffset(app, cpu, node, lane); });
}

x86::reg32 frontend::weatherSetUp(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dcdf0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::weatherStart(app, cpu); });
}

x86::reg32 frontend::weatherScatterAtCut(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dd230", 0, 0, nullptr, 0 };
    const reg32 view = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::weatherScatter(app, cpu, view); });
}

x86::reg32 frontend::weatherFadeByTime(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dd330", 0, 0, nullptr, 0 };
    const reg32 amount = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::weatherFadeInTime(app, cpu, amount); });
}

x86::reg32 frontend::weatherFallingAtNode(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dd3d0", 0, 0, nullptr, 0 };
    const reg32 node = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::weatherFallAtNode(app, cpu, node); });
}

x86::reg32 frontend::weatherShareSeen(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dd4c0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 seen = args.eax, player = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::weatherShare(app, cpu, seen, player); });
}

x86::reg32 frontend::weatherViewChanged(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dd510", 0, 0, nullptr, 0 };
    const reg32 player = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::weatherViewChange(app, cpu, player); });
}

x86::reg32 frontend::weatherMoveDrops(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dd5e0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 view = args.eax, wanted = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::weatherDropsStep(app, cpu, view, wanted); });
}

x86::reg32 frontend::weatherPassOverView(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dece0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const reg32 view = args.eax, out = args.esi;
    // esi is left on the frame's list, as the original leaves it (the core's frame: 20 + 0x14 bytes, the list 8 in).
    const reg32 list = cpu.esp - (20 + 0x14) + 8;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
        const reg32 result = tail::weatherOnView(app, cpu, view, out);
        args.esi = list;
        return result;
    });
}

x86::reg32 frontend::angleSineCosine(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4eac60", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 angle = args.eax, sineOut = args.edx, cosineOut = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return tail::angleParts(app, cpu, angle, sineOut, cosineOut); });
}

}
