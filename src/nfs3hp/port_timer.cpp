#include "native_ea_threads.h"
#include "native_frontend_eab.h"
#include "port_snd_stream.h"
#include "port_timer.h"
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
 * (raceload::raceControlsTick) leaves alone.  Each is a thunk around a
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

// The timer's own, whose addresses are handed to the game's code and Windows.
constexpr reg32 kThreadFunction = 0x48eb90, kTickFunction = 0x48ed30, kStopFunction = 0x48ef50;
constexpr reg32 kClearFunction = 0x48ed00;

/* The registers the timer's thread is entered with, which it hands the functions registered to run
 * with each tick (a call through a pointer, as the code makes it). */
struct Regs
{
    reg32 eax = 0, edx = 0, ebx = 0, ecx = 0;
};

/* A typed core of another file called as C++ (guest::direct) with esp as the code has it at the
 * call; nullopt when the game stopped in it.  The menus' (native_frontend_*) throw
 * fe::Terminated when it did. */
template <reg32 Address, auto Core, typename... Args>
std::optional<reg32> menuCore(App* app, CPU& cpu, Args... args)
{
    try
    {
        const reg32 result = guest::direct<Address, Core>(app, cpu, args...);
        if (cpu.terminate)
            return std::nullopt;
        return result;
    }
    catch (const fe::Terminated&)
    {
        return std::nullopt;
    }
}

/* The calls the timer makes from its frame: `esp` where the game's code has it, `eax` what the
 * code has in eax -- the last call's result -- as a variable.  The game's own functions (the events',
 * the thread's, the sleep's, the runtime's atexit and the low timer's signal) are typed cores called
 * as C++; the one call through a pointer, the registered functions, hands over `a`'s registers. */
struct Calls
{
    App* app;
    CPU& cpu;
    reg32 esp;
    reg32 eax;
    bool take(const std::optional<reg32>& result)
    {
        if (!result)
            return false;
        eax = *result;
        return true;
    }
    /* 0x4dfb00: an event, auto-reset. */
    bool createEvent()
    {
        cpu.esp = esp;
        return take(guest::direct<0x4dfb00, ea::newAutoEvent>(app, cpu));
    }
    /* 0x4dfb20: `event` set. */
    bool setEvent(reg32 event)
    {
        cpu.esp = esp;
        return take(guest::direct<0x4dfb20, ea::eventPosted>(app, cpu, event));
    }
    /* 0x4dfb70: `event` waited on. */
    bool waitEvent(reg32 event)
    {
        cpu.esp = esp;
        return take(guest::direct<0x4dfb70, ea::eventWaited>(app, cpu, event));
    }
    /* 0x4dfc00: `event` closed. */
    bool closeEvent(reg32 event)
    {
        cpu.esp = esp;
        return take(guest::direct<0x4dfc00, ea::eventClosed>(app, cpu, event));
    }
    /* 0x4df8e0: `ms` slept. */
    bool sleep(reg32 ms)
    {
        cpu.esp = esp;
        return take(guest::direct<0x4df8e0, ea::sleepFor>(app, cpu, ms));
    }
    /* 0x4df7a0: a thread running `function`, its stack `stackBytes`, priority `priority`, into the record
     * at `record`. */
    bool createThread(reg32 function, reg32 stackBytes, reg32 priority, reg32 record)
    {
        cpu.esp = esp;
        return take(guest::direct<0x4df7a0, ea::threadStarted>(app, cpu, function, stackBytes, priority, record));
    }
    /* 0x4f2a78: the runtime's atexit. */
    bool atExit(reg32 function)
    {
        cpu.esp = esp;
        return take(menuCore<0x4f2a78, frontend::eab::atexitJump>(app, cpu, function));
    }
    /* 0x4f2a60: the low timer's event signalled, if it has one (eax as it is when it has none). */
    bool tickDone()
    {
        cpu.esp = esp;
        return take(menuCore<0x4f2a60, frontend::eab::lowTimerSignal>(app, cpu, eax));
    }
    /* A function through a pointer (a registered one), `a` its registers. */
    bool with(reg32 target, Args a, std::initializer_list<reg32> stack = {})
    {
        cpu.esp = esp;
        x86::Args registers;
        registers.eax = a.eax;
        registers.edx = a.edx;
        registers.ebx = a.ebx;
        registers.ecx = a.ecx;
        const std::optional<reg32> result = guest::pointerCall(app, cpu, target, registers, stack);
        if (!result)
            return false;
        eax = *result;
        return true;
    }
    /* A Windows function (`Function`) handed `words`: the port's own, called as C++
     * (native_windows.h). */
    template <reg32 Slot, auto Function, typename... Words>
    bool windows(Words... words)
    {
        cpu.esp = esp;
        return take(guest::windows<Slot, Function>(app, cpu, words...));  // every one of them returns a value
    }
};

/* 0x48eb90: the timer's thread: its event made, then each time the
 * multimedia timer sets it -- until the timer is stopped -- the ticks
 * counted, the time since the last turn taken into the milliseconds
 * (1193 / 65536 of the milliseconds' count: the code's fixed point), and the
 * registered functions run, each handed its place in the table times 4,
 * until one that is not there.  eax as the event's close leaves it. */
Result runTimer(App* app, CPU& cpu, reg32 esp, Regs r)
{
    Calls c{app, cpu, esp - 5 * 4, r.eax};
    if (!c.windows<kTimeGetTime, &host::ticks>())
        return {};
    reg32 last = c.eax;
    if (!c.createEvent())
        return {};
    int32_t fraction = 0;
    kTimerEvent.set(app, c.eax);
    for (;;)
    {
        safepoint(app, cpu);
        if (!c.waitEvent(kTimerEvent(app)))
            return {};
        if (kTimerRunning(app) == 0)
        {
            if (!c.closeEvent(kTimerEvent(app)))
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
            if (!c.with(kCallbacks(app, i), {reg32(i) * 4, r.edx, r.ebx, r.ecx}))
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
    const reg32 frame = esp - 4 * 4;
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
    if (event != 0 && kBusy(app) == 0 && !c.setEvent(event))
        return {};
    const reg32 server = kServerEvent(app);
    if (server != 0 && !c.setEvent(server))
        return {};
    if (!c.tickDone())
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
    Calls c{app, cpu, frame, eax};
    if (kTimerRunning(app) != 0)
    {
        const reg32 handle = kTimerHandle(app);
        kTimerRunning.set(app, 0);
        kTimerRate.set(app, 0);
        if (handle != 0 && !c.windows<kTimeKillEvent, &host::killTimer>(handle))
            return {};
        c.eax = kTimerResolution(app);
        if (!c.windows<kTimeEndPeriod, &host::timerPeriodEnd>(kTimerResolution(app)))
            return {};
        const reg32 event = kTimerEvent(app);
        if (event != 0 && !c.setEvent(event))
            return {};
        for (;;)
        {
            safepoint(app, cpu);
            if (kTimerHandle(app) == 0 || kTimerEvent(app) == 0)
                break;
            if (!c.sleep(1))
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
    const reg32 ebpValue = esp - 6 * 4;   // ebx, ecx, edx, esi, edi, ebp pushed; two locals below
    const reg32 frame = ebpValue - 8;
    Calls c{app, cpu, frame, rate};
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
        if (!c.createThread(kThreadFunction, 0, 2, kTimerThread))
            return {};
        if (c.eax == 0)
        {
            kTimerRunning.set(app, 0);
            kTimerRate.set(app, 0);
            return 0;
        }
        const reg32 resolution = kTimerResolution(app);
        if (!c.windows<kTimeBeginPeriod, &host::timerPeriodBegin>(resolution))
            return {};
        if (c.eax != 0)
        {
            const reg32 event = kTimerEvent(app);
            kTimerRate.set(app, 0);
            kTimerRunning.set(app, 0);
            if (!c.setEvent(event))
                return {};
            return int32_t(c.eax);
        }
        // The stop run at exit.
        if (!c.atExit(kStopFunction))
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
            if (!c.sleep(1))
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

/* The game's entries: thunks around the cores above. */

x86::reg32 timerThread(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, runTimer(app, cpu, e.esp, Regs{e.eax, e.edx, e.ebx, e.ecx}));
}

x86::reg32 clearTimerCallbacks(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, reg32(clearCallbacks(app)));
}

x86::reg32 timerTick(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, tickTimer(app, cpu, e.esp), 0x14);
}

x86::reg32 addTimerCallback(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, reg32(addCallback(app, e.eax)));
}

x86::reg32 removeTimerCallback(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, reg32(removeCallback(app, e.eax)));
}

x86::reg32 stopTimer(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, stopClock(app, cpu, e.esp, e.eax));
}

x86::reg32 startTimer(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, startClock(app, cpu, e.esp, e.eax));
}

/* The typed cores of the thunks above, for callers that call them as C++ (port_timer.h). */
std::optional<reg32> timerStarted(App* app, CPU& cpu, reg32 rate)
{
    const Result r = startClock(app, cpu, cpu.esp, rate);
    if (!r)
        return std::nullopt;
    return reg32(*r);
}

std::optional<reg32> timerStopped(App* app, CPU& cpu, reg32 unchanged)
{
    const Result r = stopClock(app, cpu, cpu.esp, unchanged);
    if (!r)
        return std::nullopt;
    return reg32(*r);
}

reg32 timerCallbackAdded(App* app, reg32 function)
{
    return reg32(addCallback(app, function));
}

reg32 timerCallbackRemoved(App* app, reg32 function)
{
    return reg32(removeCallback(app, function));
}

reg32 timerCallbacksCleared(App* app)
{
    return reg32(clearCallbacks(app));
}

}  // namespace port
}  // namespace nfs3hp
