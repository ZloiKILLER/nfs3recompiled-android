#include "native_guest.h"
#include <algorithm>
#include <cstring>
#include <utility>

/* EA SND's mixing, plain: adding a block into the accumulator at its volume, the
 * accumulator into the output, interpolating, decoding EA's XA ADPCM -- and the
 * memmove's forward copy.  The game has these written for MMX; here they are
 * ordinary loops over 16-bit samples that give the same samples to the bit.
 * disassemble_nfs3hp.py binds each address to the function of that name here
 * (MIX_NATIVES), the generated code calls it by name, and tools/game_checks holds
 * each against the game's code.
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
using x86::reg16;
using x86::reg32;
using x86::reg64;
using x86::reg8;
using x86::sreg16;
using x86::sreg32;
using x86::sreg8;

reg16 at16(win32::WinApplication* app, reg32 address)
{
    return app->getMemory<reg16>(address);
}

void put16(win32::WinApplication* app, reg32 address, reg16 value)
{
    app->getMemory<reg16>(address) = value;
}

sreg16 sample(win32::WinApplication* app, reg32 address)
{
    return sreg16(at16(app, address));
}

/* The high word of a signed product of words. */
sreg16 high(sreg32 a, sreg32 b)
{
    return sreg16(reg32(a * b) >> 16);
}

sreg16 clamp16(sreg32 v)
{
    return sreg16(v < -32768 ? -32768 : v > 32767 ? 32767 : v);
}

sreg8 clamp8(sreg32 v)
{
    return sreg8(v < -128 ? -128 : v > 127 ? 127 : v);
}

/* A word of the accumulator with `amount` added, saturating. */
void addTo(win32::WinApplication* app, reg32 address, sreg32 amount)
{
    put16(app, address, reg16(clamp16(sample(app, address) + amount)));
}

/* A cdecl argument: the i-th dword above the return address. */
reg32 argument(win32::WinApplication* app, const x86::CPU& cpu, unsigned i)
{
    return at(app, cpu.esp + 4 + 4 * i);
}

/* Signed comparisons as the x86's jl and jg take them after a sub, or a cmp:
 * of the 32-bit values, before the subtraction. */
bool less(reg32 a, sreg32 b)
{
    return sreg32(a) < b;
}

bool greater(reg32 a, sreg32 b)
{
    return sreg32(a) > b;
}

/* The adds' loops' first steps: four (or eight) frames at a time, unless
 * the count is not a multiple and is under them -- then straight to the tail,
 * and the count less them. */
bool straightToTail(reg32& count, reg32 block)
{
    if ((count & (block - 1)) == 0)
        return false;
    const bool tail = less(count, sreg32(block));
    count -= block;
    return tail;
}

/* 16.16 positions: the fraction's add and the whole's add with its carry. */
void step(reg32& fraction, reg32& position, reg32 stepFraction, reg32 stepWhole)
{
    const reg32 sum = fraction + stepFraction;
    const reg32 carry = sum < fraction ? 1 : 0;
    fraction = sum;
    position += stepWhole + carry;
}

/* The volumes: four words. */
struct Volumes
{
    sreg16 word[4];

    Volumes(win32::WinApplication* app, reg32 address)
    {
        for (unsigned i = 0; i < 4; ++i)
            word[i] = sample(app, address + 2 * i);
    }
};

/* The XA decoders' table of nibbles, shifted: 16 dwords for each shift. */
constexpr reg32 kNibbles = 0xa0bea0;
/* XA's filters, two words each: the first sample's and the second's. */
constexpr reg32 kFilterFirst = 0x56ad70, kFilterSecond = 0x56ad80;
/* outputStereo8's xor, 0x80 a byte: signed to unsigned. */
constexpr reg32 kSignBits = 0x56ad00;

/* The interpolators' weights: two words made of a 16.16 fraction,
 * (f >> 16 | f) ^ 0xffff, the first for a frame and the second for the next. */
struct Weights
{
    reg16 first, second;

    explicit Weights(reg32 made)
        : first(reg16(made))
        , second(reg16(made >> 16))
    {
    }
    static Weights of(reg32 fraction)
    {
        return Weights(((fraction >> 16) | fraction) ^ 0xffff);
    }
    /* What they move by each frame. */
    static Weights byStep(reg32 stepFraction)
    {
        return Weights((((stepFraction - 1) >> 16) | stepFraction) ^ 0xffff);
    }
    void move(const Weights& by)
    {
        first = reg16(first + by.first);
        second = reg16(second + by.second);
    }
    /* A word and the next, weighted. */
    reg32 mix(sreg16 a, sreg16 b) const
    {
        return reg32(sreg32(a) * sreg32(first >> 1)) + reg32(sreg32(b) * sreg32(second >> 1));
    }
};

}  // namespace

/* 0x51f5dd: a mono block of 8-bit samples into the stereo accumulator, at
 * the two volumes', four frames at a time, then one at a time; cdecl (volumes,
 * count, source, accumulator). */
void addMono8(win32::WinApplication* app, x86::CPU& cpu)
{
    const Volumes left(app, argument(app, cpu, 0)), right(app, argument(app, cpu, 0) + 8);
    reg32 count = argument(app, cpu, 1), source = argument(app, cpu, 2), target = argument(app, cpu, 3);
    bool tail = straightToTail(count, 4);
    if (!tail)
    {
        reg16 words[4] = {};
        bool more;
        do
        {
            const reg8 lows[4] = {reg8(words[0]), reg8(words[0] >> 8), reg8(words[1]), reg8(words[1] >> 8)};
            for (unsigned i = 0; i < 4; ++i)
                words[i] = reg16(reg16(byte(app, source + i)) << 8 | lows[i]);
            for (unsigned i = 0; i < 4; ++i)
            {
                addTo(app, target + 4 * i, high(left.word[i], sreg16(words[i])));
                addTo(app, target + 4 * i + 2, high(right.word[i], sreg16(words[i])));
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
            const sreg16 word = sreg16(reg16(byte(app, source)) << 8);
            addTo(app, target, high(left.word[0], word));
            addTo(app, target + 2, high(right.word[0], word));
            source += 1;
            target += 4;
            more = less(count, -1);
            count += 1;
        } while (more);
    }
    cpu.esp += 4;
}

/* 0x51f6c7: a stereo block of 8-bit samples into the accumulator, the same
 * way, four frames, then one; cdecl (volume, count, source, accumulator). */
void addStereo8(win32::WinApplication* app, x86::CPU& cpu)
{
    const Volumes volume(app, argument(app, cpu, 0));
    reg32 count = argument(app, cpu, 1), source = argument(app, cpu, 2), target = argument(app, cpu, 3);
    bool tail = straightToTail(count, 4);
    if (!tail)
    {
        reg16 first[4] = {}, second[4] = {};
        bool more;
        do
        {
            const reg8 lowFirst[4] = {reg8(first[0]), reg8(first[0] >> 8), reg8(first[1]), reg8(first[1] >> 8)};
            const reg8 lowSecond[4] = {reg8(second[2]), reg8(second[2] >> 8), reg8(second[3]), reg8(second[3] >> 8)};
            for (unsigned i = 0; i < 4; ++i)
            {
                first[i] = reg16(reg16(byte(app, source + i)) << 8 | lowFirst[i]);
                second[i] = reg16(reg16(byte(app, source + 4 + i)) << 8 | lowSecond[i]);
            }
            for (unsigned i = 0; i < 4; ++i)
            {
                addTo(app, target + 2 * i, high(volume.word[i], sreg16(first[i])));
                addTo(app, target + 8 + 2 * i, high(volume.word[i], sreg16(second[i])));
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
            addTo(app, target, high(volume.word[0], sreg16(reg16(byte(app, source)) << 8)));
            addTo(app, target + 2, high(volume.word[1], sreg16(reg16(byte(app, source + 1)) << 8)));
            source += 2;
            target += 4;
            more = less(count, -1);
            count += 1;
        } while (more);
    }
    cpu.esp += 4;
}

/* 0x51f7ad: a mono block of 16-bit samples into the stereo accumulator at the
 * two volumes', four frames at a time, then one; cdecl (volumes, count,
 * source, accumulator). */
void addMono16(win32::WinApplication* app, x86::CPU& cpu)
{
    const Volumes left(app, argument(app, cpu, 0)), right(app, argument(app, cpu, 0) + 8);
    reg32 count = argument(app, cpu, 1), source = argument(app, cpu, 2), target = argument(app, cpu, 3);
    bool tail = straightToTail(count, 4);
    if (!tail)
    {
        bool more;
        do
        {
            for (unsigned i = 0; i < 4; ++i)
            {
                const sreg16 value = sample(app, source + 2 * i);
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
            const sreg16 value = sample(app, source);
            addTo(app, target, high(left.word[0], value));
            addTo(app, target + 2, high(right.word[0], value));
            source += 2;
            target += 4;
            more = less(count, -1);
            count += 1;
        } while (more);
    }
    cpu.esp += 4;
}

/* 0x51f897: a stereo block of 16-bit samples into the accumulator at its
 * volume, eight frames at a time, then one; cdecl (volume, count, source,
 * accumulator). */
void addStereo16(win32::WinApplication* app, x86::CPU& cpu)
{
    const Volumes volume(app, argument(app, cpu, 0));
    reg32 count = argument(app, cpu, 1), source = argument(app, cpu, 2), target = argument(app, cpu, 3);
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
    cpu.esp += 4;
}

/* 0x520830: the accumulator's frames, left and right added with saturation,
 * into 8-bit unsigned mono, last frame first; cdecl (count, accumulator,
 * output). */
void outputMono8(win32::WinApplication* app, x86::CPU& cpu)
{
    reg32 count = argument(app, cpu, 0);
    const reg32 source = argument(app, cpu, 1), target = argument(app, cpu, 2);
    bool more;
    do
    {
        const sreg16 both = clamp16(sample(app, source + count * 4 - 4) + sample(app, source + count * 4 - 2));
        putByte(app, count + target - 1, reg8((reg16(both) ^ 0x8000) >> 8));
        more = greater(count, 1);
        count -= 1;
    } while (more);
    cpu.esp += 4;
}

/* 0x520870: the accumulator doubled with saturation into 8-bit unsigned
 * stereo -- the high bytes, the sign flipped by 0x56ad00's -- 32 bytes at a time
 * from the end; cdecl (frames, accumulator, output). */
void outputStereo8(win32::WinApplication* app, x86::CPU& cpu)
{
    reg32 count = argument(app, cpu, 0) << 1;
    const reg32 source = argument(app, cpu, 1), target = argument(app, cpu, 2);
    bool more;
    do
    {
        reg8 bytes[32];
        for (unsigned i = 0; i < 32; ++i)
        {
            const sreg16 doubled = clamp16(2 * sreg32(sample(app, source + count * 2 - 64 + 2 * i)));
            bytes[i] = reg8(reg16(doubled) >> 8) ^ byte(app, kSignBits + i % 8);
        }
        for (unsigned i = 32; i-- > 0;)
            putByte(app, count + target - 32 + i, bytes[i]);
        more = greater(count, 0x20);
        count -= 0x20;
    } while (more);
    cpu.esp += 4;
}

/* 0x520920: the accumulator doubled with saturation into 16-bit stereo, 64
 * bytes at a time from the end; cdecl (frames, accumulator, output). */
void outputStereo16(win32::WinApplication* app, x86::CPU& cpu)
{
    reg32 count = argument(app, cpu, 0) << 2;
    const reg32 source = argument(app, cpu, 1), target = argument(app, cpu, 2);
    bool more;
    do
    {
        reg16 words[32];
        for (unsigned i = 0; i < 32; ++i)
            words[i] = reg16(clamp16(2 * sreg32(sample(app, count + source - 64 + 2 * i))));
        for (unsigned i = 32; i-- > 0;)
            put16(app, count + target - 64 + 2 * i, words[i]);
        more = greater(count, 0x40);
        count -= 0x40;
    } while (more);
    cpu.esp += 4;
}

/* 0x5209a0: the accumulator cleared; cdecl (accumulator, dwords). */
void clear(win32::WinApplication* app, x86::CPU& cpu)
{
    const sreg32 count = sreg32(argument(app, cpu, 1));
    const reg32 target = argument(app, cpu, 0);
    for (sreg32 i = 0; i < count; ++i)
        put(app, target + 4 * reg32(i), 0);
    cpu.esp += 4;
}

/* 0x520a00: what the game's mixing ends with, to give the MMX registers back. */
void finished(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    cpu.esp += 4;
}

/* 0x520a10: the accumulator's frames, left and right added with saturation,
 * into 16-bit mono, last frame first; cdecl (count, accumulator, output). */
void outputMono16(win32::WinApplication* app, x86::CPU& cpu)
{
    reg32 count = argument(app, cpu, 0);
    const reg32 source = argument(app, cpu, 1), target = argument(app, cpu, 2);
    bool more;
    do
    {
        const sreg16 both = clamp16(sample(app, source + count * 4 - 4) + sample(app, source + count * 4 - 2));
        put16(app, target + count * 2 - 2, reg16(both));
        more = greater(count, 1);
        count -= 1;
    } while (more);
    cpu.esp += 4;
}

/* The interpolators' arguments: cdecl (frames, source, output, the whole
 * position's dword, the fraction's, the step's whole, its fraction); both
 * positions written back at the end. */
struct Interpolation
{
    win32::WinApplication* app;
    const x86::CPU& cpu;
    reg32 count, source, target, stepWhole, stepFraction, fraction, whole;

    Interpolation(win32::WinApplication* app, const x86::CPU& cpu)
        : app(app)
        , cpu(cpu)
        , count(argument(app, cpu, 0))
        , source(argument(app, cpu, 1))
        , target(argument(app, cpu, 2))
        , stepWhole(argument(app, cpu, 5))
        , stepFraction(argument(app, cpu, 6))
        , fraction(at(app, argument(app, cpu, 4)))
        , whole(at(app, argument(app, cpu, 3)))
    {
    }
    /* A frame on: the output on, the count down. */
    bool next(reg32 size)
    {
        target += size;
        const bool more = greater(count, 1);
        count -= 1;
        return more;
    }
    /* The positions back where the arguments point. */
    void end()
    {
        put(app, argument(app, cpu, 4), fraction);
        put(app, argument(app, cpu, 3), whole);
    }
};

/* 0x5270f0: 8-bit mono interpolated between a frame and the next, by the
 * position's fraction, into 8 bits. */
void interpolateMono8(win32::WinApplication* app, x86::CPU& cpu)
{
    Interpolation s(app, cpu);
    Weights weights = Weights::of(s.fraction);
    const Weights by = Weights::byStep(s.stepFraction);
    reg32 last = 0;
    do
    {
        const sreg16 a = sreg16(reg16(byte(app, s.whole + s.source)) << 8 | reg8(last));
        const sreg16 b = sreg16(reg16(byte(app, s.whole + s.source + 1)) << 8 | reg8(last >> 8));
        last = weights.mix(a, b) >> 23;
        weights.move(by);
        putByte(app, s.target, reg8(last));
        step(s.fraction, s.whole, s.stepFraction, s.stepWhole);
    } while (s.next(1));
    s.end();
    cpu.esp += 4;
}

/* 0x527170: 8-bit stereo, the same, into 8 bits with signed saturation; a
 * dword written for each two bytes, as the game's code does. */
void interpolateStereo8(win32::WinApplication* app, x86::CPU& cpu)
{
    Interpolation s(app, cpu);
    Weights weights = Weights::of(s.fraction);
    const Weights by = Weights::byStep(s.stepFraction);
    reg8 last[4] = {};
    do
    {
        const reg32 here = s.source + s.whole * 2;
        const reg8 a0 = byte(app, here), a1 = byte(app, here + 1), c0 = byte(app, here + 2), c1 = byte(app, here + 3);
        const reg32 wordsOf0 = reg32(a0) | reg32(c0) << 8 | reg32(a1) << 16 | reg32(c1) << 24;
        const reg32 wordsOf1 = reg32(c0) | reg32(byte(app, here + 4)) << 8 | reg32(c1) << 16
                               | reg32(byte(app, here + 5)) << 24;
        const sreg32 first = sreg32(weights.mix(sreg16(reg16(a0) << 8 | last[0]), sreg16(reg16(c0) << 8 | last[1]))) >> 23;
        const sreg32 second = sreg32(weights.mix(sreg16(reg16(a1) << 8 | last[2]), sreg16(reg16(c1) << 8 | last[3]))) >> 23;
        weights.move(by);
        last[0] = reg8(clamp8(clamp16(first)));
        last[1] = reg8(clamp8(clamp16(second)));
        last[2] = reg8(clamp8(clamp16(sreg32(wordsOf0))));
        last[3] = reg8(clamp8(clamp16(sreg32(wordsOf1))));
        put(app, s.target, reg32(last[0]) | reg32(last[1]) << 8 | reg32(last[2]) << 16 | reg32(last[3]) << 24);
        step(s.fraction, s.whole, s.stepFraction, s.stepWhole);
    } while (s.next(2));
    s.end();
    cpu.esp += 4;
}

/* 0x527210: 16-bit mono, the same, truncated into 16 bits. */
void interpolateMono16(win32::WinApplication* app, x86::CPU& cpu)
{
    Interpolation s(app, cpu);
    Weights weights = Weights::of(s.fraction);
    const Weights by = Weights::byStep(s.stepFraction);
    do
    {
        const reg32 here = s.source + s.whole * 2;
        const reg32 mixed = weights.mix(sample(app, here), sample(app, here + 2));
        weights.move(by);
        put16(app, s.target, reg16(mixed >> 15));
        step(s.fraction, s.whole, s.stepFraction, s.stepWhole);
    } while (s.next(2));
    s.end();
    cpu.esp += 4;
}

/* 0x527290: 16-bit stereo, the same, into 16 bits with signed saturation. */
void interpolateStereo16(win32::WinApplication* app, x86::CPU& cpu)
{
    Interpolation s(app, cpu);
    Weights weights = Weights::of(s.fraction);
    const Weights by = Weights::byStep(s.stepFraction);
    do
    {
        const reg32 here = s.source + s.whole * 4;
        const sreg32 left = sreg32(weights.mix(sample(app, here), sample(app, here + 4))) >> 15;
        const sreg32 right = sreg32(weights.mix(sample(app, here + 2), sample(app, here + 6))) >> 15;
        weights.move(by);
        put16(app, s.target, reg16(clamp16(left)));
        put16(app, s.target + 2, reg16(clamp16(right)));
        step(s.fraction, s.whole, s.stepFraction, s.stepWhole);
    } while (s.next(4));
    s.end();
    cpu.esp += 4;
}

/* 0x52d1a0: mono XA -- [+0] samples, whole blocks of 28 of 15 bytes each from
 * [+8], a header's filter and shift then 14 bytes of nibbles -- into 16-bit
 * samples, two to a dword, from [+0xc]; the last two samples, the filter's
 * history, kept in [+4].  cdecl (the decoder's state). */
void xaMono(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 state = argument(app, cpu, 0);
    reg32 source = at(app, state + 8), target = at(app, state + 0xc);
    const reg32 history = at(app, state + 4);
    sreg16 older = sreg16(reg16(history)), newer = sreg16(reg16(history >> 16));
    while (greater(at(app, state), 0))
    {
        const reg32 header = byte(app, source);
        put(app, state, at(app, state) - 0x1c);
        target += 0x38;
        const reg32 first = at(app, (header >> 4) * 4 + kFilterFirst);
        const reg32 second = at(app, (header >> 4) * 4 + kFilterSecond);
        const sreg32 first0 = sreg16(reg16(first)), first1 = sreg16(reg16(first >> 16));
        const sreg32 second0 = sreg16(reg16(second)), second1 = sreg16(reg16(second >> 16));
        const reg32 nibbles = kNibbles + ((header & 0xf) << 6);
        source += 0xf;
        for (reg32 i = 0xfffffff2; ; ++i)
        {
            const reg32 packed = byte(app, i + source);
            const reg32 a = reg32(older * first0) + reg32(newer * first1) + at(app, nibbles + (packed >> 4) * 4);
            const sreg16 made = sreg16(reg16((a << 8) >> 16));
            const reg32 b = reg32(made * second0) + reg32(newer * second1) + at(app, nibbles + (packed & 0xf) * 4);
            const sreg16 next = sreg16(reg16(b >> 8));
            put(app, target + i * 4, reg32(reg16(made)) | reg32(reg16(next)) << 16);
            older = made;
            newer = next;
            if (!less(i, -1))
                break;
        }
    }
    put(app, state + 4, reg32(reg16(older)) | reg32(reg16(newer)) << 16);
    put(app, state + 0xc, target);
    cpu.esp += 4;
}

/* 0x52d250: stereo XA -- 30-byte blocks of 28 frames from [+0xc], a header
 * for each side then their nibbles interleaved -- into 16-bit frames from
 * [+0x10]; the history in [+4] and [+8].  cdecl (the decoder's state). */
void xaStereo(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 state = argument(app, cpu, 0);
    reg32 source = at(app, state + 0xc), target = at(app, state + 0x10);
    reg32 history[2] = {at(app, state + 4), at(app, state + 8)};
    while (greater(at(app, state), 0))
    {
        const reg32 filters = byte(app, source);
        put(app, state, at(app, state) - 0x1c);
        target += 0x70;
        const reg32 first[2] = {at(app, (filters >> 4) * 4 + kFilterFirst), at(app, (filters & 0xf) * 4 + kFilterFirst)};
        const reg32 second[2] = {at(app, (filters >> 4) * 4 + kFilterSecond),
                                 at(app, (filters & 0xf) * 4 + kFilterSecond)};
        const reg32 shifts = byte(app, source + 1);
        const reg32 nibbles[2] = {kNibbles + ((shifts >> 4) << 6), kNibbles + ((shifts & 0xf) << 6)};
        source += 0x1e;
        for (reg32 i = 0xffffffe4; ; i += 2)
        {
            const reg8 packed[2] = {byte(app, i + source), byte(app, i + source + 1)};
            for (unsigned side = 0; side < 2; ++side)
            {
                const unsigned nibbleA = side == 0 ? packed[0] >> 4 : packed[0] & 0xf;
                const unsigned nibbleB = side == 0 ? packed[1] >> 4 : packed[1] & 0xf;
                const sreg32 h0 = sreg16(reg16(history[side])), h1 = sreg16(reg16(history[side] >> 16));
                const reg32 a = reg32(h0 * sreg16(reg16(first[side])))
                                + reg32(h1 * sreg16(reg16(first[side] >> 16))) + at(app, nibbles[side] + nibbleA * 4);
                const reg16 made = reg16(a >> 8);
                const reg32 b = reg32(sreg32(sreg16(made)) * sreg16(reg16(second[side])))
                                + reg32(h1 * sreg16(reg16(second[side] >> 16))) + at(app, nibbles[side] + nibbleB * 4);
                history[side] = reg32(made) | ((b << 8) & 0xffff0000);
            }
            const reg32 frames = target + i * 4;
            put16(app, frames, reg16(history[0]));
            put16(app, frames + 2, reg16(history[1]));
            put16(app, frames + 4, reg16(history[0] >> 16));
            put16(app, frames + 6, reg16(history[1] >> 16));
            if (!less(i, -2))
                break;
        }
    }
    put(app, state + 4, history[0]);
    put(app, state + 8, history[1]);
    put(app, state + 0x10, target);
    cpu.esp += 4;
}

namespace
{

/* The memmove's forward copy, eax to, edx from, ebx bytes, eax, edx and ebx left
 * past them as the game's code leaves them. */
void copyForward(win32::WinApplication* app, x86::CPU& cpu)
{
    const reg32 bytes = cpu.ebx;
    std::memmove(&app->getMemory<reg8>(cpu.eax), &app->getMemory<reg8>(cpu.edx), bytes);
    cpu.eax += bytes;
    cpu.edx += bytes;
    cpu.ebx = 0;
}

}  // namespace

/* 0x5033e5: the memmove's forward copy, eax to, edx from, ebx bytes. */
void copy(win32::WinApplication* app, x86::CPU& cpu)
{
    copyForward(app, cpu);
    cpu.esp += 4;
}

/* 0x5033e4: the same, eax from and edx to (xchg first). */
void copySwapped(win32::WinApplication* app, x86::CPU& cpu)
{
    std::swap(cpu.eax, cpu.edx);
    copyForward(app, cpu);
    cpu.esp += 4;
}

}  // namespace mix
}  // namespace nfs3hp
