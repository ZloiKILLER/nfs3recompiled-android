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
 *     guest::sound<isnd::kDirectVol, &port::eacsnd::volume>(app, ts, buffer, volume)
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
nfs::u32 iSNDdirectcaps(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectstart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectstop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectrecordstart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectrecordpacket(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectrecordstop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectserve(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectcreate3dbuf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectremovebuf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectplay3d(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectpos3d(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectvol(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectrate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 iSNDdirectstopbuf(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);

/* The four of the game's functions eacsnd calls (native_game.cpp): the
 * natives where the game's code was, and each as C++ -- its words as its
 * parameters, what it leaves in eax, nullopt when the game stopped in it;
 * the registers left as the native leaves them. */
nfs::u32 AcquireSoundDataLambda(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 StopSoundBufferLambda(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 AcquireSoundBufferPositionLambda(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 TickSoundLambda(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);

std::optional<nfs::u32> acquireSoundDataOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer,
                                             nfs::u32 frames);
std::optional<nfs::u32> stopSoundBufferOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 buffer);
std::optional<nfs::u32> acquireSoundBufferPositionOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 first,
                                                       nfs::u32 second, nfs::u32 position);
std::optional<nfs::u32> tickSoundOf(nfs::App* app, nfs::ThreadState& ts);

namespace port
{

/* The library's 100 Hz tick (port_snd_voices.cpp, port_snd.h), the game's
 * 0x500c80, and its thunk: what TickSoundLambda's pointer holds unless the
 * game set another. */
nfs::u32 soundTick(App* app, ThreadState& ts, nfs::Args& args);

/* eacsnd's 14 exports the game keeps pointers to, as C++
 * (native_sound.cpp: caps, start, stop, serve; port_snd_isnd.cpp: the rest):
 * each one's words as its parameters, in their order (isnd.h), and its result
 * -- what eacsnd leaves in eax.  The iSNDdirect* thunks take the words off
 * the stack, call these and pop the words.
 *
 * Those that call the game's functions give nullopt when the game stopped in
 * one.  pos3d, play3d and stopbuf are eacsnd's code to the register
 * (port_snd_isnd.cpp): theirs put the words below esp, where a caller pushes
 * them, run on the stack from there and read the caller's registers from ts
 * (esi and the four eacsnd keeps), leaving them as eacsnd's code does.  A
 * way the port has not -- eacsnd with DirectSound -- asserts and gives -1, as
 * the exports do. */
namespace eacsnd
{

u32 caps(App* app, ThreadState& ts, u32 window);
u32 start(App* app, ThreadState& ts, u32 caps, u32 window);
u32 stop(App* app, ThreadState& ts);
/* Recording: none to start, nothing to hand over or stop. */
u32 recordStart(App* app, ThreadState& ts, u32, u32, u32, u32);
u32 recordPacket(App* app, ThreadState& ts, u32);
u32 recordStop(App* app, ThreadState& ts);
std::optional<u32> serve(App* app, ThreadState& ts);
u32 create3d(App* app, ThreadState& ts, u32 channels, u32 format, u32 rate, u32 length, u32 loopStart,
               u32 loopEnd, u32 data);
u32 remove3d(App* app, ThreadState& ts, u32 slot);
/* `esi` the caller's: the record eacsnd's way out of a failure clears. */
std::optional<u32> play3d(App* app, ThreadState& ts, u32 esi, u32 slot, u32 buffer, u32 looping, u32 rate,
                            u32 volume, u32 positionA, u32 positionB);
std::optional<u32> position(App* app, ThreadState& ts, u32 buffer, u32 positionA, u32 positionB);
u32 volume(App* app, ThreadState& ts, u32 buffer, u32 volume);
u32 rate(App* app, ThreadState& ts, u32 buffer, u32 rate);
std::optional<u32> stopBuffer(App* app, ThreadState& ts, u32 buffer);

}  // namespace eacsnd
}  // namespace port

namespace guest
{


/* Each pointer's callee: its typed function, its words and, in the game, its
 * thunk. */
template <nfs::u32 Slot>
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
        static constexpr nfs::ThunkPtr registered = &nfs3hp::thunk;           \
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
inline std::optional<nfs::u32> soundResult(nfs::ThreadState& ts, nfs::u32 esp, const R& result)
{
    nfs::u32 eax;
    if constexpr (std::is_same_v<R, std::optional<nfs::u32>>)
    {
        if (!result)
            return std::nullopt;
        eax = *result;
    }
    else
    {
        eax = result;
    }
    if (ts.terminate)
        return std::nullopt;
    ts.sp = esp;
    return eax;
}

/* The callee at Slot with `words`, from esp as ts has it: its typed function
 * or through the pointer (above). */
template <nfs::u32 Slot, auto Fn, typename... W>
inline std::optional<nfs::u32> sound(nfs::App* app, nfs::ThreadState& ts, W... words)
{
    static_assert(Fn == SoundCallee<Slot>::function, "the callee the pointer is to");
    static_assert(sizeof...(W) == SoundCallee<Slot>::count, "every word the callee takes");
    static_assert(isnd::exportAt(Slot).eacsnd == 0 || 4 * sizeof...(W) == isnd::exportAt(Slot).bytes,
                  "every word the export takes");
#ifndef NFS3HP_SOUND_THROUGH_SLOTS
    if (app->thunkAt(at(app, Slot)) == SoundCallee<Slot>::registered)
    {
        const nfs::u32 esp = ts.sp;
        return soundResult(ts, esp, Fn(app, ts, nfs::u32(words)...));
    }
#endif
    nfs::Args a;
    return pointerCall(app, ts, at(app, Slot), a, {nfs::u32(words)...});
}

/* The same from a rewrite's frame: esp first set to `esp`, where the game's
 * code has it at the call, and left so.  `eax` is what the game's code has in
 * eax there: the callee's typed function takes only its words, but through
 * the pointer (port.h's callGame) eax goes along. */
template <nfs::u32 Slot, auto Fn, typename... W>
inline std::optional<nfs::u32> soundFrom(nfs::App* app, nfs::ThreadState& ts, nfs::u32 esp, nfs::u32 eax,
                                           W... words)
{
    static_assert(Fn == SoundCallee<Slot>::function, "the callee the pointer is to");
    static_assert(sizeof...(W) == SoundCallee<Slot>::count, "every word the callee takes");
    ts.sp = esp;
#ifndef NFS3HP_SOUND_THROUGH_SLOTS
    if (app->thunkAt(at(app, Slot)) == SoundCallee<Slot>::registered)
        return soundResult(ts, esp, Fn(app, ts, nfs::u32(words)...));
#endif
    nfs::Args a;
    a.eax = eax;
    return pointerCall(app, ts, at(app, Slot), a, {nfs::u32(words)...});
}

/* soundFrom for eacsnd's play3d, whose way out of a failure clears the record
 * at the caller's esi: `esi` what the game's code has in esi at the call, for
 * the typed function (through the pointer its seven words go). */
inline std::optional<nfs::u32> soundPlay3d(nfs::App* app, nfs::ThreadState& ts, nfs::u32 esp, nfs::u32 eax,
                                             nfs::u32 esi, nfs::u32 slot, nfs::u32 buffer, nfs::u32 looping,
                                             nfs::u32 rate, nfs::u32 volume, nfs::u32 positionA,
                                             nfs::u32 positionB)
{
    ts.sp = esp;
#ifndef NFS3HP_SOUND_THROUGH_SLOTS
    if (app->thunkAt(at(app, isnd::kDirectPlay3d)) == SoundCallee<isnd::kDirectPlay3d>::registered)
        return soundResult(ts, esp,
                           port::eacsnd::play3d(app, ts, esi, slot, buffer, looping, rate, volume, positionA, positionB));
#else
    NFS2_USE(esi);
#endif
    nfs::Args a;
    a.eax = eax;
    return pointerCall(app, ts, at(app, isnd::kDirectPlay3d), a,
                       {slot, buffer, looping, rate, volume, positionA, positionB});
}

template <std::size_t... I>
inline std::optional<nfs::u32> soundPushedThrough(nfs::App* app, nfs::ThreadState& ts, nfs::u32 target,
                                                    std::index_sequence<I...>)
{
    nfs::Args a;
    const nfs::u32 esp = ts.sp;
    const auto result = pointerCall(app, ts, target, a, {at(app, esp + 4 * nfs::u32(I))...});
    if (result)
        ts.sp = esp + 4 * nfs::u32(sizeof...(I));
    return result;
}

#ifndef NFS3HP_SOUND_THROUGH_SLOTS
template <auto Fn, std::size_t... I>
inline auto soundPushedTyped(nfs::App* app, nfs::ThreadState& ts, std::index_sequence<I...>)
{
    return Fn(app, ts, at(app, ts.sp + 4 * nfs::u32(I))...);
}
#endif

/* A call from eacsnd's code to the register (port_snd_isnd.cpp), its words
 * already pushed and esp at the first: the return address's slot taken, as a
 * call takes it (nothing is written there: the game's code does not write
 * it either), and the callee at Slot called -- its typed function with the
 * words where they lie, or through the pointer.  esp as the stdcall's ret
 * leaves it, past the words, unless the game stopped (false). */
template <nfs::u32 Slot, auto Fn>
inline bool soundPushed(nfs::App* app, nfs::ThreadState& ts)
{
    static_assert(Fn == SoundCallee<Slot>::function, "the callee the pointer is to");
#ifndef NFS3HP_SOUND_THROUGH_SLOTS
    if (app->thunkAt(at(app, Slot)) == SoundCallee<Slot>::registered)
    {
        constexpr std::size_t kCount = SoundCallee<Slot>::count;
        const nfs::u32 esp = ts.sp + 4 * nfs::u32(kCount);
        return bool(soundResult(ts, esp, soundPushedTyped<Fn>(app, ts, std::make_index_sequence<kCount>())));
    }
#endif
    return bool(soundPushedThrough(app, ts, at(app, Slot), std::make_index_sequence<SoundCallee<Slot>::count>()));
}

}  // namespace guest
}  // namespace nfs3hp

#endif
