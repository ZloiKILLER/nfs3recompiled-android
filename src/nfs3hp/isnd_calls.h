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
 * pointer leads to the port's export -- the thunk registered under its name
 * (disassembly/eacsnd.cpp), which GetProcAddress gave iSNDloaddll -- the
 * typed function is called with them; a pointer to anything else is called as
 * the game's code calls it.  Likewise guest::lambda for eacsnd's calls of the
 * game's functions, the natives native_game.cpp registers where the game's
 * code was.
 *
 * Either way the registers are left as the thunk the pointer leads to
 * leaves them -- the typed function does what the thunk does, and leaves
 * them so -- and esp where the caller had it, as a stdcall's ret and
 * guest::call leave it.  What the callee leaves in eax, or nullopt when the
 * game stopped in it. */
namespace nfs3hp
{

/* eacsnd's exports as the game calls them (native_sound.cpp,
 * port_snd_isnd.cpp). */
x86::reg32 iSNDdirectcaps(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 iSNDdirectstart(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 iSNDdirectstop(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 iSNDdirectrecordstart(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 iSNDdirectrecordpacket(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 iSNDdirectrecordstop(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 iSNDdirectserve(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 iSNDdirectcreate3dbuf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 iSNDdirectremovebuf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 iSNDdirectplay3d(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 iSNDdirectpos3d(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 iSNDdirectvol(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 iSNDdirectrate(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 iSNDdirectstopbuf(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);

/* The four of the game's functions eacsnd calls (native_game.cpp): the
 * natives where the game's code was, and each as C++ -- its words as its
 * parameters, what it leaves in eax, nullopt when the game stopped in it;
 * the registers left as the native leaves them. */
x86::reg32 AcquireSoundDataLambda(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 StopSoundBufferLambda(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 AcquireSoundBufferPositionLambda(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 TickSoundLambda(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args);

std::optional<x86::reg32> acquireSoundDataOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer,
                                             x86::reg32 frames);
std::optional<x86::reg32> stopSoundBufferOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 buffer);
std::optional<x86::reg32> acquireSoundBufferPositionOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 first,
                                                       x86::reg32 second, x86::reg32 position);
std::optional<x86::reg32> tickSoundOf(win32::WinApplication* app, x86::CPU& cpu);

namespace port
{

/* The library's 100 Hz tick (port_snd_voices.cpp, port_snd.h), the game's
 * 0x500c80, and its thunk: what TickSoundLambda's pointer holds unless the
 * game set another. */
x86::reg32 soundTick(App* app, CPU& cpu, x86::Args& args);

/* eacsnd's 14 exports the game keeps pointers to, as C++
 * (native_sound.cpp: caps, start, stop, serve; port_snd_isnd.cpp: the rest):
 * each one's words as its parameters, in their order (isnd.h), and its result
 * -- what eacsnd leaves in eax.  The iSNDdirect* thunks take the words off
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
reg32 remove3d(App* app, CPU& cpu, reg32 slot);
/* `esi` the caller's: the record eacsnd's way out of a failure clears. */
std::optional<reg32> play3d(App* app, CPU& cpu, reg32 esi, reg32 slot, reg32 buffer, reg32 looping, reg32 rate,
                            reg32 volume, reg32 positionA, reg32 positionB);
std::optional<reg32> position(App* app, CPU& cpu, reg32 buffer, reg32 positionA, reg32 positionB);
reg32 volume(App* app, CPU& cpu, reg32 buffer, reg32 volume);
reg32 rate(App* app, CPU& cpu, reg32 buffer, reg32 rate);
std::optional<reg32> stopBuffer(App* app, CPU& cpu, reg32 buffer);

}  // namespace eacsnd
}  // namespace port

namespace guest
{


/* Each pointer's callee: its typed function, its words and, in the game, its
 * thunk. */
template <x86::reg32 Slot>
struct SoundCallee;

#ifdef NFS3HP_SOUND_THROUGH_SLOTS
#define NFS3HP_SOUND_CALLEE(slot, typed, thunk, words)                 \
    template <>                                                          \
    struct SoundCallee<isnd::slot>                                       \
    {                                                                    \
        static constexpr auto function = &typed;                         \
        static constexpr std::size_t count = words;                      \
    }
#else
#define NFS3HP_SOUND_CALLEE(slot, typed, thunk, words)                 \
    template <>                                                          \
    struct SoundCallee<isnd::slot>                                       \
    {                                                                    \
        static constexpr auto function = &typed;                         \
        static constexpr std::size_t count = words;                      \
        static constexpr win32::ThunkPtr registered = &nfs3hp::thunk;           \
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

/* The typed function's result (its eax), unless the game stopped: esp back
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
    if (app->thunkAt(at(app, Slot)) == SoundCallee<Slot>::registered)
    {
        const x86::reg32 esp = cpu.esp;
        return soundResult(cpu, esp, Fn(app, cpu, x86::reg32(words)...));
    }
#endif
    x86::Args a;
    return pointerCall(app, cpu, at(app, Slot), a, {x86::reg32(words)...});
}

/* The same from a rewrite's frame: esp first set to `esp`, where the game's
 * code has it at the call, and left so.  `eax` is what the game's code has in
 * eax there: the callee's typed function takes only its words, but through
 * the pointer (port.h's callGame) eax goes along. */
template <x86::reg32 Slot, auto Fn, typename... W>
inline std::optional<x86::reg32> soundFrom(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 esp, x86::reg32 eax,
                                           W... words)
{
    static_assert(Fn == SoundCallee<Slot>::function, "the callee the pointer is to");
    static_assert(sizeof...(W) == SoundCallee<Slot>::count, "every word the callee takes");
    cpu.esp = esp;
#ifndef NFS3HP_SOUND_THROUGH_SLOTS
    if (app->thunkAt(at(app, Slot)) == SoundCallee<Slot>::registered)
        return soundResult(cpu, esp, Fn(app, cpu, x86::reg32(words)...));
#endif
    x86::Args a;
    a.eax = eax;
    return pointerCall(app, cpu, at(app, Slot), a, {x86::reg32(words)...});
}

/* soundFrom for eacsnd's play3d, whose way out of a failure clears the record
 * at the caller's esi: `esi` what the game's code has in esi at the call, for
 * the typed function (through the pointer its seven words go). */
inline std::optional<x86::reg32> soundPlay3d(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 esp, x86::reg32 eax,
                                             x86::reg32 esi, x86::reg32 slot, x86::reg32 buffer, x86::reg32 looping,
                                             x86::reg32 rate, x86::reg32 volume, x86::reg32 positionA,
                                             x86::reg32 positionB)
{
    cpu.esp = esp;
#ifndef NFS3HP_SOUND_THROUGH_SLOTS
    if (app->thunkAt(at(app, isnd::kDirectPlay3d)) == SoundCallee<isnd::kDirectPlay3d>::registered)
        return soundResult(cpu, esp,
                           port::eacsnd::play3d(app, cpu, esi, slot, buffer, looping, rate, volume, positionA, positionB));
#else
    NFS2_USE(esi);
#endif
    x86::Args a;
    a.eax = eax;
    return pointerCall(app, cpu, at(app, isnd::kDirectPlay3d), a,
                       {slot, buffer, looping, rate, volume, positionA, positionB});
}

template <std::size_t... I>
inline std::optional<x86::reg32> soundPushedThrough(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 target,
                                                    std::index_sequence<I...>)
{
    x86::Args a;
    const x86::reg32 esp = cpu.esp;
    const auto result = pointerCall(app, cpu, target, a, {at(app, esp + 4 * x86::reg32(I))...});
    if (result)
        cpu.esp = esp + 4 * x86::reg32(sizeof...(I));
    return result;
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
 * call takes it (nothing is written there: the game's code does not write
 * it either), and the callee at Slot called -- its typed function with the
 * words where they lie, or through the pointer.  esp as the stdcall's ret
 * leaves it, past the words, unless the game stopped (false). */
template <x86::reg32 Slot, auto Fn>
inline bool soundPushed(win32::WinApplication* app, x86::CPU& cpu)
{
    static_assert(Fn == SoundCallee<Slot>::function, "the callee the pointer is to");
#ifndef NFS3HP_SOUND_THROUGH_SLOTS
    if (app->thunkAt(at(app, Slot)) == SoundCallee<Slot>::registered)
    {
        constexpr std::size_t kCount = SoundCallee<Slot>::count;
        const x86::reg32 esp = cpu.esp + 4 * x86::reg32(kCount);
        return bool(soundResult(cpu, esp, soundPushedTyped<Fn>(app, cpu, std::make_index_sequence<kCount>())));
    }
#endif
    return bool(soundPushedThrough(app, cpu, at(app, Slot), std::make_index_sequence<SoundCallee<Slot>::count>()));
}

}  // namespace guest
}  // namespace nfs3hp

#endif
