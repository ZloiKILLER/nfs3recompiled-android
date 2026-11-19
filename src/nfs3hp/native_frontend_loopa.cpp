#include "native_frontend_loopa.h"
#include "native_load_saves.h"
#include "native_load_track.h"
#include "native_aiplan.h"
#include "native_track.h"
#include "native_carstate.h"
#include "native_watcom.h"
#include "native_game.h"
#include "game_settings.h"
#include "native_controls_input.h"
#include "native_ea_memory.h"
#include "native_ea_net.h"
#include "native_ea_support.h"
#include "native_ea_formats.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_fx.h"
#include "native_frontend_eaa.h"
#include "native_frontend_eab.h"
#include "native_frontend_inpa.h"
#include "native_frontend_loada.h"
#include "native_frontend_loopb.h"
#include "native_frontend_menua.h"
#include "native_frontend_menub.h"
#include "native_frontend_menuc.h"
#include "native_frontend_menz.h"
#include "native_frontend_neta.h"
#include "native_frontend_pause.h"
#include "native_frontend_rendd.h"
#include "native_frontend_rende.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_frontend_sime.h"
#include "native_frontend_wata.h"
#include "native_frontend_wina.h"
#include "native_graphics.h"
#include "port_sound_calls.h"
#include "native_load_cardb.h"
#include "port_hud.h"
#include "port_text.h"
#include "native_vertices.h"
#include "port_snd_stream_calls.h"
#include "port_sound_music.h"
#include "port_sound_racesetup.h"
#include "native_render.h"
#include "port_timer.h"
#include <nfs3hp.h>
#include <game_float.h>
#include <cstdint>
#include <cstring>
#include <iterator>

/* The game's loop, decompiled:
 *
 *   the game         its start (sub_4a3390, sub_4b6020) and end (sub_4a33a0), the table
 *                    of reciprocal roots made at the start (sub_497430), whether the game
 *                    is installed "remote" (sub_4b62a0);
 *   a race           one session of it (sub_4a3690): its loading (sub_4a3400), its restart
 *                    (sub_4a33b0), the loop that runs until it ends, what it hands back;
 *   the ghost        a car's pose recorded a tick in eight (sub_477c20) and read back
 *                    between two of them (sub_477d90), the recording begun (sub_477a50),
 *                    its file written and read (sub_4777d0, sub_477b50);
 *   a pose           18 bytes: a word of the car's, the road's slice and an offset from it
 *                    in three words (sub_4778c0, sub_477850), then a rotation of four words
 *                    (sub_477830, sub_4b6e80, sub_4b6ca0, sub_4b6d90), blended by
 *                    sub_4b6a30;
 *   small ones       the panel behind the menus' items (sub_4a75e0 and its two quads), a
 *                    frame round an item (sub_459410), the 3D font let go (sub_475b10),
 *                    the loading bar's corners scaled (sub_495190).
 *
 * The replays depend on the poses, so all of it is held to the bit: the integers wrap as
 * the original's do, and the floats are written out through fp:: in the original's order,
 * each value stored and fetched back where the original stores and fetches it.  The state
 * stays in guest memory, what these call is called by address, and the locals the game's
 * functions read are in the frame where they were.  What a function leaves in the registers
 * it does not save is left too, and where a callee reads a register left over from the code
 * before it (specs_from_exe.py names them) it finds there what the original had.
 *
 * Plan step 5: each function a typed core in frontend::loopa (native_frontend_loopa.h), in a
 * frame of its own where the game's function has one and calls anything (fe::Frame: what it calls
 * runs at the stack addresses it did), and a thunk under the game's name for the calls by
 * address.  A core of this file calls another, and native_frontend_loopb.cpp's, as C++
 * (guest::direct), and the typed cores of other files too (fe::direct); the rest of the game still
 * by address.  Before such a call the registers the call by address would load and leave are set
 * where what follows, or the function's end, meets them. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

/* The port's own, by the patches of the port's race state change and
 * the port's loading screen change: the mark of a race, the end of the 4:3 loading screen
 * (nfs3hp_main.cpp). */
void raceRunning(bool running);
void loadingScreenFit(nfs::App* app, nfs::ThreadState& ts, bool on);

namespace
{

using fe::args;
using fe::Guest;
using fe::GuestString;
using nfs::Float;
using nfs::u16;
using nfs::u32;
using nfs::u64;
using nfs::s16;
using nfs::s32;
using nfs::s64;

// ---------------------------------------------------------------------------
// The game's data
// ---------------------------------------------------------------------------

// The race.
constexpr u32 kRaceAbort = 0x55e88c;         // nonzero: the load was stopped
constexpr u32 kRaceToggle = 0x552854;        // flips with each session
constexpr u32 kRaceEnd = 0x7d36b4;           // how it ended: 0 finished, 1 restart, else left
constexpr u32 kRaceLooping = 0x7d3698;       // nonzero: the loop goes round
constexpr u32 kMusicOn = 0x55e890;           // byte
constexpr u32 kNetworkSpoken = 0x7a22dc;     // word
constexpr u32 kNetworkHost = 0x7a22e0;
constexpr u32 kNetworkClosed = 0x7a22de;     // word
constexpr u32 kSettingsDwords = 0x83c;
constexpr u32 kChosenWidth = 0x55fec4;

// What a session returns to the menus.
constexpr u32 kSessionMenus = 0xb, kSessionRestart = 0xa, kSessionNext = 0xc;

// A session's calls.
constexpr u32 kTakeDownRace = 0x4a35d0, kCloseNetwork = 0x4a65e0;
constexpr u32 kRaceMusicStart = 0x4107c0;

// A load's calls.
constexpr u32 kScreenMode = 0x4bef50, kViewport = 0x4beb80;
constexpr u32 kFlushKeys = 0x4f0240;
constexpr u32 kInitCameras = 0x422cf0;

// The loading bar's steps, floats' bits, on its way to 100.
constexpr u32 kProgress5 = 0x40a00000, kProgress10 = 0x41200000, kProgress35 = 0x420c0000;
constexpr u32 kProgress40 = 0x42200000, kProgress70 = 0x428c0000, kProgress75 = 0x42960000;
constexpr u32 kProgress90 = 0x42b40000, kProgress93 = 0x42ba0000, kProgress95 = 0x42be0000;
constexpr u32 kProgress100 = 0x42c80000;

// The road's slices.
constexpr u32 kSlices = 0x5dd948;
constexpr u32 kSliceBytes = 36;

// The clock.
constexpr u32 kTick = 0x7d3684;

// The ghost: what is being recorded, settings first and its poses after, and what was loaded.
constexpr u32 kRecording = 0x679050;
constexpr u32 kRecordedTicks = 0x67b140;
constexpr u32 kRecordedPoses = 0x67b144;
constexpr u32 kLoaded = 0x6ba5c4;
constexpr u32 kLoadedTicks = 0x6bc6b4;
constexpr u32 kLoadedPoses = 0x6bc6b8;
constexpr u32 kGhostBytes = 0x41574;
constexpr u32 kGhostHeaderBytes = 0x20f4;    // the settings and the ticks' count
constexpr s32 kGhostMostTicks = 0x3840;
constexpr u32 kPoseBytes = 18;
constexpr u32 kPoseRotation = 0xa;
constexpr u32 kGhostLoaded = 0x55d000;       // 1: a ghost's poses are there to read
constexpr u32 kKeptDwords = 0x5db;           // the settings a loaded ghost's leave as they were

constexpr u32 kFillZero = 0x4e070c;

// A car's pose.
constexpr u32 kCarWord = 6, kCarSlice = 0x1a;  // dwords whose upper half is the word
constexpr u32 kCarPosition = 0x98, kCarMatrix = 0xc0;

// ---------------------------------------------------------------------------
// The arithmetic these share
// ---------------------------------------------------------------------------

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, u32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

void copyDwords(Guest& guest, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* The upper half of the dword at `address`, a signed word as `mov; sar 16` makes it. */
s32 upperWord(const Guest& guest, u32 address)
{
    return s32(guest.dword(address)) >> 16;
}

/* The game's fixed-point product, 16.16, inline at each of its uses: `imul; shl edx, 16;
 * shr eax, 16; adc eax, edx`.  The bit shifted out rounds it; the sum wraps. */
u32 fixedProduct(s32 a, s32 b, u32& shiftedHigh)
{
    const u64 product = u64(s64(a) * s64(b));
    shiftedHigh = u32(product >> 32) << 16;
    const u32 low = u32(product);
    return (low >> 16) + shiftedHigh + ((low >> 15) & 1);
}

u32 fixedProduct(s32 a, s32 b)
{
    u32 shiftedHigh;
    return fixedProduct(a, b, shiftedHigh);
}

/* The game's square root of a 16.16 number, inline at each of its uses: a straight line
 * for each power of two, from the tables at 0x5661c4 and 0x566248; 0 of 0. */
u32 fixedRoot(const Guest& guest, u32 value)
{
    if (value == 0)
        return 0;
    const u32 power = u32(31 - __builtin_clz(value));
    const u64 product = u64(value) * u64(guest.dword(0x5661c4 + power * 4));
    return ((u32(product) >> 16) | (u32(product >> 32) << 16)) + guest.dword(0x566248 + power * 4);
}

// ---------------------------------------------------------------------------
// What the functions below share
// ---------------------------------------------------------------------------

/* A float of its bits, as fld dword reads a stack argument. */
float floatOf(u32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* The tick's place in a recording: one pose a tick in eight, rounded towards 0 as Watcom's
 * `sar; shl; sbb; sar` does it. */
u32 poseIndex(u32 tick)
{
    return u32(s32(tick) / 8);
}

/* What the clipper gets for a vertex, the original's inline test four times over: a float's
 * bits as integers, so a negative one, its top bit set, is beyond the low side at once. */
constexpr u32 kClipFar = 0x7d34f8, kClipLeft = 0x7d34fc, kClipRight = 0x7d350c, kClipTop = 0x7d3504, kClipBottom = 0x7d3500;

std::uint8_t clipCode(const Guest& guest, u32 vertex)
{
    const u32 depth = guest.dword(vertex + 0xc);
    if ((depth & 0x80000000) != 0 || s32(depth) >= s32(guest.dword(kClipFar)))
        return 0x10;
    std::uint8_t code = 0;
    const u32 y = guest.dword(vertex + 4);
    if ((y & 0x80000000) != 0 || s32(y) < s32(guest.dword(kClipTop)))
        code |= 8;
    else if (s32(y) > s32(guest.dword(kClipBottom)))
        code |= 4;
    const u32 x = guest.dword(vertex);
    if ((x & 0x80000000) != 0 || s32(x) < s32(guest.dword(kClipLeft)))
        code |= 1;
    else if (s32(x) > s32(guest.dword(kClipRight)))
        code |= 2;
    return code;
}

/* Four vertices of 32 bytes in the frame. */
constexpr u32 kVertexBytes = 0x20, kVertexColour = 0x10, kVertexCode = 0x14, kVertexU = 0x18, kVertexV = 0x1c;

void setPlace(Guest& guest, u32 vertex, u32 x, u32 y)
{
    guest.setFloat(vertex, float(Float(s32(x))));
    guest.setFloat(vertex + 4, float(Float(s32(y))));
}

u32 clipAndDraw(Guest& guest, u32 frame)
{
    for (u32 i = 0; i < 4; ++i)
        guest.setByte(frame + i * kVertexBytes + kVertexCode, clipCode(guest, frame + i * kVertexBytes));
    return fe::direct<0x4c12e0, vertices::quadClippedAt>(guest.app(), guest.ts(), guest.ts().sp - 4, frame, frame + kVertexBytes,
                                                       frame + 2 * kVertexBytes, frame + 3 * kVertexBytes);
}

constexpr GuestString kInitStrings[] = {
    { 0x53fe08, "Need For Speed III" },
    { 0x53fe1c, "Out of memory! %d" },
};

/* A game function's stack arguments, before its frame. */
u32 stackArguments(const nfs::ThreadState& ts)
{
    return ts.sp + 4;
}

u32 stackArgument(nfs::App* app, const nfs::ThreadState& ts, u32 place)
{
    return app->getMemory<u32>(stackArguments(ts) + 4 * place);
}

constexpr std::uint8_t kSavesNone = 0;

}  // namespace

// ---------------------------------------------------------------------------
// A pose
// ---------------------------------------------------------------------------

/* sub_4b6a30: the rotation and the position `fraction` (16.16) of the way from the first
 * to the second, each part on its own: the rotation is not made unit again. */
nfs::u32 frontend::loopa::blend(App* app, ThreadState& ts, u32 rotationFrom, u32 rotationTo, u32 positionFrom,
                                  u32 positionTo, u32 fraction, u32 rotationOut, u32 positionOut)
{
    Guest guest(app, ts);
    const s32 to = s32(fraction);
    const s32 from = s32(0x10000 - fraction);
    const auto part = [&](s32 a, s32 b) { return fixedProduct(a, from) + fixedProduct(b, to); };

    guest.setWord(rotationOut, u16(part(s16(guest.word(rotationFrom)), s16(guest.word(rotationTo)))));
    guest.setWord(rotationOut + 2, u16(part(upperWord(guest, rotationFrom), upperWord(guest, rotationTo))));
    guest.setWord(rotationOut + 4, u16(part(upperWord(guest, rotationFrom + 2), upperWord(guest, rotationTo + 2))));
    guest.setWord(rotationOut + 6, u16(part(upperWord(guest, rotationFrom + 4), upperWord(guest, rotationTo + 4))));

    guest.setDword(positionOut, part(s32(guest.dword(positionFrom)), s32(guest.dword(positionTo))));
    guest.setDword(positionOut + 4, part(s32(guest.dword(positionFrom + 4)), s32(guest.dword(positionTo + 4))));
    const u32 first = fixedProduct(s32(guest.dword(positionFrom + 8)), from);
    u32 shiftedHigh;
    const u32 sum = first + fixedProduct(s32(guest.dword(positionTo + 8)), to, shiftedHigh);
    guest.setDword(positionOut + 8, sum);
    return positionOut;
}

/* sub_4b6ca0: the matrix of a rotation of four words, its nine parts 16.16. */
nfs::u32 frontend::loopa::matrixOfRotation(App* app, ThreadState& ts, u32 rotation, u32 out)
{
    Guest guest(app, ts);
    constexpr u32 kOne = 0x10000000;
    const u32 x = u32(s32(s16(guest.word(rotation))));
    const u32 y = u32(upperWord(guest, rotation));
    const u32 z = u32(upperWord(guest, rotation + 2));
    const u32 w = u32(upperWord(guest, rotation + 4));

    const u32 x2 = x + x, y2 = y + y, z2 = z + z;
    const u32 xw = x2 * w, yw = w * y2, zw = w * z2;
    const u32 xx = x2 * x, xy = x * y2, xz = x * z2;
    const u32 yy = y2 * y, yz = y * z2, zz = z2 * z;
    const u32 oneLessXX = kOne - xx;
    const auto scaled = [](u32 value) { return u32(s32(value) >> 12); };

    guest.setDword(out, scaled(kOne - yy - zz));
    guest.setDword(out + 4, scaled(zw + xy));
    guest.setDword(out + 8, scaled(xz - yw));
    guest.setDword(out + 0xc, scaled(xy - zw));
    guest.setDword(out + 0x10, scaled(oneLessXX - zz));
    guest.setDword(out + 0x14, scaled(xw + yz));
    guest.setDword(out + 0x18, scaled(xz + yw));
    guest.setDword(out + 0x1c, scaled(yz - xw));
    const u32 last = scaled(oneLessXX - yy);
    guest.setDword(out + 0x20, last);
    return last;
}

/* sub_4b6d90: the second rotation negated when that brings it nearer the first, the same
 * rotation the other way round: what makes the blend between two poses go the short way.
 * The squares of each sum and each difference, wrapping as the original's do. */
nfs::u32 frontend::loopa::rotationNearest(App* app, ThreadState& ts, u32 rotation, u32 other)
{
    Guest guest(app, ts);
    const s32 a0 = s16(guest.word(rotation)), b0 = s16(guest.word(other));
    const s32 a1 = upperWord(guest, rotation), b1 = upperWord(guest, other);
    const s32 a2 = upperWord(guest, rotation + 2), b2 = upperWord(guest, other + 2);
    const s32 a3 = upperWord(guest, rotation + 4), b3 = upperWord(guest, other + 4);
    const auto square = [](s32 value) { return u32(value) * u32(value); };

    const s32 sums = s32(square(a0 + b0) + square(a1 + b1) + square(a2 + b2) + square(a3 + b3)) >> 14;
    const s32 differences = s32(square(a0 - b0) + square(a1 - b1) + square(a2 - b2) + square(a3 - b3)) >> 14;
    if (sums < differences)
    {
        const u16 first = guest.word(other), second = guest.word(other + 2);
        const u16 third = guest.word(other + 4), fourth = guest.word(other + 6);
        guest.setWord(other + 2, u16(0 - second));
        guest.setWord(other + 4, u16(0 - third));
        guest.setWord(other + 6, u16(0 - fourth));
        guest.setWord(other, u16(0 - first));
    }
    return u32(sums);
}

/* sub_4b6e80: the rotation of a matrix, the usual way: by the trace when that is positive,
 * else by the largest of the diagonal.  Its four parts are worked in the frame and stored
 * a quarter of that; eax the last of them, edx still where. */
nfs::u32 frontend::loopa::rotationOfMatrix(App* app, ThreadState& ts, u32 matrix, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x3c);
    constexpr u32 kNextAxis[3] = { 1, 2, 0 };  // the original's copy of the table at 0x4b6a10
    const u32 part = guest.frame();            // [ebp - 0x3c]: x, y, z, w
    const auto at = [&](u32 row, u32 column) { return guest.dword(matrix + row * 0xc + column * 4); };
    const auto difference = [](u32 a, u32 b) { return s32(a - b); };

    const u32 trace = at(0, 0) + at(1, 1) + at(2, 2);
    if (s32(trace) > 0)
    {
        const u32 root = fixedRoot(guest, trace + 0x10000);
        guest.setDword(part + 12, u32(s32(root) >> 1));
        const s32 inverse = s32(0x80000000u / root);
        guest.setDword(part, fixedProduct(difference(at(1, 2), at(2, 1)), inverse));
        guest.setDword(part + 4, fixedProduct(difference(at(2, 0), at(0, 2)), inverse));
        guest.setDword(part + 8, fixedProduct(difference(at(0, 1), at(1, 0)), inverse));
    }
    else
    {
        u32 i = s32(at(1, 1)) > s32(at(0, 0)) ? 1 : 0;
        if (s32(at(i, i)) < s32(at(2, 2)))
            i = 2;
        const u32 j = kNextAxis[i];
        const u32 k = kNextAxis[j];

        const u32 root = fixedRoot(guest, at(i, i) - (at(j, j) + at(k, k)) + 0x10000);
        guest.setDword(part + i * 4, u32(s32(root) >> 1));
        const s32 inverse = root != 0 ? s32(0x80000000u / root) : 0;
        guest.setDword(part + 12, fixedProduct(difference(at(j, k), at(k, j)), inverse));
        guest.setDword(part + j * 4, fixedProduct(s32(at(i, j) + at(j, i)), inverse));
        guest.setDword(part + k * 4, fixedProduct(s32(at(i, k) + at(k, i)), inverse));
    }

    guest.setWord(out, u16(s32(guest.dword(part)) >> 2));
    guest.setWord(out + 2, u16(s32(guest.dword(part + 4)) >> 2));
    guest.setWord(out + 4, u16(s32(guest.dword(part + 8)) >> 2));
    const u32 last = u32(s32(guest.dword(part + 12)) >> 2);
    guest.setWord(out + 6, u16(last));
    return last;
}

/* sub_477830: a pose's rotation from a matrix. */
nfs::u32 frontend::loopa::poseSetRotation(App* app, ThreadState& ts, u32 pose, u32 matrix)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    return guest::direct<0x4b6e80, rotationOfMatrix>(app, ts, matrix, pose + kPoseRotation);
}

/* sub_4778c0: a position stored in a pose: the slice's word and the car's, then the offset
 * from the slice in three words.  The x87's own order: each slice's coordinate is stored as
 * a float and fetched back before it is taken off, and the last two offsets are stored and
 * fetched back before they are scaled, where the first goes on at the x87's width. */
nfs::u32 frontend::loopa::poseSetPosition(App* app, ThreadState& ts, u32 pose, u32 x, u32 y, u32 z, u32 slice,
                                            u32 car)
{
    Guest guest(app, ts);
    const u32 sliceWord = slice;
    const u32 carWord = car;
    const u32 sliceAt = guest.dword(kSlices) + u32(s32(s16(u16(sliceWord)))) * kSliceBytes;
    const Float toUnits = doubleAt(guest, 0x53b0b8);
    const Float toOffset = doubleAt(guest, 0x53b0c0);

    const Float sliceX = fp::mul(Float(s32(guest.dword(sliceAt))), toUnits);
    const Float sliceY = fp::mul(Float(s32(guest.dword(sliceAt + 4))), toUnits);
    const Float atX = Float(floatOf(x));
    const Float sliceZInteger = Float(s32(guest.dword(sliceAt + 8)));
    const float storedX = float(sliceX);
    const Float offsetX = fp::sub(atX, Float(storedX));
    const Float sliceZ = fp::mul(toUnits, sliceZInteger);
    const Float scaledX = fp::mul(offsetX, toOffset);
    const Float atY = Float(floatOf(y));
    const Float atZ = Float(floatOf(z));
    guest.setWord(pose + 2, u16(sliceWord));
    const float storedY = float(sliceY);
    const float storedZ = float(sliceZ);
    guest.setWord(pose, u16(carWord));
    const Float offsetY = fp::sub(atY, Float(storedY));
    const Float offsetZ = fp::sub(atZ, Float(storedZ));
    const float keptY = float(offsetY);
    const float keptZ = float(offsetZ);

    const auto stored = [](const Float& value) { return s32(fp::toInteger<s32>(value)) >> 8; };
    guest.setWord(pose + 4, u16(stored(scaledX)));
    guest.setWord(pose + 6, u16(stored(fp::mul(Float(keptY), toOffset))));
    const s32 last = stored(fp::mul(Float(keptZ), toOffset));
    guest.setWord(pose + 8, u16(last));
    return u32(last);
}

/* sub_477850: a pose's position again: its slice's point and its offset.  `out` is esi, and
 * esi is left past the copy in the frame, as the movsd left it. */
nfs::u32 frontend::loopa::posePosition(App* app, ThreadState& ts, u32 pose, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x18);
    const u32 slice = guest.dword(kSlices) + u32(upperWord(guest, pose)) * kSliceBytes;
    const u32 x = guest.dword(slice) + (u32(upperWord(guest, pose + 2)) << 8);
    const u32 y = guest.dword(slice + 4) + (u32(upperWord(guest, pose + 4)) << 8);
    const u32 z = guest.dword(slice + 8) + (u32(upperWord(guest, pose + 6)) << 8);
    guest.setDword(out, x);
    guest.setDword(out + 4, y);
    guest.setDword(out + 8, z);
    return out;
}

/* sub_477990: the position and matrix `fraction` of the way between two poses. */
nfs::u32 frontend::loopa::poseBetween(App* app, ThreadState& ts, u32 from, u32 to, u32 fraction, u32 positionOut,
                                        u32 matrixOut)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x20);
    const u32 ebp = guest.frame() + 0x20;
    const u32 positionFrom = ebp - 0x14, positionTo = ebp - 0x20, rotation = ebp - 8;
    guest::direct<0x477850, posePosition>(app, ts, from, positionFrom);
    guest::direct<0x477850, posePosition>(app, ts, to, positionTo);
    // What sub_4b6a30 leaves in ebx and ecx its only caller (sub_477d90, as C++) does not read.
    guest::direct<0x4b6a30, blend>(app, ts, from + kPoseRotation, to + kPoseRotation, positionFrom, positionTo,
                                   fraction, rotation, positionOut);
    return guest::direct<0x4b6ca0, matrixOfRotation>(app, ts, rotation, matrixOut);
}

// ---------------------------------------------------------------------------
// The ghost
// ---------------------------------------------------------------------------

/* sub_4779e0: the recording's pose of an index, or none past its end. */
nfs::u32 frontend::loopa::recordedAt(App*, ThreadState&, u32 index)
{
    return s32(index) >= kGhostMostTicks ? u32(0) : kRecordedPoses + index * kPoseBytes;
}

/* sub_477a10: the loaded ghost's pose of an index, past its end the last but one. */
nfs::u32 frontend::loopa::loadedAt(App* app, ThreadState& ts, u32 index)
{
    Guest guest(app, ts);
    const u32 ticks = guest.dword(kLoadedTicks);
    const u32 place = s32(index) < s32(ticks - 1) ? index : ticks - 2;
    return kLoadedPoses + place * kPoseBytes;
}

/* sub_477a50: a recording begun, with the settings of the race, unless this kind of game
 * records none. */
nfs::u32 frontend::loopa::recordingStart(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (gameSettings(guest).replayMode != 0)
        return 0;
    const u32 result = fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kRecording, kGhostBytes);
    copyDwords(guest, kRecording, SETTINGS_ADDRESS(fileSize), kSettingsDwords);
    return result;
}

/* sub_477c20: a car's pose recorded at this tick, when the recording goes on and has room,
 * and its rotation made near the last one's.  What is handed back is what is left in eax. */
nfs::u32 frontend::loopa::recordCar(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x30);
    const u32 matrix = guest.frame();  // [ebp - 0x30]: 16.16
    const u32 index = poseIndex(guest.dword(kTick));
    if (gameSettings(guest).replayMode != 0)
        return index;
    const u32 pose = guest::direct<0x4779e0, recordedAt>(app, ts, index);
    if (pose == 0)
        return 0;

    guest::direct<0x4778c0, poseSetPosition>(
        app, ts, pose, guest.dword(car + kCarPosition), guest.dword(car + kCarPosition + 4),
        guest.dword(car + kCarPosition + 8), u32(upperWord(guest, car + kCarSlice)), u32(upperWord(guest, car + kCarWord)));
    const Float toFixed = doubleAt(guest, 0x53b0d8);
    for (u32 i = 0; i < 9; ++i)
    {
        const Float scaled = fp::mul(Float(guest.floatAt(car + kCarMatrix + i * 4)), toFixed);
        guest.setDword(matrix + i * 4, u32(fp::toInteger<s32>(scaled)));
    }
    guest::direct<0x477830, poseSetRotation>(app, ts, pose, matrix);
    const u32 before = guest::direct<0x4779e0, recordedAt>(app, ts, index - 1);
    // What the calls leave in eax: the last one's.
    u32 left = before;
    if (before != 0)
        left = guest::direct<0x4b6d90, rotationNearest>(app, ts, before + kPoseRotation, pose + kPoseRotation);
    guest.setDword(kRecordedTicks, index);
    return left;
}

/* sub_477d90: the loaded ghost's pose at this tick, between the two recorded round it: 1, or
 * 0 when no ghost is loaded, edx then left the tick's index. */
nfs::u32 frontend::loopa::playbackPose(App* app, ThreadState& ts, u32 positionOut, u32 matrixOut)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const u32 index = poseIndex(guest.dword(kTick));
    if (guest.dword(kGhostLoaded) != 1)
    {
        return 0;
    }
    const u32 from = guest::direct<0x477a10, loadedAt>(app, ts, index);
    const u32 to = guest::direct<0x477a10, loadedAt>(app, ts, index + 1);
    const u32 fraction = (guest.dword(kTick) & 7) << 13;
    guest::direct<0x477990, poseBetween>(app, ts, from, to, fraction, positionOut, matrixOut);
    return 1;
}

/* sub_4777d0: the recording written to a file, only by one player alone in the kinds of
 * race that keep one. */
nfs::u32 frontend::loopa::saveGhost(App* app, ThreadState& ts, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    if (name == 0)
        return 0;
    const u32 ticks = guest.dword(kRecordedTicks);
    if (gameSettings(guest).playMode != 0)
        return 0;
    const u32 kind = gameSettings(guest).raceType;
    if (kind == 1 || kind == 2)
        return 0;
    fe::direct<0x4eeae0, load::blockSaved>(app, ts, name, kRecording, ticks * kPoseBytes + kGhostHeaderBytes);
    return 0;
}

/* sub_477b50: a ghost's file read into the place for it, its settings taken but for those
 * the frame kept.  0 when it is not there, is too long or is not valid. */
nfs::u32 frontend::loopa::loadGhost(App* app, ThreadState& ts, u32 name)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x176c);
    const u32 kept = guest.frame();
    copyDwords(guest, kept, SETTINGS_ADDRESS(language), kKeptDwords);
    if (!*fe::direct<0x4e0e60, ea::fileThere>(app, ts, name))
        return 0;
    if (fe::direct<0x4e43a0, frontend::eaa::fileSize>(app, ts, name) > kGhostBytes)
        return 0;
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.ts(), kLoaded, kGhostBytes);
    fe::direct<0x4f22e0, frontend::eab::fileLoadInto>(app, ts, name, kLoaded);
    if (fe::direct<0x478100, frontend::loada::savedResolveEntries>(app, ts, kLoaded) == 0)
        return 0;
    copyDwords(guest, SETTINGS_ADDRESS(fileSize), kLoaded, kSettingsDwords);
    guest.setDword(kGhostLoaded, 1);
    copyDwords(guest, SETTINGS_ADDRESS(language), kept, kKeptDwords);
    return 1;
}

// ---------------------------------------------------------------------------
// The panel
// ---------------------------------------------------------------------------

/* sub_4a7080: a flat quad of one colour, from the template at 0x4a6f80. */
nfs::u32 frontend::loopa::panelQuad(App* app, ThreadState& ts, u32 x0, u32 y0, u32 x1, u32 y1, u32 x2, u32 y2,
                                      u32 x3, u32 y3, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x90);
    const u32 quad = guest.frame();
    copyDwords(guest, quad, 0x4a6f80, 4 * kVertexBytes / 4);
    setPlace(guest, quad, x0, y0);
    setPlace(guest, quad + kVertexBytes, x1, y1);
    setPlace(guest, quad + 2 * kVertexBytes, x2, y2);
    setPlace(guest, quad + 3 * kVertexBytes, x3, y3);
    for (u32 i = 0; i < 4; ++i)
        guest.setDword(quad + i * kVertexBytes + kVertexColour, colour);
    fe::direct<0x431900, graphics::sendState>(app, ts, 1, 0);
    return clipAndDraw(guest, quad);
}

/* sub_4a7310: a quad of a texture, from the template at 0x4a7000: the fifth to twelfth
 * stack arguments its corners on the texture, then the texture and the colour. */
nfs::u32 frontend::loopa::panelTexturedQuad(App* app, ThreadState& ts, u32 x0, u32 y0, u32 x1, u32 y1, u32 x2,
                                              u32 y2, u32 x3, u32 y3, u32 u0, u32 v0, u32 u1, u32 v1,
                                              u32 u2, u32 v2, u32 u3, u32 v3, u32 texture, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x90);
    const u32 quad = guest.frame();
    copyDwords(guest, quad, 0x4a7000, 4 * kVertexBytes / 4);
    setPlace(guest, quad, x0, y0);
    setPlace(guest, quad + kVertexBytes, x1, y1);
    setPlace(guest, quad + 2 * kVertexBytes, x2, y2);
    setPlace(guest, quad + 3 * kVertexBytes, x3, y3);
    const u32 u[4] = { u0, u1, u2, u3 }, v[4] = { v0, v1, v2, v3 };
    for (u32 i = 0; i < 4; ++i)
    {
        const u32 vertex = quad + i * kVertexBytes;
        guest.setDword(vertex + kVertexColour, colour);
        guest.setDword(vertex + kVertexU, u[i]);
        guest.setDword(vertex + kVertexV, v[i]);
    }
    fe::direct<0x431900, graphics::sendState>(app, ts, 1, texture);
    return clipAndDraw(guest, quad);
}

/* sub_4a75e0: the panel, sized by the view: a dark quad, its edges shaded (two dark, two
 * light) and three pieces of the texture at [0x749a38] across its top: the left end, the
 * middle and the right end. */
nfs::u32 frontend::loopa::panelDraw(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 0x18);
    constexpr u32 kPanelTexture = 0x749a38;
    constexpr u32 kBody = 0x3f10063f, kDark = 0x20000000, kLight = 0x20ffffff, kBar = 0xafffffff;
    constexpr u32 kBarTop = 0x3f440000, kBarBottom = 0x3f630000;
    const u32 ebp = guest.frame() + 0x18;
    fe::direct<kViewport, port::text::viewportOf>(app, ts, ebp - 0x18, ebp - 0x14, ebp - 0x10, ebp - 0xc);
    const u32 viewX = guest.dword(ebp - 0x18), viewY = guest.dword(ebp - 0x14);
    const u32 viewWidth = guest.dword(ebp - 0x10), viewHeight = guest.dword(ebp - 0xc);
    const auto divided = [](u32 value, s32 by) { return u32(s32(value) / by); };

    const u32 barHeight = divided(viewHeight * 32, 960);
    const u32 barEnd = divided(viewWidth * 21, 640);
    const u32 barReach = divided(viewWidth * 16, 640);
    const u32 border = divided(viewWidth * 3 + 0x140, 640);
    const u32 left = viewX + divided(viewWidth, 20);
    const u32 top = viewY + divided(viewHeight, 20) + barHeight;
    const u32 right = left + divided(viewWidth * 18, 20);
    const u32 bottom = top + divided(viewHeight * 18, 20);

    guest::direct<0x4a7080, panelQuad>(app, ts, left, top, right, top, right, bottom, left, bottom, kBody);
    guest::direct<0x4a7080, panelQuad>(app, ts, left + border, bottom - border, right - border, bottom - border, right,
                                       bottom, left, bottom, kDark);
    guest::direct<0x4a7080, panelQuad>(app, ts, right - border, top + border, right, top, right, bottom,
                                       right - border, bottom - border, kDark);
    guest::direct<0x4a7080, panelQuad>(app, ts, left, top, right, top, right - border, top + border, left + border,
                                       top + border, kLight);
    guest::direct<0x4a7080, panelQuad>(app, ts, left, top, left + border, top + border, left + border,
                                       bottom - border, left, bottom, kLight);

    const u32 barLeft = left - barReach;
    const u32 barRight = barLeft + (right - left) + barReach * 2;
    const u32 barY = top - barHeight - barHeight;
    const auto bar = [&](u32 from, u32 to, u32 u0, u32 u1) {
        return guest::direct<0x4a7310, panelTexturedQuad>(app, ts, from, barY, to, barY, to, top, from, top, u0,
                                                          kBarTop, u1, kBarTop, u1, kBarBottom, u0, kBarBottom,
                                                          guest.dword(kPanelTexture), kBar);
    };
    bar(barLeft, barLeft + barEnd, 0x3f4d0000, 0x3f620000);
    bar(barLeft + barEnd, barRight - barEnd, 0x3f7b0000, 0x3f7b0000);
    return bar(barRight - barEnd, barRight, 0x3f648000, 0x3f790000);
}

// ---------------------------------------------------------------------------
// A race
// ---------------------------------------------------------------------------

/* sub_4a3400: a race set up and loaded, the bar going up, each stage free to stop it: 1, and
 * then the session takes the race down, or 0.  The first stop goes through the end of the
 * loading screen and the others do not, as the port's patch has it; raceRunning(false) in
 * the race's taking down ends it for them. */
nfs::u32 frontend::loopa::raceLoad(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x10);
    const u32 ebp = guest.frame() + 0x10;
    const auto progress = [&](u32 percent) {
        // The per cent in the word the code pushes, the bar handed its address.
        const u32 pushed = ts.sp - 4;
        guest.setDword(pushed, percent);
        fe::direct<0x495230, graphics::loadingProgress>(app, ts, pushed);
    };
    const auto stopped = [&] {
        fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, ts);
        return guest.dword(kRaceAbort) != 0;
    };

    raceRunning(true);
    fe::direct<0x431900, graphics::sendState>(app, ts, 0xd, 1);
    const u32 width = fe::direct<0x4482f0, frontend::menua::chosenModeWidth>(app, ts);
    guest.setDword(kChosenWidth, width);
    // The width in eax, where sub_4482f0 leaves it for the game's code.
    fe::direct<0x4bef50, render::screenModeSet>(app, ts, width);
    guest::direct<0x4be5e0, loopb::viewsLayout>(
        app, ts, (std::uint8_t(gameSettings(guest).entries[0].kind) & 8) != 0 ? u32(1) : u32(0));
    u32 eax = fe::direct<kViewport, port::text::viewportOf>(app, ts, ebp - 8, ebp - 4, ebp - 0x10, ebp - 0xc);
    fe::direct<0x4352d0, controls::boundPointer>(app, ts, 0, 0, guest.dword(ebp - 0x10), guest.dword(ebp - 0xc));
    // The HUD's loading text and scales as C++, eax as the calls by address leave it for the next.
    eax = u32(*fe::direct<0x484040, port::hud::loadingTextOf>(app, ts, ts.sp - 4, eax));
    eax = u32(*fe::direct<0x484b70, port::hud::screenScalesOf>(app, ts, ts.sp - 4));
    eax = *fe::direct<0x494fa0, graphics::loadingScreenAnswer>(app, ts, eax);
    eax = u32(*fe::direct<0x484040, port::hud::loadingTextOf>(app, ts, ts.sp - 4, eax));
    guest::direct<0x477a50, recordingStart>(app, ts);
    progress(kProgress5);
    fe::direct<kFlushKeys, ea::menuKeysFlushed>(app, ts);
    fe::direct<0x477f60, frontend::neta::raceLoadStep>(app, ts);
    if (stopped())
    {
        loadingScreenFit(app, ts, false);
        return 1;
    }
    progress(kProgress10);
    fe::direct<0x477fd0, frontend::neta::raceMusicStep>(app, ts);
    if (stopped())
        return 1;
    progress(kProgress35);
    fe::direct<0x4d56f0, frontend::rende::loadEffects>(app, ts);
    if (stopped())
        return 1;
    progress(kProgress40);
    if (fe::direct<0x4dbbf0, frontend::sime::loadTrackPieces>(app, ts, gameSettings(guest).track) != 0)
        return 1;
    progress(kProgress70);
    fe::direct<0x4dcd20, load::weatherLoaded>(app, ts);
    if (stopped())
        return 1;
    progress(kProgress75);
    if (fe::direct<0x4283d0, frontend::loada::raceCarSteps>(app, ts) != 0)
        return 1;
    progress(kProgress90);
    fe::direct<kInitCameras, carstate::camerasSetUp>(guest.app(), guest.ts());
    progress(kProgress93);
    fe::direct<0x4bc830, frontend::loada::raceSteps>(app, ts);
    if (stopped())
        return 1;
    progress(kProgress95);
    if (fe::direct<0x4c62a0, frontend::inpa::raceSetUp>(app, ts) != 0)
        return 1;
    progress(kProgress100);
    loadingScreenFit(app, ts, false);
    return 0;
}

/* sub_4a3690: a whole race.  `mode` 1 starts it as finished unless told otherwise, `replay`
 * nonzero has the config read again after it.  A load stopped puts the settings back from the
 * frame.  What a callee reads left over from the code before it -- the copy's esi and edi above
 * all -- it finds as the original left them.  The answer goes to the menus. */
nfs::u32 frontend::loopa::raceSession(App* app, ThreadState& ts, u32 mode, u32 replay)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20 + 0x20f4);
    const u32 saved = guest.frame();
    const u32 savedEnd = saved + kSettingsDwords * 4;

    copyDwords(guest, saved, SETTINGS_ADDRESS(fileSize), kSettingsDwords);
    guest.setDword(kRaceAbort, 0);
    fe::direct<0x4e0dee, ea::randomSeeded>(app, ts, gameSettings(guest).raceSeed);
    guest.setDword(kRaceToggle, guest.dword(kRaceToggle) == 0 ? 1 : 0);
    if (gameSettings(guest).replayMode != 2 && guest.dword(kGhostLoaded) != 1)
        fe::direct<0x474ca0, frontend::loada::rosterBuild>(app, ts);

    const u32 stopped = u32(s32(s16(u16(guest::direct<0x4a3400, raceLoad>(app, ts)))));
    guest.setDword(kRaceAbort, stopped);
    if (stopped != 0)
    {
        fe::direct<0x4a35d0, graphics::raceTakenDown>(app, ts, 1);
        copyDwords(guest, SETTINGS_ADDRESS(fileSize), saved, kSettingsDwords);
        if (s32(gameSettings(guest).playMode) > 1 && guest.dword(kNetworkHost) != 0)
        {
            fe::direct<kCloseNetwork, ea::networkClosed>(app, ts);
            guest.setWord(kNetworkClosed, 1);
            return kSessionMenus;
        }
        if (replay != 0)
            fe::direct<0x472980, load::configLoaded>(app, ts);
        return mode != 0 ? kSessionNext : kSessionMenus;
    }

    guest.setDword(kRaceEnd, mode == 1 ? 0 : 2);
    while (guest.dword(kRaceLooping) != 0)
    {
        guest.setDword(kRaceLooping, 0);
        guest::direct<0x4a33b0, raceRestart>(app, ts);
        const u32 music = gameSettings(guest).raceMusicVolume;
        if (music != 0)
        {
            const u32 byMode = s32(gameSettings(guest).musicMode) > 2 ? 1 : 0;
            fx::callAt(guest, kRaceMusicStart, args({ gameSettings(guest).track, music, byMode }));
            guest.setByte(kMusicOn, 1);
        }
        else
        {
            guest.setByte(kMusicOn, 0);
        }
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, ts, ts.sp - 4, 0);
        fe::direct<0x4c63f0, frontend::inpa::inputReset>(app, ts);
        fe::direct<0x4c4e40, frontend::inpa::runRaceLoop>(app, ts);
    }

    if (guest.dword(kRaceEnd) != 1 && replay == 0 && s32(gameSettings(guest).playMode) > 1
        && (guest.word(kNetworkSpoken) != 0 || guest.dword(kNetworkHost) == 0))
        guest.setDword(kRaceEnd, 2);
    const u32 soundEnd = u32(*fe::direct<0x412b50, port::raceSoundEndOf>(app, ts, ts.sp - 4));
    fe::direct<0x4109d0, port::musicStopped>(app, ts, ts.sp - 4, soundEnd);
    const u32 takenDown = *fe::direct<0x4a35d0, graphics::raceTakenDown>(app, ts, guest.dword(kRaceEnd) != 1 ? u32(1) : u32(0));
    fe::direct<0x410aa0, port::musicStreamEndOf>(app, ts, ts.sp - 4, takenDown, 0, 0);
    fe::direct<0x410370, port::musicStreamOf>(app, ts, ts.sp - 4, 0x12cu, replay);
    if (replay == 0 && guest.dword(kRaceEnd) == 0)
        fe::direct<0x455410, frontend::menub::resultsBuild>(app, ts);
    if (replay != 0)
        fe::direct<0x472980, load::configLoaded>(app, ts);
    const u32 end = guest.dword(kRaceEnd);
    return end == 0 ? kSessionNext : end == 1 ? kSessionRestart : kSessionMenus;
}

// ---------------------------------------------------------------------------
// The game
// ---------------------------------------------------------------------------

/* sub_4b6020: the game started: its screen's first mode, the driver chosen and started, the
 * heap, the timer, the keys, the cars' list and the config, sound and music, the network's
 * reset and thread, the tables.  The x87 last, its word asking for what the race has.  The
 * heap's budget is the port's (the port's heap budget change). */
nfs::u32 frontend::loopa::gameInit(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 20);
    constexpr u32 kWindowName = 0x53fe08, kOutOfMemory = 0x53fe1c;
    constexpr u32 kDriverType = 0x7a3a54, kDriverBit = 0x7a3a58, kLanguage = 0x7a3a50;
    constexpr u32 kPowerMessage = 0x218, kPowerHandler = 0x4b5ff0;
    constexpr u32 kHeapBudget = 64u * 1024u * 1024u;
    constexpr u32 kControlWord = 0x79f3ec, kControlWordKept = 0x79f3f4;

    guest.setDword(0x564384, 0x280);
    guest.setDword(0x564388, 0x1e0);
    guest.setDword(0x56438c, 0x10);
    guest.setDword(0x564390, 1);
    fe::direct<0x4f91d0, frontend::menz::singleInstanceCheck>(app, ts, kWindowName);
    guest.setDword(kChosenWidth, 1);
    guest.setDword(0x55fec8, 1);
    guest.setDword(0x55fec0, 1);
    fe::direct<0x4b5720, frontend::rendd::chooseDriver>(app, ts, kDriverType);

    const u32 driver = guest.dword(kDriverType);
    guest.setDword(kDriverBit, u32(1) << ((driver & 0xff) % 32));
    // The game's code hands it 0 in edx too, which it does not take.
    fe::direct<0x4b5f10, frontend::wina::checkMemory>(app, ts, guest.dword(kLanguage));

    const u32 heap = fe::direct<0x4f92b0, frontend::wata::heapStart>(app, ts, kHeapBudget);
    guest.setDword(0x7a1a78, heap);
    if (s32(heap) < 0xfce000)
        fe::direct<0x401010, ea::error>(app, ts, kOutOfMemory, guest::Words{ heap });

    fe::direct<0x4b5860, frontend::wina::checkLibrary>(app, ts, guest.dword(kDriverType), guest.dword(kLanguage));
    fe::direct<0x4b59c0, render::driverStarted>(app, ts, guest.dword(kDriverType), guest.dword(kLanguage));
    fe::direct<0x4f9360, frontend::wina::videoClose>(app, ts);
    const u32 driverType = guest.dword(kDriverType);
    if (driverType != 1)
        fe::direct<0x4b5ea0, graphics::textureMemoryWarned>(app, ts, driverType, guest.dword(kLanguage));
    fe::direct<0x4bef50, render::screenModeSet>(app, ts, guest.dword(0x55fec8));
    guest.setDword(kChosenWidth, *fe::direct<0x4bee80, graphics::screenModeFor>(app, ts, guest.dword(0x55fecc)));
    guest::direct<0x4be5e0, loopb::viewsLayout>(app, ts, 0);
    guest::direct<0x4bed50, loopb::projectionOnly>(app, ts, 0x7cdb68, 0x5a, 0);
    guest::direct<0x4bed70, loopb::useClip>(app, ts, 0x7cdb68);

    guest.setDword(0x560094, 0x3f800000);
    guest.setDword(0x560098, 0);
    fe::direct<0x4cc6d0, frontend::wina::handlerSet>(app, ts, kPowerMessage, kPowerHandler);
    guest.setDword(0x564810, 0);
    fe::direct<0x48eff0, port::timerStarted>(app, ts, 0x80);
    guest.setDword(0x5643d8, 0x80);
    fe::direct<0x4cdc90, frontend::wina::hookOn>(app, ts, 1, 0);
    fe::direct<0x4f0da0, frontend::wata::asyncInit>(app, ts, 0x14, 0);
    fe::direct<0x4d5cc0, graphics::drawingBegun>(app, ts, 1, 1);
    fe::direct<0x434f20, controls::startInputCore>(app, ts);
    if ((guest.byte(0x55eb2e) & 0x20) != 0)
        fe::direct<0x472fc0, frontend::menuc::titleWait>(app, ts);
    fe::direct<0x43bcf0, frontend::sime::removeNumberedFolders>(app, ts);
    if (*fe::direct<0x439d90, load::loadCars>(app, ts) != 0)
        fe::direct<0x4b5e50, frontend::wina::fatalMessage>(app, ts, guest.dword(kLanguage));
    if (fe::direct<0x438bd0, load::carsListed>(app, ts) == 0)
        fe::direct<0x4b5e50, frontend::wina::fatalMessage>(app, ts, guest.dword(kLanguage));
    fe::direct<0x472980, load::configLoaded>(app, ts);
    fe::direct<0x40fb10, aiplan::bringUpSound>(app, ts, 0);
    // eax as the sound's bring-up left it; musicStreamMade gives it back only with the music off, and
    // nothing here reads that.
    const u32 started = 0;
    fe::direct<0x410330, port::musicStreamMade>(app, ts, ts.sp - 4, started, 0);
    fe::direct<0x4a6390, ea::networkReset>(app, ts, 1);
    fe::direct<0x4df7a0, ea::threadStarted>(app, ts, 0x42dc20u, 0u, 1u, 0x7a3aacu);
    fe::direct<0x496310, frontend::sime::makeRootTable>(app, ts);
    guest::direct<0x497430, fillRootBytes>(app, ts);
    fe::direct<0x4bf110, frontend::rendd::fillReciprocals>(app, ts);

    // fninit's control word, as fnstcw stores it (single precision, 0x7f); fldcw changes nothing here.
    guest.setWord(kControlWord, 0x7f);
    const u32 word = guest.dword(kControlWord);
    guest.setDword(kControlWordKept, word);
    const u32 single = word & 0xfffff0ff;
    guest.setDword(kControlWord, single);
    return single;
}

/* sub_497430: a byte for each of 128 floats, 0.5 to just under 2 by their top seven mantissa
 * bits: the reciprocal of its root, rounded to the same bits.  Entry 64, the one that is 1
 * itself, is 255 by hand.  eax is what was last worked out. */
nfs::u32 frontend::loopa::fillRootBytes(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 4);
    constexpr u32 kTable = 0x79f364;
    u32 last = 0;
    for (u32 i = 0; i < 0x80; ++i)
    {
        // The square root (native_frontend_simc.cpp's), as a double.
        const double root = fe::direct<0x4ea920, frontend::simc::squareRootOf>(app, ts, (i << 17) | 0x3f000000);
        const float reciprocal = float(fp::div(1.0, root));
        u32 bits;
        std::memcpy(&bits, &reciprocal, sizeof bits);
        last = ((bits + 0x2000) >> 15) & 0xff;
        guest.setByte(kTable + i, std::uint8_t(last));
    }
    guest.setByte(kTable + 0x40, 0xff);
    return last;
}

/* sub_4b62a0: whether the install's kind at 0x7a26b4 starts "remote", which the game keeps
 * with 13 taken from each letter: decoded in the frame, as the comparison is handed it. */
nfs::u32 frontend::loopa::installIsRemote(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24 + 8);
    const u32 word = guest.frame();
    for (u32 i = 0; i < 6; ++i)
        guest.setByte(word + i, std::uint8_t(guest.byte(0x55fdc0 + i) + 0xd));
    return fe::direct<0x4eb390, watcom::clib::strncmp>(guest.app(), guest.ts(), 0x7a26b4, word, 6) == 0 ? 1 : 0;
}

/* sub_495190: the loading bar's four corners, x and y of each, scaled by the view's size.
 * The last corner's products have their operands the other way round, as the x87 had them. */
nfs::u32 frontend::loopa::barScale(App* app, ThreadState& ts, u32 corner0, u32 corner1, u32 corner2, u32 corner3)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x24);
    const u32 ebp = guest.frame() + 0x24;
    // ebx and ecx as the call by address loads them and edx as it leaves it (sub_4beb20 its last place there):
    // the function leaves them.
    fe::direct<0x4beb20, frontend::pause::readRect>(app, ts, 0x7cdac0, ebp - 0x1c, ebp - 0x18, ebp - 0x14, ebp - 0x10);

    const Float scaleX = fp::mul(Float(s32(guest.dword(ebp - 0x14))), Float(guest.floatAt(0x53be28)));
    const Float x0 = fp::mul(Float(guest.floatAt(corner0)), scaleX);
    const Float scaleY = fp::mul(Float(s32(guest.dword(ebp - 0x10))), Float(guest.floatAt(0x53be2c)));
    const Float y0 = fp::mul(Float(guest.floatAt(corner0 + 4)), scaleY);
    guest.setFloat(corner0, float(x0));
    guest.setFloat(corner0 + 4, float(y0));

    const Float x1 = fp::mul(Float(guest.floatAt(corner1)), scaleX);
    const Float y1 = fp::mul(Float(guest.floatAt(corner1 + 4)), scaleY);
    guest.setFloat(corner1, float(x1));
    guest.setFloat(corner1 + 4, float(y1));

    const Float x2 = fp::mul(Float(guest.floatAt(corner2)), scaleX);
    const Float y2 = fp::mul(Float(guest.floatAt(corner2 + 4)), scaleY);
    guest.setFloat(corner2, float(x2));
    guest.setFloat(corner2 + 4, float(y2));

    const Float x3 = fp::mul(scaleX, Float(guest.floatAt(corner3)));
    const Float y3 = fp::mul(scaleY, Float(guest.floatAt(corner3 + 4)));
    guest.setFloat(corner3, float(x3));
    guest.setFloat(corner3 + 4, float(y3));
    return corner2;
}

/* sub_43bcb0: always 1, what the loading screen asks. */
nfs::u32 frontend::loopa::loadingScreenFlag(App*, ThreadState&)
{
    return 1;
}

/* sub_459410: the frame of a menu's item (eax), in the colour edx: its place and size. */
nfs::u32 frontend::loopa::itemFrame(App* app, ThreadState& ts, u32 item, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const u32 height = guest.dword(item + 0x64), width = guest.dword(item + 0x60);
    const u32 y = u32(upperWord(guest, item + 6)), x = u32(upperWord(guest, item + 4));
    return *fe::direct<0x459350, graphics::flatRectangle>(app, ts, x, y, width, height, colour);
}

/* sub_475b10: the 3D font at 0x679010 let go. */
nfs::u32 frontend::loopa::freeLoadedBlock(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    constexpr u32 kBlock = 0x679010;
    const u32 block = guest.dword(kBlock);
    if (block == 0)
        return 0;
    const u32 result = *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, block);
    guest.setDword(kBlock, 0);
    return result;
}

/* sub_4a3390: the game's start, and sub_4bc980 handed what it returned. */
nfs::u32 frontend::loopa::gameStart(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    guest::direct<0x4b6020, gameInit>(app, ts);
    return guest::direct<0x4bc980, loopb::drawStart>(app, ts);
}

/* sub_4a33a0: its end. */
nfs::u32 frontend::loopa::gameEnd(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    guest::direct<0x4bc990, loopb::drawEnd>(app, ts);
    return *fe::direct<0x4b6250, graphics::gameEnded>(app, ts);
}

/* sub_4a33b0: what a race's loop does each time it goes round. */
nfs::u32 frontend::loopa::raceRestart(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 4);
    guest::direct<0x4bcb90, loopb::replayStart>(app, ts);
    fe::direct<0x4c4740, frontend::simd::raceRestart>(app, ts);
    fe::direct<0x41de80, track::startTrackViews>(guest.app(), guest.ts());
    fe::direct<kInitCameras, carstate::camerasSetUp>(guest.app(), guest.ts());
    return u32(*fe::direct<0x411680, port::sourcesResetOf>(app, ts));
}

/* sub_4a6f70: nothing, called as the game ends. */
nfs::u32 frontend::loopa::nothingToEnd(App*, ThreadState&)
{
    return 0;
}

/* The thunks, under the game's names. */

nfs::u32 frontend::loopLoadingScreenFlag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43bcb0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::loadingScreenFlag(app, ts); });
}

nfs::u32 frontend::loopItemFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_459410", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const u32 item = args.eax, colour = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::itemFrame(app, ts, item, colour); });
}

nfs::u32 frontend::loopFreeLoadedBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_475b10", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::freeLoadedBlock(app, ts); });
}

nfs::u32 frontend::ghostSaveFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4777d0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const u32 name = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::saveGhost(app, ts, name); });
}

nfs::u32 frontend::ghostPoseSetRotation(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_477830", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 pose = args.eax, matrix = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::poseSetRotation(app, ts, pose, matrix); });
}

nfs::u32 frontend::ghostPosePosition(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_477850", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const u32 pose = args.eax, out = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::posePosition(app, ts, pose, out); });
}

nfs::u32 frontend::ghostPoseSetPosition(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4778c0", 0x14, 0, nullptr, 0 };
    const u32 pose = args.eax;
    const u32 x = stackArgument(app, ts, 0), y = stackArgument(app, ts, 1), z = stackArgument(app, ts, 2);
    const u32 slice = stackArgument(app, ts, 3), car = stackArgument(app, ts, 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::poseSetPosition(app, ts, pose, x, y, z, slice, car); });
}

nfs::u32 frontend::ghostPoseBetween(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_477990", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 from = args.eax, to = args.edx, fraction = args.ebx, positionOut = args.ecx;
    const u32 matrixOut = stackArgument(app, ts, 0);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return loopa::poseBetween(app, ts, from, to, fraction, positionOut, matrixOut);
    });
}

nfs::u32 frontend::ghostRecordedAt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4779e0", 0, 0, nullptr, 0 };
    const u32 index = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::recordedAt(app, ts, index); });
}

nfs::u32 frontend::ghostLoadedAt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_477a10", 0, 0, nullptr, 0 };
    const u32 index = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::loadedAt(app, ts, index); });
}

nfs::u32 frontend::ghostRecordingStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_477a50", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::recordingStart(app, ts); });
}

nfs::u32 frontend::ghostLoadFile(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_477b50", 0, 0, nullptr, 0 };
    const u32 name = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::loadGhost(app, ts, name); });
}

nfs::u32 frontend::ghostRecordCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_477c20", 0, 0, nullptr, 0 };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::recordCar(app, ts, car); });
}

nfs::u32 frontend::ghostPlaybackPose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_477d90", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 positionOut = args.eax, matrixOut = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::playbackPose(app, ts, positionOut, matrixOut); });
}

nfs::u32 frontend::loopBarScale(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_495190", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 corner0 = args.eax, corner1 = args.edx, corner2 = args.ebx, corner3 = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::barScale(app, ts, corner0, corner1, corner2, corner3); });
}

nfs::u32 frontend::loopFillRootBytes(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_497430", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::fillRootBytes(app, ts); });
}

nfs::u32 frontend::loopGameStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a3390", 0, 0, nullptr, 0, kSavesNone };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::gameStart(app, ts); });
}

nfs::u32 frontend::loopGameEnd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a33a0", 0, 0, nullptr, 0, kSavesNone };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::gameEnd(app, ts); });
}

nfs::u32 frontend::loopRaceRestart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a33b0", 0, 0, nullptr, 0, kSavesNone };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::raceRestart(app, ts); });
}

nfs::u32 frontend::loopRaceLoad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a3400", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::raceLoad(app, ts); });
}

nfs::u32 frontend::loopRaceSession(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a3690", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 mode = args.eax, replay = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::raceSession(app, ts, mode, replay); });
}

nfs::u32 frontend::loopNothingToEnd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a6f70", 0, 0, nullptr, 0, kSavesNone };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::nothingToEnd(app, ts); });
}

nfs::u32 frontend::loopPanelQuad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a7080", 0x14, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 x0 = args.eax, y0 = args.edx, x1 = args.ebx, y1 = args.ecx;
    u32 s[5];
    for (u32 i = 0; i < 5; ++i)
        s[i] = stackArgument(app, ts, i);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return loopa::panelQuad(app, ts, x0, y0, x1, y1, s[0], s[1], s[2], s[3], s[4]);
    });
}

nfs::u32 frontend::loopPanelTexturedQuad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a7310", 0x38, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 x0 = args.eax, y0 = args.edx, x1 = args.ebx, y1 = args.ecx;
    u32 s[14];
    for (u32 i = 0; i < 14; ++i)
        s[i] = stackArgument(app, ts, i);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return loopa::panelTexturedQuad(app, ts, x0, y0, x1, y1, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8],
                                        s[9], s[10], s[11], s[12], s[13]);
    });
}

nfs::u32 frontend::loopPanelDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4a75e0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::panelDraw(app, ts); });
}

nfs::u32 frontend::loopGameInit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b6020", 0, 0, kInitStrings, std::size(kInitStrings), fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::gameInit(app, ts); });
}

nfs::u32 frontend::loopInstallIsRemote(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b62a0", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::installIsRemote(app, ts); });
}

nfs::u32 frontend::poseBlend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b6a30", 0xc, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 rotationFrom = args.eax, rotationTo = args.edx, positionFrom = args.ebx, positionTo = args.ecx;
    const u32 fraction = stackArgument(app, ts, 0), rotationOut = stackArgument(app, ts, 1);
    const u32 positionOut = stackArgument(app, ts, 2);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return loopa::blend(app, ts, rotationFrom, rotationTo, positionFrom, positionTo, fraction, rotationOut, positionOut);
    });
}

nfs::u32 frontend::poseMatrixOfRotation(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b6ca0", 0, 0, nullptr, 0 };
    const u32 rotation = args.eax, out = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::matrixOfRotation(app, ts, rotation, out); });
}

nfs::u32 frontend::poseRotationNearest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b6d90", 0, 0, nullptr, 0 };
    const u32 rotation = args.eax, other = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::rotationNearest(app, ts, rotation, other); });
}

nfs::u32 frontend::poseRotationOfMatrix(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b6e80", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 matrix = args.eax, out = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return loopa::rotationOfMatrix(app, ts, matrix, out); });
}

}
