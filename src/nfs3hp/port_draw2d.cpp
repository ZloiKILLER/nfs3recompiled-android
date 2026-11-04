#include "port_draw2d.h"
#include "port_text.h"
#include "port_shapes.h"

#include <cstring>

/* Each sum and product rounds as the x87 code rounds it: none fused. */
#if defined(__clang__)
#pragma clang fp contract(off)
#elif defined(_MSC_VER)
#pragma fp_contract(off)
#endif

/* The game's 2D drawing, rewritten (port_draw2d.h): the screen quad
 * (0x563ab0) and the frames' own quads filled in, clipped and drawn;
 * rectangles, boxes, panels and frames of them (0x4d81b0-0x4d8930); a line,
 * a tile's texture over a rectangle (0x4d8b80, 0x4d8ed0); the 2D screens'
 * font made, its characters, texts and titles (0x4d9240-0x4da4a0); the menus'
 * pointer, the screens started afresh (0x4dab90, 0x4daf80); the quads of 640
 * by 480 scaled to the screen (0x4db000, 0x4db380).
 *
 * tools/game_checks holds each adapter against the game's code made afresh:
 * memory, the calls out with their arguments, eax, esi, edi, ebp and esp, the
 * x87's stack, control and status words.  What these leave in ebx, ecx and
 * edx otherwise than the caller had them is read by no caller of the game's:
 * the callers' reads after each call, followed up through callers that
 * return what they were left, reach only one, the menus' panel lines
 * (0x46d3c0) after a text drawn through drawTextAt and drawMenuText -- the
 * upper half of ebx, into the word sums of the panel's layout
 * (0x46cf30, 0x46d180), whose words it never reaches.
 *
 * The x87 is the game's: a value the code loads and stores as a float goes
 * through x86::Float as the code has it, a word it moves by mov is copied as
 * it is (a float's bits, NaN or not). */
namespace nfs3hp
{
namespace port
{
namespace draw2d
{

namespace
{

using x86::Float;

/* A corner of a quad as the clipper and the drivers take it: its place on the
 * screen, its depth and w, its colour, its clip code, its texture's place. */
#pragma pack(push, 1)
struct Vertex
{
    float x, y, z, w;
    uint32_t colour;
    uint8_t code;
    uint8_t unused[3];
    float u, v;
};
/* A menu shape known: its first tile at +0xc, its name at +0x10. */
struct Shape
{
    uint8_t head[0xc];
    int16_t firstTile;
    int16_t lastTile;
    char name[4];
};
#pragma pack(pop)
static_assert(sizeof(Vertex) == 0x20, "a vertex is 0x20 bytes");
static_assert(sizeof(Shape) == 0x14, "a shape is 0x14 bytes");

/* The 2D screens' quad, which each of these fills in and draws. */
constexpr reg32 kQuadAt = 0x563ab0;
constexpr Records<Vertex> kQuad{kQuadAt};
/* The menus over a race -- its pause screens -- drawn as quads near the eye,
 * placed from 640 by 480; the 2D screens otherwise. */
constexpr Var<int32_t> kOverRace{0x559234};
/* The driver's flags: softtria's, the software triangles'. */
constexpr Var<uint8_t> kDriverFlags{0x7a3a58};
constexpr uint8_t kSofttria = 2, kSoftware = 6;
/* The 2D screens' depth and w. */
constexpr Var<float> kScreenDepth{0x563aa8}, kScreenW{0x563aac};
/* A pause screen's depth and w, and the texture's 1 and 0 (floats' bits). */
constexpr uint32_t kNearDepth = 0x37800080, kNearW = 0x3f7fff00, kOne = 0x3f800000, kTexel = 0x3b800000;
/* The clip's bounds, compared as the floats' bits: w, the left, the bottom,
 * the top, the right. */
constexpr Var<int32_t> kClipFar{0x7d34f8}, kClipLeft{0x7d34fc}, kClipBottom{0x7d3500}, kClipTop{0x7d3504};
constexpr Var<int32_t> kClipRight{0x7d350c};
/* The screen's size. */
constexpr Var<int32_t> kScreenWidth{0x7cdac8}, kScreenHeight{0x7cdacc};
/* The 640 by 480 measure: a 640th and a 480th, twice over. */
constexpr Var<double> kArtPerWidth{0x5496ec}, kArtPerHeight{0x5496f4};
constexpr Var<double> kShadePerWidth{0x5496fc}, kShadePerHeight{0x549704};
/* The fonts' constants: -1, an eighth, the pointer's 32 pixels. */
constexpr Var<float> kMinusOne{0x5496cc}, kPointerSize{0x5496e4};
constexpr Var<double> kEighth{0x5496d4};

/* The 2D screens' font: which of the fonts (the accented one, 0x24, its
 * characters found among the accented; 0x10 and 0xd at a half and three
 * quarters of their width), the colour, its texture's, a pixel's nudge to the
 * right (0 or 1; none with softtria), the language's accented characters,
 * the line the menus' highlight runs down. */
constexpr Var<int32_t> kFont{0x664650};
constexpr int32_t kAccentedFont = 0x24, kHalfFont = 0x10, kThreeQuarterFont = 0xd;
constexpr Var<uint32_t> kTextColour{0x563aa4}, kTextColourMore{0x559434};
constexpr Var<uint32_t> kFontTexture{0x8b7624};
constexpr Table<uint32_t> kFontTextures{0x8b7624};
constexpr Var<int32_t> kFontTextureIndex{0x8b7620};
constexpr Var<int32_t> kNudge{0x563aa0};
constexpr Var<reg32> kAccents{0x564180};
constexpr Table<reg32> kLanguageAccents{0x564160};
constexpr Var<int32_t> kLanguage{0x6fbb4c};
constexpr int32_t kFrench = 2;
constexpr Var<int32_t> kHighlightLine{0x7a1b98};
constexpr uint32_t kHighlight = 0xffe440;
/* The font's characters' squares: their places, widths and heights, a byte
 * each by the character's number (its code less 0x20, an accented one's 0xe0
 * on). */
constexpr Table<uint8_t> kGlyphU{0x8b7858}, kGlyphV{0x8b774a}, kGlyphWidth{0x8b7966}, kGlyphHeight{0x8b763c};
/* The font's bitmap as it is drawn into: the next place, the row's tallest,
 * the tallest row's. */
constexpr Var<reg32> kFontBitmap{0x8b761c};
constexpr Var<int32_t> kFontX{0x8b7618}, kFontY{0x8b7614}, kRowHeight{0x564178}, kTallestRow{0x56417c};
/* The cosines: a quarter turn of 0x100 steps. */
constexpr Table<int32_t> kCosines{0x5672ac};
/* A title's sweep: three steps a character, counted down. */
constexpr Var<int32_t> kSweep{0x564184};

/* The menu shapes known, their tiles, how many of each. */
constexpr Records<Shape> kShapes{0x8b48c0};
constexpr Table<uint32_t> kTileTextures{0x8b5090, 0xc};
constexpr Var<int32_t> kShapeCount{0x8b7628}, kTileCount{0x8b762c};
constexpr int32_t kShapeSlots = 100;

/* The pointer: where it is, its kind (-2 none) and picture, the clock it
 * moved on by, whether the pointers are loaded; each kind's ten pictures'
 * textures and hot spots. */
constexpr Var<int32_t> kPointerX{0x55e5e4}, kPointerY{0x55e5e8};
constexpr Var<int32_t> kPointerKind{0x563d3c}, kPointerPicture{0x563d48}, kPointerMoved{0x563d44};
constexpr Var<int32_t> kPointersLoaded{0x8b7630};
constexpr Var<int32_t> kClock{0x79c1a4};
constexpr reg32 kPointerTextures = 0x563b34, kPointerSpots = 0x563d4c;

/* THRASH_drawquad's slot. */
constexpr Var<reg32> kThrashDrawQuad{0x9ef94c};

/* What is not rewritten: the render state (eax to edx), a 640 by 480 place
 * on the screen, the clipped quad (its corners in eax, edx, ebx, ecx) and
 * line (eax, edx), a line of the log and a key, toupper. */
constexpr reg32 kSetRenderState = 0x431900;
constexpr guest::Native kToScreenX = text::toScreenX, kToScreenY = text::toScreenY;  // 0x451400, 0x451440
constexpr reg32 kClipQuad = 0x4c12e0, kClipLine = 0x4c0b20;
constexpr reg32 kTrace = 0x42d500, kInkey = 0x4f0120, kToupper = 0x4eede0;
/* The render states these set: a texture, alpha's, a quad drawn by softtria. */
constexpr reg32 kStateTexture = 1, kStateAlpha = 6, kStateShape = 0x2bf, kStateBlend = 2;
/* EA's bitmaps and fonts: a bitmap made, drawn into and let go; a font of a
 * file chosen, its colours, a character's width, its height (by pointers),
 * the character drawn; the file loaded and freed; sprintf. */
constexpr reg32 kNewBitmap = 0x4ef190, kDrawInto = 0x4eada0, kDrawIntoScreen = 0x4ead90, kTransparent = 0x4ef1b0;
constexpr reg32 kFreeBitmap = 0x4eff00, kUseFont = 0x4ef520, kColour = 0x4ef760, kFontColours = 0x4ef830;
constexpr reg32 kCharacterWidth = 0x4efe50, kCharacterExtent = 0x4f0b60, kDrawCharacter = 0x4efe30;
constexpr reg32 kLoadFile = 0x4e0ec0, kFreeFile = 0x4e1890, kSprintf = 0x4df690;
constexpr Var<uint8_t> kScreenDepthBits{0x565010};
/* The .ffn files' names: the accented font, the screens' font; the path. */
constexpr reg32 kAccentedFontName = 0x5496b0, kScreenFontName = 0x5496bc, kFontPath = 0x7a2e34;
constexpr reg32 kNoShape = 0x549630;  // "ERROR: shape %d"
/* The shapes' and the screens' functions: a shape made a texture, the
 * textures let go, the screens' states. */
constexpr reg32 kShapeTexture = 0x4d6070, kResetTextures = 0x4d5c40, kScreenStates = 0x431bc0;

uint32_t bitsOf(float f)
{
    uint32_t b;
    std::memcpy(&b, &f, 4);
    return b;
}

float fromBits(uint32_t b)
{
    float f;
    std::memcpy(&f, &b, 4);
    return f;
}

/* fld and fstp of a float; fild and fstp of a dword. */
float viaX87(float f)
{
    return float(Float(f));
}

float ofInt(int32_t i)
{
    return float(Float(i));
}

/* mov eax, v; sar edx, 0x1f; sub eax, edx; sar eax, 1: v halved toward 0. */
int32_t half(int32_t v)
{
    return int32_t(uint32_t(v) - uint32_t(v >> 31)) >> 1;
}

/* sar edx, 0x1f; shl edx, n; sbb eax, edx; sar eax, n: over 2^n toward 0. */
int32_t over(int32_t v, int n)
{
    return v < 0 ? int32_t(uint32_t(v) + ((1u << n) - 1)) >> n : v >> n;
}

/* The cosine of `angle` (0x100 a quarter turn) from the table, as the code
 * folds an angle into its quarter: a step past 0x100 the table's mirror, past
 * 0x200 the cosine's sign turned. */
int32_t cosine(App* app, int32_t angle)
{
    const uint32_t c = uint32_t(angle) + 0x100;
    const uint32_t negate = (c >> 9) & 1 ? 0xffffffffu : 0;
    const uint32_t mirror = (c >> 8) & 1 ? 0xffffffffu : 0;
    const uint32_t step = ((uint32_t(angle) ^ mirror) & 0xff) - mirror;
    return int32_t((uint32_t(kCosines(app, int32_t(step))) ^ negate) - negate);
}

/* The clip code of the vertex at `at`: 0x10 behind the eye or past the far
 * clip, else 8 above the clip, 4 below, 1 left of it, 2 right of it. */
uint8_t clipCode(App* app, reg32 at)
{
    const Vertex v = load<Vertex>(app, at);
    const int32_t w = int32_t(bitsOf(v.w));
    if (w < 0 || w >= kClipFar(app))
        return 0x10;
    uint8_t code = 0;
    const int32_t y = int32_t(bitsOf(v.y));
    if (y < 0 || y < kClipTop(app))
        code |= 8;
    else if (y > kClipBottom(app))
        code |= 4;
    const int32_t x = int32_t(bitsOf(v.x));
    if (x < 0 || x < kClipLeft(app))
        code |= 1;
    else if (x > kClipRight(app))
        code |= 2;
    return code;
}

/* The clip codes of the four corners of the quad at `at`. */
void markCorners(App* app, reg32 at)
{
    for (reg32 corner = 0; corner < 4; ++corner)
        record<Vertex>(app, at + 0x20 * corner).code = clipCode(app, at + 0x20 * corner);
}

/* The quad at `at` clipped and drawn. */
bool drawQuad(App* app, CPU& cpu, reg32 esp, reg32 at, reg32& r)
{
    return callGame(app, cpu, esp, kClipQuad, {at, at + 0x20, at + 0x40, at + 0x60}, {}, r);
}

bool renderState(App* app, CPU& cpu, reg32 esp, reg32 state, reg32 value, reg32& r, reg32 ebx = 0, reg32 ecx = 0)
{
    return callGame(app, cpu, esp, kSetRenderState, {state, value, ebx, ecx}, {}, r);
}

/* The screen quad's depth and w as the 2D screens have them: the first
 * corner's last, through the x87, each of the others through it but the last
 * corner's depth, copied. */
void screenDepths(App* app)
{
    const float depth = kScreenDepth(app), w = kScreenW(app);
    kQuad(app, 0).w = viaX87(w);
    kQuad(app, 1).z = viaX87(depth);
    kQuad(app, 1).w = viaX87(w);
    kQuad(app, 2).z = viaX87(depth);
    kQuad(app, 2).w = viaX87(w);
    kQuad(app, 3).z = depth;
    kQuad(app, 3).w = viaX87(w);
    kQuad(app, 0).z = viaX87(depth);
}

}  // namespace

/* ---- 0x4d5840 ---- */

int32_t clipCodes(App* app, int32_t count, reg32 vertices, reg32 quads)
{
    int32_t i = 0;
    for (; i < count; ++i, quads += 0x10)
        for (reg32 corner = 0; corner < 4; ++corner)
        {
            const reg32 at = (app->getMemory<reg32>(quads + 4 * corner) << 5) + vertices;
            record<Vertex>(app, at).code = clipCode(app, at);
        }
    return i;
}

/* 0x4d5840: the clip codes of the four corners of each of eax quads -- four
 * vertex numbers each, 0x10 bytes, from ebx -- among the vertices at edx. */
void quadOutcodes(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.back(cpu, reg32(clipCodes(app, int32_t(e.eax), e.edx, e.ebx)));
}

/* ---- 0x4d81b0 ---- */

Result shadedRectangle(App* app, CPU& cpu, reg32 esp, int32_t left, int32_t top, int32_t right, int32_t bottom,
                       const uint32_t colours[4])
{
    const reg32 at = esp - 0x1c;  // esi, edi, ebp and 0x10 bytes
    HeldEbp held(cpu, esp - 0xc);
    reg32 r;
    if (kOverRace(app) != 0)
    {
        for (int32_t corner = 0; corner < 4; ++corner)
        {
            kQuad(app, corner).z = fromBits(kNearDepth);
            kQuad(app, corner).w = fromBits(kNearW);
        }
        if (!callGame(app, cpu, at, kToScreenX, {reg32(left), reg32(top), reg32(right), reg32(bottom)}, {}, r))
            return {};
        left = int32_t(r);
        if (!callGame(app, cpu, at, kToScreenX, {reg32(right), reg32(top), reg32(right), reg32(bottom)}, {}, r))
            return {};
        right = int32_t(r);
        if (!callGame(app, cpu, at, kToScreenY, {reg32(top), reg32(top), reg32(right), reg32(bottom)}, {}, r))
            return {};
        top = int32_t(r);
        if (!callGame(app, cpu, at, kToScreenY, {reg32(bottom), reg32(top), reg32(right), reg32(bottom)}, {}, r))
            return {};
        bottom = int32_t(r);
    }
    else
    {
        screenDepths(app);
    }
    kQuad(app, 3).x = ofInt(left);
    kQuad(app, 2).x = ofInt(right);
    kQuad(app, 1).y = ofInt(top);
    kQuad(app, 3).y = ofInt(bottom);
    kQuad(app, 0).colour = colours[0];
    kQuad(app, 0).x = viaX87(kQuad(app, 3).x);
    kQuad(app, 1).x = viaX87(kQuad(app, 2).x);
    kQuad(app, 0).y = viaX87(kQuad(app, 1).y);
    kQuad(app, 2).y = viaX87(kQuad(app, 3).y);
    kQuad(app, 1).colour = colours[1];
    kQuad(app, 2).colour = colours[2];
    kQuad(app, 3).colour = colours[3];
    if (!renderState(app, cpu, at, kStateTexture, 0, r, reg32(right), reg32(bottom)))
        return {};
    markCorners(app, kQuadAt);
    if ((kDriverFlags(app) & kSofttria) != 0)
    {
        if (!renderState(app, cpu, at, kStateShape, 2, r)
            || !callGame(app, cpu, at, kThrashDrawQuad(app), {}, {kQuadAt, kQuadAt + 0x20, kQuadAt + 0x40, kQuadAt + 0x60},
                         r)
            || !renderState(app, cpu, at, kStateShape, 0, r))
            return {};
    }
    else if (!drawQuad(app, cpu, at, kQuadAt, r))
        return {};
    return int32_t(r);
}

/* 0x4d81b0 (stdcall 0x10): the rectangle from eax, edx to ebx, ecx, the
 * stack's four colours at its corners. */
void drawShadedRectangle(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const uint32_t colours[4] = {e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)};
    e.finish(cpu,
             shadedRectangle(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), colours),
             0x10);
}

/* ---- 0x4d8550 ---- */

Result gradient(App* app, CPU& cpu, reg32 esp, int32_t left, int32_t top, int32_t right, int32_t bottom,
                uint32_t from, uint32_t to)
{
    const uint32_t colours[4] = {from, to, to, from};
    return shadedRectangle(app, cpu, esp - 0xc - 0x10 - 4, left, top, right, bottom, colours);
}

/* 0x4d8550 (stdcall 8): the rectangle from eax, edx to ebx, ecx shaded from
 * [esp+4] at its left to [esp+8] at its right. */
void gradientRectangle(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             gradient(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), e.arg(app, 0),
                      e.arg(app, 1)),
             8);
}

/* ---- 0x4d8860 ---- */

Result shadedBox(App* app, CPU& cpu, reg32 esp, int32_t x, int32_t y, int32_t width, int32_t height,
                 const uint32_t colours[4])
{
    const reg32 at = esp - 0xc;
    HeldEbp held(cpu, at);
    reg32 r;
    if ((kDriverFlags(app) & kSofttria) != 0
        && !renderState(app, cpu, at, kStateAlpha, 1, r, reg32(width), reg32(height)))
        return {};
    const Result drawn = shadedRectangle(app, cpu, at - 0x10 - 4, x, y, x + width, y + height, colours);
    if (!drawn)
        return {};
    if ((kDriverFlags(app) & kSofttria) != 0)
        return renderState(app, cpu, at, kStateAlpha, 0, r) ? Result(int32_t(r)) : Result();
    return drawn;
}

/* 0x4d8860 (stdcall 0x10): the rectangle at eax, edx, ebx wide and ecx high,
 * the stack's four colours at its corners -- alpha on for it with softtria. */
void drawShadedBox(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const uint32_t colours[4] = {e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)};
    e.finish(cpu, shadedBox(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), colours),
             0x10);
}

/* ---- 0x4d8910 ---- */

Result filled(App* app, CPU& cpu, reg32 esp, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t colour)
{
    return gradient(app, cpu, esp - 8 - 8 - 4, x, y, x + width, y + height, colour, colour);
}

/* 0x4d8910 (stdcall 4): the rectangle at eax, edx, ebx wide and ecx high, in
 * the colour [esp+4]. */
void filledRectangle(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, filled(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), e.arg(app, 0)),
             4);
}

/* ---- 0x4d8930 ---- */

Result outline(App* app, CPU& cpu, reg32 esp, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t colour)
{
    const reg32 at = esp - 0x24;  // esi, edi, ebp and 0x18 bytes
    const reg32 callee = at - 8 - 4;
    HeldEbp held(cpu, esp - 0xc);
    reg32 r;
    if ((kDriverFlags(app) & kSofttria) != 0
        && !renderState(app, cpu, at, kStateAlpha, 1, r, reg32(width), reg32(height)))
        return {};
    const int32_t right = x + width, bottom = y + height;
    Result drawn;
    if (!(drawn = gradient(app, cpu, callee, x - 1, y - 1, x, bottom + 1, colour, colour))
        || !(drawn = gradient(app, cpu, callee, right, y - 1, right + 1, bottom + 1, colour, colour))
        || !(drawn = gradient(app, cpu, callee, x - 1, y - 1, right + 1, y, colour, colour))
        || !(drawn = gradient(app, cpu, callee, x - 1, bottom, right + 1, bottom + 1, colour, colour)))
        return {};
    if ((kDriverFlags(app) & kSofttria) != 0)
        return renderState(app, cpu, at, kStateAlpha, 0, r) ? Result(int32_t(r)) : Result();
    return drawn;
}

/* 0x4d8930 (stdcall 4): a frame a pixel wide around the rectangle at eax,
 * edx, ebx wide and ecx high, in the colour [esp+4] -- alpha on for it with
 * softtria. */
void frame(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, outline(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), e.arg(app, 0)),
             4);
}

/* ---- 0x4d89f0 ---- */

Result panel(App* app, CPU& cpu, reg32 esp, int32_t x, int32_t y, int32_t width, int32_t height)
{
    constexpr uint32_t kDark = 0xff282828, kLight = 0xff727eb3, kBlack = 0xff000000;
    const reg32 callee = esp - 0x20 - 8 - 4;  // esi, edi, ebp and 0x14 bytes
    HeldEbp held(cpu, esp - 0xc);
    const int32_t right = x + width, bottom = y + height;
    Result drawn;
    if (!(drawn = gradient(app, cpu, callee, x, y, right, bottom, kDark, kDark))
        || !(drawn = gradient(app, cpu, callee, x - 1, y - 1, x, bottom + 1, kLight, kLight))
        || !(drawn = gradient(app, cpu, callee, right, y - 1, right + 1, bottom + 1, kBlack, kBlack))
        || !(drawn = gradient(app, cpu, callee, x - 1, y - 1, right + 1, y, kLight, kLight))
        || !(drawn = gradient(app, cpu, callee, x - 1, bottom, right + 1, bottom + 1, kBlack, kBlack)))
        return {};
    return drawn;
}

/* 0x4d89f0: the menus' raised panel at eax, edx, ebx wide and ecx high. */
void drawPanel(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, panel(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx)));
}

/* ---- 0x4d8ac0 ---- */

Result fieldBox(App* app, CPU& cpu, reg32 esp, int32_t x, int32_t y, int32_t width, int32_t height,
                const uint32_t colours[4])
{
    constexpr uint32_t kLight = 0xff727eb3, kBlack = 0xff000000;
    const reg32 at = esp - 0x18;  // esi, edi, ebp and 0xc bytes
    HeldEbp held(cpu, esp - 0xc);
    const int32_t right = x + width, bottom = y + height;
    Result drawn;
    if (!(drawn = shadedRectangle(app, cpu, at - 0x10 - 4, x, y, right, bottom, colours))
        || !(drawn = line(app, cpu, at - 8 - 4, x, y - 1, x, bottom, kLight, kBlack))
        || !(drawn = line(app, cpu, at - 8 - 4, right, y - 1, right, bottom, kLight, kBlack))
        || !(drawn = line(app, cpu, at - 8 - 4, x, y - 1, right, y - 1, kLight, kLight))
        || !(drawn = line(app, cpu, at - 8 - 4, x, bottom, right, bottom, kBlack, kBlack)))
        return {};
    return drawn;
}

/* 0x4d8ac0 (stdcall 0x10): a field to type in at eax, edx, ebx wide and ecx
 * high, the stack's four colours at its corners -- nothing on a race's pause
 * screens. */
void drawFieldBox(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (kOverRace(app) != 0)
        return e.back(cpu, e.eax, 0x10);
    const uint32_t colours[4] = {e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)};
    e.finish(cpu, fieldBox(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), colours),
             0x10);
}

/* ---- 0x4d8ed0 ---- */

Result line(App* app, CPU& cpu, reg32 esp, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t from,
            uint32_t to)
{
    const reg32 at = esp - 0x1c;  // esi, edi, ebp and 0x10 bytes
    HeldEbp held(cpu, esp - 0xc);
    reg32 r;
    if ((kDriverFlags(app) & kSofttria) != 0 && !renderState(app, cpu, at, kStateAlpha, 1, r, reg32(x1), reg32(y1)))
        return {};
    Vertex& q0 = kQuad(app, 0);
    Vertex& q1 = kQuad(app, 1);
    Vertex& q2 = kQuad(app, 2);
    Vertex& q3 = kQuad(app, 3);
    const float depth = kScreenDepth(app), w = kScreenW(app);
    q3.z = viaX87(depth);
    q3.w = viaX87(w);
    q2.z = viaX87(q3.z);
    q2.w = viaX87(q3.w);
    q3.colour = from;
    q0.colour = from;
    q3.x = ofInt(x0);
    q2.x = ofInt(x1);
    q2.y = ofInt(y0);
    q3.y = ofInt(y1);
    q1.z = viaX87(q2.z);
    q1.w = viaX87(q2.w);
    q0.x = viaX87(q3.x);
    q1.x = viaX87(q2.x);
    q0.y = viaX87(q2.y);
    q1.y = viaX87(q3.y);
    q0.z = q1.z;
    q2.colour = to;
    q1.colour = to;
    q0.w = viaX87(q1.w);
    if (!renderState(app, cpu, at, kStateTexture, 0, r, reg32(x1), bitsOf(q1.z)))
        return {};
    // Its two ends.
    for (int32_t corner = 0; corner < 2; ++corner)
        kQuad(app, corner).code = clipCode(app, kQuad.address(corner));
    if (!callGame(app, cpu, at, kClipLine, {kQuadAt, kQuadAt + 0x20}, {}, r))
        return {};
    if ((kDriverFlags(app) & kSofttria) != 0 && !renderState(app, cpu, at, kStateAlpha, 0, r))
        return {};
    return int32_t(r);
}

/* 0x4d8ed0 (stdcall 8): a line from eax, edx to ebx, ecx shaded from the
 * colour [esp+4] to [esp+8] -- nothing on a race's pause screens; alpha on
 * for it with softtria. */
void drawLine(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (kOverRace(app) != 0)
        return e.back(cpu, e.eax, 8);
    e.finish(cpu,
             line(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx), e.arg(app, 0),
                  e.arg(app, 1)),
             8);
}

/* ---- 0x4d8b80 ---- */

Result tileRectangle(App* app, CPU& cpu, reg32 esp, int32_t shape, int32_t left, int32_t right, int32_t top,
                     int32_t bottom, uint32_t from, uint32_t to)
{
    const reg32 at = esp - 0x18;  // esi, edi, ebp and 0xc bytes
    HeldEbp held(cpu, esp - 0xc);
    reg32 r;
    const int32_t tile = kShapes(app, shape).firstTile;
    if (tile == -1)
    {
        // A shape not loaded: said in the log, and a key waited for.
        if (!callGame(app, cpu, at, kTrace, {0xffffffffu, reg32(left), reg32(top), reg32(right)},
                      {kNoShape, reg32(shape)}, r)
            || !callGame(app, cpu, at, kInkey, {r, reg32(left), reg32(top), reg32(right)}, {}, r))
            return {};
        return int32_t(r);
    }
    Vertex& q0 = kQuad(app, 0);
    Vertex& q1 = kQuad(app, 1);
    Vertex& q2 = kQuad(app, 2);
    Vertex& q3 = kQuad(app, 3);
    const float depth = kScreenDepth(app), w = kScreenW(app);
    q3.z = viaX87(depth);
    q3.w = viaX87(w);
    q2.z = viaX87(q3.z);
    q2.w = viaX87(q3.w);
    q3.colour = from;
    q0.colour = from;
    q3.x = ofInt(left);
    q2.x = ofInt(right);
    q1.y = ofInt(top);
    q1.z = viaX87(q2.z);
    q1.w = viaX87(q2.w);
    q0.x = viaX87(q3.x);
    q1.x = viaX87(q2.x);
    q0.y = viaX87(q1.y);
    q2.colour = to;
    q1.colour = to;
    q0.z = viaX87(q1.z);
    q0.w = viaX87(q1.w);
    q3.y = ofInt(bottom);
    q2.y = viaX87(q3.y);
    if (!renderState(app, cpu, at, kStateTexture, kTileTextures(app, tile), r, reg32(top), reg32(right)))
        return {};
    markCorners(app, kQuadAt);
    if (!drawQuad(app, cpu, at, kQuadAt, r))
        return {};
    return int32_t(r);
}

/* 0x4d8b80 (stdcall 0xc): shape eax's first tile's texture over the
 * rectangle from edx, ebx to ecx, [esp+4], shaded from the colour [esp+8] at
 * its left to [esp+0xc] at its right -- nothing on a race's pause screens. */
void drawTileRectangle(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (kOverRace(app) != 0)
        return e.back(cpu, e.eax, 0xc);
    e.finish(cpu,
             tileRectangle(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ecx), int32_t(e.ebx),
                           int32_t(e.arg(app, 0)), e.arg(app, 1), e.arg(app, 2)),
             0xc);
}

/* ---- 0x4d9320 ---- */

Result accented(App* app, CPU& cpu, reg32 esp, reg32 c)
{
    const reg32 at = esp - 0x10;  // ebx, ecx, edx, ebp
    HeldEbp held(cpu, at);
    const reg32 language = reg32(kLanguage(app));
    reg32 accents = kAccents(app);
    uint8_t al = uint8_t(c);
    if (kLanguage(app) == kFrench && (al == 0xc9 || al == 0xe9))
    {
        // In French an e acute is an E.
        reg32 r;
        if (!callGame(app, cpu, at, kToupper, {0x65, accents, language, 0xe0}, {}, r))
            return {};
        al = uint8_t(r);
    }
    else if (al >= 0xe0)
    {
        al = uint8_t(al - 0x20);
    }
    else
    {
        reg32 r;
        if (!callGame(app, cpu, at, kToupper, {c & 0xff, accents, language, 0xe0}, {}, r))
            return {};
        al = uint8_t(r);
    }
    for (int32_t place = 0xe0;; ++place, ++accents)
    {
        const uint8_t a = app->getMemory<uint8_t>(accents);
        if (a == 0)
            return 0xe0;
        if (a == al)
            return place;
    }
}

/* 0x4d9320: the place of character al among the 2D screens' accented ones
 * (the text at [0x564180]): 0xe0 and its place there, or 0xe0 where it is
 * not.  Upper case first -- the characters from 0xe0 by taking 0x20 off, the
 * others by toupper; in French an e acute is an E. */
void accentedCharacter(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, accented(app, cpu, e.esp, e.eax));
}

/* ---- 0x4d9240 ---- */

Result glyph(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t index, reg32 ebx, reg32 ecx)
{
    const reg32 at = esp - 0x24;    // ebx, ecx, esi, edi, ebp and 0x10 bytes
    const reg32 frame = esp - 0x14;  // ebp
    HeldEbp held(cpu, frame);
    reg32 r;
    if (!callGame(app, cpu, at, kCharacterWidth, {text, 1, ebx, ecx}, {}, r))
        return {};
    kGlyphWidth.set(app, index, uint8_t(r + 1));
    // Its height, and a width it is not asked for, by pointers into the frame.
    if (!callGame(app, cpu, at, kCharacterExtent, {text, 1, frame - 0x10, frame - 0xc}, {frame - 8, frame - 4}, r))
        return {};
    kGlyphHeight.set(app, index, app->getMemory<uint8_t>(frame - 4));
    const int32_t height = kGlyphHeight(app, index);
    if (height > kRowHeight(app))
        kRowHeight.set(app, height);
    const int32_t x = kFontX(app);
    if (int32_t(kGlyphWidth(app, index)) + x + 4 > 0xff)
    {
        // The next row.
        const int32_t row = kRowHeight(app);
        kFontY.set(app, kFontY(app) + row);
        if (row > kTallestRow(app))
            kTallestRow.set(app, row);
        kFontX.set(app, 0);
        kRowHeight.set(app, 0);
    }
    kGlyphU.set(app, index, uint8_t(kFontX(app)));
    kGlyphV.set(app, index, uint8_t(kFontY(app)));
    if (!callGame(app, cpu, at, kDrawCharacter, {text, reg32(kFontX(app)), reg32(kFontY(app)), reg32(kFontX(app))}, {},
                  r))
        return {};
    return int32_t(kGlyphWidth(app, index));
}

/* 0x4d9240: the character of the text at eax drawn into the 2D screens'
 * font's bitmap at its next place -- the next row's when the row is full --
 * and its square noted as character edx's: how wide it is, a pixel more than
 * EA's font has it. */
void fontCharacter(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, glyph(app, cpu, e.esp, e.eax, int32_t(e.edx), e.ebx, e.ecx));
}

/* ---- 0x4d93a0 ---- */

Result makeScreenFont(App* app, CPU& cpu, reg32 esp)
{
    const reg32 at = esp - 0x118;  // ebx, ecx, edx, esi, edi, ebp and 0x100 bytes
    const reg32 name = at;         // the .ffn file's name, then a character to draw
    const reg32 callee = at - 4;
    HeldEbp held(cpu, esp - 0x18 - 0x17e);
    reg32 r;
    // A bitmap of 256 by 256, 32 bits, drawn into.
    if (!callGame(app, cpu, at, kNewBitmap, {0x100, 0x100, 0x20, 0}, {}, r))
        return {};
    kFontBitmap.set(app, r);
    if (!callGame(app, cpu, at, kDrawInto, {r, 0x100, 0x20, 0}, {}, r)
        || !callGame(app, cpu, at, kTransparent, {kScreenDepthBits(app) == 8 ? 0xffu : 0u, 0x100, 0x20, 0}, {}, r))
        return {};
    kFontTextureIndex.set(app, 0);
    kFontY.set(app, 0);
    kFontX.set(app, 0);
    if (uint32_t(kLanguage(app)) <= 5)
        kAccents.set(app, kLanguageAccents(app, kLanguage(app)));
    // The accented characters, from the accented font.
    reg32 accents = kAccents(app);
    if (!callGame(app, cpu, at, kSprintf, {name, 0, accents, 0}, {name, kAccentedFontName, kFontPath}, r)
        || !callGame(app, cpu, at, kLoadFile, {name, 0x10, accents, 0}, {}, r))
        return {};
    reg32 file = r;
    reg32 first, second;
    if (!callGame(app, cpu, at, kUseFont, {file, 0x10, accents, 0}, {}, r)
        || !callGame(app, cpu, at, kColour, {0, 0x10, accents, 0}, {}, first)
        || !callGame(app, cpu, at, kColour, {0xffffff, first, accents, 0}, {}, second)
        || !callGame(app, cpu, at, kFontColours, {second, first, accents, 0}, {}, r))
        return {};
    app->getMemory<uint8_t>(name + 1) = 0;
    reg32 index = 0xe0;
    for (;; ++accents, ++index)
    {
        const uint8_t c = app->getMemory<uint8_t>(accents);
        if (c == 0)
            break;
        app->getMemory<uint8_t>(name) = c == 'R' ? 'r' : c;
        if (c == '-')
            app->getMemory<uint8_t>(name) = '_';
        const Result width = glyph(app, cpu, callee, name, int32_t(index), accents, index);
        if (!width)
            return {};
        kFontX.set(app, kFontX(app) + *width);
    }
    if (!callGame(app, cpu, at, kFreeFile, {file}, {}, r))
        return {};
    // The rest, from the screens' font.
    kFontX.set(app, 0);
    kFontY.set(app, 0x80);
    if (!callGame(app, cpu, at, kSprintf, {name, 0x80, accents, index}, {name, kScreenFontName, kFontPath}, r)
        || !callGame(app, cpu, at, kLoadFile, {name, 0x10, accents, index}, {}, r))
        return {};
    file = r;
    if (!callGame(app, cpu, at, kUseFont, {file, 0x10, accents, index}, {}, r)
        || !callGame(app, cpu, at, kColour, {0, 0x10, accents, 0x20}, {}, first)
        || !callGame(app, cpu, at, kColour, {0xffffff, first, accents & 0xffffff00, 0x20}, {}, second)
        || !callGame(app, cpu, at, kFontColours, {second, first, accents & 0xffffff00, 0x20}, {}, r))
        return {};
    app->getMemory<uint8_t>(name + 1) = 0;
    reg32 ebx = accents & 0xffffff00, edx = 0;
    Result width;
    for (reg32 c = 0x20; c < 0x100; ++c)
    {
        app->getMemory<uint8_t>(name) = uint8_t(c);
        if (!(width = glyph(app, cpu, callee, name, int32_t(c - 0x20), ebx, c)))
            return {};
        // edx as the character's drawing was handed it (EA's code keeps it).
        edx = reg32(kFontX(app));
        ebx = edx + reg32(*width);
        kFontX.set(app, int32_t(ebx));
    }
    // The bitmap the font's texture.
    if (!callGame(app, cpu, at, kDrawIntoScreen, {reg32(*width), edx, ebx, 0x100}, {}, r))
        return {};
    const reg32 slot = kFontTextures.address(kFontTextureIndex(app));
    const reg32 shape = app->getMemory<reg32>(kFontBitmap(app) + 0x20);
    if (!callGame(app, cpu, at, kShapeTexture, {slot, shape, 0x100, (kDriverFlags(app) & kSofttria) != 0 ? 3u : 0u}, {},
                  r)
        || !callGame(app, cpu, at, kFreeBitmap, {kFontBitmap(app)}, {}, r)
        || !callGame(app, cpu, at, kFreeFile, {file}, {}, r))
        return {};
    return int32_t(r);
}

/* 0x4d93a0: the 2D screens' font made: the language's accented characters
 * from sci20.ffn, from 0xe0 on, the others from arial10b.ffn from its second
 * half, each drawn into a bitmap of 256 by 256 and its square noted; the
 * bitmap the font's texture. */
void buildScreenFont(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, makeScreenFont(app, cpu, e.esp));
}

/* ---- 0x4daf80 ---- */

Result resetScreens(App* app, CPU& cpu, reg32 esp, const Args& a)
{
    const reg32 at = esp - 0x10;  // ebx, ecx, edx, ebp
    HeldEbp held(cpu, at);
    reg32 r;
    if (!callGame(app, cpu, at, kScreenStates, a, {}, r) || !callGame(app, cpu, at, kResetTextures, {r, a.edx, a.ebx, a.ecx}, {}, r)
        || !makeScreenFont(app, cpu, at - 4))
        return {};
    for (int32_t i = 0; i < kShapeSlots; ++i)
        kShapes(app, i).firstTile = -1;
    kShapeCount.set(app, 1);
    kTileCount.set(app, 0);
    for (int32_t i = 0; i < kShapeSlots; ++i)
        kShapes(app, i).name[0] = 0;
    if (!renderState(app, cpu, at, kStateBlend, 0, r, 0xffffffffu, 1))
        return {};
    return int32_t(r);
}

/* 0x4daf80: the 2D screens started afresh: the screens' states, the
 * textures let go, the font made again, the menu shapes known none but the
 * first, render state 2 off. */
void startScreens(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, resetScreens(app, cpu, e.esp, {e.eax, e.edx, e.ebx, e.ecx}));
}

/* ---- 0x4d95c0 ---- */

Result screenCharacter(App* app, CPU& cpu, reg32 esp, int32_t u, int32_t v, int32_t x, int32_t y, int32_t width,
                       int32_t height, uint32_t colour, reg32 measure)
{
    const reg32 at = esp - 0x40;  // esi, edi, ebp and 0x34 bytes
    HeldEbp held(cpu, esp - 0xc);
    if (kOverRace(app) != 0)
        return width;
    x86::FPU& fpu = cpu.fpu;
    const float kU = fromBits(kTexel), kV = fromBits(kTexel);
    uint32_t topColour = colour, bottomColour = colour;
    int32_t advance, drawn;
    if (kFont(app) == kAccentedFont)
    {
        // The accented font overlaps its characters by a pixel; the menus'
        // highlight its own shadowed colour, white where the line crosses it.
        const uint32_t rgb = colour & 0xffffff;
        u += 1;
        y += 4;
        v += 3;
        advance = width - 1;
        drawn = height - 1;
        if (rgb == kHighlight)
        {
            topColour = 0xfffff180;
            bottomColour = 0xffd0ae28;
            if (kHighlightLine(app) > x && kHighlightLine(app) < x + advance)
                topColour = bottomColour = 0xffffffff;
        }
    }
    else
    {
        const int32_t halfHeight = half(height), halfWidth = half(width);
        if (kFont(app) == kHalfFont)
        {
            u += 1;
            y += 3;
            advance = halfWidth;
            v += 4;
            drawn = halfHeight;
        }
        else if (kFont(app) == kThreeQuarterFont)
        {
            advance = halfWidth + over(width, 2);
            drawn = over(height, 2) + halfHeight;
        }
        else
        {
            drawn = height;
            advance = width;
        }
    }
    if (drawn < 1)
        return advance;
    const float uf = ofInt(u), vf = ofInt(v);
    if (measure != 0)
        return advance;
    Vertex& q0 = kQuad(app, 0);
    Vertex& q1 = kQuad(app, 1);
    Vertex& q2 = kQuad(app, 2);
    Vertex& q3 = kQuad(app, 3);
    const float depth = kScreenDepth(app), w = kScreenW(app);
    q3.z = viaX87(depth);
    q3.w = viaX87(w);
    q2.z = q3.z;
    q2.w = viaX87(q3.w);
    q1.colour = topColour;
    q0.colour = topColour;
    q1.y = ofInt(y);
    const Float bottom = Float(y + drawn);
    q1.z = viaX87(q2.z);
    q1.w = viaX87(q2.w);
    q0.y = viaX87(q1.y);
    q3.y = float(bottom);
    q0.z = viaX87(q1.z);
    q3.colour = bottomColour;
    q2.colour = bottomColour;
    q0.w = viaX87(q1.w);
    q2.y = viaX87(q3.y);
    q3.x = ofInt(x);
    q0.x = viaX87(q3.x);
    const Float right = Float(x + advance);
    q2.x = float(right);
    q1.x = float(right);
    if ((kDriverFlags(app) & kSofttria) != 0)
        kNudge.set(app, 0);
    // The texture's corners, a 256th of the texture a texel.
    const Float u0 = fp::mul(Float(uf), Float(kU));
    const Float v0 = fp::mul(Float(vf), Float(kV));
    const Float u1 = fp::mul(Float(kU), fp::add(Float(width), Float(uf)));
    const Float v1 = fp::mul(Float(kV), fp::add(Float(vf), Float(height)));
    const float nudge = ofInt(kNudge(app));
    const Float x0 = fp::add(Float(q0.x), Float(nudge));
    const Float x1 = fp::add(Float(q1.x), Float(nudge));
    q3.u = float(u0);
    q1.v = float(v0);
    q0.x = float(x0);
    q1.x = float(x1);
    q0.u = viaX87(q3.u);
    q0.v = viaX87(q1.v);
    q2.u = float(u1);
    q3.v = float(v1);
    q1.u = q2.u;
    q2.v = float(v1);
    reg32 r;
    const bool shape = (kDriverFlags(app) & kSofttria) != 0 && kNudge(app) == 0;
    if (shape && !renderState(app, cpu, at, kStateShape, 1, r))
        return {};
    markCorners(app, kQuadAt);
    // Drawn twice, to stand out, but with softtria or in the accented font.
    if (!drawQuad(app, cpu, at, kQuadAt, r))
        return {};
    if ((kDriverFlags(app) & kSofttria) == 0 && kFont(app) != kAccentedFont && !drawQuad(app, cpu, at, kQuadAt, r))
        return {};
    if ((kDriverFlags(app) & kSofttria) != 0 && kNudge(app) == 0 && !renderState(app, cpu, at, kStateShape, 0, r))
        return {};
    // The quad's texture as it was.
    q1.v = 0.0f;
    q0.v = 0.0f;
    q0.u = 0.0f;
    q3.v = fromBits(kOne);
    q2.v = fromBits(kOne);
    q2.u = fromBits(kOne);
    q1.u = fromBits(kOne);
    q3.u = 0.0f;
    return advance;
}

/* 0x4d95c0 (stdcall 0x14): a character of a 2D screen's font drawn as the
 * screen quad: its square of the font's texture at eax, edx, [esp+4] wide
 * and [esp+8] high, at x ebx, y ecx, in the colour [esp+0xc] -- unless
 * [esp+0x14], which only measures it ([esp+0x10] the character, unread).
 * How far on the next character goes in eax: its width, less in the fonts
 * that overlap their characters.  Nothing on a race's pause screens, where it
 * is as wide as it is. */
void drawScreenCharacter(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             screenCharacter(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx),
                             int32_t(e.arg(app, 0)), int32_t(e.arg(app, 1)), e.arg(app, 2), e.arg(app, 4)),
             0x14);
}

/* ---- 0x4d9b90 ---- */

Result turnedCharacter(App* app, CPU& cpu, reg32 esp, int32_t u, int32_t v, int32_t x, int32_t y, int32_t width,
                       int32_t height, uint32_t colour, reg32 measure, int32_t turn, uint32_t scale)
{
    const reg32 at = esp - 0x30;  // esi, edi, ebp and 0x24 bytes
    HeldEbp held(cpu, esp - 0xc);
    x86::FPU& fpu = cpu.fpu;
    const float kU = fromBits(kTexel), kV = fromBits(kTexel);
    uint32_t topColour = colour, bottomColour = colour;
    int32_t scaledWidth, scaledHeight;
    if (kFont(app) == kAccentedFont)
    {
        // The accented font overlaps its characters by a pixel; the menus'
        // highlight (in its colour less a step) its own shadowed colour.
        const Float minusOne = Float(kMinusOne(app));
        const Float w = fp::add(Float(width), minusOne);
        const Float h = fp::add(minusOne, Float(height));
        const Float s = Float(fromBits(scale));
        const Float sw = fp::mul(w, s), sh = fp::mul(s, h);
        u += 1;
        v += 3;
        y += 4;
        scaledWidth = chopped<int32_t>(cpu, sw);
        scaledHeight = chopped<int32_t>(cpu, sh);
        if (colour == 0x1ffe43f)
        {
            bottomColour = 0xffd0ae28;
            topColour = 0xfffff180;
        }
    }
    else
    {
        const Float s = Float(fromBits(scale));
        const Float sw = fp::mul(Float(width), s), sh = fp::mul(s, Float(height));
        scaledWidth = chopped<int32_t>(cpu, sw);
        scaledHeight = chopped<int32_t>(cpu, sh);
    }
    if (kOverRace(app) != 0 || scaledHeight < 1)
        return scaledWidth;
    const float uf = ofInt(u), vf = ofInt(v);
    if (measure != 0)
        return scaledWidth;
    // Its top edge turned about its middle: the cosine of the turn, a 1024th
    // of the table's, of each half of its width, an eighth of a pixel's 1024th.
    const int32_t c = over(cosine(app, int32_t(int64_t(turn) % 0x400)), 10);
    const int32_t lean = over(int32_t(uint32_t(scaledWidth) * uint32_t(c)), 7);
    Vertex& q0 = kQuad(app, 0);
    Vertex& q1 = kQuad(app, 1);
    Vertex& q2 = kQuad(app, 2);
    Vertex& q3 = kQuad(app, 3);
    q3.colour = bottomColour;
    q2.colour = bottomColour;
    q1.colour = topColour;
    q0.colour = topColour;
    const int32_t left = x + half(scaledWidth) - lean;
    const int32_t right = half(scaledWidth + 1) + x + lean;
    const float depth = kScreenDepth(app), w = kScreenW(app);
    q3.z = depth;
    q3.w = viaX87(w);
    q1.y = ofInt(y);
    q2.z = viaX87(q3.z);
    q2.w = viaX87(q3.w);
    q0.y = viaX87(q1.y);
    q1.z = q2.z;
    q1.w = viaX87(q2.w);
    q0.z = q1.z;
    q0.w = viaX87(q2.w);
    q3.x = ofInt(left);
    q0.x = ofInt(left);
    const Float bottom = Float(y + scaledHeight);
    q2.x = ofInt(right);
    q3.y = float(bottom);
    q1.x = q2.x;
    q2.y = float(bottom);
    if ((kDriverFlags(app) & kSofttria) != 0)
        kNudge.set(app, 0);
    // The texture's corners, a 256th of the texture a texel.
    const Float u0 = fp::mul(Float(uf), Float(kU));
    const Float v0 = fp::mul(Float(vf), Float(kV));
    const Float u1 = fp::mul(Float(kU), fp::add(Float(uf), Float(width)));
    const Float v1 = fp::mul(fp::add(Float(vf), Float(height)), Float(kV));
    const float nudge = ofInt(kNudge(app));
    const Float x0 = fp::add(Float(q0.x), Float(nudge));
    const Float x1 = fp::add(Float(q1.x), Float(nudge));
    q3.u = float(u0);
    q1.v = float(v0);
    q0.x = float(x0);
    q1.x = float(x1);
    q0.u = viaX87(q3.u);
    q0.v = viaX87(q1.v);
    q2.u = float(u1);
    q1.u = float(u1);
    q3.v = float(v1);
    q2.v = float(v1);
    reg32 r;
    if ((kDriverFlags(app) & kSofttria) != 0 && kNudge(app) == 0 && !renderState(app, cpu, at, kStateShape, 1, r))
        return {};
    markCorners(app, kQuadAt);
    if (!drawQuad(app, cpu, at, kQuadAt, r))
        return {};
    if ((kDriverFlags(app) & kSofttria) == 0 && kFont(app) != kAccentedFont && !drawQuad(app, cpu, at, kQuadAt, r))
        return {};
    if ((kDriverFlags(app) & kSofttria) != 0 && kNudge(app) == 0 && !renderState(app, cpu, at, kStateShape, 0, r))
        return {};
    q1.v = 0.0f;
    q0.v = 0.0f;
    q0.u = 0.0f;
    q3.u = 0.0f;
    q3.v = fromBits(kOne);
    q2.v = fromBits(kOne);
    q2.u = fromBits(kOne);
    q1.u = fromBits(kOne);
    return scaledWidth;
}

/* 0x4d9b90 (stdcall 0x1c): that character turned and scaled: its square at
 * eax, edx, [esp+4] wide and [esp+8] high, at x ebx, y ecx, in the colour
 * [esp+0xc] ([esp+0x10] the character, unread), only measured for
 * [esp+0x14]; its top edge turned by the angle [esp+0x18] (0x400 a turn)
 * against its bottom, all of it scaled by the float [esp+0x1c].  How wide it
 * comes out. */
void drawTurnedCharacter(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             turnedCharacter(app, cpu, e.esp, int32_t(e.eax), int32_t(e.edx), int32_t(e.ebx), int32_t(e.ecx),
                             int32_t(e.arg(app, 0)), int32_t(e.arg(app, 1)), e.arg(app, 2), e.arg(app, 4),
                             int32_t(e.arg(app, 5)), e.arg(app, 6)),
             0x1c);
}

/* ---- 0x4da4a0, 0x4da120 ---- */

namespace
{

/* A character's number in the font's squares: the accented font's from
 * among the accented characters, the others' its code less 0x20. */
Result glyphOf(App* app, CPU& cpu, reg32 esp, uint8_t c)
{
    if (kFont(app) == kAccentedFont)
        return accented(app, cpu, esp, c);
    return int32_t(c) - 0x20;
}

}  // namespace

Result screenText(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y, reg32 measure)
{
    const reg32 at = esp - 0x1c;  // esi, edi, ebp and 0x10 bytes
    HeldEbp held(cpu, esp - 0xc);
    if (kOverRace(app) != 0)
        return 0;
    const uint32_t colour = kTextColour(app) | kTextColourMore(app);
    reg32 r;
    if (!renderState(app, cpu, at, kStateTexture, kFontTexture(app), r, reg32(y), measure))
        return {};
    const int32_t start = x;
    for (;; ++text)
    {
        const uint8_t c = app->getMemory<uint8_t>(text);
        if (c == 0)
            break;
        const Result g = glyphOf(app, cpu, at - 4, c);
        if (!g)
            return {};
        const Result advance =
            screenCharacter(app, cpu, at - 0x14 - 4, kGlyphU(app, *g), kGlyphV(app, *g), x, y, kGlyphWidth(app, *g),
                            kGlyphHeight(app, *g), colour, measure);
        if (!advance)
            return {};
        x += *advance;
    }
    return x - start;
}

/* 0x4da4a0: the text at eax drawn on a 2D screen at x edx, y ebx, each
 * character's square of the font, the next where that one's width leaves it
 * -- the text's width in eax, or 0 on a race's pause screens; ecx only
 * measures it.  The colour is [0x563aa4] | [0x559434]. */
void drawScreenText(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, screenText(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), e.ecx));
}

Result turnedText(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y, uint32_t scale, int32_t turn,
                  uint32_t colour)
{
    const reg32 at = esp - 0x1c;  // ecx, esi, edi, ebp and 0xc bytes
    HeldEbp held(cpu, esp - 0x10);
    if (kOverRace(app) != 0)
        return 0;
    reg32 r;
    if (!renderState(app, cpu, at, kStateTexture, kFontTexture(app), r, reg32(y)))
        return {};
    const int32_t start = x;
    for (;; ++text)
    {
        const uint8_t c = app->getMemory<uint8_t>(text);
        if (c == 0)
            break;
        const Result g = glyphOf(app, cpu, at - 4, c);
        if (!g)
            return {};
        const Result advance =
            turnedCharacter(app, cpu, at - 0x1c - 4, kGlyphU(app, *g), kGlyphV(app, *g), x, y, kGlyphWidth(app, *g),
                            kGlyphHeight(app, *g), colour, 0, turn, scale);
        if (!advance)
            return {};
        x += *advance;
    }
    return x - start;
}

/* 0x4da120 (stdcall 0xc): the text at eax drawn on a 2D screen at x edx, y
 * ebx, scaled by the float [esp+4], each character's top turned by the angle
 * [esp+8], in the colour [esp+0xc]: its width, or 0 on a race's pause
 * screens. */
void drawTurnedText(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu,
             turnedText(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), e.arg(app, 0), int32_t(e.arg(app, 1)),
                        e.arg(app, 2)),
             0xc);
}

/* ---- 0x4da1e0 ---- */

Result titleText(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y, reg32 start)
{
    const reg32 at = esp - 0x38;  // esi, edi, ebp and 0x2c bytes
    HeldEbp held(cpu, esp - 0xc);
    x86::FPU& fpu = cpu.fpu;
    if ((kDriverFlags(app) & kSofttria) != 0)
        return 0;
    if (kOverRace(app) != 0)
        return 1;
    reg32 r;
    if (!renderState(app, cpu, at, kStateTexture, kFontTexture(app), r, reg32(y), start))
        return {};
    const int32_t length = int32_t(std::strlen(&app->getMemory<char>(text)));
    if (start != 0)
        kSweep.set(app, length * 3 + 0x20);
    const int32_t sweep = kSweep(app);
    if (sweep <= -0xc)
        return 0;
    kSweep.set(app, sweep - 2);
    // Each character three steps behind the one before it, coming in over
    // twelve, turned and enlarged the less the nearer it is to its place.
    int32_t behind = length - 1;
    const reg32 callee = at - 0x1c - 4;
    for (;; ++text, --behind)
    {
        const uint8_t c = app->getMemory<uint8_t>(text);
        if (c == 0)
            return 1;
        const Result g = glyphOf(app, cpu, at - 4, c);
        if (!g)
            return {};
        const int32_t width = kGlyphWidth(app, *g), height = kGlyphHeight(app, *g);
        const int32_t first = kSweep(app) - behind * 3;
        for (int32_t step = first; step <= first + 0xc; step += 3)
        {
            const int32_t e = step < 0 ? 0 : step > 0x20 ? 0x20 : step;
            const int32_t turn = over(cosine(app, 0x100 - e * 8), 8);
            const uint32_t scale = bitsOf(float(fp::add(fp::mul(Float(e), Float(kEighth(app))), Float(1.0))));
            const int32_t fade = (0x40 - e * 2) << 24;
            uint32_t alpha;
            if (step != first)
                alpha = e != 0 ? uint32_t(fade) : 0;
            else if (e != 0)
                alpha = uint32_t(fade);
            else if (first <= 0 && first > -0xc)
                alpha = uint32_t(((-first) / 3) * 50 + 50) << 24;
            else
            {
                // In its place: its shadow first, two of them.
                const int32_t u = kGlyphU(app, *g), v = kGlyphV(app, *g);
                if (!turnedCharacter(app, cpu, callee, u, v, x + 4, y + 4, width, height, 0xff000040, 0, turn, scale)
                    || !turnedCharacter(app, cpu, callee, u, v, x + 2, y + 2, width, height, 0xff000008, 0, turn,
                                        scale))
                    return {};
                alpha = 0xff000000;
            }
            if (!turnedCharacter(app, cpu, callee, kGlyphU(app, *g), kGlyphV(app, *g), x, y, width, height,
                                 alpha + kHighlight, 0, turn, scale))
                return {};
            if ((kDriverFlags(app) & kSoftware) != 0)
                break;
        }
        x += width - 1;
    }
}

/* 0x4da1e0: the title at eax swept in on a 2D screen at x edx, y ebx, its
 * characters turned in one after another, fading to the highlight's colour
 * with their shadows once in place -- the sweep three steps a character from
 * the start (ecx not 0), two more each time: 1 while it lasts, 0 once it is
 * over and with softtria. */
void drawTitleText(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, titleText(app, cpu, e.esp, e.eax, int32_t(e.edx), int32_t(e.ebx), e.ecx));
}

/* ---- 0x4dab90 ---- */

Result pointer(App* app, CPU& cpu, reg32 esp)
{
    const reg32 at = esp - 0x20;  // ebx, ecx, edx, esi, edi, ebp and 8 bytes
    HeldEbp held(cpu, esp - 0x18);
    x86::FPU& fpu = cpu.fpu;
    const int32_t clock = kClock(app);
    if (kOverRace(app) != 0 || kPointerKind(app) == -2 || kPointersLoaded(app) == 0)
        return clock;
    reg32 picture = reg32(kPointerPicture(app));
    const reg32 kind = reg32(kPointerKind(app));
    if (app->getMemory<uint32_t>(kPointerTextures + 40 * kind + 4 * picture) == 0)
        picture = 0;
    const uint32_t texture = app->getMemory<uint32_t>(kPointerTextures + 40 * kind + 4 * picture);
    if (texture == 0)
        return int32_t(40 * kind);
    const reg32 spot = kPointerSpots + 80 * kind + 8 * picture;
    const int32_t x = int32_t(reg32(kPointerX(app)) - app->getMemory<reg32>(spot));
    const int32_t y = int32_t(reg32(kPointerY(app)) - app->getMemory<reg32>(spot + 4));
    Vertex& q0 = kQuad(app, 0);
    Vertex& q1 = kQuad(app, 1);
    Vertex& q2 = kQuad(app, 2);
    Vertex& q3 = kQuad(app, 3);
    q0.x = ofInt(x);
    const Float left = Float(q0.x);
    for (int32_t corner = 0; corner < 4; ++corner)
    {
        kQuad(app, corner).z = 0.0f;
        kQuad(app, corner).w = fromBits(kNearW);
        kQuad(app, corner).colour = 0xffffffff;
    }
    q3.x = float(left);
    q0.y = ofInt(y);
    const Float size = Float(kPointerSize(app));
    q1.x = float(fp::add(left, size));
    const Float top = Float(q0.y);
    q1.y = float(top);
    q2.x = viaX87(q1.x);
    const Float lower = fp::add(size, top);
    q2.y = float(lower);
    q3.y = float(lower);
    reg32 r;
    if (!renderState(app, cpu, at, kStateTexture, texture, r, reg32(x), reg32(y)))
        return {};
    markCorners(app, kQuadAt);
    if (!drawQuad(app, cpu, at, kQuadAt, r))
        return {};
    // The quad's depth and w for the screens' quads again (as the pause
    // screens have them, the other way round).
    for (int32_t corner = 0; corner < 4; ++corner)
    {
        kQuad(app, corner).z = fromBits(kNearW);
        kQuad(app, corner).w = fromBits(kNearDepth);
    }
    // The next picture a few ticks on.
    const int32_t next = int32_t(reg32(kPointerMoved(app)) + 3);
    if (next >= clock)
        return next;
    kPointerPicture.set(app, int32_t(reg32(kPointerPicture(app)) + 1) % 10);
    kPointerMoved.set(app, clock);
    return clock;
}

/* 0x4dab90: the menus' pointer of the kind [0x563d3c] (-2 none) drawn at the
 * pointer's place less its picture's hot spot, 32 pixels square, its picture
 * the next of ten every three ticks; not over a race, nor before the
 * pointers are loaded. */
void drawPointer(App* app, CPU& cpu)
{
    const Entry e(cpu);
    e.finish(cpu, pointer(app, cpu, e.esp));
}

/* ---- 0x4db000 ---- */

Result texturedQuad(App* app, CPU& cpu, reg32 esp, const ArtQuad& quad)
{
    const reg32 at = esp - 0xbc;  // esi, edi, ebp and 0xb0 bytes
    const reg32 corners = at;     // the quad's, in the frame
    HeldEbp held(cpu, esp - 0xc - 0x4a);
    // Its corners start as the code's template has them.
    std::memcpy(&app->getMemory<uint8_t>(corners), &app->getMemory<uint8_t>(0x4d5740), 0x80);
    if (kOverRace(app) == 0)
        return quad.y[1];
    x86::FPU& fpu = cpu.fpu;
    const Float sw = Float(kScreenWidth(app));
    const Float sh = Float(float(Float(kScreenHeight(app))));
    const Float perWidth = Float(kArtPerWidth(app)), perHeight = Float(kArtPerHeight(app));
    const Float x1 = fp::mul(fp::mul(Float(quad.x[1]), sw), perWidth);
    const Float y1 = fp::mul(fp::mul(Float(quad.y[1]), sh), perHeight);
    const Float x0 = fp::mul(fp::mul(Float(quad.x[0]), sw), perWidth);
    const Float x2 = fp::mul(fp::mul(Float(quad.x[2]), sw), perWidth);
    const Float y0 = fp::mul(fp::mul(Float(quad.y[0]), sh), perHeight);
    const Float y2 = fp::mul(fp::mul(Float(quad.y[2]), sh), perHeight);
    const Float x3 = fp::mul(fp::mul(sw, Float(quad.x[3])), perWidth);
    const Float y3 = fp::mul(fp::mul(Float(quad.y[3]), sh), perHeight);
    const Float xs[4] = {x0, x1, x2, x3}, ys[4] = {y0, y1, y2, y3};
    for (int32_t i = 0; i < 4; ++i)
    {
        Vertex& c = record<Vertex>(app, corners + 0x20 * i);
        c.x = float(xs[i]);
        c.y = float(ys[i]);
        c.u = fromBits(quad.u[i]);
        c.v = fromBits(quad.v[i]);
        c.colour = quad.colour;
    }
    reg32 r;
    if (!renderState(app, cpu, at, kStateTexture, quad.texture, r, reg32(quad.x[1]), reg32(quad.y[0])))
        return {};
    markCorners(app, corners);
    if (!drawQuad(app, cpu, at, corners, r))
        return {};
    return int32_t(r);
}

/* 0x4db000 (stdcall 0x38): a quad of the menus' art in 640 by 480 measure,
 * scaled to the screen: its corners at eax, edx; ebx, ecx; [esp+4], [esp+8];
 * [esp+0xc], [esp+0x10] -- their texture's places, floats, [esp+0x14] to
 * [esp+0x30] -- of the texture [esp+0x34], in the colour [esp+0x38];
 * clipped and drawn, over a race only.  Its vertices start as 0x4d5740 has
 * them. */
void drawTexturedQuad(App* app, CPU& cpu)
{
    const Entry e(cpu);
    ArtQuad quad;
    quad.x[0] = int32_t(e.eax);
    quad.y[0] = int32_t(e.edx);
    quad.x[1] = int32_t(e.ebx);
    quad.y[1] = int32_t(e.ecx);
    for (int i = 0; i < 2; ++i)
    {
        quad.x[2 + i] = int32_t(e.arg(app, 2 * i));
        quad.y[2 + i] = int32_t(e.arg(app, 2 * i + 1));
    }
    for (int i = 0; i < 4; ++i)
    {
        quad.u[i] = e.arg(app, 4 + 2 * i);
        quad.v[i] = e.arg(app, 5 + 2 * i);
    }
    quad.texture = e.arg(app, 12);
    quad.colour = e.arg(app, 13);
    e.finish(cpu, texturedQuad(app, cpu, e.esp, quad), 0x38);
}

/* ---- 0x4db380 ---- */

Result shadedQuad(App* app, CPU& cpu, reg32 esp, const ShadedQuad& quad)
{
    const reg32 at = esp - 0xc0;  // esi, edi, ebp and 0xb4 bytes
    const reg32 corners = at;
    HeldEbp held(cpu, esp - 0xc - 0x62);
    std::memcpy(&app->getMemory<uint8_t>(corners), &app->getMemory<uint8_t>(0x4d57c0), 0x80);
    for (int32_t i = 0; i < 4; ++i)
    {
        Vertex& c = record<Vertex>(app, corners + 0x20 * i);
        if (kOverRace(app) != 0)
        {
            c.z = fromBits(kNearDepth);
            c.w = fromBits(kNearW);
        }
        else
        {
            c.z = kScreenDepth(app);
            c.w = kScreenW(app);
        }
    }
    reg32 r;
    if ((kDriverFlags(app) & kSofttria) != 0 && !renderState(app, cpu, at, kStateAlpha, 1, r, reg32(quad.y[1])))
        return {};
    x86::FPU& fpu = cpu.fpu;
    const Float sh = Float(float(Float(kScreenHeight(app))));
    const Float sw = Float(kScreenWidth(app));
    const Float perWidth = Float(kShadePerWidth(app)), perHeight = Float(kShadePerHeight(app));
    const Float y1 = fp::mul(fp::mul(Float(quad.y[1]), sh), perHeight);
    const Float x0 = fp::mul(fp::mul(Float(quad.x[0]), sw), perWidth);
    const Float y0 = fp::mul(fp::mul(Float(quad.y[0]), sh), perHeight);
    const Float x1 = fp::mul(fp::mul(Float(quad.x[1]), sw), perWidth);
    const Float x2 = fp::mul(fp::mul(Float(quad.x[2]), sw), perWidth);
    const Float y2 = fp::mul(fp::mul(Float(quad.y[2]), sh), perHeight);
    const Float x3 = fp::mul(fp::mul(sw, Float(quad.x[3])), perWidth);
    const Float y3 = fp::mul(fp::mul(Float(quad.y[3]), sh), perHeight);
    const Float xs[4] = {x0, x1, x2, x3}, ys[4] = {y0, y1, y2, y3};
    for (int32_t i = 0; i < 4; ++i)
    {
        Vertex& c = record<Vertex>(app, corners + 0x20 * i);
        c.x = float(xs[i]);
        c.y = float(ys[i]);
        c.colour = quad.colour[i];
    }
    if (!renderState(app, cpu, at, kStateTexture, 0, r, reg32(quad.y[1])))
        return {};
    markCorners(app, corners);
    if (!drawQuad(app, cpu, at, corners, r))
        return {};
    if ((kDriverFlags(app) & kSofttria) != 0 && !renderState(app, cpu, at, kStateAlpha, 0, r))
        return {};
    return int32_t(r);
}

/* 0x4db380 (stdcall 0x20): a quad in 640 by 480 measure, scaled to the
 * screen: its corners at eax, edx; ebx, ecx; [esp+4], [esp+8]; [esp+0xc],
 * [esp+0x10] -- each in its own colour, [esp+0x14] to [esp+0x20] --
 * untextured, clipped and drawn; alpha on for it with softtria.  Its
 * vertices start as 0x4d57c0 has them, at a race's pause screens' depth over
 * one, else at the 2D screens'. */
void drawShadedQuad(App* app, CPU& cpu)
{
    const Entry e(cpu);
    ShadedQuad quad;
    quad.x[0] = int32_t(e.eax);
    quad.y[0] = int32_t(e.edx);
    quad.x[1] = int32_t(e.ebx);
    quad.y[1] = int32_t(e.ecx);
    for (int i = 0; i < 2; ++i)
    {
        quad.x[2 + i] = int32_t(e.arg(app, 2 * i));
        quad.y[2 + i] = int32_t(e.arg(app, 2 * i + 1));
    }
    for (int i = 0; i < 4; ++i)
        quad.colour[i] = e.arg(app, 4 + i);
    e.finish(cpu, shadedQuad(app, cpu, e.esp, quad), 0x20);
}

}  // namespace draw2d
}  // namespace port
}  // namespace nfs3hp
