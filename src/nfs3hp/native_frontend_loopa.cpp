#include "native_frontend_loopa.h"
#include "game_settings.h"
#include "native_controls_input.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_frontend.h"
#include "native_frontend_eaa.h"
#include "native_frontend_eab.h"
#include "native_frontend_inpa.h"
#include "native_frontend_loada.h"
#include "native_frontend_loopb.h"
#include "native_frontend_menua.h"
#include "native_frontend_menub.h"
#include "native_frontend_menuc.h"
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
#include "native_load_cardb.h"
#include "port_snd_stream_calls.h"
#include "port_sound_music.h"
#include <nfs3hp.h>
#include <fpu.h>
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
 * runs at the stack addresses it did), and an adapter under the game's name for the calls by
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

/* The port's own, by the patches of tools/apply_race_state.py and
 * tools/apply_loading_screen.py: the mark of a race, the end of the 4:3 loading screen
 * (nfs3hp_main.cpp). */
void raceRunning(bool running);
void loadingScreenFit(win32::WinApplication* app, x86::CPU& cpu, bool on);

namespace
{

using fe::args;
using fe::Guest;
using fe::GuestString;
using x86::Float;
using x86::reg16;
using x86::reg32;
using x86::reg64;
using x86::sreg16;
using x86::sreg32;
using x86::sreg64;

// ---------------------------------------------------------------------------
// The game's data
// ---------------------------------------------------------------------------

// The race.
constexpr reg32 kRaceAbort = 0x55e88c;         // nonzero: the load was stopped
constexpr reg32 kRaceToggle = 0x552854;        // flips with each session
constexpr reg32 kRaceEnd = 0x7d36b4;           // how it ended: 0 finished, 1 restart, else left
constexpr reg32 kRaceLooping = 0x7d3698;       // nonzero: the loop goes round
constexpr reg32 kMusicOn = 0x55e890;           // byte
constexpr reg32 kNetworkSpoken = 0x7a22dc;     // word
constexpr reg32 kNetworkHost = 0x7a22e0;
constexpr reg32 kNetworkClosed = 0x7a22de;     // word
constexpr reg32 kSettingsDwords = 0x83c;
constexpr reg32 kChosenWidth = 0x55fec4;

// What a session returns to the menus.
constexpr reg32 kSessionMenus = 0xb, kSessionRestart = 0xa, kSessionNext = 0xc;

// A session's calls.
constexpr reg32 kTakeDownRace = 0x4a35d0, kCloseNetwork = 0x4a65e0;
constexpr reg32 kLoadConfig = 0x472980, kRaceMusicStart = 0x4107c0;
constexpr reg32 kRaceSoundEnd = 0x412b50, kMusicStreamMakeOf = 0x410370;

// A load's calls.
constexpr reg32 kScreenMode = 0x4bef50, kViewport = 0x4beb80;
constexpr reg32 kLoadingText = 0x484040, kScreenScales = 0x484b70, kLoadingScreen = 0x494fa0;
constexpr reg32 kFlushKeys = 0x4f0240;
constexpr reg32 kLoadWeather = 0x4dcd20, kInitCameras = 0x422cf0;

// The loading bar's steps, floats' bits, on its way to 100.
constexpr reg32 kProgress5 = 0x40a00000, kProgress10 = 0x41200000, kProgress35 = 0x420c0000;
constexpr reg32 kProgress40 = 0x42200000, kProgress70 = 0x428c0000, kProgress75 = 0x42960000;
constexpr reg32 kProgress90 = 0x42b40000, kProgress93 = 0x42ba0000, kProgress95 = 0x42be0000;
constexpr reg32 kProgress100 = 0x42c80000;

// The road's slices.
constexpr reg32 kSlices = 0x5dd948;
constexpr reg32 kSliceBytes = 36;

// The clock.
constexpr reg32 kTick = 0x7d3684;

// The ghost: what is being recorded, settings first and its poses after, and what was loaded.
constexpr reg32 kRecording = 0x679050;
constexpr reg32 kRecordedTicks = 0x67b140;
constexpr reg32 kRecordedPoses = 0x67b144;
constexpr reg32 kLoaded = 0x6ba5c4;
constexpr reg32 kLoadedTicks = 0x6bc6b4;
constexpr reg32 kLoadedPoses = 0x6bc6b8;
constexpr reg32 kGhostBytes = 0x41574;
constexpr reg32 kGhostHeaderBytes = 0x20f4;    // the settings and the ticks' count
constexpr sreg32 kGhostMostTicks = 0x3840;
constexpr reg32 kPoseBytes = 18;
constexpr reg32 kPoseRotation = 0xa;
constexpr reg32 kGhostLoaded = 0x55d000;       // 1: a ghost's poses are there to read
constexpr reg32 kKeptDwords = 0x5db;           // the settings a loaded ghost's leave as they were

constexpr reg32 kFillZero = 0x4e070c, kSaveFile = 0x4eeae0;

// A car's pose.
constexpr reg32 kCarWord = 6, kCarSlice = 0x1a;  // dwords whose upper half is the word
constexpr reg32 kCarPosition = 0x98, kCarMatrix = 0xc0;

// ---------------------------------------------------------------------------
// The arithmetic these share
// ---------------------------------------------------------------------------

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, reg32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

void copyDwords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* The upper half of the dword at `address`, a signed word as `mov; sar 16` makes it. */
sreg32 upperWord(const Guest& guest, reg32 address)
{
    return sreg32(guest.dword(address)) >> 16;
}

/* The game's fixed-point product, 16.16, inline at each of its uses: `imul; shl edx, 16;
 * shr eax, 16; adc eax, edx`.  The bit shifted out rounds it; the sum wraps. */
reg32 fixedProduct(sreg32 a, sreg32 b, reg32& shiftedHigh)
{
    const reg64 product = reg64(sreg64(a) * sreg64(b));
    shiftedHigh = reg32(product >> 32) << 16;
    const reg32 low = reg32(product);
    return (low >> 16) + shiftedHigh + ((low >> 15) & 1);
}

reg32 fixedProduct(sreg32 a, sreg32 b)
{
    reg32 shiftedHigh;
    return fixedProduct(a, b, shiftedHigh);
}

/* The game's square root of a 16.16 number, inline at each of its uses: a straight line
 * for each power of two, from the tables at 0x5661c4 and 0x566248; 0 of 0. */
reg32 fixedRoot(const Guest& guest, reg32 value)
{
    if (value == 0)
        return 0;
    const reg32 power = reg32(31 - __builtin_clz(value));
    const reg64 product = reg64(value) * reg64(guest.dword(0x5661c4 + power * 4));
    return ((reg32(product) >> 16) | (reg32(product >> 32) << 16)) + guest.dword(0x566248 + power * 4);
}

// ---------------------------------------------------------------------------
// What the functions below share
// ---------------------------------------------------------------------------

/* A float of its bits, as fld dword reads a stack argument. */
float floatOf(reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* The tick's place in a recording: one pose a tick in eight, rounded towards 0 as Watcom's
 * `sar; shl; sbb; sar` does it. */
reg32 poseIndex(reg32 tick)
{
    return reg32(sreg32(tick) / 8);
}

/* What the clipper gets for a vertex, the original's inline test four times over: a float's
 * bits as integers, so a negative one, its top bit set, is beyond the low side at once. */
constexpr reg32 kClipFar = 0x7d34f8, kClipLeft = 0x7d34fc, kClipRight = 0x7d350c, kClipTop = 0x7d3504, kClipBottom = 0x7d3500;

std::uint8_t clipCode(const Guest& guest, reg32 vertex)
{
    const reg32 depth = guest.dword(vertex + 0xc);
    if ((depth & 0x80000000) != 0 || sreg32(depth) >= sreg32(guest.dword(kClipFar)))
        return 0x10;
    std::uint8_t code = 0;
    const reg32 y = guest.dword(vertex + 4);
    if ((y & 0x80000000) != 0 || sreg32(y) < sreg32(guest.dword(kClipTop)))
        code |= 8;
    else if (sreg32(y) > sreg32(guest.dword(kClipBottom)))
        code |= 4;
    const reg32 x = guest.dword(vertex);
    if ((x & 0x80000000) != 0 || sreg32(x) < sreg32(guest.dword(kClipLeft)))
        code |= 1;
    else if (sreg32(x) > sreg32(guest.dword(kClipRight)))
        code |= 2;
    return code;
}

/* Four vertices of 32 bytes in the frame. */
constexpr reg32 kVertexBytes = 0x20, kVertexColour = 0x10, kVertexCode = 0x14, kVertexU = 0x18, kVertexV = 0x1c;

void setPlace(Guest& guest, reg32 vertex, reg32 x, reg32 y)
{
    guest.setFloat(vertex, float(Float(sreg32(x))));
    guest.setFloat(vertex + 4, float(Float(sreg32(y))));
}

reg32 clipAndDraw(Guest& guest, reg32 frame)
{
    for (reg32 i = 0; i < 4; ++i)
        guest.setByte(frame + i * kVertexBytes + kVertexCode, clipCode(guest, frame + i * kVertexBytes));
    return guest.call(0x4c12e0, args({ frame, frame + kVertexBytes, frame + 2 * kVertexBytes, frame + 3 * kVertexBytes }));
}

constexpr GuestString kInitStrings[] = {
    { 0x53fe08, "Need For Speed III" },
    { 0x53fe1c, "Out of memory! %d" },
};

/* A generated function's stack arguments, before its frame. */
reg32 stackArguments(const x86::CPU& cpu)
{
    return cpu.esp + 4;
}

reg32 stackArgument(win32::WinApplication* app, const x86::CPU& cpu, reg32 place)
{
    return app->getMemory<reg32>(stackArguments(cpu) + 4 * place);
}

constexpr std::uint8_t kSavesNone = 0;

}  // namespace

// ---------------------------------------------------------------------------
// A pose
// ---------------------------------------------------------------------------

/* sub_4b6a30: the rotation and the position `fraction` (16.16) of the way from the first
 * to the second, each part on its own: the rotation is not made unit again. */
x86::reg32 frontend::loopa::blend(App* app, CPU& cpu, reg32 rotationFrom, reg32 rotationTo, reg32 positionFrom,
                                  reg32 positionTo, reg32 fraction, reg32 rotationOut, reg32 positionOut)
{
    Guest guest(app, cpu);
    const sreg32 to = sreg32(fraction);
    const sreg32 from = sreg32(0x10000 - fraction);
    const auto part = [&](sreg32 a, sreg32 b) { return fixedProduct(a, from) + fixedProduct(b, to); };

    guest.setWord(rotationOut, reg16(part(sreg16(guest.word(rotationFrom)), sreg16(guest.word(rotationTo)))));
    guest.setWord(rotationOut + 2, reg16(part(upperWord(guest, rotationFrom), upperWord(guest, rotationTo))));
    guest.setWord(rotationOut + 4, reg16(part(upperWord(guest, rotationFrom + 2), upperWord(guest, rotationTo + 2))));
    guest.setWord(rotationOut + 6, reg16(part(upperWord(guest, rotationFrom + 4), upperWord(guest, rotationTo + 4))));

    guest.setDword(positionOut, part(sreg32(guest.dword(positionFrom)), sreg32(guest.dword(positionTo))));
    guest.setDword(positionOut + 4, part(sreg32(guest.dword(positionFrom + 4)), sreg32(guest.dword(positionTo + 4))));
    // The last, for what it leaves: ecx the sum, edx the product's high half shifted, ebx the fraction.
    const reg32 first = fixedProduct(sreg32(guest.dword(positionFrom + 8)), from);
    reg32 shiftedHigh;
    const reg32 sum = first + fixedProduct(sreg32(guest.dword(positionTo + 8)), to, shiftedHigh);
    guest.setDword(positionOut + 8, sum);
    guest.leaveEbx(fraction);
    guest.leaveEcx(sum);
    guest.leaveEdx(shiftedHigh);
    return positionOut;
}

/* sub_4b6ca0: the matrix of a rotation of four words, its nine parts 16.16. */
x86::reg32 frontend::loopa::matrixOfRotation(App* app, CPU& cpu, reg32 rotation, reg32 out)
{
    Guest guest(app, cpu);
    constexpr reg32 kOne = 0x10000000;
    const reg32 x = reg32(sreg32(sreg16(guest.word(rotation))));
    const reg32 y = reg32(upperWord(guest, rotation));
    const reg32 z = reg32(upperWord(guest, rotation + 2));
    const reg32 w = reg32(upperWord(guest, rotation + 4));

    const reg32 x2 = x + x, y2 = y + y, z2 = z + z;
    const reg32 xw = x2 * w, yw = w * y2, zw = w * z2;
    const reg32 xx = x2 * x, xy = x * y2, xz = x * z2;
    const reg32 yy = y2 * y, yz = y * z2, zz = z2 * z;
    const reg32 oneLessXX = kOne - xx;
    const auto scaled = [](reg32 value) { return reg32(sreg32(value) >> 12); };

    guest.setDword(out, scaled(kOne - yy - zz));
    guest.setDword(out + 4, scaled(zw + xy));
    guest.setDword(out + 8, scaled(xz - yw));
    guest.setDword(out + 0xc, scaled(xy - zw));
    guest.setDword(out + 0x10, scaled(oneLessXX - zz));
    guest.setDword(out + 0x14, scaled(xw + yz));
    guest.setDword(out + 0x18, scaled(xz + yw));
    guest.setDword(out + 0x1c, scaled(yz - xw));
    const reg32 last = scaled(oneLessXX - yy);
    guest.setDword(out + 0x20, last);
    return last;
}

/* sub_4b6d90: the second rotation negated when that brings it nearer the first, the same
 * rotation the other way round: what makes the blend between two poses go the short way.
 * The squares of each sum and each difference, wrapping as the original's do. */
x86::reg32 frontend::loopa::rotationNearest(App* app, CPU& cpu, reg32 rotation, reg32 other)
{
    Guest guest(app, cpu);
    const sreg32 a0 = sreg16(guest.word(rotation)), b0 = sreg16(guest.word(other));
    const sreg32 a1 = upperWord(guest, rotation), b1 = upperWord(guest, other);
    const sreg32 a2 = upperWord(guest, rotation + 2), b2 = upperWord(guest, other + 2);
    const sreg32 a3 = upperWord(guest, rotation + 4), b3 = upperWord(guest, other + 4);
    const auto square = [](sreg32 value) { return reg32(value) * reg32(value); };

    const sreg32 sums = sreg32(square(a0 + b0) + square(a1 + b1) + square(a2 + b2) + square(a3 + b3)) >> 14;
    const sreg32 differences = sreg32(square(a0 - b0) + square(a1 - b1) + square(a2 - b2) + square(a3 - b3)) >> 14;
    if (sums < differences)
    {
        const reg16 first = guest.word(other), second = guest.word(other + 2);
        const reg16 third = guest.word(other + 4), fourth = guest.word(other + 6);
        guest.setWord(other + 2, reg16(0 - second));
        guest.setWord(other + 4, reg16(0 - third));
        guest.setWord(other + 6, reg16(0 - fourth));
        guest.setWord(other, reg16(0 - first));
    }
    return reg32(sums);
}

/* sub_4b6e80: the rotation of a matrix, the usual way: by the trace when that is positive,
 * else by the largest of the diagonal.  Its four parts are worked in the frame and stored
 * a quarter of that; eax the last of them, edx still where. */
x86::reg32 frontend::loopa::rotationOfMatrix(App* app, CPU& cpu, reg32 matrix, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x3c);
    constexpr reg32 kNextAxis[3] = { 1, 2, 0 };  // the original's copy of the table at 0x4b6a10
    const reg32 part = guest.frame();            // [ebp - 0x3c]: x, y, z, w
    const auto at = [&](reg32 row, reg32 column) { return guest.dword(matrix + row * 0xc + column * 4); };
    const auto difference = [](reg32 a, reg32 b) { return sreg32(a - b); };

    const reg32 trace = at(0, 0) + at(1, 1) + at(2, 2);
    if (sreg32(trace) > 0)
    {
        const reg32 root = fixedRoot(guest, trace + 0x10000);
        guest.setDword(part + 12, reg32(sreg32(root) >> 1));
        const sreg32 inverse = sreg32(0x80000000u / root);
        guest.setDword(part, fixedProduct(difference(at(1, 2), at(2, 1)), inverse));
        guest.setDword(part + 4, fixedProduct(difference(at(2, 0), at(0, 2)), inverse));
        guest.setDword(part + 8, fixedProduct(difference(at(0, 1), at(1, 0)), inverse));
    }
    else
    {
        reg32 i = sreg32(at(1, 1)) > sreg32(at(0, 0)) ? 1 : 0;
        if (sreg32(at(i, i)) < sreg32(at(2, 2)))
            i = 2;
        const reg32 j = kNextAxis[i];
        const reg32 k = kNextAxis[j];

        const reg32 root = fixedRoot(guest, at(i, i) - (at(j, j) + at(k, k)) + 0x10000);
        guest.setDword(part + i * 4, reg32(sreg32(root) >> 1));
        const sreg32 inverse = root != 0 ? sreg32(0x80000000u / root) : 0;
        guest.setDword(part + 12, fixedProduct(difference(at(j, k), at(k, j)), inverse));
        guest.setDword(part + j * 4, fixedProduct(sreg32(at(i, j) + at(j, i)), inverse));
        guest.setDword(part + k * 4, fixedProduct(sreg32(at(i, k) + at(k, i)), inverse));
    }

    guest.setWord(out, reg16(sreg32(guest.dword(part)) >> 2));
    guest.setWord(out + 2, reg16(sreg32(guest.dword(part + 4)) >> 2));
    guest.setWord(out + 4, reg16(sreg32(guest.dword(part + 8)) >> 2));
    const reg32 last = reg32(sreg32(guest.dword(part + 12)) >> 2);
    guest.setWord(out + 6, reg16(last));
    return last;
}

/* sub_477830: a pose's rotation from a matrix. */
x86::reg32 frontend::loopa::poseSetRotation(App* app, CPU& cpu, reg32 pose, reg32 matrix)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    // edx as the call by address loads it, which sub_4b6e80 keeps: the function leaves it.
    guest.leaveEdx(pose + kPoseRotation);
    return guest::direct<0x4b6e80, rotationOfMatrix>(app, cpu, matrix, pose + kPoseRotation);
}

/* sub_4778c0: a position stored in a pose: the slice's word and the car's, then the offset
 * from the slice in three words.  The x87's own order: each slice's coordinate is stored as
 * a float and fetched back before it is taken off, and the last two offsets are stored and
 * fetched back before they are scaled, where the first goes on at the x87's width. */
x86::reg32 frontend::loopa::poseSetPosition(App* app, CPU& cpu, reg32 pose, reg32 x, reg32 y, reg32 z, reg32 slice,
                                            reg32 car)
{
    Guest guest(app, cpu);
    const reg32 sliceWord = slice;
    const reg32 carWord = car;
    const reg32 sliceAt = guest.dword(kSlices) + reg32(sreg32(sreg16(reg16(sliceWord)))) * kSliceBytes;
    const Float toUnits = doubleAt(guest, 0x53b0b8);
    const Float toOffset = doubleAt(guest, 0x53b0c0);

    const Float sliceX = fp::mul(Float(sreg32(guest.dword(sliceAt))), toUnits);
    const Float sliceY = fp::mul(Float(sreg32(guest.dword(sliceAt + 4))), toUnits);
    const Float atX = Float(floatOf(x));
    const Float sliceZInteger = Float(sreg32(guest.dword(sliceAt + 8)));
    const float storedX = float(sliceX);
    const Float offsetX = fp::sub(atX, Float(storedX));
    const Float sliceZ = fp::mul(toUnits, sliceZInteger);
    const Float scaledX = fp::mul(offsetX, toOffset);
    const Float atY = Float(floatOf(y));
    const Float atZ = Float(floatOf(z));
    guest.setWord(pose + 2, reg16(sliceWord));
    const float storedY = float(sliceY);
    const float storedZ = float(sliceZ);
    guest.setWord(pose, reg16(carWord));
    const Float offsetY = fp::sub(atY, Float(storedY));
    const Float offsetZ = fp::sub(atZ, Float(storedZ));
    const float keptY = float(offsetY);
    const float keptZ = float(offsetZ);

    const auto stored = [](const Float& value) { return sreg32(fp::toInteger<sreg32>(value)) >> 8; };
    guest.setWord(pose + 4, reg16(stored(scaledX)));
    guest.setWord(pose + 6, reg16(stored(fp::mul(Float(keptY), toOffset))));
    const sreg32 last = stored(fp::mul(Float(keptZ), toOffset));
    guest.setWord(pose + 8, reg16(last));
    return reg32(last);
}

/* sub_477850: a pose's position again: its slice's point and its offset.  `out` is esi, and
 * esi is left past the copy in the frame, as the movsd left it. */
x86::reg32 frontend::loopa::posePosition(App* app, CPU& cpu, reg32 pose, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0x18);
    const reg32 slice = guest.dword(kSlices) + reg32(upperWord(guest, pose)) * kSliceBytes;
    const reg32 x = guest.dword(slice) + (reg32(upperWord(guest, pose + 2)) << 8);
    const reg32 y = guest.dword(slice + 4) + (reg32(upperWord(guest, pose + 4)) << 8);
    const reg32 z = guest.dword(slice + 8) + (reg32(upperWord(guest, pose + 6)) << 8);
    guest.setDword(out, x);
    guest.setDword(out + 4, y);
    guest.setDword(out + 8, z);
    guest.setEsi(guest.frame() + 12);
    return out;
}

/* sub_477990: the position and matrix `fraction` of the way between two poses. */
x86::reg32 frontend::loopa::poseBetween(App* app, CPU& cpu, reg32 from, reg32 to, reg32 fraction, reg32 positionOut,
                                        reg32 matrixOut)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12 + 0x20);
    const reg32 ebp = guest.frame() + 0x20;
    const reg32 positionFrom = ebp - 0x14, positionTo = ebp - 0x20, rotation = ebp - 8;
    guest::direct<0x477850, posePosition>(app, cpu, from, positionFrom);
    guest::direct<0x477850, posePosition>(app, cpu, to, positionTo);
    // What sub_4b6a30 leaves in ebx and ecx its only caller (sub_477d90, as C++) does not read.
    guest::direct<0x4b6a30, blend>(app, cpu, from + kPoseRotation, to + kPoseRotation, positionFrom, positionTo,
                                   fraction, rotation, positionOut);
    // edx as the call by address loads it, which sub_4b6ca0 keeps: the function leaves it.
    guest.leaveEdx(matrixOut);
    return guest::direct<0x4b6ca0, matrixOfRotation>(app, cpu, rotation, matrixOut);
}

// ---------------------------------------------------------------------------
// The ghost
// ---------------------------------------------------------------------------

/* sub_4779e0: the recording's pose of an index, or none past its end. */
x86::reg32 frontend::loopa::recordedAt(App*, CPU&, reg32 index)
{
    return sreg32(index) >= kGhostMostTicks ? reg32(0) : kRecordedPoses + index * kPoseBytes;
}

/* sub_477a10: the loaded ghost's pose of an index, past its end the last but one. */
x86::reg32 frontend::loopa::loadedAt(App* app, CPU& cpu, reg32 index)
{
    Guest guest(app, cpu);
    const reg32 ticks = guest.dword(kLoadedTicks);
    const reg32 place = sreg32(index) < sreg32(ticks - 1) ? index : ticks - 2;
    return kLoadedPoses + place * kPoseBytes;
}

/* sub_477a50: a recording begun, with the settings of the race, unless this kind of game
 * records none. */
x86::reg32 frontend::loopa::recordingStart(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    if (gameSettings(guest).replayMode != 0)
        return guest.eax();
    const reg32 result = guest.call(kFillZero, args({ kRecording, kGhostBytes }));
    copyDwords(guest, kRecording, SETTINGS_ADDRESS(fileSize), kSettingsDwords);
    return result;
}

/* sub_477c20: a car's pose recorded at this tick, when the recording goes on and has room,
 * and its rotation made near the last one's.  What is handed back is what is left in eax. */
x86::reg32 frontend::loopa::recordCar(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x30);
    const reg32 matrix = guest.frame();  // [ebp - 0x30]: 16.16
    const reg32 index = poseIndex(guest.dword(kTick));
    if (gameSettings(guest).replayMode != 0)
        return index;
    const reg32 pose = guest::direct<0x4779e0, recordedAt>(app, cpu, index);
    if (pose == 0)
        return 0;

    guest::direct<0x4778c0, poseSetPosition>(
        app, cpu, pose, guest.dword(car + kCarPosition), guest.dword(car + kCarPosition + 4),
        guest.dword(car + kCarPosition + 8), reg32(upperWord(guest, car + kCarSlice)), reg32(upperWord(guest, car + kCarWord)));
    const Float toFixed = doubleAt(guest, 0x53b0d8);
    for (reg32 i = 0; i < 9; ++i)
    {
        const Float scaled = fp::mul(Float(guest.floatAt(car + kCarMatrix + i * 4)), toFixed);
        guest.setDword(matrix + i * 4, reg32(fp::toInteger<sreg32>(scaled)));
    }
    guest::direct<0x477830, poseSetRotation>(app, cpu, pose, matrix);
    const reg32 before = guest::direct<0x4779e0, recordedAt>(app, cpu, index - 1);
    // What the calls leave in eax: the last one's.
    reg32 left = before;
    if (before != 0)
        left = guest::direct<0x4b6d90, rotationNearest>(app, cpu, before + kPoseRotation, pose + kPoseRotation);
    guest.setDword(kRecordedTicks, index);
    return left;
}

/* sub_477d90: the loaded ghost's pose at this tick, between the two recorded round it: 1, or
 * 0 when no ghost is loaded, edx then left the tick's index. */
x86::reg32 frontend::loopa::playbackPose(App* app, CPU& cpu, reg32 positionOut, reg32 matrixOut)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    const reg32 index = poseIndex(guest.dword(kTick));
    if (guest.dword(kGhostLoaded) != 1)
    {
        guest.leaveEdx(index);
        return 0;
    }
    const reg32 from = guest::direct<0x477a10, loadedAt>(app, cpu, index);
    const reg32 to = guest::direct<0x477a10, loadedAt>(app, cpu, index + 1);
    const reg32 fraction = (guest.dword(kTick) & 7) << 13;
    // edx as the call by address leaves it (sub_477990 loads the matrix's place there last): the function leaves it.
    guest.leaveEdx(matrixOut);
    guest::direct<0x477990, poseBetween>(app, cpu, from, to, fraction, positionOut, matrixOut);
    return 1;
}

/* sub_4777d0: the recording written to a file, only by one player alone in the kinds of
 * race that keep one. */
x86::reg32 frontend::loopa::saveGhost(App* app, CPU& cpu, reg32 name)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    if (name == 0)
        return 0;
    const reg32 ticks = guest.dword(kRecordedTicks);
    if (gameSettings(guest).playMode != 0)
        return 0;
    const reg32 kind = gameSettings(guest).raceType;
    if (kind == 1 || kind == 2)
        return 0;
    guest.call(kSaveFile, args({ name, kRecording, ticks * kPoseBytes + kGhostHeaderBytes }));
    return 0;
}

/* sub_477b50: a ghost's file read into the place for it, its settings taken but for those
 * the frame kept.  0 when it is not there, is too long or is not valid. */
x86::reg32 frontend::loopa::loadGhost(App* app, CPU& cpu, reg32 name)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x176c);
    const reg32 kept = guest.frame();
    copyDwords(guest, kept, SETTINGS_ADDRESS(language), kKeptDwords);
    if (!*fe::direct<0x4e0e60, ea::fileThere>(app, cpu, name))
        return 0;
    if (fe::direct<0x4e43a0, frontend::eaa::fileSize>(app, cpu, name) > kGhostBytes)
        return 0;
    guest.call(kFillZero, args({ kLoaded, kGhostBytes }));
    fe::direct<0x4f22e0, frontend::eab::fileLoadInto>(app, cpu, name, kLoaded);
    if (fe::direct<0x478100, frontend::loada::savedResolveEntries>(app, cpu, kLoaded) == 0)
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
x86::reg32 frontend::loopa::panelQuad(App* app, CPU& cpu, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 x2, reg32 y2,
                                      reg32 x3, reg32 y3, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12 + 0x90);
    const reg32 quad = guest.frame();
    copyDwords(guest, quad, 0x4a6f80, 4 * kVertexBytes / 4);
    setPlace(guest, quad, x0, y0);
    setPlace(guest, quad + kVertexBytes, x1, y1);
    setPlace(guest, quad + 2 * kVertexBytes, x2, y2);
    setPlace(guest, quad + 3 * kVertexBytes, x3, y3);
    for (reg32 i = 0; i < 4; ++i)
        guest.setDword(quad + i * kVertexBytes + kVertexColour, colour);
    fe::direct<0x431900, graphics::sendState>(app, cpu, 1, 0);
    return clipAndDraw(guest, quad);
}

/* sub_4a7310: a quad of a texture, from the template at 0x4a7000: the fifth to twelfth
 * stack arguments its corners on the texture, then the texture and the colour. */
x86::reg32 frontend::loopa::panelTexturedQuad(App* app, CPU& cpu, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 x2,
                                              reg32 y2, reg32 x3, reg32 y3, reg32 u0, reg32 v0, reg32 u1, reg32 v1,
                                              reg32 u2, reg32 v2, reg32 u3, reg32 v3, reg32 texture, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12 + 0x90);
    const reg32 quad = guest.frame();
    copyDwords(guest, quad, 0x4a7000, 4 * kVertexBytes / 4);
    setPlace(guest, quad, x0, y0);
    setPlace(guest, quad + kVertexBytes, x1, y1);
    setPlace(guest, quad + 2 * kVertexBytes, x2, y2);
    setPlace(guest, quad + 3 * kVertexBytes, x3, y3);
    const reg32 u[4] = { u0, u1, u2, u3 }, v[4] = { v0, v1, v2, v3 };
    for (reg32 i = 0; i < 4; ++i)
    {
        const reg32 vertex = quad + i * kVertexBytes;
        guest.setDword(vertex + kVertexColour, colour);
        guest.setDword(vertex + kVertexU, u[i]);
        guest.setDword(vertex + kVertexV, v[i]);
    }
    fe::direct<0x431900, graphics::sendState>(app, cpu, 1, texture);
    return clipAndDraw(guest, quad);
}

/* sub_4a75e0: the panel, sized by the view: a dark quad, its edges shaded (two dark, two
 * light) and three pieces of the texture at [0x749a38] across its top: the left end, the
 * middle and the right end. */
x86::reg32 frontend::loopa::panelDraw(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x18);
    constexpr reg32 kPanelTexture = 0x749a38;
    constexpr reg32 kBody = 0x3f10063f, kDark = 0x20000000, kLight = 0x20ffffff, kBar = 0xafffffff;
    constexpr reg32 kBarTop = 0x3f440000, kBarBottom = 0x3f630000;
    const reg32 ebp = guest.frame() + 0x18;
    guest.call(kViewport, args({ ebp - 0x18, ebp - 0x14, ebp - 0x10, ebp - 0xc }));
    const reg32 viewX = guest.dword(ebp - 0x18), viewY = guest.dword(ebp - 0x14);
    const reg32 viewWidth = guest.dword(ebp - 0x10), viewHeight = guest.dword(ebp - 0xc);
    const auto divided = [](reg32 value, sreg32 by) { return reg32(sreg32(value) / by); };

    const reg32 barHeight = divided(viewHeight * 32, 960);
    const reg32 barEnd = divided(viewWidth * 21, 640);
    const reg32 barReach = divided(viewWidth * 16, 640);
    const reg32 border = divided(viewWidth * 3 + 0x140, 640);
    const reg32 left = viewX + divided(viewWidth, 20);
    const reg32 top = viewY + divided(viewHeight, 20) + barHeight;
    const reg32 right = left + divided(viewWidth * 18, 20);
    const reg32 bottom = top + divided(viewHeight * 18, 20);

    guest::direct<0x4a7080, panelQuad>(app, cpu, left, top, right, top, right, bottom, left, bottom, kBody);
    guest::direct<0x4a7080, panelQuad>(app, cpu, left + border, bottom - border, right - border, bottom - border, right,
                                       bottom, left, bottom, kDark);
    guest::direct<0x4a7080, panelQuad>(app, cpu, right - border, top + border, right, top, right, bottom,
                                       right - border, bottom - border, kDark);
    guest::direct<0x4a7080, panelQuad>(app, cpu, left, top, right, top, right - border, top + border, left + border,
                                       top + border, kLight);
    guest::direct<0x4a7080, panelQuad>(app, cpu, left, top, left + border, top + border, left + border,
                                       bottom - border, left, bottom, kLight);

    const reg32 barLeft = left - barReach;
    const reg32 barRight = barLeft + (right - left) + barReach * 2;
    const reg32 barY = top - barHeight - barHeight;
    const auto bar = [&](reg32 from, reg32 to, reg32 u0, reg32 u1) {
        return guest::direct<0x4a7310, panelTexturedQuad>(app, cpu, from, barY, to, barY, to, top, from, top, u0,
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
x86::reg32 frontend::loopa::raceLoad(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0x10);
    const reg32 ebp = guest.frame() + 0x10;
    const auto progress = [&](reg32 percent) {
        // The per cent in the word the code pushes, the bar handed its address.
        const reg32 pushed = cpu.esp - 4;
        guest.setDword(pushed, percent);
        fe::direct<0x495230, graphics::loadingProgress>(app, cpu, pushed);
    };
    const auto stopped = [&] {
        fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
        return guest.dword(kRaceAbort) != 0;
    };

    raceRunning(true);
    fe::direct<0x431900, graphics::sendState>(app, cpu, 0xd, 1);
    const reg32 width = fe::direct<0x4482f0, frontend::menua::chosenModeWidth>(app, cpu);
    guest.setDword(kChosenWidth, width);
    // The width in eax, where sub_4482f0 leaves it for the game's code.
    guest.call(kScreenMode, args({ width }));
    guest::direct<0x4be5e0, loopb::viewsLayout>(
        app, cpu, (std::uint8_t(gameSettings(guest).entries[0].kind) & 8) != 0 ? reg32(1) : reg32(0));
    guest.call(kViewport, args({ ebp - 8, ebp - 4, ebp - 0x10, ebp - 0xc }));
    // ebx and ecx as the call by address loads them: sub_4c62a0 below reads ebx as it is left.
    guest.leaveEbx(guest.dword(ebp - 0x10));
    guest.leaveEcx(guest.dword(ebp - 0xc));
    fe::direct<0x4352d0, controls::boundPointer>(app, cpu, 0, 0, guest.dword(ebp - 0x10), guest.dword(ebp - 0xc));
    guest.call(kLoadingText);
    guest.call(kScreenScales);
    guest.call(kLoadingScreen);
    guest.call(kLoadingText);
    guest::direct<0x477a50, recordingStart>(app, cpu);
    // By address: sub_4f0240 reads the eax sub_495230 leaves.
    guest.call(0x495230, args({}, { kProgress5 }));
    guest.call(kFlushKeys);
    fe::direct<0x477f60, frontend::neta::raceLoadStep>(app, cpu);
    if (stopped())
    {
        loadingScreenFit(app, cpu, false);
        return 1;
    }
    progress(kProgress10);
    fe::direct<0x477fd0, frontend::neta::raceMusicStep>(app, cpu);
    if (stopped())
        return 1;
    progress(kProgress35);
    fe::direct<0x4d56f0, frontend::rende::loadEffects>(app, cpu);
    if (stopped())
        return 1;
    progress(kProgress40);
    if (fe::direct<0x4dbbf0, frontend::sime::loadTrackPieces>(app, cpu, gameSettings(guest).track) != 0)
        return 1;
    progress(kProgress70);
    guest.call(kLoadWeather);
    if (stopped())
        return 1;
    progress(kProgress75);
    if (fe::direct<0x4283d0, frontend::loada::raceCarSteps>(app, cpu) != 0)
        return 1;
    progress(kProgress90);
    guest.call(kInitCameras);
    progress(kProgress93);
    fe::direct<0x4bc830, frontend::loada::raceSteps>(app, cpu);
    if (stopped())
        return 1;
    progress(kProgress95);
    if (fe::direct<0x4c62a0, frontend::inpa::raceSetUp>(app, cpu) != 0)
        return 1;
    progress(kProgress100);
    loadingScreenFit(app, cpu, false);
    return 0;
}

/* sub_4a3690: a whole race.  `mode` 1 starts it as finished unless told otherwise, `replay`
 * nonzero has the config read again after it.  A load stopped puts the settings back from the
 * frame.  What a callee reads left over from the code before it -- the copy's esi and edi above
 * all -- it finds as the original left them.  The answer goes to the menus. */
x86::reg32 frontend::loopa::raceSession(App* app, CPU& cpu, reg32 mode, reg32 replay)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0x20f4);
    const reg32 saved = guest.frame();
    const reg32 savedEnd = saved + kSettingsDwords * 4;

    copyDwords(guest, saved, SETTINGS_ADDRESS(fileSize), kSettingsDwords);
    cpu.ebx = mode;
    cpu.ecx = 0;
    cpu.esi = SETTINGS_ADDRESS(fileSize) + kSettingsDwords * 4;
    cpu.edi = savedEnd;
    guest.setDword(kRaceAbort, 0);
    // The game's code hands it 0 in edx too, which it does not read; the 0 stays there.
    guest.leaveEdx(0);
    fe::direct<0x4e0dee, ea::randomSeeded>(app, cpu, gameSettings(guest).raceSeed);
    guest.setDword(kRaceToggle, guest.dword(kRaceToggle) == 0 ? 1 : 0);
    if (gameSettings(guest).replayMode != 2 && guest.dword(kGhostLoaded) != 1)
        fe::direct<0x474ca0, frontend::loada::rosterBuild>(app, cpu);

    const reg32 stopped = reg32(sreg32(sreg16(reg16(guest::direct<0x4a3400, raceLoad>(app, cpu)))));
    guest.setDword(kRaceAbort, stopped);
    if (stopped != 0)
    {
        cpu.ecx = kSettingsDwords;
        cpu.esi = saved;
        cpu.edi = SETTINGS_ADDRESS(fileSize);
        guest.call(kTakeDownRace, args({ 1 }));
        copyDwords(guest, SETTINGS_ADDRESS(fileSize), saved, kSettingsDwords);
        cpu.ecx = 0;
        cpu.esi = savedEnd;
        cpu.edi = SETTINGS_ADDRESS(fileSize) + kSettingsDwords * 4;
        if (sreg32(gameSettings(guest).playMode) > 1 && guest.dword(kNetworkHost) != 0)
        {
            guest.call(kCloseNetwork);
            guest.leaveEdx(1);
            guest.setWord(kNetworkClosed, 1);
            return kSessionMenus;
        }
        if (replay != 0)
            guest.call(kLoadConfig);
        return mode != 0 ? kSessionNext : kSessionMenus;
    }

    guest.setDword(kRaceEnd, mode == 1 ? 0 : 2);
    while (guest.dword(kRaceLooping) != 0)
    {
        cpu.edi = 0;
        guest.setDword(kRaceLooping, 0);
        guest::direct<0x4a33b0, raceRestart>(app, cpu);
        const reg32 music = gameSettings(guest).raceMusicVolume;
        if (music != 0)
        {
            const reg32 byMode = sreg32(gameSettings(guest).musicMode) > 2 ? 1 : 0;
            guest.call(kRaceMusicStart, args({ gameSettings(guest).track, music, byMode }));
            guest.setByte(kMusicOn, 1);
        }
        else
        {
            guest.leaveEdx(guest.edx() & 0xffffff00);
            guest.setByte(kMusicOn, 0);
        }
        fe::direct<0x4e7630, port::stream::runDueTasks>(app, cpu, cpu.esp - 4, 0);
        fe::direct<0x4c63f0, frontend::inpa::inputReset>(app, cpu);
        fe::direct<0x4c4e40, frontend::inpa::runRaceLoop>(app, cpu);
    }

    if (guest.dword(kRaceEnd) != 1 && replay == 0 && sreg32(gameSettings(guest).playMode) > 1
        && (guest.word(kNetworkSpoken) != 0 || guest.dword(kNetworkHost) == 0))
        guest.setDword(kRaceEnd, 2);
    guest.call(kRaceSoundEnd);
    fe::direct<0x4109d0, port::musicStopped>(app, cpu, cpu.esp - 4, cpu.eax);
    guest.call(kTakeDownRace, args({ guest.dword(kRaceEnd) != 1 ? reg32(1) : reg32(0) }));
    fe::direct<0x410aa0, port::musicStreamEndOf>(app, cpu, cpu.esp - 4, cpu.eax, cpu.ebx, cpu.ecx);
    cpu.ecx = replay;
    guest.call(kMusicStreamMakeOf, args({ 0x12c }));
    if (replay == 0 && guest.dword(kRaceEnd) == 0)
        fe::direct<0x455410, frontend::menub::resultsBuild>(app, cpu);
    if (replay != 0)
        guest.call(kLoadConfig);
    const reg32 end = guest.dword(kRaceEnd);
    return end == 0 ? kSessionNext : end == 1 ? kSessionRestart : kSessionMenus;
}

// ---------------------------------------------------------------------------
// The game
// ---------------------------------------------------------------------------

/* sub_4b6020: the game started: its screen's first mode, the driver chosen and started, the
 * heap, the timer, the keys, the cars' list and the config, sound and music, the network's
 * reset and thread, the tables.  The x87 last, its word asking for what the race has.  The
 * heap's budget is the port's (tools/apply_heap_budget.py). */
x86::reg32 frontend::loopa::gameInit(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    constexpr reg32 kWindowName = 0x53fe08, kOutOfMemory = 0x53fe1c;
    constexpr reg32 kDriverType = 0x7a3a54, kDriverBit = 0x7a3a58, kLanguage = 0x7a3a50;
    constexpr reg32 kPowerMessage = 0x218, kPowerHandler = 0x4b5ff0;
    constexpr reg32 kHeapBudget = 64u * 1024u * 1024u;
    constexpr reg32 kControlWord = 0x79f3ec, kControlWordKept = 0x79f3f4;

    cpu.edx = 0x280;
    cpu.ecx = 0x1e0;
    cpu.ebx = 0x10;
    cpu.esi = 1;
    guest.setDword(0x564384, 0x280);
    guest.setDword(0x564388, 0x1e0);
    guest.setDword(0x56438c, 0x10);
    guest.setDword(0x564390, 1);
    // By address: sub_4f91d0 leaves edi changed, which sub_4b59c0 below reads.
    guest.call(0x4f91d0, args({ kWindowName }));
    guest.setDword(kChosenWidth, 1);
    guest.setDword(0x55fec8, 1);
    guest.setDword(0x55fec0, 1);
    fe::direct<0x4b5720, frontend::rendd::chooseDriver>(app, cpu, kDriverType);

    const reg32 driver = guest.dword(kDriverType);
    cpu.ecx = (cpu.ecx & 0xffffff00) | (driver & 0xff);
    guest.setDword(kDriverBit, reg32(1) << ((driver & 0xff) % 32));
    // The game's code hands it 0 in edx too, which it does not take.
    fe::direct<0x4b5f10, frontend::wina::checkMemory>(app, cpu, guest.dword(kLanguage));

    const reg32 heap = fe::direct<0x4f92b0, frontend::wata::heapStart>(app, cpu, kHeapBudget);
    guest.setDword(0x7a1a78, heap);
    if (sreg32(heap) < 0xfce000)
        guest.call(0x401010, args({}, { kOutOfMemory, heap }));

    fe::direct<0x4b5860, frontend::wina::checkLibrary>(app, cpu, guest.dword(kDriverType), guest.dword(kLanguage));
    guest.call(0x4b59c0, args({ guest.dword(kDriverType), guest.dword(kLanguage) }));
    fe::direct<0x4f9360, frontend::wina::videoClose>(app, cpu);
    cpu.ecx = guest.dword(kDriverType);
    if (cpu.ecx != 1)
        guest.call(0x4b5ea0, args({ cpu.ecx, guest.dword(kLanguage) }));
    guest.call(kScreenMode, args({ guest.dword(0x55fec8) }));
    // The game's code hands it 0x5a in edx too, which it does not read; sub_4bed50 below reads it there.
    guest.leaveEdx(0x5a);
    guest.setDword(kChosenWidth, *fe::direct<0x4bee80, graphics::screenModeFor>(app, cpu, guest.dword(0x55fecc)));
    cpu.ebx = 0;
    guest::direct<0x4be5e0, loopb::viewsLayout>(app, cpu, 0);
    // Its angle (0x5a) and last (0) where the code above left them in edx and ebx.  What sub_4bed50
    // leaves in ebx, ecx and edx nothing below reads: ebx is set again, sub_4cc6d0 changes ecx and edx.
    guest::direct<0x4bed50, loopb::projectionOnly>(app, cpu, 0x7cdb68, 0x5a, 0);
    // eax as the call by address leaves it: sub_4cc6d0 reads it.
    cpu.eax = guest::direct<0x4bed70, loopb::useClip>(app, cpu, 0x7cdb68);

    cpu.esi = 0;
    cpu.ebx = 0x3f800000;
    guest.setDword(0x560094, 0x3f800000);
    guest.setDword(0x560098, 0);
    fe::direct<0x4cc6d0, frontend::wina::handlerSet>(app, cpu, kPowerMessage, kPowerHandler);
    guest.setDword(0x564810, 0);
    guest.call(0x48eff0, args({ 0x80 }));
    guest.setDword(0x5643d8, 0x80);
    cpu.ebx = 0;
    fe::direct<0x4cdc90, frontend::wina::hookOn>(app, cpu, 1, 0);
    // By address: sub_4b5e50 below reads the ebx sub_4f0da0 leaves.
    guest.call(0x4f0da0, args({ 0x14, 0x2000 }));
    guest.call(0x4d5cc0, args({ 1, 1 }));
    guest.call(0x434f20);
    if ((guest.byte(0x55eb2e) & 0x20) != 0)
        fe::direct<0x472fc0, frontend::menuc::titleWait>(app, cpu);
    fe::direct<0x43bcf0, frontend::sime::removeNumberedFolders>(app, cpu);
    // By address: the second sub_4b5e50 below reads the ebx the first leaves.
    if (*fe::direct<0x439d90, load::loadCars>(app, cpu) != 0)
        guest.call(0x4b5e50, args({ guest.dword(kLanguage) }));
    if (guest.call(0x438bd0) == 0)
        fe::direct<0x4b5e50, frontend::wina::fatalMessage>(app, cpu, guest.dword(kLanguage));
    guest.call(kLoadConfig);
    guest.call(0x40fb10);
    fe::direct<0x410330, port::musicStreamMade>(app, cpu, cpu.esp - 4, cpu.eax, cpu.ecx);
    cpu.ecx = 0xffffffff;
    cpu.ebx = 1;
    guest.call(0x4a6390, args({ 1 }));
    guest.call(0x4df7a0, args({ 0x42dc20, 0 }, { 0x7a3aac }));
    fe::direct<0x496310, frontend::sime::makeRootTable>(app, cpu);
    guest::direct<0x497430, fillRootBytes>(app, cpu);
    fe::direct<0x4bf110, frontend::rendd::fillReciprocals>(app, cpu);

    guest.fpu().init();
    guest.setWord(kControlWord, guest.fpu().control.word);
    const reg32 word = guest.dword(kControlWord);
    guest.setDword(kControlWordKept, word);
    const reg32 single = word & 0xfffff0ff;
    guest.setDword(kControlWord, single);
    guest.fpu().setControl(guest.word(kControlWord));
    return single;
}

/* sub_497430: a byte for each of 128 floats, 0.5 to just under 2 by their top seven mantissa
 * bits: the reciprocal of its root, rounded to the same bits.  Entry 64, the one that is 1
 * itself, is 255 by hand.  eax is what was last worked out. */
x86::reg32 frontend::loopa::fillRootBytes(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12 + 4);
    constexpr reg32 kTable = 0x79f364;
    reg32 last = 0;
    for (reg32 i = 0; i < 0x80; ++i)
    {
        fe::direct<0x4ea920, frontend::simc::squareRoot>(app, cpu, (i << 17) | 0x3f000000);
        const Float root = guest.fpu().take();
        const float reciprocal = float(fp::div(1.0, root));
        reg32 bits;
        std::memcpy(&bits, &reciprocal, sizeof bits);
        last = ((bits + 0x2000) >> 15) & 0xff;
        guest.setByte(kTable + i, std::uint8_t(last));
    }
    guest.setByte(kTable + 0x40, 0xff);
    return last;
}

/* sub_4b62a0: whether the install's kind at 0x7a26b4 starts "remote", which the game keeps
 * with 13 taken from each letter: decoded in the frame, as the comparison is handed it. */
x86::reg32 frontend::loopa::installIsRemote(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 8);
    const reg32 word = guest.frame();
    for (reg32 i = 0; i < 6; ++i)
        guest.setByte(word + i, std::uint8_t(guest.byte(0x55fdc0 + i) + 0xd));
    return guest.call(0x4eb390, args({ 0x7a26b4, word, 6 })) == 0 ? 1 : 0;
}

/* sub_495190: the loading bar's four corners, x and y of each, scaled by the view's size.
 * The last corner's products have their operands the other way round, as the x87 had them. */
x86::reg32 frontend::loopa::barScale(App* app, CPU& cpu, reg32 corner0, reg32 corner1, reg32 corner2, reg32 corner3)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12 + 0x24);
    const reg32 ebp = guest.frame() + 0x24;
    // ebx and ecx as the call by address loads them and edx as it leaves it (sub_4beb20 its last place there):
    // the function leaves them.
    guest.leaveEbx(ebp - 0x18);
    guest.leaveEcx(ebp - 0x14);
    guest.leaveEdx(ebp - 0x10);
    fe::direct<0x4beb20, frontend::pause::readRect>(app, cpu, 0x7cdac0, ebp - 0x1c, ebp - 0x18, ebp - 0x14, ebp - 0x10);

    const Float scaleX = fp::mul(Float(sreg32(guest.dword(ebp - 0x14))), Float(guest.floatAt(0x53be28)));
    const Float x0 = fp::mul(Float(guest.floatAt(corner0)), scaleX);
    const Float scaleY = fp::mul(Float(sreg32(guest.dword(ebp - 0x10))), Float(guest.floatAt(0x53be2c)));
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
x86::reg32 frontend::loopa::loadingScreenFlag(App*, CPU&)
{
    return 1;
}

/* sub_459410: the frame of a menu's item (eax), in the colour edx: its place and size. */
x86::reg32 frontend::loopa::itemFrame(App* app, CPU& cpu, reg32 item, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    const reg32 height = guest.dword(item + 0x64), width = guest.dword(item + 0x60);
    const reg32 y = reg32(upperWord(guest, item + 6)), x = reg32(upperWord(guest, item + 4));
    return guest.call(0x459350, args({ x, y, width, height }, { colour }));
}

/* sub_475b10: the 3D font at 0x679010 let go. */
x86::reg32 frontend::loopa::freeLoadedBlock(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    constexpr reg32 kBlock = 0x679010;
    const reg32 block = guest.dword(kBlock);
    if (block == 0)
        return guest.eax();
    const reg32 result = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
    guest.setDword(kBlock, 0);
    return result;
}

/* sub_4a3390: the game's start, and sub_4bc980 handed what it returned. */
x86::reg32 frontend::loopa::gameStart(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    // The edi sub_4b6020 leaves (sub_4f91d0's) the function's caller (sub_4a39f0) does not read; its eax
    // sub_4bc980 hands on, there as the call by address leaves it.
    cpu.eax = guest::direct<0x4b6020, gameInit>(app, cpu);
    return guest::direct<0x4bc980, loopb::drawStart>(app, cpu);
}

/* sub_4a33a0: its end. */
x86::reg32 frontend::loopa::gameEnd(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    guest::direct<0x4bc990, loopb::drawEnd>(app, cpu);
    return guest.call(0x4b6250);
}

/* sub_4a33b0: what a race's loop does each time it goes round. */
x86::reg32 frontend::loopa::raceRestart(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 4);
    guest::direct<0x4bcb90, loopb::replayStart>(app, cpu);
    fe::direct<0x4c4740, frontend::simd::raceRestart>(app, cpu);
    guest.call(0x41de80);
    guest.call(kInitCameras);
    return guest.call(0x411680);
}

/* sub_4a6f70: nothing, called as the game ends. */
x86::reg32 frontend::loopa::nothingToEnd(App*, CPU& cpu)
{
    return cpu.eax;
}

/* The adapters, under the game's names. */

void frontend::loopLoadingScreenFlag(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_43bcb0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::loadingScreenFlag(app, cpu); });
}

void frontend::loopItemFrame(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_459410", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const reg32 item = cpu.eax, colour = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::itemFrame(app, cpu, item, colour); });
}

void frontend::loopFreeLoadedBlock(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_475b10", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::freeLoadedBlock(app, cpu); });
}

void frontend::ghostSaveFile(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4777d0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const reg32 name = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::saveGhost(app, cpu, name); });
}

void frontend::ghostPoseSetRotation(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477830", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 pose = cpu.eax, matrix = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::poseSetRotation(app, cpu, pose, matrix); });
}

void frontend::ghostPosePosition(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477850", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const reg32 pose = cpu.eax, out = cpu.esi;
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::posePosition(app, cpu, pose, out); });
}

void frontend::ghostPoseSetPosition(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4778c0", 0x14, 0, nullptr, 0 };
    const reg32 pose = cpu.eax;
    const reg32 x = stackArgument(app, cpu, 0), y = stackArgument(app, cpu, 1), z = stackArgument(app, cpu, 2);
    const reg32 slice = stackArgument(app, cpu, 3), car = stackArgument(app, cpu, 4);
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::poseSetPosition(app, cpu, pose, x, y, z, slice, car); });
}

void frontend::ghostPoseBetween(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477990", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 from = cpu.eax, to = cpu.edx, fraction = cpu.ebx, positionOut = cpu.ecx;
    const reg32 matrixOut = stackArgument(app, cpu, 0);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return loopa::poseBetween(app, cpu, from, to, fraction, positionOut, matrixOut);
    });
}

void frontend::ghostRecordedAt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4779e0", 0, 0, nullptr, 0 };
    const reg32 index = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::recordedAt(app, cpu, index); });
}

void frontend::ghostLoadedAt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477a10", 0, 0, nullptr, 0 };
    const reg32 index = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::loadedAt(app, cpu, index); });
}

void frontend::ghostRecordingStart(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477a50", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::recordingStart(app, cpu); });
}

void frontend::ghostLoadFile(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477b50", 0, 0, nullptr, 0 };
    const reg32 name = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::loadGhost(app, cpu, name); });
}

void frontend::ghostRecordCar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477c20", 0, 0, nullptr, 0 };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::recordCar(app, cpu, car); });
}

void frontend::ghostPlaybackPose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477d90", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 positionOut = cpu.eax, matrixOut = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::playbackPose(app, cpu, positionOut, matrixOut); });
}

void frontend::loopBarScale(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_495190", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 corner0 = cpu.eax, corner1 = cpu.edx, corner2 = cpu.ebx, corner3 = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::barScale(app, cpu, corner0, corner1, corner2, corner3); });
}

void frontend::loopFillRootBytes(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_497430", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::fillRootBytes(app, cpu); });
}

void frontend::loopGameStart(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a3390", 0, 0, nullptr, 0, kSavesNone };
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::gameStart(app, cpu); });
}

void frontend::loopGameEnd(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a33a0", 0, 0, nullptr, 0, kSavesNone };
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::gameEnd(app, cpu); });
}

void frontend::loopRaceRestart(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a33b0", 0, 0, nullptr, 0, kSavesNone };
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::raceRestart(app, cpu); });
}

void frontend::loopRaceLoad(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a3400", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::raceLoad(app, cpu); });
}

void frontend::loopRaceSession(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a3690", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 mode = cpu.eax, replay = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::raceSession(app, cpu, mode, replay); });
}

void frontend::loopNothingToEnd(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a6f70", 0, 0, nullptr, 0, kSavesNone };
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::nothingToEnd(app, cpu); });
}

void frontend::loopPanelQuad(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a7080", 0x14, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 x0 = cpu.eax, y0 = cpu.edx, x1 = cpu.ebx, y1 = cpu.ecx;
    reg32 s[5];
    for (reg32 i = 0; i < 5; ++i)
        s[i] = stackArgument(app, cpu, i);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return loopa::panelQuad(app, cpu, x0, y0, x1, y1, s[0], s[1], s[2], s[3], s[4]);
    });
}

void frontend::loopPanelTexturedQuad(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a7310", 0x38, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 x0 = cpu.eax, y0 = cpu.edx, x1 = cpu.ebx, y1 = cpu.ecx;
    reg32 s[14];
    for (reg32 i = 0; i < 14; ++i)
        s[i] = stackArgument(app, cpu, i);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return loopa::panelTexturedQuad(app, cpu, x0, y0, x1, y1, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8],
                                        s[9], s[10], s[11], s[12], s[13]);
    });
}

void frontend::loopPanelDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4a75e0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::panelDraw(app, cpu); });
}

void frontend::loopGameInit(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b6020", 0, 0, kInitStrings, std::size(kInitStrings), fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::gameInit(app, cpu); });
}

void frontend::loopInstallIsRemote(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b62a0", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::installIsRemote(app, cpu); });
}

void frontend::poseBlend(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b6a30", 0xc, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 rotationFrom = cpu.eax, rotationTo = cpu.edx, positionFrom = cpu.ebx, positionTo = cpu.ecx;
    const reg32 fraction = stackArgument(app, cpu, 0), rotationOut = stackArgument(app, cpu, 1);
    const reg32 positionOut = stackArgument(app, cpu, 2);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return loopa::blend(app, cpu, rotationFrom, rotationTo, positionFrom, positionTo, fraction, rotationOut, positionOut);
    });
}

void frontend::poseMatrixOfRotation(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b6ca0", 0, 0, nullptr, 0 };
    const reg32 rotation = cpu.eax, out = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::matrixOfRotation(app, cpu, rotation, out); });
}

void frontend::poseRotationNearest(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b6d90", 0, 0, nullptr, 0 };
    const reg32 rotation = cpu.eax, other = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::rotationNearest(app, cpu, rotation, other); });
}

void frontend::poseRotationOfMatrix(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b6e80", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 matrix = cpu.eax, out = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return loopa::rotationOfMatrix(app, cpu, matrix, out); });
}

}
