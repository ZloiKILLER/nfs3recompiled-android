#include "port_snd_mixer.h"
#include "port_snd_calls.h"

#include <cstddef>
#include <cstring>

/* EA's SND effects (cmn\seffect.c, cmn\sfxlevel.c, win\sdfxinit.c) and the
 * mixer's start (mix\smixer.c), rewritten (port_snd.h says how).  One effect
 * bus: its type, its two parameters and its master level at 0xa0a25c (0x10 a
 * bus); a voice sends to it at its own level (+0x39) scaled by the bus's.  The
 * reverb: 4 delay lines in 0xa0d2b4.. of prime lengths, their state at
 * 0xa181b0..0xa18200, their gains at 0x56ae3c; the mixer (port_snd_mixer.cpp)
 * runs it.  The mixer's start: its tables of functions (the MMX set), its
 * three buffers, its output for the card's format.
 *
 * Each is here as C++ with arguments and a result, and as the entry the game's
 * code calls by its address; the float arithmetic is the code's in double with
 * float stores (the port's x87 is double). */
namespace nfs3hp
{
namespace port
{

namespace
{

/* A bus: its effect type, its master level (0..127) and its two parameters. */
struct Bus
{
    int32_t type, level, first, second;
};
static_assert(sizeof(Bus) == 0x10, "SND's effect bus");

constexpr Records<Bus> kBuses{0xa0a25c};
constexpr Var<u32> kSetFx{0x9ef654};                // what eacsnd's start calls for the bus
constexpr Table<u32> kFxFunctions{0x9f6a60};        // the mixer's reverb, its send, its end
constexpr Var<uint8_t> kIntegerSend{0x9f6a5b};        // the reverb's send in 16 bits, as the MMX set has it
constexpr Var<uint8_t> kFrameShift{0x9f6a5a};         // the accumulator's frame shift (0: 16-bit stereo)
constexpr Var<u32> kOutputFunction{0x9f6b20};
constexpr Var<uint16_t> kBlockBytes{0x9f6a58};        // a 256-frame block's output, in bytes
constexpr Var<u32> kClearFunction{0x9f6a5c};        // cdecl (accumulator, dwords)
constexpr Var<u32> kAccumulator{0x9fbca8}, kDecoded{0x9fbcac}, kResampled{0x9fbcb0};
constexpr int32_t kChannelCount = 16;

// Where the reverb starts, and what runs it.
constexpr u32 kReverbStart = 0x5230b8, kReverbSend = 0x5230dc, kReverbEnd = 0x523118;

// The reverb's delay lines and what is kept for them: a line's four words
// each lie at 4 bytes a line from the base.
constexpr Table<u32> kLineStarts{0xa181d0};         // the line's first word
constexpr Table<u32> kReadAt{0xa181b0}, kStartAt{0xa181c0}, kStateAt{0xa181e0}, kEndAt{0xa181f0};
constexpr Table<u32> kWriteAt{0xa0d2a0};
constexpr Table<float> kGaps{0x56ae2c}, kGains{0x56ae3c};
constexpr Var<double> kLineGrowth{0x551b00};
constexpr u32 kFirstLine = 0xa0d2b4, kLinesBytes = 0xaf00;

constexpr Where kSetFxError{0x54eb00, 0x54eb10}, kFxLevelError{0x54eb4c, 0x54eb5c};
constexpr Where kInitBusError{0x54b41c, 0x54b42c}, kMasterError{0x54b41c, 0x54b540}, kReinitError{0x54e184, 0x54e194};

/* fistp after __CHP: `value` towards 0, the x87's "integer indefinite" past the
 * integer's range. */
int32_t truncated(double value)
{
    if (!(value > -2147483649.0 && value < 2147483648.0))
        return INT32_MIN;
    return int32_t(value);
}

/* The smallest prime from `n` on (2 for what is below it). */
int32_t primeFrom(int32_t n)
{
    if (n < 2)
        return 2;
    for (;; ++n)
    {
        bool prime = true;
        for (int32_t d = 2; int64_t(d) * d <= n; ++d)
            if (n % d == 0)
            {
                prime = false;
                break;
            }
        if (prime)
            return n;
    }
}

/* The effect bus's send of voice `index`: the voice's bytes at +0x38 (the
 * timbre's) and +0x39 (the level it sends at) for bus `bus`. */
int8_t& timbreSend(App* app, int32_t index, int32_t bus)
{
    return record<int8_t>(app, kVoices.address(index) + offsetof(Voice, timbreSend) + u32(bus));
}

int8_t& sendOf(App* app, int32_t index, int32_t bus)
{
    return record<int8_t>(app, kVoices.address(index) + offsetof(Voice, send) + u32(bus));
}

/* Voice `index`'s send to bus `bus` at `level` (0..127): the voice's timbre's
 * send times it times the bus's level, over 0x3f01, to the mixer.  The
 * mixer's answer. */
Result sendVoice(App* app, ThreadState& ts, u32 esp, int32_t index, int32_t bus, int32_t level, int32_t& scaled)
{
    sendOf(app, index, bus) = int8_t(level);
    scaled = idiv(imul(imul(timbreSend(app, index, bus), level), kBuses(app, bus).level), 0x3f01);
    return voiceSendLevel(app, ts, esp, index, scaled);
}

}  // namespace

/* 0x523200's: the smallest prime from `rate` times `seconds` (towards 0) on:
 * the length of a delay line. */
int32_t reverbLineLength(int32_t rate, float seconds)
{
    return primeFrom(truncated(double(rate) * double(seconds)));
}

/* 0x52325c's: the reverb started for `rate`: its delay lines cleared, each of
 * the 4 a prime length on from the last (the first 0.1 s long, each next the
 * last's times 0x551b00), its gain 1 less the one in 0x56ae2c on. */
void startReverb(App* app, int32_t rate)
{
    std::memset(&app->getMemory<uint8_t>(kFirstLine), 0, kLinesBytes);
    u32 line = kFirstLine;
    float seconds = 0.1f;
    for (int32_t i = 1; i <= 4; ++i)
    {
        kLineStarts.set(app, i, line);
        const int32_t length = reverbLineLength(rate, seconds);
        kGains.set(app, i, float(1.0 - double(kGaps(app, i))));
        const u32 start = kLineStarts(app, i);
        line += u32(length) * 4;
        const u32 last = u32(length) * 4 - 4 + start;
        kReadAt.set(app, i, start);
        kStartAt.set(app, i, start);
        kEndAt.set(app, i, last);
        kWriteAt.set(app, i, last);
        kStateAt.set(app, i, 0);
        seconds = float(double(seconds) * kLineGrowth(app));
    }
}

/* 0x502ff0's (iSNDwinsetfx): the bus's effect type `type` made the mixer's:
 * none (0), or one of the reverbs (5..120) -- the reverb started at the mixer's
 * rate, its functions the mixer's.  0; an error for another type. */
Result applyBusEffect(App* app, ThreadState& ts, u32 esp, u32 type)
{
    if (type == 0)
        return 0;
    bool known = false;
    for (const u32 t : {5u, 10u, 20u, 30u, 40u, 50u, 100u, 110u, 120u})
        known = known || type == t;
    if (!known)
        return fail(app, ts, esp - 3 * 4, kSetFxError, 0x24, {0x54eb20, type}, type);
    startReverb(app, int32_t(kMixRate(app)));
    kFxFunctions.set(app, 0, kReverbStart);
    kFxFunctions.set(app, 1, kReverbSend);
    kFxFunctions.set(app, 2, kReverbEnd);
    return 0;
}

/* 0x5030ac's: bus effect type `type` set where eacsnd's start finds it, and
 * now when the library runs.  0. */
Result setBusType(App* app, ThreadState& ts, u32 esp, u32 type)
{
    kSetFx.set(app, 0x502ff0);
    if (kInited(app) != 0 && !applyBusEffect(app, ts, esp - 4, type))
        return {};
    return 0;
}

/* 0x5030d0's (SNDfxlevel): the send to bus `bus` (0 only) of the sound
 * `handle` set to `level` (0..127): the voice's, or each playing voice's of
 * its group -- scaled by the bus's level.  What the voice's send gave back (0
 * with none); -10 before SNDinit, -8 for no sound.  A level an error let by is
 * the next group voice's too: the code keeps what it computed in the register
 * that held the level. */
Result setBusSend(App* app, ThreadState& ts, u32 esp, u32 handle, int32_t bus, int32_t level)
{
    if (kInited(app) == 0)
        return kNotStarted;
    const u32 frame = esp - 4 * 4 - 8;
    if (bus >= 1 || bus < 0)
        if (!fail(app, ts, frame, kFxLevelError, 0x3a, {0x54eb68, u32(bus)}, 0x3a))
            return {};
    if (level > 0x7f || level < 0)
        if (!fail(app, ts, frame, kFxLevelError, 0x3d, {0x54eb90, u32(level)}, 0x3d))
            return {};
    if (!enterAudio(app, ts, frame - 4))
        return {};
    const int32_t index = voiceOfHandle(app, handle);
    if (index < 0)
    {
        if (!leaveAudio(app, ts, frame - 4, u32(index)))
            return {};
        return kNoSound;
    }
    const uint8_t group = kVoices(app, index).group;
    int32_t scaled;
    Result sent = 0;
    u32 eax = u32(index);
    if (group == 0)
    {
        if (!(sent = sendVoice(app, ts, frame - 4, index, bus, level, scaled)))
            return {};
        eax = u32(*sent);
    }
    else
        for (int32_t i = 0; i < kVoiceCount; ++i)
        {
            const Voice& w = kVoices(app, i);
            if (w.group != group || w.state != 1 || w.handle < 0)
                continue;
            if (!(sent = sendVoice(app, ts, frame - 4, i, bus, level, scaled)))
                return {};
            level = scaled;
            eax = u32(*sent);
        }
    if (!leaveAudio(app, ts, frame - 4, eax))
        return {};
    return sent;
}

/* 0x4e9dfc's (SNDfxmasterlevel): bus `bus`'s master level set to `level`
 * (0..127), each voice's send set again with it.  0. */
Result setBusMaster(App* app, ThreadState& ts, u32 esp, int32_t bus, int32_t level)
{
    const u32 frame = esp - 5 * 4;
    if (bus < 0 || bus >= 1)
        if (!fail(app, ts, frame, kMasterError, 0xc2, {0x54b554, u32(bus)}, u32(bus)))
            return {};
    if (level < 0 || level > 0x7f)
        if (!fail(app, ts, frame, kMasterError, 0xc8, {0x54b57c, u32(level)}, 0x54b540))
            return {};
    kBuses(app, bus).level = level;
    // 0x5030c4's: nothing to do for the bus's level (it gives 0).
    for (int32_t i = 0; i < kVoiceCount; ++i)
        if (!setBusSend(app, ts, frame - 4, u32(kVoices(app, i).handle), bus, sendOf(app, i, bus)))
            return {};
    return 0;
}

/* 0x4e9c70's (SNDfxinitbus): bus `bus` (0 only) set to effect type `type` with
 * parameters `first` and `second` (-1 for 127), its master level `level` (0
 * for no effect).  What setBusMaster gave. */
Result initEffectBus(App* app, ThreadState& ts, u32 esp, int32_t bus, u32 type, int32_t first, int32_t second, int32_t level)
{
    const u32 frame = esp - 3 * 4;
    if (bus < 0 || bus >= 1)
        if (!fail(app, ts, frame, kInitBusError, 0x52, {0x54b43c, u32(bus)}, 0x54b42c))
            return {};
    if (first == -1)
        first = 0x7f;
    else if (first < 0 || first > 0x7f)
        if (!fail(app, ts, frame, kInitBusError, 0x5a, {0x54b468, u32(first)}, 0x5a))
            return {};
    if (second == -1)
        second = 0x7f;
    else if (second < 0 || second > 0x7f)
        if (!fail(app, ts, frame, kInitBusError, 0x62, {0x54b48c, u32(second)}, 0x54b42c))
            return {};
    Bus& b = kBuses(app, bus);
    b.type = int32_t(type);
    b.first = first;
    b.second = second;
    const Result set = setBusType(app, ts, frame - 4, type);
    if (!set)
        return {};
    if (kInited(app) == 0)
    {
        if (*set < 0 && !fail(app, ts, frame, kInitBusError, 0x7b, {0x54b4f8}, 0x7b))
            return {};
    }
    else if (*set < 0 && !fail(app, ts, frame, kInitBusError, 0x74, {0x54b4b0}, u32(*set)))
        return {};
    return setBusMaster(app, ts, frame - 4, bus, type == 0 ? 0 : level);
}

/* 0x500008's (iSNDmixreinit): the mixer's output function and its block's
 * bytes for the card's format (the low byte of `eax`: 8, 2, 4, 1 in that order
 * of preference).  0; an error for another. */
Result setMixerOutput(App* app, ThreadState& ts, u32 esp, u32 eax)
{
    const u32 format = eax & 0xff;
    struct Output
    {
        u32 function;
        uint16_t bytes;
    };
    Output output;
    if (format & 8)
        output = {0x9f6b18, 0x400};
    else if (format & 2)
        output = {0x9f6b10, 0x200};
    else if (format & 4)
        output = {0x9f6b14, 0x200};
    else if (format & 1)
        output = {0x9f6b0c, 0x100};
    else
    {
        if (!fail(app, ts, esp - 3 * 4, kReinitError, 0xbc, {0x54e1a4}, eax))
            return {};
        return 0;
    }
    kOutputFunction.set(app, app->getMemory<u32>(output.function));
    kBlockBytes.set(app, output.bytes);
    return 0;
}

/* 0x4ffd84's: the mixer's functions, the MMX set (the only one the port has):
 * its volume functions, resamplers, outputs, clear and emms; the send in 16
 * bits and the accumulator's frames unshifted.  The code's eax: the output
 * function. */
u32 pickMixerFunctions(App* app)
{
    struct Set
    {
        u32 at, value;
    };
    static const Set sets[] = {
        {0x9f6adc, 0x51f580}, {0x9f6ae0, 0x51f680}, {0x9f6ae4, 0x51f750}, {0x9f6ae8, 0x51f850},
        {0x9f6afc, 0x51fc38}, {0x9f6b00, 0x52002c}, {0x9f6b04, 0x5203a0}, {0x9f6aec, 0x51fc38},
        {0x9f6af0, 0x52002c}, {0x9f6af4, 0x5203a0}, {0x9f6b08, 0x5207e4}, {0x9f6af8, 0x5207e4},
        {0x9f6b0c, 0x520830}, {0x9f6b10, 0x520870}, {0x9f6b18, 0x520920}, {0x9f6a5c, 0x5209a0},
        {0x9f6b24, 0x520a00}, {0x9f6b14, 0x520a10}};
    kIntegerSend.set(app, 1);
    for (const Set& s : sets)
        store(app, s.at, s.value);
    kFrameShift.set(app, 0);
    return 0x520920;
}

/* 0x4fff18's (iSNDmixinit): the mixer started for the card's format `format`,
 * `ended` what it tells of a channel that ended: its tables, its 3 buffers
 * 8-aligned, its functions, its output; its channels free, its accumulator
 * cleared, the effect's start.  0. */
Result startMixer(App* app, ThreadState& ts, u32 esp, u32 format, u32 ended)
{
    const u32 frame = esp - 4 * 4;
    pickPcmFirstSetups(app);
    pickPcmSecondSetups(app);
    pickXaSetups(app);
    pickLastSetups(app);
    kAccumulator.set(app, 0x9f6b28);
    kDecoded.set(app, 0x9f7330);
    kResampled.set(app, 0x9fb498);
    for (const Var<u32>& buffer : {kAccumulator, kDecoded, kResampled})
        buffer.set(app, (buffer(app) + 7) & ~7u);
    pickMixerFunctions(app);
    kChannelEnded.set(app, ended);
    if (!setMixerOutput(app, ts, frame - 4, format))
        return {};
    for (int32_t i = 0; i < kChannelCount; ++i)
        kChannels(app, i).playing = 0;
    const u32 accumulator = kAccumulator(app);
    if (!pointerGame(app, ts, esp - 2 * 4, kClearFunction(app), {u32(kChannelCount) * kChannels.stride, 0, 0, accumulator},
                  {accumulator, 0x200}))
        return {};
    // The effect's start takes no argument; eax and edx are what the clearing left in them: the registers
    // as they are.
    if (const u32 start = kFxFunctions(app, 0); start != 0)
    {
        ts.sp = esp - 2 * 4;
        if (!guest::pointerCall(app, ts, start, {}).has_value())
            return {};
    }
    return 0;
}

/* The game's entries: each its thunk -- the game's registers and stack in,
 * the result in eax out -- around what it does. */

nfs::u32 reverbLength(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, u32(reverbLineLength(int32_t(e.eax), app->getMemory<float>(e.esp + 4))), 4);
}

nfs::u32 reverbInit(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    startReverb(app, int32_t(e.eax));
    return e.back(ts, args, 0);
}

nfs::u32 iSNDwinsetfx(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, applyBusEffect(app, ts, e.esp, e.eax));
}

nfs::u32 setBusFx(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, setBusType(app, ts, e.esp, e.edx));
}

nfs::u32 busLevelSet(App*, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, 0);
}

nfs::u32 SNDfxlevel(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, setBusSend(app, ts, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx)));
}

nfs::u32 SNDfxinitbus(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
             initEffectBus(app, ts, e.esp, int32_t(e.eax), e.ebx, int32_t(e.ecx), int32_t(e.arg(app, 0)), int32_t(e.edx)),
             4);
    return args.eax;
}

nfs::u32 SNDfxmasterlevel(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, setBusMaster(app, ts, e.esp, int32_t(e.eax), int32_t(e.edx)));
}

nfs::u32 iSNDmixreinit(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, setMixerOutput(app, ts, e.esp, e.eax));
}

nfs::u32 mixFunctions(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, pickMixerFunctions(app));
}

nfs::u32 iSNDmixinit(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, startMixer(app, ts, e.esp, e.eax, e.edx));
}

}  // namespace port
}  // namespace nfs3hp

