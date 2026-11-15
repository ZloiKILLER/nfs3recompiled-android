#include "isnd_calls.h"

#include <lib/winapp.h>
#include <x86.h>

#include <optional>

/* eacsnd's twelve exports native_sound.cpp leaves: the version, the game's
 * functions, recording, and the 3D buffers -- the functions at the bottom,
 * which disassemble_nfs3hp.py binds to the exports of those names, eacsnd
 * having no generated code -- with eacsnd's own data in guest memory as the
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
 * callee would -- tools/sound_checks lets the game's functions move them, and
 * holds the result to eacsnd's -- so their cores run on the CPU (esp and esi
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
 * was, as C++, when the pointers lead there.
 *
 * tools/sound_checks compares the sound* entries with eacsnd's generated code,
 * made afresh from the DLL, on random states (NFS_SOUND_CHECKS: the calls out
 * always through the pointers, to its stand-ins). */

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
    reg32 active;             // IDirectSoundBuffer*: what plays, the slot's or a duplicate of it
    reg32 main;               // IDirectSoundBuffer*: the slot's
    reg32 buffer3d;           // IDirectSound3DBuffer*
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
    reg32 id;                 // 0: free
    reg32 buffer;             // IDirectSoundBuffer*
    uint8_t other8[0x14];
};
#pragma pack(pop)
static_assert(sizeof(Buffer) == isnd::kBufferBytes && sizeof(Slot) == isnd::kSlotBytes, "eacsnd's records");

constexpr Records<Buffer> kBuffers{isnd::kBuffers};
constexpr Records<Slot> kSlots{isnd::kSlots};
constexpr Var<reg32> kCapsKept{isnd::kCapsKept};
constexpr Table<reg32> kFunctions{isnd::kFunctionMix};   // the game's eight functions, in the order given
constexpr Var<uint8_t> kDirectSoundMade{isnd::kDirectSoundMade}, kIs3d{isnd::kIs3d};
constexpr Var<double> kPositionScale{isnd::kPositionScale};

/* A word pushed: esp moved down and the word put there. */
void push(App* app, CPU& cpu, reg32 word)
{
    cpu.esp -= 4;
    app->getMemory<reg32>(cpu.esp) = word;
}

/* A call out, with its arguments already pushed and esp at them: the return
 * address's slot taken, as a call takes it, though nothing is written there --
 * the generated code does not write it either.  False when the game is to
 * stop, and every function on the way out stops where it is. */
bool callOut(App* app, CPU& cpu, reg32 address)
{
    cpu.esp -= 4;
    app->dynamic_call(address, cpu);
    return !cpu.terminate;
}

/* A COM object's Release, IUnknown's third method: stdcall (this). */
bool release(App* app, CPU& cpu, reg32 object)
{
    push(app, cpu, object);
    return callOut(app, cpu, app->getMemory<reg32>(app->getMemory<reg32>(object) + 8));
}

}  // namespace

/* iSNDdllversion's: 16.3, and the card's formats to be asked again. */
reg32 eacsndVersion(App* app)
{
    kCapsKept.set(app, isnd::kFailure);
    return isnd::kVersion;
}

/* iSNDdirectsetfunctions': the game's eight functions kept, in their order. */
void eacsndKeepFunctions(App* app, const reg32 (&functions)[8])
{
    for (int32_t i = 0; i < 8; ++i)
        kFunctions.set(app, i, functions[i]);
}

/* iSNDdirectvol's: buffer `buffer`'s volume's byte kept. */
Answer eacsndSetVolume(App* app, reg32 buffer, reg32 volume)
{
    if (kDirectSoundMade(app) != 0)
        return std::nullopt;
    kBuffers(app, int32_t(buffer)).volume = uint8_t(volume);
    return Result(0);
}

/* iSNDdirectrate's: buffer `buffer`'s rate's word kept. */
Answer eacsndSetRate(App* app, reg32 buffer, reg32 rate)
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
Answer eacsndPosition(App* app, CPU& cpu)
{
    if (kDirectSoundMade(app) != 0)
        return std::nullopt;
    cpu.esp -= 0x14;
    const reg32 position = cpu.esp;
    push(app, cpu, position);
    push(app, cpu, app->getMemory<reg32>(cpu.esp + 0x24));  // b
    push(app, cpu, app->getMemory<reg32>(cpu.esp + 0x24));  // a
    if (!guest::soundPushed<isnd::kFunctionBufferPosition, &acquireSoundBufferPositionOf>(app, cpu))
        return Result();

    const reg32 esp = cpu.esp;
    const double scale = kPositionScale(app);
    const reg32 index = app->getMemory<reg32>(esp + 0x18);
    for (reg32 at = esp; at < esp + 12; at += 4)
        store<float>(app, at, float(double(load<float>(app, at)) * scale));
    Buffer& record = kBuffers(app, int32_t(index));
    record.positionA = uint16_t(app->getMemory<reg32>(esp + 0x1c));
    record.positionB = uint16_t(app->getMemory<reg32>(esp + 0x20));
    cpu.esp = esp + 0x14 + 4 + 0xc;
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
 * the slot `slot` freed.  `esp` is the stack at entry: ebx is pushed below it
 * on that way, as eacsnd does. */
Answer eacsndRemove3d(App* app, reg32 esp, reg32 ebx, reg32 slot)
{
    if (kIs3d(app) == 0)
        return Result(int32_t(isnd::kFailure));
    if (kDirectSoundMade(app) != 0)
        return std::nullopt;
    app->getMemory<reg32>(esp - 4) = ebx;
    kSlots(app, int32_t(slot)).id = 0;
    return Result(0);
}

/* iSNDdirectplay3d's (seven words), without 3D buffers: eacsnd jumps straight
 * to its way out of a failure, which clears the buffer record esi points at --
 * but esi is only set on the way that plays, so the record it clears is
 * wherever the caller's esi points, taken as each call out leaves it.  Its two
 * objects there, the one that plays unless it is the slot's own, and the 3D
 * one, are released first, and ebx, esi, edi and ebp -- pushed on entry --
 * popped from where esp then is, and -1 returned. */
Answer eacsndPlay3d(App* app, CPU& cpu)
{
    if (kIs3d(app) != 0)
        return std::nullopt;
    const reg32 entry = cpu.esp;
    app->getMemory<reg32>(entry - 4) = cpu.ebx;
    app->getMemory<reg32>(entry - 8) = cpu.esi;
    app->getMemory<reg32>(entry - 0xc) = cpu.edi;
    app->getMemory<reg32>(entry - 0x10) = cpu.ebp;
    cpu.esp = entry - 0x10 - 0x64;

    Buffer* buffer = &record<Buffer>(app, cpu.esi);
    if (const reg32 active = buffer->active; active != 0 && active != buffer->main)
        if (!release(app, cpu, active))
            return Result();
    buffer = &record<Buffer>(app, cpu.esi);
    if (const reg32 object = buffer->buffer3d; object != 0)
        if (!release(app, cpu, object))
            return Result();
    buffer = &record<Buffer>(app, cpu.esi);
    buffer->main = 0;
    buffer->active = 0;
    buffer->buffer3d = 0;

    const reg32 frame = cpu.esp + 0x64;
    cpu.ebp = app->getMemory<reg32>(frame);
    cpu.edi = app->getMemory<reg32>(frame + 4);
    cpu.esi = app->getMemory<reg32>(frame + 8);
    cpu.ebx = app->getMemory<reg32>(frame + 0xc);
    cpu.esp = frame + 0x10 + 4 + 0x1c;
    return Result(int32_t(isnd::kFailure));
}

/* iSNDdirectstopbuf's (buffer on the stack), with no DirectSound: the record's
 * three objects forgotten, and the game told through its own function that the
 * buffer stopped; ebx and edi, pushed on entry, popped from where esp then
 * is. */
Answer eacsndStop(App* app, CPU& cpu)
{
    if (kDirectSoundMade(app) != 0)
        return std::nullopt;
    const reg32 entry = cpu.esp;
    app->getMemory<reg32>(entry - 4) = cpu.ebx;
    app->getMemory<reg32>(entry - 8) = cpu.edi;
    cpu.esp = entry - 8;
    const reg32 index = app->getMemory<reg32>(entry + 4);
    Buffer& record = kBuffers(app, int32_t(index));
    record.main = 0;
    record.active = 0;
    push(app, cpu, index);
    record.buffer3d = 0;
    if (!guest::soundPushed<isnd::kFunctionStopBuffer, &stopSoundBufferOf>(app, cpu))
        return Result();

    cpu.edi = app->getMemory<reg32>(cpu.esp);
    cpu.ebx = app->getMemory<reg32>(cpu.esp + 4);
    cpu.esp += 8 + 4 + 4;
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
reg32 notNative()
{
    NFS2_ASSERT(false && "eacsnd with DirectSound: not native");
    return isnd::kFailure;
}

/* A core's answer as the export gives it, for those that call nothing. */
reg32 answered(const Answer& answer)
{
    return answer ? reg32(**answer) : notNative();
}

/* `words` where a stdcall caller pushes them, below esp, and esp at the
 * return address's slot below them: the stack as the export finds it.  The
 * caller's esp. */
reg32 pushed(App* app, CPU& cpu, std::initializer_list<reg32> words)
{
    const reg32 esp = cpu.esp;
    reg32 slot = esp - 4 * reg32(words.size());
    cpu.esp = slot - 4;
    for (const reg32 word : words)
    {
        app->getMemory<reg32>(slot) = word;
        slot += 4;
    }
    return esp;
}

/* The answer of a core that ran on the stack pushed(): none, the stack as
 * the caller had it and -1; the game stopped, nullopt. */
std::optional<reg32> ranOn(CPU& cpu, reg32 esp, const Answer& answer)
{
    if (!answer)
    {
        cpu.esp = esp;
        return notNative();
    }
    if (!*answer)
        return std::nullopt;
    return reg32(**answer);
}

}  // namespace

reg32 recordStart(App*, CPU&, reg32, reg32, reg32, reg32)
{
    return isnd::kFailure;
}

reg32 recordPacket(App*, CPU&, reg32)
{
    return 0;
}

reg32 recordStop(App*, CPU&)
{
    return 0;
}

reg32 create3d(App* app, CPU&, reg32, reg32, reg32, reg32, reg32, reg32, reg32)
{
    return answered(eacsndCreate3d(app));
}

reg32 remove3d(App* app, CPU& cpu, reg32 slot)
{
    return answered(eacsndRemove3d(app, cpu.esp - 4 - 4, cpu.ebx, slot));
}

std::optional<reg32> play3d(App* app, CPU& cpu, reg32 slot, reg32 buffer, reg32 looping, reg32 rate, reg32 volume,
                            reg32 positionA, reg32 positionB)
{
    const reg32 esp = pushed(app, cpu, {slot, buffer, looping, rate, volume, positionA, positionB});
    return ranOn(cpu, esp, eacsndPlay3d(app, cpu));
}

std::optional<reg32> position(App* app, CPU& cpu, reg32 buffer, reg32 positionA, reg32 positionB)
{
    const reg32 esp = pushed(app, cpu, {buffer, positionA, positionB});
    return ranOn(cpu, esp, eacsndPosition(app, cpu));
}

reg32 volume(App* app, CPU&, reg32 buffer, reg32 volume)
{
    return answered(eacsndSetVolume(app, buffer, volume));
}

reg32 rate(App* app, CPU&, reg32 buffer, reg32 rate)
{
    return answered(eacsndSetRate(app, buffer, rate));
}

std::optional<reg32> stopBuffer(App* app, CPU& cpu, reg32 buffer)
{
    const reg32 esp = pushed(app, cpu, {buffer});
    return ranOn(cpu, esp, eacsndStop(app, cpu));
}

}  // namespace eacsnd
}  // namespace port

/* eacsnd's exports of these names (disassemble_nfs3hp.py) and the entries
 * tools/sound_checks holds to eacsnd's code: each its adapter around a core.
 * A core with no answer is eacsnd's way with DirectSound, which the port has
 * not: the entry says so (false); the export, through its typed function,
 * fails. */

namespace
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using x86::reg32;
using port::Answer;
using port::Entry;
namespace eacsnd = port::eacsnd;

/* An entry's way out: the answer in eax and the arguments (`bytes`) off the
 * stack, unless the game stopped; false when there was none. */
bool entered(CPU& cpu, const Entry& e, const Answer& answer, reg32 bytes)
{
    if (!answer)
        return false;
    e.finish(cpu, *answer, bytes);
    return true;
}

/* The same for a core that left esp and the registers as eacsnd's code does
 * (the 3D position, playing and stopping a buffer): only eax is set. */
bool entered(CPU& cpu, const Answer& answer)
{
    if (!answer)
        return false;
    if (*answer)
        cpu.eax = reg32(**answer);
    return true;
}

}  // namespace

bool soundVersion(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, port::eacsndVersion(app));
    return true;
}

bool soundSetFunctions(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 functions[8];
    for (int32_t i = 0; i < 8; ++i)
        functions[i] = e.arg(app, i);
    port::eacsndKeepFunctions(app, functions);
    e.back(cpu, 0, 0x20);
    return true;
}

/* Recording: eacsnd has none to start, and nothing to hand over or stop. */
bool soundRecordStart(App*, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, isnd::kFailure, 0x10);
    return true;
}

bool soundRecordPacket(App*, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, 0, 4);
    return true;
}

bool soundRecordStop(App*, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, 0, 0);
    return true;
}

bool soundVolume(App* app, CPU& cpu)
{
    const Entry e(cpu);
    return entered(cpu, e, port::eacsndSetVolume(app, e.arg(app, 0), e.arg(app, 1)), 8);
}

bool soundPosition(App* app, CPU& cpu)
{
    return entered(cpu, port::eacsndPosition(app, cpu));
}

bool soundRate(App* app, CPU& cpu)
{
    const Entry e(cpu);
    return entered(cpu, e, port::eacsndSetRate(app, e.arg(app, 0), e.arg(app, 1)), 8);
}

bool soundCreateBuffer(App* app, CPU& cpu)
{
    const Entry e(cpu);
    return entered(cpu, e, port::eacsndCreate3d(app), 0x1c);
}

bool soundRemoveBuffer(App* app, CPU& cpu)
{
    const Entry e(cpu);
    return entered(cpu, e, port::eacsndRemove3d(app, e.esp, e.ebx, e.arg(app, 0)), 4);
}

bool soundPlayBuffer(App* app, CPU& cpu)
{
    return entered(cpu, port::eacsndPlay3d(app, cpu));
}

bool soundStopBuffer(App* app, CPU& cpu)
{
    return entered(cpu, port::eacsndStop(app, cpu));
}

/* The exports, each a thin wrapper of its typed function: the words off the
 * stack and popped. */
void iSNDdllversion(App* app, CPU& cpu)
{
    soundVersion(app, cpu);
}

void iSNDdirectsetfunctions(App* app, CPU& cpu)
{
    soundSetFunctions(app, cpu);
}

void iSNDdirectrecordstart(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, eacsnd::recordStart(app, cpu, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)), 0x10);
}

void iSNDdirectrecordpacket(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, eacsnd::recordPacket(app, cpu, e.arg(app, 0)), 4);
}

void iSNDdirectrecordstop(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, eacsnd::recordStop(app, cpu), 0);
}

void iSNDdirectvol(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, eacsnd::volume(app, cpu, e.arg(app, 0), e.arg(app, 1)), 8);
}

void iSNDdirectrate(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, eacsnd::rate(app, cpu, e.arg(app, 0), e.arg(app, 1)), 8);
}

void iSNDdirectcreate3dbuf(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu,
           eacsnd::create3d(app, cpu, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3), e.arg(app, 4),
                            e.arg(app, 5), e.arg(app, 6)),
           0x1c);
}

/* esp where the caller had it, for the typed function. */
void iSNDdirectremovebuf(App* app, CPU& cpu)
{
    const Entry e(cpu);
    cpu.esp = e.esp + 4 + 4;
    e.back(cpu, eacsnd::remove3d(app, cpu, e.arg(app, 0)), 4);
}

/* The three that are eacsnd's code to the register: esp where the caller had
 * it, above the words, for their typed functions, which put the same words
 * back where they are and leave esp and the registers as eacsnd's code does;
 * only eax set. */
void iSNDdirectpos3d(App* app, CPU& cpu)
{
    const Entry e(cpu);
    cpu.esp = e.esp + 4 + 0xc;
    if (const std::optional<reg32> result = eacsnd::position(app, cpu, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2)))
        cpu.eax = *result;
}

void iSNDdirectplay3d(App* app, CPU& cpu)
{
    const Entry e(cpu);
    cpu.esp = e.esp + 4 + 0x1c;
    if (const std::optional<reg32> result =
            eacsnd::play3d(app, cpu, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3), e.arg(app, 4),
                           e.arg(app, 5), e.arg(app, 6)))
        cpu.eax = *result;
}

void iSNDdirectstopbuf(App* app, CPU& cpu)
{
    const Entry e(cpu);
    cpu.esp = e.esp + 4 + 4;
    if (const std::optional<reg32> result = eacsnd::stopBuffer(app, cpu, e.arg(app, 0)))
        cpu.eax = *result;
}

}  // namespace nfs3hp
