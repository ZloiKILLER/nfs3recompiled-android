#include "native_frontend.h"
#include <nfs3hp.h>
#include <cstring>

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
 * What they call -- the 2D layer, sprintf, EA's bitmaps -- is called by address. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using fe::GuestString;
using fe::X87;
using x86::Float;
using x86::reg32;
using x86::sreg32;

constexpr reg32 kSprintf = 0x4df690;

constexpr std::uint8_t kPushedEsiEdi = fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kPushedEcxEdx = fe::kRestoreEcx | fe::kRestoreEdx;
constexpr std::uint8_t kPushedEcxEsiEdi = fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;
constexpr std::uint8_t kPushedEbxEcxEsiEdi = fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi;

/* A generated function's stack argument, by its place, before its frame. */
reg32 stackArgument(win32::WinApplication* app, const x86::CPU& cpu, reg32 place)
{
    return app->getMemory<reg32>(cpu.esp + 4 + 4 * place);
}

/* A double of the game's constants, as fmul qword reads it. */
Float doubleAt(const Guest& guest, reg32 address)
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
std::int32_t int16At(const Guest& guest, reg32 address)
{
    return std::int16_t(guest.word(address));
}

/* ---------------------------------------------------------------------------
 * The sheets' text
 * ------------------------------------------------------------------------- */

constexpr reg32 kSpeedUnits = 0x6fbc68;
constexpr std::uint32_t kKilometresA = 3, kKilometresB = 4;
constexpr reg32 kToKilometres = 0x5404d8;      // float, 3.6
constexpr reg32 kToMiles = 0x5404dc;           // float, 2.2369
constexpr reg32 kSpeedFormat = 0x5404cc;       // "%5.2f"; the check puts its own there, to see the double
constexpr GuestString kNoSpeed{ 0x5404d4, "---" };

/* sub_4cedd0: a speed in metres per second into `buffer`, in the player's
 * units; "---" for 1 or less. */
reg32 metricTextBody(Guest& guest, Float speed, reg32 buffer)
{
    if (!belowOrUnordered(Float(1.0), speed))
    {
        guest.call(kSprintf, args({}, { buffer, kNoSpeed.address }));
        return guest.eax();
    }
    const std::uint32_t units = guest.dword(kSpeedUnits);
    const reg32 toUnits = units == kKilometresA || units == kKilometresB ? kToKilometres : kToMiles;
    const double shown = fp::mul(speed, Float(guest.floatAt(toUnits)));
    std::uint64_t bits;
    std::memcpy(&bits, &shown, sizeof bits);
    guest.call(kSprintf, args({}, { buffer, kSpeedFormat, reg32(bits), reg32(bits >> 32) }));
    return guest.eax();
}

constexpr reg32 kToHundredths = 0x5404f8;      // double, 100/64: the game's times are in 64ths of a second
constexpr GuestString kTimeFormat{ 0x5404e0, "%01d%c%02d%c%02d" };
constexpr reg32 kLanguage = 0x6fbb4c;
constexpr reg32 kMinuteMarks = 0x561b28;       // a language's mark after the minutes (bytes)
constexpr reg32 kSecondMarks = 0x561b2e;       // and after the seconds
constexpr std::int32_t kHundredthsAMinute = 6000;
constexpr std::int32_t kHundredthsASecond = 100;

/* sub_4cee40: a time into `buffer` as minutes, seconds and hundredths with the
 * language's marks.  The parts go through 16 bits, as the game takes them. */
reg32 timeTextBody(Guest& guest, reg32 time, reg32 buffer)
{
    X87 x87(guest);
    const std::int32_t hundredths = x87.truncated(fp::mul(X87::integer(std::int32_t(time)), doubleAt(guest, kToHundredths)));
    const std::int32_t minutes = std::int16_t(hundredths / kHundredthsAMinute);
    const std::int32_t pastMinute = std::int32_t(reg32(hundredths) - reg32(minutes) * reg32(kHundredthsAMinute));
    const std::int32_t seconds = std::int16_t(pastMinute / kHundredthsASecond);
    const std::int32_t rest = std::int16_t(reg32(pastMinute) - reg32(seconds) * reg32(kHundredthsASecond));
    const reg32 language = guest.dword(kLanguage);
    const reg32 secondMark = guest.byte(language + kSecondMarks);
    const reg32 minuteMark = guest.byte(language + kMinuteMarks);
    guest.leaveEdx(secondMark);
    guest.call(kSprintf, args({}, { buffer, kTimeFormat.address, reg32(minutes), minuteMark, reg32(seconds), secondMark, reg32(rest) }));
    return guest.eax();
}

/* ---------------------------------------------------------------------------
 * The laps' option
 * ------------------------------------------------------------------------- */

/* sub_4cf160: 2 laps the list's first place (al cleared, the rest of eax as it
 * came), 4 the second, others the third. */
reg32 lapsToOptionBody(reg32 laps)
{
    const std::uint16_t count = std::uint16_t(laps);
    if (count == 2)
        return laps & ~0xffu;
    return count == 4 ? 1 : 2;
}

/* sub_4cf190: the laps of a place in the list. */
reg32 optionToLapsBody(reg32 option)
{
    const std::uint16_t place = std::uint16_t(option);
    if (place == 0)
        return 2;
    return place == 1 ? 4 : 8;
}

/* ---------------------------------------------------------------------------
 * The language's text
 * ------------------------------------------------------------------------- */

constexpr reg32 kLanguageTexts = 0x563a80;     // its block, once loaded
constexpr reg32 kFree = 0x4e1890;

/* sub_4d1880: the language's block given back. */
reg32 freeLanguageTextsBody(Guest& guest)
{
    const reg32 texts = guest.dword(kLanguageTexts);
    if (texts != 0)
        guest.call(kFree, args({ texts }));
    guest.setDword(kLanguageTexts, 0);
    return guest.eax();
}

/* ---------------------------------------------------------------------------
 * The 2D screens' quad
 * ------------------------------------------------------------------------- */

/* The four corners the 2D layer draws from, 0x20 bytes each. */
constexpr reg32 kQuad = 0x563ab0;
constexpr reg32 kCornerSize = 0x20;
constexpr reg32 kCornerX = 0x00, kCornerY = 0x04, kCornerZ = 0x08, kCornerW = 0x0c;
constexpr reg32 kCornerColour = 0x10, kCornerClip = 0x14, kCornerU = 0x18, kCornerV = 0x1c;
constexpr reg32 kDepthZ = 0x563aa8, kDepthW = 0x563aac;   // floats: the 2D layer's depth

constexpr reg32 corner(reg32 index)
{
    return kQuad + kCornerSize * index;
}

constexpr reg32 kOneBits = 0x3f800000;  // 1.0f

// The window the corners are clipped to: floats, which the game compares as
// integers, as it may positive ones.
constexpr reg32 kClipW = 0x7d34f8, kClipLeft = 0x7d34fc, kClipBottom = 0x7d3500, kClipTop = 0x7d3504, kClipRight = 0x7d350c;
constexpr std::uint8_t kOffLeft = 1, kOffRight = 2, kOffBottom = 4, kOffTop = 8, kBehind = 0x10;

constexpr reg32 kSetRenderState = 0x431900;    // eax a state, edx its value
constexpr reg32 kRenderStateTexture = 1;
constexpr reg32 kClipQuad = 0x4c12e0;          // the four corners' addresses

/* A corner's outcode, as the game works it out in line for each corner before
 * the clipper. */
std::uint8_t clipCode(const Guest& guest, reg32 at)
{
    const std::int32_t w = sreg32(guest.dword(at + kCornerW));
    if (w < 0 || w >= sreg32(guest.dword(kClipW)))
        return kBehind;
    std::uint8_t code = 0;
    const std::int32_t y = sreg32(guest.dword(at + kCornerY));
    if (y < 0 || y < sreg32(guest.dword(kClipTop)))
        code |= kOffTop;
    else if (y > sreg32(guest.dword(kClipBottom)))
        code |= kOffBottom;
    const std::int32_t x = sreg32(guest.dword(at + kCornerX));
    if (x < 0 || x < sreg32(guest.dword(kClipLeft)))
        code |= kOffLeft;
    else if (x > sreg32(guest.dword(kClipRight)))
        code |= kOffRight;
    return code;
}

/* Each corner's outcode, then the quad to the clipper. */
reg32 clipAndDrawQuad(Guest& guest)
{
    for (reg32 i = 0; i < 4; ++i)
        guest.setByte(corner(i) + kCornerClip, clipCode(guest, corner(i)));
    return guest.call(kClipQuad, args({ corner(0), corner(1), corner(2), corner(3) }));
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
    for (reg32 i = 0; i < 4; ++i)
    {
        guest.setFloat(corner(i) + kCornerZ, z);
        guest.setFloat(corner(i) + kCornerW, w);
    }
}

/* The corners' texture coordinates, floats' bits, the corners in order. */
void placeTexture(Guest& guest, reg32 u0, reg32 v0, reg32 u1, reg32 v1, reg32 u2, reg32 v2, reg32 u3, reg32 v3)
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

constexpr reg32 kDisplayFlags = 0x7a3a58;      // byte
constexpr std::uint8_t kNoTranslucency = 0x02;
constexpr reg32 kInRace = 0x559234;

// An art record, 20 bytes.
constexpr reg32 kArtRecords = 0x8b48bc;
constexpr reg32 kArtSize = 0x14;
constexpr reg32 kArtX = 0x04;                  // int16
constexpr reg32 kArtFirstTile = 0x10;          // int16; -1 when not loaded
constexpr reg32 kArtEndTile = 0x12;            // int16: past its last
// A record's tile, 12 bytes.
constexpr reg32 kTiles = 0x8b5090;
constexpr reg32 kTileSize = 0x0c;
constexpr reg32 kTileTexture = 0x00;
constexpr reg32 kTileSide = 0x04;              // int16: its square's side, in pixels
constexpr reg32 kTileColumn = 0x06;            // int16: counted leftwards from the record's x
constexpr reg32 kTileRow = 0x08;               // int16

constexpr std::int32_t kTilePitch = 32;
constexpr reg32 kNotLoadedLog = 0x42d500;
constexpr reg32 kNotLoadedFormat = 0x549630;   // "ERROR: shape %d not loaded in texture po..."
constexpr reg32 kWaitForKey = 0x4f0120;

/* sub_4d7560: art record `art` at (x, y), a textured square white for each of
 * its tiles, with the whole texture on it; nothing in a race or without
 * translucency. */
reg32 drawArtTilesBody(Guest& guest, reg32 art, std::int32_t x, std::int32_t y)
{
    if ((guest.byte(kDisplayFlags) & kNoTranslucency) != 0 || guest.dword(kInRace) != 0)
        return guest.eax();
    const reg32 record = kArtRecords + kArtSize * art;
    const std::int32_t first = int16At(guest, record + kArtFirstTile);
    if (first == -1)
    {
        guest.leaveEdx(art);
        guest.call(kNotLoadedLog, args({ reg32(first) }, { kNotLoadedFormat, art }));
        return guest.call(kWaitForKey);
    }
    placeDepth(guest);
    for (reg32 i = 0; i < 4; ++i)
        guest.setDword(corner(i) + kCornerColour, 0xffffffffu);

    const std::int32_t end = int16At(guest, record + kArtEndTile);
    for (std::int32_t tile = first; tile < end; ++tile)
    {
        const reg32 at = kTiles + kTileSize * reg32(tile);
        placeTexture(guest, kOneBits, 0, 0, 0, 0, kOneBits, kOneBits, kOneBits);
        const Float side = X87::integer(int16At(guest, at + kTileSide));
        const Float column = X87::integer(std::int32_t(reg32(int16At(guest, at + kTileColumn)) * kTilePitch + kTilePitch));
        const Float place = X87::integer(std::int32_t(reg32(int16At(guest, record + kArtX)) + reg32(x)));
        const Float left = fp::sub(place, column);
        const float top = float(X87::integer(std::int32_t(reg32(int16At(guest, at + kTileRow)) * kTilePitch + reg32(y))));
        const Float right = fp::add(left, side);
        const Float bottom = fp::add(side, Float(top));
        placeCorners(guest, float(left), top, float(right), float(bottom));
        guest.call(kSetRenderState, args({ kRenderStateTexture, guest.dword(at + kTileTexture) }));
        clipAndDrawQuad(guest);
    }
    restoreTexture(guest);
    guest.leaveEdx(art);
    guest.leaveEbx(kOneBits);
    return reg32(end);
}

/* ---------------------------------------------------------------------------
 * The bouncing pictures
 * ------------------------------------------------------------------------- */

// A corner's fade: full in the screen's middle, less with its distance on each axis.
constexpr reg32 kFadeCentreX = 0x549684, kFadeCentreY = 0x5496a4;   // floats: -395, -225
constexpr reg32 kFadeFull = 0x549688;          // float, 255
constexpr reg32 kFadePerX = 0x54968c, kFadePerY = 0x5496a8;         // floats: 1/490, 1/450
constexpr reg32 kFadeStrength = 0x549694;      // double, 1.5
constexpr reg32 kFadeTop = 0x54969c;           // double, 255

/* A corner's distance on an axis, through a float in memory as the game takes
 * it: as it is above 0 (and when unordered), else its sign changed. */
float fadeDistance(const Guest& guest, float value, reg32 centre)
{
    const float distance = float(fp::add(Float(value), Float(guest.floatAt(centre))));
    return belowOrUnordered(Float(0.0), Float(distance)) ? distance : float(-Float(distance));
}

/* What that distance takes off. */
Float fadeAlong(const Guest& guest, float distance, reg32 perUnit)
{
    Float share = fp::mul(Float(distance), Float(guest.floatAt(kFadeFull)));
    share = fp::mul(share, Float(guest.floatAt(perUnit)));
    return fp::mul(share, doubleAt(guest, kFadeStrength));
}

/* sub_4d7cc0 (stdcall): the top byte of the colour of a corner at (x, y), of a
 * picture of `alpha`. */
reg32 cornerFadeBody(Guest& guest, float x, float y, reg32 alpha)
{
    X87 x87(guest);
    const float awayX = fadeDistance(guest, x, kFadeCentreX);
    const Float afterX = fp::sub(doubleAt(guest, kFadeTop), fadeAlong(guest, awayX, kFadePerX));
    const float awayY = fadeDistance(guest, y, kFadeCentreY);
    std::int32_t level = x87.truncated(fp::sub(afterX, fadeAlong(guest, awayY, kFadePerY)));
    if (level < 0)
        level = 0;
    const std::int32_t scaled = std::int32_t(alpha * reg32(level)) / 255;
    return reg32(scaled) << 24;
}

constexpr reg32 kCornerFade = 0x4d7cc0;
constexpr reg32 kTexelSize = 0x5496ac;         // float, 1/256

struct Bouncer
{
    reg32 texture;
    std::int32_t x, y, u, v, width, height;
    float scale;
    reg32 colour;
};

/* sub_4d7d90 (stdcall 0x14): a stretch of a texture drawn scaled at (x, y), the
 * colour its tint and its top byte the alpha, each corner fading by its place
 * on the screen. */
reg32 drawBouncerBody(Guest& guest, const Bouncer& b)
{
    const Float widthValue = X87::integer(b.width);
    const Float heightValue = X87::integer(b.height);
    const Float scaledWidth = fp::mul(widthValue, Float(b.scale));
    const Float scaledHeight = fp::mul(Float(b.scale), heightValue);

    placeDepth(guest);
    const float left = float(X87::integer(b.x));
    const float top = float(X87::integer(b.y));
    const Float right = fp::add(scaledWidth, Float(left));
    const Float bottom = fp::add(scaledHeight, Float(top));
    placeCorners(guest, left, top, float(right), float(bottom));

    const reg32 alpha = b.colour >> 24;
    const reg32 tint = b.colour & 0xffffff;
    for (reg32 i = 0; i < 4; ++i)
    {
        const reg32 fade = guest.call(kCornerFade, args({}, { guest.dword(corner(i) + kCornerX), guest.dword(corner(i) + kCornerY), alpha }));
        guest.setDword(corner(i) + kCornerColour, tint + fade);
    }

    const Float texel = Float(guest.floatAt(kTexelSize));
    const Float u = X87::integer(b.u);
    const Float v = X87::integer(b.v);
    const Float uFrom = fp::mul(u, texel);
    const Float uTo = fp::mul(fp::add(u, Float(float(widthValue))), texel);
    const Float vFrom = fp::mul(v, texel);
    const Float vTo = fp::mul(texel, fp::add(v, Float(float(heightValue))));
    const auto bitsOf = [](Float value) {
        const float single = float(value);
        reg32 bits;
        std::memcpy(&bits, &single, sizeof bits);
        return bits;
    };
    placeTexture(guest, bitsOf(uFrom), bitsOf(vFrom), bitsOf(uTo), bitsOf(vFrom), bitsOf(uTo), bitsOf(vTo), bitsOf(uFrom), bitsOf(vTo));

    guest.call(kSetRenderState, args({ kRenderStateTexture, b.texture }));
    const reg32 drawn = clipAndDrawQuad(guest);
    restoreTexture(guest);
    guest.leaveEdx(0);
    return drawn;
}

/* ---------------------------------------------------------------------------
 * The effects' band
 * ------------------------------------------------------------------------- */

/* sub_4d9180: a colour's red, green and blue scaled by `scale`/256, its alpha as
 * it is; a quarter by masks, the whole not at all.  edx is left the sum before
 * the blue. */
reg32 scaleColourBody(Guest& guest, reg32 colour, std::int32_t scale)
{
    constexpr std::int32_t kWhole = 0x100, kQuarter = 0x40;
    if (scale == kWhole)
        return colour;
    const reg32 alpha = colour & 0xff000000;
    reg32 alphaRedGreen, blue;
    if (scale == kQuarter)
    {
        alphaRedGreen = ((colour & 0xfc0000) >> 2) + alpha + ((colour & 0xfc00) >> 2);
        blue = (colour & 0xfc) >> 2;
    }
    else
    {
        const auto scaled = [scale](reg32 component) {
            return reg32(std::int32_t((component & 0xff) * reg32(scale)) / 256);
        };
        alphaRedGreen = alpha + (scaled(colour >> 16) << 16) + (scaled(colour >> 8) << 8);
        blue = scaled(colour);
    }
    guest.leaveEdx(alphaRedGreen);
    return blue + alphaRedGreen;
}

/* ---------------------------------------------------------------------------
 * The menus' pointer
 * ------------------------------------------------------------------------- */

constexpr reg32 kPointerKind = 0x563d3c;       // -2 none (port_draw2d.cpp)
constexpr reg32 kPointerTextures = 0x563b34;   // 13 kinds of 10 frames
constexpr reg32 kPointerTexturesPerKind = 0x28;
constexpr reg32 kHotSpots = 0x563d4c;          // x, y for each frame of each kind
constexpr reg32 kHotSpotsPerKind = 0x50;
constexpr reg32 kHotSpotSize = 8;
constexpr std::int32_t kPointerKinds = 13, kPointerFrames = 10;
constexpr reg32 kShapeCentreX = 0x08, kShapeCentreY = 0x0a;   // int16 of a shape
constexpr GuestString kPointerName{ 0x5496dc, "c%02d%d" };

constexpr reg32 kNewBitmap = 0x4ef190;
constexpr reg32 kFindShape = 0x4ef270;         // eax a file, edx a name
constexpr reg32 kShapeTexture = 0x4d6070;      // shape edx into [eax], ebx the most a side, ecx the format
constexpr reg32 kFreeBitmap = 0x4eff00;
constexpr reg32 kPointerSide = 0x20;

/* sub_4daa10: the pointer's shapes, c<kind><frame>, out of their file: for each
 * there is its hot spot, and a texture.  A bitmap of 32 by 32 is made for the
 * time of it. */
reg32 loadPointerBody(Guest& guest, reg32 file)
{
    const reg32 name = guest.frame();
    const reg32 canvas = guest.call(kNewBitmap, args({ kPointerSide, kPointerSide, kPointerSide, 0 }));
    for (std::int32_t kind = 0; kind < kPointerKinds; ++kind)
    {
        for (std::int32_t frame = 0; frame < kPointerFrames; ++frame)
        {
            guest.call(kSprintf, args({ name }, { name, kPointerName.address, reg32(kind), reg32(frame) }));
            const reg32 shape = guest.call(kFindShape, args({ file, name }));
            if (shape == 0)
                continue;
            const reg32 hotSpot = kHotSpots + kHotSpotsPerKind * reg32(kind) + kHotSpotSize * reg32(frame);
            guest.setDword(hotSpot, reg32(int16At(guest, shape + kShapeCentreX)));
            guest.setDword(hotSpot + 4, reg32(int16At(guest, shape + kShapeCentreY)));
            guest.call(kShapeTexture, args({ kPointerTextures + kPointerTexturesPerKind * reg32(kind) + 4 * reg32(frame), shape, kPointerSide, 0 }));
        }
    }
    return guest.call(kFreeBitmap, args({ canvas }));
}

/* sub_4dab10: the shown pointer's hot spot into [xAt] and [yAt].  In a race the
 * two the code holds, for kind 0 and for kinds 8 to 11, the others left as they
 * are; in the menus its first frame's, 0 for a kind out of the table. */
reg32 pointerHotSpotBody(Guest& guest, reg32 xAt, reg32 yAt)
{
    constexpr reg32 kRaceFirstKind = 0, kRaceOthersFirst = 8, kRaceOthersLast = 11;
    const reg32 kind = guest.dword(kPointerKind);
    reg32 result = guest.eax();
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
    else if (sreg32(kind) < 0 || sreg32(kind) >= kPointerKinds)
    {
        guest.setDword(xAt, 0);
        guest.setDword(yAt, 0);
    }
    else
    {
        const reg32 spot = kHotSpots + kHotSpotsPerKind * kind;
        guest.setDword(xAt, guest.dword(spot));
        result = guest.dword(spot + 4);
        guest.setDword(yAt, result);
    }
    guest.setDword(kPointerKind, kind);
    return result;
}

constexpr reg32 kDrawShadedTriangle = 0x4d8570;

}

/* ---------------------------------------------------------------------------
 * The functions
 * ------------------------------------------------------------------------- */

void frontend::resultsMetricText(win32::WinApplication* app, x86::CPU& cpu)
{
    static const GuestString kStrings[] = { kNoSpeed };
    static fe::Site site{ "sub_4cedd0", 8, 12, kStrings, 1 };
    const float speed = app->getMemory<float>(cpu.esp + 4);
    const reg32 buffer = stackArgument(app, cpu, 1);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return metricTextBody(guest, Float(speed), buffer); });
}

void frontend::resultsTimeText(win32::WinApplication* app, x86::CPU& cpu)
{
    static const GuestString kStrings[] = { kTimeFormat };
    static fe::Site site{ "sub_4cee40", 0, 28, kStrings, 1, kPushedEbxEcxEsiEdi };
    const reg32 time = cpu.eax, buffer = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return timeTextBody(guest, time, buffer); });
}

void frontend::lapsToOption(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf160", 0, 4, nullptr, 0 };
    const reg32 laps = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return lapsToOptionBody(laps); });
}

void frontend::optionToLaps(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4cf190", 0, 4, nullptr, 0 };
    const reg32 option = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return optionToLapsBody(option); });
}

void frontend::languageTextsFree(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d1880", 0, 12, nullptr, 0, kPushedEcxEdx };
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return freeLanguageTextsBody(guest); });
}

void frontend::artTilesDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d7560", 0, 16 + 0x1c, nullptr, 0, kPushedEcxEsiEdi };
    const reg32 art = cpu.eax;
    const std::int32_t x = std::int32_t(cpu.edx), y = std::int32_t(cpu.ebx);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawArtTilesBody(guest, art, x, y); });
}

void frontend::bouncerCornerFade(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d7cc0", 12, 12 + 0x1c, nullptr, 0, kPushedEcxEdx };
    const float x = app->getMemory<float>(cpu.esp + 4), y = app->getMemory<float>(cpu.esp + 8);
    const reg32 alpha = stackArgument(app, cpu, 2);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return cornerFadeBody(guest, x, y, alpha); });
}

void frontend::bouncerDraw(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d7d90", 0x14, 12 + 0x18, nullptr, 0, kPushedEsiEdi };
    const Bouncer bouncer{ cpu.eax,
                           std::int32_t(cpu.edx),
                           std::int32_t(cpu.ebx),
                           std::int32_t(cpu.ecx),
                           std::int32_t(stackArgument(app, cpu, 0)),
                           std::int32_t(stackArgument(app, cpu, 1)),
                           std::int32_t(stackArgument(app, cpu, 2)),
                           app->getMemory<float>(cpu.esp + 4 + 4 * 3),
                           stackArgument(app, cpu, 4) };
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawBouncerBody(guest, bouncer); });
}

/* sub_4d88c0 (stdcall 0x14): the colour picker's triangle (sub_43e310) handed on
 * to the 2D layer's as it came, its registers and the five on the stack. */
void frontend::pickerShadedTriangle(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d88c0", 0x14, 12, nullptr, 0, kPushedEsiEdi };
    const fe::Arguments forwarded = args({}, { stackArgument(app, cpu, 0), stackArgument(app, cpu, 1), stackArgument(app, cpu, 2),
                                               stackArgument(app, cpu, 3), stackArgument(app, cpu, 4) });
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return guest.call(kDrawShadedTriangle, forwarded); });
}

void frontend::effectsBandColourScaled(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d9180", 0, 20 + 4, nullptr, 0, kPushedEbxEcxEsiEdi };
    const reg32 colour = cpu.eax;
    const std::int32_t scale = std::int32_t(cpu.edx);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return scaleColourBody(guest, colour, scale); });
}

void frontend::menuPointerLoad(win32::WinApplication* app, x86::CPU& cpu)
{
    static const GuestString kStrings[] = { kPointerName };
    static fe::Site site{ "sub_4daa10", 0, 24 + 0x48, kStrings, 1 };
    const reg32 file = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return loadPointerBody(guest, file); });
}

void frontend::menuPointerHotSpot(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4dab10", 0, 16, nullptr, 0, kPushedEbxEcxEsiEdi };
    const reg32 xAt = cpu.eax, yAt = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return pointerHotSpotBody(guest, xAt, yAt); });
}

}
