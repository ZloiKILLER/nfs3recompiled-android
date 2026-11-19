#include "port_thrash.h"
#include "native_thrash.h"
#include "thrash.h"

#include <lib/renderer.h>
#include <lib/thrashrenderer.h>
#include <lib/window.h>
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

/* voodoo2a's own arithmetic -- its vertices, its fog table -- rounds as its
 * x87 code rounds: a product rounded to float straight from a fused
 * multiply-add would round once where that code rounds twice.  The port's own
 * sums on the picture keep the compiler's default (below). */
#if defined(__clang__)
#pragma clang fp contract(off)
#elif defined(_MSC_VER)
#pragma fp_contract(off)
#endif

/* voodoo2a, the THRASH driver of the Voodoo2, rewritten (port_thrash.h).
 *
 * voodoo2a drew through Glide, which the port used to emulate under it, and
 * set the card up through Win32: glide2x.dll loaded and its functions looked
 * up, DirectDraw for the screen on a card that shared one, the mode opened on
 * the game's window thread when the game asked for that.  None of that is
 * here.  Each export does what the game saw of it with the port's Glide under
 * it: the same records and variables in voodoo2a's data, the same results,
 * the same calls of the game's own functions it was handed, and the same
 * renderer calls in the same order.  Glide's part is folded
 * in: a call of the port's Glide that did nothing is gone, one that did is the
 * renderer call it made.
 *
 * What the Win32 calls left besides -- a handle for the event, a COM block for
 * DirectDraw's object, glide2x.dll's library -- is not taken any more, so the
 * handles and blocks of what the game opens later come out one apart from
 * before; the game only keeps them.
 *
 * The game was built against every THRASH driver, so it relies on what a
 * stdcall function promises and nothing more: the stack cleaned up, ebx, esi,
 * edi and ebp kept, a result in eax where there is one.  Calls voodoo2a
 * crashed in on the port -- a state set with no window open, Glide's renderer
 * pointers cleared by THRASH_is or THRASH_about under an open window -- do
 * nothing here. */

namespace nfs3hp
{
namespace port
{
namespace driver
{

namespace
{

using guest::at;
using guest::put;
using guest::byte;

/* ---- voodoo2a's data, at its load address (RVA + 0xa82200) ---- */

/* What the game is handed: THRASH_about's descriptor and its lists. */
constexpr u32 kInfo = 0xa91f54;            // thrash::kAbout*, thrash::kAboutBytes
constexpr u32 kTextureFormats = 0xa91fd4;  // the descriptor's formats, a dword each, 9 of them
constexpr u32 kUnknownList = 0xa92000;     // the descriptor's 4 of what is not known
constexpr u32 kModes = 0xa92018;           // 17 modes of thrash::kModeBytes, the first unused
constexpr u32 kResolutions = 0xa922e8;     // each mode's resolution: Glide's id, or the port's
/* voodoo2a's map of THRASH's formats to Glide's (its LOD and aspect maps,
 * kLods and kAspects, in port_thrash.h). */
constexpr u32 kFormats = 0xa92348;
/* The card as Glide described it, GrHwConfiguration: the boards, then the
 * first one's type, frame buffer megabytes, revision, texture units, SLI and
 * its first unit's revision and megabytes. */
constexpr u32 kHardware = 0xa93714;
/* Strings: the name, and the Glide versions THRASH_is weighs; Watcom's
 * character classes (_IsTable), a byte's at its value + 1. */
constexpr u32 kName = 0xa916d4;            // "3Dfx Voodoo"
constexpr u32 kVersionLow = 0xa916ec;      // "2.42"
constexpr u32 kVersionHigh = 0xa916f4;     // "2.5"
constexpr u32 kCharacterClasses = 0xa92578;

/* Its variables. */
constexpr Var<uint8_t> kFlags{0xa91ef4};     // kSetUp, kWindowOpen
constexpr Var<u32> kFlagsWord{0xa91ef4};
constexpr uint8_t kSetUp = 1, kWindowOpen = 2;
constexpr Var<u32> kRush{0xa91ef8};        // 1 on a card of type 1, which shares the screen: never here
constexpr Var<u32> kClearColour{0xa91efc}; // thrash::kStateClearColour
constexpr Var<uint16_t> kClearDepth{0xa91f00};
constexpr Var<u32> kClearDepthWord{0xa91f00};
constexpr Var<u32> kBuffer{0xa91f04};      // the buffer drawn into, THRASH_window's less one
constexpr Var<u32> kWindow{0xa91f08};      // the game's window: thrash::kStateWindow, or the lambda's
constexpr Var<u32> kSwapInterval{0xa91f0c};// thrash::kStateFlipRate
constexpr Var<u32> kTakeScreen{0xa91f10};  // thrash::kState107: voodoo2a took the screen by DirectDraw by it
constexpr Var<u32> kWidth{0xa91f18};       // the window's size
constexpr Var<u32> kHeight{0xa91f1c};
constexpr Var<u32> kSmooth{0xa91f20};      // thrash::kStateAntiAlias
constexpr Var<u32> kTwoPass{0xa91f24};     // trilinear mipmapping, thrash::kMipLinear
constexpr Var<u32> kTextures{0xa91f28};    // the last record THRASH_talloc made; each points at the one before
constexpr Var<u32> kCombineLocal{0xa91f2c};// thrash::kStateShade's
constexpr Var<u32> kFindWindow{0xa91f30};  // the game's lambdas, stdcall functions of its own (thrash::kLambda*)
constexpr Var<u32> kLog{0xa91f34};
constexpr Var<u32> kLockHook{0xa91f38};    // told 1 before a lock of the frame buffer and 0 after
constexpr Var<u32> kExecute{0xa91f3c};     // what ran voodoo2a's mode set on the window's thread
constexpr Var<u32> kWindowMode{0xa91f44};  // thrash::kStateWindowMode
constexpr Var<u32> kTexturing{0xa92354};   // 1 while a texture is selected, 0 while none; -1 at first
constexpr Var<u32> kTextureNext{0xa937a8}; // the next free address of texture memory
constexpr u32 kDefaultLog = 0xa83a80;      // sub_a83a80, voodoo2a's own log, the lambda's default

/* voodoo2a's constants: z to Glide's 1/z, w to the scale of s and t, and its
 * fog table's top. */
constexpr float kDepthScale = 65536.0f;      // [0xa9235c]
constexpr float kTexelScale = 256.0f;        // [0xa92358]
constexpr float kFogScale = 255.0f;          // [0xa916cc]

/* Texture memory as the port's Glide gave it out: each texture takes one slot
 * of it, whatever its size, so that each address is one texture's -- the key
 * ThrashRenderer keeps the texture under. */
constexpr u32 kTextureSlot = 8;
constexpr u32 kTextureMemoryLow = kTextureSlot;     // grTexMinAddress(0)
constexpr u32 kTextureMemoryHigh = 16 * 1024 * 1024;  // grTexMaxAddress(0)

/* ---- The picture ---- */

nfs::Renderer* s_screen = nullptr;        // the window, and the frame buffer the game locks
nfs::ThrashRenderer* s_renderer = nullptr;
SwapObserver s_swapObserver = nullptr;
std::vector<GrVertex>* s_trace = nullptr;
u32 s_dynamicWidth = 1920, s_dynamicHeight = 1080;  // kResolutionDynamic's, until it is set
/* fitFourThree: x becomes s_fitOffset + x * s_fitScale for everything drawn;
 * squeezeToLeft: each triangle narrowed by s_squeezeScale. */
float s_fitScale = 1.0f, s_fitOffset = 0.0f, s_squeezeScale = 1.0f;
/* The clip window as THRASH_clip last set it, put back after clearPicture. */
u32 s_clipWindow[4];
bool s_clipWindowSet = false;

/* The window, opened: grSstWinOpen as the port's Glide had it -- the
 * resolution's size (Glide's, the port's own ids, or 640x480 for one it does
 * not know), a Renderer on the game's window at it, ThrashRenderer on that,
 * and at address 0 the texture of one texel an untextured draw selects. */
void openPicture(App* app, u32 window, u32 resolution)
{
    static const struct { u32 width, height; } kGlideSizes[] = {
        {320, 200}, {320, 240}, {400, 256}, {512, 384}, {640, 200}, {640, 350}, {640, 400},
        {640, 480}, {800, 600}, {960, 720}, {856, 480}, {512, 256}, {1024, 768},
    };
    u32 width = 640, height = 480;
    if (resolution < sizeof kGlideSizes / sizeof kGlideSizes[0])
    {
        width = kGlideSizes[resolution].width;
        height = kGlideSizes[resolution].height;
    }
    else if (resolution == kResolution1280x720)
    {
        width = 1280;
        height = 720;
    }
    else if (resolution == kResolution1600x900)
    {
        width = 1600;
        height = 900;
    }
    else if (resolution == kResolution1920x1080)
    {
        width = 1920;
        height = 1080;
    }
    else if (resolution == kResolutionDynamic)
    {
        width = s_dynamicWidth;
        height = s_dynamicHeight;
    }
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Unsupported resolution: %d", resolution);
    }
    s_screen = new nfs::Renderer(app, dynamic_cast<nfs::Window*>(app->getResource(window)));
    s_screen->setVideoMode(width, height, 16);
    app->allocateResource(s_screen);
    s_renderer = new nfs::ThrashRenderer(s_screen);
    app->allocateResource(s_renderer);
    const uint16_t texel = 0xff;
    s_renderer->setTextureData(0, 0, &texel, 8, 8, nfs::TF_ARGB_4444);
}

/* The window let go: grSstWinClose, and grGlideShutdown. */
void closePicture(App* app)
{
    if (s_renderer)
    {
        app->freeResource(s_renderer->getResourceIndex());
        s_renderer = nullptr;
    }
    if (s_screen)
    {
        app->freeResource(s_screen->getResourceIndex());
        s_screen = nullptr;
    }
}

/* ---- The game's vertices ---- */

/* One of the game's vertices as voodoo2a reads it: x and y, Glide's 1/z (z
 * times kDepthScale), s/w and t/w (u and v times w times kTexelScale), 1/w as
 * it is, and the colour as bytes, r | g << 8 | b << 16 | a << 24.  voodoo2a's
 * code did its x87 arithmetic in the FPU's own type
 * without the precision control's rounding, so this does too,
 * operation by operation, in the same order: in plain double, where each
 * product of floats is exact and only the one of three factors and the stores
 * to float round.  Its colour table only ever held
 * each byte's own value, so the bytes are taken as they are. */
struct Vertex
{
    float x, y, ooz, sow, tow;
    u32 oow;  // the bits as they are
    u32 colour;
};

Vertex readVertex(App* app, u32 in)
{
    typedef nfs::Float F;
    const F zk = F(app->getMemory<float>(in + thrash::kVertexZ)) * F(kDepthScale);
    const F wk = F(app->getMemory<float>(in + thrash::kVertexRhw)) * F(kTexelScale);
    Vertex out;
    out.x = float(F(app->getMemory<float>(in + thrash::kVertexX)));
    out.y = float(F(app->getMemory<float>(in + thrash::kVertexY)));
    out.oow = at(app, in + thrash::kVertexRhw);
    out.sow = float(F(app->getMemory<float>(in + thrash::kVertexU)) * wk);
    out.tow = float(wk * F(app->getMemory<float>(in + thrash::kVertexV)));
    out.ooz = float(zk);
    const u32 bgra = at(app, in + thrash::kVertexColour);
    out.colour = (bgra & 0xff00ff00) | (bgra >> 16 & 0xff) | (bgra & 0xff) << 16;
    return out;
}

/* The vertex in a slot of the renderer's batch: u and v before
 * finishTriangle brings far ones back. */
void toBatch(const Vertex& v, nfs::ThrashVertex& out)
{
    out.x = v.x;
    out.y = v.y;
    out.z = v.ooz;
    out.color = v.colour;
    out.u = v.sow;
    out.v = v.tow;
    std::memcpy(&out.oow, &v.oow, sizeof out.oow);
}

/* The vertex as Glide's, for the trace. */
GrVertex traced(const Vertex& v)
{
    GrVertex out = {};
    out.x = v.x;
    out.y = v.y;
    std::memcpy(&out.oow, &v.oow, sizeof out.oow);
    out.a = float(v.colour >> 24);
    out.r = float(v.colour & 0xff);
    out.g = float(v.colour >> 8 & 0xff);
    out.b = float(v.colour >> 16 & 0xff);
    out.tmuvtx[0].sow = v.sow;
    out.tmuvtx[0].tow = v.tow;
    out.ooz = v.ooz;
    return out;
}

}  // namespace
}  // namespace driver
}  // namespace port
}  // namespace nfs3hp

/* The port's own arithmetic on the picture -- the 4:3 fit, the emblem's
 * narrowing, a line's width -- with the compiler's default contraction, as
 * glide2x.cpp had it, so that the same expressions round as they did. */
#if defined(__clang__)
#pragma clang fp contract(on)
#endif

namespace nfs3hp
{
namespace port
{
namespace driver
{

void fitFourThree(bool on)
{
    s_fitScale = 1.0f;
    s_fitOffset = 0.0f;
    if (!on || !s_screen)
        return;
    const float width = float(s_screen->getWidth()), height = float(s_screen->getHeight());
    if (height <= 0 || width * 3 <= height * 4)
        return;
    s_fitScale = height * 4 / (width * 3);
    s_fitOffset = (width - height * 4 / 3) / 2;
}

/* Each triangle narrowed by the 4:3 factor towards its own leftmost corner, so
 * a picture laid out for 640x480 and stretched over a wide one -- the NFS
 * emblem of a dialog box -- keeps its shape where it stands.  Both triangles
 * of a quad hold one of its left corners, so the quad narrows as one. */
void squeezeToLeft(bool on)
{
    s_squeezeScale = 1.0f;
    if (!on || !s_screen)
        return;
    const float width = float(s_screen->getWidth()), height = float(s_screen->getHeight());
    if (height > 0 && width * 3 > height * 4)
        s_squeezeScale = height * 4 / (width * 3);
}

namespace
{

/* A batch slot's x fitted to the 4:3 rectangle, while fitFourThree is on. */
void fit(nfs::ThrashVertex& v)
{
    if (s_fitScale == 1.0f && s_fitOffset == 0.0f)
        return;
    v.x = s_fitOffset + v.x * s_fitScale;
}

/* A triangle in the batch narrowed, then fitted. */
void place(nfs::ThrashVertex* out)
{
    if (s_squeezeScale != 1.0f)
    {
        const float left = std::min(std::min(out[0].x, out[1].x), out[2].x);
        for (int i = 0; i < 3; ++i)
            out[i].x = left + (out[i].x - left) * s_squeezeScale;
    }
    for (int i = 0; i < 3; ++i)
        fit(out[i]);
}

void batchTriangle(const nfs::ThrashVertex& a, const nfs::ThrashVertex& b, const nfs::ThrashVertex& c)
{
    nfs::ThrashVertex* out = s_renderer->reserveTriangle();
    out[0] = a;
    out[1] = b;
    out[2] = c;
    s_renderer->finishTriangle(out);
}

/* grDrawLine as the port's Glide drew it: fitted, then four triangles, a
 * pixel wide on each side, the outer corners clear. */
void batchLine(nfs::ThrashVertex a, nfs::ThrashVertex b)
{
    fit(a);
    fit(b);
    float orthX = a.y - b.y;
    float orthY = b.x - a.x;
    float len = std::sqrt(orthX * orthX + orthY * orthY);
    orthX /= len / 2;
    orthY /= len / 2;
    nfs::ThrashVertex a1 = a;
    a1.color &= 0x00ffffff;
    a1.x -= orthX;
    a1.y -= orthY;
    nfs::ThrashVertex a2 = a;
    nfs::ThrashVertex a3 = a;
    a3.color &= 0x00ffffff;
    a3.x += orthX;
    a3.y += orthY;
    nfs::ThrashVertex b1 = b;
    b1.color &= 0x00ffffff;
    b1.x -= orthX;
    b1.y -= orthY;
    nfs::ThrashVertex b2 = b;
    nfs::ThrashVertex b3 = b;
    b3.color &= 0x00ffffff;
    b3.x += orthX;
    b3.y += orthY;
    batchTriangle(a1, b1, a2);
    batchTriangle(b1, b2, a2);
    batchTriangle(a2, b2, a3);
    batchTriangle(b2, b3, a3);
}

}  // namespace
}  // namespace driver
}  // namespace port
}  // namespace nfs3hp

#if defined(__clang__)
#pragma clang fp contract(off)
#endif

namespace nfs3hp
{
namespace port
{
namespace driver
{

namespace
{

/* ---- Drawing ---- */

/* How voodoo2a draws, by its two flags (sub_a848e0 pointed its drawing so):
 * Glide's own triangles and lines; antialiased ones (thrash::kStateAntiAlias),
 * of which the port's Glide drew nothing; or trilinear mipmapping in two
 * passes (thrash::kMipLinear), each triangle twice and no lines. */
struct Drawing
{
    bool twoPass, smooth;
    explicit Drawing(App* app) : twoPass(kTwoPass(app) != 0), smooth(kSmooth(app) != 0) {}
};

/* grDrawTriangle(a, b, c): traced, then narrowed and fitted into the batch. */
void plainTriangle(App* app, u32 a, u32 b, u32 c)
{
    const Vertex v[3] = {readVertex(app, a), readVertex(app, b), readVertex(app, c)};
    if (s_trace)
        for (const Vertex& corner : v)
            s_trace->push_back(traced(corner));
    if (!s_renderer)
        return;
    nfs::ThrashVertex* out = s_renderer->reserveTriangle();
    for (int i = 0; i < 3; ++i)
        toBatch(v[i], out[i]);
    place(out);
    s_renderer->finishTriangle(out);
}

/* A triangle as voodoo2a's drawing draws it.  In two passes (sub_a83930,
 * sub_a839b0): the texture weighted by its LOD fraction -- a blend the port's
 * Glide took as nothing -- then by one less it, added on, which it took as the
 * destination blended whole from then on. */
void triangle(App* app, const Drawing& d, u32 a, u32 b, u32 c)
{
    if (!d.twoPass)
    {
        if (!d.smooth)
            plainTriangle(app, a, b, c);
        return;
    }
    if (!d.smooth)
        plainTriangle(app, a, b, c);
    if (s_renderer)
        s_renderer->setAlphaBlendDst(nfs::AB_1);
    if (!d.smooth)
        plainTriangle(app, a, b, c);
}

/* A quad: triangles a b c and c d a. */
void quad(App* app, const Drawing& d, u32 a, u32 b, u32 c, u32 e)
{
    triangle(app, d, a, b, c);
    triangle(app, d, c, e, a);
}

/* A line, drawn only as Glide's own: none in two passes (sub_a83a30, a bare
 * ret), and grAADrawLine drew nothing. */
void line(App* app, const Drawing& d, u32 a, u32 b)
{
    if (d.twoPass || d.smooth)
        return;
    const Vertex from = readVertex(app, a), to = readVertex(app, b);
    if (s_trace)
    {
        s_trace->push_back(traced(from));
        s_trace->push_back(traced(to));
        s_trace->push_back(traced(to));
    }
    if (!s_renderer)
        return;
    nfs::ThrashVertex ends[2];
    toBatch(from, ends[0]);
    toBatch(to, ends[1]);
    batchLine(ends[0], ends[1]);
}

/* The n-th of a mesh's vertices: an index times the vertex's size. */
u32 meshVertex(App* app, u32 vertices, u32 index)
{
    return vertices + (at(app, index) << 5);
}

/* ---- Textures ---- */

/* THRASH_talloc(width, height, format, palette, levels): a record of Glide's
 * LODs, aspect and format, where it lies in texture memory, 3 (both halves
 * of the mipmap) and the record before -- from voodoo2a's own malloc, which
 * THRASH_treset gives them back to; 0 when it does not fit.  It starts in the
 * next 2 MB bank if it would straddle one. */
Result allocateTexture(App* app, ThreadState& ts, u32 width, u32 height, u32 format, u32 levelCount)
{
    const int32_t widthLog = exponentOf(width);
    const int32_t heightLog = exponentOf(height);
    const int32_t levels = int32_t(levelCount);
    const int32_t largest = widthLog > heightLog ? widthLog : heightLog;
    int32_t smallest = largest - levels;
    if (smallest < 0)
        smallest = 0;
    const u32 largeLod = lodOf(app, largest);
    const u32 smallLod = lodOf(app, smallest);
    const u32 aspect = byte(app, kAspects + u32(widthLog - heightLog));
    const u32 size = kTextureSlot;
    u32 start = kTextureNext(app);
    const u32 firstBank = (start - kTextureMemoryLow) & 0xffe00000;
    const u32 lastBank = (size + start - 1 - kTextureMemoryLow) & 0xffe00000;
    if (lastBank != firstBank)
    {
        start = kTextureMemoryLow + lastBank;
        kTextureNext.set(app, start);
    }
    const u32 top = (kTextureMemoryHigh + 0xf) & ~u32(0xf);
    u32 record = 0;
    if (int32_t(top - start) >= int32_t(size))
    {
        record = thrashMalloc(app, ts, kRecordBytes);
        if (ts.terminate)
            return {};
    }
    if (record != 0)
    {
        put(app, record + kRecordLargeLod, largeLod);
        put(app, record + kRecordSmallLod, smallLod);
        put(app, record + kRecordAspect, aspect);
        put(app, record + kRecordFormat, byte(app, kFormats + format));
        put(app, record + kRecordTexels, 0);
        put(app, record + kRecordHalves, 3);
        put(app, record + kRecordStart, start);
        kTextureNext.set(app, start + size);
        put(app, record + kRecordBefore, kTextures(app));
        kTextures.set(app, record);
    }
    return int32_t(record);
}

/* THRASH_treset, and a window opening: every texture given up, texture memory
 * free again, every record given back. */
bool resetTextures(App* app, ThreadState& ts)
{
    if (s_renderer)
        s_renderer->resetTextures();
    kTextureNext.set(app, kTextureMemoryLow);
    while (const u32 record = kTextures(app))
    {
        const u32 before = at(app, record + kRecordBefore);
        thrashFree(app, ts, record);
        if (ts.terminate)
            return false;
        kTextures.set(app, before);
    }
    return true;
}

/* THRASH_tupdate(record, texels, palette): new texels for a record, to the
 * renderer in their format -- grTexDownloadMipMap's four the port took, the
 * rest nothing -- with the guest context let go meanwhile.  A palette went to
 * a table the port's Glide did not keep. */
u32 updateTexture(App* app, ThreadState& ts, u32 record, u32 texels)
{
    if (texels == 0)
        return record;
    put(app, record + kRecordTexels, texels);
    if (!s_renderer)
        return record;
    app->unlockContext(ts);
    const u32 format = at(app, record + kRecordFormat);
    const u32 start = at(app, record + kRecordStart);
    const void* data = &app->getMemory<const nfs::u8>(texels);
    const u32 largeLod = at(app, record + kRecordLargeLod), smallLod = at(app, record + kRecordSmallLod);
    if (format == 0xa)  // GR_TEXFMT_RGB_565
        s_renderer->setTextureData(0, start, data, largeLod, smallLod, nfs::TF_RGB_565);
    else if (format == 0xb)  // GR_TEXFMT_ARGB_1555
        s_renderer->setTextureData(0, start, data, largeLod, smallLod, nfs::TF_ARGB_1555);
    else if (format == 0xc)  // GR_TEXFMT_ARGB_4444
        s_renderer->setTextureData(0, start, data, largeLod, smallLod, nfs::TF_ARGB_4444);
    else if (format == kTexFmtArgb8888)
        s_renderer->setTextureData(0, start, data, largeLod, smallLod, nfs::TF_ARGB_8888);
    app->lockContext(ts);
    return record;
}

/* THRASH_settexture(record), and THRASH_setstate 1: the texture drawn with,
 * or none -- the colour and alpha factors following, the texture times the
 * vertex colour or the vertex colour alone. */
void selectTexture(App* app, u32 record)
{
    if (record == 0)
    {
        if (kTexturing(app) != 0)
        {
            if (s_renderer)
            {
                s_renderer->setColorFactor(0.f);
                s_renderer->setTexture(0, 0);
                s_renderer->setAlphaFactor(0.f);
            }
            kTexturing.set(app, 0);
        }
        return;
    }
    if (s_renderer)
        s_renderer->setTexture(0, at(app, record + kRecordStart));
    if (kTexturing(app) != 1)
    {
        if (s_renderer)
        {
            s_renderer->setColorFactor(1.f);
            s_renderer->setAlphaFactor(1.f);
        }
        kTexturing.set(app, 1);
    }
}

/* ---- State ---- */

/* sub_a83830 and sub_a83870: Glide's fog table for a density -- 1 - e^(-d w)
 * at each of its 64 w, scaled to 1 at the last, times 255 -- worked out as
 * voodoo2a's code works it out, x87 operation by operation in double.
 * w(i) = 2^(3 + i/4) / (8 - i%4), exact (Watcom's pow of 2 with a whole
 * exponent). */
double fogW(int i)
{
    return std::ldexp(1.0, 3 + (i >> 2)) / double(8 - (i & 3));
}

double fogExp(ThreadState& ts, double w, float density)
{
    const double x = -(w * double(density));
    const double t = 1.4426950408889634 * x;
    const double fraction = fp::rem(t, 1.0);
    return fp::scale(1.0 + fp::f2xm1(fraction), t);
}

void fogTable(ThreadState& ts, float density, uint8_t (&table)[64])
{
    const float scale = float(1.0 / (1.0 - fogExp(ts, fogW(63), density)));
    for (int i = 0; i < 64; ++i)
    {
        float value = float((1.0 - fogExp(ts, fogW(i), density)) * double(scale));
        u32 bits;
        std::memcpy(&bits, &value, sizeof bits);
        if (int32_t(bits) > 0x3f800000)
            value = 1.0f;
        else if (0.0 > double(value))  // fcomp leaves a NaN as it is
            value = 0.0f;
        const double scaled = double(value) * double(kFogScale);
        table[i] = uint8_t(fp::toInteger<int32_t>(scaled));
    }
}

/* NFS_CULL=0/1/2 overrides the game's culling, for A/B tests on a GPU that
 * takes the other winding as the front; read once. */
u32 culling(u32 mode)
{
    static const int overrideMode = []() {
        const char* value = std::getenv("NFS_CULL");
        if (!value)
            return -1;
        char* end = nullptr;
        const unsigned long chosen = std::strtoul(value, &end, 0);
        return end != value && *end == '\0' && chosen <= 2 ? int(chosen) : -1;
    }();
    return overrideMode >= 0 ? u32(overrideMode) : mode;
}

float asFloat(u32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* The blend: the destination by one less the source's alpha, or whole; a
 * source factor the port's Glide did not take (1, 4, 4, 0 aside) leaves it as
 * it was. */
void blend(u32 destination)
{
    if (!s_renderer)
        return;
    if (destination == 5)  // GR_BLEND_ONE_MINUS_SRC_ALPHA
        s_renderer->setAlphaBlendDst(nfs::AB_1mSrcAlpha);
    else if (destination == 4)  // GR_BLEND_ONE
        s_renderer->setAlphaBlendDst(nfs::AB_1);
}

}  // namespace

/* THRASH_setstate(state, value): one piece of the driver's state, to the
 * renderer or to voodoo2a's variables.  1 for done, 0 for a state or value it
 * does not take. */
u32 setState(App* app, ThreadState& ts, u32 state, u32 value)
{
    nfs::ThrashRenderer* const r = s_renderer;
    u32 result = 1;
    switch (state)
    {
    case thrash::kStateTexture:
        selectTexture(app, value);
        break;
    case thrash::kStateCull:
        if (r)
            r->setCullMode(culling(value));
        break;
    case thrash::kStateClearColour:
        kClearColour.set(app, value);
        break;
    case thrash::kStateDepth:
        // Off: compared always, not written.  On, z or w: compared
        // less-or-equal and written, cleared to the farthest.
        if (value == thrash::kDepthOff)
        {
            if (r)
            {
                r->setZTest(true);
                r->setDepthAlways(true);
                r->setZWrite(false);
            }
        }
        else if (value == thrash::kDepthZ || value == thrash::kDepthW)
        {
            if (r)
            {
                r->setZTest(true);
                r->setZTest(true);
                r->setDepthAlways(false);
            }
            kClearDepthWord.set(app, 0xffff);
            if (r)
                r->setZWrite(true);
        }
        else
        {
            result = 0;
        }
        break;
    case thrash::kStateDither:
        if (r)
            r->setDither(value != 0);
        break;
    case thrash::kStateShade:  // the vertex colour alone, or modulating the texture
    {
        kCombineLocal.set(app, value == thrash::kShadeFlat ? 1 : 0);
        if (!r)
            break;
        if (kTexturing(app) == 1)
        {
            r->setColorFactor(1.f);
            r->setAlphaFactor(1.f);
        }
        else
        {
            r->setColorFactor(0.f);
            r->setTexture(0, 0);
            r->setAlphaFactor(0.f);
        }
        break;
    }
    case thrash::kStateTextureFilter:  // always bilinear here
    case thrash::kStateLodBias:        // the levels are the GPU's to pick
        break;
    case thrash::kStateAntiAlias:
        kSmooth.set(app, value);
        break;
    case thrash::kStateTransparency:  // the chroma key; the alpha test is always greater
        if (value == thrash::kTransparencyNone)
        {
            if (r)
                r->setChromakeyMode(false);
        }
        else if (value == thrash::kTransparencyChromaKey || value == thrash::kTransparencyAlpha)
        {
            if (r)
                r->setChromakeyMode(value == thrash::kTransparencyChromaKey);
        }
        else
        {
            result = 0;
        }
        break;
    case thrash::kStateMipFilter:  // kMipLinear is kMipPoint in two passes
    {
        u32 twoPass = 0;
        if (value == thrash::kMipLinear)
            twoPass = 1;  // THRASH_setstate writes kMipPoint over the argument
        if (twoPass != kTwoPass(app))
        {
            kTwoPass.set(app, twoPass);
            blend(5);
        }
        break;
    }
    case thrash::kStateChromaKey:
        if (r)
            r->setChromakeyValue(value);
        break;
    case thrash::kStateTextureAddress:
        if (r)
            r->setTexClampMode(value != thrash::kAddressClamp, value != thrash::kAddressClamp);
        break;
    case thrash::kStateFogDensity:  // fog by density -- 1 / value -- or off
        if (value == 0)
        {
            if (r)
                r->setFogMode(0);
        }
        else
        {
            if (r)
                r->setFogMode(2);  // GR_FOG_WITH_TABLE
            uint8_t table[64];
            fogTable(ts, float(1.0 / double(int32_t(value))), table);
            if (r)
                r->setFogTable(table);
            result = 0;  // what voodoo2a gives back here
        }
        break;
    case thrash::kStateFogColour:
        if (r)
            r->setFogColor(value);
        break;
    case thrash::kStateWindowMode:
        kWindowMode.set(app, value);
        break;
    case thrash::kStateLambdas:  // four of a record at once, or all four cleared
        if (value != 0)
        {
            kFindWindow.set(app, at(app, value + thrash::kLambdaAcquireWindow));
            kExecute.set(app, at(app, value + thrash::kLambdaExecute));
            kLog.set(app, at(app, value + thrash::kLambdaLog));
            kLockHook.set(app, at(app, value + thrash::kLambdaLockWindow));
        }
        else
        {
            kExecute.set(app, 0);
            kLog.set(app, 0);
            kLockHook.set(app, 0);
            kFindWindow.set(app, 0);
        }
        break;
    case thrash::kStateFog:
        if (value == thrash::kFogOff)
        {
            if (r)
                r->setFogMode(0);
        }
        else if (value == thrash::kFogOn || value == thrash::kFogLinear || value == thrash::kFogExp
                 || value == thrash::kFogExp2)
        {
            if (r)
                r->setFogMode(2);
        }
        else
        {
            result = 0;
        }
        break;
    case thrash::kStateDepthBias:
    case thrash::kStateDepthBiasAlternative:  // the port's Glide kept no bias
        break;
    case thrash::kStateWindow:
        kWindow.set(app, value);
        break;
    case thrash::kStateLogLambda:
        kLog.set(app, value);
        break;
    case thrash::kStateExecuteLambda:
        kExecute.set(app, value);
        break;
    case thrash::kStateLockWindowLambda:
        kLockHook.set(app, value);
        break;
    case thrash::kStateGamma:
        if (r)
            r->setGamma(asFloat(value));
        break;
    case thrash::kStateFlipRate:
        kSwapInterval.set(app, value);
        break;
    case thrash::kStateBlend:
        switch (value)
        {
        case thrash::kBlendSourceAlpha: blend(5); break;
        case thrash::kBlendSourceAlphaOne: blend(4); break;
        case thrash::kBlendZeroInverseSourceAlpha: blend(5); break;
        case thrash::kBlendDestinationColourZero: break;  // a blend the port's Glide did not take
        default: result = 0; break;
        }
        break;
    case thrash::kStateFogAlphas:  // fog from the game's own table, or off
        if (value == 0)
        {
            if (r)
                r->setFogMode(0);
        }
        else if (r)
        {
            r->setFogMode(2);
            r->setFogTable(&app->getMemory<nfs::u8>(value));
        }
        break;
    case thrash::kStateDepthWrite:
        if (r)
            r->setZWrite(value == 1);
        break;
    case thrash::kState107:
        kTakeScreen.set(app, value);
        break;
    case thrash::kState108:  // Glide's grSstControl, which the port's did not take
        if (kRush(app) != 0)
            result = 0;
        break;
    default:
        result = 0;
        break;
    }
    return result;
}

namespace
{

/* ---- The window ---- */

/* grSstQueryHardware as the port's Glide answered it: one Voodoo2, 16 MB of
 * frame buffer, revision 222, one texture unit of 16 MB, revision 1. */
void queryHardware(App* app)
{
    put(app, kHardware, 1);
    put(app, kHardware + 4, 3);  // GR_SSTTYPE_Voodoo2
    put(app, kHardware + 8, 16);
    put(app, kHardware + 0xc, 222);
    put(app, kHardware + 0x10, 1);
    put(app, kHardware + 0x14, 0);
    put(app, kHardware + 0x18, 1);
    put(app, kHardware + 0x1c, 16);
}

/* sub_a83ac0: from the card, how many colour and auxiliary buffers each mode
 * can have in the frame buffer's memory, and the descriptor's card type and
 * texture memory. */
void measureModes(App* app, u32 info)
{
    int32_t memory = int32_t(at(app, kHardware + 8) << 20);
    const u32 type = at(app, kHardware + 4);
    const u32 rush = type == 1 ? 1 : 0;
    kRush.set(app, rush);
    u32 sli = 0;
    if (rush == 0)
        sli = at(app, kHardware + 0x14);
    if (sli != 0)
        memory += memory;
    u32 mode = kModes + thrash::kModeBytes;
    for (int i = 1; i <= 16; ++i, mode += thrash::kModeBytes)
    {
        const u32 colourBuffers = mode + thrash::kModeColourBuffers, auxBuffers = mode + thrash::kModeAuxBuffers;
        const int32_t pixels = int32_t(at(app, mode + thrash::kModeWidth) * at(app, mode + thrash::kModeHeight));
        const int32_t frame = (int32_t(at(app, mode + thrash::kModeBits)) >> 3) * pixels;
        put(app, colourBuffers, u32(memory / frame));
        if (at(app, mode + thrash::kModeWidth) == 0x3c0 && sli == 0)
            put(app, colourBuffers, 0);
        if (int32_t(at(app, mode + thrash::kModeWidth)) > 0x320 && kRush(app) != 0)
            put(app, colourBuffers, 0);
        if (at(app, mode + thrash::kModeAvailable) == 0 || int32_t(at(app, colourBuffers)) <= 0)
        {
            put(app, auxBuffers, 0);
            put(app, colourBuffers, 0);
            continue;
        }
        put(app, auxBuffers, at(app, colourBuffers) - 1);
        if (int32_t(at(app, colourBuffers)) > 3)
            put(app, colourBuffers, 3);
        if (int32_t(at(app, auxBuffers)) > 3)
            put(app, auxBuffers, 3);
    }
    put(app, info + thrash::kAboutSubType, type);
    if (type == 0)
    {
        const u32 revision = at(app, kHardware + 0xc);
        if ((int32_t(revision) >> 8) == 1)
            put(app, info + thrash::kAboutSubType, 2);
        if ((int32_t(revision) >> 12) == 1)
            put(app, info + thrash::kAboutSubType, 3);
        if (revision == 0 && int32_t(at(app, kHardware + 8)) >= 6)
        {
            kTakeScreen.set(app, 0);
            put(app, info + thrash::kAboutSubType, 3);
        }
    }
    const u32 texture = kTextureMemoryHigh - kTextureMemoryLow - kTextureMemoryLow;
    put(app, info + thrash::kAboutVideoMemory, texture);
    if (int32_t(texture) > 0)
        put(app, info + thrash::kAboutVideoMemory, (texture + 0xf) & ~u32(0xf));
    put(app, info + thrash::kAboutTotalMemory, 1);
}

/* sub_a84180(mode, buffers, auxiliary): the window opened in that mode, and
 * the driver's state set to its defaults.  A window already open is closed
 * first; no buffers at all only closes it, and gives back what ebx held. */
Result openMode(App* app, ThreadState& ts, u32 ebx, u32 mode, u32 buffers, u32 auxiliary)
{
    if (kFlags(app) & kWindowOpen)
    {
        closePicture(app);
        kFlags.set(app, uint8_t(kFlags(app) & ~kWindowOpen));
    }
    if (buffers == 0)
        return int32_t(ebx);
    u32 resolution = 0xff;  // GR_RESOLUTION_NONE: the window's own size, which the port's Glide did not know
    if (kRush(app) == 0 || kWindowMode(app) == 0)
        resolution = at(app, kResolutions + mode * 4);
    const u32 entry = kModes + mode * thrash::kModeBytes;
    if (auxiliary != 0 && int32_t(buffers) > int32_t(at(app, entry + thrash::kModeAuxBuffers)))
        auxiliary = 0;
    openPicture(app, kWindow(app), resolution);
    if (kRush(app) == 0)
    {
        kWidth.set(app, s_screen->getWidth());
        kHeight.set(app, s_screen->getHeight());
    }
    else
    {
        kWidth.set(app, at(app, entry + thrash::kModeWidth));
        kHeight.set(app, at(app, entry + thrash::kModeHeight));
    }
    if (!resetTextures(app, ts))
        return {};
    s_renderer->setAlphaTestRef(0x10);
    static const u32 kDefaults[][2] = {
        {thrash::kStateTexture, 0}, {thrash::kStateCull, thrash::kCullCounterClockwise},
        {thrash::kStateTextureFilter, thrash::kFilterLinear}, {thrash::kStateShade, 1},
        {thrash::kStateTransparency, thrash::kTransparencyAlpha}, {thrash::kStateMipFilter, thrash::kMipPoint},
        {thrash::kStateClearColour, 0}, {thrash::kStateChromaKey, 0}, {thrash::kStateDither, 1},
        {thrash::kStateFogDensity, 0}, {thrash::kStateFogColour, 0xffffffff},
        {thrash::kStateBlend, thrash::kBlendSourceAlpha}, {thrash::kStateTextureAddress, thrash::kAddressClamp},
        {thrash::kStateGamma, 0x3f800000}, {thrash::kStateDepthBias, 0}, {thrash::kStateLodBias, 0},
    };
    for (const auto& pair : kDefaults)
        setState(app, ts, pair[0], pair[1]);
    setState(app, ts, thrash::kStateDepth, int32_t(auxiliary) >= 1 ? thrash::kDepthW : thrash::kDepthOff);
    kFlags.set(app, uint8_t(kFlags(app) | kWindowOpen));
    return 1;
}

/* The game's hook around the frame buffer's locks, when it has set one.  False
 * when the game stopped in it. */
bool lockHook(App* app, ThreadState& ts, u32 locking)
{
    const u32 hook = kLockHook(app);
    return hook == 0 || guest::pointerCall(app, ts, hook, {}, {locking}).has_value();
}

/* ---- THRASH_is ---- */

uint8_t characterClass(App* app, uint8_t c)
{
    return byte(app, kCharacterClasses + u32(uint8_t(c + 1)));
}

/* sub_a86b30, strcmp: -1, 0 or 1. */
int32_t compareText(App* app, u32 a, u32 b)
{
    for (;; ++a, ++b)
    {
        const uint8_t x = byte(app, a), y = byte(app, b);
        if (x != y)
            return x < y ? -1 : 1;
        if (x == 0)
            return 0;
    }
}

}  // namespace

void traceTriangles(std::vector<GrVertex>* trace)
{
    s_trace = trace;
}

bool tracingTriangles()
{
    return s_trace != nullptr;
}

void setSwapObserver(SwapObserver observer)
{
    s_swapObserver = observer;
}

bool screenRect(int& x, int& y, int& w, int& h, int& pictureWidth, int& pictureHeight)
{
    if (!s_screen)
        return false;
    s_screen->getViewportRect(x, y, w, h);
    pictureWidth = int(s_screen->getWidth());
    pictureHeight = int(s_screen->getHeight());
    return w > 0 && h > 0 && pictureWidth > 0 && pictureHeight > 0;
}

void clearPicture(App* app, ThreadState& ts)
{
    if (!s_screen || !s_renderer)
        return;
    app->unlockContext(ts);
    s_renderer->setClipWindow(0, 0, s_screen->getWidth(), s_screen->getHeight());
    s_renderer->clear(0);
    if (s_clipWindowSet)
        s_renderer->setClipWindow(s_clipWindow[0], s_clipWindow[1], s_clipWindow[2], s_clipWindow[3]);
    app->lockContext(ts);
}

void setDynamicResolution(u32 width, u32 height)
{
    s_dynamicWidth = width;
    s_dynamicHeight = height;
}

bool fullColourTextures()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_TEXTURES32");
        return !value || SDL_strcmp(value, "0") != 0;
    }();
    return on;
}

/* ---- voodoo2a's exports, typed (port_thrash.h) ---- */

/* THRASH_drawtri(a, b, c). */
void drawTri(App* app, ThreadState& ts, u32 a, u32 b, u32 c)
{
    triangle(app, Drawing(app), a, b, c);
}

/* THRASH_drawquad(a, b, c, d). */
void drawQuad(App* app, ThreadState& ts, u32 a, u32 b, u32 c, u32 d)
{
    quad(app, Drawing(app), a, b, c, d);
}

/* THRASH_drawquadmesh(count, vertices, quads): four vertex indices each. */
void drawQuadMesh(App* app, ThreadState& ts, u32 count, u32 vertices, u32 quads)
{
    const Drawing d(app);
    u32 entry = quads;
    for (int32_t i = 0; i < int32_t(count); ++i, entry += 0x10)
        quad(app, d, meshVertex(app, vertices, entry), meshVertex(app, vertices, entry + 4),
             meshVertex(app, vertices, entry + 8), meshVertex(app, vertices, entry + 0xc));
}

/* THRASH_drawtrimesh(count, vertices, triangles): three indices each. */
void drawTriMesh(App* app, ThreadState& ts, u32 count, u32 vertices, u32 triangles)
{
    const Drawing d(app);
    u32 entry = triangles;
    for (int32_t i = 0; i < int32_t(count); ++i, entry += 0xc)
        triangle(app, d, meshVertex(app, vertices, entry), meshVertex(app, vertices, entry + 4),
                 meshVertex(app, vertices, entry + 8));
}

/* THRASH_drawtrifan(count, vertices): count triangles around the first
 * vertex, the n-th of it, n + 1 and n + 2. */
void drawTriFan(App* app, ThreadState& ts, u32 count, u32 vertices)
{
    const Drawing d(app);
    for (int32_t i = 0; i < int32_t(count); ++i)
        triangle(app, d, vertices, vertices + u32(i + 1) * thrash::kVertexBytes,
                 vertices + u32(i + 2) * thrash::kVertexBytes);
}

/* THRASH_drawtristrip(count, vertices): count triangles along the strip, the
 * n-th of vertices n, n + 1 and n + 2 -- every other one n, n + 2, n + 1, all
 * of them wound the same way. */
void drawTriStrip(App* app, ThreadState& ts, u32 count, u32 vertices)
{
    const Drawing d(app);
    for (int32_t i = 0; i < int32_t(count); ++i)
    {
        const u32 first = vertices + u32(i) * thrash::kVertexBytes;
        const u32 second = first + thrash::kVertexBytes, third = second + thrash::kVertexBytes;
        if (i & 1)
            triangle(app, d, first, third, second);
        else
            triangle(app, d, first, second, third);
    }
}

/* THRASH_drawline(a, b). */
void drawLine(App* app, ThreadState& ts, u32 a, u32 b)
{
    line(app, Drawing(app), a, b);
}

/* THRASH_drawlinemesh(count, vertices, lines): two indices each. */
void drawLineMesh(App* app, ThreadState& ts, u32 count, u32 vertices, u32 lines)
{
    const Drawing d(app);
    u32 entry = lines;
    for (int32_t i = 0; i < int32_t(count); ++i, entry += 8)
        line(app, d, meshVertex(app, vertices, entry), meshVertex(app, vertices, entry + 4));
}

/* THRASH_drawlinestrip(count, vertices): count lines, the n-th from vertex n
 * to n + 1. */
void drawLineStrip(App* app, ThreadState& ts, u32 count, u32 vertices)
{
    const Drawing d(app);
    for (int32_t i = 0; i < int32_t(count); ++i)
        line(app, d, vertices + u32(i) * thrash::kVertexBytes, vertices + u32(i + 1) * thrash::kVertexBytes);
}

/* THRASH_drawpoint(vertex), _drawpointmesh(count, vertices, points) and
 * _drawpointstrip(count, vertices): Glide's points, of which the port's drew
 * none. */
void drawPoint(App* app, ThreadState& ts, u32 vertex)
{
    NFS2_USE(app);
    NFS2_USE(vertex);
}

void drawPointMesh(App* app, ThreadState& ts, u32 count, u32 vertices, u32 points)
{
    NFS2_USE(app);
    NFS2_USE(count);
    NFS2_USE(vertices);
    NFS2_USE(points);
}

void drawPointStrip(App* app, ThreadState& ts, u32 count, u32 vertices)
{
    NFS2_USE(app);
    NFS2_USE(count);
    NFS2_USE(vertices);
}

/* THRASH_talloc(width, height, format, palette, levels). */
std::optional<u32> tAlloc(App* app, ThreadState& ts, u32 width, u32 height, u32 format, u32 palette, u32 levels)
{
    NFS2_USE(palette);
    const Result record = allocateTexture(app, ts, width, height, format, levels);
    if (!record)
        return std::nullopt;
    return u32(*record);
}

/* THRASH_treset: 1. */
std::optional<u32> tReset(App* app, ThreadState& ts)
{
    if (!resetTextures(app, ts))
        return std::nullopt;
    return 1;
}

/* THRASH_tupdate(record, texels, palette): the record. */
u32 tUpdate(App* app, ThreadState& ts, u32 texture, u32 texels, u32 palette)
{
    NFS2_USE(palette);
    return updateTexture(app, ts, texture, texels);
}

/* THRASH_settexture(record): 1. */
u32 setTexture(App* app, ThreadState& ts, u32 texture)
{
    NFS2_USE(ts);
    selectTexture(app, texture);
    return 1;
}

/* THRASH_window(buffer): which buffer to draw into, counted from 1; 0 is 1. */
u32 window(App* app, ThreadState& ts, u32 buffer)
{
    NFS2_USE(ts);
    if (buffer == 0)
        buffer = 1;
    kBuffer.set(app, buffer - 1);
    if (s_renderer)
        s_renderer->render(buffer - 1);
    return 1;
}

/* THRASH_clearwindow: colour and depth cleared to what THRASH_setstate set,
 * the guest context let go meanwhile; the depth back. */
u32 clearWindow(App* app, ThreadState& ts)
{
    const u32 depth = kClearDepth(app);
    if (s_renderer)
    {
        app->unlockContext(ts);
        s_renderer->clear(kClearColour(app));
        app->lockContext(ts);
    }
    return depth;
}

/* THRASH_flushwindow and THRASH_idle: nothing to do on a Voodoo2. */
void flushWindow(App* app, ThreadState& ts)
{
    NFS2_USE(app);
}

void idle(App* app, ThreadState& ts)
{
    NFS2_USE(app);
}

/* THRASH_pageflip: the buffers swapped -- with nothing ever pending -- the
 * guest context let go meanwhile, then the game layer's look at the frame. */
void pageFlip(App* app, ThreadState& ts)
{
    if (s_renderer)
    {
        app->unlockContext(ts);
        s_renderer->swap();
        app->lockContext(ts);
        if (s_swapObserver)
            s_swapObserver(app);
    }
}

/* THRASH_sync(what): 0 waits for the card, 1 asks whether it is busy, 2 waits
 * for the start of a vertical retrace, 3 tells how much of the FIFO is free.
 * The port's Glide was never busy and had no retrace, which voodoo2a waited
 * for four million tries before it gave up, and a FIFO 0xfff short of full. */
u32 sync(App* app, ThreadState& ts, u32 what)
{
    NFS2_USE(app);
    NFS2_USE(ts);
    return what == 1 ? 1 : what == 3 ? 0xf000 : 0;
}

/* THRASH_clip(minX, minY, maxX, maxY): 1. */
u32 clip(App* app, ThreadState& ts, u32 minX, u32 minY, u32 maxX, u32 maxY)
{
    NFS2_USE(app);
    NFS2_USE(ts);
    s_clipWindow[0] = minX;
    s_clipWindow[1] = minY;
    s_clipWindow[2] = maxX;
    s_clipWindow[3] = maxY;
    s_clipWindowSet = true;
    if (s_renderer)
        s_renderer->setClipWindow(minX, minY, maxX, maxY);
    return 1;
}

/* THRASH_lockwindow: the frame buffer locked for the game to write into, as a
 * record of voodoo2a's malloc -- the pixels, the stride, 4 (R5G6B5), the
 * window's size and the buffer -- or 0, the game's lock hook told before and,
 * when there is no lock, after.  (voodoo2a then asked for a read lock as well,
 * which the port's Glide refused.) */
std::optional<u32> lockWindow(App* app, ThreadState& ts)
{
    if (!lockHook(app, ts, 1))
        return std::nullopt;
    u32 record = 0;
    if (s_screen)
    {
        const u32 buffer = kBuffer(app);
        const u32 pixels = s_screen->lock(buffer);
        const u32 stride = s_screen->getWidth() * 2;
        record = thrashMalloc(app, ts, thrash::kLockBytes);
        if (ts.terminate)
            return std::nullopt;
        if (record != 0)
        {
            put(app, record + thrash::kLockData, pixels);
            put(app, record + thrash::kLockFormat, 4);
            put(app, record + thrash::kLockStride, stride);
            put(app, record + thrash::kLockWidth, kWidth(app));
            put(app, record + thrash::kLockHeight, kHeight(app));
            put(app, record + thrash::kLockBuffer, kBuffer(app));
        }
    }
    if (record == 0 && !lockHook(app, ts, 0))
        return std::nullopt;
    return record;
}

/* THRASH_unlockwindow(record): the lock let go, the record freed, the hook
 * told: 1. */
std::optional<u32> unlockWindow(App* app, ThreadState& ts, u32 lock)
{
    if (lock != 0)
    {
        if (s_screen)
            s_screen->unlock(at(app, lock + thrash::kLockBuffer));
        thrashFree(app, ts, lock);
        if (ts.terminate || !lockHook(app, ts, 0))
            return std::nullopt;
    }
    return 1;
}

/* THRASH_readrect(x, y, width, height, to) and THRASH_writerect(x, y, width,
 * height, pixels): the buffer drawn into read back, or 565 pixels written
 * into it, the game's lock hook told around -- 0, from the port's Glide, which
 * did neither. */
std::optional<u32> readRect(App* app, ThreadState& ts, u32 x, u32 y, u32 width, u32 height, u32 pixels)
{
    NFS2_USE(x);
    NFS2_USE(y);
    NFS2_USE(width);
    NFS2_USE(height);
    NFS2_USE(pixels);
    if (!lockHook(app, ts, 1) || !lockHook(app, ts, 0))
        return std::nullopt;
    return 0;
}

std::optional<u32> writeRect(App* app, ThreadState& ts, u32 x, u32 y, u32 width, u32 height, u32 pixels)
{
    NFS2_USE(x);
    NFS2_USE(y);
    NFS2_USE(width);
    NFS2_USE(height);
    NFS2_USE(pixels);
    if (!lockHook(app, ts, 1) || !lockHook(app, ts, 0))
        return std::nullopt;
    return 0;
}

/* THRASH_about: the driver described, once -- its name, what it can do, its
 * texture formats and its modes -- from a look at the card. */
u32 about(App* app, ThreadState& ts)
{
    NFS2_USE(ts);
    if (at(app, kInfo) == 0)
    {
        std::memset(&app->getMemory<nfs::u8>(kInfo), 0, thrash::kAboutBytes);
        put(app, kInfo + thrash::kAboutSignature, thrash::kSignature3dfx);
        put(app, kInfo + thrash::kAboutVersion, 0x68);
        for (const u32 field : {thrash::kAboutMinTextureWidth, thrash::kAboutTextureWidthMultiple,
                                  thrash::kAboutMinTextureHeight, thrash::kAboutTextureHeightMultiple,
                                  thrash::kAboutClipAlign})
            put(app, kInfo + field, 1);
        put(app, kInfo + thrash::kAboutSize, thrash::kAboutBytes);
        put(app, kInfo + thrash::kAboutMaxTextureWidth, kMaxTextureSide);  // port: voodoo2a 0x100
        put(app, kInfo + thrash::kAboutUnknownCount, 4);
        put(app, kInfo + thrash::kAboutModeCount, 0x10);
        put(app, kInfo + thrash::kAboutModes, kModes);
        put(app, kInfo + thrash::kAboutMaxTextureHeight, kMaxTextureSide);  // port: voodoo2a 0x100
        app->getMemory<nfs::u8>(kInfo + thrash::kAboutCaps) =
            nfs::u8((app->getMemory<nfs::u8>(kInfo + thrash::kAboutCaps) | 0xe) & 0xfe);
        put(app, kInfo + thrash::kAboutUnknown, kUnknownList);
        put(app, kInfo + thrash::kAboutTextureFormatCount, 9);
        put(app, kInfo + thrash::kAboutTextureFormats, kTextureFormats);
        if (fullColourTextures())
        {
            /* the port's full colour change: THRASH format 6, ARGB 8888, taken,
             * and handed to the renderer as such. */
            put(app, kTextureFormats + 6 * 4, 1);
            app->getMemory<nfs::u8>(kFormats + 6) = nfs::u8(kTexFmtArgb8888);
        }
        std::strcpy(&app->getMemory<char>(kInfo + thrash::kAboutName), &app->getMemory<char>(kName));
        put(app, kInfo + thrash::kAboutSubType, 0);
        if (!(kFlags(app) & kSetUp))
        {
            // glide2x.dll loaded for a look: one board, the card described.
            if (kLog(app) == 0)
                kLog.set(app, kDefaultLog);
            queryHardware(app);
        }
        measureModes(app, kInfo);
    }
    return kInfo;
}

/* THRASH_selectdisplay(board): 1 for one of the boards, else 0. */
u32 selectDisplay(App* app, ThreadState& ts, u32 board)
{
    NFS2_USE(ts);
    return int32_t(board) < int32_t(at(app, kHardware)) ? 1 : 0;
}

/* THRASH_init: Glide up, board 0, the modes measured.  The number of boards,
 * or 0 when it was already up.  (voodoo2a also had THRASH_restore run at its
 * exit, which never comes on the port.) */
u32 init(App* app, ThreadState& ts)
{
    NFS2_USE(ts);
    u32 result = 0;
    if (!(kFlags(app) & kSetUp))
    {
        if (kLog(app) == 0)
            kLog.set(app, kDefaultLog);
        queryHardware(app);
        if (0 < int32_t(at(app, kHardware)))
        {
            result = at(app, kHardware);
            measureModes(app, kInfo);
            if (result != 0)
                kFlagsWord.set(app, kSetUp);
        }
    }
    return result;
}

/* THRASH_is: how well voodoo2a would drive what there is -- for Glide 2.42 up
 * to 2.5, 100; past it, 95; older, 0.  The port's Glide never said which it
 * was, so what voodoo2a read as its version is what its frame held of the
 * stack below it: push ebx, sub esp, 0x50, the version from the bottom --
 * below esp as the export finds it, at its return address.  (Its VOODOO2_IS
 * from the environment never is: its C library's start-up, which would have
 * filled the environment, never runs on the port.) */
u32 is(App* app, ThreadState& ts)
{
    // (voodoo2a's push ebx left ebx at esp - 4, a dead slot: nothing written.)
    const u32 version = ts.sp - 0x54;
    if (kLog(app) == 0)
        kLog.set(app, kDefaultLog);
    queryHardware(app);
    // From its first digit, or its end.
    u32 number = version;
    while (byte(app, number) != 0 && !(characterClass(app, byte(app, number)) & 0x20))
        ++number;
    if (compareText(app, number, kVersionLow) < 0)
        return 0;
    if (compareText(app, number, kVersionHigh) <= 0)
        return 100;
    return 95;
}

/* THRASH_setvideomode(mode, buffers, auxiliary): the game's window found, by
 * its lambda when it gave one, and the mode opened: 1 when it opened, or what
 * ebx held for no buffers.  (voodoo2a took the screen by DirectDraw first,
 * brought the window forward and, when the game gave it a way, had the
 * window's own thread open the mode while it waited: none of which the port's
 * Win32 did anything with.) */
std::optional<u32> setVideoModeEbx(App* app, ThreadState& ts, u32 ebx, u32 mode, u32 buffers, u32 auxiliary)
{
    if (const u32 findWindow = kFindWindow(app))
    {
        nfs::Args none;
        const std::optional<u32> window = guest::pointerCall(app, ts, findWindow, none);
        if (!window)
            return std::nullopt;
        kWindow.set(app, *window);
    }
    const Result opened = openMode(app, ts, ebx, mode, buffers, auxiliary);
    if (!opened)
        return std::nullopt;
    return u32(*opened);
}

/* The same for native callers: ebx matters only for no buffers, which only restore asks and takes no value of. */
std::optional<u32> setVideoMode(App* app, ThreadState& ts, u32 mode, u32 buffers, u32 auxiliary)
{
    return setVideoModeEbx(app, ts, 0, mode, buffers, auxiliary);
}

/* THRASH_restore: everything let go, the window first: 1. */
std::optional<u32> restore(App* app, ThreadState& ts)
{
    if (kFlagsWord(app) != 0)
    {
        if ((kFlags(app) & kWindowOpen) && !setVideoMode(app, ts, 0, 0, 0))
            return std::nullopt;
        if (kExecute(app) != 0)
            kExecute.set(app, 0);
        closePicture(app);
        kLog.set(app, 0);
        kFlagsWord.set(app, 0);
    }
    return 1;
}

}  // namespace driver
}  // namespace port

/* ---- voodoo2a's exports, as the game calls them ---- */

/* Each export's words off the stack, as the game's code pushes them; its
 * typed function called; its result in eax, every other register as the
 * caller had it and the words popped (stdcall) -- nothing when the game
 * stopped in it. */

using port::App;
using port::ThreadState;
using port::Entry;
using port::u32;
namespace driver = port::driver;

nfs::u32 THRASH_drawtri(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    driver::drawTri(app, ts, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2));
    return e.back(ts, args, args.eax, 0xc);
}

nfs::u32 THRASH_drawquad(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    driver::drawQuad(app, ts, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3));
    return e.back(ts, args, args.eax, 0x10);
}

nfs::u32 THRASH_drawquadmesh(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    driver::drawQuadMesh(app, ts, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2));
    return e.back(ts, args, args.eax, 0xc);
}

nfs::u32 THRASH_drawtrimesh(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    driver::drawTriMesh(app, ts, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2));
    return e.back(ts, args, args.eax, 0xc);
}

nfs::u32 THRASH_drawtrifan(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    driver::drawTriFan(app, ts, e.arg(app, 0), e.arg(app, 1));
    return e.back(ts, args, args.eax, 8);
}

/* voodoo2a's loop draws six at a time and keeps where the next six start in
 * the argument, which it leaves there. */
nfs::u32 THRASH_drawtristrip(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const int32_t count = int32_t(e.arg(app, 0));
    const u32 vertices = e.arg(app, 1);
    const u32 eax = args.eax;
    driver::drawTriStrip(app, ts, u32(count), vertices);
    if (count >= 6)
        guest::put(app, e.esp + 8, vertices + u32(count / 6) * 6 * thrash::kVertexBytes);
    return e.back(ts, args, eax, 8);
}

nfs::u32 THRASH_drawline(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    driver::drawLine(app, ts, e.arg(app, 0), e.arg(app, 1));
    return e.back(ts, args, args.eax, 8);
}

nfs::u32 THRASH_drawlinemesh(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    driver::drawLineMesh(app, ts, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2));
    return e.back(ts, args, args.eax, 0xc);
}

nfs::u32 THRASH_drawlinestrip(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    driver::drawLineStrip(app, ts, e.arg(app, 0), e.arg(app, 1));
    return e.back(ts, args, args.eax, 8);
}

nfs::u32 THRASH_drawpoint(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    driver::drawPoint(app, ts, e.arg(app, 0));
    return e.back(ts, args, args.eax, 4);
}

nfs::u32 THRASH_drawpointmesh(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    driver::drawPointMesh(app, ts, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2));
    return e.back(ts, args, args.eax, 0xc);
}

nfs::u32 THRASH_drawpointstrip(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    driver::drawPointStrip(app, ts, e.arg(app, 0), e.arg(app, 1));
    return e.back(ts, args, args.eax, 8);
}

/* thrash::kMipLinear is kMipPoint in two passes: voodoo2a writes kMipPoint
 * over the argument. */
nfs::u32 THRASH_setstate(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 state = e.arg(app, 0), value = e.arg(app, 1);
    const u32 result = driver::setState(app, ts, state, value);
    if (state == thrash::kStateMipFilter && value == thrash::kMipLinear)
        guest::put(app, e.esp + 8, thrash::kMipPoint);
    return e.back(ts, args, result, 8);
}

nfs::u32 THRASH_talloc(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, driver::tAlloc(app, ts, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3), e.arg(app, 4)),
             0x14);
}

nfs::u32 THRASH_treset(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, driver::tReset(app, ts));
}

nfs::u32 THRASH_tupdate(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, driver::tUpdate(app, ts, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2)), 0xc);
}

nfs::u32 THRASH_settexture(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, driver::setTexture(app, ts, e.arg(app, 0)), 4);
}

nfs::u32 THRASH_window(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, driver::window(app, ts, e.arg(app, 0)), 4);
}

nfs::u32 THRASH_clearwindow(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, driver::clearWindow(app, ts));
}

nfs::u32 THRASH_flushwindow(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    driver::flushWindow(app, ts);
    return e.back(ts, args, args.eax);
}

nfs::u32 THRASH_idle(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    driver::idle(app, ts);
    return e.back(ts, args, args.eax);
}

nfs::u32 THRASH_pageflip(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    const u32 eax = args.eax;
    driver::pageFlip(app, ts);
    return e.back(ts, args, eax);
}

nfs::u32 THRASH_sync(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, driver::sync(app, ts, e.arg(app, 0)), 4);
}

nfs::u32 THRASH_clip(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, driver::clip(app, ts, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3)), 0x10);
}

nfs::u32 THRASH_lockwindow(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, driver::lockWindow(app, ts));
}

nfs::u32 THRASH_unlockwindow(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, driver::unlockWindow(app, ts, e.arg(app, 0)), 4);
}

nfs::u32 THRASH_readrect(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, driver::readRect(app, ts, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3), e.arg(app, 4)),
             0x14);
}

nfs::u32 THRASH_writerect(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args,
                    driver::writeRect(app, ts, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3), e.arg(app, 4)),
                    0x14);
}

nfs::u32 THRASH_about(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, driver::about(app, ts));
}

nfs::u32 THRASH_selectdisplay(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, driver::selectDisplay(app, ts, e.arg(app, 0)), 4);
}

nfs::u32 THRASH_init(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, driver::init(app, ts));
}

nfs::u32 THRASH_is(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.back(ts, args, driver::is(app, ts));
}

nfs::u32 THRASH_setvideomode(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, driver::setVideoModeEbx(app, ts, args.ebx, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2)), 0xc);
}

nfs::u32 THRASH_restore(App* app, ThreadState& ts, nfs::Args& args)
{
    const Entry e(ts, args);
    return e.finish(ts, args, driver::restore(app, ts));
}

/* sub_a84360, what voodoo2a's THRASH_setvideomode handed the game to run on
 * its window's thread, message 0x465.  This driver hands it nobody, so it has no mode to open: 1. */
nfs::u32 THRASH_modemessage(App* app, ThreadState& ts, nfs::Args& args)
{
    NFS2_USE(app);
    const Entry e(ts, args);
    return e.back(ts, args, 1, 0x18);
}

}  // namespace nfs3hp
