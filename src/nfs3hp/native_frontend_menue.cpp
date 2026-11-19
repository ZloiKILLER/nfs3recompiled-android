#include "native_frontend_menue.h"
#include "game_settings.h"
#include "native_frontend.h"
#include "native_ea_formats.h"
#include "native_ea_memory.h"
#include "native_frontend_eab.h"
#include "native_graphics.h"
#include "native_raceload.h"
#include "port_fields.h"
#include "port_shapes.h"

#include "native_watcom.h"
#include <nfs3hp.h>
#include <cstring>
#include "native_guest.h"
#include "native_vertices.h"

namespace nfs3hp
{
namespace
{

/* A call through a pointer or to a function of the game's that has no typed core: Watcom's register
 * arguments (eax, edx, ebx, ecx in turn) and the stack words as `arguments` have them, through
 * guest::pointerCall; its eax, and Terminated when the game stopped in it. */
nfs::u32 callAt(fe::Guest& guest, nfs::u32 target, const fe::Arguments& arguments = fe::Arguments())
{
    nfs::ThreadState& ts = guest.ts();
    nfs::Args registers;
    nfs::u32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    const nfs::u32 esp = ts.sp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        ts.sp -= 4;
        guest.setDword(ts.sp, arguments.stack[i]);
    }
    const std::optional<nfs::u32> result = guest::pointerCall(guest.app(), ts, target, registers);
    if (!result)
        throw fe::Terminated();
    ts.sp = esp;
    return *result;
}

}  // namespace
}  // namespace nfs3hp

namespace nfs3hp
{
namespace
{

/* The game's float operations on plain values: fild, the single-precision sum, difference and
 * product (fp::), what jb and ja read after fcom, and the truncation of sub_4dfd56 and fistp. */
struct FloatMath
{
    explicit FloatMath(fe::Guest& guest) : m_guest(guest) {}

    static nfs::Float integer(std::int32_t value) { return nfs::Float(nfs::s32(value)); }
    nfs::Float load(nfs::u32 address) const { return nfs::Float(m_guest.floatAt(address)); }
    nfs::Float add(const nfs::Float& a, const nfs::Float& b) { return fp::add(a, b); }
    nfs::Float sub(const nfs::Float& a, const nfs::Float& b) { return fp::sub(a, b); }
    nfs::Float mul(const nfs::Float& a, const nfs::Float& b) { return fp::mul(a, b); }
    bool above(const nfs::Float& a, const nfs::Float& b) { return a > b; }
    bool below(const nfs::Float& a, const nfs::Float& b) { return !(a >= b); }  // or unordered
    std::int32_t truncated(const nfs::Float& value)
    {
        const double chopped = fe::direct<0x4dfd56, watcom::clib::chop>(m_guest.app(), m_guest.ts(), double(value));
        return fp::toInteger<nfs::s32>(chopped);
    }

private:
    fe::Guest& m_guest;
};

}  // namespace
}  // namespace nfs3hp

/* The small things the menus' widgets lean on that the linker put far from
 * them, among the movies' stream and EA's libraries, decompiled:
 *
 *   the sheets' text      a speed and a time as the results and records show them;
 *   the laps' option      2, 4 or 8 laps and their place in the list;
 *   the language's text   its block given back as the game quits;
 *   the menus' pointer    its pictures loaded, its hot spot;
 *   the menus' drawing    an art record's tiles (the widget at 0x4549c0), the
 *                         bouncing pictures' quad and its fade (the menus'
 *                         effects, render::bouncers), the colour picker's
 *                         triangle, the effects' band colour.
 *
 * What they call -- the 2D layer, the render state, sprintf, EA's bitmaps -- the
 * typed cores as C++, the rest by address.
 *
 * Plan step 5: each function a typed core in frontend::menue (native_frontend_menue.h), in a
 * frame of its own where the game's function has one (fe::Frame: what it calls runs at the stack
 * addresses it did), and a thunk under the game's name for the calls by address.  A core of
 * the menu files, the port's or a typed library's is called as C++ (guest::direct, fe::direct);
 * the rest of the game by address, and so is a call whose registers the code goes on with. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using fe::GuestString;
using nfs::Float;
using nfs::u32;
using nfs::s32;

constexpr u32 kSprintf = 0x4df690;

constexpr std::uint8_t kPushedEsiEdi = fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kPushedEcxEdx = fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kPushedEcxEsiEdi = fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kPushedEbxEcxEsiEdi = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;

/* A game function's stack argument, by its place, before its frame. */
u32 stackArgument(nfs::App* app, const nfs::ThreadState& ts, u32 place)
{
    return app->getMemory<u32>(ts.sp + 4 + 4 * place);
}

/* A double of the game's constants, as fmul qword reads it. */
Float doubleAt(const Guest& guest, u32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* What a jb after an x87 comparison sees: below, or unordered. */
bool belowOrUnordered(const Float& a, const Float& b)
{
    return (fp::compare(a, b) & 0x100) != 0;
}

/* A record's int16, sign extended, as the game reads it: a dword, then sar 16. */
std::int32_t int16At(const Guest& guest, u32 address)
{
    return std::int16_t(guest.word(address));
}

/* ---------------------------------------------------------------------------
 * The sheets' text
 * ------------------------------------------------------------------------- */

constexpr std::uint32_t kKilometresA = 3, kKilometresB = 4;
constexpr u32 kToKilometres = 0x5404d8;      // float, 3.6
constexpr u32 kToMiles = 0x5404dc;           // float, 2.2369
constexpr u32 kSpeedFormat = 0x5404cc;       // "%5.2f"
constexpr GuestString kNoSpeed{ 0x5404d4, "---" };

}  // namespace

/* sub_4cedd0: a speed in metres per second into `buffer`, in the player's
 * units; "---" for 1 or less. */
nfs::u32 frontend::menue::metricText(App* app, ThreadState& ts, float speed, u32 buffer)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    if (!belowOrUnordered(Float(1.0), Float(speed)))
    {
        return fe::direct<kSprintf, watcom::clib::sprintf>(app, ts, buffer, kNoSpeed.address, guest::Words({}));
    }
    const std::uint32_t units = gameSettings(guest).hud[0].speedometerForm;
    const u32 toUnits = units == kKilometresA || units == kKilometresB ? kToKilometres : kToMiles;
    const double shown = fp::mul(Float(speed), Float(guest.floatAt(toUnits)));
    std::uint64_t bits;
    std::memcpy(&bits, &shown, sizeof bits);
    return fe::direct<kSprintf, watcom::clib::sprintf>(app, ts, buffer, kSpeedFormat,
                                                       guest::Words{u32(bits), u32(bits >> 32)});
}

namespace
{

constexpr u32 kToHundredths = 0x5404f8;      // double, 100/64: the game's times are in 64ths of a second
constexpr GuestString kTimeFormat{ 0x5404e0, "%01d%c%02d%c%02d" };
constexpr u32 kMinuteMarks = 0x561b28;       // a language's mark after the minutes (bytes)
constexpr u32 kSecondMarks = 0x561b2e;       // and after the seconds
constexpr std::int32_t kHundredthsAMinute = 6000;
constexpr std::int32_t kHundredthsASecond = 100;

}  // namespace

/* sub_4cee40: a time into `buffer` as minutes, seconds and hundredths with the
 * language's marks.  The parts go through 16 bits, as the game takes them. */
nfs::u32 frontend::menue::timeText(App* app, ThreadState& ts, u32 time, u32 buffer)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 28);
    FloatMath x87(guest);
    const std::int32_t hundredths = x87.truncated(fp::mul(FloatMath::integer(std::int32_t(time)), doubleAt(guest, kToHundredths)));
    const std::int32_t minutes = std::int16_t(hundredths / kHundredthsAMinute);
    const std::int32_t pastMinute = std::int32_t(u32(hundredths) - u32(minutes) * u32(kHundredthsAMinute));
    const std::int32_t seconds = std::int16_t(pastMinute / kHundredthsASecond);
    const std::int32_t rest = std::int16_t(u32(pastMinute) - u32(seconds) * u32(kHundredthsASecond));
    const u32 language = gameSettings(guest).language;
    const u32 secondMark = guest.byte(language + kSecondMarks);
    const u32 minuteMark = guest.byte(language + kMinuteMarks);
    return fe::direct<kSprintf, watcom::clib::sprintf>(
        app, ts, buffer, kTimeFormat.address,
        guest::Words{u32(minutes), minuteMark, u32(seconds), secondMark, u32(rest)});
}

/* ---------------------------------------------------------------------------
 * The laps' option
 * ------------------------------------------------------------------------- */

/* sub_4cf160: 2 laps the list's first place (al cleared, the rest of eax as it
 * came), 4 the second, others the third. */
nfs::u32 frontend::menue::lapsToOption(App*, ThreadState&, u32 laps)
{
    const std::uint16_t count = std::uint16_t(laps);
    if (count == 2)
        return laps & ~0xffu;
    return count == 4 ? 1 : 2;
}

/* sub_4cf190: the laps of a place in the list. */
nfs::u32 frontend::menue::optionToLaps(App*, ThreadState&, u32 option)
{
    const std::uint16_t place = std::uint16_t(option);
    if (place == 0)
        return 2;
    return place == 1 ? 4 : 8;
}

/* ---------------------------------------------------------------------------
 * The language's text
 * ------------------------------------------------------------------------- */

namespace
{

constexpr u32 kLanguageTexts = 0x563a80;     // its block, once loaded

}  // namespace

/* sub_4d1880: the language's block given back. */
nfs::u32 frontend::menue::freeLanguageTexts(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const u32 texts = guest.dword(kLanguageTexts);
    u32 result = eax;
    if (texts != 0)
        result = *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, texts);
    guest.setDword(kLanguageTexts, 0);
    return result;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The 2D screens' quad
 * ------------------------------------------------------------------------- */

/* The four corners the 2D layer draws from, 0x20 bytes each. */
constexpr u32 kQuad = 0x563ab0;
constexpr u32 kCornerSize = 0x20;
constexpr u32 kCornerX = 0x00, kCornerY = 0x04, kCornerZ = 0x08, kCornerW = 0x0c;
constexpr u32 kCornerColour = 0x10, kCornerClip = 0x14, kCornerU = 0x18, kCornerV = 0x1c;
constexpr u32 kDepthZ = 0x563aa8, kDepthW = 0x563aac;   // floats: the 2D layer's depth

constexpr u32 corner(u32 index)
{
    return kQuad + kCornerSize * index;
}

constexpr u32 kOneBits = 0x3f800000;  // 1.0f

// The window the corners are clipped to: floats, which the game compares as
// integers, as it may positive ones.
constexpr u32 kClipW = 0x7d34f8, kClipLeft = 0x7d34fc, kClipBottom = 0x7d3500, kClipTop = 0x7d3504, kClipRight = 0x7d350c;
constexpr std::uint8_t kOffLeft = 1, kOffRight = 2, kOffBottom = 4, kOffTop = 8, kBehind = 0x10;

constexpr u32 kRenderStateTexture = 1;
constexpr u32 kClipQuad = 0x4c12e0;          // the four corners' addresses

/* A corner's outcode, as the game works it out in line for each corner before
 * the clipper. */
std::uint8_t clipCode(const Guest& guest, u32 at)
{
    const std::int32_t w = s32(guest.dword(at + kCornerW));
    if (w < 0 || w >= s32(guest.dword(kClipW)))
        return kBehind;
    std::uint8_t code = 0;
    const std::int32_t y = s32(guest.dword(at + kCornerY));
    if (y < 0 || y < s32(guest.dword(kClipTop)))
        code |= kOffTop;
    else if (y > s32(guest.dword(kClipBottom)))
        code |= kOffBottom;
    const std::int32_t x = s32(guest.dword(at + kCornerX));
    if (x < 0 || x < s32(guest.dword(kClipLeft)))
        code |= kOffLeft;
    else if (x > s32(guest.dword(kClipRight)))
        code |= kOffRight;
    return code;
}

/* Each corner's outcode, then the quad to the clipper. */
u32 clipAndDrawQuad(Guest& guest)
{
    for (u32 i = 0; i < 4; ++i)
        guest.setByte(corner(i) + kCornerClip, clipCode(guest, corner(i)));
    return fe::direct<kClipQuad, vertices::quadClipped>(guest.app(), guest.ts(), corner(0), corner(1), corner(2), corner(3));
}

/* The quad's corners, clockwise from the top left. */
void placeCorners(Guest& guest, float left, float top, float right, float bottom)
{
    guest.setFloat(corner(0) + kCornerX, left);
    guest.setFloat(corner(0) + kCornerY, top);
    guest.setFloat(corner(1) + kCornerX, right);
    guest.setFloat(corner(1) + kCornerY, top);
    guest.setFloat(corner(2) + kCornerX, right);
    guest.setFloat(corner(2) + kCornerY, bottom);
    guest.setFloat(corner(3) + kCornerX, left);
    guest.setFloat(corner(3) + kCornerY, bottom);
}

/* The depth, the 2D layer's, on all four. */
void placeDepth(Guest& guest)
{
    const float z = float(Float(guest.floatAt(kDepthZ)));
    const float w = float(Float(guest.floatAt(kDepthW)));
    for (u32 i = 0; i < 4; ++i)
    {
        guest.setFloat(corner(i) + kCornerZ, z);
        guest.setFloat(corner(i) + kCornerW, w);
    }
}

/* The corners' texture coordinates, floats' bits, the corners in order. */
void placeTexture(Guest& guest, u32 u0, u32 v0, u32 u1, u32 v1, u32 u2, u32 v2, u32 u3, u32 v3)
{
    guest.setDword(corner(0) + kCornerU, u0);
    guest.setDword(corner(0) + kCornerV, v0);
    guest.setDword(corner(1) + kCornerU, u1);
    guest.setDword(corner(1) + kCornerV, v1);
    guest.setDword(corner(2) + kCornerU, u2);
    guest.setDword(corner(2) + kCornerV, v2);
    guest.setDword(corner(3) + kCornerU, u3);
    guest.setDword(corner(3) + kCornerV, v3);
}

/* What both drawings leave in the corners' coordinates: the whole texture,
 * the corners in order. */
void restoreTexture(Guest& guest)
{
    placeTexture(guest, 0, 0, kOneBits, 0, kOneBits, kOneBits, 0, kOneBits);
}

/* ---------------------------------------------------------------------------
 * An art record's tiles
 * ------------------------------------------------------------------------- */

constexpr u32 kDisplayFlags = 0x7a3a58;      // byte
constexpr std::uint8_t kNoTranslucency = 0x02;
constexpr u32 kInRace = 0x559234;

// An art record, 20 bytes.
constexpr u32 kArtRecords = 0x8b48bc;
constexpr u32 kArtSize = 0x14;
constexpr u32 kArtX = 0x04;                  // int16
constexpr u32 kArtFirstTile = 0x10;          // int16; -1 when not loaded
constexpr u32 kArtEndTile = 0x12;            // int16: past its last
// A record's tile, 12 bytes.
constexpr u32 kTiles = 0x8b5090;
constexpr u32 kTileSize = 0x0c;
constexpr u32 kTileTexture = 0x00;
constexpr u32 kTileSide = 0x04;              // int16: its square's side, in pixels
constexpr u32 kTileColumn = 0x06;            // int16: counted leftwards from the record's x
constexpr u32 kTileRow = 0x08;               // int16

constexpr std::int32_t kTilePitch = 32;
constexpr u32 kNotLoadedLog = 0x42d500;
constexpr u32 kNotLoadedFormat = 0x549630;   // "ERROR: shape %d not loaded in texture po..."

}  // namespace

/* sub_4d7560: art record `art` at (x, y), a textured square white for each of
 * its tiles, with the whole texture on it; nothing in a race or without
 * translucency. */
nfs::u32 frontend::menue::drawArtTiles(App* app, ThreadState& ts, u32 art, std::int32_t x, std::int32_t y)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16 + 0x1c);
    if ((guest.byte(kDisplayFlags) & kNoTranslucency) != 0 || guest.dword(kInRace) != 0)
        return art;  // eax as it came: the art
    const u32 record = kArtRecords + kArtSize * art;
    const std::int32_t first = int16At(guest, record + kArtFirstTile);
    if (first == -1)
    {
        // By address: the log's hook takes nothing, but the game's code pushes its format and the record.
        fe::direct<0x42d500, raceload::noHook>(app, ts);
        return *fe::direct<0x4f0120, ea::waitKey>(app, ts);
    }
    placeDepth(guest);
    for (u32 i = 0; i < 4; ++i)
        guest.setDword(corner(i) + kCornerColour, 0xffffffffu);

    const std::int32_t end = int16At(guest, record + kArtEndTile);
    for (std::int32_t tile = first; tile < end; ++tile)
    {
        const u32 at = kTiles + kTileSize * u32(tile);
        placeTexture(guest, kOneBits, 0, 0, 0, 0, kOneBits, kOneBits, kOneBits);
        const Float side = FloatMath::integer(int16At(guest, at + kTileSide));
        const Float column = FloatMath::integer(std::int32_t(u32(int16At(guest, at + kTileColumn)) * kTilePitch + kTilePitch));
        const Float place = FloatMath::integer(std::int32_t(u32(int16At(guest, record + kArtX)) + u32(x)));
        const Float left = fp::sub(place, column);
        const float top = float(FloatMath::integer(std::int32_t(u32(int16At(guest, at + kTileRow)) * kTilePitch + u32(y))));
        const Float right = fp::add(left, side);
        const Float bottom = fp::add(side, Float(top));
        placeCorners(guest, float(left), top, float(right), float(bottom));
        fe::direct<0x431900, graphics::sendState>(app, ts, kRenderStateTexture, guest.dword(at + kTileTexture));
        clipAndDrawQuad(guest);
    }
    restoreTexture(guest);
    return u32(end);
}

namespace
{

/* ---------------------------------------------------------------------------
 * The bouncing pictures
 * ------------------------------------------------------------------------- */

// A corner's fade: full in the screen's middle, less with its distance on each axis.
constexpr u32 kFadeCentreX = 0x549684, kFadeCentreY = 0x5496a4;   // floats: -395, -225
constexpr u32 kFadeFull = 0x549688;          // float, 255
constexpr u32 kFadePerX = 0x54968c, kFadePerY = 0x5496a8;         // floats: 1/490, 1/450
constexpr u32 kFadeStrength = 0x549694;      // double, 1.5
constexpr u32 kFadeTop = 0x54969c;           // double, 255

/* A corner's distance on an axis, through a float in memory as the game takes
 * it: as it is above 0 (and when unordered), else its sign changed. */
float fadeDistance(const Guest& guest, float value, u32 centre)
{
    const float distance = float(fp::add(Float(value), Float(guest.floatAt(centre))));
    return belowOrUnordered(Float(0.0), Float(distance)) ? distance : float(-Float(distance));
}

/* What that distance takes off. */
Float fadeAlong(const Guest& guest, float distance, u32 perUnit)
{
    Float share = fp::mul(Float(distance), Float(guest.floatAt(kFadeFull)));
    share = fp::mul(share, Float(guest.floatAt(perUnit)));
    return fp::mul(share, doubleAt(guest, kFadeStrength));
}

}  // namespace

/* sub_4d7cc0 (stdcall): the top byte of the colour of a corner at (x, y), of a
 * picture of `alpha`. */
nfs::u32 frontend::menue::cornerFade(App* app, ThreadState& ts, float x, float y, u32 alpha)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x1c);
    FloatMath x87(guest);
    const float awayX = fadeDistance(guest, x, kFadeCentreX);
    const Float afterX = fp::sub(doubleAt(guest, kFadeTop), fadeAlong(guest, awayX, kFadePerX));
    const float awayY = fadeDistance(guest, y, kFadeCentreY);
    std::int32_t level = x87.truncated(fp::sub(afterX, fadeAlong(guest, awayY, kFadePerY)));
    if (level < 0)
        level = 0;
    const std::int32_t scaled = std::int32_t(alpha * u32(level)) / 255;
    return u32(scaled) << 24;
}

namespace
{

constexpr u32 kTexelSize = 0x5496ac;         // float, 1/256

struct Bouncer
{
    u32 texture;
    std::int32_t x, y, u, v, width, height;
    float scale;
    u32 colour;
};

}  // namespace

/* sub_4d7d90 (stdcall 0x14): a stretch of a texture drawn scaled at (x, y), the
 * colour its tint and its top byte the alpha, each corner fading by its place
 * on the screen. */
nfs::u32 frontend::menue::drawBouncer(App* app, ThreadState& ts, u32 texture, std::int32_t x, std::int32_t y,
                                        std::int32_t fromU, std::int32_t fromV, std::int32_t width, std::int32_t height,
                                        float scale, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12 + 0x18);
    const Bouncer b{ texture, x, y, fromU, fromV, width, height, scale, colour };
    const Float widthValue = FloatMath::integer(b.width);
    const Float heightValue = FloatMath::integer(b.height);
    const Float scaledWidth = fp::mul(widthValue, Float(b.scale));
    const Float scaledHeight = fp::mul(Float(b.scale), heightValue);

    placeDepth(guest);
    const float left = float(FloatMath::integer(b.x));
    const float top = float(FloatMath::integer(b.y));
    const Float right = fp::add(scaledWidth, Float(left));
    const Float bottom = fp::add(scaledHeight, Float(top));
    placeCorners(guest, left, top, float(right), float(bottom));

    const u32 alpha = b.colour >> 24;
    const u32 tint = b.colour & 0xffffff;
    for (u32 i = 0; i < 4; ++i)
    {
        const u32 fade = guest::direct<0x4d7cc0, menue::cornerFade>(app, ts, guest.floatAt(corner(i) + kCornerX),
                                                                      guest.floatAt(corner(i) + kCornerY), alpha);
        guest.setDword(corner(i) + kCornerColour, tint + fade);
    }

    const Float texel = Float(guest.floatAt(kTexelSize));
    const Float u = FloatMath::integer(b.u);
    const Float v = FloatMath::integer(b.v);
    const Float uFrom = fp::mul(u, texel);
    const Float uTo = fp::mul(fp::add(u, Float(float(widthValue))), texel);
    const Float vFrom = fp::mul(v, texel);
    const Float vTo = fp::mul(texel, fp::add(v, Float(float(heightValue))));
    const auto bitsOf = [](Float value) {
        const float single = float(value);
        u32 bits;
        std::memcpy(&bits, &single, sizeof bits);
        return bits;
    };
    placeTexture(guest, bitsOf(uFrom), bitsOf(vFrom), bitsOf(uTo), bitsOf(vFrom), bitsOf(uTo), bitsOf(vTo), bitsOf(uFrom), bitsOf(vTo));

    fe::direct<0x431900, graphics::sendState>(app, ts, kRenderStateTexture, b.texture);
    const u32 drawn = clipAndDrawQuad(guest);
    restoreTexture(guest);
    return drawn;
}

/* ---------------------------------------------------------------------------
 * The effects' band
 * ------------------------------------------------------------------------- */

/* sub_4d9180: a colour's red, green and blue scaled by `scale`/256, its alpha as
 * it is; a quarter by masks, the whole not at all.  edx is left the sum before
 * the blue. */
nfs::u32 frontend::menue::scaleColour(App* app, ThreadState& ts, u32 colour, std::int32_t scale)
{
    Guest guest(app, ts);
    constexpr std::int32_t kWhole = 0x100, kQuarter = 0x40;
    if (scale == kWhole)
        return colour;
    const u32 alpha = colour & 0xff000000;
    u32 alphaRedGreen, blue;
    if (scale == kQuarter)
    {
        alphaRedGreen = ((colour & 0xfc0000) >> 2) + alpha + ((colour & 0xfc00) >> 2);
        blue = (colour & 0xfc) >> 2;
    }
    else
    {
        const auto scaled = [scale](u32 component) {
            return u32(std::int32_t((component & 0xff) * u32(scale)) / 256);
        };
        alphaRedGreen = alpha + (scaled(colour >> 16) << 16) + (scaled(colour >> 8) << 8);
        blue = scaled(colour);
    }
    return blue + alphaRedGreen;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The menus' pointer
 * ------------------------------------------------------------------------- */

constexpr u32 kPointerKind = 0x563d3c;       // -2 none (port_draw2d.cpp)
constexpr u32 kPointerTextures = 0x563b34;   // 13 kinds of 10 frames
constexpr u32 kPointerTexturesPerKind = 0x28;
constexpr u32 kHotSpots = 0x563d4c;          // x, y for each frame of each kind
constexpr u32 kHotSpotsPerKind = 0x50;
constexpr u32 kHotSpotSize = 8;
constexpr std::int32_t kPointerKinds = 13, kPointerFrames = 10;
constexpr u32 kShapeCentreX = 0x08, kShapeCentreY = 0x0a;   // int16 of a shape
constexpr GuestString kPointerName{ 0x5496dc, "c%02d%d" };

constexpr u32 kFindShape = 0x4ef270;         // eax a file, edx a name
constexpr u32 kPointerSide = 0x20;

constexpr u32 kDrawShadedTriangle = 0x4d8570;

}  // namespace

/* sub_4daa10: the pointer's shapes, c<kind><frame>, out of their file: for each
 * there is its hot spot, and a texture.  A bitmap of 32 by 32 is made for the
 * time of it. */
nfs::u32 frontend::menue::loadPointer(App* app, ThreadState& ts, u32 file)
{
    Guest guest(app, ts);
    const fe::Frame stackFrame(guest, ts, 24 + 0x48);
    const u32 name = guest.frame();
    const u32 canvas =
        fe::direct<0x4ef190, eab::windowBitmapCreateDefault>(app, ts, kPointerSide, kPointerSide, kPointerSide, 0);
    for (std::int32_t kind = 0; kind < kPointerKinds; ++kind)
    {
        for (std::int32_t frame = 0; frame < kPointerFrames; ++frame)
        {
            // eax, loaded with the name for it by the game's code, is the result's again after it.
            fe::direct<kSprintf, watcom::clib::sprintf>(app, ts, name, kPointerName.address,
                                                        guest::Words{u32(kind), u32(frame)});
            const u32 shape = callAt(guest, kFindShape, args({ file, name }));
            if (shape == 0)
                continue;
            const u32 hotSpot = kHotSpots + kHotSpotsPerKind * u32(kind) + kHotSpotSize * u32(frame);
            guest.setDword(hotSpot, u32(int16At(guest, shape + kShapeCentreX)));
            guest.setDword(hotSpot + 4, u32(int16At(guest, shape + kShapeCentreY)));
            fe::direct<0x4d6070, port::shapes::textureOf>(
                app, ts, ts.sp - 4, kPointerTextures + kPointerTexturesPerKind * u32(kind) + 4 * u32(frame), shape,
                std::int32_t(kPointerSide), 0);
        }
    }
    return fe::direct<0x4eff00, eab::windowBitmapFree>(app, ts, canvas);
}

/* sub_4dab10: the shown pointer's hot spot into [xAt] and [yAt].  In a race the
 * two the code holds, for kind 0 and for kinds 8 to 11, the others left as they
 * are; in the menus its first frame's, 0 for a kind out of the table. */
nfs::u32 frontend::menue::pointerHotSpot(App* app, ThreadState& ts, u32 xAt, u32 yAt)
{
    Guest guest(app, ts);
    constexpr u32 kRaceFirstKind = 0, kRaceOthersFirst = 8, kRaceOthersLast = 11;
    const u32 kind = guest.dword(kPointerKind);
    u32 result = xAt;  // eax as it came
    if (guest.dword(kInRace) != 0)
    {
        if (kind == kRaceFirstKind)
        {
            guest.setDword(xAt, 7);
            guest.setDword(yAt, 1);
        }
        else if (kind >= kRaceOthersFirst && kind <= kRaceOthersLast)
        {
            guest.setDword(xAt, 0x10);
            guest.setDword(yAt, 0x10);
        }
    }
    else if (s32(kind) < 0 || s32(kind) >= kPointerKinds)
    {
        guest.setDword(xAt, 0);
        guest.setDword(yAt, 0);
    }
    else
    {
        const u32 spot = kHotSpots + kHotSpotsPerKind * kind;
        guest.setDword(xAt, guest.dword(spot));
        result = guest.dword(spot + 4);
        guest.setDword(yAt, result);
    }
    guest.setDword(kPointerKind, kind);
    return result;
}

/* sub_4d88c0 (stdcall 0x14): the colour picker's triangle (sub_43e310) handed on
 * to the 2D layer's as it came, its registers and the five on the stack. */
nfs::u32 frontend::menue::shadedTriangle(App* app, ThreadState& ts, u32 x0, u32 y0, u32 x1, u32 y1, u32 x2,
                                           u32 y2, u32 colour0, u32 colour1, u32 colour2)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
        // The stack word x2 is pushed under the call's five, where the core finds it (and rewrites it).
    const u32 x2At = ts.sp - 4 * 5;
    guest.setDword(x2At, x2);
    return u32(*fe::direct<kDrawShadedTriangle, port::fields::triangle>(
        app, ts, ts.sp - 4 - 4 * 5, std::int32_t(x0), std::int32_t(y0), std::int32_t(x1), std::int32_t(y1), x2At,
        std::int32_t(y2), colour0, colour1, colour2));
}

/* ---------------------------------------------------------------------------
 * The functions
 * ------------------------------------------------------------------------- */

nfs::u32 frontend::resultsMetricText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static const GuestString kStrings[] = { kNoSpeed };
    static fe::Site site{ "sub_4cedd0", 8, 0, kStrings, 1 };
    const float speed = app->getMemory<float>(ts.sp + 4);
    const u32 buffer = stackArgument(app, ts, 1);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menue::metricText(app, ts, speed, buffer); });
}

nfs::u32 frontend::resultsTimeText(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static const GuestString kStrings[] = { kTimeFormat };
    static fe::Site site{ "sub_4cee40", 0, 0, kStrings, 1, kPushedEbxEcxEsiEdi };
    const u32 time = args.eax, buffer = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menue::timeText(app, ts, time, buffer); });
}

nfs::u32 frontend::lapsToOption(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf160", 0, 0, nullptr, 0 };
    const u32 laps = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menue::lapsToOption(app, ts, laps); });
}

nfs::u32 frontend::optionToLaps(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4cf190", 0, 0, nullptr, 0 };
    const u32 option = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menue::optionToLaps(app, ts, option); });
}

nfs::u32 frontend::languageTextsFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d1880", 0, 0, nullptr, 0, kPushedEcxEdx };
    const nfs::u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menue::freeLanguageTexts(app, ts, eax); });
}

nfs::u32 frontend::artTilesDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d7560", 0, 0, nullptr, 0, kPushedEcxEsiEdi };
    const u32 art = args.eax;
    const std::int32_t x = std::int32_t(args.edx), y = std::int32_t(args.ebx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menue::drawArtTiles(app, ts, art, x, y); });
}

nfs::u32 frontend::bouncerCornerFade(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d7cc0", 12, 0, nullptr, 0, kPushedEcxEdx };
    const float x = app->getMemory<float>(ts.sp + 4), y = app->getMemory<float>(ts.sp + 8);
    const u32 alpha = stackArgument(app, ts, 2);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menue::cornerFade(app, ts, x, y, alpha); });
}

nfs::u32 frontend::bouncerDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d7d90", 0x14, 0, nullptr, 0, kPushedEsiEdi };
    const Bouncer bouncer{ args.eax,
                           std::int32_t(args.edx),
                           std::int32_t(args.ebx),
                           std::int32_t(args.ecx),
                           std::int32_t(stackArgument(app, ts, 0)),
                           std::int32_t(stackArgument(app, ts, 1)),
                           std::int32_t(stackArgument(app, ts, 2)),
                           app->getMemory<float>(ts.sp + 4 + 4 * 3),
                           stackArgument(app, ts, 4) };
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return menue::drawBouncer(app, ts, bouncer.texture, bouncer.x, bouncer.y, bouncer.u, bouncer.v, bouncer.width,
                                  bouncer.height, bouncer.scale, bouncer.colour);
    });
}

nfs::u32 frontend::pickerShadedTriangle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d88c0", 0x14, 0, nullptr, 0, kPushedEsiEdi };
    const u32 x0 = args.eax, y0 = args.edx, x1 = args.ebx, y1 = args.ecx;
    const u32 x2 = stackArgument(app, ts, 0), y2 = stackArgument(app, ts, 1);
    const u32 colour0 = stackArgument(app, ts, 2), colour1 = stackArgument(app, ts, 3), colour2 = stackArgument(app, ts, 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return menue::shadedTriangle(app, ts, x0, y0, x1, y1, x2, y2, colour0, colour1, colour2);
    });
}

nfs::u32 frontend::effectsBandColourScaled(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d9180", 0, 0, nullptr, 0, kPushedEbxEcxEsiEdi };
    const u32 colour = args.eax;
    const std::int32_t scale = std::int32_t(args.edx);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menue::scaleColour(app, ts, colour, scale); });
}

nfs::u32 frontend::menuPointerLoad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static const GuestString kStrings[] = { kPointerName };
    static fe::Site site{ "sub_4daa10", 0, 0, kStrings, 1 };
    const u32 file = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menue::loadPointer(app, ts, file); });
}

nfs::u32 frontend::menuPointerHotSpot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4dab10", 0, 0, nullptr, 0, kPushedEbxEcxEsiEdi };
    const u32 xAt = args.eax, yAt = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return menue::pointerHotSpot(app, ts, xAt, yAt); });
}

}
