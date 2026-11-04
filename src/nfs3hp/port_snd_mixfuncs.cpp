#include "port_snd.h"

#include <cstring>

#pragma clang fp contract(off)

/* EA SND's mixer's own functions, rewritten (port.h says how): what the MMX
 * set's tables and set-ups hand a channel -- its volume set-up, its
 * resampler's state, step and frames needed -- and what a channel's decoder
 * is: PCM played once, looped, streamed from memory or from a file; XA ADPCM,
 * mono and stereo, in all those ways and with an intro before its loop; the
 * frame-coded sound (a bit reader, pulses, the synthesis filter); and the
 * conversions of a sample to floats and back.  native_mixer.cpp mixes with
 * most of the resamplers itself; these are the entries the game's code and the
 * library still call through the channels' pointers.
 *
 * Each is here twice, as in port_snd.h: as C++ with arguments and a result,
 * and as the entry the game's code knows (an adapter named as it was).  The
 * records stay where the game has them -- a channel's resampler, a decoder's
 * state, the bit reader -- and are read and written in place, in the order
 * the code has it: the MMX decoders (mix::xaMono, mix::interpolateMono8 ...)
 * run on the same memory.  Only the x87 is the host's: the floats the coded
 * sound is made of are doubles that are stored as floats where the code
 * stores them (the x87 at the 53 bits the port runs it at), and the sums
 * are done in the order its code has them.
 *
 * What none of the game's callers reads of the registers is not left: the
 * decoders and set-ups give back eax and leave every other register as the
 * caller had it.  The mixer calls these through its pointers (native_mixer.cpp
 * takes only eax), the library's start-up through callGame; the direct callers
 * of the conversions (0x4e1b6a.. in iSNDconvertsample) use the registers it
 * keeps across them, which the conversions leave as they were (the code pushes
 * all of them). */
namespace nfs3hp
{
namespace port
{

namespace
{

// What the code calls: EA's sine and cosine, memset, the packet players, the MMX decoders.
constexpr reg32 kSineCosine = 0x4eac60;  // eax angle, edx and ebx where the two go
constexpr reg32 kFill = 0x4e0640;        // memset: eax where, edx the byte, ebx how many
constexpr reg32 kPacketPlayerOf = 0x52d160;
constexpr reg32 kNextPacket = 0x5118fc;          // eax the player, edx where the packet's length goes
constexpr reg32 kPacketFramesPlayed = 0x511998;  // eax the player, edx the frames taken
constexpr reg32 kXaMono = 0x52d1a0;    // mix::xaMono (cdecl: the state)
constexpr reg32 kXaStereo = 0x52d250;  // mix::xaStereo (cdecl: the state)
constexpr reg32 kInterpolateMono8 = 0x5270f0, kInterpolateStereo8 = 0x527170;
constexpr reg32 kInterpolateMono16 = 0x527210, kInterpolateStereo16 = 0x527290;
constexpr Var<reg32> kRefill{0x5678b4};  // what a streamed sound asks its next chunk of
constexpr Var<reg32> kMmxEnd{0x9f6b24};  // the mixer's emms

// The memory as the code reads and writes it.
uint32_t word(App* app, reg32 at) { return load<uint32_t>(app, at); }
void setWord(App* app, reg32 at, uint32_t v) { store<uint32_t>(app, at, v); }
uint16_t half(App* app, reg32 at) { return load<uint16_t>(app, at); }
void setHalf(App* app, reg32 at, uint16_t v) { store<uint16_t>(app, at, v); }
int32_t signedHalf(App* app, reg32 at) { return load<int16_t>(app, at); }
uint8_t byteAt(App* app, reg32 at) { return load<uint8_t>(app, at); }
void setByte(App* app, reg32 at, uint8_t v) { store<uint8_t>(app, at, v); }
int32_t signedByte(App* app, reg32 at) { return load<int8_t>(app, at); }
int32_t signedWord(App* app, reg32 at) { return load<int32_t>(app, at); }
double singleAt(App* app, reg32 at) { return double(load<float>(app, at)); }
double doubleAt(App* app, reg32 at) { return load<double>(app, at); }
void setSingle(App* app, reg32 at, double v) { store<float>(app, at, float(v)); }

/* A float held as the bits the game keeps it in. */
float floatOf(uint32_t bits)
{
    float f;
    std::memcpy(&f, &bits, 4);
    return f;
}
uint32_t bitsOf(float f)
{
    uint32_t bits;
    std::memcpy(&bits, &f, 4);
    return bits;
}

/* `bytes` copied from `from` to `to` as rep movsd and rep movsb do it:
 * forward, dwords and then bytes (where they overlap, forward is what is
 * meant). */
void copyForward(App* app, reg32 to, reg32 from, uint32_t bytes)
{
    uint8_t* d = &app->getMemory<uint8_t>(to);
    const uint8_t* s = &app->getMemory<uint8_t>(from);
    if (uint64_t(to) + bytes <= from || uint64_t(from) + bytes <= to)
    {
        std::memcpy(d, s, bytes);
        return;
    }
    uint32_t i = 0;
    for (uint32_t n = bytes >> 2; n != 0; --n, i += 4)
    {
        uint32_t w;
        std::memcpy(&w, s + i, 4);
        std::memcpy(d + i, &w, 4);
    }
    for (uint32_t n = bytes & 3; n != 0; --n, ++i)
        d[i] = s[i];
}

/* memset by the game's fill (0x4e0640): the byte 0, how many. */
bool clearBytes(App* app, CPU& cpu, reg32 esp, reg32 where, uint32_t bytes)
{
    return callGame(app, cpu, esp, kFill, {where, 0, bytes});
}

/* Halves written to `at`, moving on: a decoder's output. */
struct Output
{
    App* app;
    reg32 at;
    void put(uint16_t v)
    {
        setHalf(app, at, v);
        at += 2;
    }
};

#pragma pack(push, 1)

/* A channel's resampler (kChannelResampler): the position in the source --
 * the frame, the fraction of the next, 16.16 -- and the step, then the frames
 * of the last block that the next one is put after, in the layout each of the
 * four formats has, and (8 bit) the slots of the functions the step selects. */
struct Resampler8Mono
{
    uint32_t frame, fraction, step;
    uint8_t earlierFrame;   // the frame before the kept one
    uint8_t keptFrame;      // the last block's last frame
    uint8_t hasEarlier;
    uint8_t hasKept;
    int8_t leftover;        // frames the last block took beyond the step's
    uint8_t other11[3];
    uint32_t stepSlot, neededSlot, resamplerSlot;
};
static_assert(sizeof(Resampler8Mono) == 0x20, "a resampler's state");

struct Resampler8Stereo
{
    uint32_t frame, fraction, step;
    uint8_t earlier[2];     // left, right
    uint8_t kept[2];
    uint8_t hasEarlier;
    uint8_t hasKept;
    int8_t leftover;
    uint8_t other13;
    uint32_t stepSlot, neededSlot, resamplerSlot;
};
static_assert(sizeof(Resampler8Stereo) == 0x20, "a resampler's state");

struct Resampler16Mono
{
    uint32_t frame, fraction, step;
    uint16_t earlierFrame;
    uint16_t keptFrame;
    uint8_t hasEarlier;
    uint8_t hasKept;
    int8_t leftover;
};

struct Resampler16Stereo
{
    uint32_t frame, fraction, step;
    uint16_t earlier[2];
    uint16_t kept[2];
    uint8_t hasEarlier;
    uint8_t hasKept;
    int8_t leftover;
};

#pragma pack(pop)

// Where each format keeps its flags, for the frames needed.
struct Flags
{
    reg32 earlier, kept, leftover;
};
constexpr Flags kMono8Flags{0xe, 0xf, 0x10}, kStereo8Flags{0x10, 0x11, 0x12}, kStereo16Flags{0x14, 0x15, 0x16};

// The functions the step selects for each format: frames needed and resampler, down / same / up.
struct StepFunctions
{
    Flags flags;
    reg32 neededDown, neededSame, neededUp;
    reg32 down, same, up;
};
constexpr StepFunctions kMono8Steps{kMono8Flags, 0x51f930, 0x51f978, 0x51f980, 0x51f9ac, 0x51fa80, 0x51fa8c};
constexpr StepFunctions kStereo8Steps{kStereo8Flags, 0x51fc80, 0x51fcc8, 0x51fcd0, 0x51fcfc, 0x51fe24, 0x51fe30};

/* The volume as the MMX multiplies by it: the level in the top byte and,
 * as a word, the level times 256. */
uint32_t volumeLevel(uint32_t v) { return (v << 24) | ((v << 8) & 0xffff); }

/* A mono source's volume setter: the add's slot and the volume's slot given
 * the MMX set's functions (0x51f5dd and 0x51f5a4). */
void hookMonoVolume(App* app, reg32 state, reg32 addSlot, reg32 volumeSlot)
{
    setWord(app, state + 0x10, addSlot);
    setWord(app, addSlot, 0x51f5dd);
    setWord(app, state + 0x14, volumeSlot);
    setWord(app, volumeSlot, 0x51f5a4);
}

/* A mono source's volume: left twice, then right twice. */
void setMonoLevels(App* app, reg32 state, uint32_t left, uint32_t right)
{
    setWord(app, state, volumeLevel(left));
    setWord(app, state + 4, volumeLevel(left));
    setWord(app, state + 8, volumeLevel(right));
    setWord(app, state + 0xc, volumeLevel(right));
}

void hookStereoVolume(App* app, reg32 state, reg32 addSlot, reg32 volumeSlot)
{
    setWord(app, state + 8, addSlot);
    setWord(app, addSlot, 0x51f6c7);
    setWord(app, state + 0xc, volumeSlot);
    setWord(app, volumeSlot, 0x51f6a4);
}

/* A stereo source's volume: right's level over left's word, twice. */
void setStereoLevels(App* app, reg32 state, uint32_t left, uint32_t right)
{
    const uint32_t v = (right << 24) | ((left << 8) & 0xffff);
    setWord(app, state, v);
    setWord(app, state + 4, v);
}

/* A pan's two sides from `angle` (64ths of EA's sine table's steps): the sine
 * and cosine of angle >> 6 (0x4eac60 leaves them on the stack), each moved by
 * the rest (angle & 63) towards the other.  The two values; `esp` is the
 * esp at entry (the code pushes three registers and keeps 8 bytes). */
Result panOffsets(App* app, CPU& cpu, reg32 esp, uint32_t angle, int32_t& first, int32_t& second)
{
    const reg32 f = esp - 3 * 4 - 8;
    const uint32_t rest = angle & 0x3f;
    if (!callGame(app, cpu, f, kSineCosine, {uint32_t(int32_t(angle) >> 6), f, f + 4, rest}))
        return std::nullopt;
    const int32_t sine = signedWord(app, f), cosine = signedWord(app, f + 4);
    const int32_t step = imul(int32_t(rest), 0x6487e) >> 9;
    first = int32_t(uint32_t(sine) + uint32_t(imul(cosine >> 2, step) >> 0x15));
    second = int32_t(uint32_t(cosine) - uint32_t(imul(step, sine >> 2) >> 0x15));
    return second;
}

/* A voice's place for DirectSound 3D, as three floats at `to`: x, y and z
 * from the angle `angle` around and the elevation (EA's sine table's
 * elevation, 0x400 up at a turn's quarter, and the pan's sides).  `esp` is the
 * esp at entry (two pushed registers, 0x18 bytes kept).  eax as the code
 * leaves it: the pan's second side. */
Result voiceBufferPosition(App* app, CPU& cpu, reg32 esp, uint32_t angle, uint32_t elevation, reg32 to)
{
    const reg32 f = esp - 2 * 4 - 0x18;
    const uint32_t lift = 0x400 - uint32_t(int32_t(elevation & 0xffff) >> 6);
    if (!callGame(app, cpu, f, kSineCosine, {lift, f + 0xc, f + 8, to}))
        return std::nullopt;
    const int32_t cosElevation = signedWord(app, f + 8), sinElevation = signedWord(app, f + 0xc);
    int32_t first, second;
    // The call at 0x51f520, from the esp the code has here.
    if (!panOffsets(app, cpu, f - 4, angle, first, second))
        return std::nullopt;
    const double c1 = doubleAt(app, 0x54e174), c2 = doubleAt(app, 0x54e17c);
    setSingle(app, to, (double(cosElevation) * double(first)) * c1);
    setSingle(app, to + 4, double(sinElevation) * c2);
    setSingle(app, to + 8, c1 * (double(cosElevation) * double(second)));
    return second;
}

/* An 8-bit mono resampler set up: its position at the start, no frames kept,
 * its step setter's slot given the function that sets the step. */
void setupResampler8Mono(App* app, reg32 s, reg32 stepSlot, reg32 neededSlot, reg32 resamplerSlot)
{
    auto& st = record<Resampler8Mono>(app, s);
    st.frame = 0;
    st.fraction = 0;
    st.hasEarlier = 0;
    st.keptFrame = 0;
    st.hasKept = 0;
    st.leftover = 1;
    st.stepSlot = stepSlot;
    st.neededSlot = neededSlot;
    st.resamplerSlot = resamplerSlot;
    setWord(app, st.stepSlot, 0x51fb94);
}

void setupResampler8Stereo(App* app, reg32 s, reg32 stepSlot, reg32 neededSlot, reg32 resamplerSlot)
{
    auto& st = record<Resampler8Stereo>(app, s);
    st.frame = 0;
    st.fraction = 0;
    st.hasEarlier = 0;
    st.hasKept = 0;
    st.leftover = 0;
    st.stepSlot = stepSlot;
    st.neededSlot = neededSlot;
    st.resamplerSlot = resamplerSlot;
    setWord(app, st.stepSlot, 0x51ff88);
}

/* A resampler's step set: across 1.0 (or from one) the frames it keeps are
 * dropped and the position starts afresh; the functions that count the frames
 * needed and resample are the step's -- over 1.0 (down), 1.0 (same) or under
 * (up). */
void setResamplerStep(App* app, reg32 s, uint32_t step, const StepFunctions& fn)
{
    const uint32_t old = word(app, s + 8);
    if ((old >= 0x10000 || step >= 0x10000) && step != old && !(old > 0x10000 && step > 0x10000))
    {
        setByte(app, s + fn.flags.earlier, 0);
        setByte(app, s + fn.flags.kept, 0);
        setByte(app, s + fn.flags.leftover, 0);
        setWord(app, s + 4, 0);
        setWord(app, s, 0);
    }
    setWord(app, s + 8, step);
    reg32 needed = fn.neededDown, resampler = fn.down;
    if (step < 0x10000)
        needed = fn.neededUp, resampler = fn.up;
    else if (step == 0x10000)
        needed = fn.neededSame, resampler = fn.same;
    setWord(app, word(app, s + 0x18), needed);
    setWord(app, word(app, s + 0x1c), resampler);
}

/* The source frames `frames` take at a step over 1.0, the leftover the last
 * block had counted in and what this one leaves kept. */
uint32_t framesNeededDown(App* app, reg32 s, uint32_t frames, const Flags& flags)
{
    const uint32_t used = (((frames - 1) * word(app, s + 8) + word(app, s + 4)) >> 16) + 1;
    uint32_t needed = used + uint32_t(signedByte(app, s + flags.leftover));
    if (byteAt(app, s + flags.kept) == 0)
        ++needed;
    setByte(app, s + flags.leftover, uint8_t(((word(app, s + 8) * frames + word(app, s + 4)) >> 16) - used));
    return needed;
}

/* At a step under 1.0: less the frame kept before. */
uint32_t framesNeededUp(App* app, reg32 s, uint32_t frames, const Flags& flags)
{
    uint32_t needed = (((frames - 1) * word(app, s + 8) + word(app, s + 4)) >> 16) + 1;
    if (byteAt(app, s + flags.kept) == 0)
        ++needed;
    return needed - uint32_t(signedByte(app, s + flags.earlier));
}

/* At the step 1.0 the source itself is the target: a block resampled is the
 * source, as it is; `target` is where the pointer to the block goes. */
void useSourceAsBlock(App* app, reg32 source, reg32 targetSlot) { setWord(app, targetSlot, source); }

/* 0x5270f0 .. 0x527290, the MMX interpolations of a block's frames but the
 * last: the position kept as 16.16 in a cell of the stack (`f`) and back.
 * The registers are the code's: the callee takes the count, the source and
 * the target in edi, esi and ebp. */
bool interpolateBlock(App* app, CPU& cpu, reg32 f, reg32 s, reg32 function, uint32_t count, reg32 source,
                      reg32 target)
{
    const uint32_t step = word(app, s + 8);
    const reg32 whole = step >> 16, fraction = step << 16;
    setWord(app, f, word(app, s + 4) << 16);
    cpu.ebp = target;
    cpu.esi = source;
    cpu.edi = count;
    if (!callGame(app, cpu, f, function, {f, fraction, s, whole}, {count, source, target, s, f, whole, fraction}))
        return false;
    setWord(app, s + 4, word(app, f) >> 16);
    return true;
}

/* A block's last frame, one sample between source samples a and b by the
 * fraction `fraction`: (65535 - f) * a and f * b, each to its high half. */
uint32_t lastSample8(int32_t a, int32_t b, uint32_t fraction)
{
    const uint32_t first = uint32_t(imul(a * 0x100, int32_t(0xffff - fraction))) >> 16;
    const uint32_t second = uint32_t(imul(b * 0x100, int32_t(fraction))) >> 16;
    return first + second;
}

/* An 8-bit mono block resampled down (a step over 1.0): the frame kept from
 * the last block put in front, all but the last frame by the MMX
 * interpolation, the last here; the next block's start and what it leaves
 * kept.  `esp` is the esp at entry (four pushed registers, 4 bytes kept). */
Result resampleDown8Mono(App* app, CPU& cpu, reg32 esp, reg32 s, uint32_t count, reg32 source, reg32 targetSlot)
{
    auto& st = record<Resampler8Mono>(app, s);
    const reg32 f = esp - 4 * 4 - 4;
    if (st.hasKept != 0)
    {
        --source;
        setByte(app, source, st.keptFrame);
    }
    const reg32 target = word(app, targetSlot);
    if (--count != 0 && !interpolateBlock(app, cpu, f, s, kInterpolateMono8, count, source, target))
        return std::nullopt;
    const uint32_t i = st.frame, fraction = st.fraction;
    const uint32_t last = lastSample8(signedByte(app, source + i), signedByte(app, source + i + 1), fraction);
    setByte(app, target + count, uint8_t(int32_t(last) >> 8));
    st.keptFrame = byteAt(app, source + st.frame + 1);
    const uint32_t position = st.fraction + st.step;
    st.fraction = position & 0xffff;
    st.frame += position >> 16;
    const int32_t eax = st.leftover;
    st.hasKept = 1;
    st.frame = uint32_t(eax);
    return eax;
}

/* The same under 1.0: the frames kept from the last two blocks first, and
 * whether the next block needs this one's last frame kept. */
Result resampleUp8Mono(App* app, CPU& cpu, reg32 esp, reg32 s, uint32_t count, reg32 source, reg32 targetSlot)
{
    auto& st = record<Resampler8Mono>(app, s);
    const reg32 f = esp - 4 * 4 - 4;
    uint32_t eax = s;
    if (st.hasKept != 0)
    {
        --source;
        setByte(app, source, st.keptFrame);
        eax = (eax & 0xffffff00) | st.keptFrame;
    }
    if (st.hasEarlier != 0)
    {
        --source;
        setByte(app, source, st.earlierFrame);
        eax = (eax & 0xffffff00) | st.earlierFrame;
    }
    const reg32 target = word(app, targetSlot);
    st.frame = 0;
    if (--count != 0 && !interpolateBlock(app, cpu, f, s, kInterpolateMono8, count, source, target))
        return std::nullopt;
    const uint32_t i = st.frame, fraction = st.fraction;
    const uint32_t last = lastSample8(signedByte(app, source + i), signedByte(app, source + i + 1), fraction);
    setByte(app, target + count, uint8_t(int32_t(last) >> 8));
    st.keptFrame = byteAt(app, source + st.frame + 1);
    const uint32_t position = st.fraction + st.step;
    const uint32_t step = st.step;
    st.fraction = position & 0xffff;
    st.frame += position >> 16;
    eax = st.fraction;
    if (eax < step)
        st.hasEarlier = 0;
    else
    {
        const uint8_t lastFrame = byteAt(app, source + st.frame);
        st.hasEarlier = 1;
        eax = (eax & 0xffffff00) | lastFrame;
        st.earlierFrame = lastFrame;
    }
    st.hasKept = 1;
    return int32_t(eax);
}

/* An 8-bit stereo block's last frame at `last`: each side between source
 * frames [s] and [s]+1 by the fraction at +4; the next frame's two bytes
 * kept. */
void lastFrame8Stereo(App* app, Resampler8Stereo& st, reg32 source, reg32 last)
{
    const uint32_t i = st.frame, fraction = st.fraction;
    setByte(app, last,
            uint8_t(int32_t(lastSample8(signedByte(app, source + i * 2), signedByte(app, source + i * 2 + 2),
                                        fraction)) >> 8));
    setByte(app, last + 1,
            uint8_t(int32_t(lastSample8(signedByte(app, source + i * 2 + 1), signedByte(app, source + i * 2 + 3),
                                        fraction)) >> 8));
    st.kept[0] = byteAt(app, source + i * 2 + 2);
    st.kept[1] = byteAt(app, source + i * 2 + 3);
}

/* An 8-bit stereo block resampled down, as 8-bit mono (four pushed registers,
 * 8 bytes kept). */
Result resampleDown8Stereo(App* app, CPU& cpu, reg32 esp, reg32 s, uint32_t count, reg32 source, reg32 targetSlot)
{
    auto& st = record<Resampler8Stereo>(app, s);
    const reg32 f = esp - 4 * 4 - 8;
    if (st.hasKept != 0)
    {
        const uint8_t right = st.kept[1], left = st.kept[0];
        setByte(app, --source, right);
        setByte(app, --source, left);
    }
    const reg32 target = word(app, targetSlot);
    if (--count != 0 && !interpolateBlock(app, cpu, f, s, kInterpolateStereo8, count, source, target))
        return std::nullopt;
    lastFrame8Stereo(app, st, source, target + count * 2);
    const uint32_t position = st.fraction + st.step;
    st.fraction = position & 0xffff;
    st.frame += position >> 16;
    const int32_t eax = st.leftover;
    st.hasKept = 1;
    st.frame = uint32_t(eax);
    return eax;
}

Result resampleUp8Stereo(App* app, CPU& cpu, reg32 esp, reg32 s, uint32_t count, reg32 source, reg32 targetSlot)
{
    auto& st = record<Resampler8Stereo>(app, s);
    const reg32 f = esp - 4 * 4 - 4;
    uint32_t eax = s;
    if (st.hasKept != 0)
    {
        const uint8_t right = st.kept[1], left = st.kept[0];
        setByte(app, --source, right);
        setByte(app, --source, left);
        eax = (eax & 0xffffff00) | left;
    }
    if (st.hasEarlier != 0)
    {
        const uint8_t right = st.earlier[1], left = st.earlier[0];
        setByte(app, --source, right);
        setByte(app, --source, left);
        eax = (eax & 0xffffff00) | left;
    }
    const reg32 target = word(app, targetSlot);
    st.frame = 0;
    if (--count != 0 && !interpolateBlock(app, cpu, f, s, kInterpolateStereo8, count, source, target))
        return std::nullopt;
    lastFrame8Stereo(app, st, source, target + count * 2);
    const uint32_t position = st.fraction + st.step;
    const uint32_t step = st.step;
    st.fraction = position & 0xffff;
    st.frame += position >> 16;
    eax = st.fraction;
    if (eax < step)
        st.hasEarlier = 0;
    else
    {
        st.hasEarlier = 1;
        st.earlier[0] = byteAt(app, source + st.frame * 2);
        const uint8_t right = byteAt(app, source + st.frame * 2 + 1);
        eax = (st.frame & 0xffffff00) | right;
        st.earlier[1] = right;
    }
    st.hasKept = 1;
    return int32_t(eax);
}

/* A 16-bit mono block's last frame at `last`: the first sample's share
 * written, then the second's added to it; the next frame kept. */
void lastFrame16Mono(App* app, Resampler16Mono& st, reg32 source, reg32 last)
{
    const uint32_t i = st.frame, fraction = st.fraction;
    setHalf(app, last, uint16_t(uint32_t(imul(signedHalf(app, source + i * 2), int32_t(0xffff - fraction))) >> 16));
    const uint32_t second = uint32_t(imul(int32_t(fraction), signedHalf(app, source + i * 2 + 2))) >> 16;
    setHalf(app, last, uint16_t(uint32_t(int32_t(int16_t(half(app, last)))) + second));
    st.keptFrame = half(app, source + st.frame * 2 + 2);
}

/* A 16-bit mono block resampled down, as 8-bit mono (four pushed registers,
 * 4 bytes kept). */
Result resampleDown16Mono(App* app, CPU& cpu, reg32 esp, reg32 s, uint32_t count, reg32 source, reg32 targetSlot)
{
    auto& st = record<Resampler16Mono>(app, s);
    const reg32 f = esp - 4 * 4 - 4;
    if (st.hasKept != 0)
    {
        source -= 2;
        setHalf(app, source, st.keptFrame);
    }
    const reg32 target = word(app, targetSlot);
    if (--count != 0 && !interpolateBlock(app, cpu, f, s, kInterpolateMono16, count, source, target))
        return std::nullopt;
    lastFrame16Mono(app, st, source, target + count * 2);
    const uint32_t position = st.fraction + st.step;
    st.fraction = position & 0xffff;
    st.frame += position >> 16;
    const int32_t eax = st.leftover;
    st.hasKept = 1;
    st.frame = uint32_t(eax);
    return eax;
}

Result resampleUp16Mono(App* app, CPU& cpu, reg32 esp, reg32 s, uint32_t count, reg32 source, reg32 targetSlot)
{
    auto& st = record<Resampler16Mono>(app, s);
    const reg32 f = esp - 4 * 4 - 4;
    uint32_t eax = s;
    if (st.hasKept != 0)
    {
        source -= 2;
        setHalf(app, source, st.keptFrame);
        eax = (eax & 0xffff0000) | st.keptFrame;
    }
    if (st.hasEarlier != 0)
    {
        source -= 2;
        setHalf(app, source, st.earlierFrame);
        eax = (eax & 0xffff0000) | st.earlierFrame;
    }
    const reg32 target = word(app, targetSlot);
    st.frame = 0;
    if (--count != 0 && !interpolateBlock(app, cpu, f, s, kInterpolateMono16, count, source, target))
        return std::nullopt;
    lastFrame16Mono(app, st, source, target + count * 2);
    const uint32_t position = st.fraction + st.step;
    const uint32_t step = st.step;
    st.fraction = position & 0xffff;
    st.frame += position >> 16;
    eax = st.fraction;
    if (eax < step)
        st.hasEarlier = 0;
    else
    {
        st.hasEarlier = 1;
        const uint16_t lastFrame = half(app, source + st.frame * 2);
        eax = (st.frame & 0xffff0000) | lastFrame;
        st.earlierFrame = lastFrame;
    }
    st.hasKept = 1;
    return int32_t(eax);
}

/* A 16-bit stereo block's last frame at `last`: each side's first share
 * written, then its second's added; the next frame kept. */
void lastFrame16Stereo(App* app, Resampler16Stereo& st, reg32 source, reg32 last)
{
    const uint32_t i = st.frame, fraction = st.fraction;
    const reg32 frame = source + i * 4;
    setHalf(app, last, uint16_t(uint32_t(imul(signedHalf(app, frame), int32_t(0xffff - fraction))) >> 16));
    uint32_t second = uint32_t(imul(signedHalf(app, frame + 4), int32_t(fraction))) >> 16;
    setHalf(app, last, uint16_t(uint32_t(int32_t(int16_t(half(app, last)))) + second));
    setHalf(app, last + 2, uint16_t(uint32_t(imul(signedHalf(app, frame + 2), int32_t(0xffff - fraction))) >> 16));
    second = uint32_t(imul(signedHalf(app, frame + 6), int32_t(fraction))) >> 16;
    setHalf(app, last + 2, uint16_t(uint32_t(int32_t(int16_t(half(app, last + 2)))) + second));
    st.kept[0] = half(app, frame + 4);
    st.kept[1] = half(app, frame + 6);
}

/* What a 16-bit stereo block keeps in front of it: the last block's frame
 * when it has one.  The source moved back. */
reg32 putKeptFrames16Stereo(App* app, const Resampler16Stereo& st, reg32 source)
{
    if (st.hasKept != 0)
    {
        source -= 2;
        setHalf(app, source, st.kept[1]);
        source -= 2;
        setHalf(app, source, st.kept[0]);
    }
    return source;
}

Result resampleDown16Stereo(App* app, CPU& cpu, reg32 esp, reg32 s, uint32_t count, reg32 source,
                            reg32 targetSlot)
{
    auto& st = record<Resampler16Stereo>(app, s);
    const reg32 f = esp - 4 * 4 - 4;
    source = putKeptFrames16Stereo(app, st, source);
    const reg32 target = word(app, targetSlot);
    if (--count != 0 && !interpolateBlock(app, cpu, f, s, kInterpolateStereo16, count, source, target))
        return std::nullopt;
    lastFrame16Stereo(app, st, source, target + count * 4);
    const uint32_t position = st.fraction + st.step;
    st.fraction = position & 0xffff;
    st.frame += position >> 16;
    const int32_t eax = st.leftover;
    st.hasKept = 1;
    st.frame = uint32_t(eax);
    return eax;
}

Result resampleUp16Stereo(App* app, CPU& cpu, reg32 esp, reg32 s, uint32_t count, reg32 source, reg32 targetSlot)
{
    auto& st = record<Resampler16Stereo>(app, s);
    const reg32 f = esp - 4 * 4 - 4;
    source = putKeptFrames16Stereo(app, st, source);
    if (st.hasEarlier != 0)
    {
        source -= 2;
        setHalf(app, source, st.earlier[1]);
        source -= 2;
        setHalf(app, source, st.earlier[0]);
    }
    const reg32 target = word(app, targetSlot);
    st.frame = 0;
    if (--count != 0 && !interpolateBlock(app, cpu, f, s, kInterpolateStereo16, count, source, target))
        return std::nullopt;
    lastFrame16Stereo(app, st, source, target + count * 4);
    const uint32_t position = st.fraction + st.step;
    const uint32_t step = st.step;
    st.fraction = position & 0xffff;
    st.frame += position >> 16;
    uint32_t eax = st.fraction;
    if (eax < step)
        st.hasEarlier = 0;
    else
    {
        st.hasEarlier = 1;
        st.earlier[0] = half(app, source + st.frame * 4);
        const uint16_t right = half(app, source + st.frame * 4 + 2);
        eax = (st.frame & 0xffff0000) | right;
        st.earlier[1] = right;
    }
    st.hasKept = 1;
    return int32_t(eax);
}

/* ---------------------------------------------------------------- PCM */

#pragma pack(push, 1)
struct PcmOnce
{
    uint32_t data, frame, length;
    uint8_t shift;  // a frame is 1 << shift bytes
};
struct PcmLooped
{
    uint32_t data, frame, loopStart, last;
    uint8_t shift;
};
struct PcmStreamed
{
    uint32_t data, chunkFrames, frame;
    uint8_t shift;
};
struct PcmFile
{
    uint32_t data, chunkFrames, frame, stream;
    int16_t shift;
    uint16_t taken;  // frames taken from the stream since the last release
};
#pragma pack(pop)

/* PCM played once: its frames copied, those past its end silence.  1; -1 at
 * its end.  `esp` is the esp at entry (four pushed registers, 4 bytes kept). */
Result playPcmOnce(App* app, CPU& cpu, reg32 esp, reg32 s, uint32_t wanted, reg32 target)
{
    auto& st = record<PcmOnce>(app, s);
    const uint32_t frame = st.frame, length = st.length;
    if (frame >= length)
        return -1;
    const uint8_t shift = st.shift & 0x1f;
    st.frame = frame + wanted;
    if (frame + wanted < length)
        copyForward(app, target, st.data + (frame << shift), wanted << shift);
    else
    {
        const uint32_t left = length - frame;
        copyForward(app, target, st.data + (frame << shift), left << shift);
        if (!clearBytes(app, cpu, esp - 4 * 4 - 4, (left << shift) + target, (wanted - left) << shift))
            return std::nullopt;
    }
    return 1;
}

/* PCM looped: copied a stretch at a time, back to the loop's start past the
 * end.  1. */
int32_t playPcmLooped(App* app, reg32 s, int32_t wanted, reg32 target)
{
    auto& st = record<PcmLooped>(app, s);
    while (wanted > 0)
    {
        uint32_t n = st.last - st.frame + 1;
        if (wanted < int32_t(n))
            n = uint32_t(wanted);
        const uint8_t shift = st.shift & 0x1f;
        copyForward(app, target, st.data + (st.frame << shift), n << shift);
        wanted -= int32_t(n);
        const uint32_t frame = st.frame + n;
        target += n << shift;
        st.frame = frame;
        if (frame > st.last)
            st.frame = st.loopStart;
    }
    return 1;
}

/* PCM streamed from memory: each chunk copied, the next asked of the refill
 * (eax the state, edx where its length goes); none, and the rest is silence.
 * 1; -1 with no data.  (four pushed registers, 0xc bytes kept) */
Result playPcmStreamed(App* app, CPU& cpu, reg32 esp, reg32 s, int32_t wanted, reg32 target)
{
    auto& st = record<PcmStreamed>(app, s);
    const reg32 f = esp - 4 * 4 - 0xc;
    if (st.data == 0)
        return -1;
    while (wanted > 0)
    {
        if (int32_t(st.frame) >= int32_t(st.chunkFrames))
        {
            reg32 more;
            if (!callGame(app, cpu, f, kRefill(app), {s, s + 4}, {}, more))
                return std::nullopt;
            if (more != 0 || st.data == 0)
            {
                if (!clearBytes(app, cpu, f, target, uint32_t(wanted) << (st.shift & 0x1f)))
                    return std::nullopt;
                st.frame = 0;
                st.chunkFrames = 0;
                return 1;
            }
            st.frame = more;
        }
        uint32_t n = st.chunkFrames - st.frame;
        if (int32_t(n) > wanted)
            n = uint32_t(wanted);
        const uint8_t shift = st.shift & 0x1f;
        copyForward(app, target, st.data + (st.frame << shift), n << shift);
        st.frame += n;
        target += n << shift;
        wanted -= int32_t(n);
    }
    return 1;
}

/* PCM streamed from a file: the frames taken last time given back, each chunk
 * copied, the next asked of the stream's packet player (it leaves the chunk's
 * length in the stack cell it is given); none, and the rest is silence.  The
 * frames taken this time.  (four pushed registers, 0xc bytes kept) */
Result playPcmFromFile(App* app, CPU& cpu, reg32 esp, reg32 s, int32_t wanted, reg32 target)
{
    auto& st = record<PcmFile>(app, s);
    const reg32 f = esp - 4 * 4 - 0xc;
    if (st.taken != 0)
    {
        if (!callGame(app, cpu, f, kPacketFramesPlayed, {st.stream, st.taken}))
            return std::nullopt;
        st.taken = 0;
    }
    while (wanted > 0)
    {
        if (int32_t(st.frame) >= int32_t(st.chunkFrames))
        {
            reg32 packet;
            if (!callGame(app, cpu, f, kNextPacket, {st.stream, f}, {}, packet))
                return std::nullopt;
            st.data = packet;
            if (packet == 0)
            {
                if (st.taken != 0 && !clearBytes(app, cpu, f, target, uint32_t(wanted) << (st.shift & 0x1f)))
                    return std::nullopt;
                break;
            }
            st.frame = 0;
            st.chunkFrames = word(app, f);
        }
        uint32_t n = st.chunkFrames - st.frame;
        if (int32_t(n) > wanted)
            n = uint32_t(wanted);
        const uint32_t shift = uint32_t(int32_t(st.shift)) & 0x1f;
        copyForward(app, target, st.data + (st.frame << shift), n << shift);
        wanted -= int32_t(n);
        st.frame += n;
        st.taken = uint16_t(st.taken + n);
        target += n << shift;
    }
    return st.taken;
}

/* PCM streamed from a file set up, after its packet player (0x52d160): a
 * frame of `frameBytes` bytes; the decoder's slot given the decoder.  0x14,
 * the state's size. */
Result setupPcmFile(App* app, CPU& cpu, reg32 esp, reg32 stream, int32_t frameBytes, reg32 decoderSlot, reg32 s)
{
    reg32 player;
    if (!callGame(app, cpu, esp, kPacketPlayerOf, {stream}, {}, player))
        return std::nullopt;
    auto& st = record<PcmFile>(app, s);
    st.data = 0;
    st.chunkFrames = 0xffffffff;
    st.frame = 0;
    st.taken = 0;
    st.stream = player;
    st.shift = int16_t(frameBytes >> 1);
    setWord(app, decoderSlot, 0x52cba0);
    return 0x14;
}

/* ------------------------------------------------------------ XA, mono */

/* The XA decoders' block's place: frame * 15 / 28 (15 bytes a block of 28
 * frames), cdq; idiv as the code has it. */
uint32_t xaMonoBlockBytes(uint32_t frame) { return uint32_t(idiv(imul(int32_t(frame), 15), 0x1c)); }

/* The stereo decoders': frame * 30 / 28 (30 bytes a block of 28 frames). */
uint32_t xaStereoBlockBytes(uint32_t frame) { return uint32_t(idiv(int32_t(frame * 30), 0x1c)); }

/* The frames a decoder kept from its last block, `there` of them from
 * `from`: as many as are wanted copied out to the output (the pointer in
 * the word at `outSlot`) and the rest moved to the front.  How many were
 * copied. */
int32_t copyKeptMono(App* app, int32_t there, reg32 from, int32_t wanted, reg32 outSlot)
{
    Output out{app, word(app, outSlot)};
    int32_t copied = wanted;
    if (there < copied)
        copied = there;
    int32_t i = 0;
    for (; i < copied; ++i)
        out.put(half(app, from + 2 * i));
    const int32_t start = i;
    for (; i < there; ++i)
        setHalf(app, from + 2 * (i - start), half(app, from + 2 * i));
    setWord(app, outSlot, out.at);
    return copied;
}

/* The same for stereo -- a frame is two halves. */
int32_t copyKeptStereo(App* app, int32_t there, reg32 from, int32_t wanted, reg32 outSlot)
{
    Output out{app, word(app, outSlot)};
    int32_t copied = wanted;
    if (there < copied)
        copied = there;
    int32_t i = 0;
    for (; i < copied; ++i)
    {
        out.put(half(app, from + 4 * i));
        out.put(half(app, from + 4 * i + 2));
    }
    const int32_t start = i;
    for (; i < there; ++i)
    {
        setHalf(app, from + 4 * (i - start), half(app, from + 4 * i));
        setHalf(app, from + 4 * (i - start) + 2, half(app, from + 4 * i + 2));
    }
    setWord(app, outSlot, out.at);
    return copied;
}

// The frames a decoder keeps, as halves from `base`; `index` signed, as the code adds it.
reg32 keptAt(reg32 base, int32_t index) { return base + reg32(index) * 2; }

bool decodeXaMono(App* app, CPU& cpu, reg32 esp, reg32 s) { return callGame(app, cpu, esp, kXaMono, {}, {s}); }
bool decodeXaStereo(App* app, CPU& cpu, reg32 esp, reg32 s) { return callGame(app, cpu, esp, kXaStereo, {}, {s}); }

/* Mono XA played once: its state has the data at +0x10, the length at +0x14,
 * the frame at +0x18, the frames kept at +0x1c and the kept samples from
 * +0x1e.  The kept frames first, then whole blocks by mix::xaMono, what a
 * block leaves over kept; past the end, silence.  1; -1 at its end.  (four
 * pushed registers) */
Result playXaMonoOnce(App* app, CPU& cpu, reg32 esp, reg32 s, uint32_t wanted, reg32 target)
{
    if (signedWord(app, s + 0x18) >= signedWord(app, s + 0x14))
        return -1;
    Output out{app, target};
    const int32_t held = signedHalf(app, s + 0x1c);
    if (int32_t(wanted) <= held)
    {
        int32_t i = 0;
        for (; i < int32_t(wanted); ++i)
            out.put(half(app, s + 0x1e + 2 * i));
        uint32_t left = 0;
        for (; i < signedHalf(app, s + 0x1c); ++i, ++left)
            setHalf(app, s + 0x1e + 2 * left, half(app, s + 0x1e + 2 * i));
        setHalf(app, s + 0x1c, uint16_t(left));
        setWord(app, s + 0xc, out.at);
        return 1;
    }
    for (int32_t i = 0; i < held; ++i)
        out.put(half(app, s + 0x1e + 2 * i));
    const uint32_t rest = wanted - uint32_t(held);
    int32_t left = signedWord(app, s + 0x14) - signedWord(app, s + 0x18);
    if (left < 0)
        left = 0;
    const uint32_t n = int32_t(rest) >= left ? uint32_t(left) : rest;
    setWord(app, s + 0xc, out.at);
    setWord(app, s, n);
    setWord(app, s + 8, word(app, s + 0x10) + xaMonoBlockBytes(word(app, s + 0x18)));
    if (!decodeXaMono(app, cpu, esp - 4 * 4, s))
        return std::nullopt;
    setWord(app, s + 0x18, word(app, s + 0x18) + n);
    setWord(app, s + 0x18, word(app, s + 0x18) - word(app, s));
    out.at = word(app, s + 0xc);
    if (left == int32_t(n))
    {
        int32_t silence = int32_t(rest - n - (word(app, s + 0x18) - word(app, s + 0x14)));
        for (; silence > 0; --silence)
            out.put(0);
        setWord(app, s + 0xc, out.at);
    }
    else
    {
        const int32_t over = signedWord(app, s);
        uint32_t kept = 0;
        for (int32_t k = over; k < 0; ++k, ++kept)
            setHalf(app, s + 0x1e + 2 * kept, half(app, out.at + 2 * reg32(k)));
        setHalf(app, s + 0x1c, uint16_t(kept));
    }
    return 1;
}

/* Mono XA looped: the state as above but the loop's start at +0x1c, the
 * length at +0x20, the frames kept at +0x24 and from +0x26, and the decoder's
 * history at the loop at +0x94 and +0x96.  The kept frames first, then whole
 * blocks; the block across the end decoded into the kept frames, and the
 * loop's start (its block decoded on the stack, the frames from the start
 * kept) after it.  1.  (four pushed registers, 0x44 bytes kept) */
Result playXaMonoLooped(App* app, CPU& cpu, reg32 esp, reg32 s, int32_t wanted, reg32 target)
{
    const reg32 f = esp - 4 * 4 - 0x44;
    setWord(app, s + 0xc, target);
    if (wanted > 0)
    {
        do
        {
            if (half(app, s + 0x24) != 0)
            {
                const int32_t copied = copyKeptMono(app, signedHalf(app, s + 0x24), s + 0x26, wanted, s + 0xc);
                setHalf(app, s + 0x24, uint16_t(half(app, s + 0x24) - copied));
                wanted -= copied;
            }
            else if (signedWord(app, s + 0x18) + 0x1b < signedWord(app, s + 0x20))
            {
                setWord(app, s + 8, word(app, s + 0x10) + xaMonoBlockBytes(word(app, s + 0x18)));
                setWord(app, s, 0x1c);
                if (!decodeXaMono(app, cpu, f, s))
                    return std::nullopt;
                setWord(app, s + 0x18, word(app, s + 0x18) + 0x1c);
                wanted -= 0x1c;
            }
            else
            {
                // The block across the end, into the kept frames.
                const reg32 saved = word(app, s + 0xc);
                setWord(app, s + 8, word(app, s + 0x10) + xaMonoBlockBytes(word(app, s + 0x18)));
                setWord(app, s, 0x1c);
                setWord(app, s + 0xc, s + 0x26);
                if (!decodeXaMono(app, cpu, f, s))
                    return std::nullopt;
                setWord(app, s + 0x18, word(app, s + 0x18) + 0x1c);
                const uint16_t overshoot = uint16_t(half(app, s + 0x18) - half(app, s + 0x20));
                setHalf(app, s + 0x24, uint16_t(0x1c - overshoot + 1));
                // Then the decoder where the loop starts: its history, its block.
                setHalf(app, s + 6, half(app, s + 0x94));
                setHalf(app, s + 4, half(app, s + 0x96));
                setWord(app, s + 0x18, uint32_t(imul(idiv(int32_t(word(app, s + 0x1c)), 0x1c), 0x1c)));
                if (int32_t(word(app, s + 0x1c)) % 0x1c != 0)
                {
                    setWord(app, s, 0x1c);
                    setWord(app, s + 8, word(app, s + 0x10) + xaMonoBlockBytes(word(app, s + 0x18)));
                    setWord(app, s + 0xc, f);
                    if (!decodeXaMono(app, cpu, f, s))
                        return std::nullopt;
                    const uint32_t frame = word(app, s + 0x18) + 0x1c;
                    const uint32_t from = frame - word(app, s + 0x1c);
                    setWord(app, s + 0x18, frame);
                    reg32 sample = f + 2 * (0x1c - from);
                    for (int32_t j = 0; int32_t(from) > 0 && j < int32_t(from); ++j, sample += 2)
                    {
                        setHalf(app, keptAt(s + 0x26, signedHalf(app, s + 0x24)), half(app, sample));
                        setHalf(app, s + 0x24, uint16_t(half(app, s + 0x24) + 1));
                    }
                }
                setWord(app, s + 0xc, saved);
            }
        } while (wanted > 0);
    }
    setWord(app, s + 0xc, word(app, s + 0xc) + uint32_t(wanted) * 2);
    for (; wanted < 0; ++wanted)
    {
        setHalf(app, keptAt(s + 0x26, signedHalf(app, s + 0x24)), half(app, word(app, s + 0xc)));
        setWord(app, s + 0xc, word(app, s + 0xc) + 2);
        setHalf(app, s + 0x24, uint16_t(half(app, s + 0x24) + 1));
    }
    return 1;
}

/* Mono XA with an intro: the state as the loop's.  The kept frames first,
 * then the intro's blocks up to the loop's start, what the last leaves over
 * kept; at the loop the decoder's slot (+0x98) given the loop decoder, its
 * history saved for it, and the rest of the frames asked of it.  1.  (four
 * pushed registers, 8 bytes kept) */
Result playXaMonoIntro(App* app, CPU& cpu, reg32 esp, reg32 s, int32_t wanted, reg32 target)
{
    const reg32 f = esp - 4 * 4 - 8;
    Output out{app, target};
    const int32_t held = signedHalf(app, s + 0x24);
    if (held >= wanted)
    {
        int32_t i = 0;
        for (; i < wanted; ++i)
            out.put(half(app, s + 0x26 + 2 * i));
        uint32_t left = 0;
        for (; i < signedHalf(app, s + 0x24); ++i, ++left)
            setHalf(app, s + 0x26 + 2 * left, half(app, s + 0x26 + 2 * i));
        setHalf(app, s + 0x24, uint16_t(left));
        setWord(app, s + 0xc, out.at);
        return 1;
    }
    for (int32_t i = 0; i < held; ++i)
        out.put(half(app, s + 0x26 + 2 * i));
    setWord(app, s + 0xc, out.at);
    int32_t rest = wanted - held;
    int32_t n = signedWord(app, s + 0x1c) - signedWord(app, s + 0x18) - 0x1b;
    if (n > rest)
        n = rest;
    if (n < 0)
        n = 0;
    setWord(app, s + 8, word(app, s + 0x10) + xaMonoBlockBytes(word(app, s + 0x18)));
    setWord(app, s, uint32_t(n));
    if (!decodeXaMono(app, cpu, f, s))
        return std::nullopt;
    setWord(app, s + 0x18, word(app, s + 0x18) + uint32_t(n) - word(app, s));
    const int32_t over = signedWord(app, s);
    const reg32 from = word(app, s + 0xc);
    uint32_t kept = 0;
    for (int32_t k = over; k < 0; ++k, ++kept)
        setHalf(app, s + 0x26 + 2 * kept, half(app, from + 2 * reg32(k)));
    rest -= n;
    setHalf(app, s + 0x24, uint16_t(kept));
    setWord(app, s + 0xc, word(app, s + 0xc) - 2 * kept);
    if (int32_t(word(app, s + 0x1c) - word(app, s + 0x18)) < 0x1c)
    {
        setHalf(app, s + 0x94, half(app, s + 6));
        setHalf(app, s + 0x96, half(app, s + 4));
        setWord(app, word(app, s + 0x98), 0x5291a8);
        if (rest > 0 && !playXaMonoLooped(app, cpu, f - 4, s, rest, word(app, s + 0xc)))
            return std::nullopt;
    }
    return 1;
}

/* Mono XA streamed from memory: the state has the chunk's data at +0x10, its
 * length at +0x14, the frame at +0x18, the frames kept at +0x1c and from
 * +0x1e.  The kept frames, whole blocks of the chunk, the block across its
 * end; at its end the next chunk asked of the refill (eax and edx the data's
 * and the length's slots), its first eight bytes the decoder's history; none,
 * and the rest is silence.  1; -1 with no chunk.  (three pushed registers,
 * 0x10 bytes kept, and the code's push) */
Result playXaMonoStreamed(App* app, CPU& cpu, reg32 esp, reg32 s, int32_t wanted, reg32 target)
{
    const reg32 f = esp - 3 * 4 - 0x10 - 4;
    if (word(app, s + 0x10) == 0)
        return -1;
    setWord(app, s + 0xc, target);
    if (wanted > 0)
    {
        do
        {
            if (half(app, s + 0x1c) != 0)
            {
                const int32_t copied = copyKeptMono(app, signedHalf(app, s + 0x1c), s + 0x1e, wanted, s + 0xc);
                setHalf(app, s + 0x1c, uint16_t(half(app, s + 0x1c) - copied));
                wanted -= copied;
            }
            else if (signedWord(app, s + 0x18) + 0x1b < signedWord(app, s + 0x14))
            {
                uint32_t blocks =
                    uint32_t(imul(idiv(int32_t(word(app, s + 0x14) - word(app, s + 0x18)), 0x1c), 0x1c));
                if (wanted < int32_t(blocks))
                    blocks = uint32_t(imul(idiv(wanted + 0x1b, 0x1c), 0x1c));
                setWord(app, s, blocks);
                setWord(app, s + 8, word(app, s + 0x10) + xaMonoBlockBytes(word(app, s + 0x18)));
                if (!decodeXaMono(app, cpu, f, s))
                    return std::nullopt;
                setWord(app, s + 0x18, word(app, s + 0x18) + blocks);
                wanted -= int32_t(blocks);
            }
            else
            {
                if (word(app, s + 0x10) != 1 && signedWord(app, s + 0x14) > signedWord(app, s + 0x18))
                {
                    setWord(app, s, 0x1c);
                    setWord(app, s + 8, word(app, s + 0x10) + xaMonoBlockBytes(word(app, s + 0x18)));
                    if (!decodeXaMono(app, cpu, f, s))
                        return std::nullopt;
                    const uint32_t frame = word(app, s + 0x18) + 0x1c;
                    const uint32_t overshoot = frame - word(app, s + 0x14);
                    setWord(app, s + 0x18, frame);
                    wanted -= int32_t(0x1c - overshoot);
                    setWord(app, s + 0xc, word(app, s + 0xc) - overshoot * 2);
                }
                if (!callGame(app, cpu, f, kRefill(app), {s + 0x10, s + 0x14}))
                    return std::nullopt;
                const reg32 data = word(app, s + 0x10);
                if (data == 0)
                {
                    if (!clearBytes(app, cpu, f, word(app, s + 0xc), uint32_t(wanted) * 2))
                        return std::nullopt;
                    setWord(app, s + 0xc, word(app, s + 0xc) + uint32_t(wanted) * 2);
                    wanted = 0;
                    break;
                }
                setHalf(app, s + 6, half(app, data));
                setHalf(app, s + 4, half(app, data + 2));
                setWord(app, s + 0x18, 0);
                setWord(app, s + 0x10, data + 8);
            }
        } while (wanted > 0);
    }
    setWord(app, s + 0xc, word(app, s + 0xc) + uint32_t(wanted) * 2);
    for (; wanted < 0; ++wanted)
    {
        setHalf(app, keptAt(s + 0x1e, signedHalf(app, s + 0x1c)), half(app, word(app, s + 0xc)));
        setWord(app, s + 0xc, word(app, s + 0xc) + 2);
        setHalf(app, s + 0x1c, uint16_t(half(app, s + 0x1c) + 1));
    }
    return 1;
}

/* Mono XA from a packet player: the state has the packet's data at +0x10, its
 * length at +0x14, the frame at +0x18, the player at +0x1c, the frames taken
 * since the last release at +0x20, the frames kept at +0x24 and from +0x26.
 * The frames taken last time given back (0x511998), the kept frames, the
 * packet's whole blocks and the block across its end; at its end the next
 * packet (0x5118fc), its first eight bytes the decoder's history; none, and
 * the rest is silence -- what a block left over kept.  The frames taken this
 * time.  (four pushed registers, 0xc bytes kept) */
Result playXaMonoPackets(App* app, CPU& cpu, reg32 esp, reg32 s, int32_t wanted, reg32 target)
{
    const reg32 f = esp - 4 * 4 - 0xc;
    setWord(app, s + 0xc, target);
    if (word(app, s + 0x20) != 0)
    {
        if (!callGame(app, cpu, f, kPacketFramesPlayed, {word(app, s + 0x1c), word(app, s + 0x20)}))
            return std::nullopt;
        setWord(app, s + 0x20, 0);
    }
    while (wanted > 0)
    {
        if (half(app, s + 0x24) != 0)
        {
            const int32_t copied = copyKeptMono(app, signedHalf(app, s + 0x24), s + 0x26, wanted, s + 0xc);
            setHalf(app, s + 0x24, uint16_t(half(app, s + 0x24) - copied));
            setWord(app, s + 0x20, word(app, s + 0x20) + uint32_t(copied));
            wanted -= copied;
            continue;
        }
        const uint32_t length = word(app, s + 0x14);
        if (signedWord(app, s + 0x18) + 0x1b < int32_t(length))
        {
            uint32_t blocks = uint32_t(imul(idiv(int32_t(length - word(app, s + 0x18)), 0x1c), 0x1c));
            if (int32_t(blocks) > wanted)
                blocks = uint32_t(imul(idiv(wanted + 0x1b, 0x1c), 0x1c));
            setWord(app, s + 8, word(app, s + 0x10) + xaMonoBlockBytes(word(app, s + 0x18)));
            setWord(app, s, blocks);
            if (!decodeXaMono(app, cpu, f, s))
                return std::nullopt;
            setWord(app, s + 0x18, word(app, s + 0x18) + blocks);
            setWord(app, s + 0x20, word(app, s + 0x20) + blocks);
            wanted -= int32_t(blocks);
            continue;
        }
        if (int32_t(length) > signedWord(app, s + 0x18))
        {
            setWord(app, s + 8, word(app, s + 0x10) + xaMonoBlockBytes(word(app, s + 0x18)));
            setWord(app, s, 0x1c);
            if (!decodeXaMono(app, cpu, f, s))
                return std::nullopt;
            const uint32_t frame = word(app, s + 0x18) + 0x1c;
            const uint32_t overshoot = frame - word(app, s + 0x14);
            const uint32_t inside = 0x1c - overshoot;
            setWord(app, s + 0x18, frame);
            const reg32 out = word(app, s + 0xc);
            wanted -= int32_t(inside);
            setWord(app, s + 0xc, out - overshoot * 2);
            setWord(app, s + 0x20, word(app, s + 0x20) + inside);
        }
        reg32 packet;
        if (!callGame(app, cpu, f, kNextPacket, {word(app, s + 0x1c), s + 0x14}, {}, packet))
            return std::nullopt;
        setWord(app, s + 0x10, packet);
        if (packet != 0)
        {
            setHalf(app, s + 6, half(app, packet));
            setHalf(app, s + 4, half(app, packet + 2));
            setWord(app, s + 0x18, 0);
            setWord(app, s + 0x10, packet + 8);
            continue;
        }
        if (wanted <= 0)
            continue;
        if (word(app, s + 0x20) != 0 && !clearBytes(app, cpu, f, word(app, s + 0xc), uint32_t(wanted) * 2))
            return std::nullopt;
        setWord(app, s + 0xc, word(app, s + 0xc) + uint32_t(wanted) * 2);
        wanted = 0;
    }
    setWord(app, s + 0xc, word(app, s + 0xc) + uint32_t(wanted) * 2);
    if (wanted < 0)
    {
        const uint32_t over = uint32_t(-wanted);
        copyForward(app, keptAt(s + 0x26, signedHalf(app, s + 0x24)), word(app, s + 0xc), over * 2);
        setHalf(app, s + 0x24, uint16_t(half(app, s + 0x24) + over));
        setWord(app, s + 0x20, word(app, s + 0x20) - over);
    }
    return int32_t(word(app, s + 0x20));
}

/* ---------------------------------------------------------- XA, stereo */

/* Stereo XA played once: the state has mix::xaStereo's head (the frames to
 * decode, the history of both sides, +0xc where it reads and +0x10 where it
 * writes), the data at +0x14, the length at +0x18, the frame at +0x1c, the
 * samples kept at +0x20 (two a frame) and from +0x24.  As the mono's, two
 * samples a frame.  1; -1 at its end.  (four pushed registers, 4 bytes kept) */
Result playXaStereoOnce(App* app, CPU& cpu, reg32 esp, reg32 s, int32_t wanted, reg32 target)
{
    const reg32 f = esp - 4 * 4 - 4;
    if (signedWord(app, s + 0x1c) >= signedWord(app, s + 0x18))
        return -1;
    Output out{app, target};
    const int32_t held = signedHalf(app, s + 0x20);
    if (wanted <= (held >> 1))
    {
        const int32_t samples = wanted * 2;
        int32_t i = 0;
        for (; i < samples; ++i)
            out.put(half(app, s + 0x24 + 2 * i));
        uint32_t left = 0;
        for (; i < signedHalf(app, s + 0x20); ++i, ++left)
            setHalf(app, s + 0x24 + 2 * left, half(app, s + 0x24 + 2 * i));
        setHalf(app, s + 0x20, uint16_t(left));
        setWord(app, s + 0x10, out.at);
        return 1;
    }
    for (int32_t i = 0; i < held; ++i)
        out.put(half(app, s + 0x24 + 2 * i));
    const int32_t rest = wanted - (held >> 1);
    int32_t left = signedWord(app, s + 0x18) - signedWord(app, s + 0x1c);
    if (left < 0)
        left = 0;
    const int32_t n = rest >= left ? left : rest;
    setWord(app, s + 0x10, out.at);
    setWord(app, s, uint32_t(n));
    setWord(app, s + 0xc, word(app, s + 0x14) + xaStereoBlockBytes(word(app, s + 0x1c)));
    if (!decodeXaStereo(app, cpu, f, s))
        return std::nullopt;
    setWord(app, s + 0x1c, word(app, s + 0x1c) + uint32_t(n) - word(app, s));
    out.at = word(app, s + 0x10);
    if (left == n)
    {
        int32_t silence = (rest - n - int32_t(word(app, s + 0x1c) - word(app, s + 0x18))) * 2;
        for (; silence > 0; --silence)
            out.put(0);
        setWord(app, s + 0x10, out.at);
    }
    else
    {
        int32_t over = -(0x1c - rest % 0x1c);
        if (over == -0x1c)
            over = 0;
        over *= 2;
        uint32_t kept = 0;
        for (int32_t k = over; k < 0; ++k, ++kept)
            setHalf(app, s + 0x24 + 2 * kept, half(app, out.at + 2 * reg32(k)));
        setHalf(app, s + 0x20, uint16_t(kept));
    }
    return 1;
}

/* Stereo XA looped: the state has the data at +0x14, the frame at +0x1c, the
 * loop's start at +0x20, the length at +0x24, the frames kept at +0x28 and
 * from +0x2c, the history of the four at the loop at +0x108 and the decoder's
 * slot at +0x110.  As the mono's.  1.  (four pushed registers, 0x7c bytes
 * kept) */
Result playXaStereoLooped(App* app, CPU& cpu, reg32 esp, reg32 s, int32_t wanted, reg32 target)
{
    const reg32 f = esp - 4 * 4 - 0x7c;
    setWord(app, s + 0x10, target);
    if (wanted > 0)
    {
        do
        {
            if (half(app, s + 0x28) != 0)
            {
                const int32_t copied = copyKeptStereo(app, signedHalf(app, s + 0x28), s + 0x2c, wanted, s + 0x10);
                setHalf(app, s + 0x28, uint16_t(half(app, s + 0x28) - copied));
                wanted -= copied;
            }
            else if (signedWord(app, s + 0x1c) + 0x1b < signedWord(app, s + 0x24))
            {
                setWord(app, s + 0xc, word(app, s + 0x14) + xaStereoBlockBytes(word(app, s + 0x1c)));
                setWord(app, s, 0x1c);
                if (!decodeXaStereo(app, cpu, f, s))
                    return std::nullopt;
                setWord(app, s + 0x1c, word(app, s + 0x1c) + 0x1c);
                wanted -= 0x1c;
            }
            else
            {
                const reg32 saved = word(app, s + 0x10);
                setWord(app, s + 0xc, word(app, s + 0x14) + xaStereoBlockBytes(word(app, s + 0x1c)));
                setWord(app, s, 0x1c);
                setWord(app, s + 0x10, s + 0x2c);
                if (!decodeXaStereo(app, cpu, f, s))
                    return std::nullopt;
                setWord(app, s + 0x1c, word(app, s + 0x1c) + 0x1c);
                const uint16_t overshoot = uint16_t(half(app, s + 0x1c) - half(app, s + 0x24));
                setHalf(app, s + 0x28, uint16_t(0x1c - overshoot + 1));
                setHalf(app, s + 6, half(app, s + 0x108));
                setHalf(app, s + 4, half(app, s + 0x10a));
                setHalf(app, s + 0xa, half(app, s + 0x10c));
                setHalf(app, s + 8, half(app, s + 0x10e));
                setWord(app, s + 0x1c, uint32_t(imul(idiv(int32_t(word(app, s + 0x20)), 0x1c), 0x1c)));
                if (int32_t(word(app, s + 0x20)) % 0x1c != 0)
                {
                    setWord(app, s, 0x1c);
                    setWord(app, s + 0xc, word(app, s + 0x14) + xaStereoBlockBytes(word(app, s + 0x1c)));
                    setWord(app, s + 0x10, f);
                    if (!decodeXaStereo(app, cpu, f, s))
                        return std::nullopt;
                    const uint32_t frame = word(app, s + 0x1c) + 0x1c;
                    const uint32_t from = frame - word(app, s + 0x20);
                    setWord(app, s + 0x1c, frame);
                    reg32 sample = f + 4 * (0x1c - from);
                    for (int32_t j = 0; int32_t(from) > 0 && j < int32_t(from); ++j, sample += 4)
                    {
                        const reg32 kept = s + 0x2c + reg32(signedHalf(app, s + 0x28)) * 4;
                        setHalf(app, kept, half(app, sample));
                        setHalf(app, s + 0x2c + reg32(signedHalf(app, s + 0x28)) * 4 + 2, half(app, sample + 2));
                        setHalf(app, s + 0x28, uint16_t(half(app, s + 0x28) + 1));
                    }
                }
                setWord(app, s + 0x10, saved);
            }
        } while (wanted > 0);
    }
    setWord(app, s + 0x10, word(app, s + 0x10) + uint32_t(wanted) * 4);
    for (; wanted < 0; ++wanted)
    {
        const reg32 out = word(app, s + 0x10);
        const reg32 kept = s + 0x2c + reg32(signedHalf(app, s + 0x28)) * 4;
        setHalf(app, kept, half(app, out));
        setHalf(app, kept + 2, half(app, out + 2));
        setWord(app, s + 0x10, out + 4);
        setHalf(app, s + 0x28, uint16_t(half(app, s + 0x28) + 1));
    }
    return 1;
}

/* Stereo XA with an intro: as the mono's, two samples a frame.  1.  (four
 * pushed registers, 0xc bytes kept) */
Result playXaStereoIntro(App* app, CPU& cpu, reg32 esp, reg32 s, int32_t wanted, reg32 target)
{
    const reg32 f = esp - 4 * 4 - 0xc;
    Output out{app, target};
    const int32_t held = signedHalf(app, s + 0x28);
    const auto putKeptFrame = [&](int32_t i) {
        out.put(half(app, s + 0x2c + 4 * reg32(i)));
        out.put(half(app, s + 0x2e + 4 * reg32(i)));
    };
    if (held >= wanted)
    {
        int32_t i = 0;
        for (; i < wanted; ++i)
            putKeptFrame(i);
        uint32_t left = 0;
        for (; i < signedHalf(app, s + 0x28); ++i, ++left)
        {
            setHalf(app, s + 0x2c + 4 * left, half(app, s + 0x2c + 4 * reg32(i)));
            setHalf(app, s + 0x2e + 4 * left, half(app, s + 0x2e + 4 * reg32(i)));
        }
        setHalf(app, s + 0x28, uint16_t(left));
        setWord(app, s + 0x10, out.at);
        return 1;
    }
    for (int32_t i = 0; i < held; ++i)
        putKeptFrame(i);
    setWord(app, s + 0x10, out.at);
    int32_t rest = wanted - held;
    int32_t n = signedWord(app, s + 0x20) - signedWord(app, s + 0x1c) - 0x1b;
    if (n > rest)
        n = rest;
    if (n < 0)
        n = 0;
    setWord(app, s, uint32_t(n));
    setWord(app, s + 0xc, word(app, s + 0x14) + xaStereoBlockBytes(word(app, s + 0x1c)));
    if (!decodeXaStereo(app, cpu, f, s))
        return std::nullopt;
    setWord(app, s + 0x1c, word(app, s + 0x1c) + uint32_t(n) - word(app, s));
    const int32_t over = signedWord(app, s);
    uint32_t kept = 0;
    for (int32_t k = over; k < 0; ++k, ++kept)
    {
        const reg32 from = word(app, s + 0x10) + 4 * reg32(k);
        setHalf(app, s + 0x2c + 4 * kept, half(app, from));
        setHalf(app, s + 0x2e + 4 * kept, half(app, from + 2));
    }
    rest -= n;
    setHalf(app, s + 0x28, uint16_t(kept));
    setWord(app, s + 0x10, word(app, s + 0x10) - 4 * kept);
    if (int32_t(word(app, s + 0x20) - word(app, s + 0x1c)) < 0x1c)
    {
        setHalf(app, s + 0x108, half(app, s + 6));
        setHalf(app, s + 0x10a, half(app, s + 4));
        setHalf(app, s + 0x10c, half(app, s + 0xa));
        setHalf(app, s + 0x10e, half(app, s + 8));
        setWord(app, word(app, s + 0x110), 0x529cfc);
        if (rest > 0 && !playXaStereoLooped(app, cpu, f - 4, s, rest, word(app, s + 0x10)))
            return std::nullopt;
    }
    return 1;
}

/* Stereo XA streamed from memory: the state has the chunk's data at +0x14,
 * its length at +0x18, the frame at +0x1c, the frames kept (a word) at +0x20
 * and from +0x24.  As the mono's, two samples a frame and the history four
 * halves; its silence written by halves.  1; -1 with no chunk.  (three
 * pushed registers, 0xc bytes kept, and the code's push) */
Result playXaStereoStreamed(App* app, CPU& cpu, reg32 esp, reg32 s, int32_t wanted, reg32 target)
{
    const reg32 f = esp - 3 * 4 - 0xc - 4;
    if (word(app, s + 0x14) == 0)
        return -1;
    setWord(app, s + 0x10, target);
    if (wanted > 0)
    {
        do
        {
            if (word(app, s + 0x20) != 0)
            {
                const int32_t copied = copyKeptStereo(app, signedWord(app, s + 0x20), s + 0x24, wanted, s + 0x10);
                setWord(app, s + 0x20, word(app, s + 0x20) - uint32_t(copied));
                wanted -= copied;
                continue;
            }
            if (signedWord(app, s + 0x1c) + 0x1b < signedWord(app, s + 0x18))
            {
                setWord(app, s + 0xc, word(app, s + 0x14) + xaStereoBlockBytes(word(app, s + 0x1c)));
                setWord(app, s, 0x1c);
                if (!decodeXaStereo(app, cpu, f, s))
                    return std::nullopt;
                setWord(app, s + 0x1c, word(app, s + 0x1c) + 0x1c);
                wanted -= 0x1c;
                continue;
            }
            if (word(app, s + 0x14) != 1)
            {
                setWord(app, s + 0xc, word(app, s + 0x14) + xaStereoBlockBytes(word(app, s + 0x1c)));
                setWord(app, s, 0x1c);
                if (!decodeXaStereo(app, cpu, f, s))
                    return std::nullopt;
                const uint32_t frame = word(app, s + 0x1c) + 0x1c;
                setWord(app, s + 0x1c, frame);
                const uint32_t overshoot = frame - word(app, s + 0x18);
                wanted -= int32_t(0x1c - overshoot);
                setWord(app, s + 0x10, word(app, s + 0x10) - (overshoot << 2));
            }
            if (!callGame(app, cpu, f, kRefill(app), {s + 0x14, s + 0x18}))
                return std::nullopt;
            const reg32 data = word(app, s + 0x14);
            if (data == 0)
            {
                Output silence{app, word(app, s + 0x10)};
                for (wanted *= 2; wanted > 0; --wanted)
                    silence.put(0);
                setWord(app, s + 0x10, silence.at);
                continue;
            }
            setHalf(app, s + 6, half(app, data));
            setHalf(app, s + 4, half(app, data + 2));
            setHalf(app, s + 0xa, half(app, data + 4));
            setHalf(app, s + 8, half(app, data + 6));
            setWord(app, s + 0x1c, 0);
            setWord(app, s + 0x14, data + 8);
        } while (wanted > 0);
    }
    setWord(app, s + 0x10, word(app, s + 0x10) + uint32_t(wanted) * 4);
    for (; wanted < 0; ++wanted)
    {
        const reg32 out = word(app, s + 0x10);
        const reg32 kept = s + 0x24 + word(app, s + 0x20) * 4;
        setHalf(app, kept, half(app, out));
        setHalf(app, kept + 2, half(app, out + 2));
        setWord(app, s + 0x10, out + 4);
        setWord(app, s + 0x20, word(app, s + 0x20) + 1);
    }
    return 1;
}

/* Stereo XA from a packet player: the state has the packet's data at +0x14,
 * its length at +0x18, the frame at +0x1c, the frames kept (a word) at +0x20,
 * the player at +0x24, the frames taken since the last release at +0x28 and
 * the kept samples from +0x2c.  As the mono's, two samples a frame.  The
 * frames taken this time.  (four pushed registers, 8 bytes kept) */
Result playXaStereoPackets(App* app, CPU& cpu, reg32 esp, reg32 s, int32_t wanted, reg32 target)
{
    const reg32 f = esp - 4 * 4 - 8;
    setWord(app, s + 0x10, target);
    if (word(app, s + 0x28) != 0)
    {
        if (!callGame(app, cpu, f, kPacketFramesPlayed, {word(app, s + 0x24), word(app, s + 0x28)}))
            return std::nullopt;
        setWord(app, s + 0x28, 0);
    }
    while (wanted > 0)
    {
        if (word(app, s + 0x20) != 0)
        {
            const int32_t copied = copyKeptStereo(app, signedWord(app, s + 0x20), s + 0x2c, wanted, s + 0x10);
            wanted -= copied;
            setWord(app, s + 0x20, word(app, s + 0x20) - uint32_t(copied));
            setWord(app, s + 0x28, word(app, s + 0x28) + uint32_t(copied));
            continue;
        }
        if (signedWord(app, s + 0x1c) + 0x1b < signedWord(app, s + 0x18))
        {
            setWord(app, s + 0xc, word(app, s + 0x14) + xaStereoBlockBytes(word(app, s + 0x1c)));
            setWord(app, s, 0x1c);
            if (!decodeXaStereo(app, cpu, f, s))
                return std::nullopt;
            setWord(app, s + 0x1c, word(app, s + 0x1c) + 0x1c);
            setWord(app, s + 0x28, word(app, s + 0x28) + 0x1c);
            wanted -= 0x1c;
            continue;
        }
        if (word(app, s + 0x14) != 0)
        {
            setWord(app, s + 0xc, word(app, s + 0x14) + xaStereoBlockBytes(word(app, s + 0x1c)));
            setWord(app, s, 0x1c);
            if (!decodeXaStereo(app, cpu, f, s))
                return std::nullopt;
            const uint32_t frame = word(app, s + 0x1c) + 0x1c;
            setWord(app, s + 0x1c, frame);
            const uint32_t overshoot = frame - word(app, s + 0x18);
            const uint32_t inside = 0x1c - overshoot;
            const reg32 out = word(app, s + 0x10) - (overshoot << 2);
            wanted -= int32_t(inside);
            setWord(app, s + 0x10, out);
            setWord(app, s + 0x28, word(app, s + 0x28) + inside);
        }
        reg32 packet;
        if (!callGame(app, cpu, f, kNextPacket, {word(app, s + 0x24), s + 0x18}, {}, packet))
            return std::nullopt;
        setWord(app, s + 0x14, packet);
        if (packet != 0)
        {
            setHalf(app, s + 6, half(app, packet));
            setHalf(app, s + 4, half(app, packet + 2));
            setHalf(app, s + 0xa, half(app, packet + 4));
            setHalf(app, s + 8, half(app, packet + 6));
            setWord(app, s + 0x1c, 0);
            setWord(app, s + 0x14, packet + 8);
            continue;
        }
        if (wanted <= 0)
            continue;
        if (word(app, s + 0x28) != 0 && !clearBytes(app, cpu, f, word(app, s + 0x10), uint32_t(wanted) << 2))
            return std::nullopt;
        setWord(app, s + 0x10, word(app, s + 0x10) + (uint32_t(wanted) << 2));
        wanted = 0;
    }
    setWord(app, s + 0x10, word(app, s + 0x10) + (uint32_t(wanted) << 2));
    if (wanted < 0)
    {
        const uint32_t over = uint32_t(-wanted);
        copyForward(app, s + 0x2c + (word(app, s + 0x20) << 2), word(app, s + 0x10), over * 4);
        setWord(app, s + 0x20, word(app, s + 0x20) + over);
        setWord(app, s + 0x28, word(app, s + 0x28) - over);
    }
    return int32_t(word(app, s + 0x28));
}

/* ------------------------------------------------------- frame-coded */

/* The bit reader the coded sound reads its bits through (the state's first
 * three words): the next byte, the bits, how many there are.  The next `n`
 * bits (a mask of the table at 0x56b1d0 over the bits), the reader topped up
 * with a byte when fewer than 8 are left. */
uint32_t readCodedBits(App* app, reg32 r, uint32_t n)
{
    const uint32_t value = word(app, n * 4 + 0x56b1d0) & word(app, r + 4);
    const uint32_t left = word(app, r + 8) - n;
    setWord(app, r + 8, left);
    setWord(app, r + 4, word(app, r + 4) >> (n & 0x1f));
    if (int32_t(left) < 8)
    {
        const uint32_t next = uint32_t(byteAt(app, word(app, r))) << (left & 0x1f);
        setWord(app, r, word(app, r) + 1);
        setWord(app, r + 8, left + 8);
        setWord(app, r + 4, word(app, r + 4) | next);
    }
    return value;
}

/* `n` bits passed over. */
void skipCodedBits(App* app, reg32 r, uint32_t n)
{
    setWord(app, r + 4, word(app, r + 4) >> (n & 0x1f));
    const uint32_t left = word(app, r + 8) - n;
    setWord(app, r + 8, left);
    if (int32_t(left) < 8)
    {
        const uint32_t next = uint32_t(byteAt(app, word(app, r))) << (left & 0x1f);
        setWord(app, r + 4, word(app, r + 4) | next);
        setWord(app, r + 8, left + 8);
        setWord(app, r, word(app, r) + 1);
    }
}

/* A subframe's pulses, every `step`-th of 0x6c floats given to `put` (index,
 * the float's bits).  Kind 0: two bits each -- 0, -2.0, 0 or 2.0.  Otherwise
 * the codes of the tables at 0x56b2f4 (the next state by the low byte) and
 * 0x56b4f4 (the next table, the bits it took, the value): past 3 the value;
 * 2 or 3 a run of zeros (6 bits + 7); 0 or 1 a count of 1 bits from 7, then
 * its sign. */
template <class Put>
void readPulses(App* app, reg32 r, uint32_t kind, int32_t step, Put&& put)
{
    int32_t i = 0;
    if (kind == 0)
    {
        do
        {
            const uint32_t pair = word(app, r + 4) & 3;
            uint32_t bits = 1, value = 0;
            if (pair == 1)
                bits = 2, value = 0xc0000000;
            else if (pair == 3)
                bits = 2, value = 0x40000000;
            put(i, value);
            skipCodedBits(app, r, bits);
            i += step;
        } while (i < 0x6c);
        return;
    }
    uint32_t table = 0;
    do
    {
        const uint32_t code = byteAt(app, (table << 8) + (word(app, r + 4) & 0xff) + 0x56b2f4);
        const uint32_t entry = code * 0xc;
        table = word(app, entry + 0x56b4f4);
        skipCodedBits(app, r, word(app, entry + 0x56b4f8));
        if (int32_t(code) > 3)
        {
            put(i, word(app, entry + 0x56b4fc));
            i += step;
        }
        else if (int32_t(code) > 1)
        {
            int32_t run = int32_t(readCodedBits(app, r, 6) + 7);
            if (int32_t(uint32_t(imul(run, step)) + uint32_t(i)) > 0x6c)
                run = step != 0 ? idiv(0x6c - i, step) : 0;
            for (int32_t k = 0; k < run; ++k, i += step)
                put(i, 0);
        }
        else
        {
            uint32_t count = 7;
            while (readCodedBits(app, r, 1) == 1)
                count += 1;
            const uint32_t negative = readCodedBits(app, r, 1) != 1;
            if (negative)
                count = 0u - count;
            put(i, bitsOf(float(int32_t(count))));
            i += step;
        }
    } while (i < 0x6c);
}

/* The odd samples between the even ones of an excitation (or the even
 * between the odd), 54 of them from `first` on: each from its neighbours 1,
 * 3 and 5 samples away either side, by the weights at 0x5520ec, 0x5520f4 and
 * 0x5520fc.  The samples from -5 to 5 + 108 must be there. */
void interpolateExcitation(App* app, float* first)
{
    const double c1 = doubleAt(app, 0x5520ec), c2 = doubleAt(app, 0x5520f4), c3 = doubleAt(app, 0x5520fc);
    for (int32_t i = 0; i < 0x1b0 / 8; ++i)
    {
        float* p = first + 2 * i;
        double sum = (double(p[-1]) + double(p[1])) * c1;
        sum = sum + (double(p[-3]) + double(p[3])) * c2;
        sum = sum + (double(p[-5]) + double(p[5])) * c3;
        *p = float(sum);
    }
}

/* The synthesis filter's 12 coefficients from the 12 reflection coefficients
 * at `k`, step by step through a lattice, a float at each step (the sums run
 * on in the x87's registers, the doubles here). */
void lattice(App* app, reg32 k, float* out)
{
    float reflection[12];
    for (reg32 i = 0; i < 12; ++i)
        reflection[i] = load<float>(app, k + 4 * i);
    float a[12];  // a[0] the lattice's -1st, a[i + 1] its i-th
    float history[12];
    a[0] = 1.0f;
    for (int32_t i = 0; i < 11; ++i)
        a[i + 1] = reflection[i];
    for (int32_t n = 0; n < 12; ++n)
    {
        double acc = -double(reflection[11]) * double(a[11]);
        for (int32_t e = 10; e >= 0; --e)
        {
            acc = acc - double(reflection[e]) * double(a[e]);
            a[e + 1] = float(double(reflection[e]) * acc + double(a[e]));
        }
        const float latest = float(acc);
        a[0] = latest;
        history[n] = latest;
        float result = latest;
        if (n > 0)
        {
            double sum = double(latest);
            for (int32_t i = 0; i < n; ++i)
                sum = sum - double(out[i]) * double(history[n - 1 - i]);
            result = float(sum);
        }
        out[n] = result;
    }
}

/* `blocks` of 12 samples of the excitation from sample `first` (at +0x684 of
 * the state) through the synthesis filter: its 12 coefficients from the
 * reflection coefficients at +0x114, each sample the excitation there plus
 * the 12 last outputs (a ring at +0x144) by them; the sample stored plus 0
 * (or 1.5 * 2^23, which rounds it to a whole number).  Unrolled by 12 as the
 * code is: of each 12, the 4th and the 11th add the output as stored (a
 * float), the others as the x87 holds it. */
void synthesize(App* app, reg32 s, uint32_t first, int32_t blocks, bool round)
{
    float coefficients[12];
    lattice(app, s + 0x114, coefficients);
    const double offset = round ? double(floatOf(0x4b400000)) : 0.0;
    reg32 block = first * 4 + s + 0x684;
    for (int32_t b = 0; b < blocks; ++b, block += 0x30)
        for (reg32 j = 0; j < 12; ++j)
        {
            const reg32 x = block + 4 * j;
            double sum = double(coefficients[0]) * singleAt(app, s + 0x144 + 4 * ((12 - j) % 12)) + singleAt(app, x);
            for (reg32 k = 1; k < 12; ++k)
                sum = sum + double(coefficients[k]) * singleAt(app, s + 0x144 + 4 * ((k + 12 - j) % 12));
            const reg32 ring = s + 0x144 + 4 * (11 - j);
            setSingle(app, ring, sum);
            if (j == 3 || j == 10)
                sum = singleAt(app, ring);
            setSingle(app, x, sum + offset);
        }
}

/* A frame-coded sound's decoder set up, the mixer's emms first: its bit
 * reader on the data after the first byte, its header (a flag, a shift, a
 * scale and the gains -- 64 floats, each the scale times the last) and its
 * history cleared.  (two pushed registers, 8 bytes kept, and the code's
 * push) */
Result startCoded(App* app, CPU& cpu, reg32 esp, reg32 data, reg32 s)
{
    const reg32 f = esp - 2 * 4 - 8;
    if (!callGame(app, cpu, f - 4, kMmxEnd(app)))
        return std::nullopt;
    setWord(app, s + 8, 8);
    setWord(app, s + 4, byteAt(app, data));
    setWord(app, s, data + 1);
    setWord(app, s + 0xc, readCodedBits(app, s, 1));
    setWord(app, s + 0x10, 0x20 - readCodedBits(app, s, 4));
    const uint32_t first = readCodedBits(app, s, 4) + 1;
    setSingle(app, s + 0x14, double(int64_t(first)) * doubleAt(app, 0x552104));
    const uint32_t shift = readCodedBits(app, s, 6);
    const double scale = double(int64_t(shift)) * doubleAt(app, 0x55210c) + doubleAt(app, 0x552114);
    for (reg32 a = s + 4; a != s + 0x100; a += 4)
        setSingle(app, a + 0x14, singleAt(app, a + 0x10) * scale);
    for (reg32 a = s + 4; a != s + 0x34; a += 4)
    {
        setWord(app, a + 0x110, 0);
        setWord(app, a + 0x140, 0);
    }
    for (reg32 a = s + 4; a != s + 0x514; a += 4)
        setWord(app, a + 0x170, 0);
    return int32_t(s + 0x510);
}

/* One frame of 432 samples, the mixer's emms first: the change of the 12
 * reflection coefficients (6 bits each for 4, 5 for 8, by the tables at
 * 0x56b1f4 and 0x56b234, a quarter of it a step); 4 subframes of 108, each
 * the past excitation at its lag by one gain plus the pulses (every other
 * one, the rest interpolated and the gain scaled) by the other; the
 * excitation kept, then the synthesis filter over 12, 12, 12 and 396
 * samples, the coefficients a step nearer before each.  (five pushed
 * registers, 0x230 bytes kept) */
Result decodeCodedFrame(App* app, CPU& cpu, reg32 esp, reg32 s, uint32_t round)
{
    const reg32 f = esp - 5 * 4 - 0x230;
    if (!callGame(app, cpu, f, kMmxEnd(app)))
        return std::nullopt;
    float deltas[12];
    const double quarter = doubleAt(app, 0x55211c);
    const auto delta = [&](int32_t i, uint32_t index, reg32 table) {
        deltas[i] = float((singleAt(app, index * 4 + table) - singleAt(app, s + 0x114 + 4 * reg32(i))) * quarter);
    };
    const uint32_t firstBits = readCodedBits(app, s, 6);
    const uint32_t shortBlock = firstBits < word(app, s + 0x10) ? 1 : 0;
    delta(0, firstBits, 0x56b1f4);
    for (int32_t i = 1; i < 4; ++i)
        delta(i, readCodedBits(app, s, 6), 0x56b1f4);
    for (int32_t i = 4; i < 12; ++i)
        delta(i, readCodedBits(app, s, 5), 0x56b234);
    float buffer[5 + 108 + 5] = {};  // the excitation, with 5 zeros either side for the interpolation
    float* excitation = buffer + 5;
    uint32_t lagBase = 0xd8;
    for (uint32_t sub = 0; sub != 0x6c0; sub += 0x1b0)
    {
        const uint32_t lag = lagBase - readCodedBits(app, s, 8);
        const double gain1 = double(float(double(readCodedBits(app, s, 4)) * doubleAt(app, 0x552124)));
        double gain2 = singleAt(app, s + readCodedBits(app, s, 6) * 4 + 0x14);
        const auto put = [&](int32_t i, uint32_t bits) { excitation[i] = floatOf(bits); };
        if (word(app, s + 0xc) == 0)
            readPulses(app, s, shortBlock, 1, put);
        else
        {
            const uint32_t phase = readCodedBits(app, s, 1);
            const uint32_t second = readCodedBits(app, s, 1);
            readPulses(app, s, shortBlock, 2, [&](int32_t i, uint32_t bits) { excitation[phase + i] = floatOf(bits); });
            if (second != 0)
            {
                for (int32_t k = 0; k < 0x1b0 / 8; ++k)
                    excitation[1 - int32_t(phase) + 2 * k] = 0.0f;
            }
            else
            {
                for (int32_t i = 0; i < 5; ++i)
                {
                    buffer[i] = 0.0f;
                    buffer[5 + 108 + i] = 0.0f;
                }
                interpolateExcitation(app, excitation + 1 - int32_t(phase));
                gain2 = double(float(gain2 * doubleAt(app, 0x55212c)));
            }
        }
        const reg32 lagAt = lag * 4 + s;
        for (uint32_t i = 0; i != 0x1b0; i += 4)
            setSingle(app, s + sub + 4 + i + 0x680,
                      gain1 * singleAt(app, lagAt + i + 0x174) + gain2 * double(excitation[i / 4]));
        lagBase += 0x6c;
    }
    copyForward(app, s + 0x174, s + 0x834, 0x510);
    const reg32 places[4][2] = {{0, 1}, {0xc, 1}, {0x18, 1}, {0x24, 0x21}};
    for (const auto& p : places)
    {
        for (reg32 i = 0; i < 12; ++i)
            setSingle(app, s + 0x114 + 4 * i, double(deltas[i]) + singleAt(app, s + 0x114 + 4 * i));
        synthesize(app, s, p[0], int32_t(p[1]), round != 0);
    }
    return int32_t(s + 0xd44);  // as the last synthesis leaves eax: past the last sample it filtered
}

/* A decoded frame's samples as dwords, each to 16 bits, clipped by its low 17
 * bits as the code tests them.  The last sample as written (`to` with none). */
uint32_t clipToWords(App* app, reg32 from, reg32 to, int32_t count)
{
    uint32_t last = to;
    for (int32_t i = 0; i < count; ++i)
    {
        uint32_t v = word(app, from + 4 * reg32(i)) & 0x1ffff;
        if (v > 0x7fff && v < 0x18000)
            v = v >= 0x10000 ? 0x8000 : 0x7fff;
        setHalf(app, to + 2 * reg32(i), uint16_t(v));
        last = v;
    }
    return last;
}

/* A frame-coded sound: what is left of the frame (432 samples as dwords at
 * +0x684, the length at +0xd44, the samples of the frame left at +0xd48, the
 * frames played at +0xd4c) first, then frame after frame; past the end,
 * silence.  1; -1 at its end.  (four pushed registers, 0xc bytes kept) */
Result playCoded(App* app, CPU& cpu, reg32 esp, reg32 s, int32_t wanted, reg32 out)
{
    const reg32 f = esp - 4 * 4 - 0xc;
    const uint32_t played = word(app, s + 0xd4c);
    if (played >= word(app, s + 0xd44))
        return -1;
    int32_t over = int32_t(played + uint32_t(wanted) - word(app, s + 0xd44));
    setWord(app, s + 0xd4c, played + uint32_t(wanted));
    if (over > 0)
        wanted -= over;
    else
        over = 0;
    const reg32 frame = s + 0x684;
    const auto copy = [&](int32_t n) {
        clipToWords(app, frame + ((0x1b0 - word(app, s + 0xd48)) << 2), out, n);
        out += uint32_t(n) * 2;
        setWord(app, s + 0xd48, word(app, s + 0xd48) - uint32_t(n));
        wanted -= n;
    };
    int32_t n = int32_t(word(app, s + 0xd48));
    if (wanted < n)
        n = wanted;
    copy(n);
    if (wanted > 0)
    {
        do
        {
            if (!decodeCodedFrame(app, cpu, f - 4, s, 1))
                return std::nullopt;
            setWord(app, s + 0xd48, 0x1b0);
            copy(wanted < 0x1b0 ? wanted : 0x1b0);
        } while (wanted > 0);
    }
    for (; over > 0; --over, out += 2)
        setHalf(app, out, 0);
    return 1;
}

/* A frame-coded sound set up, its decoder by startCoded.  0xd50, the state's
 * size.  (the code pushes esi) */
Result setupCoded(App* app, CPU& cpu, reg32 esp, reg32 data, uint32_t length, reg32 s, reg32 decoderSlot,
                  reg32 playedSlot)
{
    if (!startCoded(app, cpu, esp - 4 - 4, data, s))
        return std::nullopt;
    setWord(app, s + 0xd48, 0);
    setWord(app, s + 0xd4c, 0);
    setWord(app, s + 0xd44, length);
    setWord(app, decoderSlot, 0x52a668);
    setWord(app, playedSlot, 0x52a610);
    return 0xd50;
}

/* A chunk's raw block (a first byte of 0): the bit reader after the byte and
 * the one that follows it, 8 bits there. */
void startRawBlock(App* app, reg32 s, reg32 chunk)
{
    setWord(app, s + 4, byteAt(app, chunk + 1));
    setWord(app, s + 8, 8);
    setWord(app, s, word(app, s + 0xd44) + 2);
}

/* A frame-coded sound streamed from memory: the chunk at +0xd44, its samples
 * at +0xd48, the sample in it at +0xd4c and the samples of the decoded frame
 * left at +0xd50.  Frame after frame, the chunk's end asked of the refill
 * (eax and edx the chunk's and its samples' slots); a chunk starting with 0 a
 * raw block, any other its decoder's set-up; none, and the rest is silence.
 * 1; -1 with no chunk.  (four pushed registers, 0xc bytes kept) */
Result playCodedStreamed(App* app, CPU& cpu, reg32 esp, reg32 s, int32_t wanted, reg32 out)
{
    const reg32 f = esp - 4 * 4 - 0xc;
    if (word(app, s + 0xd44) == 0)
        return -1;
    while (wanted > 0)
    {
        if (signedWord(app, s + 0xd4c) >= signedWord(app, s + 0xd48))
        {
            reg32 more;
            if (!callGame(app, cpu, f, kRefill(app), {s + 0xd44, s + 0xd48}, {}, more))
                return std::nullopt;
            const reg32 chunk = word(app, s + 0xd44);
            if (more != 0 || chunk == 0)
            {
                if (!clearBytes(app, cpu, f, out, uint32_t(wanted) * 2))
                    return std::nullopt;
                setWord(app, s + 0xd4c, 0);
                setWord(app, s + 0xd48, 0);
                return 1;
            }
            if (byteAt(app, chunk) == 0)
                startRawBlock(app, s, chunk);
            else if (!startCoded(app, cpu, f - 4, chunk + 1, s))
                return std::nullopt;
            setWord(app, s + 0xd4c, 0);
        }
        if (signedWord(app, s + 0xd50) <= 0)
        {
            if (!decodeCodedFrame(app, cpu, f - 4, s, 1))
                return std::nullopt;
            setWord(app, s + 0xd50, 0x1b0);
        }
        int32_t n = signedWord(app, s + 0xd48) - signedWord(app, s + 0xd4c);
        if (wanted < n)
            n = wanted;
        if (n > signedWord(app, s + 0xd50))
            n = signedWord(app, s + 0xd50);
        clipToWords(app, ((0x1b0 - word(app, s + 0xd50)) << 2) + s + 0x684, out, n);
        setWord(app, s + 0xd4c, word(app, s + 0xd4c) + uint32_t(n));
        out += uint32_t(n) * 2;
        setWord(app, s + 0xd50, word(app, s + 0xd50) - uint32_t(n));
        wanted -= n;
    }
    return 1;
}

/* A frame-coded sound streamed set up, its first chunk to come.  0xd54, the
 * state's size. */
void setupCodedStreamed(App* app, reg32 s, reg32 decoderSlot, reg32 resultSlot)
{
    setWord(app, s + 0xd44, 1);
    setWord(app, s + 0xd48, 0);
    setWord(app, s + 0xd4c, 0);
    setWord(app, s + 0xd50, 0);
    setWord(app, resultSlot, 0);
    setWord(app, decoderSlot, 0x52a7e0);
}

/* A frame-coded sound from a packet player: the packet at +0xd44, its
 * samples at +0xd48, the player at +0xd4c, the frames taken since the last
 * release at +0xd50, and (words) the sample in the packet at +0xd54 and the
 * samples of the decoded frame left at +0xd56.  As the streamed one's, the
 * packets from 0x5118fc, those taken given back (0x511998).  The frames taken
 * this time.  (four pushed registers, 8 bytes kept) */
Result playCodedPackets(App* app, CPU& cpu, reg32 esp, reg32 s, int32_t wanted, reg32 out)
{
    const reg32 f = esp - 4 * 4 - 8;
    if (word(app, s + 0xd50) != 0)
    {
        if (!callGame(app, cpu, f, kPacketFramesPlayed, {word(app, s + 0xd4c), word(app, s + 0xd50)}))
            return std::nullopt;
        setWord(app, s + 0xd50, 0);
    }
    while (wanted > 0)
    {
        if (int32_t(half(app, s + 0xd54)) >= signedWord(app, s + 0xd48))
        {
            reg32 packet;
            if (!callGame(app, cpu, f, kNextPacket, {word(app, s + 0xd4c), s + 0xd48}, {}, packet))
                return std::nullopt;
            setWord(app, s + 0xd44, packet);
            if (packet == 0)
            {
                if (word(app, s + 0xd50) != 0 && !clearBytes(app, cpu, f, out, uint32_t(wanted) * 2))
                    return std::nullopt;
                setHalf(app, s + 0xd54, 0);
                setWord(app, s + 0xd48, 0);
                break;
            }
            if (byteAt(app, packet) != 0)
            {
                if (!startCoded(app, cpu, f - 4, packet + 1, s))
                    return std::nullopt;
            }
            else
                startRawBlock(app, s, packet);
            setHalf(app, s + 0xd54, 0);
        }
        if (signedHalf(app, s + 0xd56) <= 0)
        {
            if (!decodeCodedFrame(app, cpu, f - 4, s, 1))
                return std::nullopt;
            setHalf(app, s + 0xd56, 0x1b0);
        }
        int32_t n = int32_t(word(app, s + 0xd48) - half(app, s + 0xd54));
        if (wanted < n)
            n = wanted;
        if (signedHalf(app, s + 0xd56) < n)
            n = signedHalf(app, s + 0xd56);
        clipToWords(app, ((0x1b0 - reg32(signedHalf(app, s + 0xd56))) << 2) + s + 0x684, out, n);
        setHalf(app, s + 0xd54, uint16_t(half(app, s + 0xd54) + n));
        setWord(app, s + 0xd50, word(app, s + 0xd50) + uint32_t(n));
        out += uint32_t(n) * 2;
        setHalf(app, s + 0xd56, uint16_t(half(app, s + 0xd56) - n));
        wanted -= n;
    }
    return int32_t(word(app, s + 0xd50));
}

/* A frame-coded sound from a packet player set up, its player by 0x52d160.
 * 0xd58, the state's size. */
Result setupCodedPackets(App* app, CPU& cpu, reg32 esp, reg32 sound, reg32 s, reg32 decoderSlot,
                         reg32 resultSlot)
{
    reg32 player;
    if (!callGame(app, cpu, esp, kPacketPlayerOf, {sound}, {}, player))
        return std::nullopt;
    setWord(app, s + 0xd44, 0);
    setWord(app, s + 0xd48, 0);
    setHalf(app, s + 0xd54, 0);
    setHalf(app, s + 0xd56, 0);
    setWord(app, s + 0xd4c, player);
    setWord(app, s + 0xd50, 0);
    setWord(app, resultSlot, 0);
    setWord(app, decoderSlot, 0x52a9c0);
    return 0xd58;
}

/* ------------------------------------------------------ conversions */

/* 16-bit samples to floats, eight at a time (read, then written) and the
 * rest one by one. */
void shortsToFloats(App* app, int32_t count, reg32 from, reg32 to)
{
    if (count > 7)
    {
        count -= 7;
        do
        {
            float v[8];
            for (reg32 i = 0; i < 8; ++i)
                v[i] = float(load<int16_t>(app, from + 2 * i));
            for (reg32 i = 0; i < 8; ++i)
                store<float>(app, to + 4 * i, v[i]);
            from += 0x10;
            to += 0x20;
            count -= 8;
        } while (count > 0);
        count += 7;
    }
    for (; count > 0; --count, from += 2, to += 4)
        store<float>(app, to, float(load<int16_t>(app, from)));
}

/* 8-bit samples to floats by the table at 0xa0c2a0, four at a time (read,
 * then written) and the rest one by one. */
void bytesToFloats(App* app, int32_t count, reg32 from, reg32 to)
{
    if (count > 3)
    {
        count -= 3;
        do
        {
            uint32_t v[4];
            for (reg32 i = 0; i < 4; ++i)
                v[i] = word(app, byteAt(app, from + i) * 4 + 0xa0c2a0);
            for (reg32 i = 0; i < 4; ++i)
                setWord(app, to + 4 * i, v[i]);
            from += 4;
            to += 0x10;
            count -= 4;
        } while (count > 0);
        count += 3;
    }
    for (; count > 0; --count, ++from, to += 4)
        setWord(app, to, word(app, byteAt(app, from) * 4 + 0xa0c2a0));
}

/* `frames` of stereo floats at `from`: each frame's two sides added and
 * halved, rounded to a whole number by adding 1.5 * 2^23 (the float written
 * back over the left side), then clipped to 16 bits by its low 20 bits as
 * the code tests them. */
reg32 floatsToWords(App* app, uint32_t frames, reg32 from, reg32 to)
{
    const reg32 end = (frames << 3) + from;
    const double half_ = doubleAt(app, 0x54e304), rounder = doubleAt(app, 0x54e30c);
    reg32 a = from;
    for (; a < end; a += 8, to += 2)
    {
        const float rounded = float((singleAt(app, a + 4) + singleAt(app, a)) * half_ + rounder);
        store<float>(app, a, rounded);
        const uint32_t bits = bitsOf(rounded) & 0xfffff;
        uint16_t w = uint16_t(bits);
        if (bits > 0x7fff && bits < 0xf8000)
            w = bits >= 0x80000 ? 0x8000 : 0x7fff;
        setHalf(app, to, w);
    }
    return a;  // where the loop stopped: eax as the code leaves it
}

/* The history of stereo XA and of mono, kept by the code in the game's
 * variables. */
constexpr reg32 kXaFirst = 0x56ad50, kXaSecond = 0x56ad60, kXaNibbles = 0xa0baa0;
constexpr reg32 kStereoHistory = 0x567870, kMonoHistory = 0x567880;

/* Stereo XA by the x87, 28 frames a block of 30 bytes: two header bytes -- a
 * filter for each side, and a shift for each -- then the nibbles, a byte a
 * frame, the high one the left side's.  Each side a float, the sum of its
 * last two by the filter's pair of coefficients and the nibble's value; the
 * history (the four floats at 0x567870) as stored.  The state: [+0] frames
 * left, [+4..+0x10] the history, [+0x14] the data, [+0x18] where. */
void decodeXaStereoFloats(App* app, reg32 s)
{
    reg32 from = word(app, s + 0x14), to = word(app, s + 0x18);
    uint32_t h[4];
    for (reg32 i = 0; i < 4; ++i)
    {
        h[i] = word(app, s + 4 + 4 * i);
        setWord(app, kStereoHistory + 4 * i, h[i]);
    }
    while (signedWord(app, s) > 0)
    {
        setWord(app, s, word(app, s) - 0x1c);
        const uint32_t lo = byteAt(app, from) & 0xf, hi = byteAt(app, from) >> 4;
        const double secondLo = singleAt(app, lo * 4 + kXaSecond), firstLo = singleAt(app, lo * 4 + kXaFirst);
        const double secondHi = singleAt(app, hi * 4 + kXaSecond), firstHi = singleAt(app, hi * 4 + kXaFirst);
        const reg32 shiftHi = reg32(byteAt(app, from + 1) >> 4) << 6, shiftLo = reg32(byteAt(app, from + 1) & 0xf) << 6;
        from += 2;
        for (int32_t n = 0x1c; n > 0; n -= 2)
        {
            for (reg32 half_ = 0; half_ < 2; ++half_)
            {
                // The first half takes (a, b, c, d) = (h0, h1, h2, h3); the second (h1, h0, h3, h2).
                const reg32 a = half_ == 0 ? 0 : 1, b = half_ == 0 ? 1 : 0;
                const reg32 c = half_ == 0 ? 2 : 3, d = half_ == 0 ? 3 : 2;
                const uint8_t code = byteAt(app, from + half_);
                const double left = (double(floatOf(h[a])) * firstHi + double(floatOf(h[b])) * secondHi)
                                    + singleAt(app, shiftHi + reg32(code >> 4) * 4 + kXaNibbles);
                const double right = (double(floatOf(h[c])) * firstLo + double(floatOf(h[d])) * secondLo)
                                     + singleAt(app, shiftLo + reg32(code & 0xf) * 4 + kXaNibbles);
                setSingle(app, to + 8 * half_, left);
                h[b] = bitsOf(float(left));
                setSingle(app, to + 8 * half_ + 4, right);
                h[d] = bitsOf(float(right));
            }
            from += 2;
            to += 0x10;
        }
    }
    setWord(app, s + 0x18, to);
    for (reg32 i = 0; i < 4; ++i)
    {
        setWord(app, kStereoHistory + 4 * i, h[i]);
        setWord(app, s + 4 + 4 * i, h[i]);
    }
}

/* Mono XA by the x87, 28 samples a block of 15 bytes: a header byte -- the
 * filter and the shift -- then the nibbles, the high one first.  Each sample
 * a float, the sum of the last two by the filter's pair of coefficients and
 * the nibble's value; a pair at a time, the second of one going into the next
 * as the x87 holds it.  The history (the last two, as stored) is the
 * variables 0x567880 and 0x567884.  The state: [+0] samples left, [+4] and
 * [+8] the history, [+0xc] the data, [+0x10] where. */
void decodeXaMonoFloats(App* app, reg32 s)
{
    reg32 from = word(app, s + 0xc), to = word(app, s + 0x10);
    setWord(app, kMonoHistory, word(app, s + 4));
    setWord(app, kMonoHistory + 4, word(app, s + 8));
    while (signedWord(app, s) > 0)
    {
        setWord(app, s, word(app, s) - 0x1c);
        const uint32_t filter = byteAt(app, from) >> 4;
        const double second = singleAt(app, filter * 4 + kXaSecond), first = singleAt(app, filter * 4 + kXaFirst);
        const reg32 shift = reg32(byteAt(app, from) & 0xf) << 6;
        ++from;
        const double lastSample = singleAt(app, kMonoHistory), beforeLast = singleAt(app, kMonoHistory + 4);
        double a = lastSample, b = lastSample, c = beforeLast * second;
        double y1 = 0, y2 = 0;
        for (int32_t n = 0; n < 14; ++n)
        {
            const uint8_t code = byteAt(app, from + reg32(n));
            y1 = (c + a * first) + singleAt(app, shift + reg32(code >> 4) * 4 + kXaNibbles);
            const double bSecond = b * second;
            setSingle(app, to + reg32(n) * 8, y1);
            y2 = (bSecond + y1 * first) + singleAt(app, shift + reg32(code & 0xf) * 4 + kXaNibbles);
            setSingle(app, to + reg32(n) * 8 + 4, y2);
            c = y1 * second;
            a = y2;
            b = y2;
        }
        from += 0xe;
        to += 0x70;
        setSingle(app, kMonoHistory + 4, y1);
        setSingle(app, kMonoHistory, y2);
    }
    setWord(app, s + 4, word(app, kMonoHistory));
    setWord(app, s + 8, word(app, kMonoHistory + 4));
    setWord(app, s + 0x10, to);
}

}  // namespace

/* The entries the game's code knows.  The mixer's volume set-ups, the
 * resamplers' and the decoders' are cdecl (the state, then the arguments, on the
 * stack; the caller pops them) or Watcom's (the state in eax, the frames wanted
 * in edx, where they go in ebx); the set-ups the mixer start calls are stdcall
 * of four words. */

// ------------------------------------------------------------ volumes

/* 0x51f580 (cdecl: state, the add's slot, the volume's slot): a mono
 * source's add and volume, MMX's own pair. */
void volumeMonoPair(App* app, CPU& cpu)
{
    const Entry e(cpu);
    hookMonoVolume(app, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2));
    e.back(cpu, e.eax);
}

/* 0x51f5a4 (cdecl: state, left, right): a mono source's volume, left twice,
 * then right twice. */
void volumeMonoLevels(App* app, CPU& cpu)
{
    const Entry e(cpu);
    setMonoLevels(app, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2));
    e.back(cpu, e.eax);
}

/* 0x51f774: the same, the MMX set's other entry. */
void volumeMonoLevelsMmx(App* app, CPU& cpu) { volumeMonoLevels(app, cpu); }

/* 0x51f680 (cdecl: state, the add's slot, the volume's slot): a stereo
 * source's. */
void volumeStereoPair(App* app, CPU& cpu)
{
    const Entry e(cpu);
    hookStereoVolume(app, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2));
    e.back(cpu, e.eax);
}

/* 0x51f6a4 (cdecl: state, left, right): a stereo source's volume, right's
 * level over left's word, twice. */
void volumeStereoLevels(App* app, CPU& cpu)
{
    const Entry e(cpu);
    setStereoLevels(app, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2));
    e.back(cpu, e.eax);
}

/* 0x51f874: the same, the MMX set's other entry. */
void volumeStereoLevelsMmx(App* app, CPU& cpu) { volumeStereoLevels(app, cpu); }

// ---------------------------------------------------------------- pan

/* 0x51f520 (eax the angle, edx and ebx where the two sides go): a pan's two
 * sides from the angle, its sine and cosine moved by the rest of the table's
 * step.  eax the second side. */
void panSides(App* app, CPU& cpu)
{
    const Entry e(cpu);
    int32_t first, second;
    if (!panOffsets(app, cpu, e.esp, e.eax, first, second))
        return;
    setWord(app, e.edx, uint32_t(first));
    setWord(app, e.ebx, uint32_t(second));
    e.back(cpu, uint32_t(second));
}

/* 0x4ffce0 (eacsnd's buffer position; eax the angle, edx the elevation, ebx
 * where the three floats go): a voice's place for DirectSound 3D. */
void bufferPosition(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, voiceBufferPosition(app, cpu, e.esp, e.eax, e.edx, e.ebx));
}

// ---------------------------------------------------------- resamplers

/* 0x51fc38 (cdecl: state, the step setter's slot, the frames needed's, the
 * resampler's): an 8-bit mono source's resampler, its step setter given. */
void resample8MonoSetup(App* app, CPU& cpu)
{
    const Entry e(cpu);
    setupResampler8Mono(app, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3));
    e.back(cpu, 0);
}

/* 0x52002c: the same for 8-bit stereo. */
void resample8StereoSetup(App* app, CPU& cpu)
{
    const Entry e(cpu);
    setupResampler8Stereo(app, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3));
    e.back(cpu, 0);
}

/* 0x51fb94 (cdecl: state, step): an 8-bit mono resampler's step set, and by
 * it its frames needed's and its resampler: down, the same or up. */
void resample8MonoStep(App* app, CPU& cpu)
{
    const Entry e(cpu);
    setResamplerStep(app, e.arg(app, 0), e.arg(app, 1), kMono8Steps);
    e.back(cpu, 0);
}

/* 0x51ff88: the same for 8-bit stereo. */
void resample8StereoStep(App* app, CPU& cpu)
{
    const Entry e(cpu);
    setResamplerStep(app, e.arg(app, 0), e.arg(app, 1), kStereo8Steps);
    e.back(cpu, 0);
}

/* The frames needed (cdecl: state, frames): down, the same, up -- the source
 * frames `frames` take at a step over 1.0, 1.0 and under.  The 16-bit mono
 * one has the 8-bit stereo one's state. */
void resample8MonoNeededDown(App* app, CPU& cpu)  // 0x51f930
{
    const Entry e(cpu);
    e.back(cpu, framesNeededDown(app, e.arg(app, 0), e.arg(app, 1), kMono8Flags));
}
void resample8MonoNeededUp(App* app, CPU& cpu)  // 0x51f980
{
    const Entry e(cpu);
    e.back(cpu, framesNeededUp(app, e.arg(app, 0), e.arg(app, 1), kMono8Flags));
}
void resample8StereoNeededDown(App* app, CPU& cpu)  // 0x51fc80
{
    const Entry e(cpu);
    e.back(cpu, framesNeededDown(app, e.arg(app, 0), e.arg(app, 1), kStereo8Flags));
}
void resample8StereoNeededUp(App* app, CPU& cpu)  // 0x51fcd0
{
    const Entry e(cpu);
    e.back(cpu, framesNeededUp(app, e.arg(app, 0), e.arg(app, 1), kStereo8Flags));
}
void resample16MonoNeededDown(App* app, CPU& cpu)  // 0x520070
{
    resample8StereoNeededDown(app, cpu);
}
void resample16MonoNeededUp(App* app, CPU& cpu)  // 0x5200c0
{
    resample8StereoNeededUp(app, cpu);
}
void resample16StereoNeededDown(App* app, CPU& cpu)  // 0x5203f0
{
    const Entry e(cpu);
    e.back(cpu, framesNeededDown(app, e.arg(app, 0), e.arg(app, 1), kStereo16Flags));
}
void resample16StereoNeededUp(App* app, CPU& cpu)  // 0x520440
{
    const Entry e(cpu);
    e.back(cpu, framesNeededUp(app, e.arg(app, 0), e.arg(app, 1), kStereo16Flags));
}

/* At the step 1.0 as many (cdecl: state, frames). */
void resample8MonoNeededSame(App* app, CPU& cpu)  // 0x51f978
{
    const Entry e(cpu);
    e.back(cpu, e.arg(app, 1));
}
void resample8StereoNeededSame(App* app, CPU& cpu) { resample8MonoNeededSame(app, cpu); }    // 0x51fcc8
void resample16MonoNeededSame(App* app, CPU& cpu) { resample8MonoNeededSame(app, cpu); }     // 0x5200b8
void resample16StereoNeededSame(App* app, CPU& cpu) { resample8MonoNeededSame(app, cpu); }   // 0x520438

/* At 1.0 the source itself is the target (cdecl: state, count, source, the
 * target's slot). */
void resample8MonoSame(App* app, CPU& cpu)  // 0x51fa80
{
    const Entry e(cpu);
    useSourceAsBlock(app, e.arg(app, 2), e.arg(app, 3));
    e.back(cpu, e.arg(app, 2));
}
void resample8StereoSame(App* app, CPU& cpu) { resample8MonoSame(app, cpu); }    // 0x51fe24
void resample16MonoSame(App* app, CPU& cpu) { resample8MonoSame(app, cpu); }     // 0x5201d8
void resample16StereoSame(App* app, CPU& cpu) { resample8MonoSame(app, cpu); }   // 0x5205b0

/* A block resampled, down (a step over 1.0) and up (cdecl: state, count,
 * source, the target's slot); the block that is left where the slot says. */
void resample8MonoDown(App* app, CPU& cpu)  // 0x51f9ac
{
    const Entry e(cpu);
    e.finish(cpu, resampleDown8Mono(app, cpu, e.esp, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)));
}
void resample8MonoUp(App* app, CPU& cpu)  // 0x51fa8c
{
    const Entry e(cpu);
    e.finish(cpu, resampleUp8Mono(app, cpu, e.esp, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)));
}
void resample8StereoDown(App* app, CPU& cpu)  // 0x51fcfc
{
    const Entry e(cpu);
    e.finish(cpu, resampleDown8Stereo(app, cpu, e.esp, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)));
}
void resample8StereoUp(App* app, CPU& cpu)  // 0x51fe30
{
    const Entry e(cpu);
    e.finish(cpu, resampleUp8Stereo(app, cpu, e.esp, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)));
}
void resample16MonoDown(App* app, CPU& cpu)  // 0x5200ec
{
    const Entry e(cpu);
    e.finish(cpu, resampleDown16Mono(app, cpu, e.esp, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)));
}
void resample16MonoUp(App* app, CPU& cpu)  // 0x5201e4
{
    const Entry e(cpu);
    e.finish(cpu, resampleUp16Mono(app, cpu, e.esp, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)));
}
void resample16StereoDown(App* app, CPU& cpu)  // 0x52046c
{
    const Entry e(cpu);
    e.finish(cpu, resampleDown16Stereo(app, cpu, e.esp, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)));
}
void resample16StereoUp(App* app, CPU& cpu)  // 0x5205bc
{
    const Entry e(cpu);
    e.finish(cpu, resampleUp16Stereo(app, cpu, e.esp, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)));
}

// ---------------------------------------------------------------- PCM

/* 0x52c8f0 (eax the state): a PCM sound's frames played so far. */
void pcmPlayed(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, word(app, e.eax + 4));
}

/* The decoders (eax the state, edx the frames wanted, ebx where they go; the
 * frames wanted given: 1 or the frames taken; -1 at the end or with no
 * data), the PCM ones: 0x52c8f4 played once, 0x52c9d0 looped, 0x52ca80
 * streamed from memory, 0x52cba0 streamed from a file. */
void pcmOnceDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playPcmOnce(app, cpu, e.esp, e.eax, e.edx, e.ebx));
}
void pcmLoopedDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, playPcmLooped(app, e.eax, int32_t(e.edx), e.ebx));
}
void pcmStreamedDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playPcmStreamed(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}
void pcmFileDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playPcmFromFile(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}

/* 0x52ccb8 (eax the stream, ebx the frame's bytes, ecx the decoder's slot,
 * edx the state): PCM streamed from a file set up.  0x14, the state's size. */
void pcmFileSetup(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, setupPcmFile(app, cpu, e.esp, e.eax, int32_t(e.ebx), e.ecx, e.edx));
}

namespace
{

/* 0x527520 and its kin (stdcall, 4 words): PCM streamed from a file, `bytes`
 * a frame -- [esp+0xc] the decoder's slot, [esp+0x10] the second function's,
 * none. */
void pcmFileEntry(App* app, CPU& cpu, int32_t bytes)
{
    const Entry e(cpu);
    setWord(app, e.arg(app, 3), 0);
    // Called from here as the code does: its esp one return address down.
    e.finish(cpu, setupPcmFile(app, cpu, e.esp - 4, e.eax, bytes, e.arg(app, 2), e.edx), 0x10);
}

}  // namespace

void pcmFileMono(App* app, CPU& cpu) { pcmFileEntry(app, cpu, 1); }
void pcmFileStereo(App* app, CPU& cpu) { pcmFileEntry(app, cpu, 2); }
void pcmFileStereo2(App* app, CPU& cpu) { pcmFileEntry(app, cpu, 2); }
void pcmFileQuad(App* app, CPU& cpu) { pcmFileEntry(app, cpu, 4); }

// ------------------------------------------------------------ XA, mono

/* 0x528f70 (eax the state): an XA sound's frames played so far. */
void xaPlayed(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, word(app, e.eax + 0x18));
}

/* 0x528f74 (eax the state, edx the frames wanted, ebx where): XA played once. */
void xaOnceDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playXaMonoOnce(app, cpu, e.esp, e.eax, e.edx, e.ebx));
}

/* 0x529140 (eax how many there are, edx where they are, ebx how many are
 * wanted, ecx the slot of where they go): the frames an XA loop kept, as
 * many as are wanted copied out and the rest moved to the front.  How many
 * were copied. */
void xaKeptCopy(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, uint32_t(copyKeptMono(app, int32_t(e.eax), e.edx, int32_t(e.ebx), e.ecx)));
}

/* 0x5291a8: XA looped. */
void xaLoopDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playXaMonoLooped(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}

/* 0x529384: XA with an intro before its loop. */
void xaIntroDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playXaMonoIntro(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}

/* 0x529590: XA streamed from memory. */
void xaStreamDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playXaMonoStreamed(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}

/* 0x529778 (stdcall, 4 words; edx the state, ecx the chunk's length): XA
 * streamed set up, its first chunk to come.  0x54, the state's size. */
void xaStreamSetup(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 s = e.edx;
    setWord(app, s + 0x10, 1);
    setWord(app, s + 0x18, 0);
    setHalf(app, s + 0x1c, 0);
    setHalf(app, s + 6, 0);
    setHalf(app, s + 4, 0);
    setWord(app, s + 0x14, e.ecx);
    setWord(app, e.arg(app, 2), 0x529590);
    setWord(app, e.arg(app, 3), 0);
    e.back(cpu, 0x54, 0x10);
}

/* 0x5297c0: XA from a packet player. */
void xaPacketDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playXaMonoPackets(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}

/* 0x5299f4 (stdcall, 4 words; eax the sound, ecx the packet's length, edx the
 * state): XA from a packet player set up, its player by 0x52d160.  0x5c, the
 * state's size. */
void xaPacketSetup(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 player;
    if (!callGame(app, cpu, e.esp, kPacketPlayerOf, {e.eax}, {}, player))
        return;
    const reg32 s = e.edx;
    setWord(app, s + 0x10, 0);
    setWord(app, s + 0x18, 0);
    setHalf(app, s + 0x24, 0);
    setHalf(app, s + 6, 0);
    setHalf(app, s + 4, 0);
    setWord(app, s + 0x20, 0);
    setWord(app, s + 0x1c, player);
    setWord(app, s + 0x14, e.ecx);
    setWord(app, e.arg(app, 2), 0x5297c0);
    setWord(app, e.arg(app, 3), 0);
    e.back(cpu, 0x5c, 0x10);
}

// ---------------------------------------------------------- XA, stereo

/* 0x529a50 (eax the state): a stereo XA sound's frames played so far. */
void xaStereoPlayed(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, word(app, e.eax + 0x1c));
}

/* 0x529a54: stereo XA played once. */
void xaStereoOnceDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playXaStereoOnce(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}

/* 0x529c2c (stdcall, 4 words; edx the state, ebx the data, ecx the length):
 * stereo XA played once set up.  0x90, the state's size. */
void xaStereoOnceSetup(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 s = e.edx;
    setWord(app, s + 0x1c, 0);
    setHalf(app, s + 0x20, 0);
    setHalf(app, s + 6, 0);
    setHalf(app, s + 4, 0);
    setHalf(app, s + 0xa, 0);
    setHalf(app, s + 8, 0);
    setWord(app, s + 0x14, e.ebx);
    setWord(app, s + 0x18, e.ecx);
    setWord(app, e.arg(app, 2), 0x529a54);
    setWord(app, e.arg(app, 3), 0x529a50);
    e.back(cpu, 0x90, 0x10);
}

/* 0x529c80: the kept copy for stereo -- frames of two samples. */
void xaStereoKeptCopy(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, uint32_t(copyKeptStereo(app, int32_t(e.eax), e.edx, int32_t(e.ebx), e.ecx)));
}

/* 0x529cfc: stereo XA looped. */
void xaStereoLoopDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playXaStereoLooped(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}

/* 0x529f24: stereo XA with an intro. */
void xaStereoIntroDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playXaStereoIntro(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}

/* 0x52a144 (stdcall, 4 words; edx the state, ebx the data, ecx its bytes; the
 * loop's start, the length, the decoder's slot, the second function's):
 * stereo XA with an intro set up.  0x114, the state's size. */
void xaStereoIntroSetup(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 s = e.edx;
    setWord(app, s + 0x1c, 0);
    setHalf(app, s + 0x28, 0);
    setHalf(app, s + 6, 0);
    setHalf(app, s + 4, 0);
    setHalf(app, s + 0xa, 0);
    setHalf(app, s + 8, 0);
    setWord(app, s + 0x14, e.ebx);
    setWord(app, s + 0x18, e.ecx);
    setWord(app, s + 0x20, e.arg(app, 0));
    setWord(app, s + 0x24, e.arg(app, 1));
    setWord(app, s + 0x110, e.arg(app, 2));
    setWord(app, e.arg(app, 2), 0x529f24);
    setWord(app, e.arg(app, 3), 0);
    e.back(cpu, 0x114, 0x10);
}

/* 0x52a1a0: stereo XA streamed from memory. */
void xaStereoStreamDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playXaStereoStreamed(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}

/* 0x52a3a0: stereo XA from a packet player. */
void xaStereoPacketDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playXaStereoPackets(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}

// ------------------------------------------------------- frame-coded

/* 0x52a610 (eax the state): a frame-coded sound's frames played so far. */
void framesPlayedCoded(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, word(app, e.eax + 0xd4c));
}

/* 0x52a618 (eax the decoded samples as dwords, edx where, ebx how many):
 * each to 16 bits, clipped by its low 17 bits.  eax the last as written. */
void codedToWords(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, clipToWords(app, e.eax, e.edx, int32_t(e.ebx)));
}

/* 0x52a668 (eax the state, edx frames wanted, ebx where): a frame-coded
 * sound. */
void codedDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playCoded(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}

/* 0x52a790 (stdcall, 4 words; ebx the data, ecx the length, edx the state):
 * a frame-coded sound set up, its decoder by 0x52de94.  0xd50, the state's
 * size. */
void codedSetup(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, setupCoded(app, cpu, e.esp, e.ebx, e.ecx, e.edx, e.arg(app, 2), e.arg(app, 3)), 0x10);
}

/* 0x52a7e0: a frame-coded sound streamed. */
void codedStreamDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playCodedStreamed(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}

/* 0x52a970 (stdcall, 4 words; edx the state): a frame-coded sound streamed
 * set up, its first chunk to come.  0xd54, the state's size. */
void codedStreamSetup(App* app, CPU& cpu)
{
    const Entry e(cpu);
    setupCodedStreamed(app, e.edx, e.arg(app, 2), e.arg(app, 3));
    e.back(cpu, 0xd54, 0x10);
}

/* 0x52a9c0: a frame-coded sound from a packet player. */
void codedPacketDecode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, playCodedPackets(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx));
}

/* 0x52ab70 (stdcall, 4 words; eax the sound, edx the state): a frame-coded
 * sound from a packet player set up, its player by 0x52d160.  0xd58, the
 * state's size. */
void codedPacketSetup(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, setupCodedPackets(app, cpu, e.esp, e.eax, e.edx, e.arg(app, 2), e.arg(app, 3)), 0x10);
}

/* 0x52d370 (eax the bit reader, edx how many): the next bits. */
void codedBits(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, readCodedBits(app, e.eax, e.edx));
}

/* 0x52d3cc (eax the bit reader, edx how many): bits passed over. */
void codedSkip(App* app, CPU& cpu)
{
    const Entry e(cpu);
    skipCodedBits(app, e.eax, e.edx);
    e.back(cpu, e.eax);
}

/* 0x52d428 (eax the bit reader, edx the kind, ebx where, ecx the step): a
 * subframe's pulses, every ecx-th of 0x6c floats. */
void codedPulses(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 out = e.ebx;
    readPulses(app, e.eax, e.edx, int32_t(e.ecx), [&](int32_t i, uint32_t bits) { setWord(app, out + 4 * reg32(i), bits); });
    e.back(cpu, e.eax);
}

/* 0x52d5cc (eax the samples): the odd samples between the even ones, 0x36 of
 * them.  eax past them. */
void codedInterpolate(App* app, CPU& cpu)
{
    const Entry e(cpu);
    // The samples from 5 before the first to 5 after the last it reads.
    float buffer[5 + 107 + 5];
    std::memcpy(buffer, &app->getMemory<uint8_t>(e.eax - 5 * 4), sizeof buffer);
    interpolateExcitation(app, buffer + 5);
    for (reg32 i = 0; i < 0x1b0 / 8; ++i)
        store<float>(app, e.eax + 8 * i, buffer[5 + 2 * i]);
    e.back(cpu, e.eax + 0x1b0);
}

/* 0x52d618 (eax the 12 reflection coefficients, edx where): the synthesis
 * filter's 12 coefficients from them. */
void codedFilter(App* app, CPU& cpu)
{
    const Entry e(cpu);
    float out[12];
    lattice(app, e.eax, out);
    for (reg32 i = 0; i < 12; ++i)
        store<float>(app, e.edx + 4 * i, out[i]);
    e.back(cpu, bitsOf(out[11]));
}

/* 0x52d6d8 (eax the state, edx the first sample, ebx how many blocks of 12,
 * ecx whether to round): the synthesis filter.  eax past the last block's
 * start. */
void codedSynthesis(App* app, CPU& cpu)
{
    const Entry e(cpu);
    synthesize(app, e.eax, e.edx, int32_t(e.ebx), e.ecx != 0);
    e.back(cpu, e.edx * 4 + e.eax + 0x684 + (int32_t(e.ebx) > 0 ? e.ebx * 0x30 : 0));
}

/* 0x52de94 (eax the data, edx the state): a frame-coded sound's decoder set
 * up. */
void codedStart(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, startCoded(app, cpu, e.esp, e.eax, e.edx));
}

/* 0x52df80 (eax the state, edx whether to round): one frame of 432 samples. */
void codedFrame(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, decodeCodedFrame(app, cpu, e.esp, e.eax, e.edx));
}

// ------------------------------------------------------- conversions

/* 0x5008a0 (cdecl: count, 16-bit samples, where): each to a float.  Every
 * register kept (the code pushes them all). */
void convert16(App* app, CPU& cpu)
{
    const Entry e(cpu);
    shortsToFloats(app, int32_t(e.arg(app, 0)), e.arg(app, 1), e.arg(app, 2));
    e.back(cpu, e.eax);
}

/* 0x500910 (cdecl: frames, stereo floats, where): each frame to a 16-bit
 * sample.  eax where the loop stopped. */
void convertToWords(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, floatsToWords(app, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2)));
}

/* 0x500990 (cdecl: the state): stereo XA by the x87.  Every register kept. */
void convertXaStereo(App* app, CPU& cpu)
{
    const Entry e(cpu);
    decodeXaStereoFloats(app, e.arg(app, 0));
    e.back(cpu, e.eax);
}

/* 0x500b10 (cdecl: count, 8-bit samples, where): each to a float by a table.
 * Every register kept. */
void convert8(App* app, CPU& cpu)
{
    const Entry e(cpu);
    bytesToFloats(app, int32_t(e.arg(app, 0)), e.arg(app, 1), e.arg(app, 2));
    e.back(cpu, e.eax);
}

/* 0x500b90 (cdecl: the state): mono XA by the x87.  Every register kept. */
void convertXaMono(App* app, CPU& cpu)
{
    const Entry e(cpu);
    decodeXaMonoFloats(app, e.arg(app, 0));
    e.back(cpu, e.eax);
}

}  // namespace port
}  // namespace nfs3hp
