#include "port_snd_mixer.h"

#include <initializer_list>

/* EA's SND mixer's set-up, rewritten (port_snd.h says how): what gives a mixer
 * channel the functions the mix (port_snd_mixer.cpp) uses.  iSNDmixstart
 * (port_snd_play.cpp) calls, through the tables the mixer's start fills, a
 * decoder's set-up for the sound's format and way of playing, its volume's and
 * its resampler's; setMixStep calls the resampler's step setter, which picks
 * the resampler by the step.
 *
 * The port's CPU has MMX ([0x5643b4] set): the tables hold the MMX set.  Of it,
 * these are the set-ups of the decoders the mixer knows -- PCM looped, once and
 * streamed, XA once, XA with an intro, XA stereo streamed from memory and from
 * a file.  The others (8-bit PCM, the rest of the XA variants, the plain set)
 * are dead code (native_snd_dead.inc).
 *
 * A decoder's set-up, as the tables' entries take it (stdcall, 4 words): eax
 * the voice, edx the channel's decoder state, ebx and ecx the sound's data and
 * length (or its stream), [esp+4] and [esp+8] more of the same, [esp+0xc] where
 * the decoder's function goes, [esp+0x10] the channel's second function.  The
 * state's size is what they give back.  Of the registers the code leaves
 * changed, none is read by the one caller that counts (startChannel), nor by
 * the tables' entries in the game's code: they come back as they were. */
namespace nfs3hp
{
namespace port
{

namespace
{

constexpr Var<u32> kMmx{0x5643b4};
constexpr Table<u32> kDecoderSetups{0x9f6a6c};   // the 28 set-ups, by format and way
constexpr Table<float> kXaFloats{0xa0baa0};        // 13 shifts of 16 nibbles each
constexpr Table<int32_t> kXaIntegers{0xa0bea0};
constexpr Table<u32> kPacketPlayers{0xa0b044};   // 16 packet players (0: none)
constexpr Var<int32_t> kReverbIdle{0xa0d2a0};      // blocks since a channel last sent; the tail stops at 500
constexpr u32 kReverbSendBuffer = 0xa0caa0;      // 16-bit stereo, a block of it
constexpr Var<u32> kClearFunction{0x9f6a5c};     // cdecl (buffer, dwords)
constexpr u32 kPlainTables = 0x522100;           // the plain set's tables, first

void setWord(App* app, u32 at, u32 value) { app->getMemory<u32>(at) = value; }
void setHalf(App* app, u32 at, uint16_t value) { app->getMemory<uint16_t>(at) = value; }
void setByte(App* app, u32 at, uint8_t value) { app->getMemory<uint8_t>(at) = value; }
u32 word(App* app, u32 at) { return app->getMemory<u32>(at); }

/* A frame's bytes as a shift's count: the code halves 1, 2 and 4 (an
 * arithmetic shift). */
u32 frameShift(int32_t frameBytes) { return u32(frameBytes >> 1); }

/* The decoders' set-ups from index `first` on. */
void fillSetups(App* app, int32_t first, std::initializer_list<u32> setups)
{
    for (const u32 s : setups)
        kDecoderSetups.set(app, first++, s);
}

}  // namespace

/* 0x52c9a0's: PCM played once: its data, its length in frames, and
 * `frameBytes` a frame; the decoder it is played with and the channel's second
 * function put where they go.  The state's size, 0x10. */
u32 setUpPcmOnce(App* app, u32 state, u32 data, u32 length, int32_t frameBytes, u32 decoderSlot,
                   u32 secondSlot)
{
    setWord(app, state + 4, 0);
    setWord(app, state, data);
    setWord(app, state + 8, length);
    setWord(app, state + 0xc, frameShift(frameBytes));
    setWord(app, decoderSlot, 0x52c8f4);
    setWord(app, secondSlot, 0x52c8f0);
    return 0x10;
}

/* 0x52ca48's: PCM looped from `loopStart` to `loopEnd`.  0x14. */
u32 setUpPcmLooped(App* app, u32 state, u32 data, u32 loopStart, u32 loopEnd, int32_t frameBytes,
                     u32 decoderSlot)
{
    setWord(app, state + 4, 0);
    setWord(app, state, data);
    setWord(app, state + 8, loopStart);
    setWord(app, state + 0xc, loopEnd);
    setWord(app, state + 0x10, frameShift(frameBytes));
    setWord(app, decoderSlot, 0x52c9d0);
    return 0x14;
}

/* 0x52cb74's: PCM streamed, its chunks from the stream's refill.  0x10. */
u32 setUpPcmStreamed(App* app, u32 state, int32_t frameBytes, u32 decoderSlot)
{
    setWord(app, state, 1);
    setWord(app, state + 4, 0);
    setWord(app, state + 8, 0);
    setWord(app, state + 0xc, frameShift(frameBytes));
    setWord(app, decoderSlot, 0x52ca80);
    return 0x10;
}

/* 0x529104's: XA played once: its data and length.  0x54. */
u32 setUpXaOnce(App* app, u32 state, u32 data, u32 length, u32 decoderSlot, u32 secondSlot)
{
    setWord(app, state + 0x18, 0);
    setHalf(app, state + 0x1c, 0);
    setHalf(app, state + 6, 0);
    setHalf(app, state + 4, 0);
    setWord(app, state + 0x10, data);
    setWord(app, state + 0x14, length);
    setWord(app, decoderSlot, 0x528f74);
    setWord(app, secondSlot, 0x528f70);
    return 0x54;
}

/* 0x529534's: XA with an intro, then its loop (`loopStart`, `loopEnd`); the
 * decoder's place kept in the state.  0x9c. */
u32 setUpXaIntro(App* app, u32 state, u32 data, u32 length, u32 loopStart, u32 loopEnd,
                   u32 decoderSlot, u32 secondSlot)
{
    setWord(app, state + 0x18, 0);
    setHalf(app, state + 0x24, 0);
    setHalf(app, state + 6, 0);
    setHalf(app, state + 4, 0);
    setWord(app, state + 0x10, data);
    setWord(app, state + 0x14, length);
    setWord(app, state + 0x1c, loopStart);
    setWord(app, state + 0x20, loopEnd);
    setWord(app, state + 0x98, decoderSlot);
    setWord(app, decoderSlot, 0x529384);
    setWord(app, secondSlot, 0);
    return 0x9c;
}

/* The history a stereo XA decoder starts from, the same in both. */
void clearStereoXa(App* app, u32 state)
{
    setWord(app, state + 0x1c, 0);
    setWord(app, state + 0x20, 0);
    setHalf(app, state + 6, 0);
    setHalf(app, state + 4, 0);
    setHalf(app, state + 0xa, 0);
    setHalf(app, state + 8, 0);
}

/* 0x52a354's: stereo XA streamed from memory, from `stream`.  0x90. */
u32 setUpXaStereoStream(App* app, u32 state, u32 stream, u32 decoderSlot, u32 secondSlot)
{
    setWord(app, state + 0x14, 1);
    clearStereoXa(app, state);
    setWord(app, state + 0x18, stream);
    setWord(app, decoderSlot, 0x52a1a0);
    setWord(app, secondSlot, 0);
    return 0x90;
}

/* 0x52d160's: the packet player whose voice is voice `voice` (its index), -1
 * for none. */
int32_t packetPlayerOfVoice(App* app, u32 voice)
{
    for (int32_t i = 0; i < 0x10; ++i)
    {
        const u32 player = kPacketPlayers(app, i);
        if (player != 0 && u32(voiceOfHandle(app, word(app, player))) == voice)
            return i;
    }
    return -1;
}

/* 0x52a5b4's: stereo XA streamed from a file through the packet player of
 * voice `voice`, from `stream`.  0x98. */
u32 setUpXaStereoFile(App* app, u32 voice, u32 state, u32 stream, u32 decoderSlot, u32 secondSlot)
{
    const int32_t player = packetPlayerOfVoice(app, voice);
    setWord(app, state + 0x14, 0);
    clearStereoXa(app, state);
    setWord(app, state + 0x28, 0);
    setWord(app, state + 0x24, u32(player));
    setWord(app, state + 0x18, stream);
    setWord(app, decoderSlot, 0x52a3a0);
    setWord(app, secondSlot, 0);
    return 0x98;
}

/* 0x51f750's and 0x51f850's: a mono and a stereo source's volume functions
 * (MMX): the add's slot and the volume's slot given the functions, the state
 * keeping where they are. */
void setUpMonoVolume(App* app, u32 state, u32 addSlot, u32 volumeSlot)
{
    setWord(app, state + 0x10, addSlot);
    setWord(app, addSlot, 0x51f7ad);
    setWord(app, state + 0x14, volumeSlot);
    setWord(app, volumeSlot, 0x51f774);
}

void setUpStereoVolume(App* app, u32 state, u32 addSlot, u32 volumeSlot)
{
    setWord(app, state + 8, addSlot);
    setWord(app, addSlot, 0x51f897);
    setWord(app, state + 0xc, volumeSlot);
    setWord(app, volumeSlot, 0x51f874);
}

/* 0x5203a0's and 0x5207e4's: a mono and a stereo resampler (MMX): its position
 * and fraction at the start, its history empty, its step setter, frames-needed
 * and resampling slots kept, the step setter given its function. */
void setUpMonoResampler(App* app, u32 state, u32 stepSlot, u32 neededSlot, u32 resampleSlot)
{
    setWord(app, state, 0);
    setWord(app, state + 4, 0);
    setByte(app, state + 0x10, 0);
    setHalf(app, state + 0xe, 0);
    setByte(app, state + 0x11, 0);
    setByte(app, state + 0x12, 1);
    setWord(app, state + 0x14, stepSlot);
    setWord(app, state + 0x18, neededSlot);
    setWord(app, state + 0x1c, resampleSlot);
    setWord(app, word(app, state + 0x14), 0x5202fc);
}

void setUpStereoResampler(App* app, u32 state, u32 stepSlot, u32 neededSlot, u32 resampleSlot)
{
    setWord(app, state, 0);
    setWord(app, state + 4, 0);
    setByte(app, state + 0x14, 0);
    setByte(app, state + 0x15, 0);
    setByte(app, state + 0x16, 0);
    setWord(app, state + 0x18, stepSlot);
    setWord(app, state + 0x1c, neededSlot);
    setWord(app, state + 0x20, resampleSlot);
    setWord(app, word(app, state + 0x18), 0x520740);
}

namespace
{

/* A resampler's functions for a step under, at and over 1 (0x10000): the
 * frames-needed function and the resampler. */
struct StepFunctions
{
    u32 needed, resample;
};

/* The step setters: the step (16.16) kept, the history dropped when it crosses
 * 1 (or, from past 1, comes back to it or under); the resampler and its
 * frames-needed picked by the step against 1.  `flags` is where the history's
 * three flags are, `slots` where the two slots are. */
void setResamplerStep(App* app, u32 state, u32 step, u32 flags, u32 slots, const StepFunctions& under,
                      const StepFunctions& same, const StepFunctions& over)
{
    const u32 old = word(app, state + 8);
    const bool keep = (old < 0x10000 && step < 0x10000) || step == old || (old > 0x10000 && step > 0x10000);
    if (!keep)
    {
        setByte(app, state + flags, 0);
        setByte(app, state + flags + 1, 0);
        setByte(app, state + flags + 2, 0);
        setWord(app, state + 4, 0);
        setWord(app, state, 0);
    }
    setWord(app, state + 8, step);
    const StepFunctions& pick = step < 0x10000 ? under : step == 0x10000 ? same : over;
    setWord(app, word(app, state + slots), pick.needed);
    setWord(app, word(app, state + slots + 4), pick.resample);
}

}  // namespace

/* 0x5202fc's and 0x520740's: the mono and the stereo resampler by its step. */
void setMonoStep(App* app, u32 state, u32 step)
{
    setResamplerStep(app, state, step, 0x10, 0x18, {0x5200c0, 0x5201e4}, {0x5200b8, 0x5201d8}, {0x520070, 0x5200ec});
}

void setStereoStep(App* app, u32 state, u32 step)
{
    setResamplerStep(app, state, step, 0x14, 0x1c, {0x520440, 0x5205bc}, {0x520438, 0x5205b0}, {0x5203f0, 0x52046c});
}

/* 0x521d80's: the PCM decoders' first set-ups' table, by MMX (the plain set
 * starts with its own routine). */
Result pickPcmFirstSetups(App* app, ThreadState& ts, u32 esp)
{
    if (kMmx(app) != 0)
        fillSetups(app, 0, {0x5274b0, 0x5274d0, 0x527500, 0x527520, 0x527540, 0x527560, 0x527590, 0x5275b0});
    else
    {
        if (!pointerGame(app, ts, esp - 6 * 4, kPlainTables))
            return {};
        fillSetups(app, 0, {0x5275d0, 0x5275f0, 0x527620, 0x527640, 0x527660, 0x527680, 0x5276b0, 0x5276d0});
    }
    return 0;
}

/* 0x521e60's: the second table of the PCM set-ups. */
void pickPcmSecondSetups(App* app)
{
    if (kMmx(app) != 0)
        fillSetups(app, 8, {0x5276f0, 0x527710, 0x527740, 0x527760, 0x527780, 0x5277a0, 0x5277d0, 0x5277f0});
    else
        fillSetups(app, 8, {0x527810, 0x527840, 0x527870, 0x527890, 0x5278b0, 0x5278e0, 0x527910, 0x527930});
}

/* 0x521f30's: the XA float table: for each of 13 shifts, the 16 nibbles
 * (signed, at the top of a word) shifted down, as floats. */
void fillXaFloatTable(App* app)
{
    for (int32_t shift = 0; shift < 13; ++shift)
        for (int32_t k = 0; k < 16; ++k)
            kXaFloats.set(app, shift * 16 + k, float((int32_t(uint32_t(k) << 28) >> 16) >> shift));
}

/* 0x521f7c's: the same as integers, times 256. */
void fillXaIntegerTable(App* app)
{
    for (int32_t shift = 0; shift < 13; ++shift)
        for (int32_t k = 0; k < 16; ++k)
            kXaIntegers.set(app, shift * 16 + k, ((int32_t(uint32_t(k) << 28) >> 16) >> shift) * 256);
}

/* 0x521fc0's: the XA decoders' set-ups' table, its tables first. */
void pickXaSetups(App* app)
{
    fillXaFloatTable(app);
    if (kMmx(app) != 0)
    {
        fillXaIntegerTable(app);
        fillSetups(app, 16, {0x529104, 0x529534, 0x529778, 0x5299f4, 0x529c2c, 0x52a144, 0x52a354, 0x52a5b4});
    }
    else
        fillSetups(app, 16, {0x527ae8, 0x527f20, 0x52813c, 0x5283a4, 0x5285b8, 0x528a98, 0x528cac, 0x528f0c});
}

/* 0x5220a0's: the last of the decoders' set-ups' tables. */
void pickLastSetups(App* app)
{
    if (kMmx(app) != 0)
    {
        kDecoderSetups.set(app, 24, 0x52a790);
        kDecoderSetups.set(app, 26, 0x52a970);
        kDecoderSetups.set(app, 27, 0x52ab70);
    }
    else
    {
        kDecoderSetups.set(app, 24, 0x52ad28);
        kDecoderSetups.set(app, 26, 0x52af30);
        kDecoderSetups.set(app, 27, 0x52b160);
    }
}

/* 0x5230b8's: the reverb's start: its send cleared, its tail run out (500).
 * What the clearing gives back; `eax` is what the code has in it, which the
 * clearing is called with. */
Result startReverbTail(App* app, ThreadState& ts, u32 esp, u32 eax)
{
    kReverbIdle.set(app, 0x1f4);
    if (!pointerGame(app, ts, esp - 2 * 4, kClearFunction(app), {eax, 0x1f4}, {kReverbSendBuffer, 0x200}, eax))
        return {};
    return int32_t(eax);
}

/* The game's entries: each its thunk -- the game's registers and stack in,
 * the result in eax out -- around what it does. */

namespace
{

/* The tables' entries for PCM once, looped and streamed, `frameBytes` a frame:
 * a set-up's registers and stack as the comment above says. */
nfs::u32 pcmOnceEntry(App* app, ThreadState& ts, nfs::Args& args, int32_t frameBytes)
{
    const Entry e(ts, args);
    return e.back(ts, args, setUpPcmOnce(app, e.edx, e.ebx, e.ecx, frameBytes, e.arg(app, 2), e.arg(app, 3)), 0x10);
}

nfs::u32 pcmLoopedEntry(App* app, ThreadState& ts, nfs::Args& args, int32_t frameBytes)
{
    const Entry e(ts, args);
    setWord(app, e.arg(app, 3), 0);
    return e.back(ts, args, setUpPcmLooped(app, e.edx, e.ebx, e.arg(app, 0), e.arg(app, 1), frameBytes, e.arg(app, 2)), 0x10);
}

nfs::u32 pcmStreamedEntry(App* app, ThreadState& ts, nfs::Args& args, int32_t frameBytes)
{
    const Entry e(ts, args);
    setWord(app, e.arg(app, 3), 0);
    return e.back(ts, args, setUpPcmStreamed(app, e.edx, frameBytes, e.arg(app, 2)), 0x10);
}

}  // namespace

/* The decoder set-ups' tables' entries, MMX set: 16-bit PCM mono (1 byte a
 * frame, as the code counts), stereo (2) for 0x9f6a6c.., and for 0x9f6a8c..
 * stereo (2) and four bytes a frame (4). */
nfs::u32 pcmOnceMono(App* app, ThreadState& ts, nfs::Args& args) { return pcmOnceEntry(app, ts, args, 1); }
nfs::u32 pcmOnceStereo(App* app, ThreadState& ts, nfs::Args& args) { return pcmOnceEntry(app, ts, args, 2); }
nfs::u32 pcmOnceStereo2(App* app, ThreadState& ts, nfs::Args& args) { return pcmOnceEntry(app, ts, args, 2); }
nfs::u32 pcmOnceQuad(App* app, ThreadState& ts, nfs::Args& args) { return pcmOnceEntry(app, ts, args, 4); }
nfs::u32 pcmLoopedMono(App* app, ThreadState& ts, nfs::Args& args) { return pcmLoopedEntry(app, ts, args, 1); }
nfs::u32 pcmLoopedStereo(App* app, ThreadState& ts, nfs::Args& args) { return pcmLoopedEntry(app, ts, args, 2); }
nfs::u32 pcmLoopedStereo2(App* app, ThreadState& ts, nfs::Args& args) { return pcmLoopedEntry(app, ts, args, 2); }
nfs::u32 pcmLoopedQuad(App* app, ThreadState& ts, nfs::Args& args) { return pcmLoopedEntry(app, ts, args, 4); }
nfs::u32 pcmStreamedMono(App* app, ThreadState& ts, nfs::Args& args) { return pcmStreamedEntry(app, ts, args, 1); }
nfs::u32 pcmStreamedStereo(App* app, ThreadState& ts, nfs::Args& args) { return pcmStreamedEntry(app, ts, args, 2); }
nfs::u32 pcmStreamedStereo2(App* app, ThreadState& ts, nfs::Args& args) { return pcmStreamedEntry(app, ts, args, 2); }
nfs::u32 pcmStreamedQuad(App* app, ThreadState& ts, nfs::Args& args) { return pcmStreamedEntry(app, ts, args, 4); }

/* The PCM set-ups themselves, as the entries call them: the registers and 2
 * words, 2 words, the registers. */
nfs::u32 pcmOnceSetup(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, setUpPcmOnce(app, e.eax, e.edx, e.ebx, int32_t(e.ecx), e.arg(app, 0), e.arg(app, 1)), 8);
}

nfs::u32 pcmLoopedSetup(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, setUpPcmLooped(app, e.eax, e.edx, e.ebx, e.ecx, int32_t(e.arg(app, 0)), e.arg(app, 1)), 8);
}

nfs::u32 pcmStreamedSetup(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, setUpPcmStreamed(app, e.eax, int32_t(e.edx), e.ebx));
}

nfs::u32 xaOnceSetup(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, setUpXaOnce(app, e.edx, e.ebx, e.ecx, e.arg(app, 2), e.arg(app, 3)), 0x10);
}

nfs::u32 xaIntroSetup(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args,
           setUpXaIntro(app, e.edx, e.ebx, e.ecx, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)),
           0x10);
    return args.eax;
}

nfs::u32 xaStereoStreamSetup(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, setUpXaStereoStream(app, e.edx, e.ecx, e.arg(app, 2), e.arg(app, 3)), 0x10);
}

nfs::u32 packetPlayerOf(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(packetPlayerOfVoice(app, e.eax)));
}

nfs::u32 xaStereoFileSetup(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, setUpXaStereoFile(app, e.eax, e.edx, e.ecx, e.arg(app, 2), e.arg(app, 3)), 0x10);
}

/* The volume functions' and the resamplers' set-ups are cdecl: their
 * arguments on the stack, eax as it was (the volumes') or 0 (the resamplers'). */
nfs::u32 volumeMonoSetup(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    setUpMonoVolume(app, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2));
    return e.back(ts, args, e.eax);
}

nfs::u32 volumeStereoSetup(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    setUpStereoVolume(app, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2));
    return e.back(ts, args, e.eax);
}

nfs::u32 resampleMonoSetup(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    setUpMonoResampler(app, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3));
    return e.back(ts, args, 0);
}

nfs::u32 resampleStereoSetup(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    setUpStereoResampler(app, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3));
    return e.back(ts, args, 0);
}

nfs::u32 resampleMonoStep(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    setMonoStep(app, e.arg(app, 0), e.arg(app, 1));
    return e.back(ts, args, 0);
}

nfs::u32 resampleStereoStep(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    setStereoStep(app, e.arg(app, 0), e.arg(app, 1));
    return e.back(ts, args, 0);
}

nfs::u32 pcmTablesFirst(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, pickPcmFirstSetups(app, ts, e.esp));
}

nfs::u32 pcmTablesSecond(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    pickPcmSecondSetups(app);
    return e.back(ts, args, 0);
}

nfs::u32 xaFloatTable(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    fillXaFloatTable(app);
    return e.back(ts, args, 0x340);
}

nfs::u32 xaIntTable(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    fillXaIntegerTable(app);
    return e.back(ts, args, 0x340);
}

nfs::u32 xaTables(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    pickXaSetups(app);
    return e.back(ts, args, 0);
}

nfs::u32 lastTables(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    pickLastSetups(app);
    return e.back(ts, args, 0);
}

nfs::u32 reverbStart(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, startReverbTail(app, ts, e.esp, e.eax));
}

}  // namespace port
}  // namespace nfs3hp
