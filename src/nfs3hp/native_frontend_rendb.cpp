#include "native_frontend_rendb.h"
#include "game_settings.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_frontend.h"
#include "native_frontend_eab.h"
#include "native_frontend_hudx.h"
#include "native_frontend_hudz.h"
#include "native_frontend_loada.h"
#include "native_frontend_loopb.h"
#include "native_frontend_renda.h"
#include "native_frontend_rende.h"
#include "native_frontend_sima.h"
#include "native_frontend_simd.h"
#include "native_graphics.h"
#include "native_vertices.h"
#include <nfs3hp.h>
#include <cstring>
#include <initializer_list>
#include <iterator>

/* The race's HUD, the parts of it under what port_hud.cpp and native_render.cpp
 * draw, decompiled.  All of them run on voodoo2a: nothing here is another
 * driver's, and the one test of the driver's flags ([0x7a3a58] & 2, off on
 * voodoo2a) is kept as it is.
 *
 *   setting up   the HUD's vertices as a race starts: the art's rectangles and
 *                the batches taken from the queues (sub_47bc40), and its pictures,
 *                hud98.fsh with the speedometer's scale for this car's revs laid
 *                into it (sub_47f590, sub_47f340, sub_47f2e0);
 *   each frame   a group's eight quads queued and taken back (sub_47fbe0,
 *                sub_47fcb0), the frame of element 9 fitted to the screen
 *                (sub_47fd80), which kinds of element are text (sub_47bc20),
 *                the standings' rows (sub_47ee80), the cop's markers
 *                (sub_47d480), the mark around the car a cop chases (sub_47d580),
 *                a box in screen numbers (sub_47d1d0), and the turn of the road
 *                in a matrix of the HUD's (sub_47f090).
 *
 * The HUD's state stays in guest memory, where the port's functions read it, and
 * what they do is called by address.
 *
 * Plan step 5: each function a typed core in frontend::rendb (native_frontend_rendb.h), in a
 * frame of its own (fe::Frame: what it calls runs at the stack addresses it did), and an adapter
 * under the game's name for the calls by address.  A core of this file calls another as C++
 * (guest::direct), and the typed cores of other files too (fe::direct); the rest of the game
 * still by address. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using fe::X87;
using x86::Float;
using x86::reg16;
using x86::reg32;
using x86::sreg32;

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, reg32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

Float floatFrom(const Guest& guest, reg32 address)
{
    return Float(guest.floatAt(address));
}

reg32 bitsOf(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

float floatOf(reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* What a jump after fcom, fnstsw ax and sahf reads of fp::compare's word. */
bool jumpsAbove(reg16 status) { return (status & 0x4100) == 0; }
bool jumpsAboveOrEqual(reg16 status) { return (status & 0x0100) == 0; }
bool jumpsBelow(reg16 status) { return (status & 0x0100) != 0; }
bool jumpsBelowOrEqual(reg16 status) { return (status & 0x4100) != 0; }

// The registers a function's pushes keep; the rest it leaves as they fall.
constexpr std::uint8_t kKeepsEsiEdi = fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kKeepsEcxEsiEdi = fe::kRestoreEcx | kKeepsEsiEdi;
constexpr std::uint8_t kKeepsEbxEcxEsiEdi = fe::kRestoreEbx | kKeepsEcxEsiEdi;

// The game's functions, by address.
constexpr reg32 kQueueQuad = 0x49bd30;         // eax a queue, edx, ebx, ecx and the stack its corners
constexpr reg32 kUnqueueQuad = 0x49bdc0;       // likewise
constexpr reg32 kDrawQuad = 0x4c12e0;          // eax, edx, ebx, ecx its corners
constexpr reg32 kDrawSegment = 0x4c0b20;       // eax, edx its ends
constexpr reg32 kChop = 0x4dfd56;              // __CHP: st(0) rounded toward 0

// A corner: 0x20 bytes.  What the clipping functions read of it.
constexpr reg32 kCornerBytes = 0x20;
constexpr reg32 kQuadBytes = 4 * kCornerBytes;
constexpr reg32 kCornerX = 0, kCornerY = 4, kCornerZ = 8, kCornerW = 0xc;
constexpr reg32 kCornerColour = 0x10, kCornerClip = 0x14, kCornerU = 0x18, kCornerV = 0x1c;

// The view's limits, compared with a corner's numbers as integers.
constexpr reg32 kClipFar = 0x7d34f8, kClipLeft = 0x7d34fc, kClipBottom = 0x7d3500, kClipTop = 0x7d3504, kClipRight = 0x7d350c;

constexpr reg32 kDisplayFlags = 0x7a3a58;      // byte
constexpr std::uint8_t kCardWithAlpha = 2;     // not voodoo2a's
constexpr reg32 kViewWidth = 0x7cdac8, kViewHeight = 0x7cdacc;
constexpr reg32 kCarCount = 0x5efda0;
constexpr reg32 kCars = 0x5efc48;              // by the place in the race's list

// The queues.
constexpr reg32 kDialQueue = 0x749a38;
constexpr reg32 kBackQueue = 0x725748;
constexpr reg32 kFrameQueue = 0x700248;

constexpr reg32 kWhite = 0xffffffff;
constexpr reg32 kBlack = 0xff000000;

/* A corner's clip code as sub_4c12e0 wants it, as the game writes it out in front
 * of each call: behind the view or past its far limit 0x10; otherwise a bit for
 * each side it is past: 1 left, 2 right, 4 bottom, 8 top. */
void storeClipCode(Guest& guest, reg32 corner)
{
    const reg32 w = guest.dword(corner + kCornerW);
    std::uint8_t code = 0x10;
    if ((w & 0x80000000) == 0 && sreg32(w) < sreg32(guest.dword(kClipFar)))
    {
        code = 0;
        const reg32 y = guest.dword(corner + kCornerY);
        if ((y & 0x80000000) != 0 || sreg32(y) < sreg32(guest.dword(kClipTop)))
            code |= 8;
        else if (sreg32(y) > sreg32(guest.dword(kClipBottom)))
            code |= 4;
        const reg32 x = guest.dword(corner + kCornerX);
        if ((x & 0x80000000) != 0 || sreg32(x) < sreg32(guest.dword(kClipLeft)))
            code |= 1;
        else if (sreg32(x) > sreg32(guest.dword(kClipRight)))
            code |= 2;
    }
    guest.setByte(corner + kCornerClip, code);
}

/* `words` dwords copied within guest memory, as rep movsd copies them. */
void copyWords(Guest& guest, reg32 to, reg32 from, reg32 words)
{
    for (reg32 i = 0; i < words; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* The colour of `count` corners. */
void paintCorners(Guest& guest, reg32 corners, reg32 count, reg32 colour)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(corners + i * kCornerBytes + kCornerColour, colour);
}

/* A rectangle of the art, for a quad's four corners: left and right, top and
 * bottom, as the bits of the floats. */
struct ArtRectangle
{
    reg32 left, right, top, bottom;
};

void placeArt(Guest& guest, reg32 quad, const ArtRectangle& art)
{
    guest.setDword(quad + kCornerU, art.left);
    guest.setDword(quad + kCornerV, art.top);
    guest.setDword(quad + kCornerBytes + kCornerU, art.right);
    guest.setDword(quad + kCornerBytes + kCornerV, art.top);
    guest.setDword(quad + 2 * kCornerBytes + kCornerU, art.right);
    guest.setDword(quad + 2 * kCornerBytes + kCornerV, art.bottom);
    guest.setDword(quad + 3 * kCornerBytes + kCornerU, art.left);
    guest.setDword(quad + 3 * kCornerBytes + kCornerV, art.bottom);
}

/* ---------------------------------------------------------------------------
 * The kinds of element that are text: sub_47bc20
 * ------------------------------------------------------------------------- */

}  // namespace

/* Eax an element's kind: 1 for the speed and the revs in figures, the laps, the
 * times, the percentage, the place and the like, 0 for the rest and past the 23
 * kinds. */
x86::reg32 frontend::rendb::kindIsText(App*, CPU&, reg32 kind)
{
    constexpr reg32 kLastKind = 22;
    constexpr reg32 kTextKinds = 1u << 0 | 1u << 2 | 1u << 4 | 1u << 5 | 1u << 7 | 1u << 10 | 1u << 11 | 1u << 12 | 1u << 14 | 1u << 19;
    return kind <= kLastKind ? (kTextKinds >> kind) & 1 : 0;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The HUD's vertices as a race starts: sub_47bc40
 * ------------------------------------------------------------------------- */

// Where the art of the fixed quads is: 21 quads at 0x724790, four corners each.
constexpr reg32 kFixedQuads = 0x724790;
constexpr ArtRectangle kFixedArt[] = {
    { 0x3f27a7a8, 0x3f28a8a9, 0x3f3ababb, 0x3f63e3e4 },
    { 0x3ec8c8c9, 0x3efefeff, 0x3f3ababb, 0x3f63e3e4 },
    { 0x3f018182, 0x3f26a6a7, 0x3f3ababb, 0x3f63e3e4 },
    { 0x3ea8a8a9, 0x3ec6c6c7, 0x3f57d7d8, 0x3f66e6e7 },
    { 0x3e78f8f9, 0x3e909091, 0x3f57d7d8, 0x3f66e6e7 },
    { 0x3e929293, 0x3ea6a6a7, 0x3f57d7d8, 0x3f66e6e7 },
    { 0x3d50d0d1, 0x3e3cbcbd, 0x3f44c4c5, 0x3f56d6d7 },
    { 0x3e40c0c1, 0x3ea4a4a5, 0x3f44c4c5, 0x3f56d6d7 },
    { 0x00000000, 0x3d40c0c1, 0x3f44c4c5, 0x3f56d6d7 },
    { 0x3ea6a6a7, 0x3ec6c6c7, 0x3f44c4c5, 0x3f56d6d7 },
    { 0x3e38b8b9, 0x3e74f4f5, 0x3f57d7d8, 0x3f66e6e7 },
    { 0x3f29a9aa, 0x3f4ccccd, 0x3f46c6c7, 0x3f63e3e4 },
    { 0x3ea0a0a1, 0x3eb2b2b3, 0x3f3ababb, 0x3f43c3c4 },
    { 0x00000000, 0x3d109091, 0x3f3ababb, 0x3f43c3c4 },
    { 0x3d20a0a1, 0x3d989899, 0x3f3ababb, 0x3f43c3c4 },
    { 0x3da0a0a1, 0x3de8e8e9, 0x3f3ababb, 0x3f43c3c4 },
    { 0x3df0f0f1, 0x3e1c9c9d, 0x3f3ababb, 0x3f43c3c4 },
    { 0x3e20a0a1, 0x3e44c4c5, 0x3f3ababb, 0x3f43c3c4 },
    { 0x3e48c8c9, 0x3e6ceced, 0x3f3ababb, 0x3f43c3c4 },
    { 0x3e70f0f1, 0x3e8a8a8b, 0x3f3ababb, 0x3f43c3c4 },
    { 0x3e8c8c8d, 0x3e9e9e9f, 0x3f3ababb, 0x3f43c3c4 },
};

// A corner's depth and its w, wherever the HUD sets them itself.
constexpr reg32 kFlatDepth = 0x37800080, kFlatW = 0x3f7fff00;

/* The fixed quads: white, flat, and each at its place in the art. */
void setFixedQuads(Guest& guest)
{
    paintCorners(guest, kFixedQuads, reg32(4 * std::size(kFixedArt)), kWhite);
    for (std::size_t q = 0; q < std::size(kFixedArt); ++q)
    {
        const reg32 quad = kFixedQuads + reg32(q) * kQuadBytes;
        for (reg32 i = 0; i < 4; ++i)
        {
            guest.setDword(quad + i * kCornerBytes + kCornerZ, kFlatDepth);
            guest.setDword(quad + i * kCornerBytes + kCornerW, kFlatW);
        }
        placeArt(guest, quad, kFixedArt[q]);
    }
}

/* A batch of corners from a queue.  Their address, which sub_49bd10 has put in
 * `corners`. */
reg32 newBatch(Guest& guest, x86::CPU& cpu, reg32 queue, reg32 count, reg32 corners, reg32 firstNumber)
{
    fe::direct<0x49bd10, frontend::hudz::queueTakeCorners>(guest.app(), cpu, queue, count, corners, firstNumber);
    return guest.dword(corners);
}

void newWhiteBatch(Guest& guest, x86::CPU& cpu, reg32 queue, reg32 count, reg32 corners, reg32 firstNumber)
{
    paintCorners(guest, newBatch(guest, cpu, queue, count, corners, firstNumber), count, kWhite);
}

/* Sixteen corners, four quads: the first and third white; the second and fourth a
 * point of the art each, white where [0x6fd4c8] is set and black on another point
 * where not. */
void setDialBatch(Guest& guest, x86::CPU& cpu)
{
    constexpr reg32 kSecondU = 0x3ebf0000, kSecondV = 0x3d080000, kSecondPlainU = 0x3e020000, kSecondPlainV = 0x3d080000;
    constexpr reg32 kFourthU = 0x3f608000, kFourthV = 0x3c600000, kFourthPlainU = 0x3e460000, kFourthPlainV = 0x3d9c0000;
    const reg32 corners = newBatch(guest, cpu, kDialQueue, 0x10, 0x791b38, 0x791ad0);
    const auto setQuad = [&](reg32 quad, reg32 u, reg32 v, reg32 plainU, reg32 plainV) {
        const bool flagged = gameSettings(guest).night != 0;
        paintCorners(guest, quad, 4, flagged ? kWhite : kBlack);
        placeArt(guest, quad, flagged ? ArtRectangle{ u, u, v, v } : ArtRectangle{ plainU, plainU, plainV, plainV });
    };
    paintCorners(guest, corners, 4, kWhite);
    setQuad(corners + kQuadBytes, kSecondU, kSecondV, kSecondPlainU, kSecondPlainV);
    paintCorners(guest, corners + 2 * kQuadBytes, 4, kWhite);
    setQuad(corners + 3 * kQuadBytes, kFourthU, kFourthV, kFourthPlainU, kFourthPlainV);
}

/* Two quads, white, on the same rectangle of the art. */
void setPairBatch(Guest& guest, x86::CPU& cpu)
{
    constexpr ArtRectangle kArt = { 0x3f358000, 0x3f700000, 0x3f380000, 0x3f430000 };
    const reg32 corners = newBatch(guest, cpu, kDialQueue, 8, 0x791ae0, 0x791acc);
    paintCorners(guest, corners, 8, kWhite);
    placeArt(guest, corners, kArt);
    placeArt(guest, corners + kQuadBytes, kArt);
}

/* A back panel's eight corners: see through, but on a card with the flag. */
void setPanelBatch(Guest& guest, x86::CPU& cpu, reg32 corners, reg32 firstNumber)
{
    constexpr reg32 kPanelColour = 0x3f10063f, kPanelColourOpaque = 0xff10063f;
    const reg32 at = newBatch(guest, cpu, kBackQueue, 8, corners, firstNumber);
    for (reg32 i = 0; i < 8; ++i)
        paintCorners(guest, at + i * kCornerBytes, 1, (guest.byte(kDisplayFlags) & kCardWithAlpha) != 0 ? kPanelColourOpaque : kPanelColour);
}

/* The figures' rectangles: nine of the art's quads by the game's tables of left,
 * top, width and height in texels, half a texel in, and again thirteen quads on;
 * four of them turned back to front after those.  The rectangle counts by view,
 * cleared first. */
void setFigureBatch(Guest& guest, x86::CPU& cpu)
{
    constexpr reg32 kRectangleCounts = 0x700240;   // by view
    constexpr reg32 kLefts = 0x55d370, kTops = 0x55d394, kWidths = 0x55d3b8, kHeights = 0x55d3dc;
    constexpr reg32 kHalf = 0x53b420, kPerTexel = 0x53b424, kMinusHalf = 0x53b428;
    constexpr reg32 kFigures = 9, kTurnedFrom = 1, kTurnedTo = 5;
    constexpr reg32 kAgain = 13 * kQuadBytes;      // the same quads, a second time
    constexpr reg32 kTurned = 8 * kQuadBytes;      // the turned ones: number 1 of them is quad 9

    guest.setDword(kRectangleCounts, 0);
    guest.setDword(kRectangleCounts + 4, 0);
    const reg32 corners = newBatch(guest, cpu, kDialQueue, 0x68, 0x791a58, 0x791a84);
    paintCorners(guest, corners, 0x68, kWhite);

    const Float half = floatFrom(guest, kHalf), perTexel = floatFrom(guest, kPerTexel), minusHalf = floatFrom(guest, kMinusHalf);
    const auto sum = [&](reg32 table, reg32 extent, reg32 i) {
        return fp::add(floatFrom(guest, table + 4 * i), floatFrom(guest, extent + 4 * i));
    };
    for (reg32 i = 0; i < kFigures; ++i)
    {
        const float left = float(fp::mul(fp::add(floatFrom(guest, kLefts + 4 * i), half), perTexel));
        const float top = float(fp::mul(fp::add(half, floatFrom(guest, kTops + 4 * i)), perTexel));
        const float right = float(fp::mul(fp::add(sum(kLefts, kWidths, i), minusHalf), perTexel));
        const float bottom = float(fp::mul(perTexel, fp::add(minusHalf, sum(kTops, kHeights, i))));
        const ArtRectangle art = { bitsOf(left), bitsOf(right), bitsOf(top), bitsOf(bottom) };
        placeArt(guest, corners + i * kQuadBytes, art);
        placeArt(guest, corners + kAgain + i * kQuadBytes, art);
    }
    for (reg32 i = kTurnedFrom; i < kTurnedTo; ++i)
    {
        const float left = float(fp::mul(fp::add(sum(kLefts, kWidths, i), minusHalf), perTexel));
        const float top = float(fp::mul(fp::add(floatFrom(guest, kTops + 4 * i), half), perTexel));
        const float right = float(fp::mul(fp::add(half, floatFrom(guest, kLefts + 4 * i)), perTexel));
        const float bottom = float(fp::mul(fp::add(minusHalf, sum(kTops, kHeights, i)), perTexel));
        const ArtRectangle art = { bitsOf(left), bitsOf(right), bitsOf(top), bitsOf(bottom) };
        placeArt(guest, corners + kTurned + i * kQuadBytes, art);
        placeArt(guest, corners + kAgain + kTurned + i * kQuadBytes, art);
    }
}

/* Twenty corners kept in two places: five quads, three on the art and two whole
 * textures in colours of their own. */
void setKeptQuads(Guest& guest)
{
    constexpr reg32 kFirst = 0x7244d8, kSecond = 0x724258;
    constexpr reg32 kOne = 0x3f800000;
    constexpr ArtRectangle kArt[] = {
        { 0x3f268000, 0x3f448000, 0x3f648000, kOne },
        { 0x3f660000, 0x3f660000, 0x3f648000, kOne },
        { 0x3f468000, 0x3f640000, 0x3f648000, kOne },
        { 0, kOne, 0, kOne },
        { 0, kOne, 0, kOne },
    };
    constexpr reg32 kColours[] = { 0xafffffff, 0xafffffff, 0xafffffff, 0x9ff73030, 0x9f477dfc };
    for (reg32 q = 0; q < 5; ++q)
        for (const reg32 place : { kFirst, kSecond })
        {
            const reg32 quad = place + q * kQuadBytes;
            for (reg32 i = 0; i < 4; ++i)
            {
                const reg32 corner = quad + i * kCornerBytes;
                guest.setDword(corner + kCornerZ, kFlatDepth);
                guest.setDword(corner + kCornerW, kFlatW);
                guest.setDword(corner + kCornerColour, kColours[q]);
                guest.setDword(corner + kCornerClip, 0);
            }
            placeArt(guest, quad, kArt[q]);
        }
}

}  // namespace

/* What sub_484070 starts a race's HUD with. */
x86::reg32 frontend::rendb::setUpHudVertices(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    constexpr reg32 kGreyColour = 0xaf7f7f7f;
    // What the last of its loops leaves in eax: its last corner's offset.
    constexpr reg32 kLeft = 19 * kCornerBytes;

    setFixedQuads(guest);
    newWhiteBatch(guest, cpu, kFrameQueue, 0x20, 0x791a90, 0x791a80);
    newWhiteBatch(guest, cpu, kDialQueue, 0x40, 0x791ae4, 0x791ac8);
    newWhiteBatch(guest, cpu, kDialQueue, 0x40, 0x791b34, 0x791b30);
    newWhiteBatch(guest, cpu, kDialQueue, 0xe, 0x791b2c, 0x791b24);
    newWhiteBatch(guest, cpu, kDialQueue, 0x40, 0x791a8c, 0x791ac4);
    setDialBatch(guest, cpu);
    setPairBatch(guest, cpu);
    newWhiteBatch(guest, cpu, kDialQueue, 0x40, 0x791b40, 0x791b48);
    setPanelBatch(guest, cpu, 0x791b58, 0x791b20);
    newWhiteBatch(guest, cpu, kDialQueue, 0x40, 0x791b18, 0x791ad8);
    setPanelBatch(guest, cpu, 0x791ad4, 0x791adc);
    paintCorners(guest, newBatch(guest, cpu, kDialQueue, 0x38, 0x791ac0, 0x791a88), 0x38, kGreyColour);
    setFigureBatch(guest, cpu);
    setKeptQuads(guest);
    return kLeft;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The HUD's pictures: sub_47f590, sub_47f340, sub_47f2e0
 * ------------------------------------------------------------------------- */

constexpr reg32 kPictureHeader = 0x10;

}  // namespace

/* A rectangle of 32-bit pixels from one picture into another, a row at a time
 * (sub_4ea4f0, memmove, which keeps ecx): eax and edx the pictures' pixels, ebx and
 * ecx the place in the first; the stack the place in the second, the pictures'
 * widths, the rectangle's width and height. */
x86::reg32 frontend::rendb::copyRectangle(App* app, CPU& cpu, reg32 from, reg32 to, reg32 fromX, reg32 fromY, reg32 toX,
                                         reg32 toY, reg32 fromPitch, reg32 toPitch, reg32 width, reg32 height)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    constexpr reg32 kMemmove = 0x4ea4f0;
    from += (fromY * fromPitch + fromX) << 2;
    const reg32 toStart = (toY * toPitch + toX) << 2;
    to += toStart;
    guest.leaveEcx(toStart);
    guest.leaveEdx(toPitch);
    for (reg32 rows = height; --rows != reg32(-1);)
    {
        guest.call(kMemmove, args({ from, to, width << 2, toPitch << 2 }));
        from += fromPitch << 2;
        to += toPitch << 2;
    }
    return reg32(-1);
}

namespace
{

/* The name of the scale for a car's revs, "400" to "900": by five sixths of its
 * limit. */
reg32 scaleName(reg32 revLimit)
{
    struct Scale
    {
        std::int32_t above;
        reg32 name;
    };
    static constexpr Scale kScales[] = {
        { 0x222e, 0x53b4a8 }, { 0x203a, 0x53b4ac }, { 0x1e46, 0x53b4b0 }, { 0x1c52, 0x53b4b4 }, { 0x1a5e, 0x53b4b8 },
        { 0x186a, 0x53b4bc }, { 0x1676, 0x53b4c0 }, { 0x1482, 0x53b4c4 }, { 0x128e, 0x53b4c8 }, { 0x109a, 0x53b4cc },
    };
    const std::int32_t shown = std::int32_t(revLimit * 5) / 6;
    for (const Scale& scale : kScales)
        if (shown > scale.above)
            return scale.name;
    return 0x53b4d0;
}

}  // namespace

/* Into the picture "4444" of the file (eax): each view's car's scale ("900L", or
 * "900H" on a screen over 320 wide), side by side; and on a narrow screen "LSPD"
 * twice over at its left. */
x86::reg32 frontend::rendb::layScalesIntoHud(App* app, CPU& cpu, reg32 file)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x14);
    constexpr reg32 kPictureName = 0x53b4a0;   // "4444"
    constexpr reg32 kLowSuffix = 0x53b4d4, kHighSuffix = 0x53b4d8;  // "L", "H"
    constexpr reg32 kNarrowName = 0x53b4dc;    // "LSPD"
    constexpr reg32 kPlayerCars = 0x5efac8;
    constexpr reg32 kCarData = 0x528, kDataRevLimit = 0x1c4;
    constexpr std::int32_t kNarrowWidth = 0x140;
    constexpr reg32 kPitch = 0x100, kScalePitch = 0x80, kScaleSide = 0x40, kScalesX = 0x80;

    const reg32 name = guest.frame();          // 8 bytes, before the locals it had
    const reg32 target = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, cpu, file, kPictureName);
    reg32 views = 0;
    for (reg32 seat = 0;; ++seat)
    {
        views = gameSettings(guest).playMode == 1 ? 2 : 1;
        if (std::int32_t(views) <= std::int32_t(seat))
            break;
        const reg32 player = gameSettings(guest).playMode == 1 ? seat : gameSettings(guest).localPlayer;
        const reg32 car = guest.dword(kPlayerCars + 4 * player);
        guest.copyString(name, scaleName(guest.dword(guest.dword(car + kCarData) + kDataRevLimit)));
        const bool narrow = std::int32_t(guest.dword(kViewWidth)) <= kNarrowWidth;
        guest.copyString(name + guest.length(name), narrow ? kLowSuffix : kHighSuffix);
        const reg32 picture = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, cpu, file, name);
        if (picture != 0)
        {
            const reg32 fromX = gameSettings(guest).night != 0 ? kScaleSide : 0;
            guest::direct<0x47f2e0, copyRectangle>(app, cpu, picture + kPictureHeader, target + kPictureHeader, fromX, 0,
                                                   (seat << 6) + kScalesX, 0, kScalePitch, kPitch, kScaleSide, kScaleSide);
        }
    }
    if (std::int32_t(guest.dword(kViewWidth)) > kNarrowWidth)
        return views;
    const reg32 narrowPicture = fe::direct<0x4ef1e0, frontend::eab::shapeFileFind>(app, cpu, file, kNarrowName);
    if (narrowPicture == 0)
        return 0;
    const reg32 from = narrowPicture + kPictureHeader, to = target + kPictureHeader;
    guest::direct<0x47f2e0, copyRectangle>(app, cpu, from, to, 0, 0, 0, 0, kScalePitch, kPitch, kScalePitch, kScaleSide);
    return guest::direct<0x47f2e0, copyRectangle>(app, cpu, from, to, 0, 0, 0, kScaleSide, kScalePitch, kPitch, kScalePitch,
                                                  kScaleSide);
}

/* hud98.fsh read: its "1555" and its "4444", with the scales laid in first, made
 * textures for 0x791a94 and 0x791ae8, the file let go, and the cabin's instruments
 * loaded.  The screen kept alive between each (sub_4a33d0). */
x86::reg32 frontend::rendb::loadHudPictures(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x100);
    constexpr reg32 kSprintf = 0x4df690;
    constexpr reg32 kLoadCabin = 0x488a80;
    constexpr reg32 kFileFormat = 0x53b4e4;    // "%shud98.fsh"
    constexpr reg32 kArtFolder = 0x7a29b4;
    constexpr reg32 kPlainName = 0x53b4f0;     // "1555"
    constexpr reg32 kAlphaName = 0x53b4a0;     // "4444"
    constexpr reg32 kPlainTexture = 0x791a94, kAlphaTexture = 0x791ae8;

    const reg32 path = guest.frame();
    guest.call(kSprintf, args({}, { path, kFileFormat, kArtFolder }));
    const reg32 file = *fe::direct<0x4e0ec0, ea::loadWhole>(app, cpu, path, 0x10);
    if (file == 0)
        return 0;
    fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
    fe::direct<0x4d4080, frontend::rende::loadShapeTexture>(app, cpu, file, kPlainName, 0x20, kPlainTexture, 3);
    fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
    guest::direct<0x47f340, layScalesIntoHud>(app, cpu, file);
    fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
    const reg32 alphaMode = (guest.byte(kDisplayFlags) & kCardWithAlpha) != 0 ? 3 : 7;
    fe::direct<0x4d4080, frontend::rende::loadShapeTexture>(app, cpu, file, kAlphaName, 0x100, kAlphaTexture, alphaMode);
    fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
    fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, file);
    fe::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
    return guest.call(kLoadCabin);
}

namespace
{

/* ---------------------------------------------------------------------------
 * Eight quads queued and taken back: sub_47fbe0, sub_47fcb0
 * ------------------------------------------------------------------------- */

reg32 quadCall(Guest& guest, reg32 function, reg32 queue, reg32 first)
{
    return guest.call(function, args({ queue, first, first + 1, first + 2 }, { first + 3 }));
}

/* A group's eight quads, the corners numbered on from eax, on the dials' queue. */
reg32 eightQuads(Guest& guest, reg32 function, reg32 first)
{
    reg32 result = 0;
    for (reg32 q = 0; q < 8; ++q)
        result = quadCall(guest, function, kDialQueue, first + 4 * q);
    return result;
}

}  // namespace

/* sub_47fbe0: a group's eight quads queued. */
x86::reg32 frontend::rendb::queueEightQuads(App* app, CPU& cpu, reg32 first)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    return eightQuads(guest, kQueueQuad, first);
}

/* sub_47fcb0: a group's eight quads taken out of the queue. */
x86::reg32 frontend::rendb::unqueueEightQuads(App* app, CPU& cpu, reg32 first)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20);
    return eightQuads(guest, kUnqueueQuad, first);
}

/* ---------------------------------------------------------------------------
 * The frame of element 9: sub_47fd80
 * ------------------------------------------------------------------------- */

/* Set around its rectangle (sub_47f650) and queued, eight quads; then each of its
 * four sides that comes off the screen pulled back to the screen's edge, and the
 * three quads of that side taken out of the queue. */
x86::reg32 frontend::rendb::fitFrameToScreen(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x3c);
    constexpr reg32 kFrameCorners = 0x791a90, kFrameFirst = 0x791a80;
    constexpr reg32 kBox = 0x7497e8;           // left, top, right, bottom
    constexpr reg32 kBuildFrame = 0x47f650;    // eax the corners, ebx, edx, stack, ecx its inside

    const reg32 width = guest.dword(kViewWidth);
    const reg32 corners = guest.dword(kFrameCorners);
    const reg32 first = guest.dword(kFrameFirst);
    const reg32 height = guest.dword(kViewHeight);
    guest.call(kBuildFrame, args({ corners, guest.dword(kBox + 4), guest.dword(kBox), guest.dword(kBox + 0xc) }, { guest.dword(kBox + 8) }));
    for (reg32 q = 0; q < 8; ++q)
        quadCall(guest, kQueueQuad, kFrameQueue, first + 4 * q);
    const auto takeOut = [&](reg32 quad) { return quadCall(guest, kUnqueueQuad, kFrameQueue, first + 4 * quad); };
    const auto setFour = [&](reg32 a, reg32 b, reg32 c, reg32 d, float value) {
        for (const reg32 at : { a, b, c, d })
            guest.setFloat(corners + at, value);
    };

    // Past the right.
    if (!jumpsAbove(fp::compare(X87::integer(std::int32_t(width)), floatFrom(guest, corners + 0x120))))
    {
        setFour(0x2c0, 0x2a0, 0xc0, 0xa0, float(X87::integer(std::int32_t(width - 1))));
        takeOut(2);
        takeOut(3);
        takeOut(4);
    }
    // Past the left.
    if (!jumpsBelowOrEqual(fp::compare(Float(0.0), floatFrom(guest, corners))))
    {
        setFour(0x2e0, 0x280, 0xe0, 0x80, 0.0f);
        takeOut(0);
        takeOut(6);
        takeOut(7);
    }
    // Past the top.
    if (!jumpsBelowOrEqual(fp::compare(Float(0.0), floatFrom(guest, corners + 4))))
    {
        setFour(0x3a4, 0x384, 0x1a4, 0x184, 0.0f);
        takeOut(0);
        takeOut(1);
        takeOut(2);
    }
    // Past the bottom.  With nothing to do, eax has the comparison's word.
    const reg16 status = fp::compare(X87::integer(std::int32_t(height)), floatFrom(guest, corners + 0x244));
    if (jumpsAbove(status))
        return (height & 0xffff0000) | status;
    setFour(0x3e4, 0x3c4, 0x1e4, 0x1c4, float(X87::integer(std::int32_t(height - 1))));
    takeOut(4);
    takeOut(5);
    return takeOut(6);
}

namespace
{

/* ---------------------------------------------------------------------------
 * The standings' rows: sub_47ee80
 * ------------------------------------------------------------------------- */

}  // namespace

/* Edx the car looked from.  A row for each place of the race in order: the car's
 * number, name and picture, the gap to the car looked from, none for one that is out
 * (states 3 and 4), and a colour for ahead of it, it and behind; after them the
 * cars without a place that are wrecked.  Their count at 0x7254f8. */
x86::reg32 frontend::rendb::prepareStandings(App* app, CPU& cpu, reg32 from)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 0xc);
    constexpr reg32 kRowCount = 0x7254f8;
    constexpr reg32 kRowBytes = 24;
    constexpr reg32 kRowName = 0x7254fc, kRowPicture = 0x725500, kRowGap = 0x725504, kRowColour = 0x725508;
    constexpr reg32 kRowUnplaced = 0x72550c, kRowNumber = 0x725510;
    constexpr reg32 kCarPlace = 0x2bc, kCarNumber = 0x1f0, kCarRecord = 0x220, kCarFlags = 0x200, kCarState = 0x2c4;
    constexpr reg32 kNameInRecord = 0x34;
    constexpr std::uint8_t kCarOut = 1;
    constexpr reg32 kStateArrested = 3, kStateWrecked = 4;
    constexpr reg32 kPictures = 0x793098;
    constexpr reg32 kGapBetween = 0x485a60;    // eax the car behind, edx the car ahead
    constexpr reg32 kColourAhead = 0xffa01010, kColourBehind = 0xff008c00, kColourHere = 0xfff0f0f0;

    const auto outIn = [&](reg32 car, reg32 state) {
        return (guest.byte(car + kCarFlags) & kCarOut) != 0 && guest.dword(car + kCarState) == state;
    };
    const std::int32_t ownPlace = std::int32_t(guest.dword(from + kCarPlace));
    reg32 rows = 0;
    // What eax and edx hold at the end, which the check holds it to.
    reg32 leftEax = reg32(ownPlace), leftEdx = from;
    for (std::int32_t place = 1; place <= std::int32_t(guest.dword(kCarCount)); ++place)
    {
        for (std::int32_t i = 0; i < std::int32_t(guest.dword(kCarCount)); ++i)
        {
            const reg32 car = guest.dword(kCars + 4 * reg32(i));
            leftEdx = car;
            if (reg32(place) != guest.dword(car + kCarPlace))
                continue;
            const reg32 row = rows * kRowBytes;
            guest.setDword(row + kRowNumber, guest.dword(car + kCarNumber));
            guest.setDword(row + kRowName, guest.dword(car + kCarRecord) + kNameInRecord);
            guest.setDword(row + kRowUnplaced, 0);
            guest.setDword(row + kRowPicture, guest.dword(kPictures + 4 * guest.dword(car + kCarNumber)));
            if (outIn(car, kStateArrested) || outIn(car, kStateWrecked))
                guest.setDword(row + kRowGap, 0);
            else
            {
                const reg32 gap = place > ownPlace ? 0 - guest.call(kGapBetween, args({ from, car }))
                                                   : guest.call(kGapBetween, args({ car, from }));
                leftEdx = guest.edx();
                guest.setDword(row + kRowGap, gap);
            }
            leftEax = row;
            guest.setDword(row + kRowColour, place < ownPlace ? kColourAhead : place > ownPlace ? kColourBehind : kColourHere);
            ++rows;
            break;
        }
    }
    for (std::int32_t i = 0; i < std::int32_t(guest.dword(kCarCount)); ++i)
    {
        const reg32 car = guest.dword(kCars + 4 * reg32(i));
        leftEdx = car;
        if (guest.dword(car + kCarPlace) != 0 || !outIn(car, kStateWrecked))
            continue;
        const reg32 row = rows * kRowBytes;
        leftEax = rows * 3;
        guest.setDword(row + kRowNumber, guest.dword(car + kCarNumber));
        guest.setDword(row + kRowColour, kColourHere);
        guest.setDword(row + kRowName, guest.dword(car + kCarRecord) + kNameInRecord);
        guest.setDword(row + kRowGap, 0);
        guest.setDword(row + kRowUnplaced, 1);
        leftEdx = guest.dword(kPictures + 4 * guest.dword(car + kCarNumber));
        ++rows;
        guest.setDword(row + kRowPicture, leftEdx);
    }
    guest.setDword(kRowCount, rows);
    guest.leaveEdx(leftEdx);
    return leftEax;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The cop's markers: sub_47d480
 * ------------------------------------------------------------------------- */

}  // namespace

/* Eax a view, edx its car.  A cop with someone to chase and one of the view's rows
 * in pursuit (kind 7) has the markers set for that one; any other has their corners
 * taken out of the queue.  When they come or go, the view's list of what is shown
 * has their box added, or its last taken away. */
x86::reg32 frontend::rendb::updateCopMarkers(App* app, CPU& cpu, reg32 view, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 20 + 8);
    constexpr reg32 kMarkersColour = 0x8f004f00;
    constexpr reg32 kCarChased = 0x624, kCarFlags = 0x200;
    constexpr std::uint8_t kCarIsCop = 0x20;
    constexpr reg32 kRows = 0x725278, kRowsPerView = 0x140, kRowBytes = 0x28, kRowInPursuit = 7;
    constexpr reg32 kMarkersShown = 0x700230, kMarkersWereShown = 0x700238;
    constexpr reg32 kMarkerCorners = 0x791ac8;
    constexpr reg32 kBoxes = 0x749878, kBoxesPerView = 0x170;
    constexpr reg32 kSetMarkers = 0x48b5f0;    // eax a view, edx the car, ebx the one chased, ecx the colour
    constexpr reg32 kAddToList = 0x487350;

    const reg32 chased = guest.dword(car + kCarChased);
    bool anyInPursuit = false;
    for (std::int32_t i = 0; i < std::int32_t(guest.dword(kCarCount)); ++i)
        if (guest.dword(view * kRowsPerView + reg32(i) * kRowBytes + kRows) == kRowInPursuit)
            anyInPursuit = true;
    if ((guest.byte(car + kCarFlags) & kCarIsCop) != 0 && anyInPursuit && chased != 0)
        guest.setDword(kMarkersShown + 4 * view, guest.call(kSetMarkers, args({ view, car, chased, kMarkersColour })));
    else
    {
        guest::direct<0x47fcb0, unqueueEightQuads>(app, cpu, (view << 5) + guest.dword(kMarkerCorners));
        guest.setDword(kMarkersShown + 4 * view, 0);
    }
    const reg32 shown = guest.dword(kMarkersShown + 4 * view);
    guest.leaveEdx(shown);
    if (shown == guest.dword(kMarkersWereShown + 4 * view))
        return view * 4;
    guest.setDword(kMarkersWereShown + 4 * view, shown);
    if (shown == 0)
        return fe::direct<0x4873c0, frontend::hudx::groupDropLast>(app, cpu, view);
    const reg32 box = kBoxes + view * kBoxesPerView;
    return guest.call(kAddToList, args({ view, guest.dword(box + 4), guest.dword(box), guest.dword(box + 0xc) }, { guest.dword(box + 8) }));
}

namespace
{

/* ---------------------------------------------------------------------------
 * A box: sub_47d1d0
 * ------------------------------------------------------------------------- */

}  // namespace

/* Four corners in screen numbers and a colour, on the stack: filled (the ninth
 * argument nonzero) or as four lines, without a texture. */
x86::reg32 frontend::rendb::drawBox(App* app, CPU& cpu, reg32 x0, reg32 y0, reg32 x1, reg32 y1, reg32 x2, reg32 y2,
                                   reg32 x3, reg32 y3, reg32 filled, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24 + 0x80);
    const reg32 place[8] = { x0, y0, x1, y1, x2, y2, x3, y3 };
    constexpr reg32 kBlankQuad = 0x47ba70;
    constexpr reg32 kStateTexture = 1;
    const reg32 first = guest.frame(), second = first + kCornerBytes, third = first + 2 * kCornerBytes, fourth = first + 3 * kCornerBytes;
    copyWords(guest, first, kBlankQuad, kQuadBytes / 4);
    for (reg32 i = 0; i < 4; ++i)
    {
        const reg32 corner = first + i * kCornerBytes;
        guest.setDword(corner + kCornerX, place[2 * i]);
        guest.setDword(corner + kCornerY, place[2 * i + 1]);
        guest.setDword(corner + kCornerColour, colour);
    }
    for (reg32 i = 0; i < 4; ++i)
        storeClipCode(guest, first + i * kCornerBytes);
    fe::direct<0x431900, graphics::sendState>(app, cpu, kStateTexture, 0);
    if (filled != 0)
        return guest.call(kDrawQuad, args({ first, second, third, fourth }));
    guest.call(kDrawSegment, args({ first, second }));
    guest.call(kDrawSegment, args({ second, third }));
    guest.call(kDrawSegment, args({ third, fourth }));
    return guest.call(kDrawSegment, args({ fourth, first }));
}

namespace
{

/* ---------------------------------------------------------------------------
 * The mark around the car a cop chases: sub_47d580
 * ------------------------------------------------------------------------- */

/* A quad's four corners, x and y. */
struct MarkCorners
{
    float x0, y0, x1, y1, x2, y2, x3, y3;
};

/* Where the mark is: its middle on the screen, the corners' depth and w, and the
 * sizes of its parts for the screen's width. */
struct Mark
{
    reg32 quad;        // the corners, in the frame
    float x, y;        // the middle
    reg32 z, w;
    Float outer;       // the green frame's outside, from the middle
    Float inner;       // and its inside
    Float gapFrom;     // half the outside: where the gap in its top and bottom starts
    Float square;      // the inside of the square that closes on the car
    Float squareEdge;  // and its outside
};

void setMarkDepth(Guest& guest, const Mark& mark)
{
    for (reg32 i = 0; i < 4; ++i)
    {
        guest.setDword(mark.quad + i * kCornerBytes + kCornerZ, mark.z);
        guest.setDword(mark.quad + i * kCornerBytes + kCornerW, mark.w);
    }
}

/* A quad of the mark: its corners, their clip codes, drawn. */
void drawMarkQuad(Guest& guest, const Mark& mark, const MarkCorners& c)
{
    const reg32 quad = mark.quad;
    guest.setFloat(quad + kCornerX, c.x0);
    guest.setFloat(quad + kCornerY, c.y0);
    guest.setFloat(quad + kCornerBytes + kCornerX, c.x1);
    guest.setFloat(quad + kCornerBytes + kCornerY, c.y1);
    guest.setFloat(quad + 2 * kCornerBytes + kCornerX, c.x2);
    guest.setFloat(quad + 2 * kCornerBytes + kCornerY, c.y2);
    guest.setFloat(quad + 3 * kCornerBytes + kCornerX, c.x3);
    guest.setFloat(quad + 3 * kCornerBytes + kCornerY, c.y3);
    for (reg32 i = 0; i < 4; ++i)
        storeClipCode(guest, quad + i * kCornerBytes);
    guest.call(kDrawQuad, args({ quad, quad + kCornerBytes, quad + 2 * kCornerBytes, quad + 3 * kCornerBytes }));
}

/* The green frame around the car, in the blank quad's colour: its top in two parts,
 * its right side, its bottom in two parts, its left side.  Each sum and difference
 * in the original's order: the float it stores is what the next corner copies. */
void drawMarkFrame(Guest& guest, const Mark& mark)
{
    const Float x(mark.x), y(mark.y);
    const Float inner = mark.inner, outer = mark.outer, gapFrom = mark.gapFrom;
    {
        const float top = float(fp::sub(y, inner));
        const float left = float(fp::sub(x, outer));
        const float gap = float(fp::sub(x, gapFrom));
        const float bottom = float(fp::sub(y, outer));
        setMarkDepth(guest, mark);
        drawMarkQuad(guest, mark, { left, top, gap, top, gap, bottom, left, bottom });
    }
    {
        const float top = float(fp::sub(y, inner));
        const float right = float(fp::add(x, outer));
        const float gap = float(fp::add(x, gapFrom));
        const float bottom = float(fp::sub(y, outer));
        setMarkDepth(guest, mark);
        drawMarkQuad(guest, mark, { gap, top, right, top, right, bottom, gap, bottom });
    }
    {
        const float top = float(fp::sub(y, inner));
        const float right = float(fp::add(x, outer));
        const float side = float(fp::add(x, inner));
        const float bottom = float(fp::add(y, inner));
        drawMarkQuad(guest, mark, { side, top, right, top, right, bottom, side, bottom });
    }
    {
        const float gap = float(fp::sub(x, gapFrom));
        const float low = float(fp::add(y, outer));
        const float left = float(fp::sub(x, outer));
        const float high = float(fp::add(y, inner));
        drawMarkQuad(guest, mark, { gap, high, gap, low, left, low, left, high });
    }
    {
        const float right = float(fp::add(x, outer));
        const float low = float(fp::add(outer, y));
        const float gap = float(fp::add(x, gapFrom));
        const float high = float(fp::add(y, inner));
        drawMarkQuad(guest, mark, { right, high, right, low, gap, low, gap, high });
    }
    {
        const float top = float(fp::add(y, inner));
        const float left = float(fp::sub(x, outer));
        const float side = float(fp::sub(x, inner));
        const float bottom = float(fp::sub(y, inner));
        drawMarkQuad(guest, mark, { side, top, left, top, left, bottom, side, bottom });
    }
}

/* The square's four sides, in a colour. */
void drawMarkSquare(Guest& guest, const Mark& mark, reg32 colour)
{
    const Float x(mark.x), y(mark.y);
    const Float square = mark.square, edge = mark.squareEdge;
    setMarkDepth(guest, mark);
    paintCorners(guest, mark.quad, 4, colour);
    {
        const float inside = float(fp::sub(y, square));
        const float right = float(fp::add(x, square));
        const float left = float(fp::sub(x, square));
        const float outside = float(fp::sub(y, edge));
        drawMarkQuad(guest, mark, { left, inside, right, inside, right, outside, left, outside });
    }
    {
        const float top = float(fp::sub(y, square));
        const float outside = float(fp::add(x, edge));
        const float inside = float(fp::add(x, square));
        const float bottom = float(fp::add(y, square));
        drawMarkQuad(guest, mark, { inside, top, outside, top, outside, bottom, inside, bottom });
    }
    {
        const float right = float(fp::add(x, square));
        const float outside = float(fp::add(y, edge));
        const float left = float(fp::sub(x, square));
        const float inside = float(fp::add(y, square));
        drawMarkQuad(guest, mark, { right, inside, right, outside, left, outside, left, inside });
    }
    {
        const float bottom = float(fp::add(y, square));
        const float outside = float(fp::sub(x, edge));
        const float inside = float(fp::sub(x, square));
        const float top = float(fp::sub(y, square));
        drawMarkQuad(guest, mark, { inside, bottom, outside, bottom, outside, top, inside, top });
    }
}

/* The car's place to the screen as the view's camera has it.  What the mark takes
 * of it: the middle, the depth and its w. */
void projectMark(Guest& guest, Mark& mark, reg32 view, reg32 place)
{
    constexpr reg32 kViewPosition = 0x38, kViewAxes = 0x44;
    constexpr reg32 kCentreX = 0x5600a4, kCentreY = 0x5600a8, kScaleX = 0x56009c, kScaleY = 0x5600a0;
    constexpr reg32 kNearDepth = 0x37800080;

    const Float px = floatFrom(guest, place), py = floatFrom(guest, place + 4), pz = floatFrom(guest, place + 8);
    const reg32 axes = view + kViewAxes;
    const auto axis = [&](reg32 column) {
        return fp::add(fp::add(fp::mul(px, floatFrom(guest, axes + column)), fp::mul(py, floatFrom(guest, axes + 0xc + column))),
                       fp::mul(pz, floatFrom(guest, axes + 0x18 + column)));
    };
    const float turnedX = float(axis(0)), turnedY = float(axis(4)), turnedZ = float(axis(8));
    const float eyeX = float(fp::add(floatFrom(guest, view + kViewPosition), Float(turnedX)));
    const float eyeY = float(fp::add(floatFrom(guest, view + kViewPosition + 4), Float(turnedY)));
    float eyeZ = float(fp::add(floatFrom(guest, view + kViewPosition + 8), Float(turnedZ)));
    if ((bitsOf(eyeZ) & 0x7fffffff) == 0)
        eyeZ = floatOf(kNearDepth);
    const float w = float(fp::div(Float(1.0), Float(eyeZ)));
    mark.x = float(fp::add(fp::mul(fp::mul(floatFrom(guest, kScaleX), Float(w)), Float(eyeX)), floatFrom(guest, kCentreX)));
    mark.y = float(fp::add(fp::mul(fp::mul(floatFrom(guest, kScaleY), Float(w)), Float(eyeY)), floatFrom(guest, kCentreY)));
    mark.z = bitsOf(eyeZ);
    mark.w = bitsOf(w);
}

}  // namespace

/* Eax a view, edx a row of the view's list in pursuit, ebx the screen's width.  Around
 * the row's car, while it is under 300 away: a green frame, sized for the screen, and
 * inside it a square that closes on the car and goes from cyan to red as it comes
 * nearer, flashing (bit 4 of the race's clock) once close. */
x86::reg32 frontend::rendb::drawPursuitMark(App* app, CPU& cpu, reg32 view, reg32 row, reg32 screenWidth)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16 + 0xf8);
    constexpr reg32 kBlankQuad = 0x47baf0;     // green
    constexpr reg32 kViewNumber = 4, kViewEye = 8;
    constexpr reg32 kRowCar = 0x72529c, kRowsPerView = 0x140, kRowBytes = 0x28;
    constexpr reg32 kCarPlace = 0x98;
    constexpr reg32 kPlaceInFrame = 0xa0;      // the car's place, handed over by address
    constexpr reg32 kOuterSize = 0x53b42c, kInnerSize = 0x53b434, kPerWidth = 0x53b430;  // floats, 22, 20 and 1/640
    constexpr reg32 kEdgeSize = 0x53b450;      // a float, 2
    constexpr reg32 kFarthest = 0x53b438;      // a double, 300.0
    constexpr reg32 kBrightest = 0x53b440;     // a double, 255.0
    constexpr reg32 kPerFarthest = 0x53b448;   // a double, 1/300
    constexpr reg32 kHalf = 0x53b458;          // a double
    constexpr reg32 kClock = 0x7d3684;
    constexpr std::uint8_t kFlashBit = 0x10;
    constexpr std::int32_t kFlashFrom = 0xc8;
    constexpr reg32 kStateTexture = 1;
    constexpr reg32 kFlatView = 0x7cdb88, kRaceView = 0x7cdac0;

    const reg32 place = guest.frame() + kPlaceInFrame;
    const Float width = X87::integer(std::int32_t(screenWidth));
    const float widthStored = float(width);
    const Float perWidth = floatFrom(guest, kPerWidth);
    const float outer = float(fp::mul(fp::mul(width, floatFrom(guest, kOuterSize)), perWidth));
    const float inner = float(fp::mul(fp::mul(Float(widthStored), floatFrom(guest, kInnerSize)), perWidth));

    const reg32 car = guest.dword(guest.dword(view + kViewNumber) * kRowsPerView + row * kRowBytes + kRowCar);
    copyWords(guest, place, car + kCarPlace, 3);
    copyWords(guest, guest.frame(), kBlankQuad, kQuadBytes / 4);
    // The eye in edx for it, as the code loads it there: what this function leaves when it goes no further.
    guest.leaveEdx(view + kViewEye);
    const reg32 measured = fe::direct<0x4965b0, frontend::sima::quickDistance>(app, cpu, place, view + kViewEye);
    const Float distance = guest.fpu().take();

    const Float edgeSize = fp::mul(fp::mul(Float(widthStored), floatFrom(guest, kEdgeSize)), perWidth);
    const Float farthest = doubleAt(guest, kFarthest);
    const Float nearness = fp::sub(farthest, distance);
    const Float brightness = fp::mul(nearness, doubleAt(guest, kBrightest));
    const Float perFarthest = doubleAt(guest, kPerFarthest);
    const float square = float(fp::mul(fp::mul(nearness, perFarthest), Float(inner)));
    guest.fpu().push(fp::mul(perFarthest, brightness));
    guest.call(kChop);
    const std::int32_t red = fp::toInteger<sreg32>(guest.fpu().take());
    const float squareEdge = float(fp::add(edgeSize, Float(square)));
    const reg16 status = fp::compare(distance, farthest);
    // eax as the code leaves it: the distance's, which __CHP keeps, with the x87's status in ax.
    if (jumpsAbove(status))
        return (measured & 0xffff0000) | status;

    Mark mark;
    mark.quad = guest.frame();
    projectMark(guest, mark, view, place);
    mark.outer = Float(outer);
    mark.inner = Float(inner);
    mark.gapFrom = fp::mul(Float(outer), doubleAt(guest, kHalf));
    mark.square = Float(square);
    mark.squareEdge = Float(squareEdge);

    fe::direct<0x431900, graphics::sendState>(app, cpu, kStateTexture, 0);
    fe::direct<0x4bed70, frontend::loopb::useClip>(app, cpu, kFlatView);
    drawMarkFrame(guest, mark);
    if (red < kFlashFrom || (guest.byte(kClock) & kFlashBit) == 0)
    {
        const reg32 fade = 0xff - reg32(red);
        drawMarkSquare(guest, mark, 0xff000000 | (reg32(red) << 16) | (fade << 8) | fade);
    }
    return fe::direct<0x4bed70, frontend::loopb::useClip>(app, cpu, kRaceView);
}

namespace
{

/* ---------------------------------------------------------------------------
 * The road's turn in a matrix of the HUD's: sub_47f090
 * ------------------------------------------------------------------------- */

/* A number of a slice of the track: the top byte of the dword at `at`, signed,
 * from -1 to 1, as the game's fixed point has it. */
Float sliceNumber(const Guest& guest, reg32 at)
{
    constexpr reg32 kPerFixed = 0x53b460;      // a double, 1/65536
    const reg32 fixed = reg32(sreg32(guest.dword(at)) >> 24) << 9;
    return fp::mul(X87::integer(std::int32_t(fixed)), doubleAt(guest, kPerFixed));
}

/* The turn, as a share of a whole one: folded so that facing back along the road
 * counts as facing along it, and kept between 0.02 and 0.12 either way. */
float foldedTurn(const Guest& guest, Float turn)
{
    constexpr reg32 kFoldAbove = 0x53b470, kMinusHalf = 0x53b474;   // floats
    constexpr reg32 kFoldBelow = 0x53b478;     // a double
    constexpr reg32 kPlusHalf = 0x53b480;      // a float
    constexpr reg32 kLeastAbove = 0x53b484;    // a float
    constexpr reg32 kLeastBelow = 0x53b488;    // a double
    constexpr reg32 kMostAbove = 0x53b490;     // a float
    constexpr reg32 kMostBelow = 0x53b498;     // a double
    constexpr reg32 kLeast = 0x3ca3d70a, kMinusLeast = 0xbca3d70a, kMost = 0x3df5c28f, kMinusMost = 0xbdf5c28f;

    float folded = float(turn);
    if (!jumpsBelowOrEqual(fp::compare(turn, floatFrom(guest, kFoldAbove))))
        folded = float(-fp::add(Float(folded), floatFrom(guest, kMinusHalf)));
    if (!jumpsAboveOrEqual(fp::compare(Float(folded), doubleAt(guest, kFoldBelow))))
        folded = float(-fp::add(Float(folded), floatFrom(guest, kPlusHalf)));

    bool checkMost = true;
    if (!jumpsAboveOrEqual(fp::compare(Float(0.0), Float(folded)))
        && !jumpsAboveOrEqual(fp::compare(Float(folded), floatFrom(guest, kLeastAbove))))
        folded = floatOf(kLeast);
    else if (!jumpsBelow(fp::compare(Float(0.0), Float(folded)))
             && !jumpsBelowOrEqual(fp::compare(Float(folded), doubleAt(guest, kLeastBelow))))
    {
        folded = floatOf(kMinusLeast);
        checkMost = false;
    }
    if (checkMost && !jumpsBelowOrEqual(fp::compare(Float(folded), floatFrom(guest, kMostAbove))))
        folded = floatOf(kMost);
    if (!jumpsAboveOrEqual(fp::compare(Float(folded), doubleAt(guest, kMostBelow))))
        folded = floatOf(kMinusMost);
    return folded;
}

}  // namespace

/* Eax a car.  First the look the table by [0x55d2d8] asks for (look 4, applied, or
 * none).  Then where the other table by it says so: the road's turn at the car's slice
 * against the car's axes (two dot products and their angle), and the identity turned
 * by that and scaled, into 0x791a5c. */
x86::reg32 frontend::rendb::turnMatrixToRoad(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 24 + 0x98);
    constexpr reg32 kMode = 0x55d2d8;
    constexpr reg32 kModeHasLook = 0x55d304, kModeHasTurn = 0x55d2dc;
    constexpr reg32 kSelectedLook = 0x55b088, kTurnLook = 4, kFirstLook = 0;
    constexpr reg32 kCarSlice = 0x1c, kCarAxes = 0xd8;
    constexpr reg32 kSlices = 0x5dd948, kSliceBytes = 36;
    constexpr reg32 kSliceFirstVector = 0x11, kSliceSecondVector = 0x15;
    constexpr reg32 kMinusOne = 0x53b468;      // a double
    constexpr reg32 kIdentity = 0x47bb70;
    // eax, edx two matrices, ebx where to.  By address: the eax it leaves is this function's answer, and
    // its core hands back none.
    constexpr reg32 kMultiply = 0x4e0280;
    constexpr reg32 kResult = 0x791a5c;
    constexpr reg32 kResultScale = 0x3d4ccccd; // a float, 0.05
    // The frame.
    constexpr reg32 kRotated = 0, kScaled = 0x24, kStart = 0x48, kVector = 0x6c;

    const reg32 frame = guest.frame();
    if (guest.dword(kModeHasLook + 4 * guest.dword(kMode)) == 0)
        guest.setDword(kSelectedLook, kFirstLook);
    else
    {
        guest.setDword(kSelectedLook, kTurnLook);
        guest::direct<0x4774b0, frontend::renda::sparksOnScreen>(app, cpu);
    }
    const reg32 mode = guest.dword(kMode);
    if (guest.dword(kModeHasTurn + 4 * mode) == 0)
        return mode;

    const auto setVector = [&](reg32 at) {
        const reg32 slice = guest.dword(kSlices) + guest.dword(car + kCarSlice) * kSliceBytes + at;
        guest.setFloat(frame + kVector, float(sliceNumber(guest, slice)));
        guest.setFloat(frame + kVector + 4, float(sliceNumber(guest, slice + 1)));
        guest.setFloat(frame + kVector + 8, float(sliceNumber(guest, slice + 2)));
    };
    copyWords(guest, frame + kStart, kIdentity, 9);
    setVector(kSliceFirstVector);
    const Float firstDot = Float(fe::direct<0x4e01f0, vertices::dotProduct>(app, cpu, car + kCarAxes, frame + kVector));
    setVector(kSliceSecondVector);
    float against = float(firstDot);
    float along = float(Float(fe::direct<0x4e01f0, vertices::dotProduct>(app, cpu, car + kCarAxes, frame + kVector)));
    if (gameSettings(guest).backwards != 0)
    {
        const Float minusOne = doubleAt(guest, kMinusOne);
        against = float(fp::mul(Float(against), minusOne));
        along = float(fp::mul(minusOne, Float(along)));
    }
    fe::direct<0x4ea9c0, frontend::simd::angleOfFloats>(app, cpu, bitsOf(along), bitsOf(against));
    const float turn = foldedTurn(guest, guest.fpu().take());
    fe::direct<0x4eaab0, frontend::simd::rotateY>(app, cpu, frame + kStart, bitsOf(turn), frame + kRotated);
    fe::direct<0x4ea9e0, frontend::simd::rotateX>(app, cpu, frame + kStart, kResultScale, frame + kScaled);
    return guest.call(kMultiply, args({ frame + kRotated, frame + kScaled, kResult }));
}

/* ---------------------------------------------------------------------------
 * Under the game's addresses
 * ------------------------------------------------------------------------- */

void frontend::rendKindIsText(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47bc20", 0, 0, nullptr, 0 };
    const reg32 kind = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendb::kindIsText(app, cpu, kind); });
}

void frontend::rendSetUpHudVertices(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47bc40", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendb::setUpHudVertices(app, cpu); });
}

void frontend::rendDrawBox(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47d1d0", 40, 0, nullptr, 0 };
    reg32 place[8];
    for (reg32 i = 0; i < 8; ++i)
        place[i] = app->getMemory<reg32>(cpu.esp + 4 + 4 * i);
    const reg32 filled = app->getMemory<reg32>(cpu.esp + 4 + 4 * 8);
    const reg32 colour = app->getMemory<reg32>(cpu.esp + 4 + 4 * 9);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return rendb::drawBox(app, cpu, place[0], place[1], place[2], place[3], place[4], place[5], place[6], place[7],
                              filled, colour);
    });
}

void frontend::rendUpdateCopMarkers(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47d480", 0, 0, nullptr, 0, kKeepsEbxEcxEsiEdi };
    const reg32 view = cpu.eax, car = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendb::updateCopMarkers(app, cpu, view, car); });
}

void frontend::rendDrawPursuitMark(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47d580", 0, 0, nullptr, 0, kKeepsEcxEsiEdi };
    const reg32 view = cpu.eax, row = cpu.edx, width = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendb::drawPursuitMark(app, cpu, view, row, width); });
}

void frontend::rendPrepareStandings(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47ee80", 0, 0, nullptr, 0, kKeepsEbxEcxEsiEdi };
    const reg32 from = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendb::prepareStandings(app, cpu, from); });
}

void frontend::rendTurnMatrixToRoad(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47f090", 0, 0, nullptr, 0 };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendb::turnMatrixToRoad(app, cpu, car); });
}

void frontend::rendCopyRectangle(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47f2e0", 0x18, 0, nullptr, 0, kKeepsEsiEdi };
    const reg32 from = cpu.eax, to = cpu.edx, fromX = cpu.ebx, fromY = cpu.ecx;
    const auto stack = [&](reg32 i) { return app->getMemory<reg32>(cpu.esp + 4 + 4 * i); };
    const reg32 toX = stack(0), toY = stack(1), fromPitch = stack(2), toPitch = stack(3), width = stack(4), height = stack(5);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return rendb::copyRectangle(app, cpu, from, to, fromX, fromY, toX, toY, fromPitch, toPitch, width, height);
    });
}

void frontend::rendLayScalesIntoHud(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47f340", 0, 0, nullptr, 0 };
    const reg32 file = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendb::layScalesIntoHud(app, cpu, file); });
}

void frontend::rendLoadHudPictures(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47f590", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendb::loadHudPictures(app, cpu); });
}

void frontend::rendQueueEightQuads(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47fbe0", 0, 0, nullptr, 0 };
    const reg32 first = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendb::queueEightQuads(app, cpu, first); });
}

void frontend::rendUnqueueEightQuads(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47fcb0", 0, 0, nullptr, 0 };
    const reg32 first = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendb::unqueueEightQuads(app, cpu, first); });
}

void frontend::rendFitFrameToScreen(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_47fd80", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendb::fitFrameToScreen(app, cpu); });
}

}
