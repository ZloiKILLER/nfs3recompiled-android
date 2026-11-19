#include "native_mix.h"
#include <algorithm>
#include <cstring>
#include <utility>

/* EA SND's mixing routines, the MMX set (the only one the port's mixer is
 * given): adding a block into the accumulator at its volume, the accumulator
 * into the output, interpolating, decoding EA's XA ADPCM -- and the memmove's
 * forward copy.  Here they are ordinary loops over 16-bit samples that give
 * the MMX code's samples to the bit.
 *
 * Their counts run as the x86's did, signed and at least once where its loops
 * did.  What a block's 8-bit samples get in their low bytes is what the last
 * block left in the register the game's code unpacks them with: it hardly moves
 * a product's high word, but it is the sound the game makes, so it is kept. */

namespace nfs3hp
{
namespace mix
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using nfs::u16;
using nfs::u32;
using nfs::u64;
using nfs::u8;
using nfs::s16;
using nfs::s32;
using nfs::s8;

u16 at16(nfs::App* app, u32 address)
{
    return app->getMemory<u16>(address);
}

void put16(nfs::App* app, u32 address, u16 value)
{
    app->getMemory<u16>(address) = value;
}

s16 sample(nfs::App* app, u32 address)
{
    return s16(at16(app, address));
}

/* The high word of a signed product of words. */
s16 high(s32 a, s32 b)
{
    return s16(u32(a * b) >> 16);
}

s16 clamp16(s32 v)
{
    return s16(v < -32768 ? -32768 : v > 32767 ? 32767 : v);
}

s8 clamp8(s32 v)
{
    return s8(v < -128 ? -128 : v > 127 ? 127 : v);
}

/* A word of the accumulator with `amount` added, saturating. */
void addTo(nfs::App* app, u32 address, s32 amount)
{
    put16(app, address, u16(clamp16(sample(app, address) + amount)));
}

/* A cdecl argument: the i-th dword above the return address. */
u32 argument(nfs::App* app, const nfs::ThreadState& ts, unsigned i)
{
    return at(app, ts.sp + 4 + 4 * i);
}

/* Signed comparisons as the x86's jl and jg take them after a sub, or a cmp:
 * of the 32-bit values, before the subtraction. */
bool less(u32 a, s32 b)
{
    return s32(a) < b;
}

bool greater(u32 a, s32 b)
{
    return s32(a) > b;
}

/* The adds' loops' first steps: four (or eight) frames at a time, unless
 * the count is not a multiple and is under them -- then straight to the tail,
 * and the count less them. */
bool straightToTail(u32& count, u32 block)
{
    if ((count & (block - 1)) == 0)
        return false;
    const bool tail = less(count, s32(block));
    count -= block;
    return tail;
}

/* 16.16 positions: the fraction's add and the whole's add with its carry. */
void step(u32& fraction, u32& position, u32 stepFraction, u32 stepWhole)
{
    const u32 sum = fraction + stepFraction;
    const u32 carry = sum < fraction ? 1 : 0;
    fraction = sum;
    position += stepWhole + carry;
}

/* The volumes: four words. */
struct Volumes
{
    s16 word[4];

    Volumes(nfs::App* app, u32 address)
    {
        for (unsigned i = 0; i < 4; ++i)
            word[i] = sample(app, address + 2 * i);
    }
};

/* The XA decoders' table of nibbles, shifted: 16 dwords for each shift. */
constexpr u32 kNibbles = 0xa0bea0;
/* XA's filters, two words each: the first sample's and the second's. */
constexpr u32 kFilterFirst = 0x56ad70, kFilterSecond = 0x56ad80;
/* outputStereo8's xor, 0x80 a byte: signed to unsigned. */
constexpr u32 kSignBits = 0x56ad00;

/* The interpolators' weights: two words made of a 16.16 fraction,
 * (f >> 16 | f) ^ 0xffff, the first for a frame and the second for the next. */
struct Weights
{
    u16 first, second;

    explicit Weights(u32 made)
        : first(u16(made))
        , second(u16(made >> 16))
    {
    }
    static Weights of(u32 fraction)
    {
        return Weights(((fraction >> 16) | fraction) ^ 0xffff);
    }
    /* What they move by each frame. */
    static Weights byStep(u32 stepFraction)
    {
        return Weights((((stepFraction - 1) >> 16) | stepFraction) ^ 0xffff);
    }
    void move(const Weights& by)
    {
        first = u16(first + by.first);
        second = u16(second + by.second);
    }
    /* A word and the next, weighted. */
    u32 mix(s16 a, s16 b) const
    {
        return u32(s32(a) * s32(first >> 1)) + u32(s32(b) * s32(second >> 1));
    }
};

}  // namespace

/* 0x51f5dd: a mono block of 8-bit samples into the stereo accumulator, at
 * the two volumes', four frames at a time, then one at a time; cdecl (volumes,
 * count, source, accumulator). */
nfs::u32 addMono8(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Volumes left(app, argument(app, ts, 0)), right(app, argument(app, ts, 0) + 8);
    u32 count = argument(app, ts, 1), source = argument(app, ts, 2), target = argument(app, ts, 3);
    bool tail = straightToTail(count, 4);
    if (!tail)
    {
        u16 words[4] = {};
        bool more;
        do
        {
            const u8 lows[4] = {u8(words[0]), u8(words[0] >> 8), u8(words[1]), u8(words[1] >> 8)};
            for (unsigned i = 0; i < 4; ++i)
                words[i] = u16(u16(byte(app, source + i)) << 8 | lows[i]);
            for (unsigned i = 0; i < 4; ++i)
            {
                addTo(app, target + 4 * i, high(left.word[i], s16(words[i])));
                addTo(app, target + 4 * i + 2, high(right.word[i], s16(words[i])));
            }
            source += 4;
            target += 0x10;
            more = greater(count, 4);
            count -= 4;
        } while (more);
        tail = count != 0;
    }
    if (tail)
    {
        count = 0 - (count + 4);
        bool more;
        do
        {
            const s16 word = s16(u16(byte(app, source)) << 8);
            addTo(app, target, high(left.word[0], word));
            addTo(app, target + 2, high(right.word[0], word));
            source += 1;
            target += 4;
            more = less(count, -1);
            count += 1;
        } while (more);
    }
    ts.sp += 4;
    return args.eax;
}

/* 0x51f6c7: a stereo block of 8-bit samples into the accumulator, the same
 * way, four frames, then one; cdecl (volume, count, source, accumulator). */
nfs::u32 addStereo8(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Volumes volume(app, argument(app, ts, 0));
    u32 count = argument(app, ts, 1), source = argument(app, ts, 2), target = argument(app, ts, 3);
    bool tail = straightToTail(count, 4);
    if (!tail)
    {
        u16 first[4] = {}, second[4] = {};
        bool more;
        do
        {
            const u8 lowFirst[4] = {u8(first[0]), u8(first[0] >> 8), u8(first[1]), u8(first[1] >> 8)};
            const u8 lowSecond[4] = {u8(second[2]), u8(second[2] >> 8), u8(second[3]), u8(second[3] >> 8)};
            for (unsigned i = 0; i < 4; ++i)
            {
                first[i] = u16(u16(byte(app, source + i)) << 8 | lowFirst[i]);
                second[i] = u16(u16(byte(app, source + 4 + i)) << 8 | lowSecond[i]);
            }
            for (unsigned i = 0; i < 4; ++i)
            {
                addTo(app, target + 2 * i, high(volume.word[i], s16(first[i])));
                addTo(app, target + 8 + 2 * i, high(volume.word[i], s16(second[i])));
            }
            source += 8;
            target += 0x10;
            more = greater(count, 4);
            count -= 4;
        } while (more);
        tail = count != 0;
    }
    if (tail)
    {
        count = 0 - (count + 4);
        bool more;
        do
        {
            addTo(app, target, high(volume.word[0], s16(u16(byte(app, source)) << 8)));
            addTo(app, target + 2, high(volume.word[1], s16(u16(byte(app, source + 1)) << 8)));
            source += 2;
            target += 4;
            more = less(count, -1);
            count += 1;
        } while (more);
    }
    ts.sp += 4;
    return args.eax;
}

/* 0x51f7ad: a mono block of 16-bit samples into the stereo accumulator at the
 * two volumes', four frames at a time, then one; cdecl (volumes, count,
 * source, accumulator). */
nfs::u32 addMono16(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Volumes left(app, argument(app, ts, 0)), right(app, argument(app, ts, 0) + 8);
    u32 count = argument(app, ts, 1), source = argument(app, ts, 2), target = argument(app, ts, 3);
    bool tail = straightToTail(count, 4);
    if (!tail)
    {
        bool more;
        do
        {
            for (unsigned i = 0; i < 4; ++i)
            {
                const s16 value = sample(app, source + 2 * i);
                addTo(app, target + 4 * i, high(left.word[i], value));
                addTo(app, target + 4 * i + 2, high(right.word[i], value));
            }
            source += 8;
            target += 0x10;
            more = greater(count, 4);
            count -= 4;
        } while (more);
        tail = count != 0;
    }
    if (tail)
    {
        count = 0 - (count + 4);
        bool more;
        do
        {
            const s16 value = sample(app, source);
            addTo(app, target, high(left.word[0], value));
            addTo(app, target + 2, high(right.word[0], value));
            source += 2;
            target += 4;
            more = less(count, -1);
            count += 1;
        } while (more);
    }
    ts.sp += 4;
    return args.eax;
}

/* 0x51f897: a stereo block of 16-bit samples into the accumulator at its
 * volume, eight frames at a time, then one; cdecl (volume, count, source,
 * accumulator). */
nfs::u32 addStereo16(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const Volumes volume(app, argument(app, ts, 0));
    u32 count = argument(app, ts, 1), source = argument(app, ts, 2), target = argument(app, ts, 3);
    bool tail = straightToTail(count, 8);
    if (!tail)
    {
        bool more;
        do
        {
            for (unsigned i = 0; i < 16; ++i)
                addTo(app, target + 2 * i, high(volume.word[i % 4], sample(app, source + 2 * i)));
            source += 0x20;
            target += 0x20;
            more = greater(count, 8);
            count -= 8;
        } while (more);
        tail = count != 0;
    }
    if (tail)
    {
        count = 0 - (count + 8);
        bool more;
        do
        {
            addTo(app, target, high(volume.word[0], sample(app, source)));
            addTo(app, target + 2, high(volume.word[1], sample(app, source + 2)));
            source += 4;
            target += 4;
            more = less(count, -1);
            count += 1;
        } while (more);
    }
    ts.sp += 4;
    return args.eax;
}

/* 0x520830: the accumulator's frames, left and right added with saturation,
 * into 8-bit unsigned mono, last frame first; cdecl (count, accumulator,
 * output). */
nfs::u32 outputMono8(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 count = argument(app, ts, 0);
    const u32 source = argument(app, ts, 1), target = argument(app, ts, 2);
    bool more;
    do
    {
        const s16 both = clamp16(sample(app, source + count * 4 - 4) + sample(app, source + count * 4 - 2));
        putByte(app, count + target - 1, u8((u16(both) ^ 0x8000) >> 8));
        more = greater(count, 1);
        count -= 1;
    } while (more);
    ts.sp += 4;
    return args.eax;
}

/* 0x520870: the accumulator doubled with saturation into 8-bit unsigned
 * stereo -- the high bytes, the sign flipped by 0x56ad00's -- 32 bytes at a time
 * from the end; cdecl (frames, accumulator, output). */
nfs::u32 outputStereo8(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 count = argument(app, ts, 0) << 1;
    const u32 source = argument(app, ts, 1), target = argument(app, ts, 2);
    bool more;
    do
    {
        u8 bytes[32];
        for (unsigned i = 0; i < 32; ++i)
        {
            const s16 doubled = clamp16(2 * s32(sample(app, source + count * 2 - 64 + 2 * i)));
            bytes[i] = u8(u16(doubled) >> 8) ^ byte(app, kSignBits + i % 8);
        }
        for (unsigned i = 32; i-- > 0;)
            putByte(app, count + target - 32 + i, bytes[i]);
        more = greater(count, 0x20);
        count -= 0x20;
    } while (more);
    ts.sp += 4;
    return args.eax;
}

/* 0x520920: the accumulator doubled with saturation into 16-bit stereo, 64
 * bytes at a time from the end; cdecl (frames, accumulator, output). */
nfs::u32 outputStereo16(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 count = argument(app, ts, 0) << 2;
    const u32 source = argument(app, ts, 1), target = argument(app, ts, 2);
    bool more;
    do
    {
        u16 words[32];
        for (unsigned i = 0; i < 32; ++i)
            words[i] = u16(clamp16(2 * s32(sample(app, count + source - 64 + 2 * i))));
        for (unsigned i = 32; i-- > 0;)
            put16(app, count + target - 64 + 2 * i, words[i]);
        more = greater(count, 0x40);
        count -= 0x40;
    } while (more);
    ts.sp += 4;
    return args.eax;
}

/* 0x5209a0: the accumulator cleared; cdecl (accumulator, dwords). */
nfs::u32 clear(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const s32 count = s32(argument(app, ts, 1));
    const u32 target = argument(app, ts, 0);
    for (s32 i = 0; i < count; ++i)
        put(app, target + 4 * u32(i), 0);
    ts.sp += 4;
    return args.eax;
}

/* 0x520a00: what the game's mixing ends with, to give the MMX registers back. */
nfs::u32 finished(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x520a10: the accumulator's frames, left and right added with saturation,
 * into 16-bit mono, last frame first; cdecl (count, accumulator, output). */
nfs::u32 outputMono16(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    u32 count = argument(app, ts, 0);
    const u32 source = argument(app, ts, 1), target = argument(app, ts, 2);
    bool more;
    do
    {
        const s16 both = clamp16(sample(app, source + count * 4 - 4) + sample(app, source + count * 4 - 2));
        put16(app, target + count * 2 - 2, u16(both));
        more = greater(count, 1);
        count -= 1;
    } while (more);
    ts.sp += 4;
    return args.eax;
}

/* The interpolators' arguments: cdecl (frames, source, output, the whole
 * position's dword, the fraction's, the step's whole, its fraction); both
 * positions written back at the end. */
struct Interpolation
{
    nfs::App* app;
    u32 wholeAt, fractionAt;
    u32 count, source, target, stepWhole, stepFraction, fraction, whole;

    Interpolation(nfs::App* app, const Interpolate& a)
        : app(app)
        , wholeAt(a.wholeAt)
        , fractionAt(a.fractionAt)
        , count(a.count)
        , source(a.source)
        , target(a.target)
        , stepWhole(a.stepWhole)
        , stepFraction(a.stepFraction)
        , fraction(at(app, a.fractionAt))
        , whole(at(app, a.wholeAt))
    {
    }
    /* A frame on: the output on, the count down. */
    bool next(u32 size)
    {
        target += size;
        const bool more = greater(count, 1);
        count -= 1;
        return more;
    }
    /* The positions back where the arguments point. */
    void end()
    {
        put(app, fractionAt, fraction);
        put(app, wholeAt, whole);
    }
};

/* An interpolator's words where its cdecl caller pushed them. */
Interpolate interpolateWords(nfs::App* app, const nfs::ThreadState& ts)
{
    return {argument(app, ts, 0), argument(app, ts, 1), argument(app, ts, 2), argument(app, ts, 3),
            argument(app, ts, 4), argument(app, ts, 5), argument(app, ts, 6)};
}

/* 0x5270f0: 8-bit mono interpolated between a frame and the next, by the
 * position's fraction, into 8 bits. */
void interpolatedMono8(nfs::App* app, const Interpolate& a)
{
    Interpolation s(app, a);
    Weights weights = Weights::of(s.fraction);
    const Weights by = Weights::byStep(s.stepFraction);
    u32 last = 0;
    do
    {
        const s16 a = s16(u16(byte(app, s.whole + s.source)) << 8 | u8(last));
        const s16 b = s16(u16(byte(app, s.whole + s.source + 1)) << 8 | u8(last >> 8));
        last = weights.mix(a, b) >> 23;
        weights.move(by);
        putByte(app, s.target, u8(last));
        step(s.fraction, s.whole, s.stepFraction, s.stepWhole);
    } while (s.next(1));
    s.end();
}

/* 0x527170: 8-bit stereo, the same, into 8 bits with signed saturation; a
 * dword written for each two bytes, as the game's code does. */
void interpolatedStereo8(nfs::App* app, const Interpolate& a)
{
    Interpolation s(app, a);
    Weights weights = Weights::of(s.fraction);
    const Weights by = Weights::byStep(s.stepFraction);
    u8 last[4] = {};
    do
    {
        const u32 here = s.source + s.whole * 2;
        const u8 a0 = byte(app, here), a1 = byte(app, here + 1), c0 = byte(app, here + 2), c1 = byte(app, here + 3);
        const u32 wordsOf0 = u32(a0) | u32(c0) << 8 | u32(a1) << 16 | u32(c1) << 24;
        const u32 wordsOf1 = u32(c0) | u32(byte(app, here + 4)) << 8 | u32(c1) << 16
                               | u32(byte(app, here + 5)) << 24;
        const s32 first = s32(weights.mix(s16(u16(a0) << 8 | last[0]), s16(u16(c0) << 8 | last[1]))) >> 23;
        const s32 second = s32(weights.mix(s16(u16(a1) << 8 | last[2]), s16(u16(c1) << 8 | last[3]))) >> 23;
        weights.move(by);
        last[0] = u8(clamp8(clamp16(first)));
        last[1] = u8(clamp8(clamp16(second)));
        last[2] = u8(clamp8(clamp16(s32(wordsOf0))));
        last[3] = u8(clamp8(clamp16(s32(wordsOf1))));
        put(app, s.target, u32(last[0]) | u32(last[1]) << 8 | u32(last[2]) << 16 | u32(last[3]) << 24);
        step(s.fraction, s.whole, s.stepFraction, s.stepWhole);
    } while (s.next(2));
    s.end();
}

/* 0x527210: 16-bit mono, the same, truncated into 16 bits. */
void interpolatedMono16(nfs::App* app, const Interpolate& a)
{
    Interpolation s(app, a);
    Weights weights = Weights::of(s.fraction);
    const Weights by = Weights::byStep(s.stepFraction);
    do
    {
        const u32 here = s.source + s.whole * 2;
        const u32 mixed = weights.mix(sample(app, here), sample(app, here + 2));
        weights.move(by);
        put16(app, s.target, u16(mixed >> 15));
        step(s.fraction, s.whole, s.stepFraction, s.stepWhole);
    } while (s.next(2));
    s.end();
}

/* 0x527290: 16-bit stereo, the same, into 16 bits with signed saturation. */
void interpolatedStereo16(nfs::App* app, const Interpolate& a)
{
    Interpolation s(app, a);
    Weights weights = Weights::of(s.fraction);
    const Weights by = Weights::byStep(s.stepFraction);
    do
    {
        const u32 here = s.source + s.whole * 4;
        const s32 left = s32(weights.mix(sample(app, here), sample(app, here + 4))) >> 15;
        const s32 right = s32(weights.mix(sample(app, here + 2), sample(app, here + 6))) >> 15;
        weights.move(by);
        put16(app, s.target, u16(clamp16(left)));
        put16(app, s.target + 2, u16(clamp16(right)));
        step(s.fraction, s.whole, s.stepFraction, s.stepWhole);
    } while (s.next(4));
    s.end();
}

/* 0x52d1a0: mono XA -- [+0] samples, whole blocks of 28 of 15 bytes each from
 * [+8], a header's filter and shift then 14 bytes of nibbles -- into 16-bit
 * samples, two to a dword, from [+0xc]; the last two samples, the filter's
 * history, kept in [+4].  cdecl (the decoder's state). */
void xaMonoDecoded(nfs::App* app, u32 state)
{
    u32 source = at(app, state + 8), target = at(app, state + 0xc);
    const u32 history = at(app, state + 4);
    s16 older = s16(u16(history)), newer = s16(u16(history >> 16));
    while (greater(at(app, state), 0))
    {
        const u32 header = byte(app, source);
        put(app, state, at(app, state) - 0x1c);
        target += 0x38;
        const u32 first = at(app, (header >> 4) * 4 + kFilterFirst);
        const u32 second = at(app, (header >> 4) * 4 + kFilterSecond);
        const s32 first0 = s16(u16(first)), first1 = s16(u16(first >> 16));
        const s32 second0 = s16(u16(second)), second1 = s16(u16(second >> 16));
        const u32 nibbles = kNibbles + ((header & 0xf) << 6);
        source += 0xf;
        for (u32 i = 0xfffffff2; ; ++i)
        {
            const u32 packed = byte(app, i + source);
            const u32 a = u32(older * first0) + u32(newer * first1) + at(app, nibbles + (packed >> 4) * 4);
            const s16 made = s16(u16((a << 8) >> 16));
            const u32 b = u32(made * second0) + u32(newer * second1) + at(app, nibbles + (packed & 0xf) * 4);
            const s16 next = s16(u16(b >> 8));
            put(app, target + i * 4, u32(u16(made)) | u32(u16(next)) << 16);
            older = made;
            newer = next;
            if (!less(i, -1))
                break;
        }
    }
    put(app, state + 4, u32(u16(older)) | u32(u16(newer)) << 16);
    put(app, state + 0xc, target);
}

/* 0x52d250: stereo XA -- 30-byte blocks of 28 frames from [+0xc], a header
 * for each side then their nibbles interleaved -- into 16-bit frames from
 * [+0x10]; the history in [+4] and [+8].  cdecl (the decoder's state). */
void xaStereoDecoded(nfs::App* app, u32 state)
{
    u32 source = at(app, state + 0xc), target = at(app, state + 0x10);
    u32 history[2] = {at(app, state + 4), at(app, state + 8)};
    while (greater(at(app, state), 0))
    {
        const u32 filters = byte(app, source);
        put(app, state, at(app, state) - 0x1c);
        target += 0x70;
        const u32 first[2] = {at(app, (filters >> 4) * 4 + kFilterFirst), at(app, (filters & 0xf) * 4 + kFilterFirst)};
        const u32 second[2] = {at(app, (filters >> 4) * 4 + kFilterSecond),
                                 at(app, (filters & 0xf) * 4 + kFilterSecond)};
        const u32 shifts = byte(app, source + 1);
        const u32 nibbles[2] = {kNibbles + ((shifts >> 4) << 6), kNibbles + ((shifts & 0xf) << 6)};
        source += 0x1e;
        for (u32 i = 0xffffffe4; ; i += 2)
        {
            const u8 packed[2] = {byte(app, i + source), byte(app, i + source + 1)};
            for (unsigned side = 0; side < 2; ++side)
            {
                const unsigned nibbleA = side == 0 ? packed[0] >> 4 : packed[0] & 0xf;
                const unsigned nibbleB = side == 0 ? packed[1] >> 4 : packed[1] & 0xf;
                const s32 h0 = s16(u16(history[side])), h1 = s16(u16(history[side] >> 16));
                const u32 a = u32(h0 * s16(u16(first[side])))
                                + u32(h1 * s16(u16(first[side] >> 16))) + at(app, nibbles[side] + nibbleA * 4);
                const u16 made = u16(a >> 8);
                const u32 b = u32(s32(s16(made)) * s16(u16(second[side])))
                                + u32(h1 * s16(u16(second[side] >> 16))) + at(app, nibbles[side] + nibbleB * 4);
                history[side] = u32(made) | ((b << 8) & 0xffff0000);
            }
            const u32 frames = target + i * 4;
            put16(app, frames, u16(history[0]));
            put16(app, frames + 2, u16(history[1]));
            put16(app, frames + 4, u16(history[0] >> 16));
            put16(app, frames + 6, u16(history[1] >> 16));
            if (!less(i, -2))
                break;
        }
    }
    put(app, state + 4, history[0]);
    put(app, state + 8, history[1]);
    put(app, state + 0x10, target);
}

/* Their entries, the game's cdecl calls: the words where the caller pushed
 * them, the return address popped. */
nfs::u32 interpolateMono8(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    interpolatedMono8(app, interpolateWords(app, ts));
    ts.sp += 4;
    return args.eax;
}

nfs::u32 interpolateStereo8(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    interpolatedStereo8(app, interpolateWords(app, ts));
    ts.sp += 4;
    return args.eax;
}

nfs::u32 interpolateMono16(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    interpolatedMono16(app, interpolateWords(app, ts));
    ts.sp += 4;
    return args.eax;
}

nfs::u32 interpolateStereo16(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    interpolatedStereo16(app, interpolateWords(app, ts));
    ts.sp += 4;
    return args.eax;
}

nfs::u32 xaMono(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    xaMonoDecoded(app, argument(app, ts, 0));
    ts.sp += 4;
    return args.eax;
}

nfs::u32 xaStereo(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    xaStereoDecoded(app, argument(app, ts, 0));
    ts.sp += 4;
    return args.eax;
}

namespace
{

/* The memmove's forward copy, eax to, edx from, ebx bytes, eax, edx and ebx left
 * past them as the game's code leaves them. */
void copyForward(nfs::App* app, nfs::Args& args)
{
    const u32 bytes = args.ebx;
    std::memmove(&app->getMemory<u8>(args.eax), &app->getMemory<u8>(args.edx), bytes);
    args.eax += bytes;
    args.edx += bytes;
    args.ebx = 0;
}

}  // namespace

/* 0x5033e5: the memmove's forward copy, eax to, edx from, ebx bytes. */
nfs::u32 copy(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    copyForward(app, args);
    ts.sp += 4;
    return args.eax;
}

/* 0x5033e4: the same, eax from and edx to (xchg first). */
nfs::u32 copySwapped(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    std::swap(args.eax, args.edx);
    copyForward(app, args);
    ts.sp += 4;
    return args.eax;
}

}  // namespace mix
}  // namespace nfs3hp
