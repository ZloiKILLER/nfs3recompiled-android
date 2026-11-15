#include "port_snd_stream.h"
#include "host_threads.h"
#include "isnd_calls.h"

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
 * what they keep.  So the adapters give back eax and nothing else (the
 * registers as they were), where the code leaves otherwise what it had no
 * use for: ecx, edx of iSNDmessageinit, edx of iSNDinit, SNDinit and
 * iSNDverifymode, ecx of SNDrestore. */
namespace nfs3hp
{
namespace port
{

namespace
{

using stream::Preserve;
using stream::residue;
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
 * calls it, from `esp` with the registers `a` (its type in eax).  What
 * setBusType puts there, iSNDwinsetfx, as C++ (applyBusEffect): its frame
 * where the call's return address would be, every register but eax kept as
 * its adapter keeps them.  Anything else, and in tools/game_checks (which
 * stands the pointer in) any, through the pointer.  False when the game
 * stopped. */
bool busEffect(App* app, CPU& cpu, reg32 esp, Args a)
{
#ifndef NFS3HP_SOUND_THROUGH_SLOTS
    if (app->methodAt(kSetFx(app)) == &iSNDwinsetfx)
    {
        const reg32 esi = cpu.esi, edi = cpu.edi, ebp = cpu.ebp;
        cpu.esp = esp;
        cpu.eax = a.eax;
        cpu.edx = a.edx;
        cpu.ebx = a.ebx;
        cpu.ecx = a.ecx;
        const Result result = applyBusEffect(app, cpu, esp - 4, a.eax);
        if (!result)
            return false;
        cpu.eax = reg32(*result);
        cpu.ebx = a.ebx;
        cpu.ecx = a.ecx;
        cpu.edx = a.edx;
        cpu.esi = esi;
        cpu.edi = edi;
        cpu.ebp = ebp;
        cpu.esp = esp;
        return true;
    }
#endif
    return callGame(app, cpu, esp, kSetFx(app), a);
}

/* The cores: what the game's entries below do, with their arguments and a
 * result. */

/* 0x4e217c (iSNDunloaddll): eacsnd unloaded, when it was loaded.  0. */
Result unloadDriver(App* app, CPU& cpu, reg32 esp)
{
    const Preserve keep(cpu);
    const reg32 module = kDriverModule(app);
    if (module != 0)
    {
        Args a = residue(cpu);
        a.edx = module;
        reg32 freed;
        if (!callWindows<kFreeLibrary, &host::freeLibrary>(app, cpu, esp - 3 * 4, a, freed, module))
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
    const Preserve keep(cpu);
    while (int8_t(kEntered(app)) > 0)
    {
        kEntered.set(app, uint8_t(kEntered(app) - 1));
        if (!callWindows<kLeaveCriticalSection, &host::leaveLock>(
                app, cpu, esp - 2 * 4, {eax, kLock(app), cpu.ebx, cpu.ecx}, eax, kLock(app)))
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
    const Preserve keep(cpu);
    const reg32 frame = esp - 5 * 4;
    const HeldEbp ebp(cpu, frame);
    reg32 eax = server;
    kServer.set(app, server);
    if (server != 0 && kServerEvent(app) == 0)
    {
        if (!callGame(app, cpu, frame, kCreateThread, {kThreadFunction, 0, 3, 0xffffffff}, {kServerThread.at}, eax))
            return {};
        while (kServerEvent(app) == 0)
        {
            safepoint(app, cpu);
            if (!callGame(app, cpu, frame, kSleep, {0, cpu.edx, cpu.ebx, cpu.ecx}, {}, eax))
                return {};
        }
    }
    else if (kServer(app) == 0 && kServerEvent(app) != 0)
    {
        const reg32 event = kServerEvent(app);
        if (!callGame(app, cpu, frame, kSetEvent, {kServerThread.at, event, cpu.ebx, cpu.ecx}, {}, eax))
            return {};
        while (kServerEvent(app) != 0)
        {
            safepoint(app, cpu);
            if (!callGame(app, cpu, frame, kSleep, {0, event, cpu.ebx, cpu.ecx}, {}, eax))
                return {};
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
    const HeldEbp ebp(cpu, frame);
    reg32 eax;
    if (!callGame(app, cpu, frame, kCreateEvent, residue(cpu), {}, eax))
        return {};
    kServerEvent.set(app, eax);
    for (;;)
    {
        safepoint(app, cpu);
        if (kServer(app) == 0)
            break;
        if (!callGame(app, cpu, frame, kServer(app), residue(cpu))
            || !callGame(app, cpu, frame, kWaitEvent, {kServerEvent(app), cpu.edx, cpu.ebx, cpu.ecx}, {}, eax))
            return {};
    }
    if (!callGame(app, cpu, frame, kCloseHandle, {kServerEvent(app), cpu.edx, cpu.ebx, cpu.ecx}, {}, eax))
        return {};
    // The event's place is cleared from edx, which was 0 when the server was
    // found to be none and which the close keeps.
    kServerEvent.set(app, 0);
    return int32_t(eax);
}

/* 0x4e2754 (iSNDverifymode): mode `mode` (-1 any) against eacsnd's caps: its
 * format (8 before 2, 4, 1), its rate (8, 0x10, 4, 0x20, 2, 1, in the second
 * byte) and stereo (0x20000) as both have them.  0 with an error for no format
 * or no rate.  `second` goes to SNDcaps as its eax (which it ignores). */
Result verifyMode(App* app, CPU& cpu, reg32 esp, reg32 mode, reg32 second)
{
    const Preserve keep(cpu);
    const reg32 frame = esp - 4 * 4;
    reg32 caps;
    if (!callGame(app, cpu, frame, kCaps, {second, cpu.edx, cpu.ebx, cpu.ecx}, {}, caps))
        return {};
    const reg32 both = (mode == 0xffffffff ? caps : mode) & caps;
    const reg32 formats = both & 0xff, rates = both & 0xff00;
    const reg32 stereo = mode == 0xffffffff ? 0 : both & 0xff0000;
    if (formats == 0)
    {
        const HeldEbp ebp(cpu, 0xfc);
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

/* 0x4e225c (iSNDmessageinit, stdcall 24): the driver's message, on the game's
 * message thread, for window `window`: `command` 0 starts the driver -- eacsnd
 * checked for the mode, the rate taken from it, the mixer and eacsnd started,
 * the sound thread and the focus handler added -- another ends it.  The
 * result in 0x9ef60c, the message done.  0; `eax` is what the caller left,
 * which an error is told. */
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
            if (!setServer(app, cpu, frame - 4, 0))
                return {};
        }
        if (kFocusAdded(app) != 0)
        {
            if (!callGame(app, cpu, frame, kFocusHandler, {0, cpu.edx, cpu.ebx, cpu.ecx}))
                return {};
            kFocusAdded.set(app, 0);
        }
        if (!guest::soundFrom<isnd::kDirectStop, &eacsnd::stop>(app, cpu, frame, residue(cpu)))
            return {};
        kDriverResult.set(app, 0);
        return done();
    }
    if (window == 0)
    {
        const HeldEbp ebp(cpu, 0x2d9);
        if (!fail(app, cpu, frame - 4, kMessageError, 0x2d9, {0x549f64}, eax))
            return {};
    }
    const reg32 mode = kDriverModeAsked(app);
    const std::optional<reg32> caps =
        guest::soundFrom<isnd::kDirectCaps, &eacsnd::caps>(app, cpu, frame, {cpu.eax, cpu.edx, cpu.ebx, cpu.ecx}, window);
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
        // The restore registered to run at exit; the register the flag comes
        // from (ch) is kept by the registration.
        if (!callGame(app, cpu, frame, kAtExitRegister, {kRestoreFunction, cpu.edx, cpu.ebx, cpu.ecx}))
            return {};
        kAtExit.set(app, 1);
    }
    // The mixer's start, with the mode and what the mixer calls when a channel ends.
    if (!callGame(app, cpu, frame, kMixInit, {kDriverMode(app), kMixerEnded, mode, cpu.ecx}))
        return {};
    if (kSetFx(app) != 0 && !busEffect(app, cpu, frame, {kBusType(app), cpu.edx, mode, cpu.ecx}))
        return {};
    const std::optional<reg32> started = guest::soundFrom<isnd::kDirectStart, &eacsnd::start>(
        app, cpu, frame, {cpu.eax, cpu.edx, cpu.ebx, cpu.ecx}, mode, window);
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
        if (!callGame(app, cpu, frame, kFocusHandler, {kFocusFunction, cpu.edx, cpu.ebx, cpu.ecx}))
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
    const Preserve keep(cpu);
    const reg32 frame = esp - 3 * 4;
    kMessageDone.set(app, 0);
    reg32 eax;
    if (!callGame(app, cpu, frame, kSendMessage, {kMessageNumber, 1, 0, 0}, {}, eax))
        return {};
    if (eax == 0)
    {
        if (kThreadStarted(app) != 0)
        {
            kThreadStarted.set(app, 0);
            if (!setServer(app, cpu, frame - 4, 0))
                return {};
        }
        if (kFocusAdded(app) != 0)
        {
            if (!callGame(app, cpu, frame, kFocusHandler, {0, cpu.edx, cpu.ebx, cpu.ecx}))
                return {};
            kFocusAdded.set(app, 0);
        }
        kDriverResult.set(app, 0);
    }
    else
        while (kMessageDone(app) == 0)
        {
            safepoint(app, cpu);
            if (!callGame(app, cpu, frame, kSleep, {0, cpu.edx, cpu.ebx, cpu.ecx}))
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
    const Preserve keep(cpu);
    const reg32 frame = esp - 3 * 4;
    kDriverModeAsked.set(app, mode);
    kLeaveAll.set(app, kLeaveAllFunction);
    reg32 eax;
    if (!callGame(app, cpu, frame, kLoadDriver, {mode, cpu.edx, cpu.ebx, cpu.ecx}, {}, eax))
        return {};
    if (kSoundRunning(app) == 0)
    {
        const HeldEbp ebp(cpu, kInitError.file);
        if (!fail(app, cpu, frame, kInitError, 0x349, {0x549fc8}, kInitError.function))
            return {};
        return int32_t(0xfffffff0);
    }
    if (!callGame(app, cpu, frame, kWindowOfGame, residue(cpu), {}, eax))
        return {};
    if (eax == 0)
    {
        if (!fail(app, cpu, frame - 2 * 4, kInitError, 0x351, {0x54a004}, eax))
            return {};
        return int32_t(0xfffffff0);
    }
    if (!callGame(app, cpu, frame, kRegisterHandler, residue(cpu), {kMessageNumber, kMessageHandler}))
        return {};
    kMessageDone.set(app, 0);
    if (!callGame(app, cpu, frame, kSendMessage, {kMessageNumber, 0, 0, 0}))
        return {};
    // The message thread's answer waited for, as the code waits: a slice
    // given up a time.
    while (kMessageDone(app) == 0)
    {
        safepoint(app, cpu);
        if (!callGame(app, cpu, frame, kSleep, {0, 0, cpu.ebx, cpu.ecx}))
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
    const Preserve keep(cpu);
    const reg32 frame = esp - 3 * 4;
    if (kInited(app) != 0)
        return 0;
    if (kVoicesUsed(app) == 0)
        kVoicesUsed.set(app, 0x10);
    if (kSoundTick(app) == 0)
        kSoundTick.set(app, kDefaultTick);
    reg32 lock;
    if (!callGame(app, cpu, frame, kMakeLock, residue(cpu), {}, lock))
        return {};
    kLock.set(app, lock);
    cpu.edi = 0;
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

/* 0x4e26b4 (SNDrestore): the library ended: what registered a restore
 * restored (the game's, Pathfinder, the streamers, the effects), every sound
 * stopped, the effects' end told, the driver ended, the lock gone.  The
 * driver's result; -10 before SNDinit. */
Result endLibrary(App* app, CPU& cpu, reg32 esp)
{
    if (kInited(app) == 0)
        return int32_t(0xfffffff6);
    for (const int32_t i : kRestoreOrder)
        if (kRestores(app, i) != 0 && !callGame(app, cpu, esp, kRestores(app, i), residue(cpu)))
            return {};
    if (!callGame(app, cpu, esp, kStopAll, residue(cpu)))
        return {};
    if (kBankHook(app) != 0 && !callGame(app, cpu, esp, kBankHook(app), {0xffffffff, cpu.edx, cpu.ebx, cpu.ecx}))
        return {};
    // edx and ebx are pushed, to be popped again.
    const Result result = stopSoundDriver(app, cpu, esp - 2 * 4 - 4);
    if (!result)
        return {};
    kInited.set(app, 0);
    if (!callGame(app, cpu, esp - 2 * 4, kFreeLock, {kLock(app), reg32(*result), cpu.ebx, cpu.ecx}))
        return {};
    return result;
}

}  // namespace

/* The game's entries: adapters around the cores above. */

void iSNDunloaddll(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, unloadDriver(app, cpu, e.esp));
}

void SNDsetrate(App* app, CPU& cpu)
{
    const Entry e(cpu);
    setPriorityOffset(app, uint16_t(e.eax));
    e.back(cpu, 0);
}

void leaveAudioLocks(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, leaveAllLocks(app, cpu, e.esp, e.eax));
}

void iSNDmessageinit(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, driverMessage(app, cpu, e.esp, e.arg(app, 1), e.arg(app, 4), e.eax), 0x18);
}

void iSNDinit(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, startSoundDriver(app, cpu, e.esp, e.eax));
}

void iSNDrestore(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, stopSoundDriver(app, cpu, e.esp));
}

void iSNDverifymode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, verifyMode(app, cpu, e.esp, e.eax, e.edx));
}

void SNDinit(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, startLibrary(app, cpu, e.esp, e.eax, e.edx));
}

void SNDrestore(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, endLibrary(app, cpu, e.esp));
}

void soundThread(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, serveSounds(app, cpu, e.esp));
}

void setSoundServer(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, setServer(app, cpu, e.esp, e.eax));
}

}  // namespace port
}  // namespace nfs3hp
