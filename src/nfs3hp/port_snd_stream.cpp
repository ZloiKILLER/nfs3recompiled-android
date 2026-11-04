#include "port_snd_stream.h"

#include <lib/thread.h>

#ifdef __ANDROID__
#include <SDL3/SDL.h>
#endif

#include <chrono>
#include <cstdint>
#include <cstdio>

/* EA's streaming, rewritten (port_snd_stream.h says how): the system tasks
 * the game's loop and the streams run on, the coordination of the stream's
 * top-up with the game's file loads, and the SND streamer that plays a CD
 * stream (music, a movie's sound) through a voice.
 *
 * The system tasks: 16 of 0x10 bytes at 0x9ef7e8 -- the function, its period
 * in ticks, the tick it runs next, whether it runs now.
 *
 * The streamer's state (sstream.c) at 0x564670..0x56480c: the CD stream it
 * plays, the queue of chunks to play and the one playing, the voice's handle,
 * the stream's patch, its state (0 off, 1 filling, 2 playing, 3 ending, 4
 * interrupted), the voice's chunk count, the playing chunk's state, the
 * loops' ring and the file's name.
 *
 * Each function is here twice: as a core -- arguments and a result, esp the
 * game's code has at its return address -- which the rest of the streams call
 * as C++, and as the game's entry, by the name the game's code knows
 * (disassemble_nfs3hp.py binds its address to it), an adapter around the
 * core.  The game's callers read of what the code leaves only eax and
 * the registers Watcom keeps; every other register goes back as it came.
 * The game's code the streamer calls that is not rewritten (threads, events,
 * files, the sound library's parts) is called as the game's code calls it,
 * from the stack where the original has it. */
namespace nfs3hp
{
namespace port
{
namespace stream
{

void safepoint(App* app, CPU& cpu)
{
    if (app->contextWanted())
        app->yieldContext(cpu);
}

void giveWay(App* app, CPU& cpu)
{
    app->unlockContext(cpu);
    win32::Thread::sleep(0);
    app->lockContext(cpu);
}

// What comes further on in this file.
Result purgeStreamer(App* app, CPU& cpu, reg32 esp);
Result dropQueued(App* app, CPU& cpu, reg32 esp);
Result startStreamVoice(App* app, CPU& cpu, reg32 esp);

namespace
{

#pragma pack(push, 1)
// A system task: the function, its period and next tick, whether it runs.
struct Task
{
    reg32 function;
    int32_t period;
    int32_t next;
    uint32_t running;
};

// A chunk the streamer has queued: the id of the chunk it was, now the next
// one queued; its length, and its data (the frames first).
struct Queued
{
    reg32 next;
    uint32_t size;
    uint32_t frames;
    uint8_t data[1];
};
#pragma pack(pop)

// The system tasks.
constexpr Records<Task> kTasks{0x9ef7e8};
constexpr int32_t kTaskCount = 0x10;
constexpr Var<reg32> kTaskDepth{0x564670};  // the tasks' nesting
constexpr Var<reg32> kTaskTick{0x564674};   // the tick the tasks last ran at
constexpr Var<reg32> kTaskTiming{0x56466c}, kTaskTime{0x564668};
constexpr Var<int32_t> kClock{0x79c1a0};    // the tasks' clock
constexpr Var<reg32> kSoundRunning{0x5643d8};  // the sound thread runs

// The streamer's.
constexpr Var<reg32> kCdStreamOf{0x56467c};  // the CD stream it plays
constexpr Var<reg32> kQueueHead{0x564680}, kQueueTail{0x564684}, kPlaying{0x564688};
constexpr Var<int32_t> kStreamHandle{0x564690};
constexpr Var<reg32> kChunksPlayed{0x564694};
constexpr Var<int32_t> kChunksQueued{0x564698};
constexpr Var<reg32> kBytesQueued{0x56469c};
constexpr reg32 kStreamPatch = 0x5646a0;  // the stream's patch, up to 0x5c bytes
constexpr Var<uint8_t> kPatchSet{0x5646fc}, kStreamState{0x5646fd}, kNoEnd{0x5646fe}, kVoiceChunks{0x5646ff};
constexpr Var<uint8_t> kFirstHeader{0x564700};
constexpr Var<int8_t> kStreamVoice{0x564701};
constexpr Var<uint8_t> kStreamVolume{0x564702}, kStreamPan{0x564703}, kStreamBend{0x564704}, kStreamEffect{0x564705};
constexpr Var<uint16_t> kStartTries{0x564706};
constexpr Var<uint8_t> kServiceAdded{0x564708};
constexpr Var<int8_t> kFilesQueued{0x564709}, kFilesStarted{0x56470a}, kSkipHeaders{0x56470b};
constexpr Var<uint8_t> kFormatKnown{0x56470c}, kChunkState{0x56470d};
constexpr Var<int8_t> kLoopNow{0x56470e}, kLoopLast{0x56470f};
constexpr Var<int32_t> kHeadersSeen{0x564710}, kHeadersWanted{0x564714}, kBytesWanted{0x564718};
constexpr Table<int32_t> kLoops{0x56471c};
constexpr Var<int32_t> kTopupWanted{0x56476c}, kTopupBy{0x564770};
constexpr Table<uint8_t> kOldFormat{0x564774}, kNewFormat{0x564780};
constexpr reg32 kStreamName = 0x56478c;        // the file's name, up to 0x80 bytes
constexpr Var<uint8_t> kStreamNameEnd{0x56480b};
constexpr Var<reg32> kEndsSeen{0x564678};
constexpr Var<reg32> kTopupHook{0x56480c};     // Pathfinder's, when it streams
constexpr Var<reg32> kNextChunk{0x5678b4};     // what the mixer calls for the next chunk
constexpr Var<reg32> kStreamerRestore{0xa09b30};
constexpr reg32 kStreamService = 0x4e85a8;

// The top-up's coordination (iocoord.c).
constexpr Var<reg32> kTopupFunction{0x9f6a50};
constexpr Var<reg32> kTopupEvent{0x567864}, kToppedUp{0x56785c};
constexpr Var<reg32> kLoadLock{0x564480};

// The game's functions the streams call.
constexpr reg32 kMainThread = 0x4df930, kCreateEvent = 0x4dfb00, kSetEvent = 0x4dfb20, kWaitEvent = 0x4dfb70;
constexpr reg32 kSleep = 0x4df8e0, kStrncpy = 0x4e0e30, kStrncmp = 0x4eb390;

// The sources of the errors, and their functions.
constexpr reg32 kStreamFile = 0x54ac34, kTaskFile = 0x54abf8;
constexpr Table<reg32> kWhere{0x552190};

/* Integer division as the code has it: a quotient towards 0 (and, as on the
 * phone's arm64 where the generated code runs, 0 for a divisor of 0). */
int32_t quotientOf(int32_t a, int32_t b) { return b == 0 ? 0 : idiv(a, b); }

/* The time the tasks' timing goes by: what the game's code has on a machine with
 * no timestamp counter, what is in eax. */
reg32 timestamp(CPU& cpu, reg32 fallback)
{
    cpu.eax = fallback;
    return cpu.eax;
}

}  // namespace

/* 0x4e7520 (addsystemtask): function `function` run every `period` ticks, the
 * first `delay` ticks from now; in the first free slot past as many as the
 * tasks are nested.  The tick it runs first. */
Result scheduleTask(App* app, CPU& cpu, reg32 esp, reg32 function, int32_t period, reg32 delay)
{
    const reg32 frame = esp - 4 * 4 - 4;
    reg32 skip = kTaskDepth(app);
    store<reg32>(app, frame, delay);
    kTaskDepth.set(app, skip + 1);
    int32_t slot = -1;
    for (int32_t i = 0; i < kTaskCount; ++i)
    {
        const reg32 f = kTasks(app, i).function;
        if (f == function)
            slot = i;
        else if (f == 0 && slot == -1)
        {
            if (skip == 0)
                slot = i;
            else
                --skip;
        }
    }
    if (slot == -1)
    {
        const Result r = fail(app, cpu, frame, Where{kTaskFile, 0x54ac08}, 0x60, {0x54ac18}, function);
        if (!r)
            return {};
        kTaskDepth.set(app, kTaskDepth(app) - 1);
        return r;
    }
    Task& task = kTasks(app, slot);
    task.function = function;
    task.period = period;
    const reg32 first = reg32(kClock(app)) + delay;
    task.running = 0;
    task.next = int32_t(first);
    kTaskDepth.set(app, kTaskDepth(app) - 1);
    return int32_t(first);
}

/* 0x4e75e0: `function` no longer a task.  Its slot's offset, 0x100 for none. */
reg32 unscheduleTask(App* app, reg32 function)
{
    reg32 offset = 0;
    int32_t index = 0;
    while (function != kTasks(app, index).function)
    {
        offset += 0x10;
        ++index;
        if (offset >= 0x100)
            break;
    }
    if (index < kTaskCount && kTasks(app, index).function == function)
        kTasks(app, index).function = 0;
    return offset;
}

/* 0x4e7630: the tasks due by the clock run, once a tick: each with `argument`
 * and the ticks it was late, not while it runs already.  What they gave back,
 * or-ed.  The code keeps its tasks' state in ebx, ecx, esi, edi and ebp,
 * where a task finds them. */
Result runDueTasks(App* app, CPU& cpu, reg32 esp, reg32 argument)
{
    const Preserve keep(cpu);
    const reg32 frame = esp - 6 * 4;
    reg32 result = 0, started = cpu.ebp;
    const bool timing = kTaskTiming(app) != 0;
    if (timing)
        started = timestamp(cpu, argument);
    // The code's safepoint before the tick's test (0x4e7648), on every call.
    safepoint(app, cpu);
    // eax as the loop leaves it: the clock, 0 after a task ran.
    reg32 left = reg32(kClock(app));
    if (kTaskTick(app) != reg32(kClock(app)))
    {
        kTaskTick.set(app, reg32(kClock(app)));
        for (int32_t i = 0; i < kTaskCount; ++i)
        {
            // The loop's safepoint, as the code's: the frame wait polls the
            // clock through here, and the timer's thread advances it only
            // with the guest's context handed over.
            safepoint(app, cpu);
            Task& task = kTasks(app, i);
            const reg32 function = task.function;
            if (function == 0)
                continue;
            left = reg32(kClock(app));
            if (kClock(app) < task.next || task.running != 0)
                continue;
            left = 0;
            task.running = 1;
            cpu.esi = result;
            cpu.edi = argument;
            cpu.ebp = started;
            reg32 gave;
            if (!callGame(app, cpu, frame, function,
                          {argument, reg32(kClock(app)) - reg32(task.next), function, reg32(i) * 0x10}, {}, gave))
                return {};
            result |= gave;
            kTasks(app, i).next = int32_t(reg32(kClock(app)) + reg32(kTasks(app, i).period));
            kTasks(app, i).running = 0;
        }
    }
    if (timing)
        kTaskTime.set(app, kTaskTime(app) + (timestamp(cpu, left) - started));
    return int32_t(result);
}

/* 0x4e7720: the tasks run until `ticks` of the timer have passed or one gave
 * something back -- what it gave.  A busy wait the timer's thread ends. */
Result runTasksFor(App* app, CPU& cpu, reg32 esp, reg32 ticks)
{
    const reg32 frame = esp - 2 * 4;
    const int32_t until = int32_t(ticks + reg32(kTimer(app)));
    reg32 result = 0;
    while (kTimer(app) < until && result == 0)
    {
        safepoint(app, cpu);
        giveWay(app, cpu);
        const Result r = runDueTasks(app, cpu, frame - 4, 0);
        if (!r)
            return {};
        result = reg32(*r);
    }
    return int32_t(result);
}

/* 0x4ff670: the stream topped up with `argument` before a load -- under the
 * load's lock but on the main thread -- or, with no top-up, the wait ended at
 * once. */
Result topUp(App* app, CPU& cpu, reg32 esp, reg32 argument)
{
    const Preserve keep(cpu);
    const reg32 frame = esp - 5 * 4;
    reg32 eax;
    Args a = residue(cpu);
    a.eax = 0;
    if (!callGame(app, cpu, frame, kMainThread, a, {}, eax))
        return {};
    const bool other = eax == 0;
    if (kTopupEvent(app) == 0)
    {
        if (!callGame(app, cpu, frame, kCreateEvent, residue(cpu), {}, eax))
            return {};
        kTopupEvent.set(app, eax);
    }
    if (kTopupFunction(app) != 0)
    {
        // The load's lock taken with the lock pushed from edi.
        if (other && !callGame(app, cpu, esp - 6 * 4, app->getMemory<reg32>(kEnterCriticalSection), residue(cpu),
                               {kLoadLock(app)}, eax))
            return {};
        kToppedUp.set(app, 0);
        cpu.ebp = 0;
        Args b = residue(cpu);
        b.eax = argument;
        b.ebx = argument;
        if (!callGame(app, cpu, frame, kTopupFunction(app), b, {}, eax))
            return {};
        if (other)
        {
            const Result r = leaveSection(app, cpu, frame, kLoadLock(app));
            if (!r)
                return {};
            eax = reg32(*r);
        }
        return int32_t(eax);
    }
    kToppedUp.set(app, 1);
    if (!callGame(app, cpu, frame, kSetEvent, {0, kTopupEvent(app)}, {}, eax))
        return {};
    return int32_t(eax);
}

/* 0x4ff780: the top-up done, its waiters let go.  `eax` is what the code has
 * there, which it gives back unless it sets the event. */
Result topUpDone(App* app, CPU& cpu, reg32 esp, reg32 eax)
{
    const reg32 frame = esp - 2 * 4;
    if (kTopupEvent(app) != 0)
    {
        kToppedUp.set(app, 1);
        if (!callGame(app, cpu, frame, kSetEvent, {0, kTopupEvent(app)}, {}, eax))
            return {};
    }
    return int32_t(eax);
}

Result topUpFinished(App* app, CPU& cpu, reg32 esp, reg32 how);

/* Port: how long the main thread waits for a top-up at most, in real time.
 * The game's own check ends one within a second of its clock; one that waits
 * on past this the check is not ending at all -- the menus froze so on the way
 * back from the showcase to the car screen, rarely, since 0.76, no stack to
 * say why.  Past it the top-up is ended as the check's timeout ends it, and
 * the state it was left in logged once. */
constexpr long kTopupWaitMostMs = 3000;

/* The top-up's state, as a wait cut short left it. */
void logTopupWaitCut(App* app)
{
    int32_t due = 0;
    uint32_t running = 0;
    bool found = false;
    for (int32_t i = 0; i < kTaskCount && !found; ++i)
    {
        const Task& task = kTasks(app, i);
        if (task.function != kStreamService)
            continue;
        found = true;
        due = task.next;
        running = task.running;
    }
    char service[64];
    if (found)
        std::snprintf(service, sizeof service, "due %d ticks ago, running %u", kTimer(app) - due, running);
    else
        std::snprintf(service, sizeof service, "not among the tasks");
#ifdef __ANDROID__
    SDL_Log
#else
    std::printf
#endif
        ("[SND] a top-up's wait cut after %ld ms: timer %d, tasks' clock %d (last ran %d), depth %d, wanted %d by %d,"
         " stream state %u, handle %d, service added %u, the stream's service %s",
         kTopupWaitMostMs, kTimer(app), kClock(app), int32_t(kTaskTick(app)), int32_t(kTaskDepth(app)),
         kTopupWanted(app), kTopupBy(app) - kTimer(app), unsigned(kStreamState(app)), kStreamHandle(app),
         unsigned(kServiceAdded(app)), service);
}

/* The error waittopupstream raises when a surface is locked, with the
 * registers it keeps pushed, and then the tasks run a ms apart until the
 * top-up is done (or there is no event): its wait on the main thread, entered
 * by a jump with edx and ebp pushed (`esp` is the code's after them).  eax
 * as it leaves.  Port: no longer than kTopupWaitMostMs. */
Result waitOnMainThread(App* app, CPU& cpu, reg32 esp)
{
    reg32 eax;
    if (!callGame(app, cpu, esp, kSurface, residue(cpu), {}, eax))
        return {};
    // The error with esi, ecx and ebx pushed (and popped after it).
    if (eax != 0)
    {
        const Result r = fail(app, cpu, esp - 12, Where{0x54e0c4, 0x54e0d4}, 0x5e, {0x54e0e4}, eax);
        if (!r)
            return {};
        eax = reg32(*r);
    }
    const auto since = std::chrono::steady_clock::now();
    for (;;)
    {
        safepoint(app, cpu);
        if (kTopupEvent(app) == 0)
            break;
        cpu.ebp = kToppedUp(app);
        if (cpu.ebp != 0)
            break;
        if (std::chrono::steady_clock::now() - since > std::chrono::milliseconds(kTopupWaitMostMs))
        {
            logTopupWaitCut(app);
            const reg32 edx = cpu.edx;
            const Result r = topUpFinished(app, cpu, esp - 4, 0);
            if (!r)
                return {};
            eax = reg32(*r);
            cpu.edx = edx;
            break;
        }
        Args sleep = residue(cpu);
        sleep.eax = 1;
        if (!callGame(app, cpu, esp, kSleep, sleep, {}, eax))
            return {};
        const Result r = runDueTasks(app, cpu, esp - 4, cpu.ebp);
        if (!r)
            return {};
        eax = reg32(*r);
    }
    return int32_t(eax);
}

/* 0x4ff76e: waittopupstream's wait on another thread: the event waited for
 * (`esp` the code's after the pushes). */
Result waitOnEvent(App* app, CPU& cpu, reg32 esp, reg32 event)
{
    reg32 eax;
    if (!callGame(app, cpu, esp, kWaitEvent, {event}, {}, eax))
        return {};
    return int32_t(eax);
}

/* 0x4ff7f0 (waittopupstream): the top-up waited for -- on the main thread by
 * the tasks, on another by its event (none, no wait). */
Result waitForTopup(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 2 * 4;  // edx and ebp pushed
    reg32 eax;
    Args a = residue(cpu);
    a.eax = 0;
    if (!callGame(app, cpu, frame, kMainThread, a, {}, eax))
        return {};
    // The waits are the code's jumps: on with the stack as it is.
    if (eax != 0)
        return waitOnMainThread(app, cpu, frame);
    const reg32 event = kTopupEvent(app);
    if (event != 0)
        return waitOnEvent(app, cpu, frame, event);
    return int32_t(eax);
}

/* 0x5017b4: the chunks the platform holds of a stream's: none here. */
int32_t platformHolds(int32_t) { return 0; }

/* 0x5016c0 (iSNDclearchunk): `frames` frames of silence at `buffer` in voice
 * `voice`'s format: XA's 8-byte header and 15 bytes a 28 frames, or PCM's
 * zeros.  eax as the code leaves it. */
Result clearChunk(App* app, CPU& cpu, reg32 esp, reg32 buffer, int32_t frames, int32_t voice)
{
    const PlatformVoice& p = kPlatform(app, voice);
    const reg32 compression = p.compression;
    reg32 eax = (buffer & 0xffff00ff) | compression << 8;
    if (compression == 7)
    {
        const int32_t blocks = quotientOf(int32_t(reg32(frames) + 0x1b), 0x1c);
        for (reg32 i = 0; i < 8; i += 2)
            store<uint16_t>(app, buffer + i, 0);
        int32_t bytes = imul(blocks, 15);
        if (p.channels == 2)
            bytes = int32_t(reg32(bytes) + reg32(bytes));
        eax = 0;
        for (int32_t i = 0; i < bytes; ++i)
            app->getMemory<uint8_t>(buffer + 8 + reg32(i)) = 0, eax = reg32(i + 1);
        return int32_t(eax);
    }
    if (compression == 9)
        return int32_t(eax);
    if (compression == 0)
    {
        if (p.channels == 2)
            frames = int32_t(reg32(frames) + reg32(frames));
        const reg32 bits = p.bits;
        eax = (buffer & 0xffff00ff) | bits << 8;
        if (bits == 0x10)
        {
            eax = 0;
            for (int32_t i = 0; i < frames; ++i)
                store<uint16_t>(app, buffer + 2 * reg32(i), 0), eax = reg32(i + 1);
        }
        else if (bits == 8 && frames > 0)
        {
            for (int32_t i = 0; i < frames; ++i)
                app->getMemory<uint8_t>(buffer + reg32(i)) = 0;
            eax = buffer + reg32(frames);
        }
        return int32_t(eax);
    }
    // Error with eax as the code has it.
    return fail(app, cpu, esp - 2 * 4, Where{0x54e5e8, 0x54e5f8}, 0x4a, {0x54e608}, eax);
}


namespace
{

// The streamer's errors as the game's code returns them.
constexpr int32_t kNotStarted = -14;  // before SNDinit
constexpr int32_t kNoRoom = -13, kNotPlaying = -16, kInterrupted = -18, kBadChunk = -15;

Queued& queued(App* app, reg32 chunk) { return record<Queued>(app, chunk); }

reg32 cdStreamAt(App* app) { return kCdStreamOf(app); }

}  // namespace

/* 0x4e7470 (SNDstreamautovol): the stream's voice to volume `volume` over
 * `ticks`. */
Result fadeStream(App* app, CPU& cpu, reg32 esp, int32_t ticks, int32_t volume)
{
    const reg32 frame = esp - 2 * 4;
    if (kInited(app) == 0)
        return kNotStarted;
    if (volume > 0x7f || volume < 0)
    {
        if (!fail(app, cpu, esp - 3 * 4, Where{0x54ab74, 0x54ab84}, 0x7c, {0x54ab98, reg32(volume)}, reg32(ticks)))
            return {};
        return -8;
    }
    kStreamVolume.set(app, uint8_t(volume));
    if (!fadeSound(app, cpu, frame - 4, reg32(kStreamHandle(app)), ticks, volume))
        return {};
    return 0;
}

/* 0x4e74f0: the stream's volume: its voice's, or what it was set to. */
Result streamVolume(App* app, CPU& cpu, reg32 esp)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const Result volume = playingLevel(app, cpu, esp - 1 * 4 - 4, reg32(kStreamHandle(app)));
    if (!volume)
        return {};
    return *volume < 0 ? int32_t(int8_t(kStreamVolume(app))) : *volume;
}

/* 0x4e8944: the streamer reset: nothing queued, no voice, the mixer's refill
 * its, and its service among the tasks. */
Result resetStreamerState(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 3 * 4;
    kNextChunk.set(app, 0x4e8328);
    kStreamerRestore.set(app, 0x4e784c);
    kHeadersSeen.set(app, 0);
    kChunksPlayed.set(app, 0);
    kChunksQueued.set(app, 0);
    kBytesQueued.set(app, 0);
    kQueueHead.set(app, 0);
    kQueueTail.set(app, 0);
    kStreamHandle.set(app, -1);
    kStreamState.set(app, 0);
    kLoopNow.set(app, 0);
    kLoopLast.set(app, 0);
    kLoops.set(app, kLoopLast(app), -1);
    kNoEnd.set(app, 1);
    kChunkState.set(app, 0);
    if (kServiceAdded(app) == 0)
    {
        if (!scheduleTask(app, cpu, frame - 4, kStreamService, 0, 1))
            return {};
        kServiceAdded.set(app, 1);
    }
    return 0;
}

/* 0x4e7750 (SNDstreamsetup): the streamer set up on CD stream `stream`, the
 * play's options at `options`. */
Result setUpStreamer(App* app, CPU& cpu, reg32 esp, reg32 stream, reg32 options)
{
    const reg32 frame = esp - 3 * 4;
    if (kInited(app) == 0)
        return kNotStarted;
    if (kStreamerRestore(app) != 0)
    {
        if (!fail(app, cpu, frame, Where{kStreamFile, 0x54ac44}, 0xb1, {0x54ac54}, stream))
            return {};
        return kNotPlaying;
    }
    const Result entered = enterAudio(app, cpu, frame - 4);
    if (!entered)
        return {};
    const int32_t result = platformHolds(*entered);
    if (result < 0)
    {
        if (!leaveAudio(app, cpu, frame - 4, reg32(result)))
            return {};
        return result;
    }
    const reg32 inner = esp - 4 * 4;
    app->getMemory<uint8_t>(kStreamName) = 0;
    kFirstHeader.set(app, 1);
    Var<reg32>{0x56468c}.set(app, 0);
    kStreamVolume.set(app, app->getMemory<uint8_t>(options + 8));
    kStreamPan.set(app, app->getMemory<uint8_t>(options + 7));
    kStreamBend.set(app, app->getMemory<uint8_t>(options + 9));
    kStreamEffect.set(app, app->getMemory<uint8_t>(options + 0xa));
    kCdStreamOf.set(app, stream);
    kPatchSet.set(app, 0);
    kFilesQueued.set(app, 0);
    kFilesStarted.set(app, 0);
    kHeadersWanted.set(app, 0xf);
    kSkipHeaders.set(app, -1);
    kFormatKnown.set(app, 0);
    const Result reset = resetStreamerState(app, cpu, inner - 4);
    if (!reset || !leaveAudio(app, cpu, inner - 4, reg32(*reset)))
        return {};
    return result;
}

/* 0x4e784c (SNDstreamrestore): the streamer done with: its stream purged,
 * its service gone. */
Result restoreStreamer(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 1 * 4;
    kTopupFunction.set(app, 0);
    if (kInited(app) == 0 || kStreamerRestore(app) == 0)
        return kNotStarted;
    if (!purgeStreamer(app, cpu, frame - 4))
        return {};
    if (kServiceAdded(app) != 0)
    {
        unscheduleTask(app, kStreamService);
        kServiceAdded.set(app, 0);
    }
    kStreamerRestore.set(app, 0);
    return 0;
}

/* 0x4e78a0 (SNDstreamqueue): file `name` queued to play, from `offset`: the
 * first starts the stream; another is read on as the stream needs it, but for
 * at most 19 files at once.  0, -13 for no room, -14 before the setup. */
Result queueStreamFile(App* app, CPU& cpu, reg32 esp, reg32 name, reg32 offset)
{
    const reg32 frame = esp - 3 * 4;
    if (kInited(app) == 0)
        return kNotStarted;
    if (kStreamerRestore(app) == 0)
    {
        if (!fail(app, cpu, frame, Where{kStreamFile, 0x54aca4}, 0x1c5, {0x54acb4}, name))
            return {};
        return kNotStarted;
    }
    reg32 eax = name;
    if (kStreamState(app) == 0)
    {
        if (offset != 0)
        {
            if (!purgeNamed(app, cpu, frame - 4, cdStreamAt(app), name, Command::Restart, true)
                || !queueArgument(app, cpu, frame - 4, cdStreamAt(app), offset, Command::ReadOn, false, true))
                return {};
        }
        else if (!purgeNamed(app, cpu, frame - 4, cdStreamAt(app), name, Command::Continue, true))
            return {};
        Result r = enterAudio(app, cpu, frame - 4);
        if (!r || !(r = resetStreamerState(app, cpu, frame - 4)))
            return {};
        kStreamState.set(app, 1);
        if (!leaveAudio(app, cpu, frame - 4, reg32(*r)))
            return {};
    }
    else
    {
        Result r = enterAudio(app, cpu, frame - 4);
        if (!r)
            return {};
        const bool full = int32_t(kFilesQueued(app)) + int32_t(kFilesStarted(app)) >= 0x13;
        if (!(r = leaveAudio(app, cpu, frame - 4, reg32(*r))))
            return {};
        if (full)
            return kNoRoom;
        if (!(r = commandBlocksAvailable(app, cpu, frame - 4, offset == 0 ? 1 : 2)))
            return {};
        if (*r == 0)
            return kNoRoom;
        if (offset == 0)
        {
            if (!queueNamed(app, cpu, frame - 4, cdStreamAt(app), name, Command::OpenNext, false, true))
                return {};
        }
        else
        {
            if (!callGame(app, cpu, frame, kStrncmp, {kStreamName, name, 0x80}, {}, eax))
                return {};
            if (eax != 0
                && !queueNamed(app, cpu, frame - 4, cdStreamAt(app), name, Command::QueueFile, false, true))
                return {};
            if (!queueArgument(app, cpu, frame - 4, cdStreamAt(app), offset, Command::ReadOn, false, true))
                return {};
        }
    }
    kFilesQueued.set(app, int8_t(kFilesQueued(app) + 1));
    if (!callGame(app, cpu, frame, kStrncpy, {kStreamName, name, 0x80}))
        return {};
    kStreamNameEnd.set(app, 0);
    return 0;
}

/* 0x4e7a14 (SNDstreaminterrupt): the stream cut to file `name` at `offset`:
 * what the reader holds past the voice's chunks dropped.  0; -18 while
 * already interrupted, -16 when not playing. */
Result interruptStream(App* app, CPU& cpu, reg32 esp, reg32 name, reg32 offset)
{
    const reg32 frame = esp - 3 * 4;
    if (kInited(app) == 0)
        return kNotStarted;
    if (kStreamerRestore(app) == 0)
    {
        if (!fail(app, cpu, frame, Where{kStreamFile, 0x54ace0}, 0x21b, {0x54acf4}, name))
            return {};
        return kNotStarted;
    }
    if (kStreamState(app) == 4)
        return kInterrupted;
    if (kStreamState(app) != 2)
        return kNotPlaying;
    if (!purgeNamed(app, cpu, frame - 4, cdStreamAt(app), name, Command::Switch, true)
        || !queueArgument(app, cpu, frame - 4, cdStreamAt(app), offset, Command::ReadOn, false, true))
        return {};
    Result r = enterAudio(app, cpu, frame - 4);
    if (!r)
        return {};
    reg32 c = kQueueHead(app);
    for (reg32 i = 0; int32_t(i) < int32_t(reg32(kVoiceChunks(app)) * 3) && c != kQueueTail(app); ++i)
        c = queued(app, c).next;
    kQueueTail.set(app, c);
    c = queued(app, c).next;
    while (c != 0)
    {
        const reg32 chunk = c;
        c = queued(app, c).next;
        if (!(r = giveBackChunk(app, cpu, frame - 4, cdStreamAt(app), chunk)))
            return {};
        kChunksQueued.set(app, kChunksQueued(app) - 1);
    }
    if (!leaveAudio(app, cpu, frame - 4, reg32(*r)))
        return {};
    kStreamState.set(app, 4);
    if (!callGame(app, cpu, frame, kStrncpy, {kStreamName, name, 0x80}))
        return {};
    kStreamNameEnd.set(app, 0);
    return 0;
}

/* 0x4e7b28 (SNDstreampurge): the stream stopped -- its voice, waited for --
 * and everything it held dropped. */
Result purgeStreamer(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 3 * 4;
    if (kInited(app) == 0)
        return kNotStarted;
    if (kStreamerRestore(app) == 0)
    {
        if (!fail(app, cpu, esp - 5 * 4, Where{kStreamFile, 0x54ad24}, 0x2b3, {0x54ad34}, cpu.eax))
            return {};
        return kNotStarted;
    }
    if (!stopSound(app, cpu, frame - 4, reg32(kStreamHandle(app))))
        return {};
    const reg32 deadline = kTicks(app) + 0xc8;
    for (;;)
    {
        const Result stopped = soundStatus(app, cpu, frame - 4, reg32(kStreamHandle(app)));
        if (!stopped)
            return {};
        if (*stopped != 0)
            break;
        safepoint(app, cpu);
        if (deadline < kTicks(app)
            && !fail(app, cpu, frame, Where{kStreamFile, 0x54ad24}, 0x2c6, {0x54ad60}, reg32(*stopped)))
            return {};
        if (!runDueTasks(app, cpu, frame - 4, 0))
            return {};
    }
    Result r = enterAudio(app, cpu, frame - 4);
    if (!r || !(r = dropQueued(app, cpu, frame - 4)))
        return {};
    kSkipHeaders.set(app, -1);
    kFormatKnown.set(app, 0);
    kFilesQueued.set(app, 0);
    kFilesStarted.set(app, 0);
    kStreamState.set(app, 0);
    app->getMemory<uint8_t>(kStreamName) = 0;
    kPatchSet.set(app, 0);
    kTopupFunction.set(app, 0);
    reg32 eax = 0;
    if (kServiceAdded(app) != 0)
    {
        eax = unscheduleTask(app, kStreamService);
        kServiceAdded.set(app, 0);
    }
    if (!leaveAudio(app, cpu, frame - 4, eax))
        return {};
    return 0;
}

/* 0x4e7c58 (SNDstreamchunksleft): the chunks queued to play. */
Result chunksLeft(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 1 * 4;
    int32_t result = kNotStarted;
    if (kInited(app) == 0)
        return result;
    if (kStreamerRestore(app) == 0)
    {
        if (!fail(app, cpu, esp - 4 * 4, Where{kStreamFile, 0x54ada4}, 0x34b, {0x54adb8}, cpu.eax))
            return {};
        return result;
    }
    Result r = enterAudio(app, cpu, frame - 4);
    if (!r)
        return {};
    if (kStreamState(app) != 0)
    {
        result = kChunksQueued(app) + platformHolds(kVoiceChunks(app));
        if (kChunkState(app) == 2)
            --result;
    }
    if (!leaveAudio(app, cpu, frame - 4, reg32(*r)))
        return {};
    return result;
}

/* 0x4e7ce0 (SNDstreambuftime): the stream's buffered time in ms: its chunks
 * left times 1000 over the voice's chunks a second. */
Result bufferedTime(App* app, CPU& cpu, reg32 esp)
{
    if (kInited(app) == 0)
        return kNotStarted;
    if (kStreamerRestore(app) == 0)
    {
        if (!fail(app, cpu, esp - 2 * 4, Where{kStreamFile, 0x54adec}, 0x3c2, {0x54ae00}, cpu.eax))
            return {};
        return kNotStarted;
    }
    reg32 base = esp - 3 * 4;
    Result r = enterAudio(app, cpu, base - 4);
    if (!r)
        return {};
    int32_t result = kNotStarted;
    if (kStreamState(app) != 0 && kStreamState(app) != 1)
    {
        base = esp - 4 * 4;
        if (kVoiceChunks(app) == 0
            && !fail(app, cpu, esp - 6 * 4, Where{kStreamFile, 0x54adec}, 0x3d3, {0x54ae30}, cpu.eax))
            return {};
        const Result left = chunksLeft(app, cpu, base - 4);
        if (!left)
            return {};
        result = quotientOf(imul(*left, 1000), int32_t(kVoiceChunks(app)));
        r = left;
    }
    if (!leaveAudio(app, cpu, base - 4, reg32(*r)))
        return {};
    return result;
}

/* 0x4e7dd0: the top-up wanted no more: done (its waiters let go), and
 * Pathfinder's hook told with `how`. */
Result topUpFinished(App* app, CPU& cpu, reg32 esp, reg32 how)
{
    const reg32 frame = esp - 1 * 4;
    kTopupWanted.set(app, 0);
    Result r = topUpDone(app, cpu, frame - 4, how);
    if (!r)
        return {};
    reg32 eax = reg32(*r);
    if (kTopupHook(app) != 0 && !callGame(app, cpu, frame, kTopupHook(app), {0, 0}, {}, eax))
        return {};
    return int32_t(eax);
}

/* 0x4e7df4 (iSNDstreamchecktopup): whether the top-up asked for is done --
 * the stream holds the time asked, its reader waits, or it has nothing more
 * to read -- or timed out. */
Result checkTopUp(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 1 * 4;
    if (kTopupWanted(app) == 0)
        return int32_t(cpu.eax);
    const Result time = bufferedTime(app, cpu, frame - 4);
    if (!time)
        return {};
    int32_t how = -1;
    if (*time >= kTopupWanted(app))
        how = 1;
    else if (readerIsWaiting(app, cdStreamAt(app)) != 0)
    {
        const int32_t held = platformHolds(kVoiceChunks(app));
        if (held > 1)
            return held;
        how = 0;
    }
    else if (readerIsIdle(app) && kTopupHook(app) == 0)
        how = 1;
    if (how != -1 && !topUpFinished(app, cpu, frame - 4, reg32(how)))
        return {};
    // The timer read into eax, what the function gives back unless timed out.
    const int32_t timer = kTimer(app);
    if (timer > kTopupBy(app))
    {
        const Result r = fail(app, cpu, esp - 2 * 4, Where{kStreamFile, 0x54ae8c}, 0x412, {0x54aea4}, 0x412);
        if (!r)
            return {};
        return topUpFinished(app, cpu, frame - 4, 0);
    }
    return timer;
}

/* 0x4e7e98 (iSNDstreamsetbuftime): a top-up to `wanted` ms asked for
 * (Pathfinder's hook adding its own), within as many seconds as the sound
 * thread runs. */
Result askTopUp(App* app, CPU& cpu, reg32 esp, int32_t wanted)
{
    const reg32 frame = esp - 3 * 4;
    if (kInited(app) == 0)
        return int32_t(cpu.eax);
    if (wanted <= 0)
        return fail(app, cpu, esp - 5 * 4, Where{kStreamFile, 0x54aedc}, 0x426, {0x54aef4}, reg32(wanted));
    if (kTopupHook(app) != 0)
    {
        reg32 added;
        if (!callGame(app, cpu, frame, kTopupHook(app), {reg32(wanted), reg32(wanted)}, {}, added))
            return {};
        wanted = int32_t(reg32(wanted) + added);
    }
    kTopupBy.set(app, int32_t(reg32(kTimer(app)) + kSoundRunning(app) * 60));
    kTopupWanted.set(app, wanted);
    reg32 eax;
    if (!callGame(app, cpu, frame, kSurface, residue(cpu), {}, eax))
        return {};
    if (eax == 0)
    {
        const Result r = checkTopUp(app, cpu, frame - 4);
        if (!r)
            return {};
        eax = reg32(*r);
    }
    return int32_t(eax);
}

/* 0x4e7ff8: the chunks queued and playing given back to the reader. */
Result dropQueued(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 2 * 4;
    Result r = enterAudio(app, cpu, frame - 4);
    if (!r)
        return {};
    if (kQueueHead(app) != 0)
    {
        while (kQueueHead(app) != kQueueTail(app))
        {
            const reg32 chunk = kQueueHead(app);
            kQueueHead.set(app, queued(app, chunk).next);
            if (!(r = giveBackChunk(app, cpu, frame - 4, cdStreamAt(app), chunk)))
                return {};
        }
        if (!(r = giveBackChunk(app, cpu, frame - 4, cdStreamAt(app), kQueueHead(app))))
            return {};
        if (kPlaying(app) != 0 && !(r = giveBackChunk(app, cpu, frame - 4, cdStreamAt(app), kPlaying(app))))
            return {};
    }
    kQueueHead.set(app, 0);
    kQueueTail.set(app, 0);
    kPlaying.set(app, 0);
    return leaveAudio(app, cpu, frame - 4, reg32(*r));
}

/* 0x4e8078: a data chunk ('SCDl') queued to play.  3. */
int32_t queueData(App* app, reg32 chunk)
{
    Queued& c = queued(app, chunk);
    kChunksQueued.set(app, kChunksQueued(app) + 1);
    kBytesQueued.set(app, kBytesQueued(app) + c.size);
    if (c.next != 0xffffffff)
        c.next = 0;
    if (kQueueHead(app) == 0)
    {
        kQueueHead.set(app, chunk);
        kQueueTail.set(app, chunk);
    }
    else
    {
        queued(app, kQueueTail(app)).next = chunk;
        kQueueTail.set(app, chunk);
    }
    return 3;
}

/* 0x4e80cc (iSNDparseheader): a header chunk ('SCHl'): its patch copied to
 * the stream's and resolved -- a later file's started again when its format
 * differs.  1. */
Result takeHeader(App* app, CPU& cpu, reg32 esp, reg32 chunk)
{
    const reg32 frame = esp - 6 * 4 - 0x10;
    if (kSkipHeaders(app) > 0)
        kSkipHeaders.set(app, int8_t(kSkipHeaders(app) - 1));
    const reg32 size = queued(app, chunk).size - 8;
    const reg32 header = chunk + 8;
    store<reg32>(app, frame + 0xc, header);
    reg32 eax = chunk;
    if ((int32_t(size) <= 0 || int32_t(size) > 0x5c)
        && !fail(app, cpu, frame, Where{kStreamFile, 0x54af34}, 0x4aa, {0x54af44, 0x5c, size}, eax))
        return {};
    if (kFirstHeader(app) != 0)
    {
        kFilesStarted.set(app, int8_t(kFilesStarted(app) + 1));
        kFilesQueued.set(app, int8_t(kFilesQueued(app) - 1));
        for (int32_t i = 0; i < 12; ++i)
            kOldFormat.set(app, i, kNewFormat(app, i));
        const Result info = sampleInfo(app, cpu, frame - 4, header, 0x564780);
        if (!info)
            return {};
        eax = reg32(*info);
        if (kFormatKnown(app) != 0)
        {
            bool differs = false;
            for (int32_t i = 0; i < 8 && !differs; ++i)
                differs = kNewFormat(app, i) != kOldFormat(app, i);
            if (differs)
            {
                if (kStreamState(app) != 2 && kStreamState(app) != 3)
                {
                    const Result started = startStreamVoice(app, cpu, frame - 4);
                    if (!started)
                        return {};
                    eax = reg32(*started);
                    if (int32_t(eax) < 0
                        && !fail(app, cpu, frame, Where{kStreamFile, 0x54af34}, 0x4c7, {0x54afbc}, eax))
                        return {};
                }
                kStreamState.set(app, 3);
            }
        }
        kFormatKnown.set(app, 1);
    }
    for (reg32 i = 0; i < size; ++i)
        app->getMemory<uint8_t>(kStreamPatch + i) = app->getMemory<uint8_t>(header + i);
    store<reg32>(app, frame, 0xffffffff);
    if (!resolvePatch(app, cpu, frame - 4, kStreamPatch, 0))
        return {};
    kPatchSet.set(app, 1);
    if (!giveBackChunk(app, cpu, frame - 4, cdStreamAt(app), chunk))
        return {};
    return 1;
}

/* 0x4e8258: a loop chunk ('SCCl'): its count into the loops' ring.  2. */
Result queueLoop(App* app, CPU& cpu, reg32 esp, reg32 chunk)
{
    const reg32 frame = esp - 2 * 4;
    if (kLoopNow(app) != kLoopLast(app) || kLoops(app, kLoopNow(app)) > 0)
        kLoopLast.set(app, int8_t(kLoopLast(app) + 1));
    if (kLoopLast(app) >= 0x14)
        kLoopLast.set(app, 0);
    kLoops.set(app, kLoopLast(app), int32_t(queued(app, chunk).frames));
    if (!giveBackChunk(app, cpu, frame - 4, cdStreamAt(app), chunk))
        return {};
    return 2;
}

/* 0x4e82c4: a seek chunk ('SCLl'): the reader sent back to its offset.  4. */
Result takeSeek(App* app, CPU& cpu, reg32 esp, reg32 chunk)
{
    const reg32 frame = esp - 2 * 4;
    if (!queueArgument(app, cpu, frame - 4, cdStreamAt(app), queued(app, chunk).frames, Command::Seek, true, true)
        || !giveBackChunk(app, cpu, frame - 4, cdStreamAt(app), chunk))
        return {};
    return 4;
}

/* 0x4e82ec: an end chunk ('SCEl'): counted, the queue's last marked the end
 * unless the stream goes on.  5. */
Result takeEnd(App* app, CPU& cpu, reg32 esp, reg32 chunk)
{
    const reg32 frame = esp - 2 * 4;
    kEndsSeen.set(app, kEndsSeen(app) + 1);
    if (kNoEnd(app) == 0)
        queued(app, kQueueTail(app)).next = 0xffffffff;
    if (!giveBackChunk(app, cpu, frame - 4, cdStreamAt(app), chunk))
        return {};
    return 5;
}

/* 0x4e7f30: a chunk the reader read handed on by its id: data, a header, a
 * loop, a seek, an end; another given back.  -15 for it, -14 before SNDinit. */
Result handOnChunk(App* app, CPU& cpu, reg32 esp, reg32 chunk)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const reg32 frame = esp - 3 * 4;
    const Result entered = enterAudio(app, cpu, frame - 4);
    if (!entered)
        return {};
    int32_t result = kBadChunk;
    reg32 eax = reg32(*entered);
    const reg32 id = record<ChunkHeader>(app, chunk).id;
    if (id == 0x6c444353)
    {
        result = queueData(app, chunk);
        eax = reg32(result);
    }
    else if (id == 0x6c484353 || id == 0x6c434353 || id == 0x6c4c4353 || id == 0x6c454353)
    {
        const Result h = id == 0x6c484353   ? takeHeader(app, cpu, frame - 4, chunk)
                         : id == 0x6c434353 ? queueLoop(app, cpu, frame - 4, chunk)
                         : id == 0x6c4c4353 ? takeSeek(app, cpu, frame - 4, chunk)
                                            : takeEnd(app, cpu, frame - 4, chunk);
        if (!h)
            return {};
        result = *h;
        eax = reg32(result);
    }
    else
    {
        const Result g = giveBackChunk(app, cpu, frame - 4, cdStreamAt(app), chunk);
        if (!g)
            return {};
        eax = reg32(*g);
    }
    if (!leaveAudio(app, cpu, frame - 4, eax))
        return {};
    return result;
}

/* 0x4e8328: the mixer's refill: the next chunk to play into [data] and its
 * frames into [frames] -- the one playing given back first, the loops counted
 * down; a chunk's first pass silenced.  1 once the chunk plays, 0 before; at
 * the stream's end none. */
Result refillChunk(App* app, CPU& cpu, reg32 esp, reg32 data, reg32 frames)
{
    const reg32 frame = esp - 5 * 4;
    int32_t result = 0;
    // The chunk taken off the queue: the one playing given back, the counts
    // moved on, the loops' ring stepped when this loop ran out.
    const auto take = [&](reg32 chunk) {
        kQueueHead.set(app, queued(app, chunk).next);
        const reg32 bytes = queued(app, chunk).size;
        if (kPlaying(app) != 0 && !giveBackChunk(app, cpu, frame - 4, cdStreamAt(app), kPlaying(app)))
            return false;
        kPlaying.set(app, chunk);
        kChunksQueued.set(app, kChunksQueued(app) - 1);
        kBytesQueued.set(app, kBytesQueued(app) - bytes);
        kChunksPlayed.set(app, kChunksPlayed(app) + 1);
        kLoops.set(app, kLoopNow(app), int32_t(reg32(kLoops(app, kLoopNow(app))) - 1));
        if (kLoops(app, kLoopNow(app)) <= 0)
        {
            kFilesStarted.set(app, int8_t(kFilesStarted(app) - 1));
            if (kLoopNow(app) != kLoopLast(app))
            {
                kLoopNow.set(app, int8_t(kLoopNow(app) + 1));
                if (kLoopNow(app) >= 0x14)
                    kLoopNow.set(app, 0);
            }
        }
        return true;
    };
    const uint8_t state = kChunkState(app);
    if ((state == 2 || state == 1) && kQueueHead(app) != kQueueTail(app))
    {
        if (!take(kQueueHead(app)))
            return {};
        kChunkState.set(app, 0);
    }
    if (kQueueHead(app) != kQueueTail(app))
    {
        const reg32 chunk = kQueueHead(app);
        store<reg32>(app, data, chunk + 0xc);
        store<reg32>(app, frames, queued(app, chunk).frames);
        if (!take(chunk))
            return {};
        return result;
    }
    if (queued(app, kQueueHead(app)).next == 0xffffffff)
    {
        if (!giveBackChunk(app, cpu, frame - 4, cdStreamAt(app), kQueueHead(app)))
            return {};
        store<reg32>(app, data, 0);
        store<reg32>(app, frames, 0);
        return result;
    }
    store<reg32>(app, data, kQueueHead(app) + 0xc);
    store<reg32>(app, frames, queued(app, kQueueHead(app)).frames);
    if (kChunkState(app) == 1)
    {
        if (!clearChunk(app, cpu, frame - 4, load<reg32>(app, data), int32_t(load<reg32>(app, frames)),
                        kStreamVoice(app)))
            return {};
        kChunkState.set(app, 2);
    }
    if (kChunkState(app) == 0)
        kChunkState.set(app, 1);
    if (kChunkState(app) == 2)
        result = 1;
    return result;
}

/* 0x4e89f8 (iSNDstreamstart): the stream's voice started on its patch, with
 * the stream's volume, pan, bend and effect level: its handle, its voice, its
 * chunks; the top-up the loads wait for set.  The handle; -1 while the last
 * still plays. */
Result startStreamVoice(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 1 * 4 - 0x10;
    const Result stopped = soundStatus(app, cpu, frame - 4, reg32(kStreamHandle(app)));
    if (!stopped)
        return {};
    if (*stopped == 0)
        return -1;
    if (kPatchSet(app) == 0
        && !fail(app, cpu, frame - 0x10, Where{kStreamFile, 0x54b040}, 0x714, {0x54b050}, reg32(*stopped)))
        return {};
    PlayOptions& options = record<PlayOptions>(app, frame);
    playDefaults(options);
    options.volume = int8_t(kStreamVolume(app));
    options.pan = int8_t(kStreamPan(app));
    options.bend = int8_t(kStreamBend(app));
    options.effectLevel = int8_t(kStreamEffect(app));
    const Result handle = playPatch(app, cpu, frame - 0xc - 4, kStreamPatch, options);
    if (!handle)
        return {};
    kStreamHandle.set(app, *handle);
    if (kStreamHandle(app) < 0)
    {
        kStartTries.set(app, uint16_t(kStartTries(app) + 1));
        if (kStartTries(app) > 0x2710
            && !fail(app, cpu, frame - 0xc, Where{kStreamFile, 0x54b040}, 0x737, {0x54b074}, reg32(*handle)))
            return {};
        return kStreamHandle(app);
    }
    kStreamVoice.set(app, int8_t(voiceOfHandle(app, reg32(kStreamHandle(app)))));
    kVoiceChunks.set(app, kVoices(app, kStreamVoice(app)).timbre6c);
    kStreamState.set(app, 2);
    kPatchSet.set(app, 0);
    kStartTries.set(app, 0);
    kTopupFunction.set(app, 0x4e7e98);
    return kStreamHandle(app);
}

/* 0x4e8b60 (SNDstreamvol): the stream's volume to `volume`. */
Result setStreamVolume(App* app, CPU& cpu, reg32 esp, int32_t volume)
{
    const reg32 frame = esp - 2 * 4;
    if (kInited(app) == 0)
        return kNotStarted;
    if (volume > 0x7f || volume < 0)
    {
        if (!fail(app, cpu, esp - 3 * 4, Where{0x54b0c0, 0x54b0cc}, 0x74, {0x54b0dc, reg32(volume)}, reg32(volume)))
            return {};
        return -8;
    }
    kStreamVolume.set(app, uint8_t(volume));
    if (!setVolume(app, cpu, frame - 4, reg32(kStreamHandle(app)), volume))
        return {};
    return 0;
}

namespace
{

/* What serviceStreamer does with each chunk the reader has: the next chunk
 * taken and handed on (parsed, -1 for none to parse; an error when it is
 * negative). */
struct Chunk
{
    reg32 chunk;
    int32_t parsed;
    bool error;
};

bool readChunk(App* app, CPU& cpu, reg32 frame, Chunk& c)
{
    const Result r = nextChunkToPlay(app, cpu, frame - 4, cdStreamAt(app));
    if (!r)
        return false;
    c.chunk = reg32(*r);
    c.parsed = -1;
    c.error = false;
    if (int32_t(c.chunk) > 0 || int32_t(c.chunk) <= -10)
    {
        const Result p = handOnChunk(app, cpu, frame - 4, c.chunk);
        if (!p)
            return false;
        c.parsed = *p;
        c.error = c.parsed < 0;
    }
    return true;
}

/* Whether the reader has enough of the headers to skip: false when the
 * service waits for it. */
bool readerReady(App* app)
{
    if (kSkipHeaders(app) != 0)
        return true;
    if (kBytesWanted(app) == -1)
        return false;
    if (int32_t(bytesBuffered(app, cdStreamAt(app))) < kBytesWanted(app) && readerIsWaiting(app, cdStreamAt(app)) == 0)
        return false;
    kSkipHeaders.set(app, -1);
    return true;
}

}  // namespace

/* 0x4e85a8 (iSNDstreamservice): the streamer's system task, by its state:
 * filling (the chunks parsed until the headers wanted are in, then the voice
 * started), playing (chunks parsed, the top-up checked), ending (the voice
 * stopped when the chunks run out), interrupted (the queue dropped and the
 * new file's chunks parsed). */
Result serviceStreamer(App* app, CPU& cpu, reg32 esp)
{
    reg32 eax;
    if (!callGame(app, cpu, esp, kSurface, residue(cpu), {}, eax))
        return {};
    if (eax != 0)
        return 0;
    const reg32 frame = esp - 3 * 4;
    const Result entered = enterAudio(app, cpu, frame - 4);
    if (!entered)
        return {};
    // Done: the lock left.
    const auto done = [&]() -> Result {
        if (!leaveAudio(app, cpu, frame - 4, eax))
            return {};
        return 0;
    };
    const uint8_t state = kStreamState(app);
    if (state == 1)
    {
        for (;;)
        {
            safepoint(app, cpu);
            if (kFirstHeader(app) == 0 && kHeadersSeen(app) >= kHeadersWanted(app))
                return done();
            if (!readerReady(app))
                return done();
            Chunk c;
            if (!readChunk(app, cpu, frame, c))
                return {};
            eax = c.chunk;
            if (int32_t(c.chunk) <= 0 && int32_t(c.chunk) > -10 && c.chunk != 0)
            {
                if (kHeadersSeen(app) == 0 || kFirstHeader(app) == 0)
                    return done();
                if (!startStreamVoice(app, cpu, frame - 4))
                    return {};
                return done();
            }
            if (c.chunk != 0)
            {
                if (c.error)
                {
                    const Result r = fail(app, cpu, frame, Where{kStreamFile, 0x54b000}, 0x61c, {0x54b014},
                                          reg32(c.parsed));
                    if (!r)
                        return {};
                    eax = reg32(*r);
                }
                else if (c.parsed == 3)
                    kHeadersSeen.set(app, kHeadersSeen(app) + 1);
                else if (c.parsed == 1 && kStreamState(app) != 1)
                    return done();
            }
            if (int32_t(c.chunk) > 0 || int32_t(c.chunk) <= -10)
                continue;
            if (kFirstHeader(app) == 0)
                return done();
            bool start = kHeadersSeen(app) >= kHeadersWanted(app);
            if (!start && kHeadersSeen(app) != 0)
                start = readerIsWaiting(app, cdStreamAt(app)) != 0;
            if (!start)
            {
                if (kHeadersSeen(app) == 0)
                    return done();
                if (!readerIsIdle(app))
                    return done();
            }
            if (!startStreamVoice(app, cpu, frame - 4))
                return {};
            return done();
        }
    }
    if (state == 2)
    {
        for (;;)
        {
            safepoint(app, cpu);
            if (!readerReady(app))
                return done();
            Chunk c;
            if (!readChunk(app, cpu, frame, c))
                return {};
            if (c.parsed == 1 || !(int32_t(c.chunk) > 0 || int32_t(c.chunk) <= -10))
                break;
        }
        if (!checkTopUp(app, cpu, frame - 4))
            return {};
        return done();
    }
    if (state == 3)
    {
        const Result left = chunksLeft(app, cpu, frame - 4);
        if (!left)
            return {};
        if (*left > 0)
            return done();
        if (!stopSound(app, cpu, frame - 4, reg32(kStreamHandle(app))))
            return {};
        const Result stopped = soundStatus(app, cpu, frame - 4, reg32(kStreamHandle(app)));
        if (!stopped)
            return {};
        if (*stopped == 0)
            return done();
        kFilesStarted.set(app, int8_t(kFilesStarted(app) - 1));
        if (!dropQueued(app, cpu, frame - 4) || !resetStreamerState(app, cpu, frame - 4))
            return {};
        kStreamState.set(app, 1);
        return done();
    }
    if (state == 4)
    {
        if (!checkTopUp(app, cpu, frame - 4))
            return {};
        if (int32_t(bytesBuffered(app, cdStreamAt(app))) <= 0x1388)
            return done();
        if (kQueueHead(app) != 0)
        {
            while (kQueueHead(app) != kQueueTail(app))
            {
                const reg32 chunk = kQueueHead(app);
                kQueueHead.set(app, queued(app, chunk).next);
                if (chunk != kPlaying(app) && !giveBackChunk(app, cpu, frame - 4, cdStreamAt(app), chunk))
                    return {};
            }
            if (!giveBackChunk(app, cpu, frame - 4, cdStreamAt(app), kQueueHead(app)))
                return {};
        }
        kQueueHead.set(app, 0);
        kQueueTail.set(app, 0);
        kFilesQueued.set(app, 1);
        kFilesStarted.set(app, 0);
        kHeadersSeen.set(app, 0);
        kChunksPlayed.set(app, 0);
        kChunksQueued.set(app, 0);
        kBytesQueued.set(app, 0);
        kLoopNow.set(app, 0);
        kLoopLast.set(app, 0);
        kLoops.set(app, kLoopLast(app), -1);
        kChunkState.set(app, 0);
        for (;;)
        {
            safepoint(app, cpu);
            const Result chunk = nextChunkToPlay(app, cpu, frame - 4, cdStreamAt(app));
            if (!chunk)
                return {};
            if (*chunk > 0 || *chunk <= -10)
            {
                if (!handOnChunk(app, cpu, frame - 4, reg32(*chunk)))
                    return {};
                continue;
            }
            break;
        }
        kStreamState.set(app, 2);
        return done();
    }
    return done();
}

}  // namespace stream

/* The game's entries. */

void addSystemTask(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::scheduleTask(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}

void removeSystemTask(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, stream::unscheduleTask(app, e.eax));
}

void runSystemTasks(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::runDueTasks(app, cpu, e.esp, e.eax));
}

void waitSystemTasks(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::runTasksFor(app, cpu, e.esp, e.eax));
}

void setTopupFunction(App* app, CPU& cpu)
{
    const Entry e(cpu);
    stream::kTopupFunction.set(app, e.eax);
    e.back(cpu, e.eax);
}

void topUpStream(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::topUp(app, cpu, e.esp, e.eax));
}

void topupDone(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::topUpDone(app, cpu, e.esp, e.eax));
}

void toppedUp(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, stream::kToppedUp(app));
}

/* The fragments of waittopupstream entered by a jump with edx and ebp pushed
 * (their esp is the code's after the pushes): they pop the two and return for
 * the function's caller. */
namespace
{

void popTwoAndReturn(App* app, CPU& cpu, const Entry& e, reg32 eax)
{
    const reg32 ebp = app->getMemory<reg32>(e.esp), edx = app->getMemory<reg32>(e.esp + 4);
    e.back(cpu, eax, 8);
    cpu.ebp = ebp;
    cpu.edx = edx;
}

}  // namespace

void waitTopupMain(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const Result r = stream::waitOnMainThread(app, cpu, e.esp);
    if (r)
        popTwoAndReturn(app, cpu, e, reg32(*r));
}

void waitTopupEvent(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const Result r = stream::waitOnEvent(app, cpu, e.esp, e.edx);
    if (r)
        popTwoAndReturn(app, cpu, e, reg32(*r));
}

void waitTopupDone(App* app, CPU& cpu)
{
    const Entry e(cpu);
    popTwoAndReturn(app, cpu, e, e.eax);
}

void waitTopupStream(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::waitForTopup(app, cpu, e.esp));
}

void topUpBeforeLoad(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (stream::kTopupFunction(app) == 0 || stream::kSoundRunning(app) == 0)
        return e.back(cpu, e.eax);
    if (!stream::topUp(app, cpu, e.esp - 4, e.eax))
        return;
    e.finish(cpu, stream::waitForTopup(app, cpu, e.esp));
}

void topupReturn(App*, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, e.eax);
}

void loadBlockSize(App*, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, 0x3e8);
}

void loadFlags(App*, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, 0);
}

void platformChunks(App*, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(stream::platformHolds(int32_t(e.eax))));
}

void iSNDclearchunk(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::clearChunk(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx)));
}

void SNDstreamautovol(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::fadeStream(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx)));
}

void SNDstreamgetvol(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::streamVolume(app, cpu, e.esp));
}

void SNDstreamsetup(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::setUpStreamer(app, cpu, e.esp, e.eax, e.edx));
}

void SNDstreamrestore(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::restoreStreamer(app, cpu, e.esp));
}

void SNDstreamqueue(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::queueStreamFile(app, cpu, e.esp, e.eax, e.edx));
}

void SNDstreaminterrupt(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::interruptStream(app, cpu, e.esp, e.eax, e.edx));
}

void SNDstreampurge(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::purgeStreamer(app, cpu, e.esp));
}

void SNDstreamchunksleft(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::chunksLeft(app, cpu, e.esp));
}

void SNDstreambuftime(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::bufferedTime(app, cpu, e.esp));
}

void streamToppedUp(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::topUpFinished(app, cpu, e.esp, e.eax));
}

void iSNDstreamchecktopup(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::checkTopUp(app, cpu, e.esp));
}

void iSNDstreamsetbuftime(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::askTopUp(app, cpu, e.esp, int32_t(e.eax)));
}

void parseStreamChunk(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::handOnChunk(app, cpu, e.esp, e.eax));
}

void dropQueuedChunks(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::dropQueued(app, cpu, e.esp));
}

void queueDataChunk(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(stream::queueData(app, e.eax)));
}

void iSNDparseheader(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::takeHeader(app, cpu, e.esp, e.eax));
}

void queueLoopChunk(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::queueLoop(app, cpu, e.esp, e.eax));
}

void streamSeekChunk(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::takeSeek(app, cpu, e.esp, e.eax));
}

void streamEndChunk(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::takeEnd(app, cpu, e.esp, e.eax));
}

void streamEndsSeen(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, stream::kEndsSeen(app));
}

void nextStreamChunk(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::refillChunk(app, cpu, e.esp, e.eax, e.edx));
}

void iSNDstreamservice(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::serviceStreamer(app, cpu, e.esp));
}

void resetStreamer(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::resetStreamerState(app, cpu, e.esp));
}

void iSNDstreamstart(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::startStreamVoice(app, cpu, e.esp));
}

void SNDstreamvol(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stream::setStreamVolume(app, cpu, e.esp, int32_t(e.eax)));
}

}  // namespace port
}  // namespace nfs3hp
