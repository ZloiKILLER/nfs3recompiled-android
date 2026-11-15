#include "port_snd_stream.h"
#include "host_threads.h"

#include <lib/winapp.h>
#include <winapi/types.h>

#include <cstdint>

/* EA's timer (the game's clock, cmn\timer.c), rewritten (port.h says how): a
 * thread of its own that Windows' multimedia timer wakes through an event,
 * once a tick, and that runs the functions registered with it -- up to eight;
 * the game registers one, the race's controls (raceload::raceControlsTick) --
 * and counts the game's tick (0x79c1a4) and the thread's own (0x79c1a0); the
 * multimedia timer's callback that sets the next shot, re-arming itself, and
 * wakes the thread and the sound thread; the start and stop of the whole.
 *
 * The state, from 0x79c16c: +0 the whole milliseconds counted, +4 the step of
 * a tick (16.16 milliseconds: 1000 / rate), +8 the multimedia timer's
 * resolution, +0xc the time the next tick is due, +0x10 the thread's event,
 * +0x14 its thread's slot, +0x28 the multimedia timer's id, +0x2c the fraction
 * of a millisecond left over, +0x30 whether the timer runs, +0x34 the thread's
 * ticks and +0x38 the game's tick.  The callbacks' table is at 0x79c130 (the
 * code reaches it from 0x79c12c), the rate the timer was started at is
 * 0x5643d8 (0 while it does not run -- the sound library asks for it), and the
 * function Error calls to stop the callbacks is 0x56441c.
 *
 * What the game's callers read of the registers: the registration's, nothing
 * past eax (they keep edx across it, which the code does); the start's two
 * callers, nothing; the thread, the callback and the clearing are entered
 * from Windows or Error, and read no result.  The thread's loop keeps the
 * code's edx, ebx and esi in locals, which the one function registered
 * (raceload::raceControlsTick) leaves alone.  Each is an adapter around a
 * core, as port.h has it.
 *
 * Not here: 0x48eef0, 0x48ef20 and 0x48ef30 -- a count of the callbacks and
 * the busy count's increment and decrement -- which nothing calls. */
namespace nfs3hp
{
namespace port
{

namespace
{

using stream::Preserve;
using stream::residue;
using stream::safepoint;

constexpr int32_t kCallbackCount = 8;
constexpr Table<reg32> kCallbacks{0x79c130};
constexpr Var<reg32> kTimeWhole{0x79c16c}, kTimeStep{0x79c170}, kTimerResolution{0x79c174}, kTimeNext{0x79c178};
constexpr Var<reg32> kTimerEvent{0x79c17c}, kTimerHandle{0x79c194}, kTimeFraction{0x79c198}, kTimerRunning{0x79c19c};
constexpr Var<reg32> kThreadTicks{0x79c1a0}, kClockTicks{0x79c1a4};
constexpr Var<reg32> kServerEvent{0x79c168};
constexpr Var<reg32> kBusy{0x55e2c4}, kResyncs{0x55e2d0}, kStarts{0x55e2d4};
constexpr Var<reg32> kTimerRate{0x5643d8}, kExitFunction{0x56441c};
constexpr reg32 kTimerThread = 0x79c180, kServerThread = 0x79c154;  // thread slots, passed by address

// Windows' multimedia timer, by its imports' slots.
// kTimeGetTime: GetTickCount's slot (the game takes the time from it).
constexpr reg32 kTimeGetTime = 0x534568, kTimeSetEvent = 0x534660, kTimeKillEvent = 0x53465c;
constexpr reg32 kTimeBeginPeriod = 0x534650, kTimeEndPeriod = 0x534654, kTimeGetDevCaps = 0x534658;

constexpr reg32 kCreateEvent = 0x4dfb00, kSetEvent = 0x4dfb20, kWaitEvent = 0x4dfb70, kCloseEvent = 0x4dfc00;
constexpr reg32 kSleep = 0x4df8e0, kCreateThread = 0x4df7a0, kAtExit = 0x4f2a78, kTickDone = 0x4f2a60;
// The timer's own, whose addresses are handed to the game's code and Windows.
constexpr reg32 kThreadFunction = 0x48eb90, kTickFunction = 0x48ed30, kStopFunction = 0x48ef50;
constexpr reg32 kClearFunction = 0x48ed00;

/* A call into the game's code with eax as the code has it there: `go` leaves
 * the other argument registers as they were, `with` hands them over. */
struct Calls
{
    App* app;
    CPU& cpu;
    reg32 esp;
    reg32 eax;
    bool go(reg32 target, std::initializer_list<reg32> stack = {})
    {
        return callGame(app, cpu, esp, target, {eax, cpu.edx, cpu.ebx, cpu.ecx}, stack, eax);
    }
    bool with(reg32 target, Args a, std::initializer_list<reg32> stack = {})
    {
        return callGame(app, cpu, esp, target, a, stack, eax);
    }
    /* The same of a Windows function (`Function`), handed `words`: the
     * port's own, called as C++ (native_windows.h); in the check through its
     * import's slot `Slot` (port_snd.h's callWindows). */
    template <reg32 Slot, auto Function, typename... Words>
    bool windows(Words... words)
    {
        return windowsWith<Slot, Function>({eax, cpu.edx, cpu.ebx, cpu.ecx}, words...);
    }
    template <reg32 Slot, auto Function, typename... Words>
    bool windowsWith(Args a, Words... words)
    {
        return callWindows<Slot, Function>(app, cpu, esp, a, eax, words...);
    }
};

/* 0x48eb90: the timer's thread: its event made, then each time the
 * multimedia timer sets it -- until the timer is stopped -- the ticks
 * counted, the time since the last turn taken into the milliseconds
 * (1193 / 65536 of the milliseconds' count: the code's fixed point), and the
 * registered functions run, each handed its place in the table times 4,
 * until one that is not there.  eax as the event's close leaves it. */
Result runTimer(App* app, CPU& cpu, reg32 esp)
{
    const HeldEbp ebp(cpu, esp - 5 * 4);
    Calls c{app, cpu, esp - 5 * 4, cpu.eax};
    if (!c.windows<kTimeGetTime, &host::ticks>())
        return {};
    reg32 last = c.eax;
    if (!c.go(kCreateEvent))
        return {};
    int32_t fraction = 0;
    kTimerEvent.set(app, c.eax);
    for (;;)
    {
        safepoint(app, cpu);
        if (!c.with(kWaitEvent, {kTimerEvent(app), cpu.edx, cpu.ebx, cpu.ecx}))
            return {};
        if (kTimerRunning(app) == 0)
        {
            if (!c.with(kCloseEvent, {kTimerEvent(app), cpu.edx, cpu.ebx, cpu.ecx}))
                return {};
            // The event's place and the rate are cleared from edx, which the
            // flag's test left 0 and the close keeps.
            kTimerEvent.set(app, 0);
            kTimerRate.set(app, 0);
            return int32_t(c.eax);
        }
        kClockTicks.set(app, kClockTicks(app) + 1);
        kThreadTicks.set(app, kThreadTicks(app) + 1);
        if (!c.windows<kTimeGetTime, &host::ticks>())
            return {};
        const reg32 now = c.eax;
        fraction = int32_t(reg32(fraction) + (now - last) * 0x4a9u);
        last = now;
        if (fraction > 0xffff)
        {
            kTimeWhole.set(app, kTimeWhole(app) + reg32(fraction >> 16));
            fraction &= 0xffff;
        }
        for (int32_t i = 0; i < kCallbackCount && kCallbacks(app, i) != 0; ++i)
        {
            safepoint(app, cpu);
            if (!c.with(kCallbacks(app, i), {reg32(i) * 4, cpu.edx, cpu.ebx, cpu.ecx}))
                return {};
        }
    }
}

/* 0x48ed00: the registered functions forgotten (what Error calls).  8. */
int32_t clearCallbacks(App* app)
{
    for (int32_t i = 0; i < kCallbackCount; ++i)
        kCallbacks.set(app, i, 0);
    return kCallbackCount;
}

/* 0x48ed30 (stdcall, 5 words it does not read): the multimedia timer's
 * callback, once a tick: the next tick's time moved on by a step (the
 * fraction of a tick kept), and the one-shot timer set again to go off then --
 * in a millisecond when late, the time taken again when late by more than a
 * second -- unless the timer is stopped; the timer's thread woken, and the
 * sound thread; the runtime's hook.  eax as that call leaves it. */
Result tickTimer(App* app, CPU& cpu, reg32 esp)
{
    const Preserve keep(cpu);
    const reg32 frame = esp - 4 * 4;
    const HeldEbp ebp(cpu, frame);
    Calls c{app, cpu, frame, kTimeStep(app)};
    const reg32 sum = c.eax + kTimeFraction(app);
    const bool running = kTimerRunning(app) != 0;
    c.eax = reg32(int32_t(sum) >> 16);
    kTimeFraction.set(app, sum & 0xffff);
    kTimeNext.set(app, kTimeNext(app) + c.eax);
    if (running)
    {
        if (!c.windows<kTimeGetTime, &host::ticks>())
            return {};
        const reg32 late = kTimeNext(app) - c.eax;
        c.eax = late - 1;
        if (int32_t(c.eax) < 1)
        {
            if (int32_t(c.eax) < -1000)
            {
                if (!c.windows<kTimeGetTime, &host::ticks>())
                    return {};
                c.eax += 1;
                kTimeNext.set(app, c.eax);
                kResyncs.set(app, kResyncs(app) + 1);
            }
            c.eax = 1;
        }
        // timeSetEvent(delay, resolution, this callback, 0, one shot)
        if (!c.windows<kTimeSetEvent, &host::startTimer>(c.eax, kTimerResolution(app), kTickFunction, 0, 0))
            return {};
        kTimerHandle.set(app, c.eax);
        if (c.eax == 0)
        {
            const reg32 resolution = kTimerResolution(app);
            kTimerRate.set(app, 0);
            kTimerRunning.set(app, 0);
            if (!c.windows<kTimeEndPeriod, &host::timerPeriodEnd>(resolution))
                return {};
        }
    }
    const reg32 event = kTimerEvent(app);
    if (event != 0 && kBusy(app) == 0 && !c.with(kSetEvent, {kTimerThread, event, cpu.ebx, cpu.ecx}))
        return {};
    const reg32 server = kServerEvent(app);
    if (server != 0 && !c.with(kSetEvent, {kServerThread, server, cpu.ebx, cpu.ecx}))
        return {};
    if (!c.go(kTickDone))
        return {};
    return int32_t(c.eax);
}

/* 0x48ee30: `function` registered to run with every tick -- in the first
 * free place, unless it is there already.  The place the search ended at: 8
 * when it found none, the function's own when it was there.  (The busy
 * count is raised and lowered, in a register: it ends as it was.) */
int32_t addCallback(App* app, reg32 function)
{
    const reg32 busy = kBusy(app);
    int32_t free = -1, i = 0;
    for (; i < kCallbackCount; ++i)
    {
        const reg32 entry = kCallbacks(app, i);
        if (entry != 0 || free >= 0)
        {
            if (function == entry)
            {
                kBusy.set(app, busy);
                return i;
            }
        }
        else
            free = i;
    }
    if (free != -1)
        kCallbacks.set(app, free, function);
    kBusy.set(app, busy);
    return i;
}

/* 0x48ee90: `function` no longer registered -- the ones after it moved down a
 * place, the last place emptied.  The place the search ended at, after the
 * moving: 7 when it found it, 8 when it did not. */
int32_t removeCallback(App* app, reg32 function)
{
    const reg32 busy = kBusy(app);
    int32_t i = 0;
    while (i < kCallbackCount && kCallbacks(app, i) != function)
        ++i;
    if (i < kCallbackCount && kCallbacks(app, i) == function)
    {
        for (; i < kCallbackCount - 1; ++i)
            kCallbacks.set(app, i, kCallbacks(app, i + 1));
        kCallbacks.set(app, kCallbackCount - 1, 0);
    }
    kBusy.set(app, busy);
    return i;
}

/* 0x48ef50: the timer stopped, when it runs: the multimedia timer killed,
 * its period ended, the thread woken to find the flag and waited for until it
 * has closed its event; the state cleared.  eax as the last call leaves it
 * (`eax` as the code is entered with when it ran none). */
Result stopClock(App* app, CPU& cpu, reg32 esp, reg32 eax)
{
    const reg32 frame = esp - 4 * 4;
    const HeldEbp ebp(cpu, frame);
    Calls c{app, cpu, frame, eax};
    if (kTimerRunning(app) != 0)
    {
        const reg32 handle = kTimerHandle(app);
        kTimerRunning.set(app, 0);
        kTimerRate.set(app, 0);
        if (handle != 0 && !c.windows<kTimeKillEvent, &host::killTimer>(handle))
            return {};
        if (!c.windowsWith<kTimeEndPeriod, &host::timerPeriodEnd>({kTimerResolution(app), cpu.edx, cpu.ebx, 0},
                                                                         kTimerResolution(app)))
            return {};
        const reg32 event = kTimerEvent(app);
        if (event != 0 && !c.with(kSetEvent, {kTimerThread, event, cpu.ebx, 0}))
            return {};
        for (;;)
        {
            safepoint(app, cpu);
            if (kTimerHandle(app) == 0 || kTimerEvent(app) == 0)
                break;
            if (!c.with(kSleep, {1, cpu.edx, cpu.ebx, 0}))
                return {};
        }
    }
    kBusy.set(app, 0);
    kTimerResolution.set(app, 0);
    kTimerHandle.set(app, 0);
    kExitFunction.set(app, 0);
    kTimeFraction.set(app, 0);
    return int32_t(c.eax);
}

/* 0x48eff0: the timer started at `rate` ticks a second (1 to 10000, else
 * 100): any before it stopped, the step taken from the rate, the multimedia
 * timer's capabilities asked and its resolution taken as a millisecond -- a
 * tick of less than that does not start it --, the thread made, the period
 * begun, the stop registered to run at exit, the first tick made by hand, and
 * the game's tick waited for to move on for as long as 5 ticks; if it does
 * not the timer is stopped and started again, once more.  eax as the way out
 * leaves it. */
Result startClock(App* app, CPU& cpu, reg32 esp, reg32 rate)
{
    const Preserve keep(cpu);
    const reg32 ebpValue = esp - 6 * 4;   // ebx, ecx, edx, esi, edi, ebp pushed; two locals below
    const reg32 frame = ebpValue - 8;
    const HeldEbp ebp(cpu, ebpValue);
    Calls c{app, cpu, frame, cpu.eax};
    for (;;)
    {
        safepoint(app, cpu);
        // The stop at the top of a try: the rate stays in edi, eax is the
        // stop's.
        {
            const Result r = stopClock(app, cpu, frame - 4, c.eax);
            if (!r)
                return {};
            c.eax = reg32(*r);
        }
        int32_t ticksPerSecond = int32_t(rate);
        if (rate == 0 || ticksPerSecond < 1 || ticksPerSecond > 10000)
            ticksPerSecond = 100;
        reg32 flag = reg32(ticksPerSecond);   // esi
        c.eax = 0x3e80000u / reg32(ticksPerSecond);
        kTimeStep.set(app, c.eax);
        kTimeFraction.set(app, 0);
        kClockTicks.set(app, 0);
        kTimeWhole.set(app, 0);
        const reg32 capabilities = ebpValue - 8;
        c.eax = capabilities;
        if (!c.windows<kTimeGetDevCaps, &host::timerCaps>(capabilities, 8))
            return {};
        if (c.eax != 0)
        {
            kTimerRunning.set(app, 0);
            kTimerRate.set(app, 0);
            return int32_t(c.eax);
        }
        // The capabilities are overwritten by 1s (a minimum and a maximum
        // that nothing reads); the resolution is 1.
        kTimerResolution.set(app, 1);
        c.eax = 1u << 16;
        if (c.eax > kTimeStep(app))
        {
            kTimerRunning.set(app, 0);
            kTimerRate.set(app, 0);
            return int32_t(c.eax);
        }
        // The timer's thread: its function in eax, no parameter, priority 2.
        if (!c.with(kCreateThread, {kThreadFunction, 0, 2, 0xffffffff}, {kTimerThread}))
            return {};
        if (c.eax == 0)
        {
            kTimerRunning.set(app, 0);
            kTimerRate.set(app, 0);
            return 0;
        }
        const reg32 resolution = kTimerResolution(app);
        if (!c.windowsWith<kTimeBeginPeriod, &host::timerPeriodBegin>({c.eax, resolution, cpu.ebx, cpu.ecx},
                                                                             resolution))
            return {};
        if (c.eax != 0)
        {
            const reg32 event = kTimerEvent(app);
            kTimerRate.set(app, 0);
            kTimerRunning.set(app, 0);
            if (!c.with(kSetEvent, {kTimerThread, event, cpu.ebx, 0}))
                return {};
            return int32_t(c.eax);
        }
        // The stop run at exit.
        if (!c.with(kAtExit, {kStopFunction, resolution, cpu.ebx, cpu.ecx}))
            return {};
        kTimerRate.set(app, flag);
        kTimerRunning.set(app, flag);
        if (flag != 0 && kTimerHandle(app) == 0)
        {
            // The first tick by hand: the busy count 1 meanwhile.
            const reg32 busy = kBusy(app);
            kBusy.set(app, 1);
            c.eax = 1;
            if (!c.windows<kTimeGetTime, &host::ticks>())
                return {};
            kTimeNext.set(app, c.eax);
            // call 0x48ed30 with five words of 0, which it does not read
            const Result tick = tickTimer(app, cpu, frame - 5 * 4 - 4);
            if (!tick)
                return {};
            c.eax = reg32(*tick);
            kBusy.set(app, busy);
        }
        // The game's tick waited for to move on, for 5 ticks of the rate at most.
        const reg32 before = kClockTicks(app);
        c.eax = before;
        if (!c.windows<kTimeGetTime, &host::ticks>())
            return {};
        // eax = 5 * the rate (lea, add) while the first look at the clock is made.
        const reg32 started = c.eax;
        c.eax = 5 * kTimerRate(app);
        const reg32 deadline = started + c.eax;
        for (;;)
        {
            safepoint(app, cpu);
            if (!c.windows<kTimeGetTime, &host::ticks>())
                return {};
            if (c.eax >= deadline)
                break;
            c.eax = kClockTicks(app);
            if (before != c.eax)
                break;
            if (!c.with(kSleep, {1, cpu.edx, cpu.ebx, cpu.ecx}))
                return {};
        }
        c.eax = kClockTicks(app);
        if (before == c.eax)
        {
            const Result r = stopClock(app, cpu, frame - 4, c.eax);
            if (!r)
                return {};
            c.eax = reg32(*r);
            kStarts.set(app, kStarts(app) + 1);
            if (int32_t(kStarts(app)) < 2)
                continue;
        }
        if (kTimerRate(app) != 0)
            kExitFunction.set(app, kClearFunction);
        return int32_t(c.eax);
    }
}

}  // namespace

/* The game's entries: adapters around the cores above. */

void timerThread(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, runTimer(app, cpu, e.esp));
}

void clearTimerCallbacks(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(clearCallbacks(app)));
}

void timerTick(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, tickTimer(app, cpu, e.esp), 0x14);
}

void addTimerCallback(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(addCallback(app, e.eax)));
}

void removeTimerCallback(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(removeCallback(app, e.eax)));
}

void stopTimer(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stopClock(app, cpu, e.esp, e.eax));
}

void startTimer(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, startClock(app, cpu, e.esp, e.eax));
}

}  // namespace port
}  // namespace nfs3hp
