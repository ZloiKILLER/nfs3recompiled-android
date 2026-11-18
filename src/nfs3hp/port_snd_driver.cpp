#include "port_snd_stream.h"
#include "host_threads.h"
#include "isnd_calls.h"
#include "native_ea_threads.h"
#include "native_frontend_wina.h"
#include "port_snd_calls.h"
#include "port_snd_voices.h"

#include <cstdint>

/* EA's SND library's start and end (cmn\sinit.c, win\sdriver.c) and the sound
 * thread under it, rewritten (port.h says how): SNDinit checks the mode
 * against eacsnd's, loads eacsnd and has the game's message thread start the
 * driver -- the mixer, eacsnd's own start, the sound thread that serves the
 * library 100 times a second, the audio focus handler -- and waits for it;
 * SNDrestore undoes it all.
 *
 * The driver's state at 0x9ef600: +0 eacsnd's module, +4 the sound thread
 * started, +5 the focus handler added, +6 the restore registered at exit, +7
 * the message done, +8 the mode, +0xc the driver's result, +0x10 the mode
 * asked for, +0x14 the rate, +0x16 the rate set; eacsnd's functions at
 * +0x1c on (isnd.h's kDirect*), called through guest::sound (isnd_calls.h),
 * and at +0x54 what the driver's start calls for the effect bus: the game's
 * iSNDwinsetfx (0x502ff0), once SNDfxinitbus has put it there (setBusType,
 * port_snd_fx.cpp), else 0.
 * The sound thread's server, its thread and its event are at 0x79c150.
 *
 * What the callers read of the registers: SNDinit's (eight of them, all in
 * the game's options) `mov edx, eax` at once; iSNDmessageinit's, the game's
 * message window, only eax; the others' are the library's own, which read
 * what they keep.  So the thunks give back eax and nothing else (the
 * registers as they were), where the code leaves otherwise what it had no
 * use for: ecx, edx of iSNDmessageinit, edx of iSNDinit, SNDinit and
 * iSNDverifymode, ecx of SNDrestore. */
namespace nfs3hp
{
namespace port
{

namespace
{

using stream::safepoint;

constexpr Var<reg32> kDriverModule{0x9ef600}, kDriverMode{0x9ef608}, kDriverResult{0x9ef60c}, kDriverModeAsked{0x9ef610};
constexpr Var<uint8_t> kThreadStarted{0x9ef604}, kFocusAdded{0x9ef605}, kAtExit{0x9ef606}, kMessageDone{0x9ef607};
constexpr Var<reg32> kSetFx{0x9ef654};             // what the driver's start calls for the bus: iSNDwinsetfx, or 0
constexpr Var<reg32> kTicksHigh{0xa09b00};          // the high word of the 100 Hz tick's count
constexpr Var<reg32> kSoundTick{0xa09b08};          // the 100 Hz tick the game's TickSoundLambda calls
constexpr Table<reg32> kRestores{0xa09b28};         // what SNDrestore calls, in the order below
constexpr int32_t kRestoreOrder[] = {3, 4, 1, 2, 0};  // 0xa09b34, 0xa09b38, 0xa09b2c, 0xa09b30, 0xa09b28
constexpr Var<reg32> kLeaveAll{0x564420};           // what Error calls to leave the library's lock
constexpr Var<reg32> kSoundRunning{0x5643d8};       // the game's timer: its rate, 0 when stopped
constexpr Var<reg32> kBusType{0xa0a25c};            // the effect bus's type, which kSetFx's function gets
// The sound thread's (the game's sound server).
constexpr Var<reg32> kServer{0x79c150}, kServerThread{0x79c154}, kServerEvent{0x79c168};

constexpr reg32 kFreeLibrary = 0x5344dc;            // its import's slot
constexpr reg32 kMessageNumber = 0x8200;            // the message the driver's start and end are sent as
constexpr reg32 kRegisterHandler = 0x4cc6d0, kSendMessage = 0x4cc7c0, kFocusHandler = 0x4cc860;
constexpr reg32 kSleep = 0x4df8e0, kCreateThread = 0x4df7a0, kCreateEvent = 0x4dfb00, kSetEvent = 0x4dfb20;
constexpr reg32 kWaitEvent = 0x4dfb70, kCloseHandle = 0x4dfc00;
constexpr reg32 kAtExitRegister = 0x4f2a78, kMixInit = 0x4fff18, kLoadDriver = 0x4e1dd4, kWindowOfGame = 0x4eba70;
constexpr reg32 kCaps = 0x4e21b0, kStopAll = 0x4e9010, kMakeLock = 0x4eb2e0, kFreeLock = 0x4eb360;
// Functions of the driver's own the game's code is handed the address of.
constexpr reg32 kLeaveAllFunction = 0x4e1d1c, kMessageHandler = 0x4e225c, kRestoreFunction = 0x4e26b4;
constexpr reg32 kServerFunction = 0x4e2234, kFocusFunction = 0x4e1d48, kThreadFunction = 0x48ec40;
constexpr reg32 kMixerEnded = 0x4ffb64;             // what the mixer's start is told to call when a channel ends
constexpr reg32 kDefaultTick = 0x500c80;            // port::soundTick

constexpr Where kMessageError{0x549b10, 0x549f54};  // iSNDmessageinit's
constexpr Where kInitError{0x549b10, 0x549fbc};     // iSNDinit's
constexpr Where kVerifyError{0x54a098, 0x54a0a4};   // iSNDverifymode's

/* The function at kSetFx for the bus's effect, called as the game's code
 * calls it, from `esp`: its type in eax, what the mixer's start left in edx
 * (the channel's end) and the mode in ebx.  What setBusType puts there,
 * iSNDwinsetfx, as C++ (applyBusEffect): its frame where the call's return
 * address would be.  Anything else, through the pointer, ecx as it is.  Its eax; nothing when
 * the game stopped. */
Result busEffect(App* app, CPU& cpu, reg32 esp, reg32 type, reg32 mode)
{
#ifndef NFS3HP_SOUND_THROUGH_SLOTS
    if (app->thunkAt(kSetFx(app)) == &iSNDwinsetfx)
    {
        cpu.esp = esp;
        return applyBusEffect(app, cpu, esp - 4, type);
    }
#endif
    cpu.esp = esp;
    const std::optional<reg32> eax = guest::pointerCall(app, cpu, kSetFx(app), {type, kMixerEnded, mode});
    if (!eax)
        return {};
    return int32_t(*eax);
}

/* The cores: what the game's entries below do, with their arguments and a
 * result. */

/* 0x4e217c (iSNDunloaddll): eacsnd unloaded, when it was loaded.  0. */
Result unloadDriver(App* app, CPU& cpu, reg32 esp)
{
    const reg32 module = kDriverModule(app);
    if (module != 0)
    {
        reg32 freed;
        if (!callWindowsWords<kFreeLibrary, &host::freeLibrary>(app, cpu, esp - 3 * 4, freed, module))
            return {};
    }
    kDriverModule.set(app, 0);
    return 0;
}

/* 0x4e2228 (SNDsetrate): the offset the library adds to a sample's priority
 * (the low word of the argument; the game's options pick 0 or -50).  0. */
void setPriorityOffset(App* app, uint16_t offset)
{
    kPriorityOffset.set(app, int16_t(offset));
}

/* 0x4e1d1c: the library's lock left as often as it was entered (Error's way
 * out of it).  eax as the last leave gave it. */
Result leaveAllLocks(App* app, CPU& cpu, reg32 esp, reg32 eax)
{
    while (int8_t(kEntered(app)) > 0)
    {
        kEntered.set(app, uint8_t(kEntered(app) - 1));
        if (!callWindowsEax<kLeaveCriticalSection, &host::leaveLock>(app, cpu, esp - 2 * 4, eax, kLock(app)))
            return {};
    }
    return int32_t(eax);
}

/* 0x48ec80: the sound thread's server `server` (0 none): a first one starts the
 * thread (waited for until its event is there), none ends it (its event set
 * and the thread waited for until it is gone).  eax as the code leaves it: the
 * last call's. */
Result setServer(App* app, CPU& cpu, reg32 esp, reg32 server)
{
    const reg32 frame = esp - 5 * 4;
    reg32 eax = server;
    kServer.set(app, server);
    if (server != 0 && kServerEvent(app) == 0)
    {
        // The thread made.
        cpu.esp = frame - 4;  // the record's word pushed
        const std::optional<reg32> thread =
            guest::direct<kCreateThread, ea::threadStarted>(app, cpu, kThreadFunction, 0u, 3u, kServerThread.at);
        cpu.esp = frame;
        if (!thread)
            return {};
        eax = *thread;
        while (kServerEvent(app) == 0)
        {
            safepoint(app, cpu);
            cpu.esp = frame;
            const std::optional<reg32> slept = guest::direct<kSleep, ea::sleepFor>(app, cpu, 0u);
            if (!slept)
                return {};
            eax = *slept;
        }
    }
    else if (kServer(app) == 0 && kServerEvent(app) != 0)
    {
        const reg32 event = kServerEvent(app);
        // By address: eax too, which the code has the thread's place in (ea::eventPosted loads
        // only edx, the one it reads).
        cpu.esp = frame;
        const std::optional<reg32> set = guest::pointerCall(app, cpu, kSetEvent, {kServerThread.at, event});
        if (!set)
            return {};
        eax = *set;
        while (kServerEvent(app) != 0)
        {
            safepoint(app, cpu);
            cpu.esp = frame;
            const std::optional<reg32> slept = guest::direct<kSleep, ea::sleepFor>(app, cpu, 0u);
            if (!slept)
                return {};
            eax = *slept;
        }
    }
    return int32_t(eax);
}

/* 0x48ec40: the sound thread: its event made, the server (0x79c150) run each
 * time the event is set, until there is none.  eax as the event's close
 * gives it. */
Result serveSounds(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 2 * 4;
    cpu.esp = frame;
    const std::optional<reg32> made = guest::direct<kCreateEvent, ea::newAutoEvent>(app, cpu);
    if (!made)
        return {};
    kServerEvent.set(app, *made);
    for (;;)
    {
        safepoint(app, cpu);
        if (kServer(app) == 0)
            break;
        // The server through its pointer, the registers as they are (it takes none).
        cpu.esp = frame;
        if (!guest::pointerCall(app, cpu, kServer(app), {}).has_value())
            return {};
        cpu.esp = frame;
        if (!guest::direct<kWaitEvent, ea::eventWaited>(app, cpu, kServerEvent(app)))
            return {};
    }
    cpu.esp = frame;
    const std::optional<reg32> closed = guest::direct<kCloseHandle, ea::eventClosed>(app, cpu, kServerEvent(app));
    if (!closed)
        return {};
    kServerEvent.set(app, 0);
    return int32_t(*closed);
}

/* 0x4e2754 (iSNDverifymode): mode `mode` (-1 any) against eacsnd's caps: its
 * format (8 before 2, 4, 1), its rate (8, 0x10, 4, 0x20, 2, 1, in the second
 * byte) and stereo (0x20000) as both have them.  0 with an error for no format
 * or no rate.  `second` goes to SNDcaps as its eax (which it ignores). */
Result verifyMode(App* app, CPU& cpu, reg32 esp, reg32 mode, reg32 second)
{
    const reg32 frame = esp - 4 * 4;
    // SNDcaps (native_game.cpp's, no typed core) by address.
    cpu.esp = frame;
    const std::optional<reg32> got = guest::pointerCall(app, cpu, kCaps, {second});
    if (!got)
        return {};
    const reg32 caps = *got;
    const reg32 both = (mode == 0xffffffff ? caps : mode) & caps;
    const reg32 formats = both & 0xff, rates = both & 0xff00;
    const reg32 stereo = mode == 0xffffffff ? 0 : both & 0xff0000;
    if (formats == 0)
    {
        if (!fail(app, cpu, frame - 4, kVerifyError, 0xfc, {0x54a0b4}, both))
            return {};
        return 0;
    }
    if (rates == 0)
    {
        if (!fail(app, cpu, frame, kVerifyError, 0x104, {0x54a100}, both))
            return {};
        return 0;
    }
    reg32 result = formats & 8 ? 8 : formats & 2 ? 2 : formats & 4 ? 4 : 1;
    const reg32 r = rates >> 8;
    result |= (r & 8 ? 8 : r & 0x10 ? 0x10 : r & 4 ? 4 : r & 0x20 ? 0x20 : r & 2 ? 2 : 1) << 8;
    if (stereo & 0x20000)
        result |= 0x20000;
    return int32_t(result);
}

/* The focus handler set (the game's 0x4cc860, a menu file's: by address), its
 * eax back. */
std::optional<reg32> setFocus(App* app, CPU& cpu, reg32 frame, reg32 handler)
{
    cpu.esp = frame;
    return guest::pointerCall(app, cpu, kFocusHandler, {handler});
}

/* 0x4e225c (iSNDmessageinit, stdcall 24): the driver's message, on the game's
 * message thread, for window `window`: `command` 0 starts the driver -- eacsnd
 * checked for the mode, the rate taken from it, the mixer and eacsnd started,
 * the sound thread and the focus handler added -- another ends it.  The
 * result in 0x9ef60c, the message done.  0; `eax` is what the caller left,
 * which an error is told, and the last call's is what eacsnd's are handed. */
Result driverMessage(App* app, CPU& cpu, reg32 esp, reg32 window, reg32 command, reg32 eax)
{
    const reg32 frame = esp - 3 * 4;
    const auto done = [&]() -> Result {
        kMessageDone.set(app, 1);
        return 0;
    };
    if (command != 0)
    {
        if (kThreadStarted(app) != 0)
        {
            kThreadStarted.set(app, 0);
            const Result r = setServer(app, cpu, frame - 4, 0);
            if (!r)
                return {};
            eax = reg32(*r);
        }
        if (kFocusAdded(app) != 0)
        {
            const std::optional<reg32> set = setFocus(app, cpu, frame, 0);
            if (!set)
                return {};
            eax = *set;
            kFocusAdded.set(app, 0);
        }
        if (!guest::soundFrom<isnd::kDirectStop, &eacsnd::stop>(app, cpu, frame, eax))
            return {};
        kDriverResult.set(app, 0);
        return done();
    }
    if (window == 0)
    {
        const Result r = fail(app, cpu, frame - 4, kMessageError, 0x2d9, {0x549f64}, eax);
        if (!r)
            return {};
        eax = reg32(*r);
    }
    const reg32 mode = kDriverModeAsked(app);
    const std::optional<reg32> caps = guest::soundFrom<isnd::kDirectCaps, &eacsnd::caps>(app, cpu, frame, eax, window);
    if (!caps)
        return {};
    eax = *caps;
    if (int32_t(eax) <= 0)
    {
        kDriverResult.set(app, eax);
        return done();
    }
    const reg32 flags = mode >> 8;
    uint16_t rate;
    if (flags & 8)
        rate = 0x5622;
    else if (flags & 4)
        rate = 0x3e80;
    else if (flags & 0x10)
        rate = 0x7d00;
    else if (flags & 0x20)
        rate = 0xac44;
    else if (flags & 2)
        rate = 0x2b11;
    else
    {
        if (!fail(app, cpu, frame, kMessageError, 0x2f7, {0x549f98}, kMessageError.file))
            return {};
        kDriverResult.set(app, 0xfffffff8);
        return done();
    }
    kMixRate.set(app, rate);
    kDriverMode.set(app, mode);
    if (kAtExit(app) == 0)
    {
        // The restore registered to run at exit (the runtime's atexit, a menu file's: by address).
        cpu.esp = frame;
        const std::optional<reg32> registered = guest::pointerCall(app, cpu, kAtExitRegister, {kRestoreFunction});
        if (!registered)
            return {};
        eax = *registered;
        kAtExit.set(app, 1);
    }
    // The mixer's start, with the mode and what the mixer calls when a channel ends.
    cpu.esp = frame;
    const Result mixer = guest::direct<kMixInit, startMixer>(app, cpu, frame - 4, kDriverMode(app), kMixerEnded);
    if (!mixer)
        return {};
    eax = reg32(*mixer);
    if (kSetFx(app) != 0)
    {
        const Result set = busEffect(app, cpu, frame, kBusType(app), mode);
        if (!set)
            return {};
        eax = reg32(*set);
    }
    const std::optional<reg32> started =
        guest::soundFrom<isnd::kDirectStart, &eacsnd::start>(app, cpu, frame, eax, mode, window);
    if (!started)
        return {};
    eax = *started;
    kDriverResult.set(app, eax);
    if (int32_t(kDriverResult(app)) < 0)
        return done();
    if (kThreadStarted(app) == 0)
    {
        if (!setServer(app, cpu, frame - 4, kServerFunction))
            return {};
        kThreadStarted.set(app, 1);
    }
    if (kFocusAdded(app) == 0)
    {
        if (!setFocus(app, cpu, frame, kFocusFunction))
            return {};
        kFocusAdded.set(app, 1);
    }
    kDriverResult.set(app, 0);
    return done();
}

/* 0x4e253c (iSNDrestore): the driver ended by the message thread (or, with
 * none, its sound thread and focus handler taken here) and eacsnd unloaded.
 * The driver's result. */
Result stopSoundDriver(App* app, CPU& cpu, reg32 esp)
{
    const reg32 frame = esp - 3 * 4;
    kMessageDone.set(app, 0);
    // The message sent (a menu file's 0x4cc7c0: by address, with the four registers).
    cpu.esp = frame;
    const std::optional<reg32> sent = guest::pointerCall(app, cpu, kSendMessage, {kMessageNumber, 1, 0, 0});
    if (!sent)
        return {};
    if (*sent == 0)
    {
        if (kThreadStarted(app) != 0)
        {
            kThreadStarted.set(app, 0);
            if (!setServer(app, cpu, frame - 4, 0))
                return {};
        }
        if (kFocusAdded(app) != 0)
        {
            if (!setFocus(app, cpu, frame, 0))
                return {};
            kFocusAdded.set(app, 0);
        }
        kDriverResult.set(app, 0);
    }
    else
        while (kMessageDone(app) == 0)
        {
            safepoint(app, cpu);
            cpu.esp = frame;
            if (!guest::direct<kSleep, ea::sleepFor>(app, cpu, 0u))
                return {};
        }
    if (!unloadDriver(app, cpu, frame - 4))
        return {};
    kLeaveAll.set(app, 0);
    return int32_t(kDriverResult(app));
}

/* 0x4e2450 (iSNDinit): the driver started for mode `mode`: eacsnd loaded, the
 * message sent to the game's message thread and waited for.  The driver's
 * result; -16 without the timer or the window. */
Result startSoundDriver(App* app, CPU& cpu, reg32 esp, reg32 mode)
{
    const reg32 frame = esp - 3 * 4;
    kDriverModeAsked.set(app, mode);
    kLeaveAll.set(app, kLeaveAllFunction);
    // iSNDloaddll and the game's window (native_game.cpp's, no typed cores) by address.
    cpu.esp = frame;
    if (!guest::pointerCall(app, cpu, kLoadDriver, {mode}))
        return {};
    if (kSoundRunning(app) == 0)
    {
        if (!fail(app, cpu, frame, kInitError, 0x349, {0x549fc8}, kInitError.function))
            return {};
        return int32_t(0xfffffff0);
    }
    cpu.esp = frame;
    const std::optional<reg32> window = guest::pointerCall(app, cpu, kWindowOfGame, {});
    if (!window)
        return {};
    if (*window == 0)
    {
        if (!fail(app, cpu, frame - 2 * 4, kInitError, 0x351, {0x54a004}, *window))
            return {};
        return int32_t(0xfffffff0);
    }
    // The message's handler (a menu file's 0x4cc6d0, stdcall).
    cpu.esp = frame - 2 * 4;  // the message and the handler pushed
    guest::direct<kRegisterHandler, frontend::wina::handlerSet>(app, cpu, kMessageNumber, kMessageHandler);
    cpu.esp = frame;
    if (cpu.terminate)
        return {};
    kMessageDone.set(app, 0);
    cpu.esp = frame;
    if (!guest::pointerCall(app, cpu, kSendMessage, {kMessageNumber, 0, 0, 0}))
        return {};
    // The message thread's answer waited for, as the code waits: a slice
    // given up a time.
    while (kMessageDone(app) == 0)
    {
        safepoint(app, cpu);
        cpu.esp = frame;
        if (!guest::direct<kSleep, ea::sleepFor>(app, cpu, 0u))
            return {};
    }
    return int32_t(kDriverResult(app));
}

/* 0x4e2630 (SNDinit): the library started for mode `mode`: its voices (16 when
 * none were asked for), its tick, its lock, the master volume; the mode
 * verified, the driver started (undone when it failed).  0, or the driver's
 * error. */
Result startLibrary(App* app, CPU& cpu, reg32 esp, reg32 mode, reg32 second)
{
    const reg32 frame = esp - 3 * 4;
    if (kInited(app) != 0)
        return 0;
    if (kVoicesUsed(app) == 0)
        kVoicesUsed.set(app, 0x10);
    if (kSoundTick(app) == 0)
        kSoundTick.set(app, kDefaultTick);
    cpu.esp = frame;
    const std::optional<reg32> lock = guest::direct<kMakeLock, ea::newLock>(app, cpu);
    if (!lock)
        return {};
    kLock.set(app, *lock);
    kTicks.set(app, 0);
    kTicksHigh.set(app, 0);
    kMasterVolume.set(app, 0x7f);
    const Result verified = verifyMode(app, cpu, frame - 4, mode, second);
    if (!verified)
        return {};
    const Result started = startSoundDriver(app, cpu, frame - 4, reg32(*verified));
    if (!started)
        return {};
    if (*started < 0)
    {
        if (!stopSoundDriver(app, cpu, frame - 4))
            return {};
        return started;
    }
    kInited.set(app, 1);
    return 0;
}

}  // namespace

/* 0x4e26b4 (SNDrestore): the library ended: what registered a restore
 * restored (the game's, Pathfinder, the streamers, the effects), every sound
 * stopped, the effects' end told, the driver ended, the lock gone.  The
 * driver's result; -10 before SNDinit.  `eax` the caller's, which the first
 * restore is handed, each next the one before's answer. */
Result endLibrary(App* app, CPU& cpu, reg32 esp, reg32 eax)
{
    if (kInited(app) == 0)
        return int32_t(0xfffffff6);
    for (const int32_t i : kRestoreOrder)
        if (kRestores(app, i) != 0)
        {
            cpu.esp = esp;
            const std::optional<reg32> restored = guest::pointerCall(app, cpu, kRestores(app, i), {eax});
            if (!restored)
                return {};
            eax = *restored;
        }
    cpu.esp = esp;
    if (!guest::direct<kStopAll, stopAllSounds>(app, cpu, esp - 4))
        return {};
    if (kBankHook(app) != 0)
    {
        cpu.esp = esp;
        if (!guest::pointerCall(app, cpu, kBankHook(app), {0xffffffff}))
            return {};
    }
    // edx and ebx are pushed, to be popped again.
    const Result result = stopSoundDriver(app, cpu, esp - 2 * 4 - 4);
    if (!result)
        return {};
    kInited.set(app, 0);
    cpu.esp = esp - 2 * 4;
    if (!guest::direct<kFreeLock, ea::lockFreed>(app, cpu, kLock(app)))
        return {};
    return result;
}

namespace
{

}  // namespace

/* The game's entries: thunks around the cores above. */

x86::reg32 iSNDunloaddll(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, unloadDriver(app, cpu, e.esp));
}

x86::reg32 SNDsetrate(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.back(cpu, args, priorityOffsetSet(app, e.eax));
}

/* 0x4e2228's core: the priority offset set from `rate`'s low word; 0. */
reg32 priorityOffsetSet(App* app, reg32 rate)
{
    setPriorityOffset(app, uint16_t(rate));
    return 0;
}

/* 0x4e2630's core: startLibrary for `mode` and `second`; esp the frame base. */
Result libraryStarted(App* app, CPU& cpu, reg32 esp, reg32 mode, reg32 second)
{
    return startLibrary(app, cpu, esp, mode, second);
}

x86::reg32 leaveAudioLocks(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, leaveAllLocks(app, cpu, e.esp, e.eax));
}

x86::reg32 iSNDmessageinit(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, driverMessage(app, cpu, e.esp, e.arg(app, 1), e.arg(app, 4), e.eax), 0x18);
}

x86::reg32 iSNDinit(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, startSoundDriver(app, cpu, e.esp, e.eax));
}

x86::reg32 iSNDrestore(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, stopSoundDriver(app, cpu, e.esp));
}

x86::reg32 iSNDverifymode(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, verifyMode(app, cpu, e.esp, e.eax, e.edx));
}

x86::reg32 SNDinit(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, startLibrary(app, cpu, e.esp, e.eax, e.edx));
}

x86::reg32 SNDrestore(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, endLibrary(app, cpu, e.esp, e.eax));
}

x86::reg32 soundThread(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, serveSounds(app, cpu, e.esp));
}

x86::reg32 setSoundServer(App* app, CPU& cpu, x86::Args& args)
{
    const Entry e(cpu, args);
    return e.finish(cpu, args, setServer(app, cpu, e.esp, e.eax));
}

}  // namespace port
}  // namespace nfs3hp
