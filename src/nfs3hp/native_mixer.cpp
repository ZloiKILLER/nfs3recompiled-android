#include <nfs3hp.h>
#include "native_thrash.h"
#include <SDL3/SDL.h>
#include <cstdint>
#include <cstring>
#include <vector>

/* The game's sound mixer, native: everything sub_500864 does with a sample on
 * its way from a sound's data to what eacsnd's serve hands SDL.
 *
 * EA's SND library keeps 16 channels at 0x9fbcb4, 0xde4 bytes each, and mixes
 * them 256 frames at a time (sub_5006f8): each channel's volume ramped a frame
 * at a time towards its target (sub_5004c8), then its data decoded into
 * [0x9fbcac]+0x10, resampled to the output's rate into [0x9fbcb0] and added,
 * at its volume and pan, to the 16-bit accumulator [0x9fbca8], which is then
 * doubled with saturation into the output and cleared.  Each step goes through
 * a function the channel was given when its sound started (sub_5000b8,
 * iSNDmixstart):
 *
 *   +0xda8 decoder, register call (state, frames, buffer): looped, one-shot and
 *          streamed PCM, and EA's XA ADPCM -- mono one-shot, mono looped (an
 *          intro that hands over to its loop), stereo streamed from memory and
 *          from a file -- with their state from +0xc.
 *   +0xdb4 how many source frames a number of output frames takes, and
 *   +0xdb8 the resampler: linear interpolation in 16.16, mono or stereo, or
 *          none at the output's own rate; their state from +0xd64.
 *   +0xdbc the volume, set from the channel's left and right bytes, and
 *   +0xdc0 the add of a block at that volume into the accumulator; their state
 *          from +0xd90 (+0xddc and +0xdc4 are a second volume, set but unused).
 *
 * These are the plain (not MMX) set the game picks here ([0x9f6a5a] 0), all
 * found by NFS_SND_TRACE in menus, races and movies.  Each is done here as its
 * x86 does it, to the bit -- the MMX of the few that use it included -- on the
 * game's own memory, so the game's half of SND (starting, stopping, volumes,
 * banks, streams) sees no difference.  A function this does not know is called
 * as the game would call it.  What stays the game's: a stream's next chunk
 * ([0x5678b4], sub_5118fc, sub_511998) and a channel's end ([0x9f6b1c]), a few
 * times a second.
 *
 * NFS_SND_MIX=0 (launch extra snd_mix false) leaves it all to the generated
 * code, as does NFS_NATIVES=0.  NFS_SND_CHECK=1 (snd_check true) mixes every
 * call both ways and compares the channels, the buffers and the output. */

namespace nfs3hp
{

namespace
{

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;

constexpr u32 kChannels = 0x9fbcb4;
constexpr u32 kChannelSize = 0xde4;
constexpr u32 kChannelCount = 16;
constexpr u32 kAccumulator = 0x9fbca8;    // pointers to the three buffers
constexpr u32 kDecoded = 0x9fbcac;
constexpr u32 kResampled = 0x9fbcb0;
constexpr u32 kBuffersStart = 0x9f6b28;   // where the three buffers lie
constexpr u32 kShift = 0x9f6a5a;          // the accumulator's frame shift (0 here)
constexpr u32 kBlockBytes = 0x9f6a58;     // a 256-frame block's output, in bytes
constexpr u32 kOutput = 0x9f6b20;         // cdecl (frames, accumulator, output)
constexpr u32 kClear = 0x9f6a5c;          // cdecl (accumulator, dwords)
constexpr u32 kAfterChannel = 0x9f6a64;
constexpr u32 kBeforeOutput = 0x9f6a68;
constexpr u32 kEndHook = 0x9f6b24;
constexpr u32 kChannelEnded = 0x9f6b1c;   // the game's: register call (channel)
constexpr u32 kNextChunk = 0x5678b4;      // the game's stream refill: (state, length)
constexpr u32 kCoefficients1 = 0x56ad70;  // XA predictors, two words each
constexpr u32 kCoefficients2 = 0x56ad80;
constexpr u32 kNibbles = 0xa0bea0;        // XA: (shift * 16 + nibble) -> the nibble scaled, << 8

/* The race's reverb: each channel sent at its second volume into kSend
 * (sub_5230dc), which before the output goes through four allpass delays and
 * two low passes, in float, and back into the accumulator (sub_523118). */
constexpr u32 kSendChannel = 0x5230dc;
constexpr u32 kReverb = 0x523118;
constexpr u32 kReverbInts = 0x9f6a5b;     // nonzero: the send in 16 bits, as here
constexpr u32 kSend = 0xa0caa0;           // 16-bit stereo, a block of it
constexpr u32 kReverbBuffer = 0xa0c6a0;   // float, one a frame
constexpr u32 kReverbIdle = 0xa0d2a0;     // blocks since a channel last sent; the tail stops at 500
constexpr u32 kReverbStateStart = 0xa0c6a0;
constexpr u32 kReverbStateEnd = 0xa0d2b4;
constexpr u32 kDelayStart = 0xa181b4;     // read pointers, starts, filter states, ends
constexpr u32 kDelayEnd = 0xa18204;
constexpr u32 kFilterStart = 0x56ae24;    // the low passes' states and the coefficients
constexpr u32 kFilterEnd = 0x56ae60;
constexpr u32 kMagicRound = 0x551af8;     // 1.5 * 2^23, a double: a float rounded to an integer

/* An allpass delay of the reverb: its write and read pointers, the start and
 * end of its line, its state and its coefficient. */
struct Delay
{
    u32 write, read, start, end, state, k;
};
constexpr Delay kDelays[4] = {
    {0xa0d2a4, 0xa181b4, 0xa181c4, 0xa181f4, 0xa181e4, 0x56ae50},
    {0xa0d2a8, 0xa181b8, 0xa181c8, 0xa181f8, 0xa181e8, 0x56ae54},
    {0xa0d2ac, 0xa181bc, 0xa181cc, 0xa181fc, 0xa181ec, 0x56ae58},
    {0xa0d2b0, 0xa181c0, 0xa181d0, 0xa18200, 0xa181f0, 0x56ae5c},
};

bool mixOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_SND_MIX");
        return !value || SDL_strcmp(value, "0") != 0;
    }();
    return on && nativesEnabled();
}

bool mixChecking()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_SND_CHECK");
        return value && SDL_strcmp(value, "1") == 0;
    }();
    return on;
}

/* Thrown, in a check, where the native mix would call the game: the call is
 * left to the original's run alone, so it happens once. */
struct GuestCallInCheck
{
};

/* The game functions the native mix called in a check, each logged once. */
void noteGuestCall(u32 address)
{
    static std::vector<u32> seen;
    for (const u32 a : seen)
        if (a == address)
            return;
    seen.push_back(address);
    SDL_Log("[SNDCHK] the mix calls the game at 0x%x: left to the original", address);
}

i16 saturate(i32 value)
{
    return i16(value > 32767 ? 32767 : value < -32768 ? -32768 : value);
}

/* pmulhw: the high half of a signed product. */
i16 mulHigh(i16 a, i16 b)
{
    return i16((i32(a) * i32(b)) >> 16);
}

struct Mixer
{
    win32::WinApplication* app;
    x86::CPU& cpu;
    u8* m;
    bool checking;

    u32 r32(u32 a) const { u32 v; std::memcpy(&v, m + a, 4); return v; }
    i32 s32(u32 a) const { return i32(r32(a)); }
    u16 r16(u32 a) const { u16 v; std::memcpy(&v, m + a, 2); return v; }
    i16 s16(u32 a) const { return i16(r16(a)); }
    u8 r8(u32 a) const { return m[a]; }
    i8 s8(u32 a) const { return i8(m[a]); }
    void w32(u32 a, u32 v) { std::memcpy(m + a, &v, 4); }
    void w16(u32 a, u16 v) { std::memcpy(m + a, &v, 2); }
    void w8(u32 a, u8 v) { m[a] = v; }

    /* rep movs: forward, byte after byte where the two overlap. */
    void copy(u32 to, u32 from, u32 bytes)
    {
        if (to > from && to < from + bytes)
        {
            for (u32 i = 0; i < bytes; ++i)
                m[to + i] = m[from + i];
        }
        else
        {
            std::memmove(m + to, m + from, bytes);
        }
    }

    /* sub_4e0640: memset, the byte in dl, the length in ebx. */
    void fill(u32 to, u8 value, u32 bytes) { std::memset(m + to, value, bytes); }

    /* A call into the game, in Watcom's registers and with cdecl arguments on
     * the stack; every register as it was afterwards but eax, the result. */
    u32 guest(u32 address, u32 eax, u32 edx, u32 ebx, u32 ecx, std::initializer_list<u32> stack = {})
    {
        if (checking)
        {
            noteGuestCall(address);
            throw GuestCallInCheck();
        }
        const u32 saved[] = {cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.esi, cpu.edi, cpu.ebp, cpu.esp};
        u32 slot = cpu.esp - 4 * u32(stack.size());
        const u32 first = slot;
        for (const u32 argument : stack)
        {
            w32(slot, argument);
            slot += 4;
        }
        cpu.eax = eax;
        cpu.edx = edx;
        cpu.ebx = ebx;
        cpu.ecx = ecx;
        cpu.esp = first - 4;
        app->dynamic_call(address, cpu);
        const u32 result = cpu.eax;
        cpu.eax = saved[0];
        cpu.ebx = saved[1];
        cpu.ecx = saved[2];
        cpu.edx = saved[3];
        cpu.esi = saved[4];
        cpu.edi = saved[5];
        cpu.ebp = saved[6];
        cpu.esp = saved[7];
        return result;
    }

    // ---------------------------------------------------------------- volume

    /* sub_51f874: a stereo source's volume, left and right as words, twice. */
    void volumeStereo(u32 state, i32 left, i32 right)
    {
        const u32 v = (u32(right) << 24) | ((u32(left) << 8) & 0xffff);
        w32(state, v);
        w32(state + 4, v);
    }

    /* sub_51f774: a mono source's, left four times, then right four times. */
    void volumeMono(u32 state, i32 left, i32 right)
    {
        const u32 l = (u32(left) << 24) | ((u32(left) << 8) & 0xffff);
        const u32 r = (u32(right) << 24) | ((u32(right) << 8) & 0xffff);
        w32(state, l);
        w32(state + 4, l);
        w32(state + 8, r);
        w32(state + 12, r);
    }

    void setVolume(u32 function, u32 state, i32 left, i32 right)
    {
        switch (function)
        {
        case 0x51f874: volumeStereo(state, left, right); return;
        case 0x51f774: volumeMono(state, left, right); return;
        default: guest(function, 0, 0, 0, 0, {state, u32(left), u32(right)}); return;
        }
    }

    /* sub_51f897: a stereo block at its volume, saturating, into the
     * accumulator -- eight frames at a time, then one at a time. */
    void addStereo(u32 state, i32 count, u32 source, u32 target)
    {
        i16 v[4];
        for (int i = 0; i < 4; ++i)
            v[i] = s16(state + 2 * i);
        i32 c = count;
        if ((c & 7) != 0)
        {
            c -= 8;
            if (c < 0)
                goto tail;
        }
        do
        {
            for (int j = 0; j < 16; ++j)
            {
                const u32 t = target + 2 * j;
                w16(t, u16(saturate(i32(s16(t)) + mulHigh(v[j & 3], s16(source + 2 * j)))));
            }
            source += 32;
            target += 32;
            c -= 8;
        } while (c > 0);
        if (c == 0)
            return;
    tail:
        for (i32 k = -(c + 8); k < 0; ++k)
        {
            w16(target, u16(saturate(i32(s16(target)) + mulHigh(s16(source), v[0]))));
            w16(target + 2, u16(saturate(i32(s16(target + 2)) + mulHigh(s16(source + 2), v[1]))));
            source += 4;
            target += 4;
        }
    }

    /* sub_51f7ad: a mono block to both sides, four frames at a time. */
    void addMono(u32 state, i32 count, u32 source, u32 target)
    {
        i16 l[4], r[4];
        for (int i = 0; i < 4; ++i)
        {
            l[i] = s16(state + 2 * i);
            r[i] = s16(state + 8 + 2 * i);
        }
        i32 c = count;
        if ((c & 3) != 0)
        {
            c -= 4;
            if (c < 0)
                goto tail;
        }
        do
        {
            for (int j = 0; j < 4; ++j)
            {
                const i16 s = s16(source + 2 * j);
                const u32 t = target + 4 * j;
                w16(t, u16(saturate(i32(s16(t)) + mulHigh(l[j], s))));
                w16(t + 2, u16(saturate(i32(s16(t + 2)) + mulHigh(r[j], s))));
            }
            source += 8;
            target += 16;
            c -= 4;
        } while (c > 0);
        if (c == 0)
            return;
    tail:
        for (i32 k = -(c + 4); k < 0; ++k)
        {
            const i16 s = s16(source);
            w16(target, u16(saturate(i32(s16(target)) + mulHigh(l[0], s))));
            w16(target + 2, u16(saturate(i32(s16(target + 2)) + mulHigh(r[0], s))));
            source += 2;
            target += 4;
        }
    }

    void add(u32 function, u32 state, i32 count, u32 source, u32 target)
    {
        switch (function)
        {
        case 0x51f897: addStereo(state, count, source, target); return;
        case 0x51f7ad: addMono(state, count, source, target); return;
        default: guest(function, 0, 0, 0, 0, {state, u32(count), source, target}); return;
        }
    }

    // ------------------------------------------------------------ resampling

    /* How many source frames `frames` output frames take. */
    u32 needed(u32 function, u32 state, u32 frames)
    {
        switch (function)
        {
        case 0x520438:
        case 0x5200b8:
            return frames;
        case 0x520440:  // stereo
        case 0x5200c0:  // mono
        {
            const u32 flags = function == 0x520440 ? 0x14 : 0x10;
            u32 n = ((frames - 1) * r32(state + 8) + r32(state + 4)) >> 16;
            ++n;
            if (r8(state + flags + 1) == 0)
                ++n;
            return n - u32(i32(s8(state + flags)));
        }
        case 0x520070:  // mono, with what it leaves over
        case 0x5203f0:  // stereo, the same
        {
            const u32 flags = function == 0x5203f0 ? 0x15 : 0x11;
            const u32 step = r32(state + 8);
            const u32 base = (((frames - 1) * step + r32(state + 4)) >> 16) + 1;
            u32 n = base + u32(i32(s8(state + flags + 1)));
            if (r8(state + flags) == 0)
                ++n;
            const u32 end = (frames * step + r32(state + 4)) >> 16;
            w8(state + flags + 1, u8(end - base));
            return n;
        }
        default:
            return guest(function, 0, 0, 0, 0, {state, frames});
        }
    }

    /* The interpolation weights as the MMX keeps them: a word of 65535 - f and
     * one of f, from a 16.16 position whose fraction is in the high half. */
    static void weights(u32 fraction, u16& w0, u16& w1)
    {
        const u32 d = (fraction | (fraction >> 16)) ^ 0xffff;
        w0 = u16(d);
        w1 = u16(d >> 16);
    }

    /* sub_527290: stereo frames interpolated, all but the last of a block. */
    void interpolateStereo(i32 count, u32 source, u32 target, u32& index, u32& fraction, u32 stepWhole,
                           u32 stepFraction)
    {
        u16 w0, w1;
        weights(fraction, w0, w1);
        const u16 i1 = u16(stepFraction >> 16);
        const u16 i0 = u16((((stepFraction - 1) >> 16) | stepFraction) ^ 0xffff);
        u32 frac = fraction;
        u32 at = index;
        do
        {
            const u32 s = source + at * 4;
            const i32 a = i32(w0 >> 1), b = i32(w1 >> 1);
            const i32 l = i32(u32(i32(s16(s)) * a) + u32(i32(s16(s + 4)) * b)) >> 15;
            const i32 r = i32(u32(i32(s16(s + 2)) * a) + u32(i32(s16(s + 6)) * b)) >> 15;
            w0 = u16(w0 + i0);
            w1 = u16(w1 + i1);
            w16(target, u16(saturate(l)));
            w16(target + 2, u16(saturate(r)));
            const uint64_t sum = uint64_t(frac) + stepFraction;
            frac = u32(sum);
            at += stepWhole + u32(sum >> 32);
            target += 4;
        } while (--count > 0);
        index = at;
        fraction = frac;
    }

    /* sub_527210: the same for mono, truncated rather than saturated. */
    void interpolateMono(i32 count, u32 source, u32 target, u32& index, u32& fraction, u32 stepWhole,
                         u32 stepFraction)
    {
        u16 w0, w1;
        weights(fraction, w0, w1);
        const u16 i1 = u16(stepFraction >> 16);
        const u16 i0 = u16((((stepFraction - 1) >> 16) | stepFraction) ^ 0xffff);
        u32 frac = fraction;
        u32 at = index;
        do
        {
            const u32 s = source + at * 2;
            const u32 d = u32(i32(s16(s)) * i32(w0 >> 1)) + u32(i32(s16(s + 2)) * i32(w1 >> 1));
            w0 = u16(w0 + i0);
            w1 = u16(w1 + i1);
            w16(target, u16(d >> 15));
            const uint64_t sum = uint64_t(frac) + stepFraction;
            frac = u32(sum);
            at += stepWhole + u32(sum >> 32);
            target += 2;
        } while (--count > 0);
        index = at;
        fraction = frac;
    }

    /* One word of a block's last frame: (65535 - f) * a + f * b, each taken
     * to its high half by itself, as the x86 does it. */
    static u16 lastWord(i16 a, i16 b, u32 f)
    {
        const u16 first = u16(u32((0xffff - f) * u32(i32(a))) >> 16);
        const u32 second = u32(u32(i32(b)) * f) >> 16;
        return u16(i32(i16(first)) + i32(second));
    }

    /* sub_5205bc: stereo, the frames kept from the last block put in front. */
    u32 resampleStereo(u32 b, u32 frames, u32 source, u32 target)
    {
        u32 s = source;
        if (r8(b + 0x15))
        {
            s -= 2;
            w16(s, r16(b + 0x12));
            s -= 2;
            w16(s, r16(b + 0x10));
        }
        if (r8(b + 0x14))
        {
            s -= 2;
            w16(s, r16(b + 0xe));
            s -= 2;
            w16(s, r16(b + 0xc));
        }
        w32(b, 0);
        const u32 last = frames - 1;
        if (last != 0)
        {
            const u32 step = r32(b + 8);
            u32 index = r32(b);
            u32 fraction = r32(b + 4) << 16;
            interpolateStereo(i32(last), s, target, index, fraction, step >> 16, step << 16);
            w32(b, index);
            w32(b + 4, fraction >> 16);
        }
        stereoLast(b, s, target, last);
        const u32 step = r32(b + 8);
        if (r32(b + 4) < step)
        {
            w8(b + 0x14, 0);
        }
        else
        {
            w8(b + 0x14, 1);
            const u32 kept = s + r32(b) * 4;
            w16(b + 0xc, r16(kept));
            w16(b + 0xe, r16(kept + 2));
        }
        w8(b + 0x15, 1);
        return target;
    }

    /* A stereo block's last frame, the next frame kept and the position moved
     * on, as sub_5205bc and sub_52046c both end. */
    void stereoLast(u32 b, u32 s, u32 target, u32 last)
    {
        const u32 index = r32(b);
        const u32 f = r32(b + 4);
        const u32 at = s + index * 4;
        w16(target + last * 4, lastWord(s16(at), s16(at + 4), f));
        w16(target + last * 4 + 2, lastWord(s16(at + 2), s16(at + 6), f));
        w16(b + 0x10, r16(at + 4));
        w16(b + 0x12, r16(at + 6));
        const u32 next = f + r32(b + 8);
        w32(b + 4, next & 0xffff);
        w32(b, index + (next >> 16));
    }

    /* sub_52046c: stereo, going on from where the last block left off. */
    u32 resampleStereoOn(u32 b, u32 frames, u32 source, u32 target)
    {
        u32 s = source;
        if (r8(b + 0x15))
        {
            s = source - 2;
            w16(s, r16(b + 0x12));
            s -= 2;
            w16(s, r16(b + 0x10));
        }
        const u32 last = frames - 1;
        if (last != 0)
        {
            const u32 step = r32(b + 8);
            u32 index = r32(b);
            u32 fraction = r32(b + 4) << 16;
            interpolateStereo(i32(last), s, target, index, fraction, step >> 16, step << 16);
            w32(b, index);
            w32(b + 4, fraction >> 16);
        }
        stereoLast(b, s, target, last);
        w32(b, u32(i32(s8(b + 0x16))));
        w8(b + 0x15, 1);
        return target;
    }

    /* sub_5201e4: mono. */
    u32 resampleMono(u32 b, u32 frames, u32 source, u32 target)
    {
        u32 s = source;
        if (r8(b + 0x11))
        {
            s = source - 2;
            w16(s, r16(b + 0xe));
        }
        if (r8(b + 0x10))
        {
            s -= 2;
            w16(s, r16(b + 0xc));
        }
        w32(b, 0);
        const u32 last = frames - 1;
        if (last != 0)
        {
            const u32 step = r32(b + 8);
            u32 index = r32(b);
            u32 fraction = r32(b + 4) << 16;
            interpolateMono(i32(last), s, target, index, fraction, step >> 16, step << 16);
            w32(b, index);
            w32(b + 4, fraction >> 16);
        }
        monoLast(b, s, target, last);
        const u32 step = r32(b + 8);
        if (r32(b + 4) < step)
        {
            w8(b + 0x10, 0);
        }
        else
        {
            w8(b + 0x10, 1);
            w16(b + 0xc, r16(s + r32(b) * 2));
        }
        w8(b + 0x11, 1);
        return target;
    }

    /* A mono block's last frame, the next sample kept and the position moved
     * on, as sub_5201e4 and sub_5200ec both end. */
    void monoLast(u32 b, u32 s, u32 target, u32 last)
    {
        const u32 index = r32(b);
        const u32 f = r32(b + 4);
        const u32 at = s + index * 2;
        w16(target + last * 2, lastWord(s16(at), s16(at + 2), f));
        w16(b + 0xe, r16(at + 2));
        const u32 next = f + r32(b + 8);
        w32(b + 4, next & 0xffff);
        w32(b, index + (next >> 16));
    }

    /* sub_5200ec: mono, going on from where the last block left off. */
    u32 resampleMonoOn(u32 b, u32 frames, u32 source, u32 target)
    {
        u32 s = source;
        if (r8(b + 0x11))
        {
            s = source - 2;
            w16(s, r16(b + 0xe));
        }
        const u32 last = frames - 1;
        if (last != 0)
        {
            const u32 step = r32(b + 8);
            u32 index = r32(b);
            u32 fraction = r32(b + 4) << 16;
            interpolateMono(i32(last), s, target, index, fraction, step >> 16, step << 16);
            w32(b, index);
            w32(b + 4, fraction >> 16);
        }
        monoLast(b, s, target, last);
        w32(b, u32(i32(s8(b + 0x12))));
        w8(b + 0x11, 1);
        return target;
    }

    /* The resampler: where the block it leaves is. */
    u32 resample(u32 function, u32 state, u32 frames, u32 source, u32 target)
    {
        switch (function)
        {
        case 0x5205b0:
        case 0x5201d8:
            return source;
        case 0x5205bc: return resampleStereo(state, frames, source, target);
        case 0x5201e4: return resampleMono(state, frames, source, target);
        case 0x5200ec: return resampleMonoOn(state, frames, source, target);
        case 0x52046c: return resampleStereoOn(state, frames, source, target);
        default:
        {
            const u32 slot = cpu.esp - 0x40;
            w32(slot, target);
            guest(function, 0, 0, 0, 0, {state, frames, source, slot});
            return r32(slot);
        }
        }
    }

    // -------------------------------------------------------------- decoding

    /* sub_52c9d0: PCM, looped between [+8] and [+0xc]. */
    i32 decodeLooped(u32 st, i32 frames, u32 target)
    {
        i32 n = frames;
        while (n > 0)
        {
            const u32 available = r32(st + 0xc) - r32(st + 4) + 1;
            const u32 k = n >= i32(available) ? available : u32(n);
            const u8 shift = r8(st + 0x10) & 31;
            copy(target, r32(st) + (r32(st + 4) << shift), k << shift);
            n -= i32(k);
            const u32 position = r32(st + 4) + k;
            target += k << shift;
            w32(st + 4, position);
            if (position > r32(st + 0xc))
                w32(st + 4, r32(st + 8));
        }
        return 1;
    }

    /* sub_52c8f4: PCM played once, silence after its end. */
    i32 decodeOnce(u32 st, i32 frames, u32 target)
    {
        const u32 position = r32(st + 4), end = r32(st + 8);
        if (position >= end)
            return -1;
        const u32 next = position + u32(frames);
        w32(st + 4, next);
        const u8 shift = r8(st + 0xc) & 31;
        if (next < end)
        {
            copy(target, r32(st) + (position << shift), u32(frames) << shift);
            return 1;
        }
        const u32 k = end - position;
        copy(target, r32(st) + (position << shift), k << shift);
        fill(target + (k << shift), 0, (u32(frames) - k) << shift);
        return 1;
    }

    /* sub_52ca80: PCM streamed a chunk at a time from the game. */
    i32 decodeStreamed(u32 st, i32 frames, u32 target)
    {
        if (r32(st) == 0)
            return -1;
        i32 n = frames;
        while (n > 0)
        {
            if (s32(st + 8) >= s32(st + 4))
            {
                const u32 result = guest(r32(kNextChunk), st, st + 4, 0, 0);
                if (result == 0 && r32(st) != 0)
                {
                    w32(st + 8, 0);
                }
                else
                {
                    fill(target, 0, u32(n) << (r8(st + 0xc) & 31));
                    w32(st + 8, 0);
                    w32(st + 4, 0);
                    return 1;
                }
            }
            const i32 available = s32(st + 4) - s32(st + 8);
            const i32 k = available > n ? n : available;
            const u8 shift = r8(st + 0xc) & 31;
            copy(target, r32(st) + (r32(st + 8) << shift), u32(k) << shift);
            w32(st + 8, r32(st + 8) + u32(k));
            target += u32(k) << shift;
            n -= k;
        }
        return 1;
    }

    i32 nibble(u32 shift, u32 value) const { return s32(kNibbles + shift + value * 4); }

    /* sub_52d1a0: mono XA, [+0] samples (whole blocks of 28, 15 bytes each)
     * from [+8] to [+0xc], the last two samples kept at +4. */
    void xaMono(u32 st)
    {
        u32 source = r32(st + 8);
        u32 target = r32(st + 0xc);
        i16 h0 = s16(st + 4), h1 = s16(st + 6);
        while (s32(st) > 0)
        {
            const u8 header = r8(source);
            w32(st, r32(st) - 28);
            const u32 p = header >> 4;
            const i32 c0 = s16(kCoefficients1 + p * 4), c1 = s16(kCoefficients1 + p * 4 + 2);
            const i32 e0 = s16(kCoefficients2 + p * 4), e1 = s16(kCoefficients2 + p * 4 + 2);
            const u32 shift = u32(header & 0xf) << 6;
            source += 15;
            target += 56;
            for (i32 c = -14; c < 0; ++c)
            {
                const u8 x = r8(source + u32(c));
                const u32 d = u32(i32(h0) * c0) + u32(i32(h1) * c1) + u32(nibble(shift, x >> 4));
                const i16 s1 = i16(d >> 8);
                const u32 d2 = u32(i32(s1) * e0) + u32(i32(h1) * e1) + u32(nibble(shift, x & 0xf));
                const i16 s2 = i16(d2 >> 8);
                w16(target + u32(c * 4), u16(s1));
                w16(target + u32(c * 4) + 2, u16(s2));
                h0 = s1;
                h1 = s2;
            }
        }
        w32(st + 4, u32(u16(h0)) | (u32(u16(h1)) << 16));
        w32(st + 0xc, target);
    }

    /* sub_52d250: stereo XA, 30-byte blocks of 28 frames, from [+0xc] to
     * [+0x10], the last two of each side kept at +4 and +8. */
    void xaStereo(u32 st)
    {
        u32 source = r32(st + 0xc);
        u32 target = r32(st + 0x10);
        i16 a0 = s16(st + 4), a1 = s16(st + 6), b0 = s16(st + 8), b1 = s16(st + 10);
        while (s32(st) > 0)
        {
            const u8 header = r8(source);
            w32(st, r32(st) - 28);
            const u32 pl = header >> 4, pr = header & 0xf;
            const i32 c0l = s16(kCoefficients1 + pl * 4), c1l = s16(kCoefficients1 + pl * 4 + 2);
            const i32 c0r = s16(kCoefficients1 + pr * 4), c1r = s16(kCoefficients1 + pr * 4 + 2);
            const i32 e0l = s16(kCoefficients2 + pl * 4), e1l = s16(kCoefficients2 + pl * 4 + 2);
            const i32 e0r = s16(kCoefficients2 + pr * 4), e1r = s16(kCoefficients2 + pr * 4 + 2);
            const u8 shifts = r8(source + 1);
            const u32 sl = u32(shifts >> 4) << 6, sr = u32(shifts & 0xf) << 6;
            source += 30;
            target += 0x70;
            for (i32 c = -28; c < 0; c += 2)
            {
                const u8 x = r8(source + u32(c)), y = r8(source + u32(c) + 1);
                const u32 dl = u32(i32(a0) * c0l) + u32(i32(a1) * c1l) + u32(nibble(sl, x >> 4));
                const u32 dr = u32(i32(b0) * c0r) + u32(i32(b1) * c1r) + u32(nibble(sr, x & 0xf));
                const i16 l1 = i16(dl >> 8), r1 = i16(dr >> 8);
                const u32 dl2 = u32(i32(l1) * e0l) + u32(i32(a1) * e1l) + u32(nibble(sl, y >> 4));
                const u32 dr2 = u32(i32(r1) * e0r) + u32(i32(b1) * e1r) + u32(nibble(sr, y & 0xf));
                const i16 l2 = i16(dl2 >> 8), r2 = i16(dr2 >> 8);
                const u32 t = target + u32(c * 4);
                w16(t, u16(l1));
                w16(t + 2, u16(r1));
                w16(t + 4, u16(l2));
                w16(t + 6, u16(r2));
                a0 = l1;
                a1 = l2;
                b0 = r1;
                b1 = r2;
            }
        }
        w32(st + 4, u32(u16(a0)) | (u32(u16(a1)) << 16));
        w32(st + 8, u32(u16(b0)) | (u32(u16(b1)) << 16));
        w32(st + 0x10, target);
    }

    /* sub_529140 / sub_529c80: decoded frames kept over from the last call
     * copied out, what is left moved down; how many were copied. */
    i32 leftOver(i32 count, u32 buffer, i32 wanted, u32 targetSlot, u32 frameBytes)
    {
        const i32 k = count >= wanted ? wanted : count;
        u32 target = r32(targetSlot);
        i32 i = 0;
        for (; i < k; ++i)
        {
            copy(target, buffer + u32(i) * frameBytes, frameBytes);
            target += frameBytes;
        }
        for (u32 to = buffer; i < count; ++i, to += frameBytes)
            copy(to, buffer + u32(i) * frameBytes, frameBytes);
        w32(targetSlot, target);
        return k;
    }

    static i32 blockOffset(u32 position, u32 bytes) { return i32(position * bytes) / 28; }

    /* sub_528f74: mono XA played once. */
    i32 xaOnce(u32 st, i32 frames, u32 target)
    {
        const i32 position = s32(st + 0x18);
        if (position >= s32(st + 0x14))
            return -1;
        w32(st + 0xc, target);
        i32 n = frames;
        if (n <= i32(s16(st + 0x1c)))
        {
            i32 i = 0;
            for (; i < n; ++i)
            {
                w16(r32(st + 0xc), r16(st + 0x1e + 2 * u32(i)));
                w32(st + 0xc, r32(st + 0xc) + 2);
            }
            i32 kept = 0;
            for (; i < i32(s16(st + 0x1c)); ++i, ++kept)
                w16(st + 0x1e + 2 * u32(kept), r16(st + 0x1e + 2 * u32(i)));
            w16(st + 0x1c, u16(kept));
            return 1;
        }
        i32 i = 0;
        for (; i < i32(s16(st + 0x1c)); ++i)
        {
            w16(r32(st + 0xc), r16(st + 0x1e + 2 * u32(i)));
            w32(st + 0xc, r32(st + 0xc) + 2);
        }
        n -= i;
        i32 remaining = s32(st + 0x14) - s32(st + 0x18);
        if (remaining < 0)
            remaining = 0;
        const i32 k = n >= remaining ? remaining : n;
        const i32 offset = blockOffset(r32(st + 0x18), 15);
        w32(st, u32(k));
        w32(st + 8, r32(st + 0x10) + u32(offset));
        xaMono(st);
        w32(st + 0x18, r32(st + 0x18) + u32(k));
        w32(st + 0x18, r32(st + 0x18) - r32(st));
        if (remaining == k)
        {
            i32 zeros = n - k - (s32(st + 0x18) - s32(st + 0x14));
            while (zeros > 0)
            {
                w16(r32(st + 0xc), 0);
                w32(st + 0xc, r32(st + 0xc) + 2);
                --zeros;
            }
            return 1;
        }
        const i32 over = s32(st);
        if (over >= 0)
        {
            w16(st + 0x1c, 0);
            return 1;
        }
        i32 at = 2 * over, kept = 0;
        do
        {
            at += 2;
            ++kept;
            w16(st + 0x1c + 2 * u32(kept), r16(r32(st + 0xc) + u32(at) - 2));
        } while (at < 0);
        w16(st + 0x1c, u16(kept));
        return 1;
    }

    /* sub_5291a8: mono XA looping from [+0x1c] to [+0x20]. */
    i32 xaLoop(u32 st, i32 frames, u32 target)
    {
        w32(st + 0xc, target);
        i32 n = frames;
        if (n > 0)
        {
            do
            {
                if (r16(st + 0x24) != 0)
                {
                    const i32 k = leftOver(s16(st + 0x24), st + 0x26, n, st + 0xc, 2);
                    w16(st + 0x24, u16(r16(st + 0x24) - u16(k)));
                    n -= k;
                }
                else if (s32(st + 0x18) + 27 < s32(st + 0x20))
                {
                    const i32 offset = blockOffset(r32(st + 0x18), 15);
                    w32(st, 28);
                    w32(st + 8, r32(st + 0x10) + u32(offset));
                    xaMono(st);
                    n -= 28;
                    w32(st + 0x18, r32(st + 0x18) + 28);
                }
                else
                {
                    const u32 saved = r32(st + 0xc);
                    const i32 offset = blockOffset(r32(st + 0x18), 15);
                    w32(st, 28);
                    w32(st + 0xc, st + 0x26);
                    w32(st + 8, r32(st + 0x10) + u32(offset));
                    xaMono(st);
                    w32(st + 0x18, r32(st + 0x18) + 28);
                    const u16 past = u16(r16(st + 0x18) - r16(st + 0x20));
                    const u16 left = u16(28 - past);
                    w16(st + 0x24, left);
                    w16(st + 6, r16(st + 0x94));
                    w16(st + 4, r16(st + 0x96));
                    w16(st + 0x24, u16(left + 1));
                    const i32 loop = s32(st + 0x1c);
                    w32(st + 0x18, u32((loop / 28) * 28));
                    if (loop % 28 != 0)
                    {
                        w32(st, 28);
                        const i32 offset2 = blockOffset(r32(st + 0x18), 15);
                        w32(st + 8, r32(st + 0x10) + u32(offset2));
                        const u32 scratch = (cpu.esp - 0x100) & ~3u;
                        w32(st + 0xc, scratch);
                        xaMono(st);
                        const u32 position = r32(st + 0x18) + 28;
                        const i32 tail = i32(position) - s32(st + 0x1c);
                        u32 from = scratch + u32(28 - tail) * 2;
                        w32(st + 0x18, position);
                        for (i32 j = 0; j < tail; ++j, from += 2)
                        {
                            w16(st + 0x26 + u32(s16(st + 0x24)) * 2, r16(from));
                            w16(st + 0x24, u16(r16(st + 0x24) + 1));
                        }
                    }
                    w32(st + 0xc, saved);
                }
            } while (n > 0);
        }
        w32(st + 0xc, r32(st + 0xc) + u32(n) * 2);
        for (; n < 0; ++n)
        {
            w16(st + 0x26 + u32(s16(st + 0x24)) * 2, r16(r32(st + 0xc)));
            w32(st + 0xc, r32(st + 0xc) + 2);
            w16(st + 0x24, u16(r16(st + 0x24) + 1));
        }
        return 1;
    }

    /* sub_529384: a looped mono XA's intro, which hands the channel over to
     * its loop (sub_5291a8) once it is played. */
    i32 xaIntro(u32 st, i32 frames, u32 target)
    {
        w32(st + 0xc, target);
        const i32 kept = s16(st + 0x24);
        if (kept >= frames)
        {
            i32 i = 0;
            for (; i < frames; ++i)
            {
                w16(r32(st + 0xc), r16(st + 0x26 + 2 * u32(i)));
                w32(st + 0xc, r32(st + 0xc) + 2);
            }
            i32 moved = 0;
            for (; i < i32(s16(st + 0x24)); ++i, ++moved)
                w16(st + 0x26 + 2 * u32(moved), r16(st + 0x26 + 2 * u32(i)));
            w16(st + 0x24, u16(moved));
            return 1;
        }
        i32 i = 0;
        for (; i < i32(s16(st + 0x24)); ++i)
        {
            w16(r32(st + 0xc), r16(st + 0x26 + 2 * u32(i)));
            w32(st + 0xc, r32(st + 0xc) + 2);
        }
        i32 rest = frames - i;
        i32 k = s32(st + 0x1c) - s32(st + 0x18) - 27;
        if (k > rest)
            k = rest;
        if (k < 0)
            k = 0;
        const i32 offset = blockOffset(r32(st + 0x18), 15);
        w32(st, u32(k));
        w32(st + 8, r32(st + 0x10) + u32(offset));
        xaMono(st);
        const u32 position = r32(st + 0x18) + u32(k);
        w32(st + 0x18, position);
        w32(st + 0x18, position - r32(st));
        i32 at = 2 * s32(st);
        u32 moved = 0;
        u32 bytes = 0;
        while (at < 0)
        {
            const u16 v = r16(r32(st + 0xc) + u32(at));
            at += 2;
            w16(st + bytes + 0x26, v);
            ++moved;
            bytes += 2;
        }
        rest -= k;
        w32(st + 0xc, r32(st + 0xc) - bytes);
        w16(st + 0x24, u16(moved));
        if (s32(st + 0x1c) - s32(st + 0x18) < 28)
        {
            w16(st + 0x94, r16(st + 6));
            w16(st + 0x96, r16(st + 4));
            w32(r32(st + 0x98), 0x5291a8);
            if (rest > 0)
                xaLoop(st, rest, r32(st + 0xc));
        }
        return 1;
    }

    /* The four words that open a stereo XA chunk: the history it starts from. */
    void xaChunkStart(u32 st, u32 chunk)
    {
        w16(st + 6, r16(chunk));
        w16(st + 4, r16(chunk + 2));
        w16(st + 0xa, r16(chunk + 4));
        w16(st + 8, r16(chunk + 6));
        w32(st + 0x1c, 0);
        w32(st + 0x14, chunk + 8);
    }

    /* sub_52a1a0: stereo XA streamed in chunks the game hands over. */
    i32 xaStereoStream(u32 st, i32 frames, u32 target)
    {
        if (r32(st + 0x14) == 0)
            return -1;
        w32(st + 0x10, target);
        i32 n = frames;
        if (n > 0)
        {
            do
            {
                if (r32(st + 0x20) != 0)
                {
                    const i32 k = leftOver(s32(st + 0x20), st + 0x24, n, st + 0x10, 4);
                    w32(st + 0x20, r32(st + 0x20) - u32(k));
                    n -= k;
                    continue;
                }
                if (s32(st + 0x1c) + 27 < s32(st + 0x18))
                {
                    const i32 offset = blockOffset(r32(st + 0x1c), 30);
                    w32(st, 28);
                    w32(st + 0xc, r32(st + 0x14) + u32(offset));
                    xaStereo(st);
                    n -= 28;
                    w32(st + 0x1c, r32(st + 0x1c) + 28);
                    continue;
                }
                if (r32(st + 0x14) != 1)
                {
                    const i32 offset = blockOffset(r32(st + 0x1c), 30);
                    w32(st, 28);
                    w32(st + 0xc, r32(st + 0x14) + u32(offset));
                    xaStereo(st);
                    const u32 position = r32(st + 0x1c) + 28;
                    w32(st + 0x1c, position);
                    const i32 over = i32(position - r32(st + 0x18));
                    w32(st + 0x10, r32(st + 0x10) - u32(over) * 4);
                    n -= 28 - over;
                }
                guest(r32(kNextChunk), st + 0x14, st + 0x18, 0, 0);
                const u32 chunk = r32(st + 0x14);
                if (chunk != 0)
                {
                    xaChunkStart(st, chunk);
                }
                else
                {
                    n *= 2;
                    for (; n > 0; --n)
                    {
                        w16(r32(st + 0x10), 0);
                        w32(st + 0x10, r32(st + 0x10) + 2);
                    }
                }
            } while (n > 0);
        }
        w32(st + 0x10, r32(st + 0x10) + u32(n) * 4);
        for (; n < 0; ++n)
        {
            const u32 kept = r32(st + 0x20);
            const u32 from = r32(st + 0x10);
            w16(st + 0x24 + kept * 4, r16(from));
            w16(st + 0x26 + kept * 4, r16(from + 2));
            w32(st + 0x10, from + 4);
            w32(st + 0x20, kept + 1);
        }
        return 1;
    }

    /* sub_52a3a0: stereo XA streamed from a file; how many frames it gave. */
    i32 xaStereoFile(u32 st, i32 frames, u32 target)
    {
        w32(st + 0x10, target);
        i32 n = frames;
        if (r32(st + 0x28) != 0)
        {
            /* The frames the last call gave, told to the stream (sub_511998:
             * `sub [stream+0x14], edx`), which is how the game knows how far
             * the sound has played and when it is done -- edx is the count the
             * original leaves there from its test of it.  Nothing here reads
             * what that changes, so a check leaves it to the original's run. */
            if (!checking)
                guest(0x511998, r32(st + 0x24), r32(st + 0x28), 0, 0);
            w32(st + 0x28, 0);
        }
        while (n > 0)
        {
            if (r32(st + 0x20) != 0)
            {
                const i32 k = leftOver(s32(st + 0x20), st + 0x2c, n, st + 0x10, 4);
                w32(st + 0x20, r32(st + 0x20) - u32(k));
                n -= k;
                w32(st + 0x28, r32(st + 0x28) + u32(k));
                continue;
            }
            if (s32(st + 0x1c) + 27 < s32(st + 0x18))
            {
                const i32 offset = blockOffset(r32(st + 0x1c), 30);
                w32(st, 28);
                w32(st + 0xc, r32(st + 0x14) + u32(offset));
                xaStereo(st);
                w32(st + 0x1c, r32(st + 0x1c) + 28);
                w32(st + 0x28, r32(st + 0x28) + 28);
                n -= 28;
                continue;
            }
            if (r32(st + 0x14) != 0)
            {
                const i32 offset = blockOffset(r32(st + 0x1c), 30);
                w32(st, 28);
                w32(st + 0xc, r32(st + 0x14) + u32(offset));
                xaStereo(st);
                const u32 position = r32(st + 0x1c) + 28;
                w32(st + 0x1c, position);
                const i32 over = i32(position - r32(st + 0x18));
                const i32 valid = 28 - over;
                n -= valid;
                w32(st + 0x10, r32(st + 0x10) - u32(over) * 4);
                w32(st + 0x28, r32(st + 0x28) + u32(valid));
            }
            const u32 chunk = guest(0x5118fc, r32(st + 0x24), st + 0x18, 0, 0);
            w32(st + 0x14, chunk);
            if (chunk != 0)
            {
                xaChunkStart(st, chunk);
                continue;
            }
            if (n > 0)
            {
                if (r32(st + 0x28) != 0)
                    fill(r32(st + 0x10), 0, u32(n) * 4);
                w32(st + 0x10, r32(st + 0x10) + u32(n) * 4);
                n = 0;
            }
        }
        w32(st + 0x10, r32(st + 0x10) + u32(n) * 4);
        if (n >= 0)
            return s32(st + 0x28);
        const u32 over = u32(-n);
        copy(st + 0x2c + r32(st + 0x20) * 4, r32(st + 0x10), over * 4);
        w32(st + 0x20, r32(st + 0x20) + over);
        w32(st + 0x28, r32(st + 0x28) - over);
        return s32(st + 0x28);
    }

    i32 decode(u32 function, u32 st, u32 frames, u32 target)
    {
        const i32 n = i32(frames);
        switch (function)
        {
        case 0x52c9d0: return decodeLooped(st, n, target);
        case 0x52c8f4: return decodeOnce(st, n, target);
        case 0x52ca80: return decodeStreamed(st, n, target);
        case 0x528f74: return xaOnce(st, n, target);
        case 0x5291a8: return xaLoop(st, n, target);
        case 0x529384: return xaIntro(st, n, target);
        case 0x52a1a0: return xaStereoStream(st, n, target);
        case 0x52a3a0: return xaStereoFile(st, n, target);
        default: return i32(guest(function, st, frames, target, 0));
        }
    }


    // ---------------------------------------------------------------- reverb

    typedef x86::Float F;

    F loadFloat(u32 a) const { float v; std::memcpy(&v, m + a, 4); return F(v); }
    void storeFloat(u32 a, F v) { const float x = float(v); std::memcpy(m + a, &x, 4); }

    /* The emms the MMX set needs before the x87, or the plain set's ret. */
    void beforeFloat()
    {
        const u32 end = r32(kEndHook);
        if (end != 0x520a00 && end != 0x4ffd80)
            guest(end, 0, 0, 0, 0);
    }

    /* sub_5230dc: a channel's block sent into the reverb at its second volume. */
    void send(u32 ch, u32 frames, u32 source, u32 ramped)
    {
        if (r8(ch + 8) == 0)
            return;
        const u32 target = kSend + ((ramped << (r8(kShift) & 31)) << 2);
        add(r32(ch + 0xde0), ch + 0xdc4, i32(frames), source, target);
        w32(kReverbIdle, 0);
    }

    /* One allpass delay of sub_52b300: the line's pointers moved on, wrapping
     * at its end; the input plus the state written to the line; the state the
     * delayed sample times k; out the delayed sample less the written one
     * times k -- in that order, the line read again after it is written. */
    F allpass(const Delay& d, F input)
    {
        x86::FPU& fpu = cpu.fpu;
        const u32 end = r32(d.end);
        u32 w = r32(d.write) + 4;
        u32 r = r32(d.read) + 4;
        if (i32(w) >= i32(end))
            w = r32(d.start);
        if (i32(r) >= i32(end))
            r = r32(d.start);
        const F a = fpu.add(input, loadFloat(d.state));
        const F delayed = loadFloat(r);
        w32(d.write, w);
        w32(d.read, r);
        const F dk = fpu.mul(delayed, loadFloat(d.k));
        storeFloat(w, a);
        const F ak = fpu.mul(a, loadFloat(d.k));
        storeFloat(d.state, dk);
        return fpu.sub(loadFloat(r), ak);
    }

    /* A low pass of sub_52b300: state * c + input * c', the state kept. */
    F lowPass(u32 state, u32 c0, u32 c1, F input)
    {
        x86::FPU& fpu = cpu.fpu;
        const F a = fpu.mul(loadFloat(state), loadFloat(c0));
        const F b = fpu.mul(input, loadFloat(c1));
        const F out = fpu.add(a, b);
        storeFloat(state, out);
        return out;
    }

    /* sub_52b300: the float buffer through the reverb, in place. */
    void reverbFilter(i32 count, u32 buffer)
    {
        do
        {
            F v = allpass(kDelays[0], loadFloat(buffer));
            v = allpass(kDelays[1], v);
            v = lowPass(0x56ae24, 0x56ae34, 0x56ae44, v);
            v = allpass(kDelays[2], v);
            v = allpass(kDelays[3], v);
            v = lowPass(0x56ae2c, 0x56ae3c, 0x56ae4c, v);
            storeFloat(buffer, v);
            buffer += 4;
        } while (--count > 0);
    }

    /* sub_522fd4: the reverb added to both sides of the accumulator, each
     * rounded to an integer through 1.5 * 2^23 and clamped by its bits. */
    static u16 clampRounded(float rounded)
    {
        u32 bits;
        std::memcpy(&bits, &rounded, 4);
        bits &= 0xfffff;
        if (bits <= 0x7fff || bits >= 0xf8000)
            return u16(bits);
        return bits >= 0x80000 ? 0x8000 : 0x7fff;
    }

    void reverbBack(u32 frames, u32 source, u32 accumulator)
    {
        x86::FPU& fpu = cpu.fpu;
        beforeFloat();
        double magic;
        std::memcpy(&magic, m + kMagicRound, 8);
        const u32 end = source + frames * 4;
        for (; source < end; source += 4, accumulator += 4)
        {
            const float left = float(fpu.add(F(i32(s16(accumulator))), loadFloat(source)));
            const float right = float(fpu.add(F(i32(s16(accumulator + 2))), loadFloat(source)));
            const float l = float(fpu.add(F(left), F(magic)));
            const float r = float(fpu.add(F(magic), F(right)));
            w16(accumulator, clampRounded(l));
            w16(accumulator + 2, clampRounded(r));
        }
    }

    void clearAccumulator(u32 at, u32 dwords)
    {
        if (r32(kClear) == 0x5209a0)
        {
            if (i32(dwords) > 0)
                fill(at, 0, dwords * 4);
        }
        else
        {
            guest(r32(kClear), 0, 0, 0, 0, {at, dwords});
        }
    }

    /* sub_523118: the send to float (sub_522f90, left plus right), the send
     * cleared, the reverb, and back into the accumulator. */
    void reverb(u32 frames, u32 accumulator)
    {
        const u32 idle = r32(kReverbIdle) + 1;
        w32(kReverbIdle, idle);
        if (i32(idle) >= 500)
            return;
        beforeFloat();
        const u32 floats = kReverbBuffer + frames * 4;
        for (u32 from = kSend, to = kReverbBuffer; to < floats; from += 4, to += 4)
            storeFloat(to, F(i32(s16(from)) + i32(s16(from + 2))));
        clearAccumulator(kSend, frames * 2);
        reverbFilter(i32(frames), kReverbBuffer);
        reverbBack(frames, kReverbBuffer, accumulator);
    }

    void afterChannel(u32 ch, u32 frames, u32 source, u32 ramped)
    {
        const u32 hook = r32(kAfterChannel);
        if (hook == kSendChannel)
            send(ch, frames, source, ramped);
        else if (hook != 0)
            guest(hook, ch, frames, source, ramped);
    }

    void beforeOutput(u32 frames, u32 accumulator)
    {
        const u32 hook = r32(kBeforeOutput);
        if (hook == kReverb && r8(kReverbInts) != 0)
            reverb(frames, accumulator);
        else if (hook != 0)
            guest(hook, frames, accumulator, 0, 0);
    }

    // ---------------------------------------------------------------- mixing

    u32 channel(u32 i) const { return kChannels + i * kChannelSize; }

    /* sub_5004c8: the volume ramped a step a frame towards its target, each
     * frame mixed by itself; how many frames that took. */
    u32 ramp(u32 ch, u32 frames)
    {
        u32 n = frames;
        u32 done = 0;
        for (;;)
        {
            if (r8(ch + 1) == r8(ch + 4) && r8(ch + 2) == r8(ch + 5))
                return done;
            if (n == 0)
                return done;
            --n;
            if (r8(ch + 1) != r8(ch + 4))
                w8(ch + 1, u8(r8(ch + 1) + r8(ch + 6)));
            if (r8(ch + 2) != r8(ch + 5))
                w8(ch + 2, u8(r8(ch + 2) + r8(ch + 7)));
            setVolume(r32(ch + 0xdbc), ch + 0xd90, s8(ch + 1), s8(ch + 2));
            const u32 need = needed(r32(ch + 0xdb4), ch + 0xd64, 1);
            if (decode(r32(ch + 0xda8), ch + 0xc, need, r32(kDecoded) + 0x10) <= 0)
                return done;
            const u32 block = resample(r32(ch + 0xdb8), ch + 0xd64, 1, r32(kDecoded) + 0x10, r32(kResampled));
            const u32 target = r32(kAccumulator) + ((done << (r8(kShift) & 31)) << 2);
            add(r32(ch + 0xdc0), ch + 0xd90, 1, block, target);
            afterChannel(ch, 1, block, done);
            ++done;
        }
    }

    /* sub_500654: the second volume, from the target and the channel's own. */
    void secondVolume(u32 ch)
    {
        if (r8(ch + 3) == 0)
        {
            w8(ch + 8, 0);
            return;
        }
        w8(ch + 8, u8((i32(s8(ch + 4)) + i32(s8(ch + 5))) >> 1));
        w8(ch + 8, u8((i32(s8(ch + 3)) * i32(s8(ch + 8))) >> 7));
        setVolume(r32(ch + 0xddc), ch + 0xdc4, s8(ch + 8), s8(ch + 8));
    }

    /* sub_500604: a channel whose sound ran out faded to nothing, freed, and
     * the game told. */
    void ended(u32 i)
    {
        const u32 ch = channel(i);
        // sub_5006a4: targets 0 and 0.
        w8(ch + 4, 0);
        w8(ch + 5, 0);
        w8(ch + 6, s8(ch + 1) < 0 ? 1 : 0xff);
        w8(ch + 7, s8(ch + 2) < 0 ? 1 : 0xff);
        secondVolume(ch);
        ramp(ch, 0x100);
        const u32 end = r32(kEndHook);
        if (end != 0x520a00 && end != 0x4ffd80)
            guest(end, 0, 0, 0, 0);
        w8(ch, 0);
        guest(r32(kChannelEnded), i, 0, 0, 0);
    }

    /* sub_520920: the accumulator doubled with saturation into the output,
     * 16 frames at a time from the end. */
    void output(u32 frames, u32 accumulator, u32 target)
    {
        i32 at = i32(frames << 2);
        do
        {
            for (i32 j = at - 64; j < at; j += 2)
                w16(target + u32(j), u16(saturate(2 * i32(s16(accumulator + u32(j))))));
            at -= 64;
        } while (at > 0);
    }

    /* sub_5006f8: one block of at most 256 frames. */
    void block(u32 target, u32 frames)
    {
        for (u32 i = 0; i < kChannelCount; ++i)
        {
            const u32 ch = channel(i);
            if (r8(ch) == 0)
                continue;
            const u32 ramped = ramp(ch, frames);
            const u32 rest = frames - ramped;
            if (rest == 0)
                continue;
            const u32 need = needed(r32(ch + 0xdb4), ch + 0xd64, rest);
            const i32 got = decode(r32(ch + 0xda8), ch + 0xc, need, r32(kDecoded) + 0x10);
            if (got < 0)
                ended(i);
            if (got <= 0)
                continue;
            const u32 source = resample(r32(ch + 0xdb8), ch + 0xd64, rest, r32(kDecoded) + 0x10, r32(kResampled));
            const u32 into = r32(kAccumulator) + ((ramped << (r8(kShift) & 31)) << 2);
            add(r32(ch + 0xdc0), ch + 0xd90, i32(rest), source, into);
            afterChannel(ch, rest, source, ramped);
        }
        const u32 accumulator = r32(kAccumulator);
        beforeOutput(frames, accumulator);
        if (r32(kOutput) == 0x520920)
            output(frames, accumulator, target);
        else
            guest(r32(kOutput), 0, 0, 0, 0, {frames, accumulator, target});
        clearAccumulator(accumulator, frames * 2);
    }

    /* sub_500864: the output, `frames` of it, a block at a time. */
    void mix(u32 target, i32 frames)
    {
        while (frames > 0)
        {
            const u32 now = frames > 0x100 ? 0x100 : u32(frames);
            block(target, now);
            target += r16(kBlockBytes);
            frames -= 0x100;
        }
    }
};

// ------------------------------------------------------------------ checking

struct Region
{
    const char* name;
    u32 start;
    u32 size;
};

struct CheckCounts
{
    unsigned calls = 0;
    unsigned skipped = 0;
    unsigned mismatches = 0;
};

void saveRegions(u8* m, const std::vector<Region>& regions, std::vector<u8>& into)
{
    into.clear();
    for (const Region& r : regions)
        into.insert(into.end(), m + r.start, m + r.start + r.size);
}

void restoreRegions(u8* m, const std::vector<Region>& regions, const std::vector<u8>& from)
{
    size_t at = 0;
    for (const Region& r : regions)
    {
        std::memcpy(m + r.start, from.data() + at, r.size);
        at += r.size;
    }
}

bool checkMix(win32::WinApplication* app, x86::CPU& cpu, u8* m, u32 target, i32 frames)
{
    static CheckCounts counts;
    static bool original = false;
    if (original)
        return false;

    const u32 outputBytes = frames > 0 ? u32(frames) * 4 : 0;
    const std::vector<Region> regions = {
        {"channels", kChannels, kChannelCount * kChannelSize},
        {"buffers", kBuffersStart, kChannels - kBuffersStart},
        {"output", target - 64, outputBytes + 64},
        {"reverb", kReverbStateStart, kReverbStateEnd - kReverbStateStart},
        {"delays", kDelayStart, kDelayEnd - kDelayStart},
        {"filters", kFilterStart, kFilterEnd - kFilterStart},
    };
    std::vector<Region> all = regions;
    for (const Delay& d : kDelays)
    {
        u32 start, end;
        std::memcpy(&start, m + d.start, 4);
        std::memcpy(&end, m + d.end, 4);
        if (start != 0 && end > start && end - start < 0x100000)
            all.push_back({"delay line", start, end - start});
    }
    std::vector<u8> before, mine, theirs;
    saveRegions(m, all, before);
    const x86::CPU entry = cpu;

    bool done = false;
    {
        Mixer mixer{app, cpu, m, true};
        try
        {
            mixer.mix(target, frames);
            done = true;
        }
        catch (const GuestCallInCheck&)
        {
        }
    }
    cpu = entry;
    saveRegions(m, all, mine);
    restoreRegions(m, all, before);

    original = true;
    cpu.esp -= 4;
    const Uint64 started = SDL_GetTicksNS();
    app->dynamic_call(0x500864, cpu);
    /* Past the 2 ms slice the original may have handed the context to the
     * game's thread at a safepoint, which may have changed a channel under it
     * (a volume, a sound started): a difference then can be the check's own. */
    const unsigned long long tookUs = (SDL_GetTicksNS() - started) / 1000;
    original = false;
    cpu = entry;

    ++counts.calls;
    if (!done)
    {
        ++counts.skipped;
    }
    else
    {
        saveRegions(m, all, theirs);
        bool reported = false;
        size_t at = 0;
        for (const Region& r : all)
        {
            unsigned shown = 0;
            for (u32 i = 0; i < r.size; ++i)
            {
                if (mine[at + i] == theirs[at + i])
                    continue;
                if (!reported)
                {
                    reported = true;
                    ++counts.mismatches;
                }
                if (counts.mismatches > 6 || ++shown > 12)
                    break;
                const u32 address = r.start + i;
                if (address >= kChannels && address < kChannels + kChannelCount * kChannelSize)
                    SDL_Log("[SNDCHK] differs at 0x%x, channel %u +0x%x: native %02x, original %02x, before %02x "
                            "(original %llu us)",
                            address, (address - kChannels) / kChannelSize, (address - kChannels) % kChannelSize,
                            mine[at + i], theirs[at + i], before[at + i], tookUs);
                else
                    SDL_Log("[SNDCHK] differs at 0x%x (%s +0x%x): native %02x, original %02x (original %llu us)",
                            address, r.name, i, mine[at + i], theirs[at + i], tookUs);
            }
            at += r.size;
        }
        /* The channels as they were before the block: their functions and
         * the first bytes of their state, to tell which path differed. */
        if (reported && counts.mismatches <= 6)
        {
            SDL_Log("[SNDCHK] block of %d frames into 0x%x", frames, target);
            for (u32 c = 0; c < kChannelCount; ++c)
            {
                const u32 o = c * kChannelSize;
                if (before[o] == 0)
                    continue;
                auto word = [&](u32 offset) {
                    u32 v;
                    std::memcpy(&v, before.data() + o + offset, 4);
                    return v;
                };
                char head[160];
                int n = 0;
                for (u32 b = 0; b < 0x40 && n < int(sizeof head) - 3; ++b)
                    n += SDL_snprintf(head + n, sizeof head - size_t(n), "%02x", before[o + b]);
                SDL_Log("[SNDCHK]  channel %u: decode %x needed %x resample %x volume %x add %x; resampler %08x %08x "
                        "%08x %08x %08x %08x; %s",
                        c, word(0xda8), word(0xdb4), word(0xdb8), word(0xdbc), word(0xdc0), word(0xd64),
                        word(0xd68), word(0xd6c), word(0xd70), word(0xd74), word(0xd78), head);
            }
        }
    }
    if (counts.calls % 2000 == 0)
        SDL_Log("[SNDCHK] %u mixes, %u left to the original (they call the game), %u differences", counts.calls,
                counts.skipped, counts.mismatches);
    cpu.esp += 4;
    return true;
}

}

/* sub_500864, Watcom's registers: the output in eax, the frames in edx. */
bool soundMix(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!mixOn())
        return false;
    u8* m = reinterpret_cast<u8*>(&app->getMemory<x86::reg8>(0));
    const u32 target = cpu.eax;
    const i32 frames = i32(cpu.edx);
    if (mixChecking())
        return checkMix(app, cpu, m, target, frames);
    Mixer mixer{app, cpu, m, false};
    mixer.mix(target, frames);
    cpu.esp += 4;
    return true;
}

}
