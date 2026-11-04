#include "native_guest.h"
#include "port_thrash.h"

#include <cstdint>
#include <utility>

/* The track: its geometry read from the .frd file into an arena of its own,
 * its colours tinted, its blocks and objects drawn in each view with the
 * headlights' light on them, its ambient sounds, the files that go with it
 * (the .col file, the textures' .qfs) and the views' bookkeeping.
 * disassemble_nfs3hp.py binds each address to the function of that name here
 * (TRACK_NATIVES), and the generated code calls it by name.
 * tools/game_checks holds each against its generated code.
 *
 * The track's blocks: 0x5c0 bytes each from 0x571370, [0x5dd83c] the last
 * one's number -- +0x58 and +0x74 its vertices' count and colours, +0x5c its
 * polygon groups, +0x70 its vertices, +0x78 its centre, +0x84 its box of four
 * corners, +0xb4 the 300 neighbours drawn with it (a word each, -1 none),
 * +0x564/+0x56c/+0x574/+0x57c its four lists of objects (a count, then 0x34
 * bytes each), +0x5b0 its sound sources, +0x5b8 its lights.  An object of a
 * list: its kind at +0 (4 animated, 3 a car's, else fixed), its vertices'
 * count at +0x20, its vertices +0x24, colours +0x28, polygons' count +0x2c
 * and polygons +0x30 (14 bytes each).  The list of the whole track's objects
 * is at 0x5dd070.
 *
 * A view: 0x54 bytes each from 0x5dd0e0, four of them, [0x5dd830] the one
 * being drawn -- +0 the block it is in, +0x30 the distance the world ends at,
 * +0x34 the distance blocks are drawn to, +0x40/+0x44 the two distances the
 * track's model gets coarser at; the camera (eax of the drawing) its kind at
 * +0 (1 the rear-view mirror's pass), its place at +8, its matrix at +0x44,
 * its view box at +0x38.
 *
 * The .frd is read by moving its bytes from where the file was loaded
 * ([0x5dd078], moved on as it goes) into blocks taken from the arena: 0x310000
 * bytes at [0x552dfc], [0x552e00] of them used.
 *
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frames' locals in the guest's memory,
 * the registers as the code leaves them, the x87's arithmetic through
 * x86::FPU as the code's (its stack kept in locals where the code's is, as
 * the generated code keeps it), the safepoint at each loop's head the
 * generated code has.  Every other function of the game's, and Windows'
 * through its imports, is called by address.
 *
 * The port's changes these functions carry: the alpha intensity's scaling on
 * this driver too (tools/apply_alpha_intensity.py, projectedTriangle) and the
 * rear-view mirror drawn as the main view (tools/apply_mirror_detail.py,
 * modelByDistance, headlightsOnObjects, drawTrack), through
 * nfs3hp::mirrorAsMain as the generated code had them. */
namespace nfs3hp
{

x86::reg32 mirrorAsMain(x86::reg32 kind);  // nfs3hp_main.cpp (tools/apply_mirror_detail.py)

namespace track
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;
using Float = x86::Float;
using i32 = std::int32_t;

// The track's blocks, the views, the objects' list, the view being drawn.
constexpr reg32 kBlocks = 0x571370, kBlockSize = 0x5c0, kLastBlock = 0x5dd83c;
constexpr reg32 kViews = 0x5dd0e0, kView = 0x5dd830, kTrackObjects = 0x5dd070;
constexpr reg32 kViewSerial = 0x5dd0d8, kViewSerialHigh = 0x5dd0dc;
// The .frd's reading: where the file is read from, the arena and its use.
constexpr reg32 kFrdAt = 0x5dd078, kArena = 0x552dfc, kArenaUsed = 0x552e00, kArenaSize = 0x310000;
// The lights and sound sources of the .frd, their count; the kinds' records.
constexpr reg32 kLights = 0x552df4, kLightCount = 0x552df8, kLightKinds = 0x8b41a0, kTrackKinds = 0x5dd858;
// The colours' tint (percent of red, green, blue) and the driver's bits
// (1 << the driver's type: 2 a grey track, 0x40 Direct3D).
constexpr reg32 kTint = 0x55d148, kDriverBits = 0x7a3a58;
// The shading's distances: near, far, the light's reach, 1 / (far - near).
constexpr reg32 kShadeNear = 0x552e04, kShadeFar = 0x552e08, kShadeReach = 0x552e0c, kShadeScale = 0x552e10;
// Alpha intensity (0..1) and the headlights' polygon's texture.
constexpr reg32 kAlphaIntensity = 0x6fbc38, kHeadlightTexture = 0x8b4814;
// The race: split screen, the network's, the track's number; the loading stopped.
constexpr reg32 kSplitScreen = 0x6fd4c8, kNetworkRace = 0x6fd4cc, kTrackNumber = 0x6fd4ac;
constexpr reg32 kLoadStopped = 0x55e88c;
// The cars (the headlights): their count and pointers.
constexpr reg32 kCarCount = 0x5efd9c, kCars = 0x5efac8;
// The objects' animation states, 3 bytes each.
constexpr reg32 kObjectStates = 0x5dd230;
// The models by distance: 3 records of 12 bytes.
constexpr reg32 kModels = 0x552e20;
// View Distance (0 Full .. 3 Close).
constexpr reg32 kViewDistance = 0x6fbc28;
// The .col file, its records, the track's textures and their handle.
constexpr reg32 kColFile = 0x552e18, kColVertices = 0x552e14, kColObjects = 0x5dd844, kColLights = 0x5dd840;
constexpr reg32 kTexturesFile = 0x5dd850, kTexturesCount = 0x5dd854, kTexturesLeft = 0x5dd85c, kTexturesAll = 0x5dd860;
constexpr reg32 kObjectPool = 0x5dd848;
// The ambient sound played last, and the pass's half-second counter.
constexpr reg32 kAmbientLast = 0x552e44, kFrameCounter = 0x5f1fa8;
// The views' buckets of polygons.
constexpr reg32 kBucketOpaque = 0x7dcfe8, kBucketA = 0x7dcff4, kBucketB = 0x7dcff0, kBucketC = 0x7dcfe4;
constexpr reg32 kBucketOrdered = 0x7dcff8, kWorldsDrawn = 0x7a3a60;
// Strings and constants of the game's.
constexpr reg32 kOpen = 0x536c30, kNoName = 0x536c60, kNoName2 = 0x536d10;
constexpr reg32 kHeadlightsBox = 0x41a880, kHeadlightsPoint = 0x41a8a4;  // data among the code
// What is called by address.
constexpr reg32 kGetModuleHandleA = 0x53454c, kShellExecuteA = 0x534480;
constexpr reg32 kAllocate = 0x4e1620, kFree = 0x4e1890, kMemmove = 0x4ea4f0, kFileExists = 0x4e0e60;
constexpr reg32 kTopUp = 0x4a33d0;

void push(App* app, CPU& cpu, reg32 value)
{
    cpu.esp -= 4;
    put(app, cpu.esp, value);
}

reg32 pop(App* app, CPU& cpu)
{
    const reg32 value = at(app, cpu.esp);
    cpu.esp += 4;
    return value;
}

/* A call as the code makes it: the return address's slot, and esp as the
 * callee leaves it.  False when the game stopped in it. */
bool direct(App* app, CPU& cpu, void (*function)(App*, CPU&))
{
    cpu.esp -= 4;
    function(app, cpu);
    return !cpu.terminate;
}

bool byAddress(App* app, CPU& cpu, reg32 target)
{
    cpu.esp -= 4;
    app->dynamic_call(target, cpu);
    return !cpu.terminate;
}

/* A loop's safepoint, as the generated code has one at each loop's head. */
void safepoint(App* app, CPU& cpu)
{
    if (app->contextWanted())
        app->yieldContext(cpu);
}

reg16 half(App* app, reg32 address)
{
    return app->getMemory<reg16>(address);
}

void putHalf(App* app, reg32 address, reg16 value)
{
    app->getMemory<reg16>(address) = value;
}

Float f32(App* app, reg32 address)
{
    return Float(app->getMemory<float>(address));
}

Float f64(App* app, reg32 address)
{
    return Float(app->getMemory<double>(address));
}

void putF32(App* app, reg32 address, const Float& value)
{
    app->getMemory<float>(address) = float(value);
}

void putF64(App* app, reg32 address, const Float& value)
{
    app->getMemory<double>(address) = double(value);
}

/* movsd: a dword from [esi] to [edi], both moved on. */
void movsd(App* app, CPU& cpu)
{
    put(app, cpu.edi, at(app, cpu.esi));
    cpu.esi += 4;
    cpu.edi += 4;
}

/* The code's cdq; idiv by: edx:eax divided, as x86 does. */
void idiv(CPU& cpu, reg32 by)
{
    const i32 n = i32(cpu.eax);
    cpu.eax = by == 0xffffffff ? 0u - cpu.eax : reg32(n / i32(by));
    cpu.edx = by == 0xffffffff ? 0 : reg32(n % i32(by));
}

/* The code's signed shift towards 0 -- mov edx, eax; sar edx, 31; shl edx,
 * k; sbb eax, edx; sar eax, k -- of eax; edx as it leaves it. */
void towardZero(CPU& cpu, unsigned k)
{
    const bool negative = i32(cpu.eax) < 0;
    cpu.edx = negative ? 0xffffffffu << k : 0;
    cpu.eax = reg32(i32(cpu.eax - cpu.edx - (negative ? 1 : 0)) >> k);
}

/* fcom(p); fnstsw ax; sahf: st(0) compared with m, the x87's status word in
 * ax; what sahf makes of it -- CF (C0) below, ZF (C3) equal, both for an
 * unordered pair. */
struct Compared
{
    bool below, equal;
    bool belowOrEqual() const { return below || equal; }
    bool above() const { return !below && !equal; }
};

Compared compare(CPU& cpu, const Float& st0, const Float& m)
{
    cpu.fpu.compare(st0, m);
    cpu.ax = cpu.fpu.status.word;
    return {(cpu.ah & 1) != 0, (cpu.ah & 0x40) != 0};
}

/* The code's colour scaled by the 16.16 factor in ecx (0x10000 and up taken
 * as 0xffff): ecx's high byte of it, eax's four channels two at a time -- ebx
 * the odd ones, edx the high half of the second mul. */
void scaleColour(CPU& cpu)
{
    if (sreg32(cpu.ecx) >= 0x10000)
        cpu.ecx = 0xffff;
    cpu.ecx >>= 8;
    cpu.ebx = cpu.eax;
    cpu.ecx &= 0xff;
    cpu.ebx &= 0xff00ff00;
    cpu.ebx >>= 8;
    cpu.eax &= 0xff00ff;
    std::uint64_t product = std::uint64_t(cpu.eax) * cpu.ecx;
    cpu.eax = reg32(product);
    cpu.edx = reg32(product >> 32);
    std::swap(cpu.eax, cpu.ebx);
    product = std::uint64_t(cpu.eax) * cpu.ecx;
    cpu.eax = reg32(product);
    cpu.edx = reg32(product >> 32);
    cpu.ebx >>= 8;
    cpu.eax &= 0xff00ff00;
    cpu.ebx &= 0xff00ff;
    cpu.eax |= cpu.ebx;
}

/* The block of number `index`: kBlocks + index * 0x5c0, as the code works it
 * out (lea, sub, shl 3, sub, shl 6). */
reg32 blockOffset(reg32 index)
{
    return index * kBlockSize;
}

/* The code's vector at eax turned by the 3 by 3 matrix at edx into ebx: each
 * row's products summed in its order; eax left past the vector. */
void rotate(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    const reg32 v = cpu.eax, m = cpu.edx;
    const Float a = fp::mul(f32(app, v), f32(app, m));
    const Float b = fp::mul(f32(app, v), f32(app, m + 4));
    const Float c = fp::mul(f32(app, v), f32(app, m + 8));
    const Float d = fp::mul(f32(app, v + 4), f32(app, m + 0xc));
    const Float e = fp::mul(f32(app, v + 4), f32(app, m + 0x10));
    const Float f = fp::mul(f32(app, v + 4), f32(app, m + 0x14));
    const Float x = fp::add(a, d), y = fp::add(b, e), z = fp::add(c, f);
    const Float g = fp::mul(f32(app, v + 8), f32(app, m + 0x18));
    const Float h = fp::mul(f32(app, v + 8), f32(app, m + 0x1c));
    const Float i = fp::mul(f32(app, v + 8), f32(app, m + 0x20));
    const Float rx = fp::add(x, g), ry = fp::add(y, h), rz = fp::add(z, i);
    cpu.eax = v + 0xc;
    putF32(app, cpu.ebx, rx);
    putF32(app, cpu.ebx + 4, ry);
    putF32(app, cpu.ebx + 8, rz);
}

/* cmp al, ah; ja; mov al, ah: the larger of two bytes. */
reg8 larger(reg8 a, reg8 b)
{
    return a > b ? a : b;
}

}  // namespace

void shellOpen(App* app, CPU& cpu);
void tintColours(App* app, CPU& cpu);
void tintObjectList(App* app, CPU& cpu);
void tintTrack(App* app, CPU& cpu);
void greyColours(App* app, CPU& cpu);
void greyObjectList(App* app, CPU& cpu);
void greyTrack(App* app, CPU& cpu);
void readObjectList(App* app, CPU& cpu);
void readLights(App* app, CPU& cpu);
void newArena(App* app, CPU& cpu);
void freeArena(App* app, CPU& cpu);
void arenaTake(App* app, CPU& cpu);
void frdRead(App* app, CPU& cpu);
void loadGeometry(App* app, CPU& cpu);
void projectedTriangle(App* app, CPU& cpu);
void shadeVertex(App* app, CPU& cpu);
void setShadeDistances(App* app, CPU& cpu);
void viewObjectVertices(App* app, CPU& cpu);
void viewTurnedObjectVertices(App* app, CPU& cpu);
void modelByDistance(App* app, CPU& cpu);
void viewOnBlock(App* app, CPU& cpu);
void lightObjectPolygons(App* app, CPU& cpu);
void bracketList(App* app, CPU& cpu);
void ambientSounds(App* app, CPU& cpu);
void ambientWanted(App* app, CPU& cpu);
void pointBehindView(App* app, CPU& cpu);
void litBoxOutside(App* app, CPU& cpu);
void flushBuckets(App* app, CPU& cpu);
void headlightsOnObjects(App* app, CPU& cpu);
void headlightsOnBlock(App* app, CPU& cpu);
void blockLights(App* app, CPU& cpu);
void drawObjectList(App* app, CPU& cpu);
void drawTrack(App* app, CPU& cpu);
void ambientSource(App* app, CPU& cpu);
void selectView(App* app, CPU& cpu);
void objectAnimation(App* app, CPU& cpu);
void turnFromAngles(App* app, CPU& cpu);
void turnFromPitchYaw(App* app, CPU& cpu);
void setViewDistance(App* app, CPU& cpu);
void nextViewDistance(App* app, CPU& cpu);
void modelsUnlimited(App* app, CPU& cpu);
void freeAt(App* app, CPU& cpu);
void unloadTrack(App* app, CPU& cpu);
void newPool(App* app, CPU& cpu);
void newObjectPool(App* app, CPU& cpu);
void freeTrackKinds(App* app, CPU& cpu);
void loadTextures(App* app, CPU& cpu);
void takeTextures(App* app, CPU& cpu);
void loadCollisions(App* app, CPU& cpu);
void freeCollisions(App* app, CPU& cpu);
void clearObjectStates(App* app, CPU& cpu);
void resetTrackViews(App* app, CPU& cpu);
void resetViewsOuter(App* app, CPU& cpu);
void resetViews(App* app, CPU& cpu);
void blockInView(App* app, CPU& cpu);

/* 0x418cc0: the file or address at eax opened (ShellExecuteA's "open", shown
 * normally).  1. */
void shellOpen(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    push(app, cpu, 1);
    push(app, cpu, 0);
    push(app, cpu, 0);
    push(app, cpu, cpu.eax);
    push(app, cpu, kOpen);
    push(app, cpu, 0);
    if (!byAddress(app, cpu, at(app, kGetModuleHandleA)))
        return;
    push(app, cpu, cpu.eax);
    if (!byAddress(app, cpu, at(app, kShellExecuteA)))
        return;
    cpu.eax = 1;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419660: the eax colours at edx (ARGB) tinted -- red, green and blue each
 * to its percent of the tint ([0x55d148], [0x55d14c], [0x55d150]), alpha
 * kept. */
void tintColours(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    const reg32 count = cpu.ebp - 8, done = cpu.ebp - 4, alpha = cpu.ebp - 0xc;
    put(app, count, cpu.eax);
    cpu.ecx = cpu.edx;
    cpu.edx = 0;
    put(app, done, cpu.edx);
    for (;;)
    {
        safepoint(app, cpu);
        cpu.eax = at(app, done);
        if (sreg32(cpu.eax) >= sreg32(at(app, count)))
            break;
        cpu.ebx = reg32(sreg32(at(app, cpu.ecx)) >> 0x10);
        cpu.edx = at(app, kTint);
        cpu.ebx &= 0xff;
        cpu.edx = cpu.edx * cpu.ebx;
        cpu.eax = at(app, cpu.ecx) & 0xff000000;
        put(app, alpha, cpu.eax);
        cpu.ebx = 0x64;
        cpu.eax = cpu.edx;
        idiv(cpu, cpu.ebx);
        cpu.edi = reg32(sreg32(at(app, cpu.ecx)) >> 8) & 0xff;
        cpu.edx = at(app, kTint + 4) * cpu.edi;
        cpu.ebx = cpu.eax;
        cpu.edi = 0x64;
        cpu.eax = cpu.edx;
        idiv(cpu, cpu.edi);
        cpu.esi = at(app, cpu.ecx) & 0xff;
        cpu.edx = at(app, kTint + 8) * cpu.esi;
        cpu.edi = cpu.eax;
        cpu.esi = 0x64;
        cpu.eax = cpu.edx;
        idiv(cpu, cpu.esi);
        cpu.esi = cpu.eax;
        cpu.eax = cpu.ebx;
        cpu.edx = at(app, alpha);
        cpu.eax <<= 0x10;
        cpu.eax |= cpu.edx;
        cpu.edx = cpu.edi;
        cpu.ecx += 4;
        cpu.edx <<= 8;
        cpu.ebx = at(app, done);
        cpu.eax |= cpu.edx;
        ++cpu.ebx;
        cpu.eax |= cpu.esi;
        put(app, done, cpu.ebx);
        put(app, cpu.ecx - 4, cpu.eax);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419720: the colours of each object of the list at eax tinted (its count,
 * then the objects, 0x34 bytes each: +0x20 the vertices' count, +0x28 their
 * colours). */
void tintObjectList(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = at(app, cpu.eax + 4);
    cpu.ebx = at(app, cpu.eax);
    for (;;)
    {
        safepoint(app, cpu);
        --cpu.ebx;
        if (cpu.ebx == 0xffffffff)
            break;
        cpu.edx = at(app, cpu.ecx + 0x28);
        cpu.eax = at(app, cpu.ecx + 0x20);
        if (!direct(app, cpu, tintColours))
            return;
        cpu.ecx += 0x34;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419750: the whole track's colours tinted: the track's objects, then each
 * block's three lists of objects and its own vertices. */
void tintTrack(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = kTrackObjects;
    if (!direct(app, cpu, tintObjectList))
        return;
    cpu.ecx = 0;
    for (;;)
    {
        safepoint(app, cpu);
        if (sreg32(cpu.ecx) > sreg32(at(app, kLastBlock)))
            break;
        cpu.eax = blockOffset(cpu.ecx);
        cpu.edx = kBlocks + cpu.eax;
        for (const reg32 list : {0x564u, 0x56cu, 0x574u})
        {
            cpu.eax = cpu.edx + list;
            if (!direct(app, cpu, tintObjectList))
                return;
        }
        cpu.eax = at(app, cpu.edx + 0x74);
        cpu.ebx = at(app, cpu.edx + 0x58);
        cpu.edx = cpu.eax;
        cpu.eax = cpu.ebx;
        ++cpu.ecx;
        if (!direct(app, cpu, tintColours))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4197c0: the eax colours at edx made grey -- the mean of red, green and
 * blue in each, alpha opaque. */
void greyColours(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 green = cpu.ebp - 8, high = cpu.ebp - 4;
    cpu.esi = cpu.eax;
    cpu.ecx = cpu.edx;
    cpu.ebx = 0;
    for (;;)
    {
        safepoint(app, cpu);
        if (sreg32(cpu.ebx) >= sreg32(cpu.esi))
            break;
        cpu.edx = reg32(sreg32(at(app, cpu.ecx)) >> 8) & 0xff;
        cpu.eax = reg32(sreg32(at(app, cpu.ecx)) >> 0x10) & 0xff;
        put(app, green, cpu.edx);
        cpu.edx = at(app, cpu.ecx) & 0xff;
        cpu.edi = at(app, green);
        cpu.eax += cpu.edi;
        cpu.edx += cpu.eax;
        cpu.edi = 3;
        cpu.eax = cpu.edx;
        idiv(cpu, cpu.edi);
        cpu.edx = (cpu.eax << 0x10) | 0xff000000;
        put(app, high, cpu.edx);
        cpu.edx = cpu.eax;
        cpu.edi = at(app, high);
        cpu.edx <<= 8;
        cpu.edx |= cpu.edi;
        cpu.ecx += 4;
        cpu.eax |= cpu.edx;
        ++cpu.ebx;
        put(app, cpu.ecx - 4, cpu.eax);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419840: the colours of each object of the list at eax made grey. */
void greyObjectList(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = at(app, cpu.eax + 4);
    cpu.ebx = at(app, cpu.eax);
    for (;;)
    {
        safepoint(app, cpu);
        --cpu.ebx;
        if (cpu.ebx == 0xffffffff)
            break;
        cpu.edx = at(app, cpu.ecx + 0x28);
        cpu.eax = at(app, cpu.ecx + 0x20);
        if (!direct(app, cpu, greyColours))
            return;
        cpu.ecx += 0x34;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419870: the whole track's colours made grey, as tintTrack goes. */
void greyTrack(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = kTrackObjects;
    if (!direct(app, cpu, greyObjectList))
        return;
    cpu.ecx = 0;
    for (;;)
    {
        safepoint(app, cpu);
        if (sreg32(cpu.ecx) > sreg32(at(app, kLastBlock)))
            break;
        cpu.eax = blockOffset(cpu.ecx);
        cpu.edx = kBlocks + cpu.eax;
        for (const reg32 list : {0x564u, 0x56cu, 0x574u})
        {
            cpu.eax = cpu.edx + list;
            if (!direct(app, cpu, greyObjectList))
                return;
        }
        cpu.eax = at(app, cpu.edx + 0x74);
        cpu.ebx = at(app, cpu.edx + 0x58);
        cpu.edx = cpu.eax;
        cpu.eax = cpu.ebx;
        ++cpu.ecx;
        if (!direct(app, cpu, greyColours))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4198e0: a list of objects read from the .frd into eax -- its count, then
 * 0x34 bytes each, from the arena, with what each points at: its extra data
 * (+0x18 bytes at +0x1c), its vertices (12 bytes each), their colours (4) and
 * its polygons (14). */
void readObjectList(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 count = cpu.ebp - 4;
    // An object's place in the list: esi * 0x34, as the code works it out.
    const auto objectOffset = [&] { return cpu.esi * 0x34; };
    cpu.ecx = cpu.eax;
    cpu.edx = 4;
    if (!direct(app, cpu, frdRead))
        return;
    cpu.edx = at(app, cpu.ecx);
    cpu.eax = cpu.edx * 0x34;
    cpu.edx = 0;
    cpu.esi = 0;
    if (!direct(app, cpu, arenaTake))
        return;
    put(app, cpu.ecx + 4, cpu.eax);
    for (;;)
    {
        safepoint(app, cpu);
        if (sreg32(cpu.esi) >= sreg32(at(app, cpu.ecx)))
            break;
        cpu.ebx = objectOffset();
        // Its first five fields: 4, 4, 4, 12 and 4 bytes.
        const reg32 fields[][2] = {{0, 4}, {4, 4}, {8, 4}, {0xc, 0xc}, {0x18, 4}};
        for (const auto& field : fields)
        {
            cpu.eax = at(app, cpu.ecx + 4) + cpu.ebx;
            cpu.edx = field[1];
            cpu.eax += field[0];
            if (!direct(app, cpu, frdRead))
                return;
        }
        cpu.eax = at(app, cpu.ecx + 4) + cpu.ebx;
        cpu.edi = at(app, cpu.eax + 0x18);
        if (cpu.edi == 0)
        {
            put(app, cpu.eax + 0x1c, cpu.edi);
        }
        else
        {
            cpu.eax = cpu.edi;
            cpu.edx = 0;
            if (!direct(app, cpu, arenaTake))
                return;
            cpu.edx = at(app, cpu.ecx + 4);
            put(app, cpu.edx + cpu.ebx + 0x1c, cpu.eax);
            cpu.eax = at(app, cpu.ecx + 4);
            cpu.edx = at(app, cpu.ebx + cpu.eax + 0x18);
            cpu.eax = at(app, cpu.ebx + cpu.eax + 0x1c);
            if (!direct(app, cpu, frdRead))
                return;
        }
        cpu.ebx = objectOffset();
        cpu.eax = at(app, cpu.ecx + 4) + cpu.ebx;
        cpu.edx = 4;
        cpu.eax += 0x20;
        if (!direct(app, cpu, frdRead))
            return;
        // The vertices.
        cpu.eax = at(app, cpu.ecx + 4);
        cpu.edx = at(app, cpu.ebx + cpu.eax + 0x20);
        cpu.eax = cpu.edx * 12;
        cpu.edx = 0;
        if (!direct(app, cpu, arenaTake))
            return;
        cpu.edx = at(app, cpu.ecx + 4);
        put(app, cpu.edx + cpu.ebx + 0x24, cpu.eax);
        cpu.eax = at(app, cpu.ecx + 4);
        cpu.edx = cpu.eax + cpu.ebx;
        cpu.eax = at(app, cpu.edx + 0x20);
        put(app, count, cpu.eax);
        cpu.edi = at(app, count);
        cpu.eax = ((cpu.eax << 2) - cpu.edi) << 2;
        cpu.edi = at(app, cpu.edx + 0x24);
        cpu.edx = cpu.eax;
        cpu.eax = cpu.edi;
        if (!direct(app, cpu, frdRead))
            return;
        // Their colours.
        cpu.eax = at(app, cpu.ecx + 4);
        cpu.eax = at(app, cpu.ebx + cpu.eax + 0x20);
        cpu.edx = 0;
        cpu.eax <<= 2;
        if (!direct(app, cpu, arenaTake))
            return;
        cpu.edx = at(app, cpu.ecx + 4);
        put(app, cpu.edx + cpu.ebx + 0x28, cpu.eax);
        cpu.eax = at(app, cpu.ecx + 4);
        cpu.edx = at(app, cpu.ebx + cpu.eax + 0x20) << 2;
        cpu.eax = at(app, cpu.ebx + cpu.eax + 0x28);
        if (!direct(app, cpu, frdRead))
            return;
        // The polygons.
        cpu.eax = at(app, cpu.ecx + 4) + cpu.ebx;
        cpu.edx = 4;
        cpu.eax += 0x2c;
        if (!direct(app, cpu, frdRead))
            return;
        cpu.eax = at(app, cpu.ecx + 4);
        cpu.eax = at(app, cpu.ebx + cpu.eax + 0x2c);
        cpu.eax += cpu.eax;
        cpu.edx = cpu.eax;
        cpu.eax = (cpu.eax << 3) - cpu.edx;
        cpu.edx = 0;
        if (!direct(app, cpu, arenaTake))
            return;
        cpu.edx = at(app, cpu.ecx + 4);
        put(app, cpu.edx + cpu.ebx + 0x30, cpu.eax);
        cpu.eax = at(app, cpu.ecx + 4);
        cpu.ebx += cpu.eax;
        cpu.eax = at(app, cpu.ebx + 0x2c);
        cpu.eax += cpu.eax;
        cpu.edx = cpu.eax;
        cpu.eax = (cpu.eax << 3) - cpu.edx;
        cpu.ebx = at(app, cpu.ebx + 0x30);
        cpu.edx = cpu.eax;
        cpu.eax = cpu.ebx;
        ++cpu.esi;
        if (!direct(app, cpu, frdRead))
            return;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419ab0: the track's lights and sound sources read from the .frd (their
 * count to 0x552df8, then 0x2f bytes each into the arena, at [0x552df4]),
 * each given the record of its kind -- +0x2b's high word the kind, from the
 * table at 0x8b41a0 where +0x2c is set, else from the track's at
 * [0x5dd858], 0x2c bytes a kind. */
void readLights(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 4;
    cpu.eax = kLightCount;
    if (!direct(app, cpu, frdRead))
        return;
    cpu.edx = at(app, kLightCount);
    cpu.eax = cpu.edx * 0x2f;
    cpu.edx = 0;
    cpu.ecx = 0;
    if (!direct(app, cpu, arenaTake))
        return;
    put(app, kLights, cpu.eax);
    for (;;)
    {
        safepoint(app, cpu);
        if (sreg32(cpu.ecx) >= sreg32(at(app, kLightCount)))
            break;
        cpu.ebx = cpu.ecx * 0x2f;
        cpu.eax = at(app, kLights);
        cpu.edx = 0x2f;
        cpu.eax += cpu.ebx;
        if (!direct(app, cpu, frdRead))
            return;
        cpu.eax = at(app, kLights);
        cpu.edx = cpu.eax + cpu.ebx;
        cpu.ebx = at(app, cpu.edx + 0x2b);
        cpu.ah = byte(app, cpu.edx + 0x2c);
        cpu.ebx = reg32(sreg32(cpu.ebx) >> 0x10);
        if (cpu.ah != 0)
        {
            cpu.eax = at(app, cpu.ebx * 4 + kLightKinds);
            cpu.eax = at(app, cpu.eax + 4);
        }
        else
        {
            cpu.eax = cpu.ebx * 11;
            cpu.ebx = cpu.eax * 4;
            cpu.eax = at(app, kTrackKinds);
            cpu.eax = at(app, cpu.ebx + cpu.eax + 4);
        }
        put(app, cpu.edx + 4, cpu.eax);
        ++cpu.ecx;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419b60: the arena allocated (0x310000 bytes, unnamed) and none of it
 * used. */
void newArena(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = kNoName;
    cpu.edx = 0;
    cpu.ebx = 0;
    put(app, kArenaUsed, cpu.edx);
    cpu.edx = kArenaSize;
    if (!byAddress(app, cpu, kAllocate))
        return;
    put(app, kArena, cpu.eax);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419b90: the arena freed, if there is one, and forgotten. */
void freeArena(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kArena);
    if (cpu.edx != 0)
    {
        cpu.eax = cpu.edx;
        cpu.ecx = 0;
        if (!byAddress(app, cpu, kFree))
            return;
        put(app, kArena, cpu.ecx);
        put(app, kArenaUsed, cpu.ecx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419bc0: eax bytes of the arena, or 0 where they would take it past its
 * size (the use then as it was). */
void arenaTake(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = at(app, kArenaUsed);
    cpu.eax += cpu.ecx;
    if (sreg32(cpu.eax) > sreg32(kArenaSize))
    {
        cpu.eax = 0;
    }
    else
    {
        cpu.edx = at(app, kArena) + cpu.ecx;
        cpu.ecx = cpu.eax;
        cpu.eax = cpu.edx;
    }
    put(app, kArenaUsed, cpu.ecx);
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x419bf0: edx bytes of the .frd moved to eax (the game's memmove), the
 * reading moved on past them. */
void frdRead(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.edx;
    cpu.esi = at(app, kFrdAt);
    cpu.ebx = cpu.edx;
    cpu.edx = cpu.eax;
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, kMemmove))
        return;
    put(app, kFrdAt, at(app, kFrdAt) + cpu.ecx);
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41a100: the track's geometry loaded (0x419c20, the .frd read), its
 * colours tinted and, on a driver that wants it (bit 2 of [0x7a3a58]),
 * made grey. */
void loadGeometry(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!byAddress(app, cpu, 0x419c20) || !direct(app, cpu, tintTrack))
        return;
    if ((byte(app, kDriverBits) & 2) != 0 && !direct(app, cpu, greyTrack))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41a120 (stdcall, 0x54 bytes): a triangle of the projected headlights'
 * pass as a polygon record at [eax] (0xa0 bytes: the header, then its three
 * corners of 0x20 bytes at +0x20, +0x40, +0x60), [eax] moved past it -- the
 * corners the stack's three of 0x1c bytes, each's colour scaled by alpha
 * intensity below 1.  Not where all three corners are off the screen on one
 * side (+0x14 of the vertex each points at), nor where none is bright enough
 * (the colours' alpha below 8).  1 for a record, else 0.
 *
 * The port's change (tools/apply_alpha_intensity.py): the scaling on this
 * driver too, not only where [0x7a3a58] says Direct3D (bit 0x40). */
void projectedTriangle(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x18;
    const reg32 first = cpu.ebp - 8, second = cpu.ebp - 4, third = cpu.ebp - 0xc, records = cpu.ebp - 0x10;
    const reg32 intensity = cpu.ebp - 0x18;
    // The corners on the stack: +0 its first word, +8 its vertex, +0xc and
    // +0x10 two words, +0x14 its colour.
    const reg32 corners[] = {cpu.ebp + 0x14, cpu.ebp + 0x30, cpu.ebp + 0x4c};
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4 + 0x54;
    };
    put(app, records, cpu.eax);
    cpu.eax = at(app, cpu.eax);
    cpu.edx = cpu.eax + 0x20;
    put(app, first, cpu.edx);
    cpu.edx = cpu.eax + 0x40;
    put(app, second, cpu.edx);
    cpu.edx = cpu.eax + 0x60;
    put(app, third, cpu.edx);
    cpu.edx = at(app, corners[0] + 8);
    cpu.ebx = byte(app, cpu.edx + 0x14);
    cpu.edx = at(app, corners[1] + 8);
    cpu.dl = byte(app, cpu.edx + 0x14);
    cpu.edx &= 0xff;
    cpu.edx &= cpu.ebx;
    cpu.ebx = at(app, corners[2] + 8);
    cpu.bl = byte(app, cpu.ebx + 0x14);
    cpu.ebx &= 0xff;
    if ((cpu.edx & cpu.ebx) != 0)
    {
        cpu.eax = 0;
        return leave();
    }
    putHalf(app, cpu.eax + 4, 1);
    put(app, cpu.eax + 0x18, kHeadlightTexture);
    cpu.edx = cpu.eax + 0xa0;
    putHalf(app, cpu.eax + 6, 0x13);
    put(app, cpu.eax, cpu.edx);
    cpu.edx = at(app, first);
    cpu.ecx = 8;
    put(app, cpu.eax + 8, cpu.edx);
    cpu.edx = at(app, second);
    cpu.edi = at(app, first);
    put(app, cpu.eax + 0xc, cpu.edx);
    cpu.edx = at(app, third);
    cpu.esi = at(app, corners[0] + 8);
    put(app, cpu.eax + 0x10, cpu.edx);
    // rep movsd: each corner's vertex, 8 dwords, into the record's corner.
    const auto copyVertex = [&] {
        for (; cpu.ecx != 0; --cpu.ecx)
            movsd(app, cpu);
    };
    copyVertex();
    cpu.ecx = 8;
    cpu.edi = at(app, second);
    cpu.esi = at(app, corners[1] + 8);
    copyVertex();
    cpu.ecx = 8;
    cpu.esi = at(app, corners[2] + 8);
    cpu.edi = cpu.edx;
    cpu.ah = byte(app, kDriverBits);
    copyVertex();
    // Port: test ah, 0x40 taken as set -- the scaling on this driver too.
    putF64(app, intensity, f32(app, kAlphaIntensity));
    if (!compare(cpu, Float(1.0), f64(app, intensity)).belowOrEqual())
    {
        const Float scaled = fp::mul(f64(app, intensity), f64(app, 0x536c6c));
        cpu.esp -= 4;
        put(app, cpu.esp, reg32(fp::toInteger<sreg32>(scaled)));
        cpu.eax = pop(app, cpu);
        cpu.esi = cpu.eax;
        for (const reg32 corner : corners)
        {
            cpu.eax = at(app, corner + 0x14);
            cpu.ecx = cpu.esi;
            scaleColour(cpu);
            put(app, corner + 0x14, cpu.eax);
        }
    }
    // Each record's corner: the stack's corner's first word at +0xc, its
    // +0xc and +0x10 at +0x18 and +0x1c, its colour at +0x10.
    const reg32 targets[] = {first, second, third};
    for (int k = 0; k < 3; ++k)
    {
        cpu.edx = at(app, targets[k]);
        cpu.eax = at(app, corners[k]);
        put(app, cpu.edx + 0xc, cpu.eax);
        cpu.eax = at(app, corners[k] + 0xc);
        put(app, cpu.edx + 0x18, cpu.eax);
        cpu.eax = at(app, corners[k] + 0x10);
        put(app, cpu.edx + 0x1c, cpu.eax);
        cpu.eax = at(app, corners[k] + 0x14);
        put(app, cpu.edx + 0x10, cpu.eax);
    }
    cpu.eax = at(app, first);
    cpu.edx = at(app, second);
    cpu.eax = at(app, cpu.eax + 0x10);
    cpu.ecx = at(app, cpu.edx + 0x10);
    cpu.edx = at(app, third);
    cpu.eax |= cpu.ecx;
    cpu.eax |= at(app, cpu.edx + 0x10);
    cpu.eax >>= 0x18;
    if (cpu.eax < 8)
    {
        cpu.eax = 0;
        return leave();
    }
    cpu.edx = at(app, records);
    cpu.esi = at(app, cpu.edx) + 0xa0;
    cpu.eax = 1;
    put(app, cpu.edx, cpu.esi);
    leave();
}

/* 0x41a3e0 (stdcall, 5 words): the colour [esp+0x10] of a track vertex
 * darkened, to [[esp+0x14]] and eax -- by how far off the view's middle it
 * is (|x| and |y| over its depth [esp+0xc], each clamped to 1, their sum times
 * 0x536c74 off 1) and, beyond the near distance, by its depth over the
 * shading's range (setShadeDistances). */
void shadeVertex(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x30;
    const reg32 x = cpu.ebp + 0x18, y = cpu.ebp + 0x1c, depth = cpu.ebp + 0x20, colour = cpu.ebp + 0x24;
    const reg32 to = cpu.ebp + 0x28;
    const reg32 shade = cpu.ebp - 8, across = cpu.ebp - 4, up = cpu.ebp - 0xc, absY = cpu.ebp - 0x10;
    const reg32 absX = cpu.ebp - 0x1c, inverse = cpu.ebp - 0x28, acrossClamped = cpu.ebp - 0x14;
    const reg32 acrossSum = cpu.ebp - 0x2c, upClamped = cpu.ebp - 0x18;
    cpu.edx = at(app, colour);
    const Float reciprocal = fp::div(Float(1.0), f32(app, depth));
    cpu.ecx = 0x3f800000;
    put(app, shade, cpu.ecx);
    putF32(app, inverse, reciprocal);
    if (!compare(cpu, Float(0.0), f32(app, x)).belowOrEqual())
    {
        putF32(app, absX, -f32(app, x));
    }
    else
    {
        cpu.eax = at(app, x);
        put(app, absX, cpu.eax);
    }
    cpu.eax = at(app, absX);
    put(app, across, cpu.eax);
    if (!compare(cpu, Float(0.0), f32(app, y)).belowOrEqual())
    {
        putF32(app, absY, -f32(app, y));
    }
    else
    {
        cpu.eax = at(app, y);
        put(app, absY, cpu.eax);
    }
    const Float t = f32(app, inverse);
    const Float a = fp::mul(f32(app, across), t);
    const Float b = fp::mul(t, f32(app, absY));
    const Float d = f32(app, depth);
    putF32(app, across, a);
    putF32(app, up, b);
    if (!compare(cpu, d, f32(app, kShadeNear)).belowOrEqual())
    {
        const Float beyond = fp::mul(fp::sub(f32(app, depth), f32(app, kShadeNear)), f32(app, kShadeScale));
        putF32(app, shade, fp::sub(Float(1.0), beyond));
    }
    if (!compare(cpu, Float(0.0), f32(app, across)).belowOrEqual())
    {
        cpu.ebx = 0;
        put(app, acrossClamped, cpu.ebx);
    }
    else
    {
        cpu.esi = at(app, across);
        put(app, cpu.ebp - 0x24, sreg32(cpu.esi) > 0x3f800000 ? 0x3f800000 : cpu.esi);
        cpu.eax = at(app, cpu.ebp - 0x24);
        put(app, acrossClamped, cpu.eax);
    }
    cpu.eax = at(app, acrossClamped);
    put(app, acrossSum, cpu.eax);
    if (!compare(cpu, Float(0.0), f32(app, up)).belowOrEqual())
    {
        cpu.eax = 0;
    }
    else
    {
        cpu.ecx = at(app, up);
        put(app, cpu.ebp - 0x20, sreg32(cpu.ecx) > 0x3f800000 ? 0x3f800000 : cpu.ecx);
        cpu.eax = at(app, cpu.ebp - 0x20);
    }
    put(app, upClamped, cpu.eax);
    const Float sum = fp::add(f32(app, acrossSum), f32(app, upClamped));
    const Float off = fp::mul(sum, f32(app, 0x536c74));
    const Float darkened = fp::mul(fp::sub(Float(1.0), off), f32(app, shade));
    putF32(app, shade, darkened);
    const Float factor = fp::mul(darkened, f64(app, 0x536c7c));
    cpu.esp -= 4;
    put(app, cpu.esp, reg32(fp::toInteger<sreg32>(factor)));
    cpu.eax = pop(app, cpu);
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.edx;
    scaleColour(cpu);
    cpu.edx = at(app, to);
    put(app, cpu.edx, cpu.eax);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4 + 0x14;
}

/* 0x41a800 (stdcall, 2 words): the shading's distances for the camera at eax
 * scaled by [esp+4]: its near (+0x68) and far (+0x6c) ones, the far at least
 * 1 past the near; the light's reach (+0x64); 1 / (far - near). */
void setShadeDistances(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 scale = cpu.ebp + 0xc, nearPlusOne = cpu.ebp - 4, farthest = cpu.ebp - 8;
    const Float s = f32(app, scale);
    const Float nearDistance = fp::mul(f32(app, cpu.eax + 0x68), s);
    const Float farDistance = fp::mul(s, f32(app, cpu.eax + 0x6c));
    cpu.edx = at(app, cpu.eax + 0x64);
    put(app, kShadeReach, cpu.edx);
    putF32(app, kShadeNear, nearDistance);
    putF32(app, kShadeFar, farDistance);
    const Float plusOne = fp::add(Float(1.0), f32(app, kShadeNear));
    const Float stored = f32(app, kShadeFar);
    putF32(app, nearPlusOne, plusOne);
    if (!compare(cpu, stored, f32(app, nearPlusOne)).belowOrEqual())
        cpu.eax = at(app, kShadeFar);
    else
        cpu.eax = at(app, nearPlusOne);
    put(app, farthest, cpu.eax);
    const Float range = fp::sub(f32(app, farthest), f32(app, kShadeNear));
    const Float inverse = fp::div(Float(1.0), range);
    cpu.eax = at(app, farthest);
    put(app, kShadeFar, cpu.eax);
    putF32(app, kShadeScale, inverse);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4 + 8;
}

/* 0x41a8d0 (stdcall, 2 words): the point at ebx taken into the camera eax's
 * view -- less its place, turned by its matrix -- and the object's vertices
 * (edx, ecx, the stack's words) into view from it (0x4dbef0). */
void viewObjectVertices(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x18;
    cpu.edi = cpu.edx;
    putF32(app, cpu.ebp - 0x18, fp::sub(f32(app, cpu.ebx), f32(app, cpu.eax + 8)));
    cpu.esi = cpu.eax + 0x44;
    putF32(app, cpu.ebp - 0x14, fp::sub(f32(app, cpu.ebx + 4), f32(app, cpu.eax + 0xc)));
    cpu.edx = cpu.esi;
    const Float z = fp::sub(f32(app, cpu.ebx + 8), f32(app, cpu.eax + 0x10));
    cpu.ebx = cpu.ebp - 0xc;
    cpu.eax = cpu.ebp - 0x18;
    putF32(app, cpu.ebp - 0x10, z);
    rotate(app, cpu);
    cpu.edx = at(app, cpu.ebp + 0x14);
    push(app, cpu, cpu.edx);
    cpu.ebx = at(app, cpu.ebp + 0x10);
    cpu.eax = cpu.esi;
    push(app, cpu, cpu.ebx);
    cpu.edx = cpu.ebp - 0xc;
    cpu.ebx = cpu.edi;
    if (!byAddress(app, cpu, 0x4dbef0))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 8;
}

/* 0x41a9b0 (stdcall, 3 words): viewObjectVertices for an object turned by
 * its own matrix (edx): the point at ecx into the camera eax's view, the
 * object's matrix times the camera's (0x4e0280), then its vertices (ebx, the
 * stack's words) into view (0x4dbef0). */
void viewTurnedObjectVertices(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x3c;
    cpu.edi = cpu.edx;
    cpu.esi = cpu.ebx;
    putF32(app, cpu.ebp - 0x18, fp::sub(f32(app, cpu.ecx), f32(app, cpu.eax + 8)));
    putF32(app, cpu.ebp - 0x14, fp::sub(f32(app, cpu.ecx + 4), f32(app, cpu.eax + 0xc)));
    cpu.ebx = cpu.ebp - 0xc;
    const Float z = fp::sub(f32(app, cpu.ecx + 8), f32(app, cpu.eax + 0x10));
    cpu.ecx = cpu.eax + 0x44;
    cpu.edx = cpu.ecx;
    cpu.eax = cpu.ebp - 0x18;
    putF32(app, cpu.ebp - 0x10, z);
    rotate(app, cpu);
    cpu.ebx = cpu.ebp - 0x3c;
    cpu.edx = cpu.ecx;
    cpu.eax = cpu.edi;
    if (!byAddress(app, cpu, 0x4e0280))
        return;
    cpu.edx = at(app, cpu.ebp + 0x18);
    cpu.ecx = at(app, cpu.ebp + 0x14);
    push(app, cpu, cpu.edx);
    cpu.eax = cpu.ebp - 0x3c;
    cpu.ebx = cpu.esi;
    push(app, cpu, cpu.ecx);
    cpu.edx = cpu.ebp - 0xc;
    cpu.ecx = at(app, cpu.ebp + 0x10);
    if (!byAddress(app, cpu, 0x4dbef0))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 0xc;
}

/* 0x41aa60 (stdcall, 1 word): which of the track's models (0 the coarsest, 2
 * the detailed one) to draw at the square of a distance [esp+4] for the
 * camera at eax: past the view's +0x44, 0; past its +0x40, 1; else 2.  The
 * mirror's pass always 0 -- the port's change (tools/apply_mirror_detail.py):
 * asked through nfs3hp::mirrorAsMain, which answers as the main view would
 * with the mirror drawn in full. */
void modelByDistance(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const reg32 distance = cpu.ebp + 0xc;
    const auto leave = [&](reg32 result) {
        cpu.eax = result;
        cpu.ebp = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.esp += 4 + 4;
    };
    if (mirrorAsMain(at(app, cpu.eax)) == 1)
        return leave(0);
    cpu.edx = at(app, kView);
    const Float coarse = f32(app, cpu.edx + 0x44);
    if (compare(cpu, fp::mul(coarse, coarse), f32(app, distance)).below)
        return leave(0);
    const Float middle = f32(app, cpu.edx + 0x40);
    if (compare(cpu, fp::mul(middle, middle), f32(app, distance)).below)
        return leave(1);
    leave(2);
}

/* 0x41aaf0: the view's distances worked out for the camera eax (0x41d620),
 * view edx selected, and the block the camera is in (0x4217e0 of its +4) its
 * block, 8 to a group (+0xc, the group before it kept at +0x18 and +0x14
 * cleared to -1 where it changed; +0x10). */
void viewOnBlock(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    if (!byAddress(app, cpu, 0x41d620))
        return;
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, selectView))
        return;
    cpu.eax = at(app, cpu.ecx + 4);
    if (!byAddress(app, cpu, 0x4217e0))
        return;
    cpu.edx = cpu.eax;
    cpu.eax = at(app, kView);
    cpu.ecx = at(app, cpu.eax + 0xc);
    put(app, cpu.eax + 4, cpu.edx);
    cpu.edx = reg32(sreg32(cpu.edx) >> 3);
    if (cpu.edx != cpu.ecx)
    {
        put(app, cpu.eax + 0xc, cpu.edx);
        cpu.edx = at(app, cpu.eax + 0x14);
        put(app, cpu.eax + 0x14, 0xffffffff);
        put(app, cpu.eax + 0x18, cpu.edx);
    }
    cpu.eax = at(app, kView);
    cpu.edx = reg32(sreg32(at(app, cpu.eax + 4)) >> 3);
    put(app, cpu.eax + 0x10, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41ab40 (stdcall, 1 word): the headlights' light on the polygons of the
 * object list ecx of block ebx: the [esp+4] vertices (0x1c bytes each, as
 * 0x41a550 leaves them) of each polygon's four corners that are lit (+4 and
 * +6 words 0) raise their vertex's colour bytes (+0x10..+0x12 of the vertex
 * at +8) to theirs (+0x14..+0x16, +0x18..+0x1a) -- not a polygon past the
 * world's end ([0x5dd830]'s +0x30) or off the screen on one side. */
void lightObjectPolygons(App* app, CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
    const reg32 count = cpu.ebp - 0x10, polygon = cpu.ebp - 0xc, done = cpu.ebp - 4, vertex = cpu.ebp - 8;
    const reg32 flags = cpu.ebp - 0x14, vertices = cpu.ebp + 0x10;
    // mov P, [V + 8]; mov al, [V + from]; mov ah, [P + to]; cmp al, ah; ja;
    // mov al, ah; mov S, [V + 8]; mov [S + to], al.
    const auto raiseByAl = [&](reg32 v, reg32& p, reg32& s, reg32 from, reg32 to) {
        p = at(app, v + 8);
        cpu.al = byte(app, v + from);
        cpu.ah = byte(app, p + to);
        cpu.al = larger(cpu.al, cpu.ah);
        s = at(app, v + 8);
        putByte(app, s + to, cpu.al);
    };
    // mov eax, [V + 8]; mov dl, [V + from]; mov X, [eax + to]; cmp dl, X;
    // jbe: mov al, X, else mov al, dl; mov edx, [V + 8]; mov [edx + to], al.
    const auto raiseByDl = [&](reg32 v, reg8& x, reg32 from, reg32 to) {
        cpu.eax = at(app, v + 8);
        cpu.dl = byte(app, v + from);
        x = byte(app, cpu.eax + to);
        cpu.al = cpu.dl > x ? cpu.dl : x;
        cpu.edx = at(app, v + 8);
        putByte(app, cpu.edx + to, cpu.al);
    };
    cpu.eax = cpu.ebx;
    cpu.edx = blockOffset(cpu.ebx);
    cpu.eax = cpu.ecx * 4;
    cpu.edx += kBlocks;
    cpu.eax += cpu.edx;
    cpu.edx = at(app, cpu.eax + 0x2c);
    put(app, polygon, cpu.edx);
    cpu.eax = at(app, cpu.eax);
    cpu.edx = 0;
    put(app, count, cpu.eax);
    put(app, done, cpu.edx);
    for (;;)
    {
        safepoint(app, cpu);
        cpu.eax = at(app, done);
        if (sreg32(cpu.eax) >= sreg32(at(app, count)))
            break;
        // The corners' vertices: the polygon's words at +0, +2, +4, +6, 0x1c
        // bytes a vertex.
        cpu.edx = reg32(sreg32(std::int16_t(half(app, at(app, polygon)))));
        cpu.edx <<= 2;
        cpu.eax = cpu.edx;
        cpu.edx <<= 3;
        cpu.esi = at(app, vertices);
        cpu.edx -= cpu.eax;
        cpu.esi += cpu.edx;
        cpu.edx = at(app, polygon);
        cpu.ebx = at(app, vertices);
        cpu.edx = at(app, cpu.edx);
        cpu.ecx = at(app, polygon);
        cpu.edx = reg32(sreg32(cpu.edx) >> 0x10);
        cpu.ecx = at(app, cpu.ecx + 2);
        cpu.edx <<= 2;
        cpu.ecx = reg32(sreg32(cpu.ecx) >> 0x10);
        cpu.eax = cpu.edx;
        cpu.edx <<= 3;
        cpu.ecx <<= 2;
        cpu.edx -= cpu.eax;
        cpu.eax = cpu.ecx;
        cpu.ecx <<= 3;
        cpu.edi = cpu.ebx;
        cpu.ecx -= cpu.eax;
        cpu.edx += cpu.ebx;
        cpu.ecx += cpu.ebx;
        cpu.ebx = at(app, polygon);
        cpu.ebx = reg32(sreg32(at(app, cpu.ebx + 4)) >> 0x10);
        cpu.ebx <<= 2;
        cpu.eax = cpu.ebx;
        cpu.ebx <<= 3;
        cpu.ebx -= cpu.eax;
        cpu.ebx += cpu.edi;
        cpu.eax = at(app, cpu.esi + 8);
        put(app, vertex, cpu.eax);
        cpu.edi = at(app, vertex);
        cpu.eax = at(app, kView);
        const bool beyond = compare(cpu, f32(app, cpu.edi + 8), f32(app, cpu.eax + 0x30)).above();
        if (!beyond)
        {
            cpu.eax = byte(app, cpu.edi + 0x14);
            put(app, flags, cpu.eax);
            cpu.eax = at(app, cpu.edx + 8);
            cpu.al = byte(app, cpu.eax + 0x14);
            cpu.edi = at(app, flags);
            cpu.eax &= 0xff;
            cpu.edi &= cpu.eax;
            cpu.eax = at(app, cpu.ecx + 8);
            cpu.al = byte(app, cpu.eax + 0x14);
            cpu.eax &= 0xff;
            cpu.edi &= cpu.eax;
            cpu.eax = at(app, cpu.ebx + 8);
            cpu.al = byte(app, cpu.eax + 0x14);
            cpu.eax &= 0xff;
        }
        if (!beyond && (cpu.eax & cpu.edi) == 0)
        {
            // The corners lit by the first light (+4), then by the second (+6).
            if (half(app, cpu.esi + 4) == 0)
                for (const reg32 k : {2u, 1u, 0u})
                    raiseByAl(cpu.esi, cpu.edi, cpu.edi, 0x14 + k, 0x10 + k);
            if (half(app, cpu.edx + 4) == 0)
                for (const reg32 k : {2u, 1u, 0u})
                    raiseByAl(cpu.edx, cpu.edi, cpu.edi, 0x14 + k, 0x10 + k);
            if (half(app, cpu.ecx + 4) == 0)
                for (const reg32 k : {2u, 1u, 0u})
                    raiseByAl(cpu.ecx, cpu.edi, cpu.edi, 0x14 + k, 0x10 + k);
            if (half(app, cpu.ebx + 4) == 0)
                for (const reg32 k : {2u, 1u, 0u})
                    raiseByAl(cpu.ebx, cpu.edi, cpu.edi, 0x14 + k, 0x10 + k);
            if (half(app, cpu.esi + 6) == 0)
            {
                raiseByAl(cpu.esi, cpu.edi, cpu.edi, 0x1a, 0x12);
                raiseByAl(cpu.esi, cpu.edi, cpu.edi, 0x19, 0x11);
                raiseByAl(cpu.esi, cpu.edi, cpu.esi, 0x18, 0x10);
            }
            if (half(app, cpu.edx + 6) == 0)
            {
                raiseByAl(cpu.edx, cpu.esi, cpu.esi, 0x1a, 0x12);
                raiseByAl(cpu.edx, cpu.esi, cpu.esi, 0x19, 0x11);
                raiseByAl(cpu.edx, cpu.esi, cpu.edx, 0x18, 0x10);
            }
            if (half(app, cpu.ecx + 6) == 0)
            {
                raiseByDl(cpu.ecx, cpu.dh, 0x1a, 0x12);
                raiseByDl(cpu.ecx, cpu.dh, 0x19, 0x11);
                raiseByDl(cpu.ecx, cpu.dh, 0x18, 0x10);
            }
            if (half(app, cpu.ebx + 6) == 0)
            {
                raiseByDl(cpu.ebx, cpu.cl, 0x1a, 0x12);
                raiseByDl(cpu.ebx, cpu.ch, 0x19, 0x11);
                raiseByDl(cpu.ebx, cpu.dh, 0x18, 0x10);
            }
        }
        put(app, polygon, at(app, polygon) + 0xe);
        put(app, done, at(app, done) + 1);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 4;
}

/* 0x41b8c0: the list eax (its first and last record), if not empty, put
 * between two state records of the drawing's (0x10 bytes from Render_GetTm,
 * kind 5, 0xd, the first +0xc 0 and the last 1): the drawing's state reset
 * (0x4bbf20), each record made a list of its own (0x4bbe90) and the lists
 * joined (0x4bbec0). */
void bracketList(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x24;
    const reg32 record = cpu.ebp - 4;
    // A state record, where Render_GetTm gave one: kind 5, 0xd, `last`.
    const auto stateRecord = [&](reg32 last) {
        cpu.eax = at(app, record);
        putHalf(app, cpu.eax + 4, 5);
        cpu.eax = at(app, record);
        put(app, cpu.eax, 0);
        cpu.eax = at(app, record);
        put(app, cpu.eax + 8, 0xd);
        cpu.eax = at(app, record);
        put(app, cpu.eax + 0xc, last);
    };
    cpu.ecx = cpu.eax;
    if (at(app, cpu.eax) != 0)
    {
        if (!byAddress(app, cpu, 0x4bbf20))
            return;
        cpu.edx = 0x10;
        cpu.eax = record;
        if (!byAddress(app, cpu, 0x4bbde0))
            return;
        if (cpu.eax != 0)
            stateRecord(0);
        cpu.ebx = at(app, cpu.ecx + 4);
        push(app, cpu, cpu.ebx);
        cpu.esi = at(app, cpu.ecx);
        push(app, cpu, cpu.esi);
        cpu.eax = at(app, record);
        cpu.esi = cpu.ebp - 0x14;
        if (!byAddress(app, cpu, 0x4bbe90))
            return;
        cpu.edi = at(app, cpu.ebp - 0x10);
        push(app, cpu, cpu.edi);
        cpu.eax = at(app, cpu.ebp - 0x14);
        push(app, cpu, cpu.eax);
        cpu.esi = cpu.ebp - 0x24;
        if (!byAddress(app, cpu, 0x4bbec0))
            return;
        cpu.edi = cpu.ecx;
        cpu.esi = cpu.ebp - 0x24;
        cpu.edx = 0x10;
        cpu.eax = record;
        movsd(app, cpu);
        movsd(app, cpu);
        if (!byAddress(app, cpu, 0x4bbde0))
            return;
        if (cpu.eax != 0)
            stateRecord(1);
        cpu.eax = at(app, record);
        cpu.esi = cpu.ebp - 0xc;
        if (!byAddress(app, cpu, 0x4bbe90))
            return;
        cpu.edx = at(app, cpu.ebp - 8);
        push(app, cpu, cpu.edx);
        cpu.ebx = at(app, cpu.ebp - 0xc);
        push(app, cpu, cpu.ebx);
        cpu.esi = at(app, cpu.ecx + 4);
        push(app, cpu, cpu.esi);
        cpu.edi = at(app, cpu.ecx);
        push(app, cpu, cpu.edi);
        cpu.esi = cpu.ebp - 0x1c;
        if (!byAddress(app, cpu, 0x4bbec0))
            return;
        cpu.esi = cpu.ebp - 0x1c;
        cpu.edi = cpu.ecx;
        movsd(app, cpu);
        movsd(app, cpu);
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41bb50: the ambient sounds around the camera at eax (none in the
 * mirror's pass): the view on its block (viewOnBlock), each sound source of
 * the block's neighbours (+0xb4, 300 words, -1 none; a block's +0x5b0 its
 * sources' count, +0x5b4 them) heard (ambientSource); none heard, the
 * ambient slot stopped (0x416f60: 0x12, or 0x20 for the second player). */
void ambientSounds(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 neighbours = cpu.ebp - 4;
    cpu.ecx = cpu.eax;
    if (at(app, cpu.eax) == 0)
    {
        cpu.edx = at(app, cpu.eax + 4);
        if (!direct(app, cpu, viewOnBlock))
            return;
        cpu.eax = at(app, kView);
        cpu.edx = at(app, cpu.eax + 0x10);
        cpu.eax = blockOffset(cpu.edx) + kBlocks;
        cpu.esi = 0;
        cpu.eax += 0xb4;
        cpu.edi = 0;
        put(app, neighbours, cpu.eax);
        for (;;)
        {
            safepoint(app, cpu);
            cpu.ebx = at(app, neighbours);
            cpu.eax = cpu.edi * 4 + cpu.ebx;
            cpu.dx = half(app, cpu.eax);
            if (cpu.dx != 0xffff)
            {
                safepoint(app, cpu);
                cpu.edx = reg32(sreg32(std::int16_t(cpu.dx)));
                if (cpu.edx != 0xffffffff)
                {
                    cpu.eax = blockOffset(cpu.edx);
                    if (at(app, cpu.eax + kBlocks + 0x5b0) != 0)
                    {
                        cpu.ebx = cpu.esi;
                        cpu.edx = at(app, cpu.eax + kBlocks + 0x5b4);
                        cpu.eax = cpu.ecx;
                        if (!direct(app, cpu, ambientSource))
                            return;
                        cpu.esi |= cpu.eax;
                    }
                }
            }
            ++cpu.edi;
            if (sreg32(cpu.edi) >= 0x12c)
                break;
        }
        if (cpu.esi == 0)
        {
            cpu.eax = at(app, cpu.ecx + 4) == 0 ? 0x12 : 0x20;
            if (!byAddress(app, cpu, 0x416f60))
                return;
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41bc20: whether the track's ambient sounds play: not in a split-screen
 * or network race but on track 0. */
void ambientWanted(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = 1;
    if ((at(app, kNetworkRace) != 0 || at(app, kSplitScreen) != 0) && at(app, kTrackNumber) == 0)
        cpu.eax = 0;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41bc50: whether the light at eax shines away from the camera edx: its
 * place (+0x4c) taken into the camera's view (0x4e0430) and its depth there
 * against the light's own -(+0x6c).  1 behind. */
void pointBehindView(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    cpu.esi = cpu.eax;
    cpu.eax = cpu.ebp - 0xc;
    cpu.ecx = cpu.edx + 0x38;
    push(app, cpu, cpu.eax);
    cpu.ebx = cpu.edx + 0x44;
    cpu.eax = 1;
    cpu.edx = cpu.esi + 0x4c;
    if (!byAddress(app, cpu, 0x4e0430))
        return;
    cpu.eax = compare(cpu, -f32(app, cpu.esi + 0x6c), f32(app, cpu.ebp - 4)).belowOrEqual() ? 0 : 1;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41bc90: whether the box of four corners at edx is out of a light's cone:
 * each corner turned by the matrix at ebx and moved by ecx, its depth's sign
 * kept off 0, projected onto the screen (0x56009c..) with the screen's
 * outcode, and |x| times the light's spread (0x4227e0, at most 1) against
 * |depth|.  1 where every corner is beside the cone and all are off the
 * screen on one side. */
void litBoxOutside(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xd4;
    cpu.ebp -= 0x82;
    const reg32 e = cpu.ebp;
    const reg32 corners = e + 0x6e, matrix = e + 0x5e, origin = e + 0x76, spread = e + 0x72, index = e + 0x7e;
    const reg32 row = e + 0x7a, depth = e + 0x62, absX = e + 0x6a, inverse = e + 0x66;
    // Each corner in view (x, y, z) from e + 0x2e, 12 bytes each; projected
    // (x, y, z, 1 / z, ..., +0x14 the outcode) from e - 0x52, 0x20 each.
    const reg32 points = e + 0x2e, projected = e - 0x52;
    put(app, corners, cpu.edx);
    put(app, matrix, cpu.ebx);
    put(app, origin, cpu.ecx);
    if (!byAddress(app, cpu, 0x4227e0))
        return;
    const Float value = fpu.st(0);
    putF32(app, spread, value);
    const bool below = compare(cpu, Float(1.0), value).below;
    fpu.count -= 1;
    if (below)
        put(app, spread, 0x3f800000);
    cpu.ecx = 0;
    put(app, index, cpu.ecx);
    for (;;)
    {
        cpu.eax = at(app, index);
        cpu.ecx = cpu.eax * 12;
        cpu.ebx = points;
        cpu.edx = at(app, matrix);
        cpu.eax = at(app, corners);
        cpu.ebx += cpu.ecx;
        cpu.eax += cpu.ecx;
        rotate(app, cpu);
        cpu.eax = at(app, origin);
        for (const reg32 k : {0u, 4u, 8u})
            putF32(app, cpu.ecx + points + k, fp::add(f32(app, cpu.eax + k), f32(app, cpu.ecx + points + k)));
        cpu.esi = at(app, cpu.ecx + points + 8);
        if ((cpu.esi & 0x7fffffff) == 0)
            put(app, cpu.ecx + points + 8, 0x37800080);
        cpu.eax = at(app, index);
        cpu.ecx = cpu.eax * 12;
        const Float w = fp::div(Float(1.0), f32(app, cpu.ecx + points + 8));
        put(app, row, cpu.ecx);
        cpu.eax <<= 5;
        cpu.edx = at(app, row);
        cpu.ecx = projected;
        putF32(app, inverse, w);
        cpu.esi = cpu.ecx + cpu.eax;
        cpu.eax = points;
        cpu.ecx = inverse;
        cpu.ebx = cpu.esi;
        cpu.eax += cpu.edx;
        cpu.edi = cpu.esi + 0x14;
        const Float p = fp::mul(f32(app, 0x56009c), f32(app, cpu.ecx));
        const Float q = fp::mul(f32(app, 0x5600a0), f32(app, cpu.ecx));
        cpu.edx = at(app, cpu.ecx);
        const Float qy = fp::mul(q, f32(app, cpu.eax + 4));
        put(app, cpu.ebx + 0xc, cpu.edx);
        const Float px = fp::mul(p, f32(app, cpu.eax));
        cpu.edx = at(app, cpu.eax + 8);
        const Float sx = fp::add(px, f32(app, 0x5600a4));
        const Float sy = fp::add(qy, f32(app, 0x5600a8));
        putF32(app, cpu.ebx, sx);
        put(app, cpu.ebx + 8, cpu.edx);
        putF32(app, cpu.ebx + 4, sy);
        // The outcode: 0x10 behind the near plane; else 8 above, 4 below, 1
        // left, 2 right of the screen ([0x7d34f8..0x7d350c]).
        cpu.eax = at(app, cpu.esi + 0xc);
        cpu.ecx = at(app, 0x7d34f8);
        if ((cpu.eax & 0x80000000) != 0 || sreg32(cpu.eax) >= sreg32(cpu.ecx))
        {
            cpu.ebx = 0x10;
        }
        else
        {
            cpu.eax = at(app, cpu.esi + 4);
            cpu.ebx = 0;
            cpu.ecx = at(app, 0x7d3504);
            cpu.edx = at(app, 0x7d3500);
            if ((cpu.eax & 0x80000000) != 0 || sreg32(cpu.eax) < sreg32(cpu.ecx))
                cpu.ebx |= 8;
            else if (sreg32(cpu.eax) > sreg32(cpu.edx))
                cpu.ebx |= 4;
            cpu.eax = at(app, cpu.esi);
            cpu.ecx = at(app, 0x7d34fc);
            cpu.edx = at(app, 0x7d350c);
            if ((cpu.eax & 0x80000000) != 0 || sreg32(cpu.eax) < sreg32(cpu.ecx))
                cpu.ebx |= 1;
            else if (sreg32(cpu.eax) > sreg32(cpu.edx))
                cpu.ebx |= 2;
        }
        putByte(app, cpu.edi, cpu.bl);
        // |z| and |x| times the spread kept for the corner.
        cpu.eax = at(app, row);
        if (!compare(cpu, Float(0.0), f32(app, cpu.eax + points + 8)).belowOrEqual())
        {
            cpu.eax = at(app, row);
            putF32(app, depth, -f32(app, cpu.eax + points + 8));
        }
        else
        {
            safepoint(app, cpu);
            cpu.eax = at(app, row);
            cpu.eax = at(app, cpu.eax + points + 8);
            put(app, depth, cpu.eax);
        }
        safepoint(app, cpu);
        cpu.eax = at(app, index);
        cpu.ecx = cpu.eax * 12;
        cpu.eax = at(app, depth);
        put(app, cpu.ecx + points + 8, cpu.eax);
        if (!compare(cpu, Float(0.0), f32(app, cpu.ecx + points)).belowOrEqual())
        {
            putF32(app, absX, -f32(app, cpu.ecx + points));
        }
        else
        {
            cpu.eax = at(app, cpu.ecx + points);
            put(app, absX, cpu.eax);
        }
        const Float x = f32(app, absX);
        cpu.eax = at(app, index);
        const Float spreadX = fp::mul(x, f32(app, spread));
        cpu.ecx = cpu.eax * 3;
        putF32(app, e + cpu.ecx * 4 + 0x2e, spreadX);
        cpu.ecx = cpu.eax + 1;
        put(app, index, cpu.ecx);
        if (sreg32(at(app, index)) >= 4)
            break;
    }
    if (compare(cpu, f32(app, points), f32(app, points + 8)).above()
        && compare(cpu, f32(app, points + 0xc), f32(app, points + 0x14)).above()
        && compare(cpu, f32(app, points + 0x18), f32(app, points + 0x20)).above()
        && compare(cpu, f32(app, points + 0x24), f32(app, points + 0x2c)).above())
    {
        cpu.ecx = byte(app, projected + 0x34);
        cpu.eax = byte(app, projected + 0x14);
        cpu.eax &= cpu.ecx;
        cpu.ecx = byte(app, projected + 0x54);
        cpu.eax &= cpu.ecx;
        cpu.ecx = byte(app, projected + 0x74);
        cpu.eax = (cpu.eax & cpu.ecx) != 0 ? 1 : 0;
    }
    else
    {
        cpu.eax = 0;
    }
    cpu.esp = e + 0x82;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41c1a0: a view's buckets of polygons handed to the drawing: the six
 * lists of the track's (heads ebx, tails edx) into the buckets at ecx (an
 * empty one where a list is), the four opaque buckets sorted into the
 * frame's (0x4c7790), the ordered ones bracketed (bracketList) and added
 * (0x4c7940), all gathered (0x4c79a0, 0x4c7750), the buckets emptied
 * (0x4bbe70) and the view eax drawn (0x4bc060). */
void flushBuckets(App* app, CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x4c;
    const reg32 view = cpu.ebp - 0xc, tails = cpu.ebp - 8, offset = cpu.ebp - 4;
    put(app, view, cpu.eax);
    put(app, tails, cpu.edx);
    cpu.esi = cpu.ebp - 0x4c;
    if (!byAddress(app, cpu, 0x4bbe70))
        return;
    cpu.edx = 0;
    for (;;)
    {
        cpu.eax = cpu.edx * 4;
        put(app, offset, cpu.eax);
        cpu.eax += cpu.ebx;
        cpu.edi = cpu.edx * 8;
        cpu.esi = at(app, cpu.eax);
        cpu.edi += cpu.ecx;
        if (cpu.esi == 0)
        {
            safepoint(app, cpu);
            cpu.esi = cpu.ebp - 0x34;
            if (!byAddress(app, cpu, 0x4bbe70))
                return;
            cpu.esi = cpu.ebp - 0x34;
            movsd(app, cpu);
            movsd(app, cpu);
        }
        else
        {
            cpu.eax = at(app, tails);
            put(app, cpu.edi, cpu.esi);
            cpu.eax += at(app, offset);
            cpu.esi = at(app, cpu.eax);
            put(app, cpu.esi, 0);
            cpu.eax = at(app, cpu.eax);
            put(app, cpu.edi + 4, cpu.eax);
        }
        safepoint(app, cpu);
        cpu.eax = cpu.edx * 4;
        cpu.edi = at(app, tails);
        cpu.esi = cpu.ebx + cpu.eax;
        cpu.eax += cpu.edi;
        put(app, cpu.esi, 0);
        ++cpu.edx;
        put(app, cpu.eax, 0);
        if (sreg32(cpu.edx) >= 6)
            break;
    }
    const reg32 opaque[][2] = {{0, kBucketOpaque}, {8, kBucketA}, {0x18, kBucketB}, {0x10, kBucketC}};
    for (const auto& bucket : opaque)
    {
        cpu.eax = at(app, cpu.ecx + bucket[0]);
        push(app, cpu, at(app, bucket[1]));
        if (!byAddress(app, cpu, 0x4c7790))
            return;
    }
    cpu.eax = cpu.ecx + 0x28;
    if (!direct(app, cpu, bracketList))
        return;
    cpu.eax = at(app, cpu.ecx + 0x2c);
    push(app, cpu, at(app, kBucketOrdered));
    push(app, cpu, cpu.eax);
    cpu.edx = at(app, cpu.ecx + 0x28);
    push(app, cpu, cpu.edx);
    if (!byAddress(app, cpu, 0x4c7940))
        return;
    cpu.ebx = at(app, cpu.ecx + 0x24);
    push(app, cpu, at(app, kBucketOrdered));
    push(app, cpu, cpu.ebx);
    cpu.esi = at(app, cpu.ecx + 0x20);
    push(app, cpu, cpu.esi);
    if (!byAddress(app, cpu, 0x4c7940))
        return;
    cpu.esi = cpu.ebp - 0x4c;
    cpu.eax = 0;
    cpu.edi = cpu.ecx;
    if (!byAddress(app, cpu, 0x4c79a0))
        return;
    cpu.eax = 0;
    cpu.esi = cpu.ebp - 0x44;
    if (!byAddress(app, cpu, 0x4c7750) || !byAddress(app, cpu, 0x4bbe70))
        return;
    cpu.esi = cpu.ebp - 0x44;
    movsd(app, cpu);
    movsd(app, cpu);
    // The other buckets emptied: their frame's empty list copied in.
    const reg32 empties[][2] = {{0x3c, 8}, {0x14, 0x18}, {0x2c, 0x10}, {0x24, 0x20}};
    for (const auto& bucket : empties)
    {
        cpu.esi = cpu.ebp - bucket[0];
        if (!byAddress(app, cpu, 0x4bbe70))
            return;
        cpu.esi = cpu.ebp - bucket[0];
        cpu.edi = cpu.ecx + bucket[1];
        movsd(app, cpu);
        movsd(app, cpu);
    }
    cpu.esi = cpu.ebp - 0x1c;
    cpu.edx = cpu.ebp - 0x4c;
    if (!byAddress(app, cpu, 0x4bbe70))
        return;
    cpu.esi = cpu.ebp - 0x1c;
    cpu.edi = cpu.ecx + 0x28;
    push(app, cpu, 0);
    cpu.eax = at(app, view);
    movsd(app, cpu);
    movsd(app, cpu);
    if (!byAddress(app, cpu, 0x4bc060))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41c310 (stdcall, 3 words): the headlights' light on the eax points at
 * ecx (12 bytes each) of an object in the camera [esp+8]'s view: for each car
 * near enough (its place within the square root of 0x536cb4 of the point at
 * ebx; not a car that is hidden, wrecked or without lights), each of its two
 * lights (+0x734 with its box at +0x75c, where +0x200 has bit 0x20; +0x660
 * with its box at +0x688) that faces the points and whose cone reaches them:
 * each point taken through the matrix edx (0x4e0430), lit as a track vertex
 * (0x41a550) and the vertex [esp+4] + 0x20 * n raised to the lit colour.  A
 * box or a point of 0 is the game's own (the code's data at 0x41a880 and
 * 0x41a8a4).  None in the mirror's pass, nor outside a split-screen race --
 * the port's change (tools/apply_mirror_detail.py): the mirror's asked
 * through nfs3hp::mirrorAsMain, as the main view would have it. */
void headlightsOnObjects(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xa0;
    cpu.ebp -= 0x76;
    const reg32 e = cpu.ebp;
    const reg32 count = e + 0x66, matrix = e + 0x6e, point = e + 0x72, points = e + 0x62, car = e + 0x6a;
    const reg32 vertices = e + 0x86, camera = e + 0x8a, view = e + 0x8e;
    const reg32 ownMatrix = e - 0x2a, ownPoint = e + 0x56, carPlace = e + 0x32;
    // Each light's lit vertex (0x41a550's record of 0x1c bytes) and the point
    // it is made from.
    const reg32 firstLit = e - 6, firstPoint = e + 0x3e, secondLit = e + 0x16, secondPoint = e + 0x4a;
    // mov edx, [V + 8]; mov al, [V + from]; mov X, [edx + to]; cmp al, X; ja;
    // mov al, X; mov edx, [V + 8]; mov [edx + to], al.
    const auto raise = [&](reg32 v, reg8& x, reg32 from, reg32 to) {
        cpu.edx = at(app, v + 8);
        cpu.al = byte(app, v + from);
        x = byte(app, cpu.edx + to);
        cpu.al = larger(cpu.al, x);
        cpu.edx = at(app, v + 8);
        putByte(app, cpu.edx + to, cpu.al);
    };
    // One light's points: each into view, lit, and its vertex raised.
    const auto lightPoints = [&](reg32 lightAt, reg32 lit, reg32 from, bool first) {
        for (;;)
        {
            safepoint(app, cpu);
            if (sreg32(cpu.edi) >= sreg32(at(app, count)))
                return true;
            cpu.eax = from;
            push(app, cpu, cpu.eax);
            cpu.eax = cpu.edi * 12;
            cpu.edx = at(app, points);
            cpu.ecx = at(app, point);
            cpu.ebx = at(app, matrix);
            cpu.edx += cpu.eax;
            cpu.eax = 1;
            if (!byAddress(app, cpu, 0x4e0430))
                return false;
            cpu.eax = cpu.edi << 5;
            cpu.edx = at(app, vertices);
            cpu.ecx = lit;
            cpu.eax += cpu.edx;
            cpu.ebx = from;
            push(app, cpu, cpu.eax);
            cpu.edx = 1;
            cpu.eax = cpu.esi + lightAt;
            if (!byAddress(app, cpu, 0x41a550))
                return false;
            if (first)
            {
                cpu.edx = at(app, lit + 4);
                if (cpu.dx == 0)
                {
                    raise(lit, cpu.bh, 0x16, 0x12);
                    raise(lit, cpu.cl, 0x15, 0x11);
                    raise(lit, cpu.ch, 0x14, 0x10);
                }
                if (half(app, lit + 6) == 0)
                {
                    raise(lit, cpu.ah, 0x1a, 0x12);
                    raise(lit, cpu.bl, 0x19, 0x11);
                    raise(lit, cpu.bh, 0x18, 0x10);
                }
            }
            else
            {
                cpu.ecx = at(app, lit + 4);
                if (cpu.cx == 0)
                {
                    raise(lit, cpu.cl, 0x16, 0x12);
                    raise(lit, cpu.ch, 0x15, 0x11);
                    raise(lit, cpu.ah, 0x14, 0x10);
                }
            }
            ++cpu.edi;
        }
    };
    put(app, count, cpu.eax);
    put(app, matrix, cpu.edx);
    put(app, point, cpu.ebx);
    put(app, points, cpu.ecx);
    cpu.ecx = 9;
    cpu.edi = ownMatrix;
    cpu.esi = kHeadlightsBox;
    for (; cpu.ecx != 0; --cpu.ecx)
        movsd(app, cpu);
    cpu.edi = ownPoint;
    cpu.esi = kHeadlightsPoint;
    cpu.eax = at(app, camera);
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    if (mirrorAsMain(at(app, cpu.eax)) != 1 && at(app, kSplitScreen) != 0)
    {
        if (at(app, matrix) == 0)
        {
            cpu.eax = ownMatrix;
            put(app, matrix, cpu.eax);
        }
        if (at(app, point) == 0)
        {
            cpu.eax = ownPoint;
            put(app, point, cpu.eax);
        }
        cpu.edi = 0;
        put(app, car, cpu.edi);
        for (;;)
        {
            safepoint(app, cpu);
            cpu.eax = at(app, car);
            if (sreg32(cpu.eax) >= sreg32(at(app, kCarCount)))
                break;
            cpu.esi = at(app, cpu.eax * 4 + kCars);
            cpu.ah = byte(app, cpu.esi + 0x200);
            if ((cpu.ah & 0x40) == 0 && byte(app, cpu.esi + 0x88) == 0 && byte(app, cpu.esi + 0x89) != 0)
            {
                cpu.eax = at(app, cpu.esi + 0x98);
                put(app, carPlace, cpu.eax);
                cpu.eax = at(app, cpu.esi + 0x9c);
                put(app, carPlace + 4, cpu.eax);
                cpu.eax = at(app, cpu.esi + 0xa0);
                put(app, carPlace + 8, cpu.eax);
                cpu.eax = at(app, point);
                const Float dz = fp::sub(f32(app, carPlace + 8), f32(app, cpu.eax + 8));
                const Float dx = fp::sub(f32(app, carPlace), f32(app, cpu.eax));
                const Float d = fp::add(fp::mul(dz, dz), fp::mul(dx, dx));
                if (!compare(cpu, d, f64(app, 0x536cb4)).above())
                {
                    bool second = true;
                    if ((byte(app, cpu.esi + 0x200) & 0x20) != 0 && at(app, cpu.esi + 0x734) != 0)
                    {
                        cpu.edi = cpu.esi + 0x734;
                        cpu.edx = at(app, camera);
                        cpu.eax = cpu.edi;
                        if (!direct(app, cpu, pointBehindView))
                            return;
                        if (cpu.eax == 0)
                        {
                            cpu.edx = at(app, view);
                            cpu.ecx = cpu.esi + 0x78c;
                            cpu.eax = at(app, camera);
                            cpu.ebx = cpu.esi + 0x75c;
                            cpu.edx += 0x84;
                            if (!direct(app, cpu, litBoxOutside))
                                return;
                            if (cpu.eax == 0)
                            {
                                cpu.ebx = at(app, camera);
                                push(app, cpu, cpu.ebx);
                                cpu.eax = cpu.edi;
                                push(app, cpu, 0x3f800000);
                                cpu.edi = 0;
                                if (!direct(app, cpu, setShadeDistances) || !lightPoints(0x734, firstLit, firstPoint, true))
                                    return;
                            }
                        }
                    }
                    if (second && at(app, cpu.esi + 0x660) != 0)
                    {
                        cpu.edi = cpu.esi + 0x660;
                        cpu.edx = at(app, camera);
                        cpu.eax = cpu.edi;
                        if (!direct(app, cpu, pointBehindView))
                            return;
                        if (cpu.eax == 0)
                        {
                            cpu.edx = at(app, view);
                            cpu.ecx = cpu.esi + 0x6b8;
                            cpu.eax = at(app, camera);
                            cpu.ebx = cpu.esi + 0x688;
                            cpu.edx += 0x84;
                            if (!byAddress(app, cpu, 0x41bf30))
                                return;
                            if (cpu.eax == 0)
                            {
                                cpu.ebx = at(app, camera);
                                push(app, cpu, cpu.ebx);
                                cpu.eax = cpu.edi;
                                push(app, cpu, 0x3f800000);
                                cpu.edi = 0;
                                if (!direct(app, cpu, setShadeDistances)
                                    || !lightPoints(0x660, secondLit, secondPoint, false))
                                    return;
                            }
                        }
                    }
                }
            }
            put(app, car, at(app, car) + 1);
        }
    }
    cpu.esp = e + 0x76;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 0xc;
}

/* 0x41c670 (stdcall, 4 words): the headlights' light on block ebx's road:
 * room for edx lit vertices taken from Render_GetTm; for each car near the
 * block's centre (within the square root of 0x536cbc; not hidden, wrecked or
 * without lights) and each of its lights facing the camera eax whose cone
 * reaches the block: the block's vertices (+0x70, ecx of them) lit for it
 * (setShadeDistances, 0x41a550) -- the second light dimmer for each car
 * before it -- and its polygon groups (the first light's raising the lists'
 * vertices, lightObjectPolygons; the second's drawn, 0x41ae90) for the
 * models [esp+0x10] and +1, 7, 8, 9 (and 6 for model 4).  The polygons drawn;
 * 0 in the mirror's pass, nor outside a split-screen race. */
void headlightsOnBlock(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x40;
    const reg32 vertexCount = cpu.ebp - 0x28, pointCount = cpu.ebp - 0x2c, drawn = cpu.ebp - 0xc;
    const reg32 lit = cpu.ebp - 0x30, car = cpu.ebp - 0x20, lightsSeen = cpu.ebp - 0x24, carAt = cpu.ebp - 8;
    const reg32 carPlace = cpu.ebp - 0x40, firstLight = cpu.ebp - 0x14, secondLight = cpu.ebp - 0x10;
    const reg32 dim = cpu.ebp - 0x34, order = cpu.ebp - 4, aheadHeads = cpu.ebp - 0x1c, aheadTails = cpu.ebp - 0x18;
    const reg32 heads = cpu.ebp + 0x10, tails = cpu.ebp + 0x14, view = cpu.ebp + 0x18, model = cpu.ebp + 0x1c;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.esp += 4 + 0x10;
    };
    cpu.esi = cpu.eax;
    put(app, vertexCount, cpu.edx);
    cpu.edi = cpu.ebx;
    put(app, pointCount, cpu.ecx);
    cpu.edx = 0;
    cpu.ecx = at(app, cpu.eax);
    put(app, drawn, cpu.edx);
    if (cpu.ecx == 1)
    {
        cpu.eax = 0;
        return leave();
    }
    if (at(app, kSplitScreen) != 0)
    {
        cpu.eax = at(app, vertexCount) << 2;
        cpu.edx = cpu.eax;
        cpu.eax = (cpu.eax << 3) - cpu.edx;
        cpu.edx = cpu.eax;
        cpu.eax = lit;
        if (!byAddress(app, cpu, 0x4bbde0))
            return;
        cpu.eax = at(app, lit);
        if (cpu.eax != 0)
        {
            cpu.edx = 0;
            put(app, car, cpu.edx);
            put(app, lightsSeen, cpu.edx);
            for (;;)
            {
                safepoint(app, cpu);
                cpu.eax = at(app, car);
                if (sreg32(cpu.eax) >= sreg32(at(app, kCarCount)))
                    break;
                cpu.eax = at(app, cpu.eax * 4 + kCars);
                put(app, carAt, cpu.eax);
                cpu.dl = byte(app, cpu.eax + 0x200);
                if ((cpu.dl & 0x40) == 0 && byte(app, cpu.eax + 0x88) == 0 && byte(app, cpu.eax + 0x89) != 0)
                {
                    cpu.edx = at(app, at(app, carAt) + 0x98);
                    cpu.eax = cpu.edi * 4;
                    put(app, carPlace, cpu.edx);
                    cpu.edx = at(app, at(app, carAt) + 0x9c);
                    cpu.eax -= cpu.edi;
                    cpu.eax <<= 3;
                    put(app, carPlace + 4, cpu.edx);
                    cpu.edx = at(app, at(app, carAt) + 0xa0);
                    cpu.eax -= cpu.edi;
                    cpu.eax <<= 6;
                    put(app, carPlace + 8, cpu.edx);
                    cpu.eax += kBlocks;
                    const Float dz = fp::sub(f32(app, carPlace + 8), f32(app, cpu.eax + 0x80));
                    const Float dx = fp::sub(f32(app, carPlace), f32(app, cpu.eax + 0x78));
                    const Float d = fp::add(fp::mul(dz, dz), fp::mul(dx, dx));
                    if (!compare(cpu, d, f64(app, 0x536cbc)).above())
                    {
                        cpu.eax = at(app, carAt);
                        if ((byte(app, cpu.eax + 0x200) & 0x20) != 0 && at(app, cpu.eax + 0x734) != 0)
                        {
                            cpu.eax += 0x734;
                            cpu.edx = cpu.esi;
                            put(app, firstLight, cpu.eax);
                            if (!direct(app, cpu, pointBehindView))
                                return;
                            if (cpu.eax == 0)
                            {
                                cpu.ecx = at(app, carAt);
                                cpu.ebx = at(app, carAt);
                                cpu.edx = at(app, view);
                                cpu.eax = cpu.esi;
                                cpu.ecx += 0x78c;
                                cpu.ebx += 0x75c;
                                cpu.edx += 0x84;
                                if (!direct(app, cpu, litBoxOutside))
                                    return;
                            }
                            if (cpu.eax == 0)
                            {
                                cpu.eax = at(app, firstLight);
                                push(app, cpu, cpu.esi);
                                cpu.ecx = at(app, pointCount);
                                cpu.ebx = at(app, view);
                                push(app, cpu, 0x3f800000);
                                cpu.edx = at(app, vertexCount);
                                if (!direct(app, cpu, setShadeDistances))
                                    return;
                                cpu.eax = at(app, firstLight);
                                push(app, cpu, cpu.ecx);
                                cpu.ebx = at(app, cpu.ebx + 0x70);
                                cpu.ecx = at(app, lit);
                                if (!byAddress(app, cpu, 0x41a550))
                                    return;
                                // The lists' vertices raised: models +0, +1, 7, 8, 9.
                                cpu.ebx = at(app, lit);
                                cpu.ecx = at(app, model);
                                cpu.edx = at(app, firstLight);
                                push(app, cpu, cpu.ebx);
                                cpu.eax = cpu.esi;
                                cpu.ebx = cpu.edi;
                                if (!direct(app, cpu, lightObjectPolygons))
                                    return;
                                cpu.eax = at(app, lit);
                                cpu.ecx = at(app, model);
                                cpu.edx = at(app, firstLight);
                                cpu.ebx = cpu.edi;
                                push(app, cpu, cpu.eax);
                                ++cpu.ecx;
                                cpu.eax = cpu.esi;
                                if (!direct(app, cpu, lightObjectPolygons))
                                    return;
                                cpu.edx = at(app, lit);
                                cpu.ecx = 7;
                                cpu.ebx = cpu.edi;
                                push(app, cpu, cpu.edx);
                                cpu.eax = cpu.esi;
                                cpu.edx = at(app, firstLight);
                                if (!direct(app, cpu, lightObjectPolygons))
                                    return;
                                cpu.ecx = at(app, lit);
                                cpu.edx = at(app, firstLight);
                                cpu.ebx = cpu.edi;
                                push(app, cpu, cpu.ecx);
                                cpu.eax = cpu.esi;
                                cpu.ecx = 8;
                                if (!direct(app, cpu, lightObjectPolygons))
                                    return;
                                cpu.ebx = at(app, lit);
                                cpu.ecx = 9;
                                cpu.edx = at(app, firstLight);
                                push(app, cpu, cpu.ebx);
                                cpu.eax = cpu.esi;
                                cpu.ebx = cpu.edi;
                                if (!direct(app, cpu, lightObjectPolygons))
                                    return;
                            }
                        }
                        cpu.eax = at(app, carAt);
                        if (at(app, cpu.eax + 0x660) != 0)
                        {
                            cpu.eax += 0x660;
                            cpu.edx = cpu.esi;
                            put(app, secondLight, cpu.eax);
                            if (!direct(app, cpu, pointBehindView))
                                return;
                            if (cpu.eax == 0)
                            {
                                cpu.ecx = at(app, carAt);
                                cpu.ebx = at(app, carAt);
                                cpu.edx = at(app, view);
                                cpu.eax = cpu.esi;
                                cpu.ecx += 0x6b8;
                                cpu.ebx += 0x688;
                                cpu.edx += 0x84;
                                if (!byAddress(app, cpu, 0x41bf30))
                                    return;
                            }
                            if (cpu.eax == 0)
                            {
                                cpu.eax = at(app, lightsSeen);
                                put(app, order, cpu.eax);
                                const Float share = fp::div(Float(sreg32(at(app, order))), Float(sreg32(at(app, kCarCount))));
                                const Float scaled = fp::mul(share, f32(app, 0x536cc4));
                                cpu.ecx = at(app, pointCount);
                                cpu.ebx = at(app, view);
                                push(app, cpu, cpu.esi);
                                const Float dimmed = fp::sub(Float(1.0), scaled);
                                cpu.eax = at(app, secondLight);
                                putF32(app, dim, dimmed);
                                push(app, cpu, at(app, dim));
                                cpu.edx = at(app, vertexCount);
                                if (!direct(app, cpu, setShadeDistances))
                                    return;
                                cpu.eax = at(app, secondLight);
                                push(app, cpu, cpu.ecx);
                                cpu.ebx = at(app, cpu.ebx + 0x70);
                                cpu.ecx = at(app, lit);
                                if (!byAddress(app, cpu, 0x41a550))
                                    return;
                                // The polygon groups drawn lit: models +0, +1, 7, 8, 9.
                                cpu.eax = at(app, tails) + 0x14;
                                put(app, aheadHeads, cpu.eax);
                                push(app, cpu, cpu.eax);
                                cpu.eax = at(app, heads);
                                cpu.edx = at(app, lit);
                                cpu.eax += 0x14;
                                cpu.ecx = at(app, model);
                                push(app, cpu, cpu.eax);
                                cpu.ebx = cpu.edi;
                                put(app, aheadTails, cpu.eax);
                                push(app, cpu, cpu.edx);
                                cpu.eax = cpu.esi;
                                cpu.edx = at(app, secondLight);
                                if (!byAddress(app, cpu, 0x41ae90))
                                    return;
                                for (const reg32 next : {0u, 7u, 8u})
                                {
                                    cpu.ecx = at(app, drawn);
                                    cpu.ebx = at(app, aheadHeads);
                                    cpu.edx = at(app, lit);
                                    push(app, cpu, cpu.ebx);
                                    cpu.ecx += cpu.eax;
                                    cpu.eax = at(app, aheadTails);
                                    put(app, drawn, cpu.ecx);
                                    push(app, cpu, cpu.eax);
                                    cpu.ebx = cpu.edi;
                                    if (next == 0)
                                    {
                                        cpu.ecx = at(app, model);
                                        push(app, cpu, cpu.edx);
                                        ++cpu.ecx;
                                    }
                                    else
                                    {
                                        cpu.ecx = next;
                                        push(app, cpu, cpu.edx);
                                    }
                                    cpu.eax = cpu.esi;
                                    cpu.edx = at(app, secondLight);
                                    if (!byAddress(app, cpu, 0x41ae90))
                                        return;
                                }
                                put(app, drawn, at(app, drawn) + cpu.eax);
                                cpu.ebx = at(app, aheadHeads);
                                push(app, cpu, cpu.ebx);
                                cpu.eax = at(app, aheadTails);
                                cpu.edx = at(app, lit);
                                push(app, cpu, cpu.eax);
                                cpu.ecx = 9;
                                cpu.ebx = cpu.edi;
                                push(app, cpu, cpu.edx);
                                cpu.eax = cpu.esi;
                                cpu.edx = at(app, secondLight);
                                if (!byAddress(app, cpu, 0x41ae90))
                                    return;
                                cpu.ecx = at(app, drawn);
                                cpu.ecx += cpu.eax;
                                cpu.ebx = at(app, model);
                                put(app, drawn, cpu.ecx);
                                if (cpu.ebx == 4)
                                {
                                    cpu.eax = at(app, aheadHeads);
                                    push(app, cpu, cpu.eax);
                                    cpu.edx = at(app, aheadTails);
                                    cpu.ecx = at(app, lit);
                                    push(app, cpu, cpu.edx);
                                    cpu.ebx = cpu.edi;
                                    cpu.eax = cpu.esi;
                                    push(app, cpu, cpu.ecx);
                                    cpu.edx = at(app, secondLight);
                                    cpu.ecx = 6;
                                    if (!byAddress(app, cpu, 0x41ae90))
                                        return;
                                    put(app, drawn, at(app, drawn) + cpu.eax);
                                }
                                put(app, lightsSeen, at(app, lightsSeen) + 1);
                            }
                        }
                    }
                }
                put(app, car, at(app, car) + 1);
            }
        }
    }
    cpu.eax = at(app, drawn);
    leave();
}

/* 0x41c9f0: block edx's lights (+0x5b8 of them at +0x5bc, 0x10 bytes each:
 * three words and a half-word) glowing for the camera eax (0x491190). */
void blockLights(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.eax = blockOffset(cpu.edx);
    cpu.ebx = at(app, cpu.eax + kBlocks + 0x5b8);
    cpu.edx = at(app, cpu.eax + kBlocks + 0x5bc);
    for (;;)
    {
        safepoint(app, cpu);
        --cpu.ebx;
        if (cpu.ebx == 0xffffffff)
            break;
        cpu.eax = half(app, cpu.edx + 0xc);
        push(app, cpu, cpu.eax);
        cpu.esi = at(app, cpu.edx + 8);
        push(app, cpu, cpu.esi);
        cpu.edi = at(app, cpu.edx + 4);
        push(app, cpu, cpu.edi);
        cpu.eax = at(app, cpu.edx);
        push(app, cpu, cpu.eax);
        cpu.eax = cpu.ecx;
        cpu.edx += 0x10;
        if (!byAddress(app, cpu, 0x491190))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41ca40 (stdcall, 3 words): the objects of the list [esp+8] (block ecx's,
 * or the track's) drawn for the camera eax, into the ordered list whose last
 * record is at [edx] (its first at [ebx] while it is empty) -- an animated
 * one (kind 4) at its state's place and turn (the states at 0x5dd230: 0xff
 * still, 0xfe turning, else a path's step, 0x40f940 .. 0x41d510), a car's
 * (kind 3, where [esp+0xc]) moved and turned as the car is and drawn only
 * where its block is in view, any other where it stands: its vertices into
 * view and lit (headlightsOnObjects), its polygons as records (0x41b640)
 * appended to the list.  The polygons drawn; [edx] the list's last record. */
void drawObjectList(App* app, CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x68;
    const reg32 lastAt = cpu.ebp - 0x18, firstAt = cpu.ebp - 0x20, blockIndex = cpu.ebp - 0x1c;
    const reg32 block = cpu.ebp - 8, last = cpu.ebp - 0x38, drawn = cpu.ebp - 0x14, left = cpu.ebp - 0x10;
    const reg32 vertices = cpu.ebp - 0x30, records = cpu.ebp - 0x34, state = cpu.ebp - 4, place = cpu.ebp - 0xc;
    const reg32 turn = cpu.ebp - 0x68, offset = cpu.ebp - 0x44, steps = cpu.ebp - 0x2c, length = cpu.ebp - 0x28;
    const reg32 height = cpu.ebp - 0x24;
    const reg32 scale = cpu.ebp + 0x10, list = cpu.ebp + 0x14, cars = cpu.ebp + 0x18;
    cpu.edi = cpu.eax;
    put(app, lastAt, cpu.edx);
    put(app, firstAt, cpu.ebx);
    put(app, blockIndex, cpu.ecx);
    cpu.ecx = at(app, list);
    cpu.edx = at(app, blockIndex);
    cpu.eax = blockOffset(cpu.edx);
    cpu.edx = kBlocks + cpu.eax;
    cpu.eax = at(app, lastAt);
    put(app, block, cpu.edx);
    cpu.eax = at(app, cpu.eax);
    cpu.edx = 0;
    put(app, last, cpu.eax);
    put(app, drawn, cpu.edx);
    cpu.eax = at(app, cpu.ecx);
    cpu.esi = at(app, cpu.ecx + 4);
    put(app, left, cpu.eax);
    // The object drawn: its vertices into view and lit by the headlights,
    // with the registers each way to it leaves.
    const auto light = [&] {
        return direct(app, cpu, headlightsOnObjects);
    };
    // mov eax, [state]; mov dh, [eax]; cmp dh, 0xfe.
    const auto turning = [&] {
        cpu.eax = at(app, state);
        cpu.dh = byte(app, cpu.eax);
        return cpu.dh == 0xfe;
    };
    for (;;)
    {
        safepoint(app, cpu);
        cpu.ecx = at(app, left) - 1;
        put(app, left, cpu.ecx);
        if (cpu.ecx == 0xffffffff)
            break;
        cpu.eax = at(app, cpu.esi);
        if (cpu.eax == 4)
        {
            // Animated: room for its vertices, its state.
            cpu.edx = at(app, cpu.esi + 0x20) << 5;
            cpu.eax = vertices;
            if (!byAddress(app, cpu, 0x4bbde0))
                return;
            cpu.edx = at(app, blockIndex);
            cpu.eax = blockOffset(cpu.edx);
            cpu.edx = at(app, cpu.esi + 4);
            cpu.ecx = at(app, cpu.eax + kBlocks + 0x5a4);
            cpu.eax = cpu.edx * 5;
            cpu.edx = half(app, cpu.ecx + cpu.eax * 4 + 0xe);
            cpu.eax = cpu.edx * 3;
            cpu.edx = kObjectStates + cpu.eax;
            cpu.eax = cpu.esi + 0xc;
            put(app, state, cpu.edx);
            put(app, place, cpu.eax);
            cpu.dl = byte(app, cpu.edx);
            if (cpu.dl == 0xff)
            {
                // Still: where it stands.
                cpu.edx = at(app, vertices);
                cpu.ebx = cpu.eax;
                push(app, cpu, cpu.edx);
                cpu.ecx = at(app, cpu.esi + 0x28);
                cpu.eax = cpu.edi;
                push(app, cpu, cpu.ecx);
                cpu.edx = at(app, cpu.esi + 0x20);
                cpu.ecx = at(app, cpu.esi + 0x24);
                if (!direct(app, cpu, viewObjectVertices))
                    return;
                cpu.ebx = at(app, block);
                push(app, cpu, cpu.ebx);
                cpu.eax = at(app, vertices);
                push(app, cpu, cpu.edi);
                cpu.ecx = at(app, cpu.esi + 0x24);
                cpu.edx = 0;
                push(app, cpu, cpu.eax);
                cpu.ebx = at(app, place);
                cpu.eax = at(app, cpu.esi + 0x20);
                if (!light())
                    return;
            }
            else if (turning())
            {
                // Turning: by the state's two angles.
                cpu.edx = at(app, state);
                cpu.al = byte(app, cpu.esi + 8);
                cpu.ebx = turn;
                putByte(app, cpu.edx + 2, cpu.al);
                cpu.eax = at(app, state);
                cpu.edx = reg32(sreg32(std::int8_t(byte(app, cpu.edx + 1))));
                cpu.eax = reg32(sreg32(std::int8_t(byte(app, cpu.eax + 2))));
                cpu.edx <<= 8;
                cpu.eax <<= 8;
                if (!direct(app, cpu, turnFromPitchYaw))
                    return;
                cpu.edx = at(app, vertices);
                push(app, cpu, cpu.edx);
                cpu.ecx = at(app, cpu.esi + 0x28);
                cpu.eax = cpu.edi;
                push(app, cpu, cpu.ecx);
                cpu.ebx = at(app, cpu.esi + 0x24);
                cpu.edx = turn;
                push(app, cpu, cpu.ebx);
                cpu.ecx = at(app, place);
                cpu.ebx = at(app, cpu.esi + 0x20);
                if (!direct(app, cpu, viewTurnedObjectVertices))
                    return;
                cpu.eax = at(app, block);
                push(app, cpu, cpu.eax);
                cpu.edx = at(app, vertices);
                push(app, cpu, cpu.edi);
                cpu.ebx = at(app, place);
                cpu.ecx = at(app, cpu.esi + 0x24);
                push(app, cpu, cpu.edx);
                cpu.eax = at(app, cpu.esi + 0x20);
                cpu.edx = turn;
                if (!light())
                    return;
            }
            else
            {
                // On a path: its step's place and turn, or none -- then still
                // from now on (0xfe), not drawn.
                cpu.ebx = turn;
                cpu.eax = reg32(sreg32(std::int8_t(cpu.dh)));
                cpu.edx = offset;
                if (!byAddress(app, cpu, 0x40f940))
                    return;
                if (cpu.eax != 1)
                {
                    cpu.eax = at(app, state);
                    cpu.eax = reg32(sreg32(std::int8_t(byte(app, cpu.eax))));
                    if (!byAddress(app, cpu, 0x40f460))
                        return;
                    cpu.eax = at(app, state);
                    putByte(app, cpu.eax, 0xfe);
                    continue;
                }
                cpu.edx = at(app, state);
                cpu.ecx = offset;
                cpu.al = byte(app, cpu.esi + 8);
                cpu.ebx = at(app, place);
                putByte(app, cpu.edx + 2, cpu.al);
                cpu.edx = offset;
                cpu.eax = 1;
                if (!byAddress(app, cpu, 0x4dffb0))
                    return;
                cpu.eax = at(app, state);
                cpu.ebx = length;
                cpu.edx = steps;
                cpu.eax = reg32(sreg32(std::int8_t(byte(app, cpu.eax))));
                if (!byAddress(app, cpu, 0x40f800))
                    return;
                cpu.eax = at(app, state);
                cpu.edx = at(app, steps);
                cpu.eax = reg32(sreg32(std::int8_t(byte(app, cpu.eax + 2))));
                cpu.ecx = at(app, length);
                cpu.eax <<= 8;
                cpu.edx <<= 0x10;
                put(app, height, cpu.eax);
                cpu.eax = cpu.edx;
                idiv(cpu, cpu.ecx);
                cpu.ebx = at(app, state);
                cpu.ebx = reg32(sreg32(std::int8_t(byte(app, cpu.ebx + 1))));
                cpu.ebx <<= 8;
                cpu.ecx = turn;
                cpu.edx = at(app, height);
                if (!direct(app, cpu, turnFromAngles))
                    return;
                cpu.ebx = at(app, vertices);
                push(app, cpu, cpu.ebx);
                cpu.eax = at(app, cpu.esi + 0x28);
                cpu.ecx = offset;
                push(app, cpu, cpu.eax);
                cpu.edx = at(app, cpu.esi + 0x24);
                cpu.ebx = at(app, cpu.esi + 0x20);
                push(app, cpu, cpu.edx);
                cpu.eax = cpu.edi;
                cpu.edx = turn;
                if (!direct(app, cpu, viewTurnedObjectVertices))
                    return;
                cpu.ecx = at(app, block);
                push(app, cpu, cpu.ecx);
                cpu.ebx = at(app, vertices);
                push(app, cpu, cpu.edi);
                cpu.edx = turn;
                cpu.eax = at(app, cpu.esi + 0x20);
                push(app, cpu, cpu.ebx);
                cpu.ecx = at(app, cpu.esi + 0x24);
                cpu.ebx = offset;
                if (!light())
                    return;
            }
        }
        else if (cpu.eax == 3)
        {
            // A car's: only where cars are drawn, as the car stands.
            if (at(app, cars) == 0)
            {
                cpu.esi += 0x34;
                continue;
            }
            cpu.eax = turn;
            cpu.ecx = offset;
            cpu.edx = 1;
            push(app, cpu, cpu.eax);
            cpu.ebx = at(app, 0x7d3684);
            cpu.eax = at(app, cpu.esi + 0x1c);
            if (!byAddress(app, cpu, 0x40f4b0))
                return;
            cpu.edx = at(app, cpu.esi + 0x20) << 5;
            cpu.eax = vertices;
            if (!byAddress(app, cpu, 0x4bbde0))
                return;
            cpu.ecx = at(app, vertices);
            push(app, cpu, cpu.ecx);
            cpu.ebx = at(app, cpu.esi + 0x28);
            cpu.edx = turn;
            push(app, cpu, cpu.ebx);
            cpu.eax = at(app, cpu.esi + 0x24);
            cpu.ecx = offset;
            push(app, cpu, cpu.eax);
            cpu.ebx = at(app, cpu.esi + 0x20);
            cpu.eax = cpu.edi;
            if (!direct(app, cpu, viewTurnedObjectVertices))
                return;
            cpu.edx = at(app, block);
            push(app, cpu, cpu.edx);
            cpu.ecx = at(app, vertices);
            push(app, cpu, cpu.edi);
            cpu.ebx = offset;
            cpu.eax = at(app, cpu.esi + 0x20);
            push(app, cpu, cpu.ecx);
            cpu.edx = turn;
            cpu.ecx = at(app, cpu.esi + 0x24);
            if (!light())
                return;
            if (!compare(cpu, f32(app, scale), f32(app, 0x536cc8)).equal)
            {
                cpu.eax = offset;
                if (!byAddress(app, cpu, 0x41e0f0))
                    return;
                cpu.edx = reg32(sreg32(cpu.eax) >> 3);
                cpu.eax = cpu.edi;
                if (!direct(app, cpu, blockInView))
                    return;
                if (cpu.eax == 0)
                {
                    cpu.esi += 0x34;
                    continue;
                }
            }
        }
        else
        {
            // Fixed: where it stands, coloured.
            cpu.edx = at(app, cpu.esi + 0x20) << 5;
            cpu.eax = vertices;
            if (!byAddress(app, cpu, 0x4bbde0))
                return;
            cpu.ebx = at(app, vertices);
            cpu.eax = cpu.edi;
            cpu.ecx = at(app, cpu.esi + 0x28);
            push(app, cpu, cpu.ebx);
            cpu.edx = at(app, cpu.esi + 0x20);
            cpu.ebx = at(app, cpu.esi + 0x24);
            if (!byAddress(app, cpu, 0x41a970))
                return;
            cpu.eax = at(app, block);
            push(app, cpu, cpu.eax);
            cpu.edx = at(app, vertices);
            push(app, cpu, cpu.edi);
            cpu.ecx = at(app, cpu.esi + 0x24);
            cpu.ebx = 0;
            push(app, cpu, cpu.edx);
            cpu.eax = at(app, cpu.esi + 0x20);
            cpu.edx = 0;
            if (!light())
                return;
        }
        // Its polygons as records, appended to the list.
        cpu.edx = at(app, cpu.esi + 0x2c) << 5;
        cpu.eax = records;
        if (!byAddress(app, cpu, 0x4bbe40))
            return;
        if (cpu.eax == 0)
        {
            cpu.esi += 0x34;
            continue;
        }
        cpu.ecx = at(app, vertices);
        cpu.eax = cpu.edi;
        cpu.ebx = at(app, cpu.esi + 0x30);
        push(app, cpu, cpu.ecx);
        cpu.edx = at(app, cpu.esi + 0x2c);
        cpu.ecx = at(app, records);
        if (!byAddress(app, cpu, 0x41b640))
            return;
        if (cpu.eax != 0)
        {
            cpu.edx = cpu.eax << 5;
            cpu.eax = records;
            if (!byAddress(app, cpu, 0x4bbde0))
                return;
            cpu.ebx = at(app, last);
            if (cpu.ebx == 0)
            {
                cpu.edx = at(app, firstAt);
                cpu.eax = at(app, records);
                put(app, cpu.edx, cpu.eax);
            }
            else
            {
                cpu.eax = at(app, records);
                put(app, cpu.ebx, cpu.eax);
            }
            cpu.edx = 0x20;
            cpu.eax = last;
            if (!byAddress(app, cpu, 0x4bbe40))
                return;
            put(app, last, at(app, last) - 0x20);
        }
        cpu.edx = at(app, drawn) + at(app, cpu.esi + 0x2c);
        cpu.esi += 0x34;
        put(app, drawn, cpu.edx);
    }
    cpu.edx = at(app, lastAt);
    cpu.eax = at(app, last);
    put(app, cpu.edx, cpu.eax);
    cpu.eax = at(app, drawn);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 0xc;
}

/* 0x41cdc0: the track drawn for the camera eax, its polygons into the
 * buckets at ebx: each block of the list at edx (the view's 300 blocks, -1
 * none) within the view's reach and in view -- its model by distance (its
 * own, or the next block's for a block of the list's second half; the
 * mirror's always the coarsest, the port's change through
 * nfs3hp::mirrorAsMain), its vertices into view (0x41a970), its polygon
 * groups (0x41b300, models 0..9 as the model's record at 0x552e20 says, a
 * tenth outside split screen and network races), the headlights on it
 * (headlightsOnBlock), its lights (blockLights) and its object lists
 * (drawObjectList) -- the buckets flushed (flushBuckets) whenever the
 * polygons pass the model's most.  Then the track's own objects, where there
 * are lights ([0x5dd840]); the six lists into the buckets; two counters
 * handed on (0x431c20). */
void drawTrack(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x70;
    const reg32 list = cpu.ebp - 0x2c, buckets = cpu.ebp - 0x28, drawn = cpu.ebp - 4, entry = cpu.ebp - 0x14;
    const reg32 first = cpu.ebp - 0x34, second = cpu.ebp - 0x30, at_ = cpu.ebp - 0x24, distance = cpu.ebp - 0x20;
    const reg32 modelAt = cpu.ebp - 0x1c, model = cpu.ebp - 0x10, block = cpu.ebp - 0xc, groups = cpu.ebp - 0x18;
    const reg32 vertices = cpu.ebp - 0x38, blockAt = cpu.ebp - 8;
    // The six lists' heads and tails.
    const reg32 heads = cpu.ebp - 0x70, tails = cpu.ebp - 0x58;
    // The polygon groups of model ebx into lists `head` and `tail`: the
    // polygons drawn added up.
    const auto groupsOf = [&](reg32 head, reg32 tail) {
        cpu.eax = head;
        push(app, cpu, cpu.eax);
        cpu.eax = tail;
        push(app, cpu, cpu.eax);
        cpu.edx = cpu.edi;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x41b300))
            return false;
        return true;
    };
    cpu.esi = cpu.eax;
    put(app, list, cpu.edx);
    put(app, buckets, cpu.ebx);
    cpu.edx = 0;
    cpu.eax = 0;
    put(app, drawn, cpu.edx);
    ++cpu.eax;
    cpu.ecx = 0;
    put(app, cpu.ebp + cpu.eax * 4 - 0x74, cpu.ecx);
    put(app, cpu.ebp + cpu.eax * 4 - 0x5c, cpu.ecx);
    while (sreg32(cpu.eax) < 6)
    {
        safepoint(app, cpu);
        ++cpu.eax;
        cpu.ecx = 0;
        put(app, cpu.ebp + cpu.eax * 4 - 0x74, cpu.ecx);
        put(app, cpu.ebp + cpu.eax * 4 - 0x5c, cpu.ecx);
    }
    cpu.ebx = 0;
    put(app, first, cpu.ebx);
    put(app, entry, cpu.ebx);
    put(app, second, cpu.ebx);
    for (;; put(app, entry, at(app, entry) + 1))
    {
        safepoint(app, cpu);
        cpu.eax = at(app, entry);
        if (sreg32(cpu.eax) > sreg32(at(app, kLastBlock)))
            break;
        cpu.edx = at(app, list);
        cpu.eax <<= 2;
        cpu.edx += cpu.eax;
        put(app, at_, cpu.edx);
        cpu.edi = reg32(sreg32(std::int16_t(half(app, cpu.edx))));
        if (cpu.edi == 0xffffffff)
            continue;
        // Within the view's reach.
        cpu.eax = blockOffset(cpu.edi);
        cpu.edx = kBlocks + cpu.eax;
        cpu.eax = cpu.edx + 0x78;
        const Float dz = fp::sub(f32(app, cpu.esi + 0x10), f32(app, cpu.eax + 8));
        const Float dx = fp::sub(f32(app, cpu.esi + 8), f32(app, cpu.eax));
        cpu.eax = at(app, kView);
        const Float d = fp::add(fp::mul(dz, dz), fp::mul(dx, dx));
        const Float reach = fp::mul(f32(app, cpu.eax + 0x34), f32(app, cpu.eax + 0x34));
        putF32(app, distance, d);
        if (compare(cpu, reach, f32(app, distance)).below)
            continue;
        // In view.
        cpu.ecx = cpu.esi + 0x38;
        cpu.ebx = cpu.esi + 0x44;
        cpu.edx += 0x84;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x41bf30))
            return;
        if (cpu.eax != 0)
            continue;
        // Its model: by its own distance for the list's first half.
        cpu.eax = reg32(sreg32(at(app, at(app, at_))) >> 0x10);
        if (cpu.eax == 0xffffffff)
        {
            cpu.eax = cpu.esi;
            push(app, cpu, at(app, distance));
            if (!direct(app, cpu, modelByDistance))
                return;
        }
        // Port: the mirror's pass asked through mirrorAsMain.
        if (mirrorAsMain(at(app, cpu.esi)) == 1)
            cpu.eax = 0;
        cpu.edx = cpu.eax;
        cpu.eax = ((cpu.eax << 2) - cpu.edx) << 2;
        cpu.edx = kModels + cpu.eax;
        cpu.ebx = at(app, kLastBlock);
        put(app, modelAt, cpu.edx);
        cpu.eax = at(app, cpu.edx + 4);
        cpu.edx = cpu.edi + 1;
        put(app, model, cpu.eax);
        if (sreg32(cpu.edx) > sreg32(cpu.ebx))
            cpu.edx = 0;
        // The next block's distance, its model asked for and not taken.
        cpu.eax = blockOffset(cpu.edx) + kBlocks;
        const Float nz = fp::sub(f32(app, cpu.esi + 0x10), f32(app, cpu.eax + 0x80));
        const Float nx = fp::sub(f32(app, cpu.esi + 8), f32(app, cpu.eax + 0x78));
        cpu.esp -= 4;
        const Float next = fp::add(fp::mul(nz, nz), fp::mul(nx, nx));
        cpu.eax = cpu.esi;
        putF32(app, cpu.esp, next);
        if (!direct(app, cpu, modelByDistance))
            return;
        // Its vertices into view.
        cpu.eax = blockOffset(cpu.edi);
        cpu.edx = kBlocks + cpu.eax;
        cpu.eax = at(app, at(app, modelAt)) << 2;
        cpu.eax += cpu.edx;
        cpu.eax = at(app, cpu.eax + 0x5c);
        put(app, block, cpu.edx);
        put(app, groups, cpu.eax);
        cpu.edx = cpu.eax;
        cpu.eax = vertices;
        cpu.edx <<= 5;
        if (!byAddress(app, cpu, 0x4bbde0))
            return;
        cpu.ecx = at(app, vertices);
        if (cpu.ecx == 0)
            continue;
        cpu.eax = at(app, block);
        cpu.edx = at(app, groups);
        push(app, cpu, cpu.ecx);
        cpu.ecx = at(app, cpu.eax + 0x74);
        cpu.ebx = at(app, cpu.eax + 0x70);
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x41a970))
            return;
        // Its polygon groups: the model's, the next, 7, 8, 9.
        cpu.eax = heads;
        cpu.ecx = at(app, vertices);
        push(app, cpu, cpu.eax);
        cpu.eax = tails;
        cpu.ebx = at(app, model);
        push(app, cpu, cpu.eax);
        cpu.edx = cpu.edi;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x41b300))
            return;
        cpu.edx = at(app, drawn);
        cpu.ebx = at(app, model);
        cpu.ecx = at(app, vertices);
        cpu.edx += cpu.eax;
        cpu.eax = heads + 4;
        ++cpu.ebx;
        push(app, cpu, cpu.eax);
        cpu.eax = tails + 4;
        put(app, drawn, cpu.edx);
        push(app, cpu, cpu.eax);
        cpu.edx = cpu.edi;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x41b300))
            return;
        cpu.ecx = at(app, drawn);
        cpu.ebx = 7;
        cpu.ecx += cpu.eax;
        cpu.eax = heads + 4;
        cpu.edx = cpu.edi;
        push(app, cpu, cpu.eax);
        cpu.eax = tails + 4;
        put(app, drawn, cpu.ecx);
        push(app, cpu, cpu.eax);
        cpu.ecx = at(app, vertices);
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x41b300))
            return;
        cpu.ebx = at(app, drawn);
        cpu.ecx = at(app, vertices);
        cpu.ebx += cpu.eax;
        cpu.eax = heads + 0xc;
        cpu.edx = cpu.edi;
        push(app, cpu, cpu.eax);
        cpu.eax = tails + 0xc;
        put(app, drawn, cpu.ebx);
        push(app, cpu, cpu.eax);
        cpu.ebx = 8;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x41b300))
            return;
        cpu.edx = at(app, drawn);
        cpu.ecx = at(app, vertices);
        cpu.edx += cpu.eax;
        cpu.eax = heads + 8;
        cpu.ebx = 9;
        push(app, cpu, cpu.eax);
        cpu.eax = tails + 8;
        put(app, drawn, cpu.edx);
        push(app, cpu, cpu.eax);
        cpu.edx = cpu.edi;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x41b300))
            return;
        put(app, drawn, at(app, drawn) + cpu.eax);
        if (at(app, kSplitScreen) == 0 && at(app, kNetworkRace) == 0)
        {
            cpu.ecx = at(app, vertices);
            cpu.ebx = 0xa;
            if (!groupsOf(heads, tails))
                return;
            put(app, drawn, at(app, drawn) + cpu.eax);
        }
        if (at(app, model) == 4)
        {
            cpu.ecx = at(app, vertices);
            cpu.ebx = 6;
            if (!groupsOf(heads + 0x10, tails + 0x10))
                return;
            put(app, drawn, at(app, drawn) + cpu.eax);
        }
        // The headlights on it.
        cpu.eax = at(app, model);
        push(app, cpu, cpu.eax);
        cpu.edx = at(app, block);
        push(app, cpu, cpu.edx);
        cpu.eax = heads;
        cpu.ecx = at(app, vertices);
        push(app, cpu, cpu.eax);
        cpu.eax = tails;
        cpu.ebx = cpu.edi;
        push(app, cpu, cpu.eax);
        cpu.edx = at(app, groups);
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, headlightsOnBlock))
            return;
        cpu.ecx = at(app, drawn);
        cpu.ecx += cpu.eax;
        cpu.eax = blockOffset(cpu.edi);
        cpu.edx = kBlocks + cpu.eax;
        put(app, drawn, cpu.ecx);
        put(app, blockAt, cpu.edx);
        cpu.ebx = at(app, cpu.edx + 0x5b8);
        if (cpu.ebx != 0)
        {
            cpu.edx = cpu.edi;
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, blockLights))
                return;
        }
        // Its object lists: three, a fourth outside split screen and network
        // races.
        push(app, cpu, 1);
        cpu.eax = at(app, blockAt);
        cpu.ebx = heads + 4;
        cpu.eax += 0x564;
        cpu.edx = tails + 4;
        push(app, cpu, cpu.eax);
        cpu.ecx = cpu.edi;
        cpu.eax = cpu.esi;
        push(app, cpu, 0xbf800000);
        if (!direct(app, cpu, drawObjectList))
            return;
        cpu.edx = at(app, drawn);
        push(app, cpu, 1);
        cpu.ebx = heads + 0xc;
        cpu.edx += cpu.eax;
        cpu.eax = at(app, blockAt);
        cpu.ecx = cpu.edi;
        cpu.eax += 0x56c;
        put(app, drawn, cpu.edx);
        push(app, cpu, cpu.eax);
        cpu.edx = tails + 0xc;
        cpu.eax = cpu.esi;
        push(app, cpu, 0xbf800000);
        if (!direct(app, cpu, drawObjectList))
            return;
        cpu.ecx = at(app, drawn);
        push(app, cpu, 1);
        cpu.ebx = heads + 8;
        cpu.ecx += cpu.eax;
        cpu.eax = at(app, blockAt);
        cpu.edx = tails + 8;
        cpu.eax += 0x574;
        put(app, drawn, cpu.ecx);
        push(app, cpu, cpu.eax);
        cpu.ecx = cpu.edi;
        cpu.eax = cpu.esi;
        push(app, cpu, 0xbf800000);
        if (!direct(app, cpu, drawObjectList))
            return;
        cpu.ebx = at(app, drawn);
        cpu.ebx += cpu.eax;
        cpu.eax = at(app, kSplitScreen);
        put(app, drawn, cpu.ebx);
        if (cpu.eax == 0 && at(app, kNetworkRace) == 0)
        {
            push(app, cpu, 1);
            cpu.eax = at(app, blockAt);
            cpu.ebx = heads;
            cpu.eax += 0x57c;
            cpu.edx = tails;
            push(app, cpu, cpu.eax);
            cpu.ecx = cpu.edi;
            cpu.eax = cpu.esi;
            push(app, cpu, 0xbf800000);
            if (!direct(app, cpu, drawObjectList))
                return;
            put(app, drawn, at(app, drawn) + cpu.eax);
        }
        // The buckets flushed past the model's most.
        cpu.edx = at(app, modelAt);
        cpu.eax = at(app, drawn);
        if (sreg32(cpu.eax) > sreg32(at(app, cpu.edx + 8)) && at(app, kWorldsDrawn) != 0)
        {
            cpu.ecx = at(app, buckets);
            cpu.ebx = heads;
            cpu.edx = tails;
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, flushBuckets))
                return;
            cpu.eax = 0;
            put(app, drawn, cpu.eax);
        }
    }
    // The track's own objects, where it has lights.
    if (at(app, kColLights) != 0)
    {
        if (!direct(app, cpu, ambientWanted))
            return;
        push(app, cpu, cpu.eax);
        cpu.ebx = heads + 8;
        push(app, cpu, kTrackObjects);
        cpu.eax = at(app, kView);
        cpu.edx = tails + 8;
        cpu.ecx = 0;
        push(app, cpu, at(app, cpu.eax + 0x30));
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, drawObjectList))
            return;
    }
    // The six lists into the buckets: a list's first and last, or an empty
    // list's.
    for (cpu.edx = 0;; )
    {
        safepoint(app, cpu);
        cpu.eax = cpu.edx * 4;
        cpu.edi = at(app, buckets);
        cpu.ebx = cpu.edx * 8;
        cpu.edi += cpu.ebx;
        cpu.ebx = at(app, cpu.eax + heads);
        if (cpu.ebx == 0)
        {
            cpu.esi = cpu.ebp - 0x40;
            if (!byAddress(app, cpu, 0x4bbe70))
                return;
            cpu.esi = cpu.ebp - 0x40;
            movsd(app, cpu);
            movsd(app, cpu);
        }
        else
        {
            put(app, cpu.edi, cpu.ebx);
            cpu.ebx = at(app, cpu.eax + tails);
            put(app, cpu.ebx, 0);
            cpu.eax = at(app, cpu.eax + tails);
            put(app, cpu.edi + 4, cpu.eax);
        }
        ++cpu.edx;
        if (sreg32(cpu.edx) >= 6)
            break;
    }
    cpu.edx = 1;
    cpu.eax = at(app, first);
    if (!byAddress(app, cpu, 0x431c20))
        return;
    cpu.edx = 2;
    cpu.eax = at(app, second);
    if (!byAddress(app, cpu, 0x431c20))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41d210: sound source edx heard from the camera eax: its distance and
 * side from the camera's place (0x412620) made a volume (0x10000 over the
 * distance's square, at most 0x7f) and a pan (0x4185e0); a source of kind 1
 * (+0xe) played in the ambient slot 0x13 at a random half-second of its own
 * (the frame counter's eighths against [0x552e44], drawn anew), else, the
 * first heard (ebx 0), in slot 0x12.  ebx, or 1 for the first played. */
void ambientSource(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 across = cpu.ebp - 8, side = cpu.ebp - 4;
    // The ambient slot's sound: slot eax, volume edi, pan ebx, the player's
    // pitch.
    const auto play = [&] {
        return byAddress(app, cpu, 0x416fc0);
    };
    cpu.ecx = cpu.edx;
    cpu.esi = cpu.ebx;
    cpu.ebx = across;
    cpu.eax += 8;
    if (!byAddress(app, cpu, 0x412620))
        return;
    cpu.ebx = cpu.eax;
    cpu.ebx = reg32(sreg32(cpu.ebx) >> 0xc);
    cpu.ebx = cpu.ebx * cpu.ebx;
    cpu.ebx = reg32(sreg32(cpu.ebx) >> 0xa);
    if (cpu.ebx == 0)
    {
        cpu.edi = 0x7f;
    }
    else
    {
        cpu.eax = 0x10000;
        idiv(cpu, cpu.ebx);
        if (sreg32(cpu.eax) >= 0x7f)
            cpu.eax = 0x7f;
        cpu.edi = cpu.eax;
    }
    cpu.eax = at(app, across);
    towardZero(cpu, 0xa);
    cpu.edx = at(app, side);
    cpu.eax += 0x40;
    if (!byAddress(app, cpu, 0x4185e0))
        return;
    cpu.ebx = cpu.eax;
    cpu.dx = half(app, cpu.ecx + 0xe);
    if (cpu.dx == 1)
    {
        cpu.edx = at(app, kFrameCounter) & 0x3f;
        cpu.eax = cpu.edx;
        towardZero(cpu, 3);
        cpu.edx = at(app, kAmbientLast);
        if (cpu.eax == cpu.edx)
        {
            cpu.eax = at(app, at(app, kView)) - 0x10;
            cpu.edx = 0;
            push(app, cpu, cpu.eax);
            cpu.dx = half(app, cpu.ecx + 0xc);
            push(app, cpu, cpu.ebx);
            cpu.ecx = 0x10000;
            push(app, cpu, cpu.edi);
            cpu.eax = 0x13;
            cpu.ebx = 0x40;
            if (!play() || !byAddress(app, cpu, 0x4e0d88))
                return;
            cpu.eax &= 7;
            put(app, kAmbientLast, cpu.eax);
        }
    }
    else if (cpu.esi == 0)
    {
        cpu.eax = at(app, at(app, kView)) - 0x10;
        push(app, cpu, cpu.eax);
        cpu.edx = cpu.eax;
        push(app, cpu, cpu.ebx);
        cpu.edx ^= cpu.eax;
        cpu.ebx = 0x40;
        cpu.eax = 0x12;
        push(app, cpu, cpu.edi);
        cpu.dx = half(app, cpu.ecx + 0xc);
        cpu.ecx = 0x10000;
        cpu.esi = 1;
        if (!play())
            return;
    }
    cpu.eax = cpu.esi;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41d310: view eax (0x54 bytes each from 0x5dd0e0) the one drawn
 * ([0x5dd830]) -- in the mirror's pass ([0x7a3d14] 1) given the first view's
 * block.  eax the view's offset. */
void selectView(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.eax = ((((cpu.eax << 2) + cpu.edx) << 2) + cpu.edx) << 2;
    cpu.ecx = kViews;
    cpu.edx = at(app, 0x7a3d14);
    cpu.ecx += cpu.eax;
    if (cpu.edx == 1)
    {
        cpu.edx = at(app, kViews + 4);
        put(app, cpu.ecx + 4, cpu.edx);
    }
    put(app, kView, cpu.ecx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41d350: object edx of the car eax's block set moving: the car's speed
 * (|+0xa4| + |+0xac|) against its threshold (0, or 0xb for a car with +0x200
 * bit 2 clear).  An object that moves when hit (+0x13 2) and still (state
 * 0xff): fast enough, set on a path (0x40f3b0: 2, or 3 at 25 and up) -- its
 * angle from the car's move (0x4ea570) and the path started (0x40f430) --
 * or turning (0xfe) where there is none; -1.  Too slow, 1; one moving
 * already, 0; one that falls (3), 2; any other 1. */
void objectAnimation(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
    const reg32 objects = cpu.ebp - 0x14, threshold = cpu.ebp - 0xc, absX = cpu.ebp - 8, absZ = cpu.ebp - 4;
    const reg32 kind = cpu.ebp - 0x10;
    const auto leave = [&](reg32 result) {
        cpu.eax = result;
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    cpu.edi = cpu.edx;
    cpu.eax += 0xc;
    cpu.eax = at(app, cpu.eax);
    cpu.eax = at(app, cpu.eax + 0x5a4);
    put(app, objects, cpu.eax);
    if ((byte(app, cpu.ecx + 0x200) & 2) != 0)
    {
        cpu.edx = 0;
        put(app, threshold, cpu.edx);
    }
    else
    {
        put(app, threshold, 0xb);
    }
    if (!compare(cpu, Float(0.0), f32(app, cpu.ecx + 0xa4)).below)
    {
        putF32(app, absX, -f32(app, cpu.ecx + 0xa4));
    }
    else
    {
        cpu.eax = at(app, cpu.ecx + 0xa4);
        put(app, absX, cpu.eax);
    }
    if (!compare(cpu, Float(0.0), f32(app, cpu.ecx + 0xac)).below)
    {
        putF32(app, absZ, -f32(app, cpu.ecx + 0xac));
    }
    else
    {
        cpu.eax = at(app, cpu.ecx + 0xac);
        put(app, absZ, cpu.eax);
    }
    const Float speed = fp::add(f32(app, absX), f32(app, absZ));
    cpu.esp -= 4;
    put(app, cpu.esp, reg32(fp::toInteger<sreg32>(speed)));
    cpu.eax = pop(app, cpu);
    cpu.ebx = cpu.eax;
    cpu.eax = cpu.edi * 5;
    cpu.edx = at(app, objects);
    cpu.eax <<= 2;
    cpu.edx += cpu.eax;
    cpu.eax = half(app, cpu.edx + 0xe);
    put(app, kind, cpu.eax);
    cpu.esi = at(app, kind);
    cpu.eax <<= 2;
    cpu.eax -= cpu.esi;
    cpu.esi = kObjectStates;
    cpu.esi += cpu.eax;
    cpu.eax = 0;
    if (byte(app, cpu.edx + 0x13) != 2)
        return leave(byte(app, cpu.edx + 0x13) == 3 ? 2 : 1);
    if (byte(app, cpu.esi) != 0xff)
        return leave(cpu.eax);
    if (sreg32(cpu.ebx) < sreg32(at(app, threshold)))
        return leave(1);
    cpu.eax = sreg32(cpu.ebx) < 0x19 ? 2 : 3;
    if (!byAddress(app, cpu, 0x40f3b0))
        return;
    put(app, kind, cpu.eax);
    cpu.eax = cpu.edi * 5;
    cpu.ebx = at(app, objects);
    cpu.eax <<= 2;
    cpu.ebx += cpu.eax;
    cpu.edx = half(app, cpu.ebx + 0xe);
    cpu.eax = cpu.edx * 3;
    cpu.dl = byte(app, kind);
    putByte(app, cpu.eax + kObjectStates, cpu.dl);
    if (std::int8_t(byte(app, cpu.esi)) < 0)
    {
        putByte(app, cpu.esi, 0xfe);
        return leave(0xffffffff);
    }
    const Float z = fp::mul(f32(app, cpu.ecx + 0xac), f64(app, 0x536ccc));
    cpu.esp -= 4;
    put(app, cpu.esp, reg32(fp::toInteger<sreg32>(z)));
    cpu.eax = pop(app, cpu);
    const Float x = f32(app, cpu.ecx + 0xa4);
    cpu.edx = cpu.eax;
    const Float xs = fp::mul(x, f64(app, 0x536ccc));
    cpu.edx = reg32(sreg32(cpu.edx) >> 8);
    cpu.esp -= 4;
    put(app, cpu.esp, reg32(fp::toInteger<sreg32>(xs)));
    cpu.eax = pop(app, cpu);
    cpu.eax = reg32(sreg32(cpu.eax) >> 8);
    if (!byAddress(app, cpu, 0x4ea570))
        return;
    cpu.eax = reg32(sreg32(cpu.eax) >> 8);
    cpu.edx = 0;
    putByte(app, cpu.esi + 1, cpu.al);
    cpu.dx = half(app, cpu.ebx + 0xe);
    cpu.ecx = reg32(sreg32(std::int8_t(cpu.al)));
    cpu.eax = cpu.edx * 3;
    cpu.ebx = 0;
    cpu.eax = at(app, cpu.eax + kObjectStates - 3);
    cpu.edx = 2;
    cpu.eax = reg32(sreg32(cpu.eax) >> 0x18);
    if (!byAddress(app, cpu, 0x40f430))
        return;
    putByte(app, cpu.esi + 2, 0);
    leave(0xffffffff);
}

/* 0x41d510: the turn matrix at ecx of the angles eax, ebx and edx (16-bit
 * turns, scaled by 0x536cd4): a turn about y by the first's share of the
 * third (0x4dfde0), one about y by ebx itself into edx (0x4dfde0), the two
 * multiplied (0x4e0280) and the result by ebx's into ecx. */
void turnFromAngles(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x84;
    cpu.ebp -= 0x82;
    const reg32 e = cpu.ebp;
    const reg32 b = e + 0x7a, a = e + 0x7e;
    put(app, b, cpu.ebx);
    put(app, a, cpu.eax);
    const Float k = f64(app, 0x536cd4);
    const Float bk = fp::mul(Float(sreg32(at(app, b))), k);
    const Float ak = fp::mul(Float(sreg32(at(app, a))), k);
    put(app, b, cpu.edx);
    const Float minusBk = -bk;
    const Float rest = fp::sub(Float(1.0), ak);
    const Float yawShare = fp::mul(minusBk, rest);
    const Float ck = fp::mul(k, Float(sreg32(at(app, b))));
    const Float minusCk = -ck;
    const Float pitchShare = fp::mul(ak, minusCk);
    cpu.esp -= 4;
    put(app, a, cpu.ebx);
    const Float angle = fp::add(pitchShare, yawShare);
    cpu.eax = e + 0x22;
    putF32(app, cpu.esp, angle);
    cpu.ebx = e + 0x46;
    if (!byAddress(app, cpu, 0x4dfde0))
        return;
    const Float second = Float(sreg32(at(app, a)));
    cpu.esp -= 4;
    cpu.eax = e - 2;
    putF32(app, cpu.esp, second);
    cpu.edx = cpu.ecx;
    if (!byAddress(app, cpu, 0x4dfde0))
        return;
    cpu.eax = e + 0x22;
    if (!byAddress(app, cpu, 0x4e0280))
        return;
    cpu.edx = e - 2;
    cpu.eax = e + 0x46;
    cpu.ebx = cpu.ecx;
    if (!byAddress(app, cpu, 0x4e0280))
        return;
    cpu.esp = e + 0x82;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41d5a0: the turn matrix at ebx of a pitch eax and a yaw edx (16-bit
 * turns, scaled by 0x536cdc): a turn about x by a quarter (0x4dfd80), about
 * y by the pitch turned back and by the yaw (0x4dfde0), all multiplied
 * (0x4e0280) into ebx. */
void turnFromPitchYaw(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x7c;
    const reg32 pitch = cpu.ebp - 8, yaw = cpu.ebp - 4, pitchAngle = cpu.ebp - 0xc, yawAngle = cpu.ebp - 0x10;
    cpu.ecx = cpu.ebx;
    put(app, pitch, cpu.eax);
    put(app, yaw, cpu.edx);
    const Float k = f64(app, 0x536cdc);
    const Float p = fp::mul(Float(sreg32(at(app, pitch))), k);
    const Float y = fp::mul(k, Float(sreg32(at(app, yaw))));
    push(app, cpu, 0x3e800000);
    cpu.eax = cpu.ebx;
    putF32(app, pitchAngle, p);
    putF32(app, yawAngle, y);
    if (!byAddress(app, cpu, 0x4dfd80))
        return;
    const Float back = -f32(app, pitchAngle);
    cpu.esp -= 4;
    cpu.eax = cpu.ebp - 0x58;
    putF32(app, cpu.esp, back);
    cpu.edx = cpu.ecx;
    if (!byAddress(app, cpu, 0x4dfde0))
        return;
    push(app, cpu, at(app, yawAngle));
    cpu.eax = cpu.ebp - 0x7c;
    cpu.ebx = cpu.ebp - 0x34;
    if (!byAddress(app, cpu, 0x4dfde0))
        return;
    cpu.eax = cpu.ebp - 0x58;
    if (!byAddress(app, cpu, 0x4e0280))
        return;
    cpu.edx = cpu.ebp - 0x7c;
    cpu.eax = cpu.ebp - 0x34;
    cpu.ebx = cpu.ecx;
    if (!byAddress(app, cpu, 0x4e0280))
        return;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41d610: View Distance set to eax. */
void setViewDistance(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    put(app, kViewDistance, cpu.eax);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41d870: View Distance the next of its four (the pause menu's item), and
 * the views' distances worked out again (0x41d620). */
void nextViewDistance(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kViewDistance) + 1;
    cpu.ecx = 4;
    put(app, kViewDistance, cpu.edx);
    cpu.eax = cpu.edx;
    idiv(cpu, cpu.ecx);
    put(app, kViewDistance, cpu.edx);
    if (!byAddress(app, cpu, 0x41d620))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41d900: on the drivers of types 7 and 8 ([0x7a3a58] bits 0x180) the
 * models' most polygons -1 (the buckets flushed after every block) and each
 * block's +0x18 cleared, but the last's. */
void modelsUnlimited(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if ((half(app, kDriverBits) & 0x180) != 0)
    {
        cpu.edx = 0xffffffff;
        put(app, kModels + 0x14, cpu.edx);
        put(app, kModels + 0x20, cpu.edx);
        put(app, kModels + 8, cpu.edx);
        for (cpu.edx = 0;;)
        {
            safepoint(app, cpu);
            if (sreg32(cpu.edx) >= sreg32(at(app, kLastBlock)))
                break;
            cpu.eax = blockOffset(cpu.edx) + kBlocks;
            ++cpu.edx;
            put(app, cpu.eax + 0x18, 0);
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41da40: the block at [eax] freed, if there is one, and [eax] 0. */
void freeAt(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, cpu.eax);
    if (cpu.ecx != 0)
    {
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kFree))
            return;
        put(app, cpu.edx, 0);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41da60: the track let go of at the race's end: the arena, the textures'
 * file, the track's lights (0x41e3b0), the views, the .col file, the paths
 * (0x40f330) and the kinds' records. */
void unloadTrack(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 0;
    put(app, 0x5dd834, cpu.edx);
    if (!direct(app, cpu, freeArena))
        return;
    if (at(app, kTexturesFile) != 0)
    {
        cpu.eax = kTexturesFile;
        if (!direct(app, cpu, freeAt))
            return;
    }
    if (!byAddress(app, cpu, 0x41e3b0) || !direct(app, cpu, resetViewsOuter) || !direct(app, cpu, freeCollisions)
        || !byAddress(app, cpu, 0x40f330) || !direct(app, cpu, freeTrackKinds))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41daa0: the pool at eax of edx records (36 bytes each, allocated,
 * unnamed): none taken (-1), +0x18 and +0xc 0. */
void newPool(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    put(app, cpu.eax, 0xffffffff);
    put(app, cpu.eax + 0x18, 0);
    cpu.eax = cpu.edx * 8;
    cpu.ebx = 0;
    cpu.edx += cpu.eax;
    cpu.eax = kNoName2;
    cpu.edx <<= 2;
    if (!byAddress(app, cpu, kAllocate))
        return;
    put(app, cpu.ecx + 0xc, 0);
    put(app, cpu.ecx + 0x10, cpu.eax);
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41dae0: the objects' pool at 0x5dd848, 0x800 records. */
void newObjectPool(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 0x800;
    cpu.eax = kObjectPool;
    if (!direct(app, cpu, newPool))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41db00: the track's kinds' records freed ([0x5dd858]). */
void freeTrackKinds(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = kTrackKinds;
    if (!direct(app, cpu, freeAt))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41db10: the textures of the .qfs at eax taken by the driver, each into
 * its record at edx (0x2c bytes apart) -- a grey driver's one way
 * (0x4d41a0), else the other (0x4d4080, at 128; the port's 512) -- the loading bar moved on from 43
 * per cent by its share of 15 (0x536d1c) a texture (0x495230), until the
 * loading stops ([0x55e88c]). */
void loadTextures(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    const reg32 step = cpu.ebp - 0x10, progress = cpu.ebp - 0xc, index = cpu.ebp - 8, count = cpu.ebp - 4;
    cpu.esi = cpu.eax;
    if (!byAddress(app, cpu, 0x4ea654))
        return;
    cpu.ecx = 0x41700000;
    cpu.edi = cpu.edx;
    cpu.edx = 0x422c0000;
    put(app, step, cpu.ecx);
    put(app, progress, cpu.edx);
    if (cpu.eax != 0)
    {
        put(app, count, cpu.eax);
        putF32(app, step, fp::div(f32(app, 0x536d1c), Float(sreg32(at(app, count)))));
    }
    cpu.ebx = 0;
    put(app, index, cpu.ebx);
    for (;; put(app, index, at(app, index) + 1))
    {
        safepoint(app, cpu);
        cpu.eax = cpu.esi;
        cpu.edx = at(app, index);
        if (!byAddress(app, cpu, 0x4ea654))
            return;
        if (sreg32(cpu.eax) <= sreg32(cpu.edx))
            break;
        if (!byAddress(app, cpu, kTopUp))
            return;
        cpu.eax = cpu.esi;
        if (!byAddress(app, cpu, 0x4ea658))
            return;
        if (cpu.eax == 0)
            continue;
        if ((byte(app, kDriverBits) & 2) != 0)
        {
            cpu.edx = cpu.edi;
            if (!byAddress(app, cpu, 0x4d41a0))
                return;
        }
        else
        {
            push(app, cpu, 3);
            // port: each texture at its own size up to 512, not 128 (port_thrash.h)
            cpu.ebx = port::driver::kMaxTextureSide;
            cpu.ecx = cpu.edi;
            cpu.edx = cpu.eax;
            cpu.eax = 0;
            if (!byAddress(app, cpu, 0x4d4080))
                return;
        }
        putF32(app, progress, fp::add(f32(app, progress), f32(app, step)));
        push(app, cpu, at(app, progress));
        if (!byAddress(app, cpu, 0x495230))
            return;
        cpu.ecx = at(app, kLoadStopped);
        cpu.edi += 0x2c;
        if (cpu.ecx != 0)
            break;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41dc60: the opened textures taken by the driver (loadTextures, into the
 * records at [0x5dd858]; their count to 0x5dd854, 0x5dd85c and 0x5dd860) and
 * the file let go of; the records handed on (0x478b70). */
void takeTextures(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kTexturesFile);
    if (cpu.edx != 0)
    {
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, 0x4ea654))
            return;
        cpu.ecx = 0;
        cpu.ebx = 0;
        cpu.edx = at(app, kTrackKinds);
        put(app, kTexturesCount, cpu.eax);
        cpu.eax = at(app, kTexturesFile);
        if (!direct(app, cpu, loadTextures))
            return;
        cpu.eax = at(app, kTexturesCount);
        put(app, kTexturesLeft, cpu.eax);
        put(app, kTexturesAll, cpu.eax);
        cpu.eax = kTexturesFile;
        cpu.ecx = 0;
        if (!direct(app, cpu, freeAt))
            return;
        put(app, kTexturesFile, cpu.ecx);
    }
    cpu.eax = at(app, kTrackKinds);
    if (!byAddress(app, cpu, 0x478b70))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41dcc0: the track's .col file named eax loaded, if it is there (0x431320,
 * at least 0x16800 bytes of the game's memory for it, 0x552e18), and its
 * chunks gone through: the vertices (kind 2: their bytes +6 and +7 given 1
 * and 0x1f where 0), the lights (7, 0x5dd840), the objects (8, 0x5dd844),
 * each object of kind 0xf (0x41e370); the file closed (0x4313b0). */
void loadCollisions(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 chunks = cpu.ebp - 4;
    cpu.edx = cpu.eax;
    cpu.ecx = 0;
    put(app, kColObjects, cpu.ecx);
    put(app, kColLights, cpu.ecx);
    if (!byAddress(app, cpu, kFileExists))
        return;
    if (cpu.eax != 0)
    {
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, 0x431320))
            return;
        cpu.ebx = 0;
        cpu.edx = cpu.eax;
        cpu.ecx = cpu.eax;
        if (!byAddress(app, cpu, 0x4313d0))
            return;
        if (sreg32(cpu.eax) > 0x16800)
        {
            cpu.eax = cpu.edx;
            if (!byAddress(app, cpu, 0x4313d0))
                return;
            cpu.edx = cpu.eax;
        }
        else
        {
            cpu.edx = 0x16800;
        }
        cpu.eax = kNoName2;
        if (!byAddress(app, cpu, kAllocate))
            return;
        put(app, kColFile, cpu.eax);
        cpu.edx = cpu.eax;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, 0x4313e0))
            return;
        if (cpu.eax != 0)
        {
            cpu.eax = cpu.ecx;
            if (!byAddress(app, cpu, 0x431410))
                return;
            cpu.ebx = 0;
            put(app, chunks, cpu.eax);
            for (; sreg32(cpu.ebx) < sreg32(at(app, chunks)); ++cpu.ebx)
            {
                safepoint(app, cpu);
                cpu.eax = at(app, kColFile);
                cpu.edx = cpu.ebx;
                if (!byAddress(app, cpu, 0x431420))
                    return;
                cpu.edx = cpu.eax;
                cpu.edi = cpu.eax;
                if (!byAddress(app, cpu, kTopUp))
                    return;
                cpu.ax = half(app, cpu.edx + 4);
                if (cpu.ax < 7)
                {
                    if (cpu.ax != 2)
                        continue;
                    // The vertices: 8 bytes each after the chunk's head.
                    cpu.edx += 8;
                    cpu.eax = 0;
                    put(app, kColVertices, cpu.edx);
                    for (;;)
                    {
                        safepoint(app, cpu);
                        cpu.edx = half(app, cpu.edi + 6);
                        if (sreg32(cpu.eax) >= sreg32(cpu.edx))
                            break;
                        cpu.esi = at(app, kColVertices);
                        cpu.edx = cpu.eax * 8 + cpu.esi;
                        if (byte(app, cpu.edx + 7) == 0)
                            putByte(app, cpu.edx + 7, 0x1f);
                        cpu.esi = at(app, kColVertices);
                        cpu.edx = cpu.eax * 8 + cpu.esi;
                        if (byte(app, cpu.edx + 6) == 0)
                            putByte(app, cpu.edx + 6, 1);
                        ++cpu.eax;
                    }
                }
                else if (cpu.ax == 7)
                {
                    put(app, kColLights, cpu.edx);
                }
                else if (cpu.ax <= 8)
                {
                    put(app, kColObjects, cpu.edx);
                }
                else if (cpu.ax == 0xf)
                {
                    cpu.eax = cpu.edx;
                    if (!byAddress(app, cpu, 0x41e370))
                        return;
                }
            }
        }
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, 0x4313b0))
            return;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41de10: the .col file's memory freed, if there is some. */
void freeCollisions(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (at(app, kColFile) != 0)
    {
        cpu.eax = kColFile;
        cpu.ecx = 0;
        if (!direct(app, cpu, freeAt))
            return;
        put(app, kColFile, cpu.ecx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41de40: the 0x200 objects' states still (0xff, 0, 0), the sound's stream
 * topped up for each (0x4a33d0). */
void clearObjectStates(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 0;
    do
    {
        safepoint(app, cpu);
        if (!byAddress(app, cpu, kTopUp))
            return;
        cpu.eax = cpu.edx * 3;
        putByte(app, cpu.eax + kObjectStates, 0xff);
        cpu.bh = 0;
        putByte(app, cpu.eax + kObjectStates + 1, cpu.bh);
        ++cpu.edx;
        putByte(app, cpu.eax + kObjectStates + 2, cpu.bh);
    } while (sreg32(cpu.edx) < 0x200);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41de80: the views as a race starts: the objects still, the first view
 * selected and the paths reset (0x40f230). */
void resetTrackViews(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!direct(app, cpu, clearObjectStates))
        return;
    cpu.eax = 0;
    if (!direct(app, cpu, selectView) || !byAddress(app, cpu, 0x40f230))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41dea0: resetViews. */
void resetViewsOuter(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!direct(app, cpu, resetViews))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41deb0: the views' serial started over (0x1234, 0) and the four views on
 * no block (-1), the sound's stream topped up for each. */
void resetViews(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 0x1234;
    cpu.ecx = 0;
    put(app, kViewSerial, cpu.edx);
    put(app, kViewSerialHigh, cpu.ecx);
    cpu.edx = 0;
    do
    {
        safepoint(app, cpu);
        if (!byAddress(app, cpu, kTopUp))
            return;
        cpu.eax = cpu.edx * 21;
        cpu.ebx = 0xffffffff;
        ++cpu.edx;
        put(app, cpu.eax * 4 + kViews, cpu.ebx);
    } while (sreg32(cpu.edx) < 4);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e070: whether block edx is drawn for the camera eax: in view (its box,
 * 0x41bf30) and within the view's reach of its centre.  1 where it is. */
void blockInView(App* app, CPU& cpu)
{
    x86::FPU& fpu = cpu.fpu;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    cpu.esi = cpu.eax;
    cpu.ecx = cpu.eax + 0x38;
    cpu.ebx = cpu.eax + 0x44;
    cpu.eax = blockOffset(cpu.edx);
    cpu.edi = kBlocks + cpu.eax;
    cpu.edx = cpu.edi + 0x84;
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, 0x41bf30))
        return;
    if (cpu.eax != 0)
    {
        cpu.eax = 0;
    }
    else
    {
        cpu.eax = cpu.edi + 0x78;
        const Float dz = fp::sub(f32(app, cpu.esi + 0x10), f32(app, cpu.eax + 8));
        const Float dx = fp::sub(f32(app, cpu.esi + 8), f32(app, cpu.eax));
        cpu.eax = at(app, kView);
        const Float reach = f32(app, cpu.eax + 0x34);
        const Float d = fp::add(fp::mul(dz, dz), fp::mul(dx, dx));
        cpu.eax = compare(cpu, fp::mul(reach, reach), d).below ? 0 : 1;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace track
}  // namespace nfs3hp
