#include <nfs3hp.h>
#include "native_thrash.h"
#include <lib/gliderenderer.h>
#include <lib/thrashrenderer.h>
#include <winapi/glide2x.h>
#include <SDL3/SDL.h>
#include <cmath>
#include <cstring>
#include <initializer_list>
#include <string>
#include <vector>

/* A product rounded to float straight from a fused multiply-add would round
 * once where voodoo2a's code rounds twice. */
#if defined(__clang__)
#pragma clang fp contract(off)
#elif defined(_MSC_VER)
#pragma fp_contract(off)
#endif

/* The first stage of a native THRASH driver: voodoo2a's THRASH functions in
 * C++, one at a time, entered from the top of the generated ones
 * (tools/apply_native_thrash.py), with voodoo2a's own state in guest memory as
 * the one copy every function reads until all of them are native.
 *
 * THRASH is the game's interface to its renderer DLLs -- voodoo2a, softtria and
 * the Direct3D one -- called through the pointers sub_4f8390 got from
 * GetProcAddress.  The game was built against all of them, so it relies on what
 * a stdcall function promises and nothing more: the stack cleaned up, ebx, esi,
 * edi and ebp kept, a result in eax where there is one.  That is what these keep,
 * and what NFS_NATIVE_CHECK compares, with everything they draw; the scratch
 * registers and flags voodoo2a happened to leave are nobody's business.
 *
 * NFS_THRASH=0 leaves voodoo2a's generated code to all of them, as does
 * NFS_NATIVES=0. */

namespace nfs3hp
{

win32::ThrashRenderer::Corner thrashCorner(win32::WinApplication* app, x86::reg32 in, float zScale, float wScale);
void thrashBatchVertex(win32::WinApplication* app, x86::reg32 in, float zScale, float wScale, win32::ThrashVertex& out);

namespace
{

/* voodoo2a's data, at its load address (RVA + 0xa82200). */
constexpr x86::reg32 kColourTable = 0xa93210;  // a colour byte as the float Glide takes, 256 of them
constexpr x86::reg32 kDepthScale = 0xa9235c;   // z to Glide's ooz
constexpr x86::reg32 kTexelScale = 0xa92358;   // w to the scale of s and t
/* The Glide entry points voodoo2a looked up (sub_a83220), and the ones its
 * drawing goes through: sub_a848e0 points these at the plain ones, or at its
 * two-pass and antialiased wrappers, as THRASH_setstate asks. */
constexpr x86::reg32 kGrDrawTriangle = 0xa936a8;
constexpr x86::reg32 kGrDrawLine = 0xa936ac;
constexpr x86::reg32 kTriangleDraw = 0xa93710;  // THRASH_drawtri, _drawtrimesh, _drawtrifan
constexpr x86::reg32 kQuadDraw = 0xa9370c;      // THRASH_drawquad, _drawquadmesh, two triangles each
constexpr x86::reg32 kLineDraw = 0xa93610;      // THRASH_drawline, _drawlinemesh
/* The game's vertex: x, y, z, w; colour bytes b, g, r, a at +0x10; u, v at +0x18. */
constexpr x86::reg32 kVertexSize = 0x20;

x86::reg32 at(win32::WinApplication* app, x86::reg32 address)
{
    return app->getMemory<x86::reg32>(address);
}

bool drawsThrough(win32::WinApplication* app, x86::reg32 pointer, x86::reg32 plain)
{
    const x86::reg32 target = at(app, pointer);
    return target != 0 && target == at(app, plain);
}

typedef win32::ThrashRenderer::Corner Corner;

/* The game's vertices as Glide's, with the two scales read once per call --
 * or, while ThrashRenderer may take them as they are (`direct`), as its
 * corners, with no GrVertex and no trip through glide2x between. */
struct Vertices
{
    win32::WinApplication* app;
    float zScale;
    float wScale;
    win32::ThrashRenderer* direct;

    explicit Vertices(win32::WinApplication* app)
        :   app(app)
        ,   zScale(app->getMemory<float>(kDepthScale))
        ,   wScale(app->getMemory<float>(kTexelScale))
        ,   direct(win32::glide2x::direct::thrashRenderer())
    {
    }

    win32::GrVertex operator()(x86::reg32 in) const
    {
        win32::GrVertex out = {};
        thrashVertex(app, in, zScale, wScale, out);
        return out;
    }

    Corner corner(x86::reg32 in) const
    {
        return thrashCorner(app, in, zScale, wScale);
    }

    void triangle(x86::reg32 a, x86::reg32 b, x86::reg32 c) const
    {
        if (direct)
        {
            win32::ThrashVertex* out = direct->reserveTriangle();
            thrashBatchVertex(app, a, zScale, wScale, out[0]);
            thrashBatchVertex(app, b, zScale, wScale, out[1]);
            thrashBatchVertex(app, c, zScale, wScale, out[2]);
            direct->finishTriangle(out);
            return;
        }
        const win32::GrVertex vertices[3] = { (*this)(a), (*this)(b), (*this)(c) };
        win32::glide2x::drawTriangle(&vertices[0], &vertices[1], &vertices[2]);
    }

    /* Triangles a b c and c d a, as Glide's quads go. */
    void quad(x86::reg32 a, x86::reg32 b, x86::reg32 c, x86::reg32 d) const
    {
        if (direct)
        {
            // the four once; each triangle's slots copied from them before rebasing
            win32::ThrashVertex v[4];
            thrashBatchVertex(app, a, zScale, wScale, v[0]);
            thrashBatchVertex(app, b, zScale, wScale, v[1]);
            thrashBatchVertex(app, c, zScale, wScale, v[2]);
            thrashBatchVertex(app, d, zScale, wScale, v[3]);
            win32::ThrashVertex* out = direct->reserveTriangle();
            out[0] = v[0];
            out[1] = v[1];
            out[2] = v[2];
            direct->finishTriangle(out);
            out = direct->reserveTriangle();
            out[0] = v[2];
            out[1] = v[3];
            out[2] = v[0];
            direct->finishTriangle(out);
            return;
        }
        const win32::GrVertex va = (*this)(a), vb = (*this)(b), vc = (*this)(c), vd = (*this)(d);
        win32::glide2x::drawTriangle(&va, &vb, &vc);
        win32::glide2x::drawTriangle(&vc, &vd, &va);
    }
};

/* THRASH_drawquad(a, b, c, d): triangles a b c and c d a. */
bool drawQuadNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const Vertices convert(app);
    convert.quad(at(app, esp + 4), at(app, esp + 8), at(app, esp + 0xc), at(app, esp + 0x10));
    cpu.esp = esp + 4 + 0x10;
    return true;
}

/* THRASH_drawquadmesh(count, vertices, quads): each quad four vertex indices,
 * drawn as THRASH_drawquad draws its four. */
bool drawQuadMeshNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::sreg32 count = x86::sreg32(at(app, esp + 4));
    const x86::reg32 vertices = at(app, esp + 8);
    x86::reg32 quad = at(app, esp + 0xc);
    const Vertices convert(app);
    for (x86::sreg32 i = 0; i < count; ++i, quad += 0x10)
        convert.quad(vertices + (at(app, quad) << 5), vertices + (at(app, quad + 4) << 5),
                     vertices + (at(app, quad + 8) << 5), vertices + (at(app, quad + 0xc) << 5));
    cpu.esp = esp + 4 + 0xc;
    return true;
}

/* THRASH_drawtrimesh(count, vertices, triangles): three indices each. */
bool drawTriMeshNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::sreg32 count = x86::sreg32(at(app, esp + 4));
    const x86::reg32 vertices = at(app, esp + 8);
    x86::reg32 triangle = at(app, esp + 0xc);
    const Vertices convert(app);
    for (x86::sreg32 i = 0; i < count; ++i, triangle += 0xc)
    {
        convert.triangle(vertices + (at(app, triangle) << 5), vertices + (at(app, triangle + 4) << 5),
                         vertices + (at(app, triangle + 8) << 5));
    }
    cpu.esp = esp + 4 + 0xc;
    return true;
}

/* THRASH_drawtrifan(count, vertices): count triangles around the first vertex,
 * the n-th of it, n + 1 and n + 2. */
bool drawTriFanNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::sreg32 count = x86::sreg32(at(app, esp + 4));
    const x86::reg32 vertices = at(app, esp + 8);
    const Vertices convert(app);
    if (count > 0 && convert.direct)
    {
        win32::ThrashVertex centre, previous, next;
        thrashBatchVertex(app, vertices, convert.zScale, convert.wScale, centre);
        thrashBatchVertex(app, vertices + kVertexSize, convert.zScale, convert.wScale, previous);
        for (x86::sreg32 i = 0; i < count; ++i)
        {
            thrashBatchVertex(app, vertices + x86::reg32(i + 2) * kVertexSize, convert.zScale, convert.wScale, next);
            win32::ThrashVertex* out = convert.direct->reserveTriangle();
            out[0] = centre;
            out[1] = previous;
            out[2] = next;
            convert.direct->finishTriangle(out);
            previous = next;
        }
    }
    else if (count > 0)
    {
        const win32::GrVertex centre = convert(vertices);
        win32::GrVertex previous = convert(vertices + kVertexSize);
        for (x86::sreg32 i = 0; i < count; ++i)
        {
            const win32::GrVertex next = convert(vertices + x86::reg32(i + 2) * kVertexSize);
            win32::glide2x::drawTriangle(&centre, &previous, &next);
            previous = next;
        }
    }
    cpu.esp = esp + 4 + 8;
    return true;
}

/* THRASH_drawline(a, b). */
bool drawLineNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const Vertices convert(app);
    const win32::GrVertex a = convert(at(app, esp + 4));
    const win32::GrVertex b = convert(at(app, esp + 8));
    win32::glide2x::drawLine(&a, &b);
    cpu.esp = esp + 4 + 8;
    return true;
}

/* THRASH_drawlinemesh(count, vertices, lines): two indices each. */
bool drawLineMeshNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const x86::sreg32 count = x86::sreg32(at(app, esp + 4));
    const x86::reg32 vertices = at(app, esp + 8);
    x86::reg32 line = at(app, esp + 0xc);
    const Vertices convert(app);
    for (x86::sreg32 i = 0; i < count; ++i, line += 8)
    {
        const win32::GrVertex a = convert(vertices + (at(app, line) << 5));
        const win32::GrVertex b = convert(vertices + (at(app, line + 4) << 5));
        win32::glide2x::drawLine(&a, &b);
    }
    cpu.esp = esp + 4 + 0xc;
    return true;
}

/* A native stand-in: false, before it has done anything, where it leaves the
 * call to the generated code. */
using Native = bool (*)(win32::WinApplication*, x86::CPU&);

void put(win32::WinApplication* app, x86::reg32 address, x86::reg32 value)
{
    app->getMemory<x86::reg32>(address) = value;
}

float asFloat(x86::reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* sub_a848e0: what voodoo2a's drawing goes through, from the two-pass flag
 * ([0xa91f24], THRASH_setstate 0xb) and the antialiasing one ([0xa91f20],
 * state 9) -- Glide's own triangle and line, its antialiased ones behind small
 * wrappers, or the two-pass wrappers. */
void drawPointers(win32::WinApplication* app, x86::reg32 twoPass, x86::reg32 antialiased)
{
    x86::reg32 triangle, line, quad;
    if (antialiased == 0)
    {
        triangle = at(app, kGrDrawTriangle);
        line = at(app, kGrDrawLine);
        quad = triangle;
    }
    else
    {
        quad = 0xa83a60;
        triangle = 0xa83a40;
        line = at(app, 0xa936e4);  // grAADrawLine
    }
    x86::reg32 lineDraw = line;
    if (twoPass == 0)
    {
        put(app, kTriangleDraw, triangle);
        put(app, kQuadDraw, quad);
    }
    else
    {
        put(app, kTriangleDraw, 0xa83930);
        put(app, kQuadDraw, 0xa839b0);
        lineDraw = 0xa83a30;  // ret 8: no lines in two passes
    }
    put(app, kLineDraw, lineDraw);
    put(app, 0xa93708, triangle);
    put(app, 0xa93704, line);
    put(app, 0xa93700, quad);
}

/* A call into voodoo2a's own C library, Watcom's register convention: the
 * argument and the result in eax, every other register kept. */
x86::reg32 callRuntime(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address, x86::reg32 argument)
{
    const x86::reg32 eax = cpu.eax;
    cpu.eax = argument;
    cpu.esp -= 4;
    app->dynamic_call(address, cpu);
    const x86::reg32 result = cpu.eax;
    cpu.eax = eax;
    return result;
}

constexpr x86::reg32 kMalloc = 0xa86cb0;
constexpr x86::reg32 kFree = 0xa86da0;
constexpr x86::reg32 kTextures = 0xa91f28;      // the last record THRASH_talloc made; each points at the one before
constexpr x86::reg32 kTextureNext = 0xa937a8;   // the next free address of texture memory
constexpr x86::reg32 kTexturing = 0xa92354;     // 1 while a texture is selected
constexpr x86::reg32 kCombineLocal = 0xa91f2c;  // THRASH_setstate 6

x86::reg8 byteAt(win32::WinApplication* app, x86::reg32 address)
{
    return app->getMemory<x86::reg8>(address);
}

/* log2 of a power of two, from the exponent of the float it converts to. */
x86::sreg32 exponentOf(x86::reg32 value)
{
    const float converted = float(double(x86::sreg32(value)));
    x86::reg32 bits;
    std::memcpy(&bits, &converted, sizeof bits);
    return x86::sreg32(bits - 0x3f800000) >> 23;
}

/* What THRASH_talloc(width, height, format, -, levels) works out before it
 * allocates: the record's Glide LODs, aspect and format, the memory it takes,
 * and where it starts -- moved to the next 2 MB bank if it would straddle one,
 * which `apply` writes back as voodoo2a does. */
struct TexturePlan
{
    x86::reg32 smallLod, largeLod, aspect, format, size, start;
    bool fits;
};

TexturePlan planTexture(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 esp, bool apply)
{
    namespace gl = win32::glide2x::direct;
    const x86::sreg32 widthLog = exponentOf(at(app, esp + 4));
    const x86::sreg32 heightLog = exponentOf(at(app, esp + 8));
    const x86::reg32 formatIndex = at(app, esp + 0xc);
    const x86::sreg32 levels = x86::sreg32(at(app, esp + 0x14));
    const x86::sreg32 largest = widthLog > heightLog ? widthLog : heightLog;
    x86::sreg32 smallest = largest - levels;
    if (smallest < 0)
        smallest = 0;
    TexturePlan plan;
    plan.largeLod = byteAt(app, 0xa92338 + x86::reg32(largest));
    plan.smallLod = byteAt(app, 0xa92338 + x86::reg32(smallest));
    plan.aspect = byteAt(app, 0xa92344 + x86::reg32(widthLog - heightLog));
    plan.format = byteAt(app, 0xa92348 + formatIndex);
    plan.size = gl::texCalcMemRequired(app, cpu, plan.smallLod, plan.largeLod, plan.aspect, plan.format);
    plan.start = at(app, kTextureNext);
    const x86::reg32 firstBank = (plan.start - gl::texMinAddress(app, cpu, 0)) & 0xffe00000;
    const x86::reg32 lastBank = (plan.size + plan.start - 1 - gl::texMinAddress(app, cpu, 0)) & 0xffe00000;
    if (lastBank != firstBank)
    {
        plan.start = gl::texMinAddress(app, cpu, 0) + lastBank;
        if (apply)
            put(app, kTextureNext, plan.start);
    }
    const x86::reg32 top = (gl::texMaxAddress(app, cpu, 0) + 0xf) & ~x86::reg32(0xf);
    plan.fits = x86::sreg32(top - plan.start) >= x86::sreg32(plan.size);
    return plan;
}

/* THRASH_talloc: a record of Glide's GrTexInfo -- small and large LOD, aspect,
 * format, texels -- then where it lies in texture memory, 3 (both halves of
 * the mipmap) and the record before; 0 when it does not fit.  The record comes
 * from voodoo2a's own malloc, which THRASH_treset gives them back to. */
bool textureAllocateNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    const TexturePlan plan = planTexture(app, cpu, esp, true);
    x86::reg32 record = 0;
    if (plan.fits)
        record = callRuntime(app, cpu, kMalloc, 0x20);
    if (record != 0)
    {
        put(app, record + 4, plan.largeLod);
        put(app, record, plan.smallLod);
        put(app, record + 8, plan.aspect);
        put(app, record + 0xc, byteAt(app, 0xa92348 + at(app, esp + 0xc)));
        put(app, record + 0x10, 0);
        put(app, record + 0x18, 3);
        put(app, record + 0x14, plan.start);
        put(app, kTextureNext, plan.start + plan.size);
        put(app, record + 0x1c, at(app, kTextures));
        put(app, kTextures, record);
    }
    cpu.eax = record;
    cpu.esp = esp + 4 + 0x14;
    return true;
}

/* THRASH_treset: all of texture memory free again, every record given back. */
void resetTextures(win32::WinApplication* app, x86::CPU& cpu)
{
    win32::glide2x::direct::resetTextures();
    put(app, kTextureNext, win32::glide2x::direct::texMinAddress(app, cpu, 0));
    while (const x86::reg32 record = at(app, kTextures))
    {
        const x86::reg32 before = at(app, record + 0x1c);
        callRuntime(app, cpu, kFree, record);
        put(app, kTextures, before);
    }
}

bool textureResetNative(win32::WinApplication* app, x86::CPU& cpu)
{
    resetTextures(app, cpu);
    cpu.eax = 1;
    cpu.esp += 4;
    return true;
}

/* THRASH_tupdate(record, texels, palette): new texels for a record, downloaded;
 * a palette for a palettised one. */
bool textureUpdateNative(win32::WinApplication* app, x86::CPU& cpu)
{
    namespace gl = win32::glide2x::direct;
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 record = at(app, esp + 4);
    const x86::reg32 texels = at(app, esp + 8);
    const x86::reg32 palette = at(app, esp + 0xc);
    if (texels != 0)
    {
        put(app, record + 0x10, texels);
        gl::texDownloadMipMap(app, cpu, 0, at(app, record + 0x14), at(app, record + 0x18), record);
    }
    if (palette != 0)
        gl::texDownloadTable(app, cpu, 0, 2, palette);
    cpu.eax = record;
    cpu.esp = esp + 4 + 0xc;
    return true;
}

/* THRASH_settexture(record), and THRASH_setstate 1: the texture drawn with, or
 * none -- the colour and alpha combine follow, texture times vertex colour or
 * the vertex colour alone. */
void selectTexture(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record)
{
    namespace gl = win32::glide2x::direct;
    if (record == 0)
    {
        if (at(app, kTexturing) != 0)
        {
            gl::colorCombine(app, cpu, 1, 0, at(app, kCombineLocal), 2, 0);
            gl::alphaCombine(app, cpu, 1, 0, at(app, kCombineLocal), 2, 0);
            put(app, kTexturing, 0);
        }
        return;
    }
    gl::texSource(app, cpu, 0, at(app, record + 0x14), at(app, record + 0x18), record);
    if (at(app, kTexturing) != 1)
    {
        gl::colorCombine(app, cpu, 3, 1, at(app, kCombineLocal), 1, 0);
        gl::alphaCombine(app, cpu, 3, 1, at(app, kCombineLocal), 1, 0);
        put(app, kTexturing, 1);
    }
}

bool setTextureNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    selectTexture(app, cpu, at(app, esp + 4));
    cpu.eax = 1;
    cpu.esp = esp + 4 + 4;
    return true;
}

constexpr x86::reg32 kBuffer = 0xa91f04;        // the buffer drawn into, THRASH_window's less one
constexpr x86::reg32 kClearColour = 0xa91efc;   // THRASH_setstate 3
constexpr x86::reg32 kClearDepth = 0xa91f00;    // set with the depth test, THRASH_setstate 4
constexpr x86::reg32 kSwapInterval = 0xa91f0c;  // THRASH_setstate 0x67
constexpr x86::reg32 kLockHook = 0xa91f38;      // the game's own function, told 1 before and 0 after a lock

/* The game's hook around the frame buffer's locks, when it has set one: a
 * stdcall of its own code with one argument. */
void lockHook(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 locking)
{
    const x86::reg32 hook = at(app, kLockHook);
    if (hook == 0)
        return;
    const x86::reg32 esp = cpu.esp;
    cpu.esp -= 4;
    put(app, cpu.esp, locking);
    cpu.esp -= 4;
    app->dynamic_call(hook, cpu);
    cpu.esp = esp;
}

/* THRASH_window(buffer): which buffer to draw into, counted from 1; 0 is 1. */
bool windowNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    x86::reg32 buffer = at(app, esp + 4);
    if (buffer == 0)
        buffer = 1;
    put(app, kBuffer, buffer - 1);
    win32::glide2x::direct::renderBuffer(app, cpu, buffer - 1);
    cpu.eax = 1;
    cpu.esp = esp + 4 + 4;
    return true;
}

/* THRASH_clearwindow: colour and depth cleared to what THRASH_setstate set. */
bool clearWindowNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg16 depth = app->getMemory<x86::reg16>(kClearDepth);
    win32::glide2x::direct::bufferClear(app, cpu, at(app, kClearColour), 0, depth);
    cpu.eax = depth;
    cpu.esp += 4;
    return true;
}

/* THRASH_flushwindow and THRASH_idle: nothing to do on a Voodoo2. */
bool nothingNative(win32::WinApplication* app, x86::CPU& cpu)
{
    NFS2_USE(app);
    cpu.esp += 4;
    return true;
}

/* THRASH_pageflip: once nothing is left pending -- or after four million
 * tries -- the buffers swapped, every THRASH_setstate 0x67 frames. */
void pageFlip(win32::WinApplication* app, x86::CPU& cpu)
{
    namespace gl = win32::glide2x::direct;
    x86::reg32 tries = 4000000;
    while (gl::bufferNumPending(app, cpu) != 0 && --tries != 0)
    {
    }
    gl::bufferSwap(app, cpu, at(app, kSwapInterval));
}

bool pageFlipNative(win32::WinApplication* app, x86::CPU& cpu)
{
    pageFlip(app, cpu);
    cpu.esp += 4;
    return true;
}

/* THRASH_sync(what): 0 waits for the card, 1 asks whether it is busy, 2 waits
 * for the start of a vertical retrace, 3 tells how much of the FIFO is free. */
bool syncNative(win32::WinApplication* app, x86::CPU& cpu)
{
    namespace gl = win32::glide2x::direct;
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 what = at(app, esp + 4);
    x86::reg32 result = 0;
    switch (what)
    {
    case 0:
        gl::sstIdle(app, cpu);
        break;
    case 1:
        gl::sstIsBusy(app, cpu);
        result = 1;  // grSstIsBusy leaves eax as it found it: the 1 just read
        break;
    case 2:
    {
        // Out of the retrace there is now, then into the next one, four
        // million tries between the two.
        x86::reg32 tries = 4000000;
        while (gl::sstVRetraceOn(app, cpu) != 0)
            if (--tries == 0)
                break;
        while (gl::sstVRetraceOn(app, cpu) == 0)
            if (--tries == 0)
                break;
        break;
    }
    case 3:
        result = 0xffff - ((gl::sstStatus(app, cpu) >> 12) & 0xffff);
        break;
    default:
        break;
    }
    cpu.eax = result;
    cpu.esp = esp + 4 + 4;
    return true;
}

/* THRASH_clip(minX, minY, maxX, maxY). */
bool clipNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    win32::glide2x::direct::clipWindow(app, cpu, at(app, esp + 4), at(app, esp + 8), at(app, esp + 0xc),
                                       at(app, esp + 0x10));
    cpu.eax = 1;
    cpu.esp = esp + 4 + 0x10;
    return true;
}

/* THRASH_lockwindow: the frame buffer locked for the game to write into, as
 * a record of voodoo2a's malloc -- the pointer, the stride, 4, two of
 * THRASH_setstate's values and the buffer -- or 0.  It then asks for a read
 * lock as well, which comes to nothing. */
bool lockWindowNative(win32::WinApplication* app, x86::CPU& cpu)
{
    namespace gl = win32::glide2x::direct;
    const x86::reg32 esp = cpu.esp;
    lockHook(app, cpu, 1);
    const x86::reg32 info = esp - 0x24;  // GrLfbInfo_t, where voodoo2a keeps it
    put(app, info, 0x14);
    x86::reg32 record = 0;
    if (gl::lfbLock(app, cpu, 1, at(app, kBuffer), 0xff, 0, 0, info) != 0)
    {
        /* Read before malloc runs: its frame lies over the info, which in
         * voodoo2a's own frame sat above it. */
        const x86::reg32 pixels = at(app, info + 4);
        const x86::reg32 stride = at(app, info + 8);
        record = callRuntime(app, cpu, kMalloc, 0x18);
        if (record != 0)
        {
            put(app, record, pixels);
            put(app, record + 8, 4);
            put(app, record + 4, stride);
            put(app, record + 0xc, at(app, 0xa91f18));
            put(app, record + 0x10, at(app, 0xa91f1c));
            put(app, record + 0x14, at(app, kBuffer));
        }
    }
    put(app, info, 0x14);
    gl::lfbLock(app, cpu, 0, at(app, kBuffer), 0xff, 0, 0, info);
    if (record == 0)
        lockHook(app, cpu, 0);
    cpu.eax = record;
    cpu.esp = esp + 4;
    return true;
}

/* THRASH_unlockwindow(record): both locks let go, the record freed. */
bool unlockWindowNative(win32::WinApplication* app, x86::CPU& cpu)
{
    namespace gl = win32::glide2x::direct;
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 record = at(app, esp + 4);
    if (record != 0)
    {
        const x86::reg32 buffer = at(app, record + 0x14);
        gl::lfbUnlock(app, cpu, 1, buffer);
        gl::lfbUnlock(app, cpu, 0, buffer);
        callRuntime(app, cpu, kFree, record);
        lockHook(app, cpu, 0);
    }
    cpu.eax = 1;
    cpu.esp = esp + 4 + 4;
    return true;
}

/* THRASH_readrect(x, y, width, height, to): the buffer drawn into, read back
 * at two bytes a pixel. */
bool readRectNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    lockHook(app, cpu, 1);
    const x86::reg32 width = at(app, esp + 0xc);
    const x86::reg32 result = win32::glide2x::direct::lfbReadRegion(
        app, cpu, at(app, kBuffer), at(app, esp + 4), at(app, esp + 8), width, at(app, esp + 0x10), width * 2,
        at(app, esp + 0x14));
    lockHook(app, cpu, 0);
    cpu.eax = result;
    cpu.esp = esp + 4 + 0x14;
    return true;
}

/* A stdcall of a guest function -- the game's hooks, Win32 through voodoo2a's
 * import table, Glide through its table, DirectDraw's methods -- made the way
 * voodoo2a's code makes it.  Its result. */
x86::reg32 stdcall(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address,
                   std::initializer_list<x86::reg32> arguments)
{
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 first = esp - 4 * x86::reg32(arguments.size());
    x86::reg32 slot = first;
    for (const x86::reg32 argument : arguments)
    {
        put(app, slot, argument);
        slot += 4;
    }
    cpu.esp = first - 4;
    app->dynamic_call(address, cpu);
    cpu.esp = esp;
    return cpu.eax;
}

/* voodoo2a's imports and state for setting the card up. */
constexpr x86::reg32 kLoadLibrary = 0xa903ec, kGetProcAddress = 0xa903d8, kFreeLibrary = 0xa90390;
constexpr x86::reg32 kCreateEvent = 0xa90368, kCloseHandle = 0xa90364, kSetForegroundWindow = 0xa9035c;
constexpr x86::reg32 kPostMessage = 0xa90358, kWaitForSingleObject = 0xa90438, kSetEvent = 0xa90404;
constexpr x86::reg32 kDirectDrawCreate = 0xa90448;
constexpr x86::reg32 kGlideModule = 0xa91f50;
constexpr x86::reg32 kHardware = 0xa93714;      // GrHwConfiguration: boards, then the first one's type, memory...
constexpr x86::reg32 kInfo = 0xa91f54;          // what THRASH_about hands the game
constexpr x86::reg32 kModes = 0xa92018;         // 17 modes of 0x28 bytes: width, height, depth, ..., buffers
constexpr x86::reg32 kResolutions = 0xa922e8;   // each mode's GR_RESOLUTION
constexpr x86::reg32 kFlags = 0xa91ef4;         // 1 set up, 2 a window open
constexpr x86::reg32 kWindow = 0xa91f08;        // the game's window
constexpr x86::reg32 kModeEvent = 0xa91f40;
constexpr x86::reg32 kDirectDraw = 0xa91f48;
constexpr x86::reg32 kModeThread = 0xa92334;    // 1 while the window's thread is to set the mode

x86::reg32 glide(win32::WinApplication* app, x86::reg32 entry)
{
    return at(app, entry);
}

/* sub_a83220: glide2x.dll loaded and its functions looked up into the table
 * from 0xa93614, the drawing pointed at the plain ones.  0 without it. */
x86::reg32 loadGlide(win32::WinApplication* app, x86::CPU& cpu)
{
    static const x86::reg32 kNames[] = {
        0xa91210, 0xa91220, 0xa91238, 0xa9124c, 0xa91260, 0xa91270, 0xa91284, 0xa91298, 0xa912b0, 0xa912c4,
        0xa912e4, 0xa912fc, 0xa91310, 0xa91324, 0xa91334, 0xa91344, 0xa91358, 0xa91368, 0xa91378, 0xa9138c,
        0xa913a0, 0xa913b4, 0xa913d0, 0xa913e0, 0xa913f4, 0xa91404, 0xa91418, 0xa91430, 0xa91448, 0xa91464,
        0xa91474, 0xa91488, 0xa91498, 0xa914a8, 0xa914b8, 0xa914c8, 0xa914e0, 0xa914f4, 0xa91508, 0xa91518,
        0xa91528, 0xa91544, 0xa91558, 0xa9156c, 0xa91580, 0xa9159c, 0xa915b4, 0xa915c8, 0xa915dc, 0xa915ec,
        0xa91600, 0xa91618, 0xa91630, 0xa91640, 0xa9164c, 0xa91664, 0xa91678, 0xa91690, 0xa916a8,
    };
    if (at(app, 0xa91f34) == 0)
        put(app, 0xa91f34, 0xa83a80);
    const x86::reg32 module = stdcall(app, cpu, at(app, kLoadLibrary), { 0xa91204 });
    put(app, kGlideModule, module);
    if (module == 0)
        return 0;
    x86::reg32 entry = 0xa93614;
    for (const x86::reg32 name : kNames)
    {
        put(app, entry, stdcall(app, cpu, at(app, kGetProcAddress), { at(app, kGlideModule), name }));
        entry += 4;
    }
    put(app, kTriangleDraw, at(app, kGrDrawTriangle));
    put(app, kQuadDraw, at(app, kGrDrawTriangle));
    put(app, kLineDraw, at(app, kGrDrawLine));
    return 1;
}

/* sub_a83800: glide2x.dll let go. */
void freeGlide(win32::WinApplication* app, x86::CPU& cpu)
{
    if (const x86::reg32 module = at(app, kGlideModule))
        stdcall(app, cpu, at(app, kFreeLibrary), { module });
    put(app, kGlideModule, 0);
}

/* sub_a83ac0: from the hardware found, how many colour and auxiliary buffers
 * each mode can have in the frame buffer's memory, and the info's card type
 * (+0x6c) and texture memory (+0x70). */
void measureModes(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 info)
{
    namespace gl = win32::glide2x::direct;
    x86::sreg32 memory = x86::sreg32(at(app, kHardware + 8) << 20);
    const x86::reg32 type = at(app, kHardware + 4);
    const x86::reg32 voodoo2 = type == 1 ? 1 : 0;
    put(app, 0xa91ef8, voodoo2);
    x86::reg32 sli = 0;
    if (voodoo2 == 0)
        sli = at(app, kHardware + 0x14);
    if (sli != 0)
        memory += memory;
    x86::reg32 mode = kModes + 0x28;
    for (int i = 1; i <= 16; ++i, mode += 0x28)
    {
        const x86::sreg32 pixels = x86::sreg32(at(app, mode) * at(app, mode + 4));
        const x86::sreg32 frame = (x86::sreg32(at(app, mode + 8)) >> 3) * pixels;
        put(app, mode + 0x14, x86::reg32(memory / frame));
        if (at(app, mode) == 0x3c0 && sli == 0)
            put(app, mode + 0x14, 0);
        if (x86::sreg32(at(app, mode)) > 0x320 && at(app, 0xa91ef8) != 0)
            put(app, mode + 0x14, 0);
        if (at(app, mode + 0x10) == 0 || x86::sreg32(at(app, mode + 0x14)) <= 0)
        {
            put(app, mode + 0x18, 0);
            put(app, mode + 0x14, 0);
            continue;
        }
        put(app, mode + 0x18, at(app, mode + 0x14) - 1);
        if (x86::sreg32(at(app, mode + 0x14)) > 3)
            put(app, mode + 0x14, 3);
        if (x86::sreg32(at(app, mode + 0x18)) > 3)
            put(app, mode + 0x18, 3);
    }
    put(app, info + 0x6c, type);
    if (type == 0)
    {
        const x86::reg32 revision = at(app, kHardware + 0xc);
        if ((x86::sreg32(revision) >> 8) == 1)
            put(app, info + 0x6c, 2);
        if ((x86::sreg32(revision) >> 12) == 1)
            put(app, info + 0x6c, 3);
        if (revision == 0 && x86::sreg32(at(app, kHardware + 8)) >= 6)
        {
            put(app, 0xa91f10, 0);
            put(app, info + 0x6c, 3);
        }
    }
    x86::reg32 texture = gl::texMaxAddress(app, cpu, 0);
    texture -= gl::texMinAddress(app, cpu, 0);
    texture -= gl::texMinAddress(app, cpu, 0);
    put(app, info + 0x70, texture);
    if (x86::sreg32(texture) > 0)
        put(app, info + 0x70, (texture + 0xf) & ~x86::reg32(0xf));
    put(app, info + 0x74, 1);
}

/* THRASH_about: the driver described, once -- its name, what it can do, its
 * texture formats (0xa91fd4) and its modes (0xa92018) -- from a look at the
 * card through Glide. */
bool aboutNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    if (at(app, kInfo) == 0)
    {
        std::memset(&app->getMemory<x86::reg8>(kInfo), 0, 0x80);
        put(app, kInfo, 0x33444658);  // "XFD3"
        put(app, 0xa91f5c, 0x68);
        for (const x86::reg32 field : { 0xa91f64u, 0xa91f6cu, 0xa91f70u, 0xa91f78u, 0xa91f7cu })
            put(app, field, 1);
        put(app, 0xa91f58, 0x80);
        put(app, 0xa91f68, 0x100);
        put(app, 0xa91f88, 4);
        put(app, 0xa91f90, 0x10);
        put(app, 0xa91f94, kModes);
        put(app, 0xa91f74, 0x100);
        app->getMemory<x86::reg8>(0xa91f60) = x86::reg8((app->getMemory<x86::reg8>(0xa91f60) | 0xe) & 0xfe);
        put(app, 0xa91f8c, 0xa92000);
        put(app, 0xa91f80, 9);
        put(app, 0xa91f84, 0xa91fd4);
        if (win32::glide2x::fullColourTextures())
        {
            /* tools/apply_full_colour.py: THRASH format 6, ARGB 8888, taken,
             * and handed to Glide as such. */
            put(app, 0xa91fd4 + 6 * 4, 1);
            app->getMemory<x86::reg8>(0xa92348 + 6) = x86::reg8(win32::glide2x::kTexFmtArgb8888);
        }
        std::strcpy(&app->getMemory<char>(kInfo + 0x4c), &app->getMemory<char>(0xa916d4));
        put(app, 0xa91fc0, 0);
        if (app->getMemory<x86::reg8>(kFlags) & 1)
        {
            measureModes(app, cpu, kInfo);
        }
        else if (loadGlide(app, cpu) != 0)
        {
            if (at(app, 0xa93614) != 0 && at(app, 0xa9361c) != 0 && at(app, 0xa93618) != 0)
            {
                x86::sreg32 boards = x86::sreg32(stdcall(app, cpu, glide(app, 0xa9361c), { kHardware }));
                if (boards != 0)
                    boards = x86::sreg32(at(app, kHardware));
                if (boards > 0)
                {
                    stdcall(app, cpu, glide(app, 0xa93614), {});
                    stdcall(app, cpu, glide(app, 0xa93618), { kHardware });
                    measureModes(app, cpu, kInfo);
                    stdcall(app, cpu, glide(app, 0xa93620), {});
                }
            }
            freeGlide(app, cpu);
        }
    }
    cpu.eax = kInfo;
    cpu.esp = esp + 4;
    return true;
}

/* THRASH_selectdisplay(board). */
x86::reg32 selectDisplay(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 board)
{
    if (x86::sreg32(board) >= x86::sreg32(at(app, kHardware)))
        return 0;
    stdcall(app, cpu, glide(app, 0xa93624), { board });
    return 1;
}

bool selectDisplayNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    cpu.eax = selectDisplay(app, cpu, at(app, esp + 4));
    cpu.esp = esp + 4 + 4;
    return true;
}

/* THRASH_init: the colour table (a byte to Glide's float), the event the
 * window's thread signals, Glide up, board 0, the modes measured, and
 * THRASH_restore run at exit.  The number of boards, or 0. */
bool initNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    x86::reg32 result = 0;
    if (!(app->getMemory<x86::reg8>(kFlags) & 1))
    {
        for (int i = 0; i < 0x100; ++i)
            app->getMemory<float>(kColourTable + 4 * x86::reg32(i)) = float(double(i));
        if (at(app, 0xa91f34) == 0)
            put(app, 0xa91f34, 0xa83a80);
        if (at(app, kModeEvent) == 0)
            put(app, kModeEvent, stdcall(app, cpu, at(app, kCreateEvent), { 0, 0, 0, 0 }));
        if (loadGlide(app, cpu) != 0)
        {
            stdcall(app, cpu, glide(app, 0xa93614), {});
            if (stdcall(app, cpu, glide(app, 0xa93618), { kHardware }) != 0 && selectDisplay(app, cpu, 0) != 0)
            {
                result = at(app, kHardware);
                measureModes(app, cpu, kInfo);
                if (result != 0)
                {
                    put(app, kFlags, 1);
                    // sub_a86c40, atexit: THRASH_restore (by way of sub_a83f40) when the game ends.
                    const x86::reg32 exits = at(app, 0xa93840);
                    if (x86::sreg32(exits) < 0x20)
                    {
                        put(app, 0xa937bc + (exits + 1) * 4, 0xa83f40);
                        put(app, 0xa93840, exits + 1);
                    }
                }
            }
        }
    }
    cpu.eax = result;
    cpu.esp = esp + 4;
    return true;
}

/* sub_a84020: DirectDraw, on a card that shares the screen, for the game's
 * window to own it -- exclusive and full screen, or only exclusive. */
x86::reg32 takeScreen(win32::WinApplication* app, x86::CPU& cpu)
{
    if (at(app, kDirectDraw) != 0 || at(app, 0xa91ef8) != 0 || at(app, 0xa91f10) == 0)
        return 0;
    if (stdcall(app, cpu, at(app, kDirectDrawCreate), { 0, kDirectDraw, 0 }) != 0)
        return 0;
    const x86::reg32 level = at(app, 0xa91f44) != 0 ? 0x10 : 0x11;
    const x86::reg32 object = at(app, kDirectDraw);
    if (stdcall(app, cpu, at(app, at(app, object) + 0x50), { object, at(app, kWindow), level }) == 0)
        return 1;
    stdcall(app, cpu, at(app, at(app, object) + 8), { object });
    put(app, kDirectDraw, 0);
    return 0;
}

/* sub_a840b0: DirectDraw let go. */
void releaseScreen(win32::WinApplication* app, x86::CPU& cpu)
{
    if (const x86::reg32 object = at(app, kDirectDraw))
    {
        stdcall(app, cpu, at(app, at(app, object) + 8), { object });
        put(app, kDirectDraw, 0);
    }
}

/* The state THRASH_setstate sets, without a call of its own: an argument it
 * writes back to (state 0xb) goes to `slot`, or nowhere for 0. */
x86::reg32 applyState(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state, x86::reg32 value, x86::reg32 slot);

/* sub_a84180(mode, buffers, auxiliary): the Glide window opened in that mode,
 * and the renderer's state set to its defaults.  A window already open is
 * closed first; no buffers at all only closes it, and gives back what ebx
 * held. */
x86::reg32 openMode(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mode, x86::reg32 buffers,
                    x86::reg32 auxiliary)
{
    if (app->getMemory<x86::reg8>(kFlags) & 2)
    {
        stdcall(app, cpu, glide(app, 0xa93628), {});  // grSstWinClose
        app->getMemory<x86::reg8>(kFlags) = x86::reg8(app->getMemory<x86::reg8>(kFlags) & 0xfd);
    }
    if (buffers == 0)
        return cpu.ebx;
    x86::reg32 resolution = 0xff;
    if (at(app, 0xa91ef8) == 0 || at(app, 0xa91f44) == 0)
        resolution = at(app, kResolutions + mode * 4);
    const x86::reg32 entry = kModes + mode * 0x28;
    if (auxiliary != 0 && x86::sreg32(buffers) > x86::sreg32(at(app, entry + 0x18)))
        auxiliary = 0;
    x86::reg32 opened = stdcall(app, cpu, glide(app, 0xa9362c),
                                { at(app, kWindow), resolution, 0, 0, 0, buffers, auxiliary });
    if (opened == 0 && resolution == 0xff)
        opened = stdcall(app, cpu, glide(app, 0xa9362c),
                         { at(app, kWindow), at(app, kResolutions + mode * 4), 0, 0, 0, buffers, auxiliary });
    if (opened == 0)
        return 0;
    if (at(app, 0xa91ef8) == 0)
    {
        put(app, 0xa91f18, stdcall(app, cpu, glide(app, 0xa936f0), {}));  // grSstScreenWidth
        put(app, 0xa91f1c, stdcall(app, cpu, glide(app, 0xa936f4), {}));  // grSstScreenHeight
    }
    else
    {
        put(app, 0xa91f18, at(app, entry));
        put(app, 0xa91f1c, at(app, entry + 4));
    }
    resetTextures(app, cpu);
    stdcall(app, cpu, glide(app, 0xa93638), { 0x10 });                  // grAlphaTestReferenceValue
    win32::glide2x::direct::texCombine(app, cpu, 0, 1, 0, 1, 0, 0, 0);
    stdcall(app, cpu, glide(app, 0xa936e8), { 3, 1 });                 // grHints
    static const x86::reg32 kDefaults[][2] = {
        { 1, 0 }, { 2, 1 }, { 7, 1 }, { 6, 1 }, { 0xa, 2 }, { 0xb, 1 }, { 3, 0 }, { 0xc, 0 },
        { 5, 1 }, { 0xe, 0 }, { 0xf, 0xffffffff }, { 0x68, 0 }, { 0xd, 0 }, { 0x65, 0x3f800000 },
        { 0x18, 0 }, { 8, 0 },
    };
    for (const auto& pair : kDefaults)
        applyState(app, cpu, pair[0], pair[1], 0);
    applyState(app, cpu, 4, x86::sreg32(auxiliary) >= 1 ? 2 : 0, 0);
    app->getMemory<x86::reg8>(kFlags) = x86::reg8(app->getMemory<x86::reg8>(kFlags) | 2);
    return opened;
}

bool openModeNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    cpu.eax = openMode(app, cpu, at(app, esp + 4), at(app, esp + 8), at(app, esp + 0xc));
    cpu.esp = esp + 4 + 0xc;
    return true;
}

/* sub_a84360: what the game's window thread runs on message 0x465 -- the
 * mode the game thread asked for, opened there, the result in voodoo2a's
 * variable and the caller's, and the game thread let go. */
bool windowThreadNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    if (at(app, kModeThread) != 0)
    {
        const x86::reg32 packed = at(app, esp + 0x14);
        const x86::reg32 opened = openMode(app, cpu, at(app, esp + 0x10), packed & 0xff, packed >> 8);
        put(app, 0xa91f14, opened);
        put(app, at(app, esp + 0x18), opened == 0 ? 1 : 0);
        if (const x86::reg32 event = at(app, kModeEvent))
            stdcall(app, cpu, at(app, kSetEvent), { event });
    }
    cpu.eax = 1;
    cpu.esp = esp + 4 + 0x18;
    return true;
}

/* THRASH_setvideomode(mode, buffers, auxiliary): the game's window found, the
 * screen taken, the window brought forward, and the mode opened -- on the
 * window's own thread when the game hooked its messages, waiting for it. */
x86::reg32 setVideoMode(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mode, x86::reg32 buffers,
                        x86::reg32 auxiliary)
{
    if (const x86::reg32 findWindow = at(app, 0xa91f30))
        put(app, kWindow, stdcall(app, cpu, findWindow, {}));
    takeScreen(app, cpu);
    if (const x86::reg32 window = at(app, kWindow))
        stdcall(app, cpu, at(app, kSetForegroundWindow), { window });
    if (at(app, 0xa91f3c) != 0 && at(app, kWindow) != 0 && at(app, kModeEvent) != 0)
    {
        stdcall(app, cpu, at(app, 0xa91f3c), { 0x465, 0xa84360 });
        put(app, kModeThread, 1);
        stdcall(app, cpu, at(app, kPostMessage), { at(app, kWindow), 0x465, mode, buffers + (auxiliary << 8) });
        stdcall(app, cpu, at(app, kWaitForSingleObject), { at(app, kModeEvent), 0xffffffff });
        put(app, kModeThread, 0);
        return at(app, 0xa91f14);
    }
    return openMode(app, cpu, mode, buffers, auxiliary);
}

bool setVideoModeNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    cpu.eax = setVideoMode(app, cpu, at(app, esp + 4), at(app, esp + 8), at(app, esp + 0xc));
    cpu.esp = esp + 4 + 0xc;
    return true;
}

/* THRASH_restore: everything let go -- the window and its mode, DirectDraw,
 * the event, Glide. */
bool restoreNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    if (at(app, kFlags) != 0)
    {
        if (app->getMemory<x86::reg8>(kFlags) & 2)
        {
            setVideoMode(app, cpu, 0, 0, 0);
            releaseScreen(app, cpu);
        }
        if (at(app, 0xa91f3c) != 0)
            put(app, 0xa91f3c, 0);
        if (const x86::reg32 event = at(app, kModeEvent))
        {
            stdcall(app, cpu, at(app, kCloseHandle), { event });
            put(app, kModeEvent, 0);
        }
        stdcall(app, cpu, glide(app, 0xa93620), {});  // grGlideShutdown
        freeGlide(app, cpu);
        put(app, 0xa91f34, 0);
        put(app, kFlags, 0);
    }
    cpu.eax = 1;
    cpu.esp = esp + 4;
    return true;
}

/* sub_a83830 and sub_a83870: Glide's fog table for a density -- 1 - e^(-d w)
 * at each of its 64 w, scaled to 1 at the last, times 255 -- worked out as
 * voodoo2a's code works it out, x87 operation by operation in double.
 * w(i) = 2^(3 + i/4) / (8 - i%4), exact (Watcom's pow of 2 with a whole
 * exponent). */
double fogW(int i)
{
    return std::ldexp(1.0, 3 + (i >> 2)) / double(8 - (i & 3));
}

double fogExp(x86::CPU& cpu, double w, float density)
{
    const double x = -(w * double(density));
    const double t = 1.4426950408889634 * x;
    const double fraction = cpu.fpu.rem(t, 1.0);
    return cpu.fpu.scale(1.0 + cpu.fpu.f2xm1(fraction), t);
}

void fogTable(win32::WinApplication* app, x86::CPU& cpu, float density, x86::reg8 (&table)[64])
{
    const float scale = float(1.0 / (1.0 - fogExp(cpu, fogW(63), density)));
    for (int i = 0; i < 64; ++i)
    {
        float value = float((1.0 - fogExp(cpu, fogW(i), density)) * double(scale));
        x86::reg32 bits;
        std::memcpy(&bits, &value, sizeof bits);
        if (x86::sreg32(bits) > 0x3f800000)
            value = 1.0f;
        else if (0.0 > double(value))  // fcomp leaves a NaN as it is
            value = 0.0f;
        const double scaled = double(value) * double(app->getMemory<float>(0xa916cc));
        table[i] = x86::reg8(cpu.fpu.toInteger<x86::sreg32>(scaled));
    }
}

/* THRASH_setstate(state, value): one piece of the renderer's state, to Glide or
 * to voodoo2a's own variables.  1 for done, 0 for a state or value it does not
 * know. */
x86::reg32 applyState(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 state, x86::reg32 value, x86::reg32 slot)
{
    namespace gl = win32::glide2x::direct;
    x86::reg32 result = 1;
    switch (state)
    {
    case 1:
        selectTexture(app, cpu, value);
        break;
    case 2:
        gl::cullMode(app, cpu, value);
        break;
    case 3:
        put(app, 0xa91efc, value);
        break;
    case 4:  // depth: off, or on, compared less-or-equal
        if (value == 0)
        {
            gl::depthBufferFunction(app, cpu, 7);
            gl::depthMask(app, cpu, 0);
        }
        else if (value == 1 || value == 2)
        {
            gl::depthBufferMode(app, cpu, value);
            gl::depthBufferFunction(app, cpu, 3);
            put(app, 0xa91f00, 0xffff);
            gl::depthMask(app, cpu, 1);
        }
        else
        {
            result = 0;
        }
        break;
    case 5:
        gl::ditherMode(app, cpu, value != 0 ? 2 : 0);
        break;
    case 6:  // the vertex colour alone, or modulating the texture
    {
        const x86::reg32 local = value == 0 ? 1 : 0;
        put(app, 0xa91f2c, local);
        if (at(app, 0xa92354) == 1)
        {
            gl::colorCombine(app, cpu, 3, 1, local, 1, 0);
            gl::alphaCombine(app, cpu, 3, 1, local, 1, 0);
        }
        else
        {
            gl::colorCombine(app, cpu, 1, 0, local, 2, 0);
            gl::alphaCombine(app, cpu, 1, 0, local, 2, 0);
        }
        break;
    }
    case 7:
        gl::texFilterMode(app, cpu, 0, value, value);
        break;
    case 8:
        gl::texLodBiasValue(app, cpu, 0, asFloat(value));
        break;
    case 9:
        put(app, 0xa91f20, value);
        drawPointers(app, at(app, 0xa91f24), value);
        break;
    case 0xa:  // chroma key and alpha test
        if (value == 0)
        {
            gl::chromakeyMode(app, cpu, 0);
            gl::alphaTestFunction(app, cpu, 7);
        }
        else if (value == 1 || value == 2)
        {
            gl::chromakeyMode(app, cpu, value == 1 ? 1 : 0);
            gl::alphaTestFunction(app, cpu, 4);
        }
        else
        {
            result = 0;
        }
        break;
    case 0xb:  // mipmapping; 3 is mode 1 drawn in two passes
    {
        x86::reg32 twoPass = 0;
        x86::reg32 mode = value;
        if (value == 3)
        {
            twoPass = 1;
            mode = 1;
            if (slot != 0)
                put(app, slot, 1);  // over the argument, as voodoo2a does
        }
        if (twoPass != at(app, 0xa91f24))
        {
            put(app, 0xa91f24, twoPass);
            gl::alphaBlendFunction(app, cpu, 1, 5, 4, 0);
            gl::texCombine(app, cpu, 0, 1, 0, 1, 0, 0, 0);
        }
        drawPointers(app, at(app, 0xa91f24), at(app, 0xa91f20));
        gl::texMipMapMode(app, cpu, 0, mode, at(app, 0xa91f24));
        break;
    }
    case 0xc:
        gl::chromakeyValue(app, cpu, value);
        break;
    case 0xd:  // 0 clamps, anything else wraps
    {
        const x86::reg32 clamp = value == 0 ? 1 : 0;
        gl::texClampMode(app, cpu, 0, clamp, clamp);
        break;
    }
    case 0xe:  // fog by density -- 1 / value -- or off
        if (value == 0)
        {
            gl::fogMode(app, cpu, 0);
        }
        else
        {
            gl::fogMode(app, cpu, 2);
            x86::reg8 table[64];
            fogTable(app, cpu, float(1.0 / double(x86::sreg32(value))), table);
            gl::fogTable(app, cpu, table);
            result = 0;  // what voodoo2a gives back here
        }
        break;
    case 0xf:
        gl::fogColorValue(app, cpu, value);
        break;
    case 0x12:
        put(app, 0xa91f44, value);
        break;
    case 0x13:  // four of a record at once, or all four cleared
        if (value != 0)
        {
            put(app, 0xa91f30, at(app, value + 4));
            put(app, 0xa91f3c, at(app, value + 8));
            put(app, 0xa91f34, at(app, value + 0x18));
            put(app, 0xa91f38, at(app, value + 0x14));
        }
        else
        {
            put(app, 0xa91f3c, 0);
            put(app, 0xa91f34, 0);
            put(app, 0xa91f38, 0);
            put(app, 0xa91f30, 0);
        }
        break;
    case 0x15:  // fog mode
        if (value == 0)
            gl::fogMode(app, cpu, 0);
        else if (value == 1 || value == 2 || value == 4 || value == 8)
            gl::fogMode(app, cpu, 2);
        else
            result = 0;
        break;
    case 0x18:
    case 0x66:
        gl::depthBiasLevel(app, cpu, x86::sreg16(value));
        break;
    case 0x19:
        put(app, 0xa91f08, value);
        break;
    case 0x1a:
        put(app, 0xa91f34, value);
        break;
    case 0x1b:
        put(app, 0xa91f3c, value);
        break;
    case 0x1c:
        put(app, 0xa91f38, value);
        break;
    case 0x65:
        gl::gammaCorrectionValue(app, cpu, asFloat(value));
        break;
    case 0x67:
        put(app, 0xa91f0c, value);
        break;
    case 0x68:  // blending
        switch (value)
        {
        case 0: gl::alphaBlendFunction(app, cpu, 1, 5, 4, 0); break;
        case 1: gl::alphaBlendFunction(app, cpu, 1, 4, 4, 0); break;
        case 2: gl::alphaBlendFunction(app, cpu, 0, 5, 4, 0); break;
        case 3: gl::alphaBlendFunction(app, cpu, 2, 0, 4, 0); break;
        default: result = 0; break;
        }
        break;
    case 0x69:  // fog from the game's own table, or off
        if (value == 0)
        {
            gl::fogMode(app, cpu, 0);
        }
        else
        {
            gl::fogMode(app, cpu, 2);
            gl::fogTable(app, cpu, &app->getMemory<x86::reg8>(value));
        }
        break;
    case 0x6a:
        gl::depthMask(app, cpu, value);
        break;
    case 0x6b:
        put(app, 0xa91f10, value);
        break;
    case 0x6c:
        if (at(app, 0xa91ef8) != 0)
            result = 0;
        else
            gl::sstControl(app, cpu, value != 0 ? 2 : 1);
        break;
    default:
        result = 0;
        break;
    }
    return result;
}

bool setStateNative(win32::WinApplication* app, x86::CPU& cpu)
{
    const x86::reg32 esp = cpu.esp;
    cpu.eax = applyState(app, cpu, at(app, esp + 4), at(app, esp + 8), esp + 8);
    cpu.esp = esp + 4 + 8;
    return true;
}

/* Set while a check runs the generated function: its stand-in steps aside. */
thread_local bool t_original = false;

/* Whether t_original can be set at all: only while a check, a trace of the
 * setting up or a generated function named in NFS_THRASH_OFF runs the
 * generated code.  Otherwise the thread-local is never read -- on Android
 * each read is a call into the runtime (__emutls_get_address). */
bool originalMayRun();

inline bool originalRunning()
{
    return originalMayRun() && (t_original || nativeOriginalRunning());
}

bool thrashOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_THRASH");
        return !value || SDL_strcmp(value, "0") != 0;
    }();
    return on && nativesEnabled();
}

/* NFS_NATIVE_CHECK=1 checks every native stand-in; =thrash only these, without
 * native_vertices.cpp's whole-memory checks, which put all of guest memory back
 * while other guest threads may be writing it. */
bool thrashChecking()
{
    static const bool check = []() {
        const char* value = SDL_getenv("NFS_NATIVE_CHECK");
        return value && SDL_strcmp(value, "thrash") == 0;
    }();
    return check || nativeChecking();
}

struct Counts
{
    unsigned calls = 0;
    unsigned vertices = 0;
    unsigned mismatches = 0;
};

/* What Glide is handed, field by field, to the bit; z and the other TMUs'
 * coordinates are never set. */
bool sameVertex(const win32::GrVertex& a, const win32::GrVertex& b)
{
    const float pairs[][2] = {
        { a.x, b.x }, { a.y, b.y }, { a.ooz, b.ooz }, { a.oow, b.oow },
        { a.r, b.r }, { a.g, b.g }, { a.b, b.b }, { a.a, b.a },
        { a.tmuvtx[0].sow, b.tmuvtx[0].sow }, { a.tmuvtx[0].tow, b.tmuvtx[0].tow },
    };
    for (const auto& pair : pairs)
        if (std::memcmp(&pair[0], &pair[1], sizeof(float)) != 0)
            return false;
    return true;
}

/* What a THRASH function may write besides the stack below its return:
 * voodoo2a's data and variables, and the arguments it was called with. */
struct Written
{
    std::vector<x86::reg8> bytes;

    /* And `extra`, when there is one: the texture record a call writes. */
    static std::pair<x86::reg32, x86::reg32> extra;

    static void ranges(x86::reg32 esp, x86::reg32 arguments, std::pair<x86::reg32, x86::reg32> (&out)[4])
    {
        out[0] = { 0xa91200, 0x1e00 };  // DGROUP
        out[1] = { 0xa93200, 0x1a00 };  // its uninitialised data
        out[2] = { esp + 4, arguments };
        out[3] = extra;
    }

    void take(win32::WinApplication* app, x86::reg32 esp, x86::reg32 arguments)
    {
        std::pair<x86::reg32, x86::reg32> list[4];
        ranges(esp, arguments, list);
        bytes.clear();
        for (const auto& range : list)
        {
            const x86::reg8* from = &app->getMemory<x86::reg8>(range.first);
            bytes.insert(bytes.end(), from, from + range.second);
        }
    }

    void restore(win32::WinApplication* app, x86::reg32 esp, x86::reg32 arguments) const
    {
        std::pair<x86::reg32, x86::reg32> list[4];
        ranges(esp, arguments, list);
        std::size_t offset = 0;
        for (const auto& range : list)
        {
            std::memcpy(&app->getMemory<x86::reg8>(range.first), &bytes[offset], range.second);
            offset += range.second;
        }
    }

    /* The guest address of the first byte that differs, or 0. */
    x86::reg32 firstDifference(const Written& other, x86::reg32 esp, x86::reg32 arguments) const
    {
        std::pair<x86::reg32, x86::reg32> list[4];
        ranges(esp, arguments, list);
        std::size_t offset = 0;
        for (const auto& range : list)
        {
            for (x86::reg32 i = 0; i < range.second; ++i)
                if (bytes[offset + i] != other.bytes[offset + i])
                    return range.first + i;
            offset += range.second;
        }
        return 0;
    }
};

std::pair<x86::reg32, x86::reg32> Written::extra;

/* NFS_NATIVE_CHECK: the call runs natively, its writes are put back, the
 * generated function runs, and what each drew, every Glide state call each
 * made, what each wrote and the state the caller sees must agree.  Everything
 * is drawn and set twice.  False where the native declined. */
bool check(win32::WinApplication* app, x86::CPU& cpu, const char* name, x86::reg32 address, x86::reg32 arguments,
           bool result, Counts& counts, Native native)
{
    static std::vector<win32::GrVertex> mine, original;
    static std::vector<x86::reg32> mineCalls, originalCalls;
    static Written before, mineWritten, originalWritten;
    mine.clear();
    original.clear();
    mineCalls.clear();
    originalCalls.clear();
    const x86::CPU entry = cpu;
    before.take(app, entry.esp, arguments);

    win32::glide2x::traceTriangles(&mine);
    win32::glide2x::traceCalls(&mineCalls);
    const bool done = native(app, cpu);
    win32::glide2x::traceTriangles(nullptr);
    win32::glide2x::traceCalls(nullptr);
    if (!done)
        return false;
    const x86::CPU after = cpu;
    mineWritten.take(app, entry.esp, arguments);
    before.restore(app, entry.esp, arguments);

    cpu = entry;
    win32::glide2x::traceTriangles(&original);
    win32::glide2x::traceCalls(&originalCalls);
    t_original = true;
    app->dynamic_call(address, cpu);
    t_original = false;
    win32::glide2x::traceTriangles(nullptr);
    win32::glide2x::traceCalls(nullptr);
    originalWritten.take(app, entry.esp, arguments);

    char what[200] = "";
    if (mine.size() != original.size())
        SDL_snprintf(what, sizeof what, "%u vertices native, %u original", unsigned(mine.size()),
                     unsigned(original.size()));
    for (std::size_t i = 0; i < mine.size() && !what[0]; ++i)
        if (!sameVertex(mine[i], original[i]))
            SDL_snprintf(what, sizeof what, "vertex %u: x %.9g/%.9g y %.9g/%.9g", unsigned(i), double(mine[i].x),
                         double(original[i].x), double(mine[i].y), double(original[i].y));
    if (!what[0] && mineCalls != originalCalls)
    {
        std::size_t i = 0;
        while (i < mineCalls.size() && i < originalCalls.size() && mineCalls[i] == originalCalls[i])
            ++i;
        SDL_snprintf(what, sizeof what, "Glide calls (%u/%u words) part from word %u: %08x/%08x",
                     unsigned(mineCalls.size()), unsigned(originalCalls.size()), unsigned(i),
                     unsigned(i < mineCalls.size() ? mineCalls[i] : 0),
                     unsigned(i < originalCalls.size() ? originalCalls[i] : 0));
    }
    if (!what[0])
        if (const x86::reg32 where = mineWritten.firstDifference(originalWritten, entry.esp, arguments))
            SDL_snprintf(what, sizeof what, "memory from %08x", unsigned(where));
    if (!what[0] && (after.esp != cpu.esp || after.ebx != cpu.ebx || after.esi != cpu.esi
                     || after.edi != cpu.edi || after.ebp != cpu.ebp || after.fpu.count != cpu.fpu.count
                     || (result && after.eax != cpu.eax)))
        SDL_snprintf(what, sizeof what,
                     "eax %08x/%08x esp %08x/%08x ebx %08x/%08x esi %08x/%08x edi %08x/%08x ebp %08x/%08x",
                     unsigned(after.eax), unsigned(cpu.eax), unsigned(after.esp), unsigned(cpu.esp),
                     unsigned(after.ebx), unsigned(cpu.ebx), unsigned(after.esi), unsigned(cpu.esi),
                     unsigned(after.edi), unsigned(cpu.edi), unsigned(after.ebp), unsigned(cpu.ebp));

    counts.calls++;
    counts.vertices += unsigned(mine.size());
    if (what[0] && ++counts.mismatches <= 20)
        SDL_Log("[NATIVE] %s differs: %s (call %u, arguments %08x %08x)", name, what, counts.calls,
                unsigned(app->getMemory<x86::reg32>(entry.esp + 4)), unsigned(app->getMemory<x86::reg32>(entry.esp + 8)));
    if (counts.calls == 1 || counts.calls % 20000 == 0)
        SDL_Log("[NATIVE] %s checked: %u calls, %u vertices, %u differ", name, counts.calls, counts.vertices,
                counts.mismatches);
    return true;
}

bool originalMayRun()
{
    static const bool may = []() {
        const char* off = SDL_getenv("NFS_THRASH_OFF");
        const char* trace = SDL_getenv("NFS_THRASH_INIT_TRACE");
        return thrashChecking() || nativeChecking() || (off && *off) || (trace && SDL_strcmp(trace, "1") == 0);
    }();
    return may;
}

/* `arguments`: how many bytes of them; `result`: whether eax is one. */
/* NFS_THRASH_OFF=THRASH_setstate,THRASH_drawtrifan: these left to the
 * generated code, to find which stand-in a fault comes from. */
bool switchedOff(const char* name)
{
    static const std::string off = []() {
        const char* value = SDL_getenv("NFS_THRASH_OFF");
        return std::string(value ? value : "");
    }();
    if (off.empty())
        return false;
    const std::size_t at = off.find(name);
    const std::size_t end = at + SDL_strlen(name);
    return at != std::string::npos && (at == 0 || off[at - 1] == ',') && (end == off.size() || off[end] == ',');
}

bool run(win32::WinApplication* app, x86::CPU& cpu, const char* name, x86::reg32 address, x86::reg32 arguments,
         bool result, Counts& counts, Native native, bool plain, std::pair<x86::reg32, x86::reg32> record = { 0, 0 })
{
    if (originalRunning() || !plain || !thrashOn() || switchedOff(name))
        return false;
    // A check inside another one's trace runs as it would without the check.
    if (thrashChecking() && !win32::glide2x::tracingTriangles())
    {
        Written::extra = record;
        return check(app, cpu, name, address, arguments, result, counts, native);
    }
    return native(app, cpu);
}

/* THRASH_talloc and _treset take memory from voodoo2a's malloc and give it
 * back, so the two paths cannot both run: under NFS_NATIVE_CHECK the generated
 * code runs, and what it did is held against what the native one plans. */
void report(const char* name, Counts& counts, const char* what)
{
    counts.calls++;
    if (what[0] && ++counts.mismatches <= 20)
        SDL_Log("[NATIVE] %s differs: %s (call %u)", name, what, counts.calls);
    if (counts.calls == 1 || counts.calls % 2000 == 0)
        SDL_Log("[NATIVE] %s checked: %u calls, %u differ", name, counts.calls, counts.mismatches);
}

bool checkTextureAllocate(win32::WinApplication* app, x86::CPU& cpu, Counts& counts)
{
    static std::vector<x86::reg32> mineCalls, originalCalls;
    mineCalls.clear();
    originalCalls.clear();
    const x86::reg32 esp = cpu.esp;
    win32::glide2x::traceCalls(&mineCalls);
    const TexturePlan plan = planTexture(app, cpu, esp, false);
    win32::glide2x::traceCalls(nullptr);
    const x86::reg32 formatIndex = at(app, esp + 0xc);
    win32::glide2x::traceCalls(&originalCalls);
    t_original = true;
    app->dynamic_call(0xa844b0, cpu);
    t_original = false;
    win32::glide2x::traceCalls(nullptr);
    const x86::reg32 record = cpu.eax;
    char what[160] = "";
    if (mineCalls != originalCalls)
        SDL_snprintf(what, sizeof what, "Glide calls (%u/%u words)", unsigned(mineCalls.size()),
                     unsigned(originalCalls.size()));
    else if (cpu.esp != esp + 4 + 0x14)
        SDL_snprintf(what, sizeof what, "esp %08x", unsigned(cpu.esp));
    else if (!plan.fits && (record != 0 || at(app, kTextureNext) != plan.start))
        SDL_snprintf(what, sizeof what, "no room planned, record %08x next %08x/%08x", unsigned(record),
                     unsigned(at(app, kTextureNext)), unsigned(plan.start));
    else if (plan.fits && record != 0
             && (at(app, record) != plan.smallLod || at(app, record + 4) != plan.largeLod
                 || at(app, record + 8) != plan.aspect || at(app, record + 0xc) != byteAt(app, 0xa92348 + formatIndex)
                 || at(app, record + 0x10) != 0 || at(app, record + 0x14) != plan.start || at(app, record + 0x18) != 3
                 || at(app, kTextures) != record || at(app, kTextureNext) != plan.start + plan.size))
        SDL_snprintf(what, sizeof what, "record %08x: lods %u-%u/%u-%u start %08x/%08x size %08x",
                     unsigned(record), unsigned(at(app, record)), unsigned(at(app, record + 4)),
                     unsigned(plan.smallLod), unsigned(plan.largeLod), unsigned(at(app, record + 0x14)),
                     unsigned(plan.start), unsigned(plan.size));
    report("THRASH_talloc", counts, what);
    return true;
}

bool checkTextureReset(win32::WinApplication* app, x86::CPU& cpu, Counts& counts)
{
    const x86::reg32 esp = cpu.esp;
    t_original = true;
    app->dynamic_call(0xa84690, cpu);
    t_original = false;
    char what[160] = "";
    if (cpu.eax != 1 || cpu.esp != esp + 4 || at(app, kTextures) != 0
        || at(app, kTextureNext) != win32::glide2x::direct::texMinAddress(app, cpu, 0))
        SDL_snprintf(what, sizeof what, "eax %08x records %08x next %08x", unsigned(cpu.eax),
                     unsigned(at(app, kTextures)), unsigned(at(app, kTextureNext)));
    report("THRASH_treset", counts, what);
    return true;
}

/* THRASH_pageflip, _lockwindow, _unlockwindow and _readrect swap, lock or read
 * the frame and take voodoo2a's malloc, so under NFS_NATIVE_CHECK they run
 * once, generated, and the Glide calls they made are held against the ones the
 * native code makes. */
constexpr x86::reg32 kEnd = 0xffffffff;

void runTraced(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 address, std::vector<x86::reg32>& calls)
{
    calls.clear();
    win32::glide2x::traceCalls(&calls);
    t_original = true;
    app->dynamic_call(address, cpu);
    t_original = false;
    win32::glide2x::traceCalls(nullptr);
}

void describeCalls(const std::vector<x86::reg32>& expected, const std::vector<x86::reg32>& calls, char* what,
                   std::size_t size)
{
    if (calls == expected)
        return;
    std::size_t i = 0;
    while (i < calls.size() && i < expected.size() && calls[i] == expected[i])
        ++i;
    SDL_snprintf(what, size, "Glide calls (%u/%u words) part from word %u: %08x/%08x", unsigned(expected.size()),
                 unsigned(calls.size()), unsigned(i), unsigned(i < expected.size() ? expected[i] : 0),
                 unsigned(i < calls.size() ? calls[i] : 0));
}

bool checkPageFlip(win32::WinApplication* app, x86::CPU& cpu, Counts& counts)
{
    static std::vector<x86::reg32> calls;
    const x86::reg32 esp = cpu.esp;
    const std::vector<x86::reg32> expected = { 0xf8, kEnd, 0x48, at(app, kSwapInterval), kEnd };
    runTraced(app, cpu, 0xa847e0, calls);
    char what[160] = "";
    describeCalls(expected, calls, what, sizeof what);
    if (!what[0] && cpu.esp != esp + 4)
        SDL_snprintf(what, sizeof what, "esp %08x", unsigned(cpu.esp));
    report("THRASH_pageflip", counts, what);
    return true;
}

bool checkLockWindow(win32::WinApplication* app, x86::CPU& cpu, Counts& counts)
{
    static std::vector<x86::reg32> calls;
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 buffer = at(app, kBuffer);
    const std::vector<x86::reg32> expected = { 0x98, 1, buffer, 0xff, 0, 0, kEnd, 0x98, 0, buffer, 0xff, 0, 0, kEnd };
    runTraced(app, cpu, 0xa850f0, calls);
    const x86::reg32 record = cpu.eax;
    char what[160] = "";
    describeCalls(expected, calls, what, sizeof what);
    if (!what[0] && cpu.esp != esp + 4)
        SDL_snprintf(what, sizeof what, "esp %08x", unsigned(cpu.esp));
    if (!what[0] && record != 0
        && (at(app, record + 8) != 4 || at(app, record + 0xc) != at(app, 0xa91f18)
            || at(app, record + 0x10) != at(app, 0xa91f1c) || at(app, record + 0x14) != buffer))
        SDL_snprintf(what, sizeof what, "record %08x", unsigned(record));
    report("THRASH_lockwindow", counts, what);
    return true;
}

bool checkUnlockWindow(win32::WinApplication* app, x86::CPU& cpu, Counts& counts)
{
    static std::vector<x86::reg32> calls;
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 record = at(app, esp + 4);
    std::vector<x86::reg32> expected;
    if (record != 0)
    {
        const x86::reg32 buffer = at(app, record + 0x14);
        expected = { 0x9c, 1, buffer, kEnd, 0x9c, 0, buffer, kEnd };
    }
    runTraced(app, cpu, 0xa851d0, calls);
    char what[160] = "";
    describeCalls(expected, calls, what, sizeof what);
    if (!what[0] && (cpu.esp != esp + 8 || cpu.eax != 1))
        SDL_snprintf(what, sizeof what, "esp %08x eax %08x", unsigned(cpu.esp), unsigned(cpu.eax));
    report("THRASH_unlockwindow", counts, what);
    return true;
}

bool checkReadRect(win32::WinApplication* app, x86::CPU& cpu, Counts& counts)
{
    static std::vector<x86::reg32> calls;
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 width = at(app, esp + 0xc);
    const std::vector<x86::reg32> expected = { 0xa4, at(app, kBuffer), at(app, esp + 4), at(app, esp + 8), width,
                                               at(app, esp + 0x10), width * 2, kEnd };
    runTraced(app, cpu, 0xa85220, calls);
    char what[160] = "";
    describeCalls(expected, calls, what, sizeof what);
    if (!what[0] && cpu.esp != esp + 0x18)
        SDL_snprintf(what, sizeof what, "esp %08x", unsigned(cpu.esp));
    report("THRASH_readrect", counts, what);
    return true;
}

/* THRASH_about, _init, _setvideomode and the rest of the setting up: run
 * once, natively, or generated when NFS_THRASH_OFF names them.  With
 * NFS_THRASH_INIT_TRACE=1 each logs a hash of voodoo2a's data after it, so a
 * run of the generated code and a native one can be set side by side; the
 * windows, events and DirectDraw objects they make cannot run twice. */
bool initTraceOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_THRASH_INIT_TRACE");
        return value && SDL_strcmp(value, "1") == 0;
    }();
    return on;
}

bool runInit(win32::WinApplication* app, x86::CPU& cpu, const char* name, x86::reg32 address, Native native)
{
    if (originalRunning() || !thrashOn())
        return false;
    const bool generated = switchedOff(name);
    if (!initTraceOn())
        return generated ? false : native(app, cpu);
    const x86::reg32 esp = cpu.esp;
    const x86::reg32 arguments[3] = { at(app, esp + 4), at(app, esp + 8), at(app, esp + 0xc) };
    if (generated)
    {
        t_original = true;
        app->dynamic_call(address, cpu);
        t_original = false;
    }
    else
    {
        native(app, cpu);
    }
    Uint64 hash = 1469598103934665603ull;
    for (const auto& range : { std::pair<x86::reg32, x86::reg32>{ 0xa91200, 0x1e00 }, { 0xa93200, 0x1a00 } })
    {
        const x86::reg8* bytes = &app->getMemory<x86::reg8>(range.first);
        for (x86::reg32 i = 0; i < range.second; ++i)
            hash = (hash ^ bytes[i]) * 1099511628211ull;
    }
    SDL_Log("[THRASHINIT] %s %s (%08x %08x %08x) -> eax %08x esp %+d data %016llx", name,
            generated ? "generated" : "native", unsigned(arguments[0]), unsigned(arguments[1]),
            unsigned(arguments[2]), unsigned(cpu.eax), int(cpu.esp - esp), (unsigned long long)hash);
    return true;
}

bool runOnce(win32::WinApplication* app, x86::CPU& cpu, const char* name, Counts& counts, Native native,
                bool (*checkIt)(win32::WinApplication*, x86::CPU&, Counts&))
{
    if (originalRunning() || !thrashOn() || switchedOff(name))
        return false;
    if (thrashChecking() && !win32::glide2x::tracingTriangles())
        return checkIt(app, cpu, counts);
    return native(app, cpu);
}

}

/* The GrVertex THRASH_drawtri and the rest make of one of the game's vertices.
 * voodoo2a's generated code does its x87 arithmetic in plain double, without
 * the precision control's rounding (disasm/dll.py), so this does too, in the
 * same order: each product of floats is exact, and only the one of three
 * factors and the stores to float round.  The colours are the table's floats
 * for the vertex's bytes (VertexColours). */
struct VertexFloats
{
    float x, y, ooz, sow, tow, oow;
};

inline VertexFloats vertexFloats(win32::WinApplication* app, x86::reg32 in, float zScale, float wScale)
{
    const double zk = double(app->getMemory<float>(in + 8)) * double(zScale);
    const double wk = double(app->getMemory<float>(in + 0xc)) * double(wScale);
    VertexFloats out;
    out.x = float(double(app->getMemory<float>(in)));
    out.y = float(double(app->getMemory<float>(in + 4)));
    out.oow = app->getMemory<float>(in + 0xc);
    out.sow = float(double(app->getMemory<float>(in + 0x18)) * wk);
    out.tow = float(wk * double(app->getMemory<float>(in + 0x1c)));
    out.ooz = float(zk);
    return out;
}

inline float colourAt(win32::WinApplication* app, x86::reg32 in, x86::reg32 offset)
{
    return app->getMemory<float>(kColourTable + x86::reg32(app->getMemory<x86::reg8>(in + offset)) * 4);
}

void thrashVertex(win32::WinApplication* app, x86::reg32 in, float zScale, float wScale, win32::GrVertex& out)
{
    const VertexFloats v = vertexFloats(app, in, zScale, wScale);
    out.x = v.x;
    out.y = v.y;
    const x86::reg32 w = app->getMemory<x86::reg32>(in + 0xc);
    std::memcpy(&out.oow, &w, sizeof w);
    out.a = colourAt(app, in, 0x13);
    out.r = colourAt(app, in, 0x12);
    out.g = colourAt(app, in, 0x11);
    out.b = colourAt(app, in, 0x10);
    out.tmuvtx[0].sow = v.sow;
    out.tmuvtx[0].tow = v.tow;
    out.ooz = v.ooz;
}

/* The same vertex straight into a slot of ThrashRenderer's batch: u and v
 * before finishTriangle brings far ones back. */
void thrashBatchVertex(win32::WinApplication* app, x86::reg32 in, float zScale, float wScale, win32::ThrashVertex& out)
{
    const VertexFloats v = vertexFloats(app, in, zScale, wScale);
    out.x = v.x;
    out.y = v.y;
    out.z = v.ooz;
    const x86::reg32 w = app->getMemory<x86::reg32>(in + 0xc);
    std::memcpy(&out.oow, &w, sizeof w);
    out.u = v.sow;
    out.v = v.tow;
    out.color = x86::reg32(x86::reg8(colourAt(app, in, 0x12))) | x86::reg32(x86::reg8(colourAt(app, in, 0x11))) << 8
              | x86::reg32(x86::reg8(colourAt(app, in, 0x10))) << 16
              | x86::reg32(x86::reg8(colourAt(app, in, 0x13))) << 24;
}

/* The same vertex as ThrashRenderer's corner: what its drawTriangle keeps of
 * the GrVertex above, the colour floats turned to bytes the way it turns them. */
win32::ThrashRenderer::Corner thrashCorner(win32::WinApplication* app, x86::reg32 in, float zScale, float wScale)
{
    const VertexFloats v = vertexFloats(app, in, zScale, wScale);
    win32::ThrashRenderer::Corner out;
    out.x = v.x;
    out.y = v.y;
    out.ooz = v.ooz;
    const x86::reg32 w = app->getMemory<x86::reg32>(in + 0xc);
    std::memcpy(&out.oow, &w, sizeof w);
    out.sow = v.sow;
    out.tow = v.tow;
    out.color = x86::reg32(x86::reg8(colourAt(app, in, 0x12))) | x86::reg32(x86::reg8(colourAt(app, in, 0x11))) << 8
              | x86::reg32(x86::reg8(colourAt(app, in, 0x10))) << 16
              | x86::reg32(x86::reg8(colourAt(app, in, 0x13))) << 24;
    return out;
}

void thrashTriangle(win32::WinApplication* app, x86::reg32 a, x86::reg32 b, x86::reg32 c)
{
    Vertices(app).triangle(a, b, c);
}

bool thrashPlainTriangles(win32::WinApplication* app)
{
    return drawsThrough(app, kTriangleDraw, kGrDrawTriangle);
}

bool thrashDrawQuad(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_drawquad", 0xa85300, 0x10, false, counts, drawQuadNative,
               drawsThrough(app, kQuadDraw, kGrDrawTriangle));
}

bool thrashDrawQuadMesh(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_drawquadmesh", 0xa85520, 0xc, false, counts, drawQuadMeshNative,
               drawsThrough(app, kQuadDraw, kGrDrawTriangle));
}

bool thrashDrawTriMesh(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_drawtrimesh", 0xa85900, 0xc, false, counts, drawTriMeshNative, thrashPlainTriangles(app));
}

bool thrashDrawTriFan(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_drawtrifan", 0xa86000, 8, false, counts, drawTriFanNative, thrashPlainTriangles(app));
}

bool thrashDrawLine(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_drawline", 0xa86240, 8, false, counts, drawLineNative,
               drawsThrough(app, kLineDraw, kGrDrawLine));
}

bool thrashDrawLineMesh(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_drawlinemesh", 0xa86350, 0xc, false, counts, drawLineMeshNative,
               drawsThrough(app, kLineDraw, kGrDrawLine));
}

bool thrashSetState(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_setstate", 0xa84990, 8, true, counts, setStateNative, true);
}

bool thrashTextureAllocate(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return runOnce(app, cpu, "THRASH_talloc", counts, textureAllocateNative, checkTextureAllocate);
}

bool thrashTextureReset(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return runOnce(app, cpu, "THRASH_treset", counts, textureResetNative, checkTextureReset);
}

bool thrashTextureUpdate(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_tupdate", 0xa84630, 0xc, true, counts, textureUpdateNative, true,
               { at(app, cpu.esp + 4), 0x20 });
}

bool thrashSetTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_settexture", 0xa846d0, 4, true, counts, setTextureNative, true);
}

bool thrashWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_window", 0xa84780, 4, true, counts, windowNative, true);
}

bool thrashClearWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_clearwindow", 0xa847b0, 0, false, counts, clearWindowNative, true);
}

bool thrashFlushWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_flushwindow", 0xa847d0, 0, false, counts, nothingNative, true);
}

bool thrashIdle(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_idle", 0xa84810, 0, false, counts, nothingNative, true);
}

bool thrashSync(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_sync", 0xa84830, 4, true, counts, syncNative, true);
}

bool thrashClip(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return run(app, cpu, "THRASH_clip", 0xa848b0, 0x10, true, counts, clipNative, true);
}

bool thrashPageFlip(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return runOnce(app, cpu, "THRASH_pageflip", counts, pageFlipNative, checkPageFlip);
}

bool thrashLockWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return runOnce(app, cpu, "THRASH_lockwindow", counts, lockWindowNative, checkLockWindow);
}

bool thrashUnlockWindow(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return runOnce(app, cpu, "THRASH_unlockwindow", counts, unlockWindowNative, checkUnlockWindow);
}

bool thrashReadRect(win32::WinApplication* app, x86::CPU& cpu)
{
    static Counts counts;
    return runOnce(app, cpu, "THRASH_readrect", counts, readRectNative, checkReadRect);
}

/* Whatever runs the driver -- these stand-ins, or its generated code when they
 * step aside -- reaches the renderer through the same functions of glide2x.cpp,
 * so either renderer serves both. */
bool thrashRendererWanted()
{
    const char* gl = SDL_getenv("NFS_THRASH_GL");
    return !(gl && SDL_strcmp(gl, "0") == 0);
}

bool thrashAbout(win32::WinApplication* app, x86::CPU& cpu)
{
    return runInit(app, cpu, "THRASH_about", 0xa83c60, aboutNative);
}

bool thrashInit(win32::WinApplication* app, x86::CPU& cpu)
{
    return runInit(app, cpu, "THRASH_init", 0xa83f50, initNative);
}

bool thrashSelectDisplay(win32::WinApplication* app, x86::CPU& cpu)
{
    return runInit(app, cpu, "THRASH_selectdisplay", 0xa83f10, selectDisplayNative);
}

bool thrashSetVideoMode(win32::WinApplication* app, x86::CPU& cpu)
{
    return runInit(app, cpu, "THRASH_setvideomode", 0xa843d0, setVideoModeNative);
}

bool thrashOpenMode(win32::WinApplication* app, x86::CPU& cpu)
{
    return runInit(app, cpu, "THRASH_openmode", 0xa84180, openModeNative);
}

bool thrashWindowThread(win32::WinApplication* app, x86::CPU& cpu)
{
    return runInit(app, cpu, "THRASH_windowthread", 0xa84360, windowThreadNative);
}

bool thrashRestore(win32::WinApplication* app, x86::CPU& cpu)
{
    return runInit(app, cpu, "THRASH_restore", 0xa84100, restoreNative);
}

}
