#include "native_ea_files.h"
#include "native_ea_threads.h"
#include "native_frontend_simd.h"
#include "native_game.h"
#include "native_guest.h"
#include "native_watcom.h"
#include "native_vertices.h"
#include "os_threads.h"
#include "os_system.h"

#include <cstdint>
#include <cstring>
#include <optional>

/* The esp a guest call's scope began with put back at its end: what guest::EspKept did of it (the other
 * registers are the thunks' args now). */
namespace
{
using nfs3hp::guest::EspKept;
}  // namespace

/* EA's threads, events, timers and locks: win\threads.c of EA's platform
 * library over Win32 -- and the small neighbours the linker left with it (the
 * rotation matrices, a random fraction, DirectDraw's surface lock flag, the
 * machine's memory).  The game's code calls each by the name it has here.
 *
 * Threads: set up the first time any of them is asked for (0x9ef5b8 1), with
 * the main thread's id (0x9ef5b4), a real handle of it duplicated (0x9ef5b0)
 * and a lock made for the game (0x564480).  A thread made by createThread is
 * a record of five dwords the caller keeps: its handle, its stack size, its
 * priority (EA's, -3 to 3), 0, its id; it starts suspended at 0x4df730, which
 * calls the function handed over, its priority set, and is let go.  Where a
 * thread is asked for, 0 is the main thread and -1 the one asking.  The
 * game's main, sound, file and network threads are made so.
 *
 * Events are the port's (os::events), auto-reset or manual; a wait is over
 * handles, giving back the handle that was set, or 0.  Times are in ms,
 * or in the game's ticks: 1000 / [0x5643d8] ms each, 100 a second where it is 0.
 *
 * Locks are critical sections in records of 0x20 bytes: the CRITICAL_SECTION's
 * 0x18 bytes, a link, and 'FREE' (0x46524545) on those free.  Free ones are a
 * list at 0x564a64, grown by [0x564a60] records at a time (32 at first; as
 * many as the pages allocated hold after).
 *
 * Each is a typed function under a name of its own, with a thunk under the
 * game's name for the callers still on registers and the dispatch: it leaves
 * the registers and the x87's stack as the game's code does.  The threads,
 * events and locks are the port's (os_threads.h); other files' typed functions (a vector made a
 * unit long, Windows' pages got) as C++ (guest::direct, their headers); the
 * game's other functions -- and those of this file (the main thread asked, a
 * sleep, a lock made and freed, DirectDraw's lock) -- by address, with their
 * arguments in the registers and on the guest's stack as the code hands them
 * over; a core keeps the registers its code pushes and pops around them
 * (Kept), and leaves the rest as the calls leave them, as the code does.  A
 * frame whose address a callee is handed stays on the guest's stack at the
 * code's depth (GuestFrame).  Where the code is a thread's entry, the CPU it
 * is handed is kept to.  The x87's operations of the rotations and the random
 * fraction are fp's, in the game's order, a float kept where the game stores
 * one. */
namespace nfs3hp
{
namespace ea
{

namespace
{

using guest::at;
using guest::put;
using nfs::u32;
using nfs::s32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;

// The threads: set up, the main thread's id, its handle; the lock made for
// the game; the game's ticks a second (0: 100).
constexpr u32 kThreadsReady = 0x9ef5b8, kMainThreadId = 0x9ef5b4, kMainThread = 0x9ef5b0;
constexpr u32 kGameLock = 0x564480, kTicksPerSecond = 0x5643d8, kDefaultTicks = 100;
// A thread's record: its handle, stack size, priority, 0, id.
constexpr u32 kHandle = 0, kStackSize = 4, kPriority = 8, kUnused = 0xc, kThreadId = 0x10;
// The threads as the functions are handed them: the main thread, the one asking.
constexpr u32 kTheMainThread = 0, kThisThread = 0xffffffff;
// The error code lastErrorText says (never set), and its text.
constexpr u32 kErrorCode = 0x9ef5bc, kNotImplemented = 0x549900;
// The locks: free ones, how many to a grown pool; a record's link and tag.
constexpr u32 kFreeLocks = 0x564a64, kLocksToGrow = 0x564a60;
constexpr u32 kLockLink = 0x18, kLockTag = 0x1c, kFree = 0x46524545, kLockBytes = 0x20, kSectionBytes = 0x18;
// Where an Error is from (Error's file, function and line), and its texts.
constexpr u32 kWhereFile = 0x552190, kWhereFunction = 0x552194, kWhereLine = 0x552198;
constexpr u32 kThreadsFile = 0x549868;
constexpr u32 kCreateThreadName = 0x549878, kCreateThreadText = 0x549888, kCreateThreadLine = 0x9f;
constexpr u32 kRemoveThreadName = 0x5498c0, kRemoveThreadText = 0x5498d0, kRemoveThreadLine = 0xdc;
// DirectDraw's surfaces locked: the screen's, another's.
constexpr u32 kScreenLocked = 0x9f3d0c, kSurfaceLocked = 0x567180;
// The identity matrix, 1/65536 (a double).
constexpr u32 kIdentity = 0x564498, kOneIn65536 = 0x54b734;
constexpr u32 kOne = 0x3f800000;
// A matrix: 3 by 3 floats, by rows.
constexpr u32 kMatrixBytes = 0x24;
// What is called by address.
constexpr u32 kEaRandom = 0x4e0d88;
constexpr u32 kIsThreadAt = 0x4df930;
constexpr u32 kThreadStart = 0x4df730;

/* What the thunks below run a core under: every register but eax kept, as the game's code keeps
 * what it pushes (the registers a core leaves are nobody's). */
template <typename Function>
auto kept(ThreadState& ts, Function function)
{
    const EspKept registers(ts);
    return function();
}

/* Room on the guest's stack for what a callee is handed the address of: the
 * code's frame (`pushed` bytes of registers under esp on entry, then
 * `bytes`), esp under it while the core runs and back after (as
 * native_collide.cpp's). */
class GuestFrame
{
public:
    GuestFrame(ThreadState& ts, u32 pushed, u32 bytes) : ts_(ts), esp_(ts.sp), base_(ts.sp - pushed)
    {
        ts.sp = base_ - bytes;
    }
    GuestFrame(const GuestFrame&) = delete;
    GuestFrame& operator=(const GuestFrame&) = delete;
    ~GuestFrame() { ts_.sp = esp_; }
    u32 operator-(u32 offset) const { return base_ - offset; }

private:
    ThreadState& ts_;
    const u32 esp_;
    const u32 base_;
};

/* One of the game's functions at `target`, its register arguments set
 * already: what it gives back in eax.  nullopt when the game stopped. */
std::optional<u32> game(App* app, ThreadState& ts, u32 target)
{
    return guest::pointerCall(app, ts, target, {});
}

/* Error (0x401010) from win\threads.c's `function` at `line` with `text`:
 * where it is from set first, edx and ecx the file's and the function's
 * names as the code hands them over.  False when the game stopped. */
bool threadsError(App* app, ThreadState& ts, u32 function, u32 line, u32 text)
{
    put(app, kWhereFile, kThreadsFile);
    put(app, kWhereFunction, function);
    put(app, kWhereLine, line);
    guest::direct<0x401010, ea::error>(app, ts, text, guest::Words({}));
    return !ts.terminate;
}

/* `ticks` of the game's in ms: times 1000 (as the code works it out, by
 * shifts and adds, so to 32 bits) over the ticks a second (100 where they are
 * 0), signed. */
u32 ticksToMs(App* app, u32 ticks)
{
    u32 rate = at(app, kTicksPerSecond);
    if (rate == 0)
        rate = kDefaultTicks;
    const std::int64_t ms = s32(ticks * 1000);
    return u32(ms / s32(rate));
}

/* What a wait gives: the handle that was set, or 0; and what the code is handed back of it, which
 * its callers find in ebx: the index of the one set, 0x102 when the time was up, -1 when the wait
 * failed. */
struct Wait
{
    u32 set;
    u32 result;
};

/* A wait on `count` handles at `handles`, `ms` at most (os::kForever: for ever) (os::events): the
 * handle that was set, else 0.  edx the handles, as the code hands them over; ecx kept. */
std::optional<Wait> waitForHandles(App* app, ThreadState& ts, u32 count, u32 handles, u32 ms)
{
    const auto wait =
        os::events::waitAny(app, ts, handles != 0 ? &app->getMemory<u32>(handles) : nullptr, count, ms);
    if (!wait)
        return std::nullopt;
    switch (wait->ended)
    {
    case os::events::Ended::set:
        return Wait{at(app, handles + wait->index * 4), wait->index};
    case os::events::Ended::timeUp:
        return Wait{0, 0x102};
    case os::events::Ended::failed:
        break;
    }
    return Wait{0, 0xffffffff};
}

/* The function a thread made by createThread runs, called with eax it.
 * False when the game stopped. */
bool runThread(App* app, ThreadState& ts, u32 function)
{
    return guest::pointerCall(app, ts, function, {function}).has_value();
}

/* The threads set up: the main thread's id kept, a handle of it of its own
 * (a copy of the asking thread's), and the game's lock made (0x4eb2e0, by
 * address): the lock back, as eax.  ecx and edx kept. */
std::optional<u32> setUpThreads(App* app, ThreadState& ts)
{
    put(app, kMainThreadId, os::threads::currentId(app, ts));
    // The process's handles the code takes for the copy, as it takes them: the handles after them
    // are numbered as they always were.
    os::currentProcess(app);
    const os::Handle thread = os::threads::current(app);
    os::currentProcess(app);
    put(app, kMainThread, os::threads::duplicate(app, thread));
    put(app, kThreadsReady, 1);
    const auto lock = guest::direct<0x4eb2e0, ea::newLock>(app, ts);
    if (!lock)
        return std::nullopt;
    put(app, kGameLock, *lock);
    return *lock;
}

/* The threads set up if they are not yet, as each of their functions does
 * first, eax 0 handed over.  False when the game stopped. */
bool threadsReady(App* app, ThreadState& ts)
{
    if (at(app, kThreadsReady) != 0)
        return true;
    return setUpThreads(app, ts).has_value();
}

/* The handle of `thread`: the main thread's, this one's (a new one), a
 * record's. */
u32 threadHandle(App* app, u32 thread)
{
    if (thread == kTheMainThread)
        return at(app, kMainThread);
    if (thread == kThisThread)
        return os::threads::current(app);
    return at(app, thread + kHandle);
}

/* `thread` given EA's priority `priority`: the port runs every thread at
 * one priority (os::threads), so nothing is set; 1, a priority set, as the
 * code is told.  The handle is taken as the code takes it.  ecx kept. */
std::optional<u32> setPriority(App* app, ThreadState& ts, u32 thread, u32 priority)
{
    NFS2_USE(priority);
    if (!threadsReady(app, ts))
        return std::nullopt;
    threadHandle(app, thread);
    return 1;
}

/* EA's createthread: a thread running `function`, with a stack of
 * `stackBytes` and EA's priority `priority`, into the record at `record` --
 * made suspended (its id into the frame), its priority set, let go, and this
 * thread's time given up.  1 if it was made, else 0.  Made while a DirectDraw
 * surface is locked, an Error says to unlock it first, and it is made all the
 * same.  ecx left the handle. */
std::optional<u32> startThread(App* app, ThreadState& ts, u32 function, u32 stackBytes, u32 priority, u32 record)
{
    // esi, edi and ebp pushed; the thread's id below them.
    const GuestFrame frame(ts, 0xc, 4);
    const u32 id = frame - 4;
    const bool locked = guest::direct<0x4f9070, ea::surfaceIsLocked>(app, ts);
    if (locked)
    {
        if (!threadsError(app, ts, kCreateThreadName, kCreateThreadLine, kCreateThreadText))
            return std::nullopt;
    }
    if (!threadsReady(app, ts))
        return std::nullopt;
    const os::threads::Started started = os::threads::start(app, kThreadStart, function, stackBytes, true);
    put(app, id, started.id);
    const u32 handle = started.handle;
    if (handle != 0)
    {
        put(app, record + kUnused, 0);
        put(app, record + kHandle, handle);
        put(app, record + kStackSize, stackBytes);
        put(app, record + kPriority, priority);
        put(app, record + kThreadId, at(app, id));
        if (!setPriority(app, ts, record, priority))
            return std::nullopt;
        os::threads::resume(app, handle);
        if (!os::threads::sleep(app, ts, 0))
            return std::nullopt;
    }
    return handle != 0 ? 1 : 0;
}

}  // namespace

/* EA's removethread: `thread` ended and its handle closed: what CloseHandle
 * gives back.  None is the main thread, which an Error says cannot be removed
 * -- and then the code goes on with `stale`, the handle ebx held as it was
 * called (a bug of the game's, kept).  ebx, ecx and edx kept. */
std::optional<u32> endThread(App* app, ThreadState& ts, u32 thread, u32 stale)
{
    u32 handle = stale;
    if (thread == kTheMainThread)
    {
        // As deep as the code calls Error, ebx, ecx, edx and esi pushed: Error's own frame goes to its
        // callees.
        const GuestFrame pushed(ts, 0x10, 0);
        if (!threadsError(app, ts, kRemoveThreadName, kRemoveThreadLine, kRemoveThreadText))
            return std::nullopt;
    }
    else if (thread == kThisThread)
    {
        handle = os::threads::current(app);
    }
    else
    {
        handle = at(app, thread + kHandle);
    }
    if (!os::threads::stop(app, ts, handle))
        return std::nullopt;
    os::release(app, handle);
    return 1;  // a handle let go: 1, as the code is told
}

/* `ms` slept (os::threads), eax the ms as the code has them there: 0, what
 * the code is told of a sleep.  ecx and edx kept. */
std::optional<u32> sleepFor(App* app, ThreadState& ts, u32 ms)
{
    if (!os::threads::sleep(app, ts, ms))
        return std::nullopt;
    return 0;
}

/* Whether this is `thread` (the main thread; any, for this one), by its id.
 * ecx and edx kept. */
std::optional<bool> isThreadNow(App* app, ThreadState& ts, u32 thread)
{
    const u32 id = os::threads::currentId(app, ts);
    if (!threadsReady(app, ts))
        return std::nullopt;
    if (thread == kTheMainThread)
        return id == at(app, kMainThreadId);
    if (thread == kThisThread)
        return true;
    return id == at(app, thread + kThreadId);
}

namespace
{

/* The priority of `thread`, EA's: 0, normal, the one the port runs every
 * thread at (os::threads).  The handle is taken as the code takes it.  ecx
 * and edx kept. */
std::optional<u32> priorityOf(App* app, ThreadState& ts, u32 thread)
{
    if (!threadsReady(app, ts))
        return std::nullopt;
    threadHandle(app, thread);
    return 0;
}

}  // namespace

std::optional<u32> eventSet(App* app, ThreadState& ts, u32 event)
{
    NFS2_USE(ts);
    return os::events::set(app, event) ? 1 : 0;
}

namespace
{

std::optional<u32> eventReset(App* app, ThreadState& ts, u32 event)
{
    NFS2_USE(ts);
    return os::events::reset(app, event) ? 1 : 0;
}

std::optional<u32> eventPulsed(App* app, ThreadState& ts, u32 event)
{
    NFS2_USE(ts);
    os::events::pulse(app, event);
    return 1;
}

std::optional<u32> handleClosed(App* app, ThreadState& ts, u32 handle)
{
    NFS2_USE(ts);
    os::release(app, handle);
    return 1;  // a handle let go: 1, as the code is told
}

/* The system's error set: the port keeps no error's code, so nothing is; 0. */
std::optional<u32> lastErrorSet(App* app, ThreadState& ts, u32 code)
{
    NFS2_USE(app);
    NFS2_USE(ts);
    NFS2_USE(code);
    return 0;
}

std::optional<u32> lockEntered(App* app, ThreadState& ts, u32 lock)
{
    if (!os::locks::enter(app, ts, lock))
        return std::nullopt;
    return 0;
}

std::optional<u32> lockLeft(App* app, ThreadState& ts, u32 lock)
{
    if (!os::locks::leave(app, ts, lock))
        return std::nullopt;
    return 0;
}

/* An event, not set, unnamed: manual-reset or not.  ecx and edx kept. */
std::optional<u32> newEvent(App* app, ThreadState& ts, u32 manual)
{
    NFS2_USE(ts);
    return os::events::make(app, manual != 0, false);
}

/* pollEvent's and the event waits' frame: two registers pushed, the event's
 * handle below them, which the wait is handed as its handles. */
constexpr u32 kEventWaitPushed = 8, kEventWaitFrame = 4;

/* Whether `event` is set within `ms` (taken, if auto-reset).  edx as the wait
 * leaves it. */
std::optional<bool> eventSetWithin(App* app, ThreadState& ts, u32 event, u32 ms)
{
    const GuestFrame frame(ts, kEventWaitPushed, kEventWaitFrame);
    const u32 handles = frame - 4;
    put(app, handles, event);
    const auto wait = waitForHandles(app, ts, 1, handles, ms);
    if (!wait)
        return std::nullopt;
    return wait->set == at(app, handles);
}

/* Whether `event` is set, without waiting.  edx kept. */
std::optional<bool> eventPolled(App* app, ThreadState& ts, u32 event)
{
    return eventSetWithin(app, ts, event, 0);
}

}  // namespace

/* `event` waited on for ever: it, or 0 if the wait failed.  edx kept. */
std::optional<u32> eventWaited(App* app, ThreadState& ts, u32 event)
{
    const GuestFrame frame(ts, kEventWaitPushed, kEventWaitFrame);
    const u32 handles = frame - 4;
    put(app, handles, event);
    const auto wait = waitForHandles(app, ts, 1, handles, os::kForever);
    if (!wait)
        return std::nullopt;
    return wait->set;
}

namespace
{

/* `event` set and its handle let go: 1.  ecx and edx kept. */
std::optional<u32> eventSetAndClosed(App* app, ThreadState& ts, u32 event)
{
    NFS2_USE(ts);
    os::events::set(app, event);
    os::release(app, event);
    return 1;
}

/* EA's stub: the error's code (0x9ef5bc's low word, which nothing sets) --
 * with `buffer`, "[%5d] sorry, not implemented" printed into it first.
 * sprintf is handed the stack as the code has it: edx, which the code pushes,
 * just above the arguments (a word sprintf never reads). */
std::optional<u32> errorText(App* app, ThreadState& ts, u32 buffer, u32 edx)
{
    if (buffer != 0)
    {
        const GuestFrame frame(ts, 4, 0);
        put(app, frame - 0, edx);  // the edx the code pushes, above the arguments
        guest::direct<0x4df690, watcom::clib::sprintf>(app, ts, buffer, kNotImplemented,
                                                       guest::Words({at(app, kErrorCode) & 0xffff}));
        if (ts.terminate)
            return std::nullopt;
    }
    return at(app, kErrorCode) & 0xffff;
}

/* An angle's sine and cosine (0x4ea970) of the angle at `angle` (the game's
 * units) into the floats at `sine` and `cosine`: eax, edx and ebx the
 * addresses, as the code hands them over; ebx and edx kept.  False when the
 * game stopped. */
bool sineCosine(App* app, ThreadState& ts, u32 angle, u32 sine, u32 cosine)
{
    guest::direct<0x4ea970, frontend::simd::sineCosine>(app, ts, angle, sine, cosine);
    return !ts.terminate;
}

nfs::Float floatAt(App* app, u32 address)
{
    return nfs::Float(app->getMemory<float>(address));
}

void putFloat(App* app, u32 address, float value)
{
    app->getMemory<float>(address) = value;
}

/* fld; fchs; fstp: the float of `bits` negated through the x87. */
float negated(u32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return float(-nfs::Float(value));
}

/* A turn's sine and cosine, as bits. */
struct Turn
{
    u32 sine;
    u32 cosine;
};

/* The sine and cosine of the angle at `angle` (the rotation's argument), as
 * the rotations work them out: into their frame, ebx, ecx and edx pushed and
 * the sine and the cosine below them. */
std::optional<Turn> turnOf(App* app, ThreadState& ts, u32 angle)
{
    const GuestFrame frame(ts, 0xc, 8);
    const u32 sine = frame - 8, cosine = frame - 4;
    if (!sineCosine(app, ts, angle, sine, cosine))
        return std::nullopt;
    return Turn{at(app, sine), at(app, cosine)};
}

}  // namespace

/* The matrix at `matrix` (3 by 3 floats, by rows) a rotation about x by the
 * angle at `angle`: the cosine's bits back, which the code leaves in eax. */
std::optional<u32> rotateAboutX(App* app, ThreadState& ts, u32 matrix, u32 angle)
{
    const auto turn = turnOf(app, ts, angle);
    if (!turn)
        return std::nullopt;
    put(app, matrix, kOne);
    put(app, matrix + 4, 0);
    put(app, matrix + 8, 0);
    put(app, matrix + 0xc, 0);
    put(app, matrix + 0x10, turn->cosine);
    put(app, matrix + 0x18, 0);
    put(app, matrix + 0x14, turn->sine);
    putFloat(app, matrix + 0x1c, negated(turn->sine));
    put(app, matrix + 0x20, turn->cosine);
    return turn->cosine;
}

/* The matrix at `matrix` a rotation about y by the angle at `angle`: the
 * cosine's bits back. */
std::optional<u32> rotateAboutY(App* app, ThreadState& ts, u32 matrix, u32 angle)
{
    const auto turn = turnOf(app, ts, angle);
    if (!turn)
        return std::nullopt;
    put(app, matrix + 4, 0);
    put(app, matrix, turn->cosine);
    put(app, matrix + 0xc, 0);
    put(app, matrix + 0x10, kOne);
    put(app, matrix + 0x14, 0);
    putFloat(app, matrix + 8, negated(turn->sine));
    put(app, matrix + 0x1c, 0);
    put(app, matrix + 0x18, turn->sine);
    put(app, matrix + 0x20, turn->cosine);
    return turn->cosine;
}

namespace
{

/* The matrix at `matrix` a rotation about z by the angle at `angle`: the
 * cosine's bits back. */
std::optional<u32> rotateAboutZ(App* app, ThreadState& ts, u32 matrix, u32 angle)
{
    const auto turn = turnOf(app, ts, angle);
    if (!turn)
        return std::nullopt;
    put(app, matrix, turn->cosine);
    put(app, matrix + 8, 0);
    put(app, matrix + 4, turn->sine);
    putFloat(app, matrix + 0xc, negated(turn->sine));
    put(app, matrix + 0x14, 0);
    put(app, matrix + 0x18, 0);
    put(app, matrix + 0x1c, 0);
    put(app, matrix + 0x20, kOne);
    put(app, matrix + 0x10, turn->cosine);
    return turn->cosine;
}

/* The matrix at `matrix` a rotation by the angle at `angle` about the axis at
 * `axis` (made a unit long first, 0x4e01c0): with s and c its sine and
 * cosine, t 1 - c, x y z the axis, t x x + c, t x y + s z, t x z - s y;
 * t x y - s z, t y y + c, t y z + s x; t x z + s y, t y z - s x, t z z + c --
 * t x kept as the float the code stores and reloads.  An angle of 0 (either
 * sign) gives the identity (0x564498).  eax as the calls leave it.  False
 * when the game stopped. */
bool rotateAboutAxis(App* app, ThreadState& ts, u32 axis, u32 angle, u32 matrix)
{
    if ((at(app, angle) & 0x7fffffff) == 0)
    {
        for (u32 i = 0; i < kMatrixBytes; i += 4)
            put(app, matrix + i, at(app, kIdentity + i));
        return true;
    }
    // ecx, esi and edi pushed; the axis made a unit long at the bottom, the
    // sine 0xc above it, the cosine at the top.
    const GuestFrame frame(ts, 0xc, 0x30);
    const u32 unit = frame - 0x30, sineAt = frame - 0x24, cosineAt = frame - 4;
    if (!sineCosine(app, ts, angle, sineAt, cosineAt))
        return false;
    guest::direct<0x4e01c0, vertices::makeUnitVector>(app, ts, axis, unit);
    if (ts.terminate)
        return false;
    const nfs::Float s = floatAt(app, sineAt), c = floatAt(app, cosineAt);
    const nfs::Float x = floatAt(app, unit), y = floatAt(app, unit + 4), z = floatAt(app, unit + 8);
    const nfs::Float sx = fp::mul(s, x);
    const nfs::Float sy = fp::mul(s, y);
    const nfs::Float sz = fp::mul(s, z);
    const nfs::Float t = fp::sub(1.0, c);
    const nfs::Float tx = fp::mul(t, x);
    const nfs::Float ty = fp::mul(t, y);
    const float txKept = float(tx);
    const nfs::Float txx = fp::mul(tx, x);
    const nfs::Float tz = fp::mul(t, z);
    putFloat(app, matrix, float(fp::add(txx, c)));
    putFloat(app, matrix + 4, float(fp::add(fp::mul(nfs::Float(txKept), y), sz)));
    putFloat(app, matrix + 8, float(fp::sub(fp::mul(nfs::Float(txKept), z), sy)));
    putFloat(app, matrix + 0xc, float(fp::sub(fp::mul(ty, x), sz)));
    putFloat(app, matrix + 0x10, float(fp::add(fp::mul(ty, y), c)));
    putFloat(app, matrix + 0x14, float(fp::add(fp::mul(ty, z), sx)));
    putFloat(app, matrix + 0x18, float(fp::add(sy, fp::mul(tz, x))));
    putFloat(app, matrix + 0x1c, float(fp::sub(fp::mul(tz, y), sx)));
    putFloat(app, matrix + 0x20, float(fp::add(fp::mul(tz, z), c)));
    return true;
}

}  // namespace

/* A random fraction, 0 to 65535/65536: eaRandom's (0x4e0d88) low word, as a
 * 64-bit integer, times the double 1/65536. */
std::optional<nfs::Float> randomFractionOf(App* app, ThreadState& ts)
{
    const auto drawn = game(app, ts, kEaRandom);
    if (!drawn)
        return std::nullopt;
    return fp::mul(nfs::Float(*drawn & 0xffff), nfs::Float(app->getMemory<double>(kOneIn65536)));
}

namespace
{

/* The free locks grown: [0x564a60] records' bytes of pages (0x501110, handed
 * the size's address in the frame, which it may round up to whole pages),
 * each record tagged free and linked to the one before -- the first to what
 * was free -- the last the list's head, and [0x564a60] as many as the pages
 * held.  None got, the list is left empty.  The pages back. */
std::optional<u32> growFreeLocks(App* app, ThreadState& ts)
{
    // ebx, ecx and esi pushed; the size below them.
    const GuestFrame frame(ts, 0xc, 4);
    const u32 size = frame - 4;
    u32 link = at(app, kFreeLocks);
    put(app, size, at(app, kLocksToGrow) << 5);
    const auto pages = guest::direct<0x501110, ea::allocatePages>(app, ts, size);
    if (!pages)
        return std::nullopt;
    u32 count = at(app, kLocksToGrow);
    u32 head = *pages;
    if (*pages != 0)
    {
        count = at(app, size) >> 5;
        u32 record = *pages;
        for (s32 i = 0; i < s32(count); ++i)
        {
            put(app, record + kLockTag, kFree);
            put(app, record + kLockLink, link);
            link = record;
            record += kLockBytes;
        }
        head = link;
    }
    put(app, kLocksToGrow, count);
    put(app, kFreeLocks, head);
    return *pages;
}

}  // namespace

/* A lock made: a free record taken (the list grown if empty), cleared
 * (0x4e070c, eax and edx) and its critical section initialized.  0 if there
 * was none.  ecx and edx kept. */
std::optional<u32> newLock(App* app, ThreadState& ts)
{
    if (!threadsReady(app, ts))
        return std::nullopt;
    if (at(app, kFreeLocks) == 0)
    {
        // As deep as the code calls growLocks, ebx pushed and the return address's slot: its frame's
        // address goes to 0x501110.
        const GuestFrame called(ts, 8, 0);
        if (!growFreeLocks(app, ts))
            return std::nullopt;
    }
    const u32 lock = at(app, kFreeLocks);
    if (lock == 0)
        return 0;
    put(app, kFreeLocks, at(app, lock + kLockLink));
    guest::direct<0x4e070c, ea::clearBytes>(app, ts, lock, kSectionBytes);
    if (ts.terminate)
        return std::nullopt;
    os::locks::make(app, lock);
    return lock;
}

/* `lock` (none: nothing) deleted and its record put back on the free list,
 * tagged free: the list's head before back (0 for none).  ecx and edx kept. */
std::optional<u32> lockFreed(App* app, ThreadState& ts, u32 lock)
{
    if (lock == 0)
        return 0;
    NFS2_USE(ts);
    os::locks::remove(app, lock);
    const u32 head = at(app, kFreeLocks);
    put(app, lock + kLockTag, kFree);
    put(app, kFreeLocks, lock);
    put(app, lock + kLockLink, head);
    return head;
}

/* Whether a DirectDraw surface is locked: the screen's, or another's. */
bool surfaceIsLocked(App* app)
{
    return at(app, kScreenLocked) != 0 || at(app, kSurfaceLocked) != 0;
}

/* The machine's physical memory in bytes, as the port tells it.  ecx and edx
 * kept. */
std::optional<u32> physicalBytes(App* app, ThreadState& ts)
{
    NFS2_USE(app);
    NFS2_USE(ts);
    return os::memory::kPhysicalBytes;
}

namespace
{

/* The thunk of a core of one register's argument: eax what it gives back,
 * ret. */
void oneArgument(App* app, ThreadState& ts, nfs::Args& args, std::optional<u32> (*core)(App*, ThreadState&, u32), u32 argument)
{
    const auto result = kept(ts, [&] { return core(app, ts, argument); });
    if (!result)
        return;
    args.eax = *result;
    ts.sp += 4;
}

}  // namespace

nfs::u32 waitForObjects(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 threadStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 threadsInit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 createThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 removeThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sleepMs(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 sleepTicks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 isThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 threadPriority(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 setThreadPriority(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 exitThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 createEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 postEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 signalEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 pollEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 waitEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 waitEventTicks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 waitForAny(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 closeEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 createManualEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 setEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 resetEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 pulseEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 waitManualEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 setAndCloseEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lastErrorText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 setLastError(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 lastError(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 doNothing(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 xRotation(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 yRotation(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 zRotation(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 axisRotation(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 randomFraction(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 growLocks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 makeLock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 enterLock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 leaveLock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 freeLock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 surfaceLocked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 physicalMemory(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);

/* 0x4df6f0: waitForHandles for eax handles at edx, ebx ms at most (-1 for
 * ever): eax the handle that was set, or 0; ebx what the wait gave back. */
nfs::u32 waitForObjects(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto wait = kept(ts, [&] { return waitForHandles(app, ts, args.eax, args.edx, args.ebx); });
    if (!wait)
        return args.eax;
    args.eax = wait->set;
    args.ebx = wait->result;
    ts.sp += 4;
    return args.eax;
}

/* 0x4df730: where a thread createThread made starts (a ThreadProc): the
 * function it was handed as its parameter run (runThread); 0 back, ret 4. */
nfs::u32 threadStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (!runThread(app, ts, at(app, ts.sp + 4)))
        return args.eax;
    args.eax = 0;
    ts.sp += 4 + 4;
    return args.eax;
}

/* 0x4df740: setUpThreads, once: eax left the lock (as it was, set up
 * already). */
std::optional<u32> threadsSetUp(App* app, ThreadState& ts, u32 eax)
{
    if (at(app, kThreadsReady) == 0)
        return kept(ts, [&] { return setUpThreads(app, ts); });
    return eax;
}

nfs::u32 threadsInit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto lock = threadsSetUp(app, ts, args.eax);
    if (!lock)
        return args.eax;
    args.eax = *lock;
    ts.sp += 4;
    return args.eax;
}

/* 0x4df7a0: startThread of function eax, a stack of edx bytes, priority ebx,
 * into the record at [esp + 4]: eax 1 if it was made; ebx left the record;
 * ret 4. */
nfs::u32 createThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 record = at(app, ts.sp + 4);
    const auto made = kept(ts, [&] { return startThread(app, ts, args.eax, args.edx, args.ebx, record); });
    if (!made)
        return args.eax;
    args.eax = *made;
    args.ebx = record;
    ts.sp += 4 + 4;
    return args.eax;
}

/* 0x4df870: endThread of record eax (-1: the one asking), ebx the handle the
 * main thread's case goes on with.  No caller in the game. */
nfs::u32 removeThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto result = kept(ts, [&] { return endThread(app, ts, args.eax, args.ebx); });
    if (!result)
        return args.eax;
    args.eax = *result;
    ts.sp += 4;
    return args.eax;
}

/* 0x4df8e0: eax ms slept. */
nfs::u32 sleepMs(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, sleepFor, args.eax);
    return args.eax;
}

/* 0x4df8f0: eax of the game's ticks slept.  No caller in the game. */
nfs::u32 sleepTicks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, sleepFor, ticksToMs(app, args.eax));
    return args.eax;
}

/* 0x4df930: 1 if this is the thread of record eax (0: the main thread; -1:
 * any, so always), else 0. */
nfs::u32 isThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto is = kept(ts, [&] { return isThreadNow(app, ts, args.eax); });
    if (!is)
        return args.eax;
    args.eax = *is ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x4df980: the priority of the thread of record eax (0: the main thread;
 * -1: this one), EA's.  No caller in the game. */
nfs::u32 threadPriority(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, priorityOf, args.eax);
    return args.eax;
}

/* 0x4dfa30: setPriority of the thread of record eax (0: the main thread; -1:
 * this one) to EA's priority edx: eax what SetThreadPriority gives back. */
nfs::u32 setThreadPriority(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto result = kept(ts, [&] { return setPriority(app, ts, args.eax, args.edx); });
    if (!result)
        return args.eax;
    args.eax = *result;
    ts.sp += 4;
    return args.eax;
}

/* 0x4dfaf0: this thread ended (os::threads::exit).  No caller in the game. */
nfs::u32 exitThread(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    os::threads::exit(ts);
    return args.eax;
}

/* 0x4dfb00's core: newEvent, auto-reset. */
std::optional<u32> newAutoEvent(App* app, ThreadState& ts)
{
    return newEvent(app, ts, 0);
}

/* 0x4dfc10's core: newEvent, manual-reset. */
std::optional<u32> newManualEvent(App* app, ThreadState& ts)
{
    return newEvent(app, ts, 1);
}

/* 0x4dfb20's core: eventSet of `event`, handed in edx. */
std::optional<u32> eventPosted(App* app, ThreadState& ts, u32 event)
{
    return eventSet(app, ts, event);
}

/* 0x4dfb00: an event, set by one and reset as a wait takes it (auto-reset). */
nfs::u32 createEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, newEvent, 0);
    return args.eax;
}

/* 0x4dfb20: event edx set; eax (what the callers would post with it) not
 * kept.  It runs on into signalEvent. */
nfs::u32 postEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, eventSet, args.edx);
    return args.eax;
}

/* 0x4dfb30: event eax set -- a second entry of postEvent's, past its first
 * instruction (and its nops), which the game calls too. */
nfs::u32 signalEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, eventSet, args.eax);
    return args.eax;
}

/* 0x4dfb40: 1 if event eax is set (taken, if auto-reset), else 0, without
 * waiting. */
nfs::u32 pollEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto set = kept(ts, [&] { return eventPolled(app, ts, args.eax); });
    if (!set)
        return args.eax;
    args.eax = *set ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x4dfb70: event eax waited on for ever: it, or 0 if the wait failed. */
nfs::u32 waitEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, eventWaited, args.eax);
    return args.eax;
}

/* 0x4dfb90: 1 if event eax is set within edx of the game's ticks, else 0.
 * edx left as the wait leaves it.  No caller in the game. */
nfs::u32 waitEventTicks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto set = kept(ts, [&] { return eventSetWithin(app, ts, args.eax, ticksToMs(app, args.edx)); });
    if (!set)
        return args.eax;
    args.eax = *set ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x4dfbf0: eax handles at edx waited on for ever: the one set, or 0. */
nfs::u32 waitForAny(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto wait = kept(ts, [&] { return waitForHandles(app, ts, args.eax, args.edx, os::kForever); });
    if (!wait)
        return args.eax;
    args.eax = wait->set;
    ts.sp += 4;
    return args.eax;
}

/* 0x4dfc00: handle eax closed -- an event's, a thread's. */
nfs::u32 closeEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, handleClosed, args.eax);
    return args.eax;
}

/* 0x4dfc10: an event that stays set until it is reset (manual-reset). */
nfs::u32 createManualEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, newEvent, 1);
    return args.eax;
}

/* 0x4dfc30: event eax set -- the same code as signalEvent's.  No caller in
 * the game. */
nfs::u32 setEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, eventSet, args.eax);
    return args.eax;
}

/* 0x4dfc40: event eax reset.  No caller in the game. */
nfs::u32 resetEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, eventReset, args.eax);
    return args.eax;
}

/* 0x4dfc50: event eax pulsed: set, those waiting let go, reset. */
nfs::u32 pulseEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, eventPulsed, args.eax);
    return args.eax;
}

/* 0x4dfc60: event eax waited on for ever -- waitEvent's code again, for the
 * manual events. */
nfs::u32 waitManualEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, eventWaited, args.eax);
    return args.eax;
}

/* 0x4dfc80: event eax set, and its handle closed. */
nfs::u32 setAndCloseEvent(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, eventSetAndClosed, args.eax);
    return args.eax;
}

/* 0x4dfca0: errorText into the buffer at eax (none: only the code).  No
 * caller in the game. */
nfs::u32 lastErrorText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 edx = args.edx;
    const auto result = kept(ts, [&] { return errorText(app, ts, args.eax, edx); });
    if (!result)
        return args.eax;
    args.eax = *result;
    ts.sp += 4;
    return args.eax;
}

/* 0x4dfce0: the system's last error set to eax.  No caller in the game. */
nfs::u32 setLastError(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, lastErrorSet, args.eax);
    return args.eax;
}

/* 0x4dfcf0: the system's last error: the port keeps none, 0. */
nfs::u32 lastError(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    args.eax = 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x4dfd00: nothing (a bare ret).  No caller in the game. */
nfs::u32 doNothing(nfs::App*, nfs::ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    return args.eax;
}

/* 0x4dfd80: rotateAboutX of the matrix at eax by the angle at [esp + 4]: eax
 * left the cosine's bits; ret 4. */
nfs::u32 xRotation(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto cosine = kept(ts, [&] { return rotateAboutX(app, ts, args.eax, ts.sp + 4); });
    if (!cosine)
        return args.eax;
    args.eax = *cosine;
    ts.sp += 4 + 4;
    return args.eax;
}

/* 0x4dfde0: rotateAboutY of the matrix at eax by the angle at [esp + 4]. */
nfs::u32 yRotation(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto cosine = kept(ts, [&] { return rotateAboutY(app, ts, args.eax, ts.sp + 4); });
    if (!cosine)
        return args.eax;
    args.eax = *cosine;
    ts.sp += 4 + 4;
    return args.eax;
}

/* 0x4dfe40: rotateAboutZ of the matrix at eax by the angle at [esp + 4]. */
nfs::u32 zRotation(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto cosine = kept(ts, [&] { return rotateAboutZ(app, ts, args.eax, ts.sp + 4); });
    if (!cosine)
        return args.eax;
    args.eax = *cosine;
    ts.sp += 4 + 4;
    return args.eax;
}

/* 0x4dfea0: rotateAboutAxis of the matrix at [esp + 8] by the angle at
 * [esp + 4] about the axis at eax; ret 8.  No caller in the game. */
nfs::u32 axisRotation(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 axis = args.eax;
    if (!rotateAboutAxis(app, ts, axis, ts.sp + 4, at(app, ts.sp + 8)))
        return args.eax;
    args.eax = axis;  // as makeUnitVector leaves it
    ts.sp += 4 + 8;
    return args.eax;
}

/* 0x4eb230: randomFractionOf onto the x87's stack; eax and edx kept. */
nfs::u32 randomFraction(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 eax = args.eax;
    const auto fraction = kept(ts, [&] { return randomFractionOf(app, ts); });
    if (!fraction)
        return args.eax;
    args.eax = eax;  // eax and edx kept
    args.floatSet = true;
    args.floatResult = *fraction;
    ts.sp += 4;
    return args.eax;
}

/* 0x4eb260: growFreeLocks: eax left the pages. */
nfs::u32 growLocks(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto pages = kept(ts, [&] { return growFreeLocks(app, ts); });
    if (!pages)
        return args.eax;
    args.eax = *pages;
    ts.sp += 4;
    return args.eax;
}

/* 0x4eb2e0: newLock: the lock, or 0. */
nfs::u32 makeLock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto lock = kept(ts, [&] { return newLock(app, ts); });
    if (!lock)
        return args.eax;
    args.eax = *lock;
    ts.sp += 4;
    return args.eax;
}

/* 0x4eb340: lock eax taken (EnterCriticalSection).  No caller in the game. */
nfs::u32 enterLock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, lockEntered, args.eax);
    return args.eax;
}

/* 0x4eb350: lock eax given back (LeaveCriticalSection).  No caller in the
 * game. */
nfs::u32 leaveLock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, lockLeft, args.eax);
    return args.eax;
}

/* 0x4eb360: lockFreed of lock eax: eax left the list's head before. */
nfs::u32 freeLock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    oneArgument(app, ts, args, lockFreed, args.eax);
    return args.eax;
}

/* 0x4f9070: 1 if a DirectDraw surface is locked (the screen's, 0x9f3d0c, or
 * another's, 0x567180), else 0 -- DirectDraw's, not the threads': what
 * createThread asks. */
nfs::u32 surfaceLocked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = surfaceIsLocked(app) ? 1 : 0;
    ts.sp += 4;
    return args.eax;
}

/* 0x5221f0: physicalBytes -- the file system's, not the threads'. */
nfs::u32 physicalMemory(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const auto bytes = kept(ts, [&] { return physicalBytes(app, ts); });
    if (!bytes)
        return args.eax;
    args.eax = *bytes;
    ts.sp += 4;
    return args.eax;
}

/* 0x4dfc00's and 0x4dfcf0's typed cores (native_ea_threads.h): as their thunks run them. */
std::optional<u32> eventClosed(App* app, ThreadState& ts, u32 event)
{
    return handleClosed(app, ts, event);
}

std::optional<u32> lastErrorOf(App* app, ThreadState& ts)
{
    NFS2_USE(app);
    NFS2_USE(ts);
    return 0;
}

std::optional<u32> threadStarted(App* app, ThreadState& ts, u32 function, u32 stackBytes, u32 priority, u32 record)
{
    return startThread(app, ts, function, stackBytes, priority, record);
}

std::optional<u32> eventPulse(App* app, ThreadState& ts, u32 event)
{
    return eventPulsed(app, ts, event);
}

std::optional<u32> eventSetClosed(App* app, ThreadState& ts, u32 event)
{
    return eventSetAndClosed(app, ts, event);
}

}  // namespace ea
}  // namespace nfs3hp
