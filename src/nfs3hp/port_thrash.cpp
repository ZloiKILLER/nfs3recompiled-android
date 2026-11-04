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
 * renderer calls in the same order (tools/thrash_checks holds it to all of
 * those against the driver and Glide as they were).  Glide's part is folded
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
constexpr reg32 kInfo = 0xa91f54;            // thrash::kAbout*, thrash::kAboutBytes
constexpr reg32 kTextureFormats = 0xa91fd4;  // the descriptor's formats, a dword each, 9 of them
constexpr reg32 kUnknownList = 0xa92000;     // the descriptor's 4 of what is not known
constexpr reg32 kModes = 0xa92018;           // 17 modes of thrash::kModeBytes, the first unused
constexpr reg32 kResolutions = 0xa922e8;     // each mode's resolution: Glide's id, or the port's
/* voodoo2a's map of THRASH's formats to Glide's (its LOD and aspect maps,
 * kLods and kAspects, in port_thrash.h). */
constexpr reg32 kFormats = 0xa92348;
/* The card as Glide described it, GrHwConfiguration: the boards, then the
 * first one's type, frame buffer megabytes, revision, texture units, SLI and
 * its first unit's revision and megabytes. */
constexpr reg32 kHardware = 0xa93714;
/* Strings: the name, and the Glide versions THRASH_is weighs; Watcom's
 * character classes (_IsTable), a byte's at its value + 1. */
constexpr reg32 kName = 0xa916d4;            // "3Dfx Voodoo"
constexpr reg32 kVersionLow = 0xa916ec;      // "2.42"
constexpr reg32 kVersionHigh = 0xa916f4;     // "2.5"
constexpr reg32 kCharacterClasses = 0xa92578;

/* Its variables. */
constexpr Var<uint8_t> kFlags{0xa91ef4};     // kSetUp, kWindowOpen
constexpr Var<reg32> kFlagsWord{0xa91ef4};
constexpr uint8_t kSetUp = 1, kWindowOpen = 2;
constexpr Var<reg32> kRush{0xa91ef8};        // 1 on a card of type 1, which shares the screen: never here
constexpr Var<reg32> kClearColour{0xa91efc}; // thrash::kStateClearColour
constexpr Var<uint16_t> kClearDepth{0xa91f00};
constexpr Var<reg32> kClearDepthWord{0xa91f00};
constexpr Var<reg32> kBuffer{0xa91f04};      // the buffer drawn into, THRASH_window's less one
constexpr Var<reg32> kWindow{0xa91f08};      // the game's window: thrash::kStateWindow, or the lambda's
constexpr Var<reg32> kSwapInterval{0xa91f0c};// thrash::kStateFlipRate
constexpr Var<reg32> kTakeScreen{0xa91f10};  // thrash::kState107: voodoo2a took the screen by DirectDraw by it
constexpr Var<reg32> kWidth{0xa91f18};       // the window's size
constexpr Var<reg32> kHeight{0xa91f1c};
constexpr Var<reg32> kSmooth{0xa91f20};      // thrash::kStateAntiAlias
constexpr Var<reg32> kTwoPass{0xa91f24};     // trilinear mipmapping, thrash::kMipLinear
constexpr Var<reg32> kTextures{0xa91f28};    // the last record THRASH_talloc made; each points at the one before
constexpr Var<reg32> kCombineLocal{0xa91f2c};// thrash::kStateShade's
constexpr Var<reg32> kFindWindow{0xa91f30};  // the game's lambdas, stdcall functions of its own (thrash::kLambda*)
constexpr Var<reg32> kLog{0xa91f34};
constexpr Var<reg32> kLockHook{0xa91f38};    // told 1 before a lock of the frame buffer and 0 after
constexpr Var<reg32> kExecute{0xa91f3c};     // what ran voodoo2a's mode set on the window's thread
constexpr Var<reg32> kWindowMode{0xa91f44};  // thrash::kStateWindowMode
constexpr Var<reg32> kTexturing{0xa92354};   // 1 while a texture is selected, 0 while none; -1 at first
constexpr Var<reg32> kTextureNext{0xa937a8}; // the next free address of texture memory
constexpr reg32 kDefaultLog = 0xa83a80;      // sub_a83a80, voodoo2a's own log, the lambda's default

/* voodoo2a's constants: z to Glide's 1/z, w to the scale of s and t, and its
 * fog table's top. */
constexpr float kDepthScale = 65536.0f;      // [0xa9235c]
constexpr float kTexelScale = 256.0f;        // [0xa92358]
constexpr float kFogScale = 255.0f;          // [0xa916cc]

/* Texture memory as the port's Glide gave it out: each texture takes one slot
 * of it, whatever its size, so that each address is one texture's -- the key
 * ThrashRenderer keeps the texture under. */
constexpr reg32 kTextureSlot = 8;
constexpr reg32 kTextureMemoryLow = kTextureSlot;     // grTexMinAddress(0)
constexpr reg32 kTextureMemoryHigh = 16 * 1024 * 1024;  // grTexMaxAddress(0)

/* ---- The picture ---- */

win32::Renderer* s_screen = nullptr;        // the window, and the frame buffer the game locks
win32::ThrashRenderer* s_renderer = nullptr;
SwapObserver s_swapObserver = nullptr;
std::vector<GrVertex>* s_trace = nullptr;
reg32 s_dynamicWidth = 1920, s_dynamicHeight = 1080;  // kResolutionDynamic's, until it is set
/* fitFourThree: x becomes s_fitOffset + x * s_fitScale for everything drawn;
 * squeezeToLeft: each triangle narrowed by s_squeezeScale. */
float s_fitScale = 1.0f, s_fitOffset = 0.0f, s_squeezeScale = 1.0f;
/* The clip window as THRASH_clip last set it, put back after clearPicture. */
reg32 s_clipWindow[4];
bool s_clipWindowSet = false;

/* The window, opened: grSstWinOpen as the port's Glide had it -- the
 * resolution's size (Glide's, the port's own ids, or 640x480 for one it does
 * not know), a Renderer on the game's window at it, ThrashRenderer on that,
 * and at address 0 the texture of one texel an untextured draw selects. */
void openPicture(App* app, reg32 window, reg32 resolution)
{
    static const struct { reg32 width, height; } kGlideSizes[] = {
        {320, 200}, {320, 240}, {400, 256}, {512, 384}, {640, 200}, {640, 350}, {640, 400},
        {640, 480}, {800, 600}, {960, 720}, {856, 480}, {512, 256}, {1024, 768},
    };
    reg32 width = 640, height = 480;
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
    s_screen = new win32::Renderer(app, dynamic_cast<win32::Window*>(app->getResource(window)));
    s_screen->setVideoMode(width, height, 16);
    app->allocateResource(s_screen);
    s_renderer = new win32::ThrashRenderer(s_screen);
    app->allocateResource(s_renderer);
    const uint16_t texel = 0xff;
    s_renderer->setTextureData(0, 0, &texel, 8, 8, win32::TF_ARGB_4444);
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
 * code as the generator made it did its x87 arithmetic in the FPU's own type
 * without the precision control's rounding (disasm/dll.py), so this does too,
 * operation by operation, in the same order: in plain double, where each
 * product of floats is exact and only the one of three factors and the stores
 * to float round.  Its colour table only ever held
 * each byte's own value, so the bytes are taken as they are. */
struct Vertex
{
    float x, y, ooz, sow, tow;
    reg32 oow;  // the bits as they are
    reg32 colour;
};

Vertex readVertex(App* app, reg32 in)
{
    typedef x86::Float F;
    const F zk = F(app->getMemory<float>(in + thrash::kVertexZ)) * F(kDepthScale);
    const F wk = F(app->getMemory<float>(in + thrash::kVertexRhw)) * F(kTexelScale);
    Vertex out;
    out.x = float(F(app->getMemory<float>(in + thrash::kVertexX)));
    out.y = float(F(app->getMemory<float>(in + thrash::kVertexY)));
    out.oow = at(app, in + thrash::kVertexRhw);
    out.sow = float(F(app->getMemory<float>(in + thrash::kVertexU)) * wk);
    out.tow = float(wk * F(app->getMemory<float>(in + thrash::kVertexV)));
    out.ooz = float(zk);
    const reg32 bgra = at(app, in + thrash::kVertexColour);
    out.colour = (bgra & 0xff00ff00) | (bgra >> 16 & 0xff) | (bgra & 0xff) << 16;
    return out;
}

/* The vertex in a slot of the renderer's batch: u and v before
 * finishTriangle brings far ones back. */
void toBatch(const Vertex& v, win32::ThrashVertex& out)
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
void fit(win32::ThrashVertex& v)
{
    if (s_fitScale == 1.0f && s_fitOffset == 0.0f)
        return;
    v.x = s_fitOffset + v.x * s_fitScale;
}

/* A triangle in the batch narrowed, then fitted. */
void place(win32::ThrashVertex* out)
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

void batchTriangle(const win32::ThrashVertex& a, const win32::ThrashVertex& b, const win32::ThrashVertex& c)
{
    win32::ThrashVertex* out = s_renderer->reserveTriangle();
    out[0] = a;
    out[1] = b;
    out[2] = c;
    s_renderer->finishTriangle(out);
}

/* grDrawLine as the port's Glide drew it: fitted, then four triangles, a
 * pixel wide on each side, the outer corners clear. */
void batchLine(win32::ThrashVertex a, win32::ThrashVertex b)
{
    fit(a);
    fit(b);
    float orthX = a.y - b.y;
    float orthY = b.x - a.x;
    float len = std::sqrt(orthX * orthX + orthY * orthY);
    orthX /= len / 2;
    orthY /= len / 2;
    win32::ThrashVertex a1 = a;
    a1.color &= 0x00ffffff;
    a1.x -= orthX;
    a1.y -= orthY;
    win32::ThrashVertex a2 = a;
    win32::ThrashVertex a3 = a;
    a3.color &= 0x00ffffff;
    a3.x += orthX;
    a3.y += orthY;
    win32::ThrashVertex b1 = b;
    b1.color &= 0x00ffffff;
    b1.x -= orthX;
    b1.y -= orthY;
    win32::ThrashVertex b2 = b;
    win32::ThrashVertex b3 = b;
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
void plainTriangle(App* app, reg32 a, reg32 b, reg32 c)
{
    const Vertex v[3] = {readVertex(app, a), readVertex(app, b), readVertex(app, c)};
    if (s_trace)
        for (const Vertex& corner : v)
            s_trace->push_back(traced(corner));
    if (!s_renderer)
        return;
    win32::ThrashVertex* out = s_renderer->reserveTriangle();
    for (int i = 0; i < 3; ++i)
        toBatch(v[i], out[i]);
    place(out);
    s_renderer->finishTriangle(out);
}

/* A triangle as voodoo2a's drawing draws it.  In two passes (sub_a83930,
 * sub_a839b0): the texture weighted by its LOD fraction -- a blend the port's
 * Glide took as nothing -- then by one less it, added on, which it took as the
 * destination blended whole from then on. */
void triangle(App* app, const Drawing& d, reg32 a, reg32 b, reg32 c)
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
        s_renderer->setAlphaBlendDst(win32::AB_1);
    if (!d.smooth)
        plainTriangle(app, a, b, c);
}

/* A quad: triangles a b c and c d a. */
void quad(App* app, const Drawing& d, reg32 a, reg32 b, reg32 c, reg32 e)
{
    triangle(app, d, a, b, c);
    triangle(app, d, c, e, a);
}

/* A line, drawn only as Glide's own: none in two passes (sub_a83a30, a bare
 * ret), and grAADrawLine drew nothing. */
void line(App* app, const Drawing& d, reg32 a, reg32 b)
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
    win32::ThrashVertex ends[2];
    toBatch(from, ends[0]);
    toBatch(to, ends[1]);
    batchLine(ends[0], ends[1]);
}

/* The n-th of a mesh's vertices: an index times the vertex's size. */
reg32 meshVertex(App* app, reg32 vertices, reg32 index)
{
    return vertices + (at(app, index) << 5);
}

/* ---- Textures ---- */

/* THRASH_talloc(width, height, format, palette, levels): a record of Glide's
 * LODs, aspect and format, where it lies in texture memory, 3 (both halves
 * of the mipmap) and the record before -- from voodoo2a's own malloc, which
 * THRASH_treset gives them back to; 0 when it does not fit.  It starts in the
 * next 2 MB bank if it would straddle one. */
Result allocateTexture(App* app, CPU& cpu, reg32 width, reg32 height, reg32 format, reg32 levelCount)
{
    const int32_t widthLog = exponentOf(width);
    const int32_t heightLog = exponentOf(height);
    const int32_t levels = int32_t(levelCount);
    const int32_t largest = widthLog > heightLog ? widthLog : heightLog;
    int32_t smallest = largest - levels;
    if (smallest < 0)
        smallest = 0;
    const reg32 largeLod = lodOf(app, largest);
    const reg32 smallLod = lodOf(app, smallest);
    const reg32 aspect = byte(app, kAspects + reg32(widthLog - heightLog));
    const reg32 size = kTextureSlot;
    reg32 start = kTextureNext(app);
    const reg32 firstBank = (start - kTextureMemoryLow) & 0xffe00000;
    const reg32 lastBank = (size + start - 1 - kTextureMemoryLow) & 0xffe00000;
    if (lastBank != firstBank)
    {
        start = kTextureMemoryLow + lastBank;
        kTextureNext.set(app, start);
    }
    const reg32 top = (kTextureMemoryHigh + 0xf) & ~reg32(0xf);
    reg32 record = 0;
    if (int32_t(top - start) >= int32_t(size))
    {
        record = thrashMalloc(app, cpu, kRecordBytes);
        if (cpu.terminate)
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
bool resetTextures(App* app, CPU& cpu)
{
    if (s_renderer)
        s_renderer->resetTextures();
    kTextureNext.set(app, kTextureMemoryLow);
    while (const reg32 record = kTextures(app))
    {
        const reg32 before = at(app, record + kRecordBefore);
        thrashFree(app, cpu, record);
        if (cpu.terminate)
            return false;
        kTextures.set(app, before);
    }
    return true;
}

/* THRASH_tupdate(record, texels, palette): new texels for a record, to the
 * renderer in their format -- grTexDownloadMipMap's four the port took, the
 * rest nothing -- with the guest context let go meanwhile.  A palette went to
 * a table the port's Glide did not keep. */
reg32 updateTexture(App* app, CPU& cpu, reg32 record, reg32 texels)
{
    if (texels == 0)
        return record;
    put(app, record + kRecordTexels, texels);
    if (!s_renderer)
        return record;
    app->unlockContext(cpu);
    const reg32 format = at(app, record + kRecordFormat);
    const reg32 start = at(app, record + kRecordStart);
    const void* data = &app->getMemory<const x86::reg8>(texels);
    const reg32 largeLod = at(app, record + kRecordLargeLod), smallLod = at(app, record + kRecordSmallLod);
    if (format == 0xa)  // GR_TEXFMT_RGB_565
        s_renderer->setTextureData(0, start, data, largeLod, smallLod, win32::TF_RGB_565);
    else if (format == 0xb)  // GR_TEXFMT_ARGB_1555
        s_renderer->setTextureData(0, start, data, largeLod, smallLod, win32::TF_ARGB_1555);
    else if (format == 0xc)  // GR_TEXFMT_ARGB_4444
        s_renderer->setTextureData(0, start, data, largeLod, smallLod, win32::TF_ARGB_4444);
    else if (format == kTexFmtArgb8888)
        s_renderer->setTextureData(0, start, data, largeLod, smallLod, win32::TF_ARGB_8888);
    app->lockContext(cpu);
    return record;
}

/* THRASH_settexture(record), and THRASH_setstate 1: the texture drawn with,
 * or none -- the colour and alpha factors following, the texture times the
 * vertex colour or the vertex colour alone. */
void selectTexture(App* app, reg32 record)
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

double fogExp(CPU& cpu, double w, float density)
{
    const double x = -(w * double(density));
    const double t = 1.4426950408889634 * x;
    const double fraction = cpu.fpu.rem(t, 1.0);
    return fp::scale(1.0 + fp::f2xm1(fraction), t);
}

void fogTable(CPU& cpu, float density, uint8_t (&table)[64])
{
    const float scale = float(1.0 / (1.0 - fogExp(cpu, fogW(63), density)));
    for (int i = 0; i < 64; ++i)
    {
        float value = float((1.0 - fogExp(cpu, fogW(i), density)) * double(scale));
        reg32 bits;
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
reg32 culling(reg32 mode)
{
    static const int overrideMode = []() {
        const char* value = std::getenv("NFS_CULL");
        if (!value)
            return -1;
        char* end = nullptr;
        const unsigned long chosen = std::strtoul(value, &end, 0);
        return end != value && *end == '\0' && chosen <= 2 ? int(chosen) : -1;
    }();
    return overrideMode >= 0 ? reg32(overrideMode) : mode;
}

float asFloat(reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* The blend: the destination by one less the source's alpha, or whole; a
 * source factor the port's Glide did not take (1, 4, 4, 0 aside) leaves it as
 * it was. */
void blend(reg32 destination)
{
    if (!s_renderer)
        return;
    if (destination == 5)  // GR_BLEND_ONE_MINUS_SRC_ALPHA
        s_renderer->setAlphaBlendDst(win32::AB_1mSrcAlpha);
    else if (destination == 4)  // GR_BLEND_ONE
        s_renderer->setAlphaBlendDst(win32::AB_1);
}

/* THRASH_setstate(state, value): one piece of the driver's state, to the
 * renderer or to voodoo2a's variables.  1 for done, 0 for a state or value it
 * does not take.  An argument it writes back to (thrash::kMipLinear) is at
 * `slot`, or nowhere for 0. */
reg32 setState(App* app, CPU& cpu, reg32 state, reg32 value, reg32 slot)
{
    win32::ThrashRenderer* const r = s_renderer;
    reg32 result = 1;
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
        reg32 twoPass = 0;
        if (value == thrash::kMipLinear)
        {
            twoPass = 1;
            if (slot != 0)
                put(app, slot, thrash::kMipPoint);  // over the argument, as voodoo2a does
        }
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
            fogTable(cpu, float(1.0 / double(int32_t(value))), table);
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
            r->setFogTable(&app->getMemory<x86::reg8>(value));
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
void measureModes(App* app, reg32 info)
{
    int32_t memory = int32_t(at(app, kHardware + 8) << 20);
    const reg32 type = at(app, kHardware + 4);
    const reg32 rush = type == 1 ? 1 : 0;
    kRush.set(app, rush);
    reg32 sli = 0;
    if (rush == 0)
        sli = at(app, kHardware + 0x14);
    if (sli != 0)
        memory += memory;
    reg32 mode = kModes + thrash::kModeBytes;
    for (int i = 1; i <= 16; ++i, mode += thrash::kModeBytes)
    {
        const reg32 colourBuffers = mode + thrash::kModeColourBuffers, auxBuffers = mode + thrash::kModeAuxBuffers;
        const int32_t pixels = int32_t(at(app, mode + thrash::kModeWidth) * at(app, mode + thrash::kModeHeight));
        const int32_t frame = (int32_t(at(app, mode + thrash::kModeBits)) >> 3) * pixels;
        put(app, colourBuffers, reg32(memory / frame));
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
        const reg32 revision = at(app, kHardware + 0xc);
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
    const reg32 texture = kTextureMemoryHigh - kTextureMemoryLow - kTextureMemoryLow;
    put(app, info + thrash::kAboutVideoMemory, texture);
    if (int32_t(texture) > 0)
        put(app, info + thrash::kAboutVideoMemory, (texture + 0xf) & ~reg32(0xf));
    put(app, info + thrash::kAboutTotalMemory, 1);
}

/* sub_a84180(mode, buffers, auxiliary): the window opened in that mode, and
 * the driver's state set to its defaults.  A window already open is closed
 * first; no buffers at all only closes it, and gives back what ebx held. */
Result openMode(App* app, CPU& cpu, reg32 ebx, reg32 mode, reg32 buffers, reg32 auxiliary)
{
    if (kFlags(app) & kWindowOpen)
    {
        closePicture(app);
        kFlags.set(app, uint8_t(kFlags(app) & ~kWindowOpen));
    }
    if (buffers == 0)
        return int32_t(ebx);
    reg32 resolution = 0xff;  // GR_RESOLUTION_NONE: the window's own size, which the port's Glide did not know
    if (kRush(app) == 0 || kWindowMode(app) == 0)
        resolution = at(app, kResolutions + mode * 4);
    const reg32 entry = kModes + mode * thrash::kModeBytes;
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
    if (!resetTextures(app, cpu))
        return {};
    s_renderer->setAlphaTestRef(0x10);
    static const reg32 kDefaults[][2] = {
        {thrash::kStateTexture, 0}, {thrash::kStateCull, thrash::kCullCounterClockwise},
        {thrash::kStateTextureFilter, thrash::kFilterLinear}, {thrash::kStateShade, 1},
        {thrash::kStateTransparency, thrash::kTransparencyAlpha}, {thrash::kStateMipFilter, thrash::kMipPoint},
        {thrash::kStateClearColour, 0}, {thrash::kStateChromaKey, 0}, {thrash::kStateDither, 1},
        {thrash::kStateFogDensity, 0}, {thrash::kStateFogColour, 0xffffffff},
        {thrash::kStateBlend, thrash::kBlendSourceAlpha}, {thrash::kStateTextureAddress, thrash::kAddressClamp},
        {thrash::kStateGamma, 0x3f800000}, {thrash::kStateDepthBias, 0}, {thrash::kStateLodBias, 0},
    };
    for (const auto& pair : kDefaults)
        setState(app, cpu, pair[0], pair[1], 0);
    setState(app, cpu, thrash::kStateDepth, int32_t(auxiliary) >= 1 ? thrash::kDepthW : thrash::kDepthOff, 0);
    kFlags.set(app, uint8_t(kFlags(app) | kWindowOpen));
    return 1;
}

/* THRASH_setvideomode(mode, buffers, auxiliary): the game's window found, by
 * its lambda when it gave one, and the mode opened.  (voodoo2a took the screen
 * by DirectDraw first, brought the window forward and, when the game gave it
 * a way, had the window's own thread open the mode while it waited: none of
 * which the port's Win32 did anything with.) */
Result setVideoMode(App* app, CPU& cpu, reg32 ebx, reg32 mode, reg32 buffers, reg32 auxiliary)
{
    if (const reg32 findWindow = kFindWindow(app))
    {
        if (!guest::call(app, cpu, findWindow))
            return {};
        kWindow.set(app, cpu.eax);
    }
    return openMode(app, cpu, ebx, mode, buffers, auxiliary);
}

/* The game's hook around the frame buffer's locks, when it has set one.  False
 * when the game stopped in it. */
bool lockHook(App* app, CPU& cpu, reg32 locking)
{
    const reg32 hook = kLockHook(app);
    return hook == 0 || guest::call(app, cpu, hook, {locking});
}

/* ---- THRASH_is ---- */

uint8_t characterClass(App* app, uint8_t c)
{
    return byte(app, kCharacterClasses + reg32(uint8_t(c + 1)));
}

/* sub_a86b30, strcmp: -1, 0 or 1. */
int32_t compareText(App* app, reg32 a, reg32 b)
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

void clearPicture(App* app, CPU& cpu)
{
    if (!s_screen || !s_renderer)
        return;
    app->unlockContext(cpu);
    s_renderer->setClipWindow(0, 0, s_screen->getWidth(), s_screen->getHeight());
    s_renderer->clear(0);
    if (s_clipWindowSet)
        s_renderer->setClipWindow(s_clipWindow[0], s_clipWindow[1], s_clipWindow[2], s_clipWindow[3]);
    app->lockContext(cpu);
}

void setDynamicResolution(reg32 width, reg32 height)
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

}  // namespace driver
}  // namespace port

/* ---- voodoo2a's exports ---- */

using port::App;
using port::CPU;
using port::Entry;
using port::Result;
using port::reg32;
namespace driver = port::driver;

void THRASH_drawtri(App* app, CPU& cpu)
{
    const Entry e(cpu);
    driver::triangle(app, driver::Drawing(app), e.arg(app, 0), e.arg(app, 1), e.arg(app, 2));
    e.back(cpu, e.eax, 0xc);
}

/* THRASH_drawquad(a, b, c, d). */
void THRASH_drawquad(App* app, CPU& cpu)
{
    const Entry e(cpu);
    driver::quad(app, driver::Drawing(app), e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3));
    e.back(cpu, e.eax, 0x10);
}

/* THRASH_drawquadmesh(count, vertices, quads): four vertex indices each. */
void THRASH_drawquadmesh(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const driver::Drawing d(app);
    const int32_t count = int32_t(e.arg(app, 0));
    const reg32 vertices = e.arg(app, 1);
    reg32 quad = e.arg(app, 2);
    for (int32_t i = 0; i < count; ++i, quad += 0x10)
        driver::quad(app, d, driver::meshVertex(app, vertices, quad), driver::meshVertex(app, vertices, quad + 4),
                     driver::meshVertex(app, vertices, quad + 8), driver::meshVertex(app, vertices, quad + 0xc));
    e.back(cpu, e.eax, 0xc);
}

/* THRASH_drawtrimesh(count, vertices, triangles): three indices each. */
void THRASH_drawtrimesh(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const driver::Drawing d(app);
    const int32_t count = int32_t(e.arg(app, 0));
    const reg32 vertices = e.arg(app, 1);
    reg32 triangle = e.arg(app, 2);
    for (int32_t i = 0; i < count; ++i, triangle += 0xc)
        driver::triangle(app, d, driver::meshVertex(app, vertices, triangle),
                         driver::meshVertex(app, vertices, triangle + 4), driver::meshVertex(app, vertices, triangle + 8));
    e.back(cpu, e.eax, 0xc);
}

/* THRASH_drawtrifan(count, vertices): count triangles around the first
 * vertex, the n-th of it, n + 1 and n + 2. */
void THRASH_drawtrifan(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const driver::Drawing d(app);
    const int32_t count = int32_t(e.arg(app, 0));
    const reg32 vertices = e.arg(app, 1);
    for (int32_t i = 0; i < count; ++i)
        driver::triangle(app, d, vertices, vertices + reg32(i + 1) * thrash::kVertexBytes,
                         vertices + reg32(i + 2) * thrash::kVertexBytes);
    e.back(cpu, e.eax, 8);
}

/* THRASH_drawtristrip(count, vertices): count triangles along the strip, the
 * n-th of vertices n, n + 1 and n + 2 -- every other one n, n + 2, n + 1, all
 * of them wound the same way.  voodoo2a's loop draws six at a time and keeps
 * where the next six start in the argument, which it leaves there. */
void THRASH_drawtristrip(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const driver::Drawing d(app);
    const int32_t count = int32_t(e.arg(app, 0));
    const reg32 vertices = e.arg(app, 1);
    for (int32_t i = 0; i < count; ++i)
    {
        const reg32 first = vertices + reg32(i) * thrash::kVertexBytes;
        const reg32 second = first + thrash::kVertexBytes, third = second + thrash::kVertexBytes;
        if (i & 1)
            driver::triangle(app, d, first, third, second);
        else
            driver::triangle(app, d, first, second, third);
    }
    if (count >= 6)
        guest::put(app, e.esp + 8, vertices + reg32(count / 6) * 6 * thrash::kVertexBytes);
    e.back(cpu, e.eax, 8);
}

/* THRASH_drawline(a, b). */
void THRASH_drawline(App* app, CPU& cpu)
{
    const Entry e(cpu);
    driver::line(app, driver::Drawing(app), e.arg(app, 0), e.arg(app, 1));
    e.back(cpu, e.eax, 8);
}

/* THRASH_drawlinemesh(count, vertices, lines): two indices each. */
void THRASH_drawlinemesh(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const driver::Drawing d(app);
    const int32_t count = int32_t(e.arg(app, 0));
    const reg32 vertices = e.arg(app, 1);
    reg32 line = e.arg(app, 2);
    for (int32_t i = 0; i < count; ++i, line += 8)
        driver::line(app, d, driver::meshVertex(app, vertices, line), driver::meshVertex(app, vertices, line + 4));
    e.back(cpu, e.eax, 0xc);
}

/* THRASH_drawlinestrip(count, vertices): count lines, the n-th from vertex n
 * to n + 1. */
void THRASH_drawlinestrip(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const driver::Drawing d(app);
    const int32_t count = int32_t(e.arg(app, 0));
    const reg32 vertices = e.arg(app, 1);
    for (int32_t i = 0; i < count; ++i)
        driver::line(app, d, vertices + reg32(i) * thrash::kVertexBytes, vertices + reg32(i + 1) * thrash::kVertexBytes);
    e.back(cpu, e.eax, 8);
}

/* THRASH_drawpoint(vertex), _drawpointmesh(count, vertices, points) and
 * _drawpointstrip(count, vertices): Glide's points, of which the port's drew
 * none. */
void THRASH_drawpoint(App* app, CPU& cpu)
{
    NFS2_USE(app);
    const Entry e(cpu);
    e.back(cpu, e.eax, 4);
}

void THRASH_drawpointmesh(App* app, CPU& cpu)
{
    NFS2_USE(app);
    const Entry e(cpu);
    e.back(cpu, e.eax, 0xc);
}

void THRASH_drawpointstrip(App* app, CPU& cpu)
{
    NFS2_USE(app);
    const Entry e(cpu);
    e.back(cpu, e.eax, 8);
}

/* THRASH_setstate(state, value). */
void THRASH_setstate(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 result = driver::setState(app, cpu, e.arg(app, 0), e.arg(app, 1), e.esp + 8);
    e.back(cpu, result, 8);
}

/* THRASH_talloc(width, height, format, palette, levels). */
void THRASH_talloc(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const Result record = driver::allocateTexture(app, cpu, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 4));
    e.finish(cpu, record, 0x14);
}

/* THRASH_treset: 1. */
void THRASH_treset(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (driver::resetTextures(app, cpu))
        e.back(cpu, 1);
}

/* THRASH_tupdate(record, texels, palette): the record. */
void THRASH_tupdate(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 record = driver::updateTexture(app, cpu, e.arg(app, 0), e.arg(app, 1));
    e.back(cpu, record, 0xc);
}

/* THRASH_settexture(record): 1. */
void THRASH_settexture(App* app, CPU& cpu)
{
    const Entry e(cpu);
    driver::selectTexture(app, e.arg(app, 0));
    e.back(cpu, 1, 4);
}

/* THRASH_window(buffer): which buffer to draw into, counted from 1; 0 is 1. */
void THRASH_window(App* app, CPU& cpu)
{
    const Entry e(cpu);
    reg32 buffer = e.arg(app, 0);
    if (buffer == 0)
        buffer = 1;
    driver::kBuffer.set(app, buffer - 1);
    if (driver::s_renderer)
        driver::s_renderer->render(buffer - 1);
    e.back(cpu, 1, 4);
}

/* THRASH_clearwindow: colour and depth cleared to what THRASH_setstate set,
 * the guest context let go meanwhile; the depth back. */
void THRASH_clearwindow(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 depth = driver::kClearDepth(app);
    if (driver::s_renderer)
    {
        app->unlockContext(cpu);
        driver::s_renderer->clear(driver::kClearColour(app));
        app->lockContext(cpu);
    }
    e.back(cpu, depth);
}

/* THRASH_flushwindow and THRASH_idle: nothing to do on a Voodoo2. */
void THRASH_flushwindow(App* app, CPU& cpu)
{
    NFS2_USE(app);
    const Entry e(cpu);
    e.back(cpu, e.eax);
}

void THRASH_idle(App* app, CPU& cpu)
{
    NFS2_USE(app);
    const Entry e(cpu);
    e.back(cpu, e.eax);
}

/* THRASH_pageflip: the buffers swapped -- with nothing ever pending -- the
 * guest context let go meanwhile, then the game layer's look at the frame. */
void THRASH_pageflip(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (driver::s_renderer)
    {
        app->unlockContext(cpu);
        driver::s_renderer->swap();
        app->lockContext(cpu);
        if (driver::s_swapObserver)
            driver::s_swapObserver(app);
    }
    e.back(cpu, e.eax);
}

/* THRASH_sync(what): 0 waits for the card, 1 asks whether it is busy, 2 waits
 * for the start of a vertical retrace, 3 tells how much of the FIFO is free.
 * The port's Glide was never busy and had no retrace, which voodoo2a waited
 * for four million tries before it gave up, and a FIFO 0xfff short of full. */
void THRASH_sync(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 what = e.arg(app, 0);
    e.back(cpu, what == 1 ? 1 : what == 3 ? 0xf000 : 0, 4);
}

/* THRASH_clip(minX, minY, maxX, maxY): 1. */
void THRASH_clip(App* app, CPU& cpu)
{
    const Entry e(cpu);
    for (int i = 0; i < 4; ++i)
        driver::s_clipWindow[i] = e.arg(app, i);
    driver::s_clipWindowSet = true;
    if (driver::s_renderer)
        driver::s_renderer->setClipWindow(e.arg(app, 0), e.arg(app, 1), e.arg(app, 2), e.arg(app, 3));
    e.back(cpu, 1, 0x10);
}

/* THRASH_lockwindow: the frame buffer locked for the game to write into, as a
 * record of voodoo2a's malloc -- the pixels, the stride, 4 (R5G6B5), the
 * window's size and the buffer -- or 0, the game's lock hook told before and,
 * when there is no lock, after.  (voodoo2a then asked for a read lock as well,
 * which the port's Glide refused.) */
void THRASH_lockwindow(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (!driver::lockHook(app, cpu, 1))
        return;
    reg32 record = 0;
    if (driver::s_screen)
    {
        const reg32 buffer = driver::kBuffer(app);
        const reg32 pixels = driver::s_screen->lock(buffer);
        const reg32 stride = driver::s_screen->getWidth() * 2;
        record = thrashMalloc(app, cpu, thrash::kLockBytes);
        if (cpu.terminate)
            return;
        if (record != 0)
        {
            guest::put(app, record + thrash::kLockData, pixels);
            guest::put(app, record + thrash::kLockFormat, 4);
            guest::put(app, record + thrash::kLockStride, stride);
            guest::put(app, record + thrash::kLockWidth, driver::kWidth(app));
            guest::put(app, record + thrash::kLockHeight, driver::kHeight(app));
            guest::put(app, record + thrash::kLockBuffer, driver::kBuffer(app));
        }
    }
    if (record == 0 && !driver::lockHook(app, cpu, 0))
        return;
    e.back(cpu, record);
}

/* THRASH_unlockwindow(record): the lock let go, the record freed, the hook
 * told: 1. */
void THRASH_unlockwindow(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 record = e.arg(app, 0);
    if (record != 0)
    {
        if (driver::s_screen)
            driver::s_screen->unlock(guest::at(app, record + thrash::kLockBuffer));
        thrashFree(app, cpu, record);
        if (cpu.terminate || !driver::lockHook(app, cpu, 0))
            return;
    }
    e.back(cpu, 1, 4);
}

/* THRASH_readrect(x, y, width, height, to) and THRASH_writerect(x, y, width,
 * height, pixels): the buffer drawn into read back, or 565 pixels written
 * into it, the game's lock hook told around -- 0, from the port's Glide, which
 * did neither. */
void THRASH_readrect(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (!driver::lockHook(app, cpu, 1) || !driver::lockHook(app, cpu, 0))
        return;
    e.back(cpu, 0, 0x14);
}

void THRASH_writerect(App* app, CPU& cpu)
{
    const Entry e(cpu);
    if (!driver::lockHook(app, cpu, 1) || !driver::lockHook(app, cpu, 0))
        return;
    e.back(cpu, 0, 0x14);
}

/* THRASH_about: the driver described, once -- its name, what it can do, its
 * texture formats and its modes -- from a look at the card. */
void THRASH_about(App* app, CPU& cpu)
{
    using namespace driver;
    const Entry e(cpu);
    if (at(app, kInfo) == 0)
    {
        std::memset(&app->getMemory<x86::reg8>(kInfo), 0, thrash::kAboutBytes);
        put(app, kInfo + thrash::kAboutSignature, thrash::kSignature3dfx);
        put(app, kInfo + thrash::kAboutVersion, 0x68);
        for (const reg32 field : {thrash::kAboutMinTextureWidth, thrash::kAboutTextureWidthMultiple,
                                  thrash::kAboutMinTextureHeight, thrash::kAboutTextureHeightMultiple,
                                  thrash::kAboutClipAlign})
            put(app, kInfo + field, 1);
        put(app, kInfo + thrash::kAboutSize, thrash::kAboutBytes);
        put(app, kInfo + thrash::kAboutMaxTextureWidth, kMaxTextureSide);  // port: voodoo2a 0x100
        put(app, kInfo + thrash::kAboutUnknownCount, 4);
        put(app, kInfo + thrash::kAboutModeCount, 0x10);
        put(app, kInfo + thrash::kAboutModes, kModes);
        put(app, kInfo + thrash::kAboutMaxTextureHeight, kMaxTextureSide);  // port: voodoo2a 0x100
        app->getMemory<x86::reg8>(kInfo + thrash::kAboutCaps) =
            x86::reg8((app->getMemory<x86::reg8>(kInfo + thrash::kAboutCaps) | 0xe) & 0xfe);
        put(app, kInfo + thrash::kAboutUnknown, kUnknownList);
        put(app, kInfo + thrash::kAboutTextureFormatCount, 9);
        put(app, kInfo + thrash::kAboutTextureFormats, kTextureFormats);
        if (fullColourTextures())
        {
            /* tools/apply_full_colour.py: THRASH format 6, ARGB 8888, taken,
             * and handed to the renderer as such. */
            put(app, kTextureFormats + 6 * 4, 1);
            app->getMemory<x86::reg8>(kFormats + 6) = x86::reg8(kTexFmtArgb8888);
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
    e.back(cpu, kInfo);
}

/* THRASH_selectdisplay(board): 1 for one of the boards, else 0. */
void THRASH_selectdisplay(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const reg32 board = e.arg(app, 0);
    e.back(cpu, int32_t(board) < int32_t(guest::at(app, driver::kHardware)) ? 1 : 0, 4);
}

/* THRASH_init: Glide up, board 0, the modes measured.  The number of boards,
 * or 0 when it was already up.  (voodoo2a also had THRASH_restore run at its
 * exit, which never comes on the port.) */
void THRASH_init(App* app, CPU& cpu)
{
    using namespace driver;
    const Entry e(cpu);
    reg32 result = 0;
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
    e.back(cpu, result);
}

/* THRASH_is: how well voodoo2a would drive what there is -- for Glide 2.42 up
 * to 2.5, 100; past it, 95; older, 0.  The port's Glide never said which it
 * was, so what voodoo2a read as its version is what its frame held of the
 * stack below it: push ebx, sub esp, 0x50, the version from the bottom.  (Its
 * VOODOO2_IS from the environment never is: its C library's start-up, which
 * would have filled the environment, never runs on the port.) */
void THRASH_is(App* app, CPU& cpu)
{
    using namespace driver;
    const Entry e(cpu);
    put(app, e.esp - 4, e.ebx);
    const reg32 version = e.esp - 0x54;
    if (kLog(app) == 0)
        kLog.set(app, kDefaultLog);
    queryHardware(app);
    // From its first digit, or its end.
    reg32 number = version;
    while (byte(app, number) != 0 && !(characterClass(app, byte(app, number)) & 0x20))
        ++number;
    reg32 result;
    if (compareText(app, number, kVersionLow) < 0)
        result = 0;
    else if (compareText(app, number, kVersionHigh) <= 0)
        result = 100;
    else
        result = 95;
    e.back(cpu, result);
}

/* THRASH_setvideomode(mode, buffers, auxiliary): 1 when it opened, or what
 * ebx held for no buffers. */
void THRASH_setvideomode(App* app, CPU& cpu)
{
    const Entry e(cpu);
    const Result opened = driver::setVideoMode(app, cpu, e.ebx, e.arg(app, 0), e.arg(app, 1), e.arg(app, 2));
    e.finish(cpu, opened, 0xc);
}

/* THRASH_restore: everything let go, the window first: 1. */
void THRASH_restore(App* app, CPU& cpu)
{
    using namespace driver;
    const Entry e(cpu);
    if (kFlagsWord(app) != 0)
    {
        if ((kFlags(app) & kWindowOpen) && !setVideoMode(app, cpu, e.ebx, 0, 0, 0))
            return;
        if (kExecute(app) != 0)
            kExecute.set(app, 0);
        closePicture(app);
        kLog.set(app, 0);
        kFlagsWord.set(app, 0);
    }
    e.back(cpu, 1);
}

/* sub_a84360, what voodoo2a's THRASH_setvideomode handed the game to run on
 * its window's thread, message 0x465 (disassemble_nfs3hp.py binds the
 * address).  This driver hands it nobody, so it has no mode to open: 1. */
void THRASH_modemessage(App* app, CPU& cpu)
{
    NFS2_USE(app);
    const Entry e(cpu);
    e.back(cpu, 1, 0x18);
}

}  // namespace nfs3hp
