#ifndef NFS3HP_ISND_CALLS_H
#define NFS3HP_ISND_CALLS_H

#include "isnd.h"
#include "port.h"

#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

/* The calls between the game and its sound driver through the pointers each
 * keeps -- the game's to eacsnd's exports (isnd.h, kDirect*), eacsnd's to the
 * game's functions setfunctions handed it (isnd.h, kFunction*) -- as C++
 * calls of the port's own functions: plan step 4's, for iSND.
 *
 *     guest::sound<isnd::kDirectVol, &port::eacsnd::volume>(app, cpu, buffer, volume)
 *
 * The words are what the caller pushes, in its order.  In the game, when the
 * pointer leads to the port's export -- the adapter registered under its name
 * (disassembly/eacsnd.cpp), which GetProcAddress gave iSNDloaddll -- the
 * typed function is called with them; a pointer to anything else is called as
 * the game's code calls it.  Likewise guest::lambda for eacsnd's calls of the
 * game's functions, the natives native_game.cpp registers where the game's
 * code was.  In tools/game_checks (NFS_GAME_CHECKS), tools/frontend_checks
 * (NFS_FRONTEND_CHECKS) and tools/sound_checks (NFS_SOUND_CHECKS) the call
 * always goes through the pointer, stdcall, the words pushed below esp, for
 * the check's stand-in; the first two build neither native_sound.cpp nor
 * port_snd_isnd.cpp, the last only port_snd_isnd.cpp.
 *
 * Either way the registers are left as the adapter the pointer leads to
 * leaves them -- the typed function does what the adapter does, and leaves
 * them so -- and esp where the caller had it, as a stdcall's ret and
 * guest::call leave it.  What the callee leaves in eax, or nullopt when the
 * game stopped in it. */
namespace nfs3hp
{

/* eacsnd's exports as the game calls them (native_sound.cpp,
 * port_snd_isnd.cpp). */
void iSNDdirectcaps(win32::WinApplication* app, x86::CPU& cpu);
void iSNDdirectstart(win32::WinApplication* app, x86::CPU& cpu);
void iSNDdirectstop(win32::WinApplication* app, x86::CPU& cpu);
void iSNDdirectrecordstart(win32::WinApplication* app, x86::CPU& cpu);
void iSNDdirectrecordpacket(win32::WinApplication* app, x86::CPU& cpu);
void iSNDdirectrecordstop(win32::WinApplication* app, x86::CPU& cpu);
void iSNDdirectserve(win32::WinApplication* app, x86::CPU& cpu);
void iSNDdirectcreate3dbuf(win32::WinApplication* app, x86::CPU& cpu);
void iSNDdirectremovebuf(win32::WinApplication* app, x86::CPU& cpu);
void iSNDdirectplay3d(win32::WinApplication* app, x86::CPU& cpu);
void iSNDdirectpos3d(win32::WinApplication* app, x86::CPU& cpu);
void iSNDdirectvol(win32::WinApplication* app, x86::CPU& cpu);
void iSNDdirectrate(win32::WinApplication* app, x86::CPU& cpu);
void iSNDdirectstopbuf(win32::WinApplication* app, x86::CPU& cpu);

/* The four of the game's functions eacsnd calls (native_game.cpp): the
 * natives where the game's code was, and each as C++ -- its words as its
 * parameters, what it leaves in eax, nullopt when the game stopped in it;
 * the registers left as the native leaves them. */
void AcquireSoundDataLambda(win32::WinApplication* app, x86::CPU& cpu);
void StopSoundBufferLambda(win32::WinApplication* app, x86::CPU& cpu);
void AcquireSoundBufferPositionLambda(win32::WinApplication* app, x86::CPU& cpu);
void TickSoundLambda(win32::WinApplication* app, x86::CPU& cpu);

std::optional<x86::reg32> acquireSoundDataOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer,
                                             x86::reg32 frames);
std::optional<x86::reg32> stopSoundBufferOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer);
std::optional<x86::reg32> acquireSoundBufferPositionOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 first,
                                                       x86::reg32 second, x86::reg32 position);
std::optional<x86::reg32> tickSoundOf(win32::WinApplication* app, x86::CPU& cpu);

namespace port
{

/* The library's 100 Hz tick (port_snd_voices.cpp, port_snd.h), the game's
 * 0x500c80, and its adapter: what TickSoundLambda's pointer holds unless the
 * game set another. */
Result tickSound(App* app, CPU& cpu, reg32 esp);
void soundTick(App* app, CPU& cpu);

/* eacsnd's 14 exports the game keeps pointers to, as C++
 * (native_sound.cpp: caps, start, stop, serve; port_snd_isnd.cpp: the rest):
 * each one's words as its parameters, in their order (isnd.h), and its result
 * -- what eacsnd leaves in eax.  The iSNDdirect* adapters take the words off
 * the stack, call these and pop the words.
 *
 * Those that call the game's functions give nullopt when the game stopped in
 * one.  pos3d, play3d and stopbuf are eacsnd's code to the register
 * (port_snd_isnd.cpp): theirs put the words below esp, where a caller pushes
 * them, run on the stack from there and read the caller's registers from cpu
 * (esi and the four eacsnd keeps), leaving them as eacsnd's code does.  A
 * way the port has not -- eacsnd with DirectSound -- asserts and gives -1, as
 * the exports do. */
namespace eacsnd
{

reg32 caps(App* app, CPU& cpu, reg32 window);
reg32 start(App* app, CPU& cpu, reg32 caps, reg32 window);
reg32 stop(App* app, CPU& cpu);
/* Recording: none to start, nothing to hand over or stop. */
reg32 recordStart(App* app, CPU& cpu, reg32, reg32, reg32, reg32);
reg32 recordPacket(App* app, CPU& cpu, reg32);
reg32 recordStop(App* app, CPU& cpu);
std::optional<reg32> serve(App* app, CPU& cpu);
reg32 create3d(App* app, CPU& cpu, reg32 channels, reg32 format, reg32 rate, reg32 length, reg32 loopStart,
               reg32 loopEnd, reg32 data);
/* ebx, which it pushes on the way that frees, read from cpu. */
reg32 remove3d(App* app, CPU& cpu, reg32 slot);
std::optional<reg32> play3d(App* app, CPU& cpu, reg32 slot, reg32 buffer, reg32 looping, reg32 rate, reg32 volume,
                            reg32 positionA, reg32 positionB);
std::optional<reg32> position(App* app, CPU& cpu, reg32 buffer, reg32 positionA, reg32 positionB);
reg32 volume(App* app, CPU& cpu, reg32 buffer, reg32 volume);
reg32 rate(App* app, CPU& cpu, reg32 buffer, reg32 rate);
std::optional<reg32> stopBuffer(App* app, CPU& cpu, reg32 buffer);

}  // namespace eacsnd
}  // namespace port

namespace guest
{

#if defined(NFS_GAME_CHECKS) || defined(NFS_FRONTEND_CHECKS) || defined(NFS_SOUND_CHECKS)
#define NFS3HP_SOUND_THROUGH_SLOTS 1
#endif

/* Each pointer's callee: its typed function, its words and, in the game, its
 * adapter. */
template <x86::reg32 Slot>
struct SoundCallee;

#ifdef NFS3HP_SOUND_THROUGH_SLOTS
#define NFS3HP_SOUND_CALLEE(slot, typed, adapter, words)                 \
    template <>                                                          \
    struct SoundCallee<isnd::slot>                                       \
    {                                                                    \
        static constexpr auto function = &typed;                         \
        static constexpr std::size_t count = words;                      \
    }
#else
#define NFS3HP_SOUND_CALLEE(slot, typed, adapter, words)                 \
    template <>                                                          \
    struct SoundCallee<isnd::slot>                                       \
    {                                                                    \
        static constexpr auto function = &typed;                         \
        static constexpr std::size_t count = words;                      \
        static constexpr Native registered = &nfs3hp::adapter;           \
    }
#endif

NFS3HP_SOUND_CALLEE(kDirectCaps, port::eacsnd::caps, iSNDdirectcaps, 1);
NFS3HP_SOUND_CALLEE(kDirectStart, port::eacsnd::start, iSNDdirectstart, 2);
NFS3HP_SOUND_CALLEE(kDirectStop, port::eacsnd::stop, iSNDdirectstop, 0);
NFS3HP_SOUND_CALLEE(kDirectRecordStart, port::eacsnd::recordStart, iSNDdirectrecordstart, 4);
NFS3HP_SOUND_CALLEE(kDirectRecordPacket, port::eacsnd::recordPacket, iSNDdirectrecordpacket, 1);
NFS3HP_SOUND_CALLEE(kDirectRecordStop, port::eacsnd::recordStop, iSNDdirectrecordstop, 0);
NFS3HP_SOUND_CALLEE(kDirectServe, port::eacsnd::serve, iSNDdirectserve, 0);
NFS3HP_SOUND_CALLEE(kDirectCreate3dBuf, port::eacsnd::create3d, iSNDdirectcreate3dbuf, 7);
NFS3HP_SOUND_CALLEE(kDirectRemoveBuf, port::eacsnd::remove3d, iSNDdirectremovebuf, 1);
NFS3HP_SOUND_CALLEE(kDirectPlay3d, port::eacsnd::play3d, iSNDdirectplay3d, 7);
NFS3HP_SOUND_CALLEE(kDirectPos3d, port::eacsnd::position, iSNDdirectpos3d, 3);
NFS3HP_SOUND_CALLEE(kDirectVol, port::eacsnd::volume, iSNDdirectvol, 2);
NFS3HP_SOUND_CALLEE(kDirectRate, port::eacsnd::rate, iSNDdirectrate, 2);
NFS3HP_SOUND_CALLEE(kDirectStopBuf, port::eacsnd::stopBuffer, iSNDdirectstopbuf, 1);
// eacsnd's pointers to the game's functions.
NFS3HP_SOUND_CALLEE(kFunctionMix, acquireSoundDataOf, AcquireSoundDataLambda, 2);
NFS3HP_SOUND_CALLEE(kFunctionStopBuffer, stopSoundBufferOf, StopSoundBufferLambda, 1);
NFS3HP_SOUND_CALLEE(kFunctionBufferPosition, acquireSoundBufferPositionOf, AcquireSoundBufferPositionLambda, 3);
NFS3HP_SOUND_CALLEE(kFunctionTick, tickSoundOf, TickSoundLambda, 0);

#undef NFS3HP_SOUND_CALLEE

/* The typed function's result as eax, unless the game stopped: esp back
 * where the caller had it. */
template <typename R>
inline std::optional<x86::reg32> soundResult(x86::CPU& cpu, x86::reg32 esp, const R& result)
{
    x86::reg32 eax;
    if constexpr (std::is_same_v<R, std::optional<x86::reg32>>)
    {
        if (!result)
            return std::nullopt;
        eax = *result;
    }
    else
    {
        eax = result;
    }
    if (cpu.terminate)
        return std::nullopt;
    cpu.eax = eax;
    cpu.esp = esp;
    return eax;
}

/* The callee at Slot with `words`, from esp as cpu has it: its typed function
 * or through the pointer (above). */
template <x86::reg32 Slot, auto Fn, typename... W>
inline std::optional<x86::reg32> sound(win32::WinApplication* app, x86::CPU& cpu, W... words)
{
    static_assert(Fn == SoundCallee<Slot>::function, "the callee the pointer is to");
    static_assert(sizeof...(W) == SoundCallee<Slot>::count, "every word the callee takes");
    static_assert(isnd::exportAt(Slot).eacsnd == 0 || 4 * sizeof...(W) == isnd::exportAt(Slot).bytes,
                  "every word the export takes");
#ifndef NFS3HP_SOUND_THROUGH_SLOTS
    if (app->methodAt(at(app, Slot)) == SoundCallee<Slot>::registered)
    {
        const x86::reg32 esp = cpu.esp;
        return soundResult(cpu, esp, Fn(app, cpu, x86::reg32(words)...));
    }
#endif
    if (!call(app, cpu, at(app, Slot), {x86::reg32(words)...}))
        return std::nullopt;
    return cpu.eax;
}

/* The same from a rewrite's frame (port.h's callGame): esp first set to
 * `esp`, where the game's code has it at the call, and the registers to what
 * it leaves in them there, `a`; both left so. */
template <x86::reg32 Slot, auto Fn, typename... W>
inline std::optional<x86::reg32> soundFrom(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 esp, port::Args a,
                                           W... words)
{
    cpu.esp = esp;
    cpu.eax = a.eax;
    cpu.edx = a.edx;
    cpu.ebx = a.ebx;
    cpu.ecx = a.ecx;
    return sound<Slot, Fn>(app, cpu, words...);
}

#ifndef NFS3HP_SOUND_THROUGH_SLOTS
template <auto Fn, std::size_t... I>
inline auto soundPushedTyped(win32::WinApplication* app, x86::CPU& cpu, std::index_sequence<I...>)
{
    return Fn(app, cpu, at(app, cpu.esp + 4 * x86::reg32(I))...);
}
#endif

/* A call from eacsnd's code to the register (port_snd_isnd.cpp), its words
 * already pushed and esp at the first: the return address's slot taken, as a
 * call takes it (nothing is written there: the generated code does not write
 * it either), and the callee at Slot called -- its typed function with the
 * words where they lie, or through the pointer.  esp as the stdcall's ret
 * leaves it, past the words, unless the game stopped (false). */
template <x86::reg32 Slot, auto Fn>
inline bool soundPushed(win32::WinApplication* app, x86::CPU& cpu)
{
    static_assert(Fn == SoundCallee<Slot>::function, "the callee the pointer is to");
#ifndef NFS3HP_SOUND_THROUGH_SLOTS
    if (app->methodAt(at(app, Slot)) == SoundCallee<Slot>::registered)
    {
        constexpr std::size_t kCount = SoundCallee<Slot>::count;
        const x86::reg32 esp = cpu.esp + 4 * x86::reg32(kCount);
        return bool(soundResult(cpu, esp, soundPushedTyped<Fn>(app, cpu, std::make_index_sequence<kCount>())));
    }
#endif
    cpu.esp -= 4;
    app->dynamic_call(at(app, Slot), cpu);
    return !cpu.terminate;
}

}  // namespace guest
}  // namespace nfs3hp

#endif
