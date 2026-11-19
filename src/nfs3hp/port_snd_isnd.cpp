#include "isnd_calls.h"

#include <lib/app.h>
#include <game_types.h>

#include <optional>

/* eacsnd's twelve exports native_sound.cpp leaves: the version, the game's
 * functions, recording, and the 3D buffers -- the functions at the bottom,
 * which the game reaches as the exports of those names -- with eacsnd's own data in guest memory as the
 * one copy.  Rewritten (port.h says how): each is here as C++ with arguments
 * and a result (nfs3hp::port), as the typed export native code calls through
 * the game's pointer to it (port::eacsnd, isnd_calls.h), and as the entry the
 * game calls through the stdcall pointer it keeps (nfs3hp, the names eacsnd
 * exports).
 *
 * Each does what eacsnd's code does on the port: the same memory written in the
 * same order, the game's functions called with the same arguments at the same
 * stack addresses, the same result, the stack and ebx, esi, edi and ebp left as
 * a stdcall function leaves them.  The scratch registers and flags eacsnd
 * happened to leave are nobody's business: the game calls these through stdcall
 * pointers.
 *
 * Three of them call the game out (the 3D position, playing and stopping a
 * buffer), and these are eacsnd's own code to the register: esp, esi and the
 * saved registers are taken as the call out leaves them, not as a stdcall
 * callee would -- so their cores run on the CPU (esp and esi
 * live) and leave esp and the registers as eacsnd does, the entry setting only
 * eax.  The rest take their arguments and give a result.
 *
 * eacsnd's other way, with DirectSound made (isnd::kDirectSoundMade) and 3D
 * buffers asked for (isnd::kIs3d), is not here: only eacsnd's start sets those,
 * and the port's start is native_sound.cpp's, which sets neither.  Should
 * either be set all the same, the core steps aside before it has done
 * anything (no answer), and the export fails (notNative).
 *
 * Their calls of the game's functions go through eacsnd's pointers to them,
 * guest::soundPushed: to the natives native_game.cpp has where the game's code
 * was, as C++, when the pointers lead there. */

namespace nfs3hp
{
namespace port
{

namespace
{

using Answer = std::optional<Result>;  // none: eacsnd's way with DirectSound, which the port has not

#pragma pack(push, 1)
/* One of the 16 buffers playing (isnd::kBuffers): the DirectSound objects
 * its sound plays on, its position's two words, rate, looping, volume. */
struct Buffer
{
    u32 active;             // IDirectSoundBuffer*: what plays, the slot's or a duplicate of it
    u32 main;               // IDirectSoundBuffer*: the slot's
    u32 buffer3d;           // IDirectSound3DBuffer*
    uint16_t positionA;       // pos3d's second argument
    uint16_t positionB;       // its third
    uint16_t rate;
    uint8_t looping;
    uint8_t volume;
    uint8_t slot;
    uint8_t other15[3];
};

/* One of the 64 sounds made (isnd::kSlots); slot 0 is never handed out. */
struct Slot
{
    u32 id;                 // 0: free
    u32 buffer;             // IDirectSoundBuffer*
    uint8_t other8[0x14];
};
#pragma pack(pop)
static_assert(sizeof(Buffer) == isnd::kBufferBytes && sizeof(Slot) == isnd::kSlotBytes, "eacsnd's records");

constexpr Records<Buffer> kBuffers{isnd::kBuffers};
constexpr Records<Slot> kSlots{isnd::kSlots};
constexpr Var<u32> kCapsKept{isnd::kCapsKept};
constexpr Table<u32> kFunctions{isnd::kFunctionMix};   // the game's eight functions, in the order given
constexpr Var<uint8_t> kDirectSoundMade{isnd::kDirectSoundMade}, kIs3d{isnd::kIs3d};
constexpr Var<double> kPositionScale{isnd::kPositionScale};

/* A word pushed: esp moved down and the word put there. */
void push(App* app, ThreadState& ts, u32 word)
{
    ts.sp -= 4;
    app->getMemory<u32>(ts.sp) = word;
}

/* A COM object's Release, IUnknown's third method: stdcall (this). */
bool release(App* app, ThreadState& ts, u32 object)
{
    // The call through the object's method table: `object` the word pushed, the return address's slot under it.
    nfs::Args registers;
    return guest::pointerCall(app, ts, app->getMemory<u32>(app->getMemory<u32>(object) + 8), registers,
                              {object})
        .has_value();
}

}  // namespace

/* iSNDdllversion's: 16.3, and the card's formats to be asked again. */
u32 eacsndVersion(App* app)
{
    kCapsKept.set(app, isnd::kFailure);
    return isnd::kVersion;
}

/* iSNDdirectsetfunctions': the game's eight functions kept, in their order. */
void eacsndKeepFunctions(App* app, const u32 (&functions)[8])
{
    for (int32_t i = 0; i < 8; ++i)
        kFunctions.set(app, i, functions[i]);
}

/* iSNDdirectvol's: buffer `buffer`'s volume's byte kept. */
Answer eacsndSetVolume(App* app, u32 buffer, u32 volume)
{
    if (kDirectSoundMade(app) != 0)
        return std::nullopt;
    kBuffers(app, int32_t(buffer)).volume = uint8_t(volume);
    return Result(0);
}

/* iSNDdirectrate's: buffer `buffer`'s rate's word kept. */
Answer eacsndSetRate(App* app, u32 buffer, u32 rate)
{
    if (kDirectSoundMade(app) != 0)
        return std::nullopt;
    kBuffers(app, int32_t(buffer)).rate = uint16_t(rate);
    return Result(0);
}

/* iSNDdirectpos3d's (buffer, a, b on the stack): the game's function asked
 * where (a, b) is, into three floats on the stack, which are scaled for
 * DirectSound in the x87's arithmetic (double, a float stored) -- for nothing,
 * with no DirectSound, but as eacsnd does -- and the two words kept.  The
 * stack as the call out leaves it is where the buffer and the words are read
 * from, and where the stdcall's return lands esp. */
Answer eacsndPosition(App* app, ThreadState& ts)
{
    if (kDirectSoundMade(app) != 0)
        return std::nullopt;
    ts.sp -= 0x14;
    const u32 position = ts.sp;
    push(app, ts, position);
    push(app, ts, app->getMemory<u32>(ts.sp + 0x24));  // b
    push(app, ts, app->getMemory<u32>(ts.sp + 0x24));  // a
    if (!guest::soundPushed<isnd::kFunctionBufferPosition, &acquireSoundBufferPositionOf>(app, ts))
        return Result();

    const u32 esp = ts.sp;
    const double scale = kPositionScale(app);
    const u32 index = app->getMemory<u32>(esp + 0x18);
    for (u32 at = esp; at < esp + 12; at += 4)
        store<float>(app, at, float(double(load<float>(app, at)) * scale));
    Buffer& record = kBuffers(app, int32_t(index));
    record.positionA = uint16_t(app->getMemory<u32>(esp + 0x1c));
    record.positionB = uint16_t(app->getMemory<u32>(esp + 0x20));
    ts.sp = esp + 0x14 + 4 + 0xc;
    return Result(0);
}

/* iSNDdirectcreate3dbuf's: no slot -- 0 -- without both 3D buffers and
 * DirectSound. */
Answer eacsndCreate3d(App* app)
{
    if (kIs3d(app) != 0 && kDirectSoundMade(app) != 0)
        return std::nullopt;
    return Result(0);
}

/* iSNDdirectremovebuf's: -1 without 3D buffers; with them and no DirectSound,
 * the slot `slot` freed.  (On that way eacsnd pushes ebx, which nothing reads:
 * the word is not written.) */
Answer eacsndRemove3d(App* app, u32 slot)
{
    if (kIs3d(app) == 0)
        return Result(int32_t(isnd::kFailure));
    if (kDirectSoundMade(app) != 0)
        return std::nullopt;
    kSlots(app, int32_t(slot)).id = 0;
    return Result(0);
}

/* iSNDdirectplay3d's (seven words), without 3D buffers: eacsnd jumps straight
 * to its way out of a failure, which clears the buffer record esi points at --
 * but esi is only set on the way that plays, so the record it clears is
 * wherever the caller's esi points: `esi`.  Its two objects there, the one
 * that plays unless it is the slot's own, and the 3D one, are released first,
 * and -1 returned.  ebx, esi, edi and ebp, pushed on entry and popped on the
 * way out, are the frame's (their words not written: the registers are as they
 * were). */
Answer eacsndPlay3d(App* app, ThreadState& ts, u32 esi)
{
    if (kIs3d(app) != 0)
        return std::nullopt;
    const u32 entry = ts.sp;
    ts.sp = entry - 0x10 - 0x64;

    Buffer* buffer = &record<Buffer>(app, esi);
    if (const u32 active = buffer->active; active != 0 && active != buffer->main)
        if (!release(app, ts, active))
            return Result();
    buffer = &record<Buffer>(app, esi);
    if (const u32 object = buffer->buffer3d; object != 0)
        if (!release(app, ts, object))
            return Result();
    buffer = &record<Buffer>(app, esi);
    buffer->main = 0;
    buffer->active = 0;
    buffer->buffer3d = 0;

    const u32 frame = ts.sp + 0x64;
    ts.sp = frame + 0x10 + 4 + 0x1c;
    return Result(int32_t(isnd::kFailure));
}

/* iSNDdirectstopbuf's (buffer on the stack), with no DirectSound: the record's
 * three objects forgotten, and the game told through its own function that the
 * buffer stopped.  ebx and edi, pushed on entry and popped on the way out, are
 * the frame's (their words not written). */
Answer eacsndStop(App* app, ThreadState& ts)
{
    if (kDirectSoundMade(app) != 0)
        return std::nullopt;
    const u32 entry = ts.sp;
    ts.sp = entry - 8;
    const u32 index = app->getMemory<u32>(entry + 4);
    Buffer& record = kBuffers(app, int32_t(index));
    record.main = 0;
    record.active = 0;
    push(app, ts, index);
    record.buffer3d = 0;
    if (!guest::soundPushed<isnd::kFunctionStopBuffer, &stopSoundBufferOf>(app, ts))
        return Result();

    ts.sp += 8 + 4 + 4;
    return Result(0);
}

/* ---- The exports, typed (isnd_calls.h) ---- */

namespace eacsnd
{

namespace
{

/* eacsnd's ways with DirectSound made or 3D buffers asked for, which nothing
 * on the port takes (see the top), and of which no code is left: should one
 * be taken all the same, the export fails as eacsnd's do -- -1 -- and says so
 * once. */
u32 notNative()
{
    NFS2_ASSERT(false && "eacsnd with DirectSound: not native");
    return isnd::kFailure;
}

/* A core's answer as the export gives it, for those that call nothing. */
u32 answered(const Answer& answer)
{
    return answer ? u32(**answer) : notNative();
}

/* `words` where a stdcall caller pushes them, below esp, and esp at the
 * return address's slot below them: the stack as the export finds it.  The
 * caller's esp. */
u32 pushed(App* app, ThreadState& ts, std::initializer_list<u32> words)
{
    const u32 esp = ts.sp;
    u32 slot = esp - 4 * u32(words.size());
    ts.sp = slot - 4;
    for (const u32 word : words)
    {
        app->getMemory<u32>(slot) = word;
        slot += 4;
    }
    return esp;
}

/* The answer of a core that ran on the stack pushed(): none, the stack as
 * the caller had it and -1; the game stopped, nullopt. */
std::optional<u32> ranOn(ThreadState& ts, u32 esp, const Answer& answer)
{
    if (!answer)
    {
        ts.sp = esp;
        return notNative();
    }
    if (!*answer)
        return std::nullopt;
    return u32(**answer);
}

}  // namespace

u32 recordStart(App*, ThreadState&, u32, u32, u32, u32)
{
    return isnd::kFailure;
}

u32 recordPacket(App*, ThreadState&, u32)
{
    return 0;
}

u32 recordStop(App*, ThreadState&)
{
    return 0;
}

u32 create3d(App* app, ThreadState&, u32, u32, u32, u32, u32, u32, u32)
{
    return answered(eacsndCreate3d(app));
}

u32 remove3d(App* app, ThreadState&, u32 slot)
{
    return answered(eacsndRemove3d(app, slot));
}

std::optional<u32> play3d(App* app, ThreadState& ts, u32 esi, u32 slot, u32 buffer, u32 looping, u32 rate,
                            u32 volume, u32 positionA, u32 positionB)
{
    const u32 esp = pushed(app, ts, {slot, buffer, looping, rate, volume, positionA, positionB});
    return ranOn(ts, esp, eacsndPlay3d(app, ts, esi));
}

std::optional<u32> position(App* app, ThreadState& ts, u32 buffer, u32 positionA, u32 positionB)
{
    const u32 esp = pushed(app, ts, {buffer, positionA, positionB});
    return ranOn(ts, esp, eacsndPosition(app, ts));
}

u32 volume(App* app, ThreadState&, u32 buffer, u32 volume)
{
    return answered(eacsndSetVolume(app, buffer, volume));
}

u32 rate(App* app, ThreadState&, u32 buffer, u32 rate)
{
    return answered(eacsndSetRate(app, buffer, rate));
}

std::optional<u32> stopBuffer(App* app, ThreadState& ts, u32 buffer)
{
    const u32 esp = pushed(app, ts, {buffer});
    return ranOn(ts, esp, eacsndStop(app, ts));
}

}  // namespace eacsnd
}  // namespace port

/* eacsnd's exports of these names and the entries
 * the game calls: each its thunk around a core.
 * A core with no answer is eacsnd's way with DirectSound, which the port has
 * not: the entry says so (false); the export, through its typed function,
 * fails. */

namespace
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using nfs::u32;
using port::Answer;
using port::Entry;
namespace eacsnd = port::eacsnd;

/* An entry's way out: the answer in eax and the arguments (`bytes`) off the
 * stack, unless the game stopped; false when there was none. */
bool entered(ThreadState& ts, nfs::Args& args, const Entry& e, const Answer& answer, u32 bytes)
{
    if (!answer)
        return false;
    e.finish(ts, args, *answer, bytes);
    return true;
}

/* The same for a core that left esp and the registers as eacsnd's code does
 * (the 3D position, playing and stopping a buffer): only eax is set. */
bool entered(nfs::Args& args, const Answer& answer)
{
    if (!answer)
        return false;
    if (*answer)
        args.eax = u32(**answer);
    return true;
}

}  // namespace

bool soundVersion(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    e.back(ts, args, port::eacsndVersion(app));
    return true;
}

bool soundSetFunctions(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    u32 functions[8];
    for (int32_t i = 0; i < 8; ++i)
        functions[i] = e.arg(app, i);
    port::eacsndKeepFunctions(app, functions);
    e.back(ts, args, 0, 0x20);
    return true;
}

/* Recording: eacsnd has none to start, and nothing to hand over or stop. */
bool soundRecordStart(App*, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    e.back(ts, args, isnd::kFailure, 0x10);
    return true;
}

bool soundRecordPacket(App*, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    e.back(ts, args, 0, 4);
    return true;
}

bool soundRecordStop(App*, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    e.back(ts, args, 0, 0);
    return true;
}

bool soundVolume(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return entered(ts, args, e, port::eacsndSetVolume(app, e.arg(app, 0), e.arg(app, 1)), 8);
}

bool soundPosition(App* app, ThreadState& ts, nfs::Args& args)
{
    return entered(args, port::eacsndPosition(app, ts));
}

bool soundRate(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return entered(ts, args, e, port::eacsndSetRate(app, e.arg(app, 0), e.arg(app, 1)), 8);
}

bool soundCreateBuffer(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return entered(ts, args, e, port::eacsndCreate3d(app), 0x1c);
}

bool soundRemoveBuffer(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return entered(ts, args, e, port::eacsndRemove3d(app, e.arg(app, 0)), 4);
}

bool soundPlayBuffer(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return entered(args, port::eacsndPlay3d(app, ts, e.esi));
}

bool soundStopBuffer(App* app, ThreadState& ts, nfs::Args& args)
{
    return entered(args, port::eacsndStop(app, ts));
}

/* The exports, each a thin wrapper of its typed function: the words off the
 * stack and popped. */
nfs::u32 iSNDdllversion(App* app, ThreadState& ts, nfs::Args& args)
{
    soundVersion(app, ts, args);
    return args.eax;
}

nfs::u32 iSNDdirectsetfunctions(App* app, ThreadState& ts, nfs::Args& args)
{
    soundSetFunctions(app, ts, args);
    return args.eax;
}

nfs::u32 iSNDdirectrecordstart(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, eacsnd::recordStart(app, ts, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)), 0x10);
}

nfs::u32 iSNDdirectrecordpacket(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, eacsnd::recordPacket(app, ts, e.arg(app, 0)), 4);
}

nfs::u32 iSNDdirectrecordstop(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, eacsnd::recordStop(app, ts), 0);
}

nfs::u32 iSNDdirectvol(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, eacsnd::volume(app, ts, e.arg(app, 0), e.arg(app, 1)), 8);
}

nfs::u32 iSNDdirectrate(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, eacsnd::rate(app, ts, e.arg(app, 0), e.arg(app, 1)), 8);
}

nfs::u32 iSNDdirectcreate3dbuf(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args,
           eacsnd::create3d(app, ts, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3), e.arg(app, 4),
                            e.arg(app, 5), e.arg(app, 6)),
           0x1c);
    return args.eax;
}

/* esp where the caller had it, for the typed function. */
nfs::u32 iSNDdirectremovebuf(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    ts.sp = e.esp + 4 + 4;
    return e.back(ts, args, eacsnd::remove3d(app, ts, e.arg(app, 0)), 4);
}

/* The three that are eacsnd's code to the register: esp where the caller had
 * it, above the words, for their typed functions, which put the same words
 * back where they are and leave esp and the registers as eacsnd's code does;
 * only eax set. */
nfs::u32 iSNDdirectpos3d(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    ts.sp = e.esp + 4 + 0xc;
    if (const std::optional<u32> result = eacsnd::position(app, ts, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2)))
        args.eax = *result;
    return args.eax;
}

nfs::u32 iSNDdirectplay3d(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    ts.sp = e.esp + 4 + 0x1c;
    if (const std::optional<u32> result =
            eacsnd::play3d(app, ts, e.esi, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3),
                           e.arg(app, 4), e.arg(app, 5), e.arg(app, 6)))
        args.eax = *result;
    return args.eax;
}

nfs::u32 iSNDdirectstopbuf(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    ts.sp = e.esp + 4 + 4;
    if (const std::optional<u32> result = eacsnd::stopBuffer(app, ts, e.arg(app, 0)))
        args.eax = *result;
    return args.eax;
}

}  // namespace nfs3hp
