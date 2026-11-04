#include "native_guest.h"

#include <cstdint>

/* The track's data and a place on it: its nodes, the road's quads around
 * them, and what the cars, the sound and the renderer ask of them.
 * disassemble_nfs3hp.py binds each address to the function of that name here
 * (TRACKDATA_NATIVES), and the generated code calls it by name.
 * tools/game_checks holds each against its generated code.
 *
 * The track's nodes: [0x5dd948] (the track's header [0x5dd938] + 8), [0x5dd940]
 * of them (the header's word at 6), 0x24 bytes each -- x, y, z in 16.16 at 0,
 * 4, 8, signed bytes of its directions from 0xd on (each the top byte of the
 * word read 3 bytes before it), nibbles at 0xe and 0x13, widths at 0x17 and
 * 0x1b.  Every 8 nodes a block of 0x5c0 bytes from 0x571370: +0x3c its strips
 * (14 bytes each, four vertices' words), +0x70 its vertices (three floats),
 * +0x74 their colours, +0x584 its first node, +0x58c its nodes' entries (8
 * bytes: the first strip's word, the strips' count at 2, a kind at 3, the
 * linked nodes' words at 4 and 6), +0x594 the strips' polygons (8 bytes, the
 * normal's index at 0, the kind's nibble at 1, 0xe off the road), +0x59c the
 * normals (12 bytes), +0x5a0 and +0x5a4 its items (20 bytes each).
 *
 * A place on the track (0x78 bytes, 0x1e words): +0 its block, +4 where it
 * is, +0x10 its node, +0x14 its strip, +0x18 its polygon, +0x1c its normal,
 * +0x20 its node's entry, +0x24 the quad's four corners in 16.16 (12 words),
 * +0x54 the four corners' vertices, +0x64 what was found, +0x68 the node
 * changed, +0x6a the strip changed, +0x6c which end it fell off, +0x70 the
 * strip across the road, +0x74 whether it is followed.
 *
 * The track's .ccm (tr%02x.ccm, [0x552e48]: a count at 4, 0x1c bytes each
 * from 8, each placed on its nearest node, at +0x18, and sorted by it) and
 * simhts.dat ([0x552e54]) are loaded with the nodes.
 *
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frame's locals in the guest's memory,
 * the registers as the code leaves them, the x87's arithmetic as the game's
 * code has it (x86::FPU's add, sub, mul, compare and fistp), its stack in
 * locals where the code's never leaves the function, on the x87's own where
 * __CHP truncates it.  The surface's switch on the track goes by the targets
 * nfs3.exe's table at 0x420814 holds.  Every other function of the game's is
 * called by address. */
namespace nfs3hp
{
namespace trackdata
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using x86::Float;
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;

// The track's: the lanes' offsets, the header, its nodes, their count.
constexpr reg32 kLaneSpan = 0x5dd930, kLaneShift = 0x5dd934;
constexpr reg32 kHeader = 0x5dd938, kNodesToo = 0x5dd93c, kNodeCount = 0x5dd940, kNodes = 0x5dd948;
constexpr reg32 kBlocks = 0x571370;  // 0x5c0 a block of 8 nodes
// The .ccm's block, its points' count and first point; simhts.dat's block.
constexpr reg32 kCcm = 0x552e48, kCcmCount = 0x552e4c, kCcmPoints = 0x552e50, kSimHeights = 0x552e54;
// The searches' steps along the nodes: 11, 7 and 3 of them.
constexpr reg32 kSteps = 0x552e58, kNearSteps = 0x552e88, kLinkSteps = 0x552ea8;
// The track's number, the data's path, the .ccm's and simhts.dat's names.
constexpr reg32 kTrack = 0x6fd4ac, kReversed = 0x6fd4c0, kDataPath = 0x7a27b4;
constexpr reg32 kCcmName = 0x536d7c, kSimHeightsName = 0x536d94, kNotFound = 0x536dc4;
// The cars' records.
constexpr reg32 kCars = 0x5efa48;
// 65536 and its inverse, doubles; 0.2.
constexpr reg32 kFixed = 0x536dac, kUnfixed = 0x536db4, kUnfixedToo = 0x536dbc, kNear = 0x536dcc;
constexpr reg32 kFixedToo = 0x536dd4, kUnfixedThree = 0x536ddc, kUnfixedFour = 0x536de4;
constexpr reg32 kFixedThree = 0x536dec, kFixedFour = 0x536df4, kFixedFive = 0x536dfc;
// What is called by address.
constexpr reg32 kSprintf = 0x4df690, kLoadFile = 0x4e0ec0, kLoadFileQuietly = 0x4e0fe0, kFree = 0x4e1890;
constexpr reg32 kDistance = 0x41aab0, kChop = 0x4dfd56, kTrace = 0x42d500, kFillZero = 0x4e070c;
constexpr reg32 kOnSurface = 0x4969c0, kLessAnother = 0x4e0000, kDot = 0x4e01f0, kCarNode = 0x4217e0;

reg16 half(App* app, reg32 a) { return app->getMemory<reg16>(a); }
void putHalf(App* app, reg32 a, reg16 v) { app->getMemory<reg16>(a) = v; }
sreg32 s(reg32 v) { return sreg32(v); }
Float f32(App* app, reg32 a) { return Float(app->getMemory<float>(a)); }
Float f64(App* app, reg32 a) { return Float(app->getMemory<double>(a)); }
Float i32(App* app, reg32 a) { return Float(sreg32(at(app, a))); }
void putF32(App* app, reg32 a, const Float& v) { app->getMemory<float>(a) = float(v); }

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

/* imul edx; shl edx, 16; shr eax, 16; adc eax, edx: eax and edx's product
 * in 16.16, rounded by the bit shifted out; edx left its high word << 16. */
void fixedMul(CPU& cpu)
{
    const std::int64_t product = std::int64_t(sreg32(cpu.eax)) * sreg32(cpu.edx);
    const reg32 low = reg32(product), high = reg32(std::uint64_t(product) >> 32);
    cpu.edx = high << 16;
    cpu.eax = (low >> 16) + cpu.edx + ((low >> 15) & 1);
}

/* sar edx, 31 (or cdq) before it; idiv by: edx:eax divided as the generated
 * code divides it. */
void idiv(CPU& cpu, reg32 by)
{
    const std::int64_t n = std::int64_t((std::uint64_t(cpu.edx) << 32) | cpu.eax);
    const std::int64_t d = sreg32(by);
    cpu.eax = reg32(n / d);
    cpu.edx = reg32(n % d);
}

/* fcom(p) and fnstsw ax: a compared with b, the status word left in ax. */
reg16 status(CPU& cpu, const Float& a, const Float& b)
{
    cpu.fpu.compare(a, b);
    cpu.ax = cpu.fpu.status.word;
    return cpu.ax;
}

constexpr reg16 kBelow = 0x100, kEqual = 0x4000;  // C0 and C3: sahf's CF and ZF

/* sub esp, 4; fistp dword [esp]; pop eax. */
reg32 fistpPop(App* app, CPU& cpu, const Float& value)
{
    cpu.esp -= 4;
    put(app, cpu.esp, reg32(fp::toInteger<sreg32>(value)));
    return pop(app, cpu);
}

/* The quads' test of a side: (a.x - b.x) * (z - b.z) against (a.z - b.z) *
 * (x - b.x), the place's x and z the floats at px and pz; true where the
 * code's jb is taken (the second below the first). */
bool sideBelow(App* app, CPU& cpu, reg32 a, reg32 b, reg32 px, reg32 pz)
{
    x86::FPU& fpu = cpu.fpu;
    const Float first = fp::mul(fp::sub(f32(app, a), f32(app, b)), fp::sub(f32(app, pz), f32(app, b + 8)));
    const Float second = fp::mul(fp::sub(f32(app, a + 8), f32(app, b + 8)), fp::sub(f32(app, px), f32(app, b)));
    return (status(cpu, second, first) & kBelow) != 0;
}

/* fstp [tmp]; fldz; fcomp [tmp]: the value's size to dst -- the word copied
 * through eax where it is not below 0, its negative stored where it is. */
void absInto(App* app, CPU& cpu, const Float& value, reg32 tmp, reg32 dst)
{
    putF32(app, tmp, value);
    if (status(cpu, Float(0.0), f32(app, tmp)) & (kBelow | kEqual))
    {
        cpu.eax = at(app, tmp);
        put(app, dst, cpu.eax);
    }
    else
        putF32(app, dst, -f32(app, tmp));
}

/* The value onto the x87's stack, __CHP's truncation, fistp to slot. */
bool chopInto(App* app, CPU& cpu, const Float& value, reg32 slot)
{
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = value;
    if (!byAddress(app, cpu, kChop))
        return false;
    const Float top = cpu.fpu.st(0);
    put(app, slot, reg32(fp::toInteger<sreg32>(top)));
    cpu.fpu.count -= 1;
    return true;
}

/* rep movsd: ecx words from esi to edi. */
void moveWords(App* app, CPU& cpu)
{
    for (; cpu.ecx != 0; --cpu.ecx)
    {
        put(app, cpu.edi, at(app, cpu.esi));
        cpu.esi += 4;
        cpu.edi += 4;
    }
}

/* movsd, n times. */
void moveWords(App* app, CPU& cpu, unsigned n)
{
    for (unsigned i = 0; i < n; ++i)
    {
        put(app, cpu.edi, at(app, cpu.esi));
        cpu.esi += 4;
        cpu.edi += 4;
    }
}

/* The block of node `node`: kBlocks + 0x5c0 * (node >> 3), as the code works it out. */
reg32 blockOffset(reg32 node)
{
    const reg32 eighth = reg32(s(node) >> 3);
    return ((eighth * 4 - eighth) * 8 - eighth) << 6;
}

}  // namespace

void loadCcmPoints(App* app, CPU& cpu);
void freeCcmPoints(App* app, CPU& cpu);
void loadSimHeights(App* app, CPU& cpu);
void freeSimHeights(App* app, CPU& cpu);
void openTrackNodes(App* app, CPU& cpu);
void closeTrackNodes(App* app, CPU& cpu);
void nearestNodeCoarse(App* app, CPU& cpu);
void followNode(App* app, CPU& cpu);
void nearestNode(App* app, CPU& cpu);
void quadVertices(App* app, CPU& cpu);
void quadStepForward(App* app, CPU& cpu);
void quadStepBack(App* app, CPU& cpu);
void quadToFixed(App* app, CPU& cpu);
void quadSet(App* app, CPU& cpu);
void nodeSection(App* app, CPU& cpu);
void resetPosition(App* app, CPU& cpu);
void findQuad(App* app, CPU& cpu);
void findQuadNearby(App* app, CPU& cpu);
void findQuadByLinks(App* app, CPU& cpu);
void findQuadFallback(App* app, CPU& cpu);
void locateOnRoad(App* app, CPU& cpu);
void updatePosition(App* app, CPU& cpu);
void findMatchingQuad(App* app, CPU& cpu);
void quadEdgeFlags(App* app, CPU& cpu);
void sectionItemCount(App* app, CPU& cpu);
void sectionItem(App* app, CPU& cpu);
void setLaneOffsets(App* app, CPU& cpu);
void lanePosition(App* app, CPU& cpu);
void positionFrame(App* app, CPU& cpu);
void snapToTrack(App* app, CPU& cpu);
void placeOnTrack(App* app, CPU& cpu);
void positionColour(App* app, CPU& cpu);
void positionSurface(App* app, CPU& cpu);
void surfaceUnder(App* app, CPU& cpu);
void inTunnel(App* app, CPU& cpu);
void carFacingWay(App* app, CPU& cpu);
void carSectionKind(App* app, CPU& cpu);

/* 0x41e180: the track's tr%02x.ccm (of [0x6fd4ac], under the data's path)
 * loaded whole, its count of points kept at 0x552e4c and its first at
 * 0x552e50; each point (0x1c bytes, a place first) placed on its nearest
 * node (nearestNodeCoarse, followNode through a place on the track in the
 * frame), the node at its +0x18, and the points sorted by it, a bubble sort
 * by way of the frame. */
void loadCcmPoints(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x298;
    cpu.ebp -= 0x82;
    const auto leave = [&] {
        cpu.esp = cpu.ebp + 0x82;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.edx = at(app, kTrack);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.edx);
    push(app, cpu, kDataPath);
    push(app, cpu, kCcmName);
    cpu.eax = cpu.ebp - 0x216;
    push(app, cpu, cpu.eax);
    cpu.ebx = 0;
    cpu.edx = 0;
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0x14;
    cpu.eax = cpu.ebp - 0x216;
    put(app, kCcmCount, cpu.ebx);
    if (!byAddress(app, cpu, kLoadFileQuietly))
        return;
    put(app, kCcm, cpu.eax);
    if (cpu.eax == 0)
        return leave();
    cpu.edx = at(app, cpu.eax + 4);
    cpu.eax += 8;
    put(app, kCcmCount, cpu.edx);
    put(app, kCcmPoints, cpu.eax);
    // Each point on its node.
    while (s(cpu.ebx) < s(at(app, kCcmCount)))
    {
        cpu.eax = cpu.ebx * 4;
        cpu.ecx = cpu.eax * 8;
        cpu.edx = cpu.ebp - 0x16;
        cpu.ecx -= cpu.eax;
        cpu.eax = at(app, kCcmPoints);
        cpu.edi = 0;
        cpu.eax += cpu.ecx;
        put(app, cpu.ebp - 6, cpu.edi);
        if (!direct(app, cpu, nearestNodeCoarse))
            return;
        cpu.eax = at(app, kCcmPoints);
        cpu.edx = cpu.ebp - 0x16;
        cpu.eax += cpu.ecx;
        if (!direct(app, cpu, followNode))
            return;
        cpu.eax = at(app, kCcmPoints);
        cpu.ecx += cpu.eax;
        cpu.eax = at(app, cpu.ebp - 6);
        ++cpu.ebx;
        put(app, cpu.ecx + 0x18, cpu.eax);
    }
    // Sorted by their nodes.
    cpu.eax = 0;
    put(app, cpu.ebp + 0x7e, cpu.eax);
    for (;;)
    {
        cpu.eax = at(app, kCcmCount);
        cpu.edx = at(app, cpu.ebp + 0x7e);
        --cpu.eax;
        if (s(cpu.eax) <= s(cpu.edx))
            return leave();
        cpu.ebx = 0;
        for (;;)
        {
            cpu.eax = at(app, kCcmCount);
            --cpu.eax;
            if (s(cpu.ebx) >= s(cpu.eax))
                break;
            cpu.eax = cpu.ebx + 1;
            cpu.eax <<= 2;
            cpu.edx = cpu.eax;
            cpu.eax <<= 3;
            cpu.esi = at(app, kCcmPoints);
            cpu.eax -= cpu.edx;
            cpu.edx = cpu.esi + cpu.eax;
            cpu.eax = cpu.ebx * 4;
            cpu.ecx = cpu.eax;
            cpu.eax <<= 3;
            cpu.eax -= cpu.ecx;
            cpu.eax += cpu.esi;
            cpu.ecx = at(app, cpu.eax + 0x18);
            if (s(cpu.ecx) > s(at(app, cpu.edx + 0x18)))
            {
                cpu.ecx = 7;
                cpu.edi = cpu.ebp + 0x62;
                cpu.esi = cpu.eax;
                moveWords(app, cpu);
                cpu.ecx = 7;
                cpu.edi = cpu.eax;
                cpu.esi = cpu.edx;
                moveWords(app, cpu);
                cpu.ecx = 7;
                cpu.esi = cpu.ebp + 0x62;
                cpu.edi = cpu.edx;
                moveWords(app, cpu);
            }
            ++cpu.ebx;
        }
        put(app, cpu.ebp + 0x7e, at(app, cpu.ebp + 0x7e) + 1);
    }
}

/* 0x41e2c0: the .ccm's block freed, if there is one, and forgotten. */
void freeCcmPoints(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kCcm);
    if (cpu.edx != 0)
    {
        cpu.eax = cpu.edx;
        cpu.ecx = 0;
        if (!byAddress(app, cpu, kFree))
            return;
        put(app, kCcm, cpu.ecx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e2f0: the track's simhts.dat (of [0x6fd4ac], under the data's path)
 * loaded whole (ea::loadFile, flags 0), its block to 0x552e54. */
void loadSimHeights(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x200;
    cpu.edx = at(app, kTrack);
    push(app, cpu, cpu.edx);
    push(app, cpu, kDataPath);
    push(app, cpu, kSimHeightsName);
    cpu.eax = cpu.ebp - 0x200;
    push(app, cpu, cpu.eax);
    if (!byAddress(app, cpu, kSprintf))
        return;
    cpu.esp += 0x10;
    cpu.edx = 0;
    cpu.eax = cpu.ebp - 0x200;
    if (!byAddress(app, cpu, kLoadFile))
        return;
    put(app, kSimHeights, cpu.eax);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e340: simhts.dat's block freed, if there is one, and forgotten. */
void freeSimHeights(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, kSimHeights);
    if (cpu.edx != 0)
    {
        cpu.eax = cpu.edx;
        cpu.ecx = 0;
        if (!byAddress(app, cpu, kFree))
            return;
        put(app, kSimHeights, cpu.ecx);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e370: the track's header eax taken: its nodes from +8, their count
 * the word at +6; the .ccm and simhts.dat loaded.  1. */
void openTrackNodes(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    put(app, kHeader, cpu.eax);
    cpu.edx = cpu.eax + 8;
    put(app, kNodesToo, cpu.edx);
    put(app, kNodes, cpu.edx);
    cpu.ax = half(app, cpu.eax + 6);
    cpu.eax &= 0xffff;
    put(app, kNodeCount, cpu.eax);
    if (!direct(app, cpu, loadCcmPoints) || !direct(app, cpu, loadSimHeights))
        return;
    cpu.eax = 1;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e3b0: the .ccm and simhts.dat freed. */
void closeTrackNodes(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!direct(app, cpu, freeCcmPoints) || !direct(app, cpu, freeSimHeights))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e3c0: the node nearest the place eax (0x41aab0's distance), of every
 * 8th, to the place on the track edx's +0x10 (-1 for none). */
void nearestNodeCoarse(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    cpu.ebx = cpu.eax;
    cpu.edi = cpu.edx;
    cpu.edx = 0x7fffffff;
    cpu.esi = 0xffffffff;
    cpu.eax = at(app, kNodeCount);
    cpu.ecx = 0;
    put(app, cpu.ebp - 8, cpu.eax);
    put(app, cpu.ebp - 4, cpu.edx);
    while (s(cpu.ecx) < s(at(app, cpu.ebp - 8)))
    {
        cpu.eax = cpu.ecx * 8;
        cpu.eax += cpu.ecx;
        cpu.edx = at(app, kNodes);
        cpu.eax <<= 2;
        cpu.eax += cpu.edx;
        cpu.edx = cpu.ebx;
        if (!byAddress(app, cpu, kDistance))
            return;
        if (s(cpu.eax) < s(at(app, cpu.ebp - 4)))
        {
            put(app, cpu.ebp - 4, cpu.eax);
            cpu.esi = cpu.ecx;
        }
        cpu.ecx += 8;
    }
    put(app, cpu.edi + 0x10, cpu.esi);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e420: the place on the track edx moved to the node nearest the place
 * eax (nearestNode), its block taken, and whether its node changed to the
 * words at +0x68 and +0x6a and eax. */
void followNode(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.edx;
    cpu.ebx = at(app, cpu.edx + 0x10);
    if (!direct(app, cpu, nearestNode))
        return;
    cpu.edx = at(app, cpu.ecx + 0x10);
    cpu.edx = reg32(s(cpu.edx) >> 3);
    cpu.eax = cpu.edx * 4;
    cpu.eax -= cpu.edx;
    cpu.eax <<= 3;
    cpu.eax -= cpu.edx;
    cpu.edx = kBlocks;
    cpu.eax <<= 6;
    cpu.edx += cpu.eax;
    put(app, cpu.ecx, cpu.edx);
    cpu.eax = cpu.ebx != at(app, cpu.ecx + 0x10) ? 1 : 0;
    putHalf(app, cpu.ecx + 0x68, cpu.ax);
    putHalf(app, cpu.ecx + 0x6a, cpu.ax);
    cpu.eax = reg32(sreg32(x86::sreg16(cpu.ax)));
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e470: the place on the track edx's node walked to the one nearest the
 * place eax: the squares of x's and z's differences (each >> 9) to the node
 * on and the one before, round the track's end, while one is nearer. */
void nearestNode(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x1c;
    const reg32 b = cpu.ebp;
    /* The node edx's squared span from the place ebx -- z's, then x's -- the
     * node to `nodeSlot`, z's square to `squareSlot`; eax x's square. */
    const auto span = [&](reg32 nodeSlot, reg32 squareSlot) {
        cpu.eax = at(app, cpu.ebx + 8);
        put(app, nodeSlot, cpu.edx);
        put(app, b - 4, cpu.eax);
        cpu.eax = at(app, cpu.edx + 8);
        cpu.edx = at(app, b - 4);
        cpu.edx -= cpu.eax;
        cpu.eax = cpu.edx;
        cpu.eax = reg32(s(cpu.eax) >> 9);
        cpu.eax *= cpu.eax;
        put(app, squareSlot, cpu.eax);
        cpu.eax = at(app, cpu.ebx);
        put(app, b - 4, cpu.eax);
        cpu.eax = at(app, nodeSlot);
        cpu.edx = at(app, b - 4);
        cpu.eax = at(app, cpu.eax);
        cpu.edx -= cpu.eax;
        cpu.eax = cpu.edx;
        cpu.eax = reg32(s(cpu.eax) >> 9);
        cpu.eax *= cpu.eax;
    };
    cpu.ebx = cpu.eax;
    put(app, b - 0xc, cpu.edx);
    cpu.edi = at(app, kNodeCount);
    cpu.esi = at(app, kNodes);
    cpu.ecx = at(app, cpu.edx + 0x10);
    cpu.edx = 0xffffffff;
    --cpu.edi;
    put(app, b - 0x10, cpu.edx);
    while (cpu.ecx != at(app, b - 0x10))
    {
        put(app, b - 0x10, cpu.ecx);
        if (s(cpu.ecx) < s(cpu.edi))
        {
            cpu.eax = cpu.ecx * 8;
            cpu.eax += cpu.ecx;
        }
        else
        {
            cpu.eax = cpu.edi + 1;
            cpu.edx = cpu.ecx;
            put(app, b - 0x1c, cpu.eax);
            cpu.edx = reg32(s(cpu.edx) >> 31);
            cpu.eax = cpu.ecx;
            idiv(cpu, at(app, b - 0x1c));
            cpu.eax = cpu.edx * 8;
            cpu.eax += cpu.edx;
        }
        cpu.eax <<= 2;
        cpu.edx = cpu.esi + cpu.eax;
        span(b - 0x18, b - 0x1c);
        cpu.edx = at(app, b - 0x1c);
        cpu.edx += cpu.eax;
        put(app, b - 8, cpu.edx);
        cpu.edx = cpu.ecx + 1;
        cpu.eax = cpu.edi - 1;
        // The next node nearer?
        if (s(cpu.ecx) < s(cpu.eax))
        {
            cpu.eax = cpu.edx * 8;
            cpu.eax += cpu.edx;
            cpu.eax <<= 2;
            cpu.edx = cpu.esi + cpu.eax;
            span(b - 0x18, b - 0x1c);
            cpu.eax += at(app, b - 0x1c);
            if (s(cpu.eax) < s(at(app, b - 8)))
            {
                ++cpu.ecx;
                continue;
            }
        }
        else
        {
            cpu.eax = cpu.edi + 1;
            put(app, b - 0x14, cpu.eax);
            cpu.eax = cpu.edx;
            cpu.edx = reg32(s(cpu.edx) >> 31);
            idiv(cpu, at(app, b - 0x14));
            cpu.eax = cpu.edx * 8;
            cpu.eax += cpu.edx;
            cpu.edx = at(app, cpu.ebx + 8);
            put(app, b - 4, cpu.edx);
            cpu.edx = at(app, cpu.esi + cpu.eax * 4 + 8);
            put(app, b - 4, at(app, b - 4) - cpu.edx);
            cpu.edx = at(app, b - 4);
            cpu.edx = reg32(s(cpu.edx) >> 9);
            cpu.edx *= cpu.edx;
            put(app, b - 0x1c, cpu.edx);
            cpu.edx = at(app, cpu.ebx);
            cpu.eax = at(app, cpu.esi + cpu.eax * 4);
            cpu.edx -= cpu.eax;
            cpu.eax = cpu.edx;
            cpu.eax = reg32(s(cpu.eax) >> 9);
            cpu.eax *= cpu.eax;
            cpu.eax += at(app, b - 0x1c);
            if (s(cpu.eax) < s(at(app, b - 8)))
            {
                ++cpu.ecx;
                cpu.eax = cpu.ecx;
                cpu.edx = cpu.ecx;
                cpu.ecx = at(app, b - 0x14);
                cpu.edx = reg32(s(cpu.edx) >> 31);
                idiv(cpu, cpu.ecx);
                cpu.ecx = cpu.edx;
                continue;
            }
        }
        // The node before nearer?
        if (s(cpu.ecx) > 0)
        {
            cpu.eax = cpu.ecx - 1;
            put(app, b - 4, cpu.eax);
            cpu.edx = at(app, b - 4);
            cpu.eax <<= 3;
            cpu.eax += cpu.edx;
            cpu.eax <<= 2;
            cpu.edx = cpu.esi + cpu.eax;
            span(b - 0x18, b - 0x1c);
            cpu.eax += at(app, b - 0x1c);
            if (s(cpu.eax) < s(at(app, b - 8)))
                --cpu.ecx;
            continue;
        }
        cpu.eax = cpu.edi + 1;
        cpu.ecx += cpu.eax;
        cpu.edx = cpu.ecx - 1;
        put(app, b - 0x1c, cpu.eax);
        cpu.eax = cpu.edx;
        cpu.edx = reg32(s(cpu.edx) >> 31);
        idiv(cpu, at(app, b - 0x1c));
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.eax <<= 2;
        cpu.edx = cpu.esi + cpu.eax;
        span(b - 0x1c, b - 0x18);
        cpu.eax += at(app, b - 0x18);
        if (s(cpu.eax) < s(at(app, b - 8)))
            --cpu.ecx;
        cpu.eax = cpu.edi + 1;
        cpu.ecx += cpu.eax;
        put(app, b - 4, cpu.eax);
        cpu.eax = cpu.ecx;
        cpu.edx = cpu.ecx;
        cpu.ecx = at(app, b - 4);
        cpu.edx = reg32(s(cpu.edx) >> 31);
        idiv(cpu, cpu.ecx);
        cpu.ecx = cpu.edx;
    }
    cpu.eax = at(app, b - 0xc);
    put(app, cpu.eax + 0x10, cpu.ecx);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e6a0: the place on the track eax's quad: the four vertices its strip
 * ([+0x14]) names, of its block's ([+0]'s +0x70), to +0x54 (the strip's
 * second word), +0x58 (its first), +0x5c (its fourth), +0x60 (its third). */
void quadVertices(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, cpu.eax + 0x14);
    cpu.ebx = at(app, cpu.edx);
    cpu.ebx = reg32(s(cpu.ebx) >> 0x10);
    cpu.edx = cpu.ebx * 4;
    cpu.ecx = at(app, cpu.eax);
    cpu.edx -= cpu.ebx;
    cpu.ecx = at(app, cpu.ecx + 0x70);
    cpu.edx <<= 2;
    cpu.ebx = cpu.ecx + cpu.edx;
    cpu.edx = at(app, cpu.eax + 0x14);
    put(app, cpu.eax + 0x54, cpu.ebx);
    cpu.ebx = reg32(sreg32(x86::sreg16(half(app, cpu.edx))));
    cpu.edx = cpu.ebx * 4;
    cpu.edx -= cpu.ebx;
    cpu.edx <<= 2;
    cpu.ebx = cpu.ecx + cpu.edx;
    put(app, cpu.eax + 0x58, cpu.ebx);
    cpu.ebx = at(app, cpu.eax + 0x14);
    cpu.ebx = at(app, cpu.ebx + 4);
    cpu.ebx = reg32(s(cpu.ebx) >> 0x10);
    cpu.edx = cpu.ebx * 4;
    cpu.edx -= cpu.ebx;
    cpu.edx <<= 2;
    cpu.ebx = cpu.ecx + cpu.edx;
    cpu.edx = at(app, cpu.eax + 0x14);
    put(app, cpu.eax + 0x5c, cpu.ebx);
    cpu.ebx = at(app, cpu.edx + 2);
    cpu.ebx = reg32(s(cpu.ebx) >> 0x10);
    cpu.edx = cpu.ebx * 4;
    cpu.edx -= cpu.ebx;
    cpu.edx <<= 2;
    cpu.ecx += cpu.edx;
    put(app, cpu.eax + 0x60, cpu.ecx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e720: the quad a strip on across the road: the old +0x54 and +0x60
 * now +0x58 and +0x5c, the new strip's second and third words' vertices
 * +0x54 and +0x60. */
void quadStepForward(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = at(app, cpu.eax);
    cpu.edx = at(app, cpu.eax + 0x54);
    cpu.ebx = at(app, cpu.ebx + 0x70);
    put(app, cpu.eax + 0x58, cpu.edx);
    cpu.edx = at(app, cpu.eax + 0x60);
    put(app, cpu.eax + 0x5c, cpu.edx);
    cpu.edx = at(app, cpu.eax + 0x14);
    cpu.ecx = at(app, cpu.edx);
    cpu.ecx = reg32(s(cpu.ecx) >> 0x10);
    cpu.edx = cpu.ecx * 4;
    cpu.edx -= cpu.ecx;
    cpu.edx <<= 2;
    cpu.ecx = cpu.ebx + cpu.edx;
    cpu.edx = at(app, cpu.eax + 0x14);
    put(app, cpu.eax + 0x54, cpu.ecx);
    cpu.ecx = at(app, cpu.edx + 2);
    cpu.ecx = reg32(s(cpu.ecx) >> 0x10);
    cpu.edx = cpu.ecx * 4;
    cpu.edx -= cpu.ecx;
    cpu.edx <<= 2;
    cpu.ebx += cpu.edx;
    put(app, cpu.eax + 0x60, cpu.ebx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e770: the quad a strip back: the old +0x58 and +0x5c now +0x54 and
 * +0x60, the new strip's first and fourth words' vertices +0x58 and +0x5c. */
void quadStepBack(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = at(app, cpu.eax);
    cpu.edx = at(app, cpu.eax + 0x58);
    cpu.ebx = at(app, cpu.ebx + 0x70);
    put(app, cpu.eax + 0x54, cpu.edx);
    cpu.edx = at(app, cpu.eax + 0x5c);
    put(app, cpu.eax + 0x60, cpu.edx);
    cpu.edx = at(app, cpu.eax + 0x14);
    cpu.ecx = reg32(sreg32(x86::sreg16(half(app, cpu.edx))));
    cpu.edx = cpu.ecx * 4;
    cpu.edx -= cpu.ecx;
    cpu.edx <<= 2;
    cpu.ecx = cpu.ebx + cpu.edx;
    cpu.edx = at(app, cpu.eax + 0x14);
    put(app, cpu.eax + 0x58, cpu.ecx);
    cpu.ecx = at(app, cpu.edx + 4);
    cpu.ecx = reg32(s(cpu.ecx) >> 0x10);
    cpu.edx = cpu.ecx * 4;
    cpu.edx -= cpu.ecx;
    cpu.edx <<= 2;
    cpu.ebx += cpu.edx;
    put(app, cpu.eax + 0x5c, cpu.ebx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e7c0: the quad's four vertices (three floats each) in 16.16, to the
 * place on the track eax's +0x24 on: each * 65536, fistp'd through the stack. */
void quadToFixed(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    for (reg32 k = 0; k < 12; ++k)
    {
        cpu.eax = at(app, cpu.edx + 0x54 + 4 * (k / 3));
        const Float value = fp::mul(f32(app, cpu.eax + 4 * (k % 3)), f64(app, kFixed));
        cpu.eax = fistpPop(app, cpu, value);
        put(app, cpu.edx + 0x24 + 4 * k, cpu.eax);
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e8d0: the quad's vertices taken and made 16.16. */
void quadSet(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    if (!direct(app, cpu, quadVertices))
        return;
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, quadToFixed))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e8f0: the place on the track eax's block, of its node, and its node's
 * entry in it (8 bytes, from the block's first node). */
void nodeSection(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.ecx = at(app, cpu.eax + 0x10);
    cpu.ecx = reg32(s(cpu.ecx) >> 3);
    cpu.eax = cpu.ecx * 4;
    cpu.eax -= cpu.ecx;
    cpu.eax <<= 3;
    cpu.eax -= cpu.ecx;
    cpu.ecx = kBlocks;
    cpu.eax <<= 6;
    cpu.ecx += cpu.eax;
    put(app, cpu.edx, cpu.ecx);
    cpu.eax = at(app, cpu.edx + 0x10);
    cpu.eax -= at(app, cpu.ecx + 0x584);
    cpu.ebx = at(app, cpu.ecx + 0x58c);
    cpu.eax <<= 3;
    cpu.ebx += cpu.eax;
    put(app, cpu.edx + 0x20, cpu.ebx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41e940: the place on the track edx at node eax, nothing found on it. */
void resetPosition(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.eax = cpu.edx;
    put(app, cpu.edx + 0x20, 0);
    put(app, cpu.edx + 0x14, 0);
    put(app, cpu.edx + 0x18, 0);
    put(app, cpu.edx + 0x1c, 0);
    putHalf(app, cpu.edx + 0x68, 0);
    putHalf(app, cpu.edx + 0x6a, 0);
    put(app, cpu.edx + 0x6c, 0);
    put(app, cpu.edx + 0x64, 1);
    put(app, cpu.edx + 0x70, 0xffffffff);
    put(app, cpu.edx + 0x10, cpu.ecx);
    if (!direct(app, cpu, nodeSection))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

namespace
{

/* The place on the track `r` on its polygon: [block+0x594]'s entry of its
 * strip (the node's first strip's word + the strip across) to +0x18; true
 * where the polygon's kind (the low nibble at 1) is not 0xe, off the road.
 * Its registers as the code leaves them: al the kind, `into` the polygon. */
bool onPolygon(App* app, CPU& cpu, reg32 r, reg32& into)
{
    cpu.eax = at(app, r);
    cpu.eax = at(app, cpu.eax + 0x594);
    put(app, r + 0x18, cpu.eax);
    cpu.eax = at(app, r + 0x20);
    cpu.ax = half(app, cpu.eax);
    cpu.eax &= 0xffff;
    cpu.eax += at(app, r + 0x70);
    into = at(app, r + 0x18);
    cpu.eax <<= 3;
    into += cpu.eax;
    put(app, r + 0x18, into);
    cpu.al = byte(app, into + 1);
    cpu.al &= 0xf;
    return cpu.al != 0xe;
}

/* Found on the road: +0x64 2, +0x6c 0, the polygon's normal ([block+0x59c]
 * + 12 * the polygon's first byte) to r's +0x1c, `into` the register it is
 * worked out in, and r's quad made 16.16 (quadToFixed). */
bool takeNormal(App* app, CPU& cpu, reg32 r, reg32& into)
{
    put(app, r + 0x64, 2);
    cpu.eax = at(app, r);
    put(app, r + 0x6c, 0);
    cpu.eax = at(app, cpu.eax + 0x59c);
    put(app, r + 0x1c, cpu.eax);
    cpu.eax = at(app, r + 0x18);
    cpu.edx = 0;
    cpu.dl = byte(app, cpu.eax);
    cpu.eax = cpu.edx * 4;
    cpu.eax -= cpu.edx;
    into = at(app, r + 0x1c);
    cpu.eax <<= 2;
    into += cpu.eax;
    cpu.eax = r;
    put(app, r + 0x1c, into);
    return direct(app, cpu, quadToFixed);
}

/* r's strip: the node's first strip's word + the strip across, 14 bytes
 * each, of its block's +0x3c, to +0x14 (`by` the register it goes through);
 * then quadVertices. */
bool takeStrip(App* app, CPU& cpu, reg32 r, reg32& by)
{
    cpu.eax = at(app, r + 0x20);
    cpu.ax = half(app, cpu.eax);
    cpu.eax &= 0xffff;
    by = at(app, r + 0x70);
    cpu.eax += by;
    cpu.eax += cpu.eax;
    by = cpu.eax;
    cpu.eax <<= 3;
    cpu.eax -= by;
    by = cpu.eax;
    cpu.eax = at(app, r);
    cpu.eax = at(app, cpu.eax + 0x3c);
    cpu.eax += by;
    put(app, r + 0x14, cpu.eax);
    cpu.eax = r;
    return direct(app, cpu, quadVertices);
}

/* r's node walked eax nodes on (`on`) or back, round the track's ends, eax
 * counted down to -1; `step` and `wrap` the registers the code goes through. */
void walkNodes(App* app, CPU& cpu, reg32 r, bool on, reg32& step, reg32& wrap)
{
    for (;;)
    {
        --cpu.eax;
        if (cpu.eax == 0xffffffff)
            return;
        if (on)
        {
            step = at(app, r + 0x10);
            ++step;
            wrap = at(app, kNodeCount);
            put(app, r + 0x10, step);
            if (s(step) >= s(wrap))
                put(app, r + 0x10, 0);
        }
        else
        {
            step = at(app, r + 0x10);
            --step;
            put(app, r + 0x10, step);
            if (s(step) < 0)
            {
                wrap = at(app, kNodeCount);
                --wrap;
                put(app, r + 0x10, wrap);
            }
        }
    }
}

/* The place on the track at eax made a node's: x and z, 16.16, times
 * `scale`; the code's fld, fild, fmul st(1), fild, fmulp st(2). */
void placeFloats(App* app, CPU& cpu, reg32 scale, Float& x, Float& z)
{
    x86::FPU& fpu = cpu.fpu;
    const Float k = f64(app, scale);
    x = fp::mul(i32(app, cpu.eax), k);
    const Float zi = i32(app, cpu.eax + 8);
    z = fp::mul(k, zi);
}

/* The steps' sum at b - 4 walked back, as the searches end when nothing is
 * found: eax its size, on if it is negative; `onStep`, `onWrap`, `backStep`,
 * `backWrap` the registers the code goes through. */
void walkBack(App* app, CPU& cpu, reg32 r, reg32 sum, reg32& test, reg32& onStep, reg32& onWrap, reg32& backStep,
              reg32& backWrap)
{
    cpu.eax = at(app, sum);
    cpu.eax = 0u - cpu.eax;
    if (s(cpu.eax) <= 0)
        cpu.eax = at(app, sum);
    test = at(app, sum);
    test = 0u - test;
    if (s(test) > 0)
        walkNodes(app, cpu, r, true, onStep, onWrap);
    else
        walkNodes(app, cpu, r, false, backStep, backWrap);
}

}  // namespace

/* 0x41e990: the road's quad under the place eax (x and z, 16.16, made floats)
 * looked for about the place on the track edx's node -- across the road from
 * the strip it was on (or the middle one) outwards, the quad's four sides'
 * tests, then inwards from the strip before; off either end, the end at
 * +0x6c; on to the next strip while its corners come no nearer (__CHP's
 * truncation of the sum of the differences) or the polygon there is off the
 * road -- then the nodes on and back by 0x552e58's steps (1, -2, 3, ...), 11
 * of them.  Found: the polygon, its normal and the quad in 16.16 taken, 1.
 * Else the node as it was, no strip or polygon, 0. */
void findQuad(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x6c;
    const reg32 b = cpu.ebp, px = b - 0x6c, pz = b - 0x64;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    x86::FPU& fpu = cpu.fpu;
    cpu.ecx = cpu.edx;
    {
        Float x, z;
        placeFloats(app, cpu, kUnfixed, x, z);
        cpu.edi = 0;
        put(app, cpu.edx + 0x14, cpu.edi);
        cpu.eax = cpu.edx;
        put(app, cpu.edx + 0x18, cpu.edi);
        putF32(app, px, x);
        putF32(app, pz, z);
    }
    if (!direct(app, cpu, nodeSection))
        return;
    cpu.ebx = at(app, cpu.edx + 0x70);
    put(app, b - 4, cpu.edi);
    // The strip it was on, if it is still across the road, or the middle one.
    bool kept = false;
    if (s(cpu.ebx) >= 0)
    {
        cpu.eax = at(app, cpu.edx + 0x20);
        cpu.al = byte(app, cpu.eax + 2);
        cpu.eax &= 0xff;
        --cpu.eax;
        if (s(cpu.eax) > s(cpu.ebx) && at(app, cpu.edx + 0x6c) == 0)
        {
            put(app, b - 8, cpu.ebx);
            kept = true;
        }
    }
    if (!kept)
    {
        cpu.eax = at(app, cpu.ecx + 0x20);
        cpu.edx = 0;
        cpu.dl = byte(app, cpu.eax + 2);
        --cpu.edx;
        cpu.eax = cpu.edx;
        cpu.edx = reg32(s(cpu.edx) >> 31);
        cpu.eax -= cpu.edx;
        cpu.eax = reg32(s(cpu.eax) >> 1);
        put(app, b - 8, cpu.eax);
        put(app, cpu.ecx + 0x70, cpu.eax);
    }
    cpu.eax = at(app, b - 8);
    put(app, cpu.ecx + 0x6c, 0);
    put(app, cpu.ecx + 0x70, cpu.eax);
    while (s(cpu.edi) < 0xb)
    {
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, nodeSection))
            return;
        cpu.eax = at(app, b - 8);
        put(app, cpu.ecx + 0x70, cpu.eax);
        cpu.eax = at(app, cpu.ecx + 0x20);
        cpu.al = byte(app, cpu.eax + 2);
        cpu.eax &= 0xff;
        cpu.ebx = at(app, cpu.ecx + 0x70);
        cpu.edx = cpu.eax - 1;
        if (s(cpu.edx) < s(cpu.ebx))
        {
            cpu.eax -= 2;
            put(app, cpu.ecx + 0x70, cpu.eax);
        }
        cpu.esi = 0x7fffffff;
        put(app, b - 0x2c, cpu.esi);
        if (!takeStrip(app, cpu, cpu.ecx, cpu.edx))
            return;
        // Outwards.
        for (;;)
        {
            cpu.eax = at(app, cpu.ecx + 0x20);
            cpu.al = byte(app, cpu.eax + 2);
            cpu.eax &= 0xff;
            cpu.edx = at(app, cpu.ecx + 0x70);
            --cpu.eax;
            if (s(cpu.eax) < s(cpu.edx))
                break;
            cpu.ebx = at(app, cpu.ecx + 0x58);
            cpu.esi = at(app, cpu.ecx + 0x5c);
            bool inside = !sideBelow(app, cpu, cpu.ebx, cpu.esi, px, pz);
            if (inside)
            {
                cpu.edx = at(app, cpu.ecx + 0x54);
                inside = !sideBelow(app, cpu, cpu.edx, cpu.ebx, px, pz);
            }
            if (inside)
            {
                cpu.ebx = at(app, cpu.ecx + 0x60);
                inside = !sideBelow(app, cpu, cpu.esi, cpu.ebx, px, pz);
            }
            if (inside)
                inside = !sideBelow(app, cpu, cpu.ebx, cpu.edx, px, pz);
            if (inside && onPolygon(app, cpu, cpu.ecx, cpu.esi))
            {
                if (!takeNormal(app, cpu, cpu.ecx, cpu.edi))
                    return;
                cpu.eax = 1;
                return leave();
            }
            // Not in it: off the far side, or on to the next strip.
            cpu.eax = at(app, cpu.ecx + 0x20);
            cpu.al = byte(app, cpu.eax + 2);
            cpu.eax &= 0xff;
            cpu.edx = at(app, cpu.ecx + 0x70);
            --cpu.eax;
            if (cpu.eax == cpu.edx)
            {
                if (at(app, cpu.ecx + 0x6c) == 0)
                {
                    cpu.edx = at(app, cpu.ecx + 0x58);
                    cpu.eax = at(app, cpu.ecx + 0x5c);
                    if (!sideBelow(app, cpu, cpu.edx, cpu.eax, px, pz))
                        put(app, cpu.ecx + 0x6c, 2);
                }
                break;
            }
            cpu.eax = at(app, cpu.ecx + 0x54);
            absInto(app, cpu, fp::sub(f32(app, pz), f32(app, cpu.eax + 8)), b - 0x1c, b - 0x54);
            cpu.eax = at(app, cpu.ecx + 0x54);
            absInto(app, cpu, fp::sub(f32(app, px), f32(app, cpu.eax)), b - 0x14, b - 0x58);
            {
                const Float across = f32(app, b - 0x58);
                const Float along = f32(app, pz);
                cpu.eax = at(app, cpu.ecx + 0x5c);
                const Float sum = fp::add(across, f32(app, b - 0x54));
                const Float dz = fp::sub(along, f32(app, cpu.eax + 8));
                putF32(app, b - 0x60, sum);
                putF32(app, b - 0x28, dz);
                if (status(cpu, Float(0.0), f32(app, b - 0x28)) & (kBelow | kEqual))
                {
                    cpu.eax = at(app, b - 0x28);
                    put(app, b - 0x40, cpu.eax);
                }
                else
                    putF32(app, b - 0x40, -f32(app, b - 0x28));
            }
            cpu.eax = at(app, cpu.ecx + 0x5c);
            absInto(app, cpu, fp::sub(f32(app, px), f32(app, cpu.eax)), b - 0x18, b - 0x48);
            {
                const Float sum = fp::add(fp::add(f32(app, b - 0x48), f32(app, b - 0x40)), f32(app, b - 0x60));
                cpu.esi = at(app, b - 0x2c);
                if (!chopInto(app, cpu, sum, b - 0x38))
                    return;
            }
            if (s(cpu.esi) < s(at(app, b - 0x38)) && onPolygon(app, cpu, cpu.ecx, cpu.ebx))
                break;
            cpu.eax = at(app, b - 0x38);
            put(app, b - 0x2c, cpu.eax);
            cpu.eax = at(app, cpu.ecx + 0x70);
            cpu.esi = at(app, cpu.ecx + 0x14);
            ++cpu.eax;
            cpu.esi += 0xe;
            put(app, cpu.ecx + 0x70, cpu.eax);
            cpu.eax = cpu.ecx;
            put(app, cpu.ecx + 0x14, cpu.esi);
            if (!direct(app, cpu, quadStepForward))
                return;
        }
        // Inwards, from the strip before the one it started at.
        cpu.eax = at(app, b - 8);
        --cpu.eax;
        put(app, cpu.ecx + 0x70, cpu.eax);
        cpu.edx = 0x7fffffff;
        put(app, b - 0x34, cpu.edx);
        if (!takeStrip(app, cpu, cpu.ecx, cpu.edx))
            return;
        while (s(at(app, cpu.ecx + 0x70)) >= 0)
        {
            cpu.ebx = at(app, cpu.ecx + 0x58);
            cpu.edx = at(app, cpu.ecx + 0x5c);
            bool inside = !sideBelow(app, cpu, cpu.ebx, cpu.edx, px, pz);
            if (inside)
            {
                cpu.esi = at(app, cpu.ecx + 0x54);
                inside = !sideBelow(app, cpu, cpu.esi, cpu.ebx, px, pz);
            }
            if (inside)
            {
                cpu.ebx = at(app, cpu.ecx + 0x60);
                inside = !sideBelow(app, cpu, cpu.edx, cpu.ebx, px, pz);
            }
            if (inside)
                inside = !sideBelow(app, cpu, cpu.ebx, cpu.esi, px, pz);
            if (inside && onPolygon(app, cpu, cpu.ecx, cpu.edx))
            {
                put(app, cpu.ecx + 0x64, 2);
                cpu.eax = at(app, cpu.ecx);
                put(app, cpu.ecx + 0x6c, 0);
                cpu.eax = at(app, cpu.eax + 0x59c);
                cpu.edx = at(app, cpu.ecx + 0x18);
                put(app, cpu.ecx + 0x1c, cpu.eax);
                cpu.dl = byte(app, cpu.edx);
                cpu.edx &= 0xff;
                cpu.eax = cpu.edx * 4;
                cpu.eax -= cpu.edx;
                cpu.ebx = at(app, cpu.ecx + 0x1c);
                cpu.eax <<= 2;
                cpu.ebx += cpu.eax;
                cpu.eax = cpu.ecx;
                put(app, cpu.ecx + 0x1c, cpu.ebx);
                if (!direct(app, cpu, quadToFixed))
                    return;
                cpu.eax = 1;
                return leave();
            }
            if (at(app, cpu.ecx + 0x70) == 0)
            {
                if (at(app, cpu.ecx + 0x6c) == 0)
                {
                    cpu.edx = at(app, cpu.ecx + 0x60);
                    cpu.eax = at(app, cpu.ecx + 0x54);
                    if (!sideBelow(app, cpu, cpu.edx, cpu.eax, px, pz))
                        put(app, cpu.ecx + 0x6c, 1);
                }
                break;
            }
            cpu.eax = at(app, cpu.ecx + 0x54);
            absInto(app, cpu, fp::sub(f32(app, pz), f32(app, cpu.eax + 8)), b - 0x10, b - 0x3c);
            cpu.eax = at(app, cpu.ecx + 0x54);
            absInto(app, cpu, fp::sub(f32(app, px), f32(app, cpu.eax)), b - 0x20, b - 0x50);
            {
                const Float across = f32(app, b - 0x50);
                const Float along = f32(app, pz);
                cpu.eax = at(app, cpu.ecx + 0x5c);
                const Float sum = fp::add(across, f32(app, b - 0x3c));
                const Float dz = fp::sub(along, f32(app, cpu.eax + 8));
                putF32(app, b - 0x5c, sum);
                putF32(app, b - 0x24, dz);
                if (status(cpu, Float(0.0), f32(app, b - 0x24)) & (kBelow | kEqual))
                {
                    cpu.eax = at(app, b - 0x24);
                    put(app, b - 0x44, cpu.eax);
                }
                else
                    putF32(app, b - 0x44, -f32(app, b - 0x24));
            }
            cpu.eax = at(app, cpu.ecx + 0x5c);
            absInto(app, cpu, fp::sub(f32(app, px), f32(app, cpu.eax)), b - 0xc, b - 0x4c);
            {
                const Float sum = fp::add(fp::add(f32(app, b - 0x4c), f32(app, b - 0x44)), f32(app, b - 0x5c));
                cpu.edx = at(app, b - 0x34);
                if (!chopInto(app, cpu, sum, b - 0x30))
                    return;
            }
            if (s(cpu.edx) < s(at(app, b - 0x30)) && onPolygon(app, cpu, cpu.ecx, cpu.esi))
                break;
            cpu.eax = at(app, b - 0x30);
            put(app, b - 0x34, cpu.eax);
            cpu.eax = at(app, cpu.ecx + 0x14);
            cpu.edx = at(app, cpu.ecx + 0x70);
            cpu.eax -= 0xe;
            --cpu.edx;
            put(app, cpu.ecx + 0x14, cpu.eax);
            cpu.eax = cpu.ecx;
            put(app, cpu.ecx + 0x70, cpu.edx);
            if (!direct(app, cpu, quadStepBack))
                return;
        }
        // The next node of the steps.
        cpu.eax = cpu.edi * 4;
        cpu.ebx = at(app, cpu.eax + kSteps);
        cpu.eax = s(cpu.ebx) > 0 ? cpu.ebx : 0u - cpu.ebx;
        if (s(at(app, cpu.edi * 4 + kSteps)) > 0)
            walkNodes(app, cpu, cpu.ecx, true, cpu.edx, cpu.ebx);
        else
            walkNodes(app, cpu, cpu.ecx, false, cpu.esi, cpu.edx);
        cpu.ebx = at(app, b - 4);
        cpu.ebx += at(app, cpu.edi * 4 + kSteps);
        ++cpu.edi;
        put(app, b - 4, cpu.ebx);
    }
    // None: the steps walked back.
    walkBack(app, cpu, cpu.ecx, b - 4, cpu.edx, cpu.esi, cpu.edi, cpu.edx, cpu.edx);
    cpu.eax = cpu.ecx;
    if (!direct(app, cpu, nodeSection))
        return;
    put(app, cpu.ecx + 0x14, 0);
    put(app, cpu.ecx + 0x18, 0);
    cpu.eax = 0;
    leave();
}

/* 0x41f090: the road's quad under the place eax looked for from the place on
 * the track edx's node -- across the road from the first strip outwards,
 * then the nodes on and back by 0x552e88's steps, 7 of them -- the quad's
 * sides tested in turn.  Found: the polygon, its normal and the quad in
 * 16.16, 1; else the node as it was, no strip or polygon, 0. */
void findQuadNearby(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
    const reg32 b = cpu.ebp, px = b - 0x14, pz = b - 0xc;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    {
        Float x, z;
        placeFloats(app, cpu, kUnfixedToo, x, z);
        cpu.ebx = 0;
        put(app, cpu.edx + 0x14, cpu.ebx);
        put(app, b - 4, cpu.ebx);
        put(app, cpu.edx + 0x18, cpu.ebx);
        put(app, b - 8, cpu.ebx);
        put(app, cpu.edx + 0x6c, cpu.ebx);
        putF32(app, pz, z);
        putF32(app, px, x);
    }
    bool atNode = true;  // the code's entry at 0x41f20c, else at 0x41f245
    for (;;)
    {
        if (atNode)
        {
            cpu.eax = cpu.edx;
            if (!direct(app, cpu, nodeSection))
                return;
            cpu.eax = at(app, b - 8);
            put(app, cpu.edx + 0x70, cpu.eax);
            if (!takeStrip(app, cpu, cpu.edx, cpu.ecx))
                return;
        }
        atNode = false;
        cpu.eax = at(app, cpu.edx + 0x20);
        cpu.al = byte(app, cpu.eax + 2);
        cpu.eax &= 0xff;
        cpu.edi = at(app, cpu.edx + 0x70);
        --cpu.eax;
        if (s(cpu.eax) >= s(cpu.edi))
        {
            cpu.edi = at(app, cpu.edx + 0x58);
            cpu.esi = at(app, cpu.edx + 0x5c);
            bool inside = !sideBelow(app, cpu, cpu.edi, cpu.esi, px, pz);
            if (inside)
            {
                cpu.ecx = at(app, cpu.edx + 0x54);
                inside = !sideBelow(app, cpu, cpu.ecx, cpu.edi, px, pz);
            }
            if (inside)
            {
                cpu.edi = at(app, cpu.edx + 0x60);
                inside = !sideBelow(app, cpu, cpu.esi, cpu.edi, px, pz);
            }
            if (inside)
                inside = !sideBelow(app, cpu, cpu.edi, cpu.ecx, px, pz);
            if (inside && onPolygon(app, cpu, cpu.edx, cpu.esi))
            {
                cpu.eax = at(app, cpu.edx);
                put(app, cpu.edx + 0x64, 2);
                cpu.eax = at(app, cpu.eax + 0x59c);
                put(app, cpu.edx + 0x1c, cpu.eax);
                cpu.eax = at(app, cpu.edx + 0x18);
                cpu.ecx = 0;
                cpu.cl = byte(app, cpu.eax);
                cpu.eax = cpu.ecx * 4;
                cpu.eax -= cpu.ecx;
                cpu.edi = at(app, cpu.edx + 0x1c);
                cpu.eax <<= 2;
                cpu.edi += cpu.eax;
                cpu.eax = cpu.edx;
                put(app, cpu.edx + 0x1c, cpu.edi);
                if (!direct(app, cpu, quadToFixed))
                    return;
                cpu.eax = 1;
                return leave();
            }
            // On to the next strip.
            cpu.eax = at(app, cpu.edx + 0x14);
            cpu.ecx = at(app, cpu.edx + 0x70);
            cpu.eax += 0xe;
            ++cpu.ecx;
            put(app, cpu.edx + 0x14, cpu.eax);
            cpu.eax = cpu.edx;
            put(app, cpu.edx + 0x70, cpu.ecx);
            if (!direct(app, cpu, quadStepForward))
                return;
            continue;
        }
        // Across the road: the next node of the steps.
        cpu.eax = cpu.ebx * 4;
        cpu.esi = at(app, cpu.eax + kNearSteps);
        cpu.eax = s(cpu.esi) > 0 ? cpu.esi : 0u - cpu.esi;
        if (s(at(app, cpu.ebx * 4 + kNearSteps)) > 0)
            walkNodes(app, cpu, cpu.edx, true, cpu.ecx, cpu.esi);
        else
            walkNodes(app, cpu, cpu.edx, false, cpu.edi, cpu.ecx);
        cpu.esi = at(app, b - 4);
        cpu.esi += at(app, cpu.ebx * 4 + kNearSteps);
        ++cpu.ebx;
        put(app, b - 4, cpu.esi);
        if (s(cpu.ebx) >= 7)
            break;
        atNode = true;
    }
    walkBack(app, cpu, cpu.edx, b - 4, cpu.ecx, cpu.edi, cpu.ebx, cpu.esi, cpu.ecx);
    cpu.eax = cpu.edx;
    if (!direct(app, cpu, nodeSection))
        return;
    put(app, cpu.edx + 0x14, 0);
    put(app, cpu.edx + 0x18, 0);
    cpu.eax = 0;
    leave();
}

/* 0x41f330: the road's quad under the place eax looked for on the nodes the
 * place on the track edx's node (ebx, set) links to: for each of 0x552ea8's
 * steps (0, 1, -2) along the nodes, of the node's two links (the words at 4
 * and 6 of its entry, -1 for none) the nearer one (0x41aab0's distance,
 * unsigned) followed and searched (followNode, findQuad) -- 1 when found;
 * else the place as it was copied back, without an entry.  0 for none. */
void findQuadByLinks(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x80;
    cpu.ebp -= 0x82;
    const reg32 b = cpu.ebp;
    const auto leave = [&] {
        cpu.esp = cpu.ebp + 0x82;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    put(app, b + 0x7a, cpu.eax);
    put(app, b + 0x7e, cpu.edx);
    put(app, cpu.edx + 0x70, 0xffffffff);
    put(app, cpu.edx + 0x10, cpu.ebx);
    cpu.ebx = 0;
    for (;;)
    {
        cpu.eax = cpu.ebx * 4;
        cpu.edx = at(app, cpu.eax + kLinkSteps);
        cpu.eax = s(cpu.edx) > 0 ? cpu.edx : 0u - cpu.edx;
        const bool on = s(at(app, cpu.ebx * 4 + kLinkSteps)) > 0;
        for (;;)
        {
            --cpu.eax;
            if (cpu.eax == 0xffffffff)
                break;
            if (on)
            {
                cpu.edx = at(app, b + 0x7e);
                cpu.esi = at(app, cpu.edx + 0x10);
                ++cpu.esi;
                cpu.edi = at(app, kNodeCount);
                put(app, cpu.edx + 0x10, cpu.esi);
                if (s(cpu.esi) >= s(cpu.edi))
                    put(app, cpu.edx + 0x10, 0);
            }
            else
            {
                cpu.edx = at(app, b + 0x7e);
                cpu.ecx = at(app, cpu.edx + 0x10);
                --cpu.ecx;
                put(app, cpu.edx + 0x10, cpu.ecx);
                if (s(cpu.ecx) < 0)
                {
                    cpu.edx = at(app, kNodeCount);
                    cpu.ecx = at(app, b + 0x7e);
                    --cpu.edx;
                    put(app, cpu.ecx + 0x10, cpu.edx);
                }
            }
        }
        cpu.eax = at(app, b + 0x7e);
        if (!direct(app, cpu, nodeSection))
            return;
        cpu.eax = at(app, b + 0x7e);
        cpu.eax = at(app, cpu.eax + 0x20);
        cpu.dx = half(app, cpu.eax + 4);
        if (cpu.dx != 0xffff || cpu.dx != half(app, cpu.eax + 6))
        {
            // The place kept in the frame; the nearer link.
            cpu.ecx = 0x1e;
            cpu.edi = b + 2;
            cpu.esi = at(app, b + 0x7e);
            moveWords(app, cpu);
            cpu.eax = at(app, b + 0x7e);
            cpu.ecx = 0xffffffff;
            cpu.eax = at(app, cpu.eax + 0x20);
            cpu.esi = cpu.ecx;
            cpu.di = half(app, cpu.eax + 4);
            if (cpu.di != 0xffff)
            {
                cpu.edx = at(app, cpu.eax + 2);
                cpu.edx = reg32(s(cpu.edx) >> 0x10);
                cpu.eax = cpu.edx * 8;
                cpu.edx += cpu.eax;
                cpu.eax = at(app, kNodes);
                cpu.edx <<= 2;
                cpu.eax += cpu.edx;
                cpu.edx = at(app, b + 0x7a);
                if (!byAddress(app, cpu, kDistance))
                    return;
                cpu.esi = cpu.eax;
            }
            cpu.eax = at(app, b + 0x7e);
            cpu.eax = at(app, cpu.eax + 0x20);
            if (half(app, cpu.eax + 6) != 0xffff)
            {
                cpu.edx = at(app, cpu.eax + 4);
                cpu.edx = reg32(s(cpu.edx) >> 0x10);
                cpu.eax = cpu.edx * 8;
                cpu.eax += cpu.edx;
                cpu.edx = at(app, kNodes);
                cpu.eax <<= 2;
                cpu.eax += cpu.edx;
                cpu.edx = at(app, b + 0x7a);
                if (!byAddress(app, cpu, kDistance))
                    return;
                cpu.ecx = cpu.eax;
            }
            const reg32 link = cpu.esi < cpu.ecx ? 2 : 4;
            cpu.eax = at(app, b + 0x7e);
            cpu.eax = at(app, cpu.eax + 0x20);
            cpu.eax = at(app, cpu.eax + link);
            cpu.edx = at(app, b + 0x7e);
            cpu.eax = reg32(s(cpu.eax) >> 0x10);
            put(app, cpu.edx + 0x10, cpu.eax);
            cpu.eax = at(app, b + 0x7a);
            if (!direct(app, cpu, followNode))
                return;
            cpu.edx = at(app, b + 0x7e);
            cpu.eax = at(app, b + 0x7a);
            if (!direct(app, cpu, findQuad))
                return;
            if (cpu.eax != 0)
            {
                cpu.eax = 1;
                return leave();
            }
            // Not there: the place as it was, without its entry.
            cpu.ecx = 0x1e;
            cpu.edi = at(app, b + 0x7e);
            cpu.esi = b + 2;
            cpu.eax = at(app, b + 0x7e);
            moveWords(app, cpu);
            put(app, cpu.eax + 0x20, 0);
            if (!direct(app, cpu, nodeSection))
                return;
        }
        ++cpu.ebx;
        if (s(cpu.ebx) >= 3)
            break;
    }
    cpu.eax = 0;
    leave();
}

/* 0x41f500: the place eax on the track edx, where findQuad and
 * findQuadByLinks found nothing: off an end of the road (+0x6c 1 the first
 * strip, else the last), that end's strip and polygon taken as found
 * (quadSet); more than 1.0 (0x41aab0's distance) from its node, the node
 * looked for afresh and searched (nearestNodeCoarse, nearestNode, findQuad);
 * still no strip, a place a 32nd of the way from it to the node searched
 * (findQuadNearby), and 0x42d500 traced "c" if it is found there.  ebx left
 * as the code leaves it. */
void findQuadFallback(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    const reg32 b = cpu.ebp;
    cpu.esi = cpu.eax;
    cpu.ecx = cpu.edx;
    if (at(app, cpu.edx + 0x6c) != 0)
    {
        if (at(app, cpu.ecx + 0x6c) == 1)
            cpu.eax = 0;
        else
        {
            cpu.eax = at(app, cpu.ecx + 0x20);
            cpu.al = byte(app, cpu.eax + 2);
            cpu.eax &= 0xff;
            --cpu.eax;
        }
        put(app, cpu.ecx + 0x70, cpu.eax);
        cpu.eax = at(app, cpu.ecx + 0x20);
        cpu.edx = 0;
        cpu.edi = at(app, cpu.ecx + 0x70);
        cpu.dx = half(app, cpu.eax);
        cpu.eax = cpu.edx + cpu.edi;
        cpu.eax += cpu.eax;
        cpu.edx = cpu.eax;
        cpu.eax <<= 3;
        cpu.eax -= cpu.edx;
        cpu.edx = at(app, cpu.ecx);
        cpu.edx = at(app, cpu.edx + 0x3c);
        cpu.edx += cpu.eax;
        cpu.eax = at(app, cpu.ecx);
        put(app, cpu.ecx + 0x14, cpu.edx);
        cpu.eax = at(app, cpu.eax + 0x594);
        put(app, cpu.ecx + 0x18, cpu.eax);
        cpu.eax = at(app, cpu.ecx + 0x20);
        cpu.ax = half(app, cpu.eax);
        cpu.eax &= 0xffff;
        cpu.eax += at(app, cpu.ecx + 0x70);
        cpu.ebx = at(app, cpu.ecx + 0x18);
        cpu.eax <<= 3;
        put(app, cpu.ecx + 0x64, 2);
        cpu.ebx += cpu.eax;
        cpu.eax = at(app, cpu.ecx);
        put(app, cpu.ecx + 0x18, cpu.ebx);
        cpu.eax = at(app, cpu.eax + 0x59c);
        put(app, cpu.ecx + 0x1c, cpu.eax);
        cpu.eax = at(app, cpu.ecx + 0x18);
        cpu.edx = 0;
        cpu.dl = byte(app, cpu.eax);
        cpu.eax = cpu.edx * 4;
        cpu.eax -= cpu.edx;
        cpu.edi = at(app, cpu.ecx + 0x1c);
        cpu.eax <<= 2;
        cpu.edi += cpu.eax;
        cpu.eax = cpu.ecx;
        put(app, cpu.ecx + 0x1c, cpu.edi);
        if (!direct(app, cpu, quadSet))
            return;
    }
    cpu.edx = at(app, cpu.ecx + 0x10);
    cpu.eax = cpu.edx * 8;
    cpu.eax += cpu.edx;
    cpu.edx = at(app, kNodes);
    cpu.eax <<= 2;
    cpu.eax += cpu.edx;
    cpu.edx = cpu.esi;
    if (!byAddress(app, cpu, kDistance))
        return;
    if (s(cpu.eax) > 0x10000)
    {
        cpu.edx = cpu.ecx;
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, nearestNodeCoarse))
            return;
        cpu.edx = cpu.ecx;
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, nearestNode))
            return;
        cpu.edx = cpu.ecx;
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, findQuad))
            return;
    }
    if (at(app, cpu.ecx + 0x14) == 0)
    {
        cpu.edx = at(app, cpu.ecx + 0x10);
        cpu.eax = cpu.edx * 8;
        cpu.ebx = at(app, kNodes);
        cpu.eax += cpu.edx;
        cpu.eax = at(app, cpu.ebx + cpu.eax * 4);
        cpu.edx = at(app, cpu.esi);
        cpu.eax -= cpu.edx;
        cpu.eax = reg32(s(cpu.eax) >> 5);
        cpu.edx += cpu.eax;
        put(app, b - 0xc, cpu.edx);
        cpu.edx = at(app, cpu.ecx + 0x10);
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.eax = at(app, cpu.ebx + cpu.eax * 4 + 8);
        cpu.ebx = at(app, cpu.esi + 8);
        cpu.eax -= cpu.ebx;
        cpu.eax = reg32(s(cpu.eax) >> 5);
        cpu.edx = cpu.ebx + cpu.eax;
        cpu.eax = b - 0xc;
        put(app, b - 4, cpu.edx);
        cpu.edx = cpu.ecx;
        if (!direct(app, cpu, findQuadNearby))
            return;
        if (cpu.eax != 0)
        {
            push(app, cpu, kNotFound);
            if (!byAddress(app, cpu, kTrace))
                return;
            cpu.esp += 4;
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41f650: the place eax found on the track edx: the nearest node
 * followed, its quad searched for (findQuad, findQuadByLinks from the node it
 * was at, findQuadFallback); +0x68 whether the node changed, +0x6a whether
 * it or the strip did; that first, a word, in eax. */
void locateOnRoad(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 b = cpu.ebp;
    cpu.esi = cpu.eax;
    cpu.ecx = cpu.edx;
    cpu.eax = at(app, cpu.edx + 0x14);
    put(app, b - 4, cpu.eax);
    cpu.eax = cpu.esi;
    cpu.edi = at(app, cpu.edx + 0x10);
    if (!direct(app, cpu, followNode))
        return;
    cpu.edx = cpu.ecx;
    cpu.eax = cpu.esi;
    if (!direct(app, cpu, findQuad))
        return;
    if (cpu.eax == 0)
    {
        cpu.ebx = cpu.edi;
        cpu.edx = cpu.ecx;
        cpu.eax = cpu.esi;
        if (!direct(app, cpu, findQuadByLinks))
            return;
        if (cpu.eax == 0)
        {
            cpu.ebx = cpu.edi;
            cpu.edx = cpu.ecx;
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, findQuadFallback))
                return;
        }
    }
    cpu.eax = cpu.edi != at(app, cpu.ecx + 0x10) ? 1 : 0;
    cpu.ebx = at(app, cpu.ecx + 0x14);
    putHalf(app, cpu.ecx + 0x68, cpu.ax);
    cpu.eax = at(app, b - 4);
    if (cpu.eax != cpu.ebx || half(app, cpu.ecx + 0x68) != 0)
        cpu.eax = 1;
    else
        cpu.eax ^= cpu.ebx;
    putHalf(app, cpu.ecx + 0x6a, cpu.ax);
    cpu.eax = at(app, cpu.ecx + 0x66);
    cpu.eax = reg32(s(cpu.eax) >> 0x10);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41f6e0: the place on the track edx brought to the place eax, followed
 * (ebx, kept at +0x74) or not.  Followed and on a quad still -- the place on
 * the inner side of its four sides, in 16.16 (fixedMul) -- nothing changed,
 * 0; else found again (locateOnRoad).  Not followed: the place reset to it
 * and its nearest node (followNode).  The place copied to +4. */
void updatePosition(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 b = cpu.ebp, place = b - 4;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    put(app, place, cpu.eax);
    cpu.ecx = cpu.edx;
    put(app, cpu.edx + 0x74, cpu.ebx);
    if (cpu.ebx == 0)
    {
        put(app, cpu.ecx + 0x20, 0);
        put(app, cpu.ecx + 0x14, 0);
        put(app, cpu.ecx + 0x18, 0);
        cpu.esi = at(app, place);
        put(app, cpu.ecx + 0x64, 1);
        cpu.eax = at(app, place);
        cpu.edi = cpu.ecx + 4;
        put(app, cpu.ecx + 0x70, 0xffffffff);
        cpu.edx = cpu.ecx;
        moveWords(app, cpu, 3);
        if (!direct(app, cpu, followNode))
            return;
        return leave();
    }
    cpu.edx = at(app, cpu.edx + 0x14);
    bool inside = cpu.edx != 0;
    if (inside)
    {
        cpu.edx = at(app, cpu.eax + 8);
        cpu.ebx = at(app, cpu.ecx + 0x44);
        cpu.esi = at(app, cpu.ecx + 0x3c);
        cpu.eax = at(app, cpu.ecx + 0x30);
        cpu.edx -= cpu.ebx;
        cpu.eax -= cpu.esi;
        fixedMul(cpu);
        cpu.ebx = cpu.eax;
        cpu.edx = at(app, cpu.ecx + 0x38);
        cpu.eax = at(app, place);
        cpu.edi = at(app, cpu.ecx + 0x44);
        cpu.esi = at(app, cpu.ecx + 0x3c);
        cpu.eax = at(app, cpu.eax);
        cpu.edx -= cpu.edi;
        cpu.eax -= cpu.esi;
        fixedMul(cpu);
        inside = s(cpu.ebx) <= s(cpu.eax);
    }
    if (inside)
    {
        cpu.edx = at(app, place);
        cpu.edi = at(app, cpu.ecx + 0x38);
        cpu.eax = at(app, cpu.ecx + 0x24);
        cpu.ebx = at(app, cpu.ecx + 0x30);
        cpu.edx = at(app, cpu.edx + 8);
        cpu.eax -= cpu.ebx;
        cpu.edx -= cpu.edi;
        fixedMul(cpu);
        cpu.ebx = cpu.eax;
        cpu.edx = at(app, cpu.ecx + 0x2c);
        cpu.eax = at(app, place);
        cpu.esi = at(app, cpu.ecx + 0x38);
        cpu.edi = at(app, cpu.ecx + 0x30);
        cpu.eax = at(app, cpu.eax);
        cpu.edx -= cpu.esi;
        cpu.eax -= cpu.edi;
        fixedMul(cpu);
        inside = s(cpu.ebx) <= s(cpu.eax);
    }
    if (inside)
    {
        cpu.eax = at(app, place);
        cpu.edx = at(app, cpu.ecx + 0x50);
        cpu.eax = at(app, cpu.eax + 8);
        cpu.eax -= cpu.edx;
        cpu.ebx = at(app, cpu.ecx + 0x48);
        cpu.edx = cpu.eax;
        cpu.eax = at(app, cpu.ecx + 0x3c);
        cpu.eax -= cpu.ebx;
        fixedMul(cpu);
        cpu.ebx = cpu.eax;
        cpu.edx = at(app, cpu.ecx + 0x44);
        cpu.eax = at(app, place);
        cpu.esi = at(app, cpu.ecx + 0x50);
        cpu.edi = at(app, cpu.ecx + 0x48);
        cpu.eax = at(app, cpu.eax);
        cpu.edx -= cpu.esi;
        cpu.eax -= cpu.edi;
        fixedMul(cpu);
        inside = s(cpu.ebx) <= s(cpu.eax);
    }
    if (inside)
    {
        cpu.eax = at(app, place);
        cpu.edx = at(app, cpu.ecx + 0x2c);
        cpu.eax = at(app, cpu.eax + 8);
        cpu.eax -= cpu.edx;
        cpu.ebx = at(app, cpu.ecx + 0x24);
        cpu.edx = cpu.eax;
        cpu.eax = at(app, cpu.ecx + 0x48);
        cpu.eax -= cpu.ebx;
        fixedMul(cpu);
        cpu.ebx = cpu.eax;
        cpu.edx = at(app, cpu.ecx + 0x50);
        cpu.eax = at(app, place);
        cpu.esi = at(app, cpu.ecx + 0x2c);
        cpu.edi = at(app, cpu.ecx + 0x24);
        cpu.eax = at(app, cpu.eax);
        cpu.edx -= cpu.esi;
        cpu.eax -= cpu.edi;
        fixedMul(cpu);
        inside = s(cpu.ebx) <= s(cpu.eax);
    }
    if (inside)
    {
        putHalf(app, cpu.ecx + 0x68, 0);
        putHalf(app, cpu.ecx + 0x6a, 0);
        cpu.eax = 0;
        return leave();
    }
    cpu.eax = at(app, place);
    cpu.edx = cpu.ecx;
    cpu.esi = at(app, place);
    if (!direct(app, cpu, locateOnRoad))
        return;
    cpu.edi = cpu.ecx + 4;
    moveWords(app, cpu, 3);
    leave();
}

namespace
{

/* findMatchingQuad's end: the place found on its polygon (takeNormal), and
 * its height there -- the normal's first word doubled, or the node's byte
 * at 0x11 << 9 without a strip, the node's offset worked out in eax
 * (`inEax`, the across search's) or in edx -- found when not 0. */
bool matchedEnd(App* app, CPU& cpu, reg32 r, reg32& into, bool inEax, bool& found)
{
    if (!takeNormal(app, cpu, r, into))
        return false;
    if (at(app, r + 0x14) == 0 && inEax)
    {
        cpu.edx = at(app, r + 0x10);
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.edx = at(app, kNodes);
        cpu.eax = at(app, cpu.edx + cpu.eax * 4 + 0xe);
        cpu.eax = reg32(s(cpu.eax) >> 0x18);
        cpu.eax <<= 9;
    }
    else if (at(app, r + 0x14) == 0)
    {
        cpu.edx = at(app, r + 0x10);
        cpu.eax = cpu.edx * 8;
        cpu.edx += cpu.eax;
        cpu.eax = at(app, kNodes);
        cpu.edx <<= 2;
        cpu.eax = at(app, cpu.edx + cpu.eax + 0xe);
        cpu.eax = reg32(s(cpu.eax) >> 0x18);
        cpu.eax <<= 9;
    }
    else
    {
        cpu.eax = at(app, r + 0x1c);
        cpu.eax = at(app, cpu.eax);
        cpu.eax = reg32(s(cpu.eax) >> 0x10);
        cpu.eax += cpu.eax;
    }
    found = cpu.eax != 0;
    return true;
}

}  // namespace

/* 0x41f870: the place on the track edx copied to ebx and moved by eax: 4 a
 * node on, 8 a node back, then every strip across the road whose quad has
 * two or more corners within 0.2 (x, y and z) of edx's; 2 the strip on, 1
 * the strip back.  1 when found on a polygon with a height (matchedEnd);
 * else 0, or the height's 0. */
void findMatchingQuad(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x20;
    const reg32 b = cpu.ebp;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    x86::FPU& fpu = cpu.fpu;
    cpu.ecx = 0x1e;
    cpu.edi = cpu.ebx;
    cpu.esi = cpu.edx;
    moveWords(app, cpu);
    if (cpu.eax == 4 || cpu.eax == 8)
    {
        if (cpu.eax == 4)
        {
            cpu.eax = 1;
            walkNodes(app, cpu, cpu.ebx, true, cpu.ecx, cpu.esi);
        }
        else
        {
            cpu.ecx = 1;
            for (;;)
            {
                --cpu.ecx;
                if (cpu.ecx == 0xffffffff)
                    break;
                cpu.edi = at(app, cpu.ebx + 0x10);
                --cpu.edi;
                put(app, cpu.ebx + 0x10, cpu.edi);
                if (s(cpu.edi) < 0)
                {
                    cpu.eax = at(app, kNodeCount);
                    --cpu.eax;
                    put(app, cpu.ebx + 0x10, cpu.eax);
                }
            }
        }
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, nodeSection))
            return;
        put(app, cpu.ebx + 0x70, 0);
        for (;;)
        {
            cpu.eax = at(app, cpu.ebx + 0x20);
            cpu.ecx = 0;
            cpu.cl = byte(app, cpu.eax + 2);
            cpu.esi = at(app, cpu.ebx + 0x70);
            --cpu.ecx;
            if (s(cpu.ecx) < s(cpu.esi))
                break;
            cpu.ax = half(app, cpu.eax);
            cpu.eax &= 0xffff;
            cpu.eax += cpu.esi;
            cpu.eax += cpu.eax;
            cpu.ecx = cpu.eax;
            cpu.eax <<= 3;
            cpu.eax -= cpu.ecx;
            cpu.ecx = cpu.eax;
            cpu.eax = at(app, cpu.ebx);
            cpu.eax = at(app, cpu.eax + 0x3c);
            cpu.eax += cpu.ecx;
            put(app, cpu.ebx + 0x14, cpu.eax);
            cpu.eax = cpu.ebx;
            if (!direct(app, cpu, quadVertices))
                return;
            if (onPolygon(app, cpu, cpu.ebx, cpu.esi))
            {
                // The corners within 0.2 of the other's, counted at b - 0x20.
                cpu.edi = 0;
                cpu.esi = 0;
                put(app, b - 0x20, cpu.edi);
                cpu.ecx = 0;
                for (;;)
                {
                    cpu.eax = cpu.ebx + 0x54;
                    if (cpu.eax != 0)
                        cpu.eax = cpu.edx + 0x54;
                    if (cpu.eax != 0)
                    {
                        cpu.eax = cpu.ecx * 4;
                        cpu.eax += cpu.ebx;
                        if (at(app, cpu.eax + 0x54) != 0)
                        {
                            cpu.edi = cpu.esi * 4;
                            cpu.edi += cpu.edx;
                            put(app, b - 0x14, cpu.edi);
                            if (at(app, cpu.edi + 0x54) != 0)
                            {
                                cpu.edi = at(app, cpu.eax + 0x54);
                                cpu.eax = at(app, b - 0x14);
                                cpu.eax = at(app, cpu.eax + 0x54);
                                bool within = true;
                                const reg32 slots[3][2] = {{b - 8, b - 0x10}, {b - 0xc, b - 0x1c}, {b - 4, b - 0x18}};
                                for (reg32 k = 0; k < 3 && within; ++k)
                                {
                                    if (k != 0)
                                    {
                                        cpu.edi = at(app, cpu.ebx + cpu.ecx * 4 + 0x54);
                                        cpu.eax = at(app, cpu.edx + cpu.esi * 4 + 0x54);
                                    }
                                    absInto(app, cpu, fp::sub(f32(app, cpu.edi + 4 * k), f32(app, cpu.eax + 4 * k)),
                                            slots[k][0], slots[k][1]);
                                    within = (status(cpu, f32(app, slots[k][1]), f64(app, kNear)) & kBelow) != 0;
                                }
                                if (within)
                                    put(app, b - 0x20, at(app, b - 0x20) + 1);
                            }
                        }
                    }
                    ++cpu.ecx;
                    if (s(cpu.ecx) < 4)
                        continue;
                    ++cpu.esi;
                    if (s(cpu.esi) >= 4)
                        break;
                    cpu.ecx = 0;
                }
                if (s(at(app, b - 0x20)) >= 2)
                {
                    bool found = false;
                    if (!matchedEnd(app, cpu, cpu.ebx, cpu.esi, true, found))
                        return;
                    if (found)
                        cpu.eax = 1;
                    return leave();
                }
            }
            put(app, cpu.ebx + 0x70, at(app, cpu.ebx + 0x70) + 1);
        }
        cpu.eax = 0;
        return leave();
    }
    if (cpu.eax == 2)
    {
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, nodeSection) || !takeStrip(app, cpu, cpu.ebx, cpu.edx))
            return;
        cpu.eax = at(app, cpu.ebx + 0x20);
        cpu.al = byte(app, cpu.eax + 2);
        cpu.eax &= 0xff;
        cpu.edx = at(app, cpu.ebx + 0x70);
        --cpu.eax;
        if (s(cpu.eax) > s(cpu.edx))
        {
            cpu.ecx = at(app, cpu.ebx + 0x14);
            cpu.esi = at(app, cpu.ebx + 0x70);
            cpu.ecx += 0xe;
            ++cpu.esi;
            put(app, cpu.ebx + 0x14, cpu.ecx);
            cpu.eax = cpu.ebx;
            put(app, cpu.ebx + 0x70, cpu.esi);
            if (!direct(app, cpu, quadStepForward))
                return;
            if (onPolygon(app, cpu, cpu.ebx, cpu.edx))
            {
                bool found = false;
                if (!matchedEnd(app, cpu, cpu.ebx, cpu.ecx, false, found))
                    return;
                if (found)
                    cpu.eax = 1;
                return leave();
            }
        }
        cpu.eax = 0;
        return leave();
    }
    if (cpu.eax == 1)
    {
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, nodeSection))
            return;
        cpu.eax = at(app, cpu.ebx + 0x20);
        cpu.edx = 0;
        cpu.edi = at(app, cpu.ebx + 0x70);
        cpu.dx = half(app, cpu.eax);
        cpu.eax = cpu.edx + cpu.edi;
        cpu.eax += cpu.eax;
        cpu.edx = cpu.eax;
        cpu.eax <<= 3;
        cpu.eax -= cpu.edx;
        cpu.edx = cpu.eax;
        cpu.eax = at(app, cpu.ebx);
        cpu.eax = at(app, cpu.eax + 0x3c);
        cpu.eax += cpu.edx;
        put(app, cpu.ebx + 0x14, cpu.eax);
        cpu.eax = cpu.ebx;
        if (!direct(app, cpu, quadVertices))
            return;
        if (s(at(app, cpu.ebx + 0x70)) > 0)
        {
            cpu.edx = at(app, cpu.ebx + 0x14);
            cpu.ecx = at(app, cpu.ebx + 0x70);
            cpu.edx -= 0xe;
            --cpu.ecx;
            put(app, cpu.ebx + 0x14, cpu.edx);
            cpu.eax = cpu.ebx;
            put(app, cpu.ebx + 0x70, cpu.ecx);
            if (!direct(app, cpu, quadStepForward))
                return;
            if (onPolygon(app, cpu, cpu.ebx, cpu.edi))
            {
                put(app, cpu.ebx + 0x64, 2);
                cpu.eax = at(app, cpu.ebx);
                put(app, cpu.ebx + 0x6c, 0);
                cpu.eax = at(app, cpu.eax + 0x59c);
                put(app, cpu.ebx + 0x1c, cpu.eax);
                cpu.eax = at(app, cpu.ebx + 0x18);
                cpu.edx = 0;
                cpu.dl = byte(app, cpu.eax);
                cpu.eax = cpu.edx * 4;
                cpu.eax -= cpu.edx;
                cpu.eax <<= 2;
                cpu.edx = at(app, cpu.ebx + 0x1c);
                cpu.edx += cpu.eax;
                cpu.eax = cpu.ebx;
                put(app, cpu.ebx + 0x1c, cpu.edx);
                if (!direct(app, cpu, quadToFixed))
                    return;
                if (at(app, cpu.ebx + 0x14) == 0)
                {
                    cpu.edx = at(app, cpu.ebx + 0x10);
                    cpu.eax = cpu.edx * 8;
                    cpu.edx += cpu.eax;
                    cpu.eax = at(app, kNodes);
                    cpu.edx <<= 2;
                    cpu.eax = at(app, cpu.edx + cpu.eax + 0xe);
                    cpu.eax = reg32(s(cpu.eax) >> 0x18);
                    cpu.eax <<= 9;
                }
                else
                {
                    cpu.eax = at(app, cpu.ebx + 0x1c);
                    cpu.eax = at(app, cpu.eax);
                    cpu.eax = reg32(s(cpu.eax) >> 0x10);
                    cpu.eax += cpu.eax;
                }
                if (cpu.eax != 0)
                    cpu.eax = 1;
                return leave();
            }
        }
    }
    cpu.eax = 0;
    leave();
}

/* 0x41fd60: which of its quad's edges the place eax is past, for the place
 * on the track ebx against edx's quad (+0x24 on, 16.16): the polygons the
 * same, only their ends' (+0x6c of each, or'd); else bit 1 and 2 the two
 * sides (within -1.5), 4 and 8 the ends, and [ecx] the height above the
 * quad's middle (ebx's four corners' y's quarter less the place's). */
void quadEdgeFlags(App* app, CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    const reg32 b = cpu.ebp;
    cpu.esi = cpu.eax;
    cpu.edi = cpu.ebx;
    put(app, b - 4, cpu.ecx);
    cpu.ecx = at(app, cpu.edi + 0x6c);
    cpu.ebx = 0;
    if (cpu.ecx != 0)
        cpu.ebx = cpu.ecx;
    cpu.eax = at(app, cpu.edx + 0x6c);
    if (cpu.eax != 0)
        cpu.ebx |= cpu.eax;
    cpu.ecx = at(app, cpu.edi + 0x18);
    if (cpu.ecx != at(app, cpu.edx + 0x18))
    {
        cpu.ecx = cpu.edx + 0x24;
        // The first side.
        cpu.edx = at(app, cpu.esi + 8);
        cpu.eax = at(app, cpu.ecx + 0x20);
        cpu.edx -= cpu.eax;
        cpu.eax = cpu.edx;
        cpu.edx = at(app, cpu.ecx + 0xc);
        cpu.edx -= at(app, cpu.ecx + 0x18);
        put(app, b - 0xc, cpu.edx);
        cpu.edx = cpu.eax;
        cpu.eax = at(app, b - 0xc);
        fixedMul(cpu);
        put(app, b - 8, cpu.eax);
        cpu.edx = at(app, cpu.ecx + 0x14);
        cpu.eax = at(app, cpu.ecx + 0x20);
        cpu.edx -= cpu.eax;
        put(app, b - 0xc, cpu.edx);
        cpu.eax = at(app, cpu.ecx + 0x18);
        cpu.edx = at(app, cpu.esi);
        cpu.edx -= cpu.eax;
        cpu.eax = cpu.edx;
        cpu.edx = at(app, b - 0xc);
        fixedMul(cpu);
        cpu.edx = at(app, b - 8);
        cpu.edx -= cpu.eax;
        if (s(cpu.edx) >= s(0xfffe8000))
            cpu.bl |= 1;
        // The second side.
        cpu.edx = at(app, cpu.esi + 8);
        cpu.eax = at(app, cpu.ecx + 8);
        cpu.edx -= cpu.eax;
        cpu.eax = cpu.edx;
        cpu.edx = at(app, cpu.ecx + 0x24);
        cpu.edx -= at(app, cpu.ecx);
        put(app, b - 0xc, cpu.edx);
        cpu.edx = cpu.eax;
        cpu.eax = at(app, b - 0xc);
        fixedMul(cpu);
        put(app, b - 8, cpu.eax);
        cpu.edx = at(app, cpu.ecx + 0x2c);
        cpu.eax = at(app, cpu.ecx + 8);
        cpu.edx -= cpu.eax;
        cpu.eax = cpu.edx;
        cpu.edx = at(app, cpu.esi);
        cpu.edx -= at(app, cpu.ecx);
        put(app, b - 0xc, cpu.edx);
        cpu.edx = cpu.eax;
        cpu.eax = at(app, b - 0xc);
        fixedMul(cpu);
        cpu.edx = at(app, b - 8);
        cpu.edx -= cpu.eax;
        if (s(cpu.edx) >= s(0xfffe8000))
            cpu.bl |= 2;
        // The first end.
        cpu.edx = at(app, cpu.esi + 8);
        cpu.eax = at(app, cpu.ecx + 0x14);
        cpu.edx -= cpu.eax;
        cpu.eax = cpu.edx;
        cpu.edx = at(app, cpu.ecx);
        cpu.edx -= at(app, cpu.ecx + 0xc);
        put(app, b - 8, cpu.edx);
        cpu.edx = cpu.eax;
        cpu.eax = at(app, b - 8);
        fixedMul(cpu);
        put(app, b - 0xc, cpu.eax);
        cpu.edx = at(app, cpu.ecx + 8);
        cpu.eax = at(app, cpu.ecx + 0x14);
        cpu.edx -= cpu.eax;
        cpu.eax = cpu.edx;
        cpu.edx = at(app, cpu.esi);
        cpu.edx -= at(app, cpu.ecx + 0xc);
        put(app, b - 8, cpu.edx);
        cpu.edx = cpu.eax;
        cpu.eax = at(app, b - 8);
        fixedMul(cpu);
        cpu.edx = at(app, b - 0xc);
        cpu.edx -= cpu.eax;
        if (s(cpu.edx) >= 0)
            cpu.bl |= 4;
        // The second end.
        cpu.edx = at(app, cpu.esi + 8);
        cpu.eax = at(app, cpu.ecx + 0x2c);
        cpu.edx -= cpu.eax;
        cpu.eax = cpu.edx;
        cpu.edx = at(app, cpu.ecx + 0x18);
        cpu.edx -= at(app, cpu.ecx + 0x24);
        put(app, b - 8, cpu.edx);
        cpu.edx = cpu.eax;
        cpu.eax = at(app, b - 8);
        fixedMul(cpu);
        put(app, b - 8, cpu.eax);
        cpu.edx = at(app, cpu.ecx + 0x20);
        cpu.eax = at(app, cpu.ecx + 0x2c);
        cpu.edx -= cpu.eax;
        cpu.eax = cpu.edx;
        cpu.edx = at(app, cpu.esi);
        cpu.edx -= at(app, cpu.ecx + 0x24);
        cpu.ecx = cpu.edx;
        cpu.edx = cpu.eax;
        cpu.eax = cpu.ecx;
        fixedMul(cpu);
        cpu.edx = at(app, b - 8);
        cpu.edx -= cpu.eax;
        if (s(cpu.edx) >= 0)
            cpu.bl |= 8;
        // The height above the quad's middle.
        cpu.edx = cpu.edi + 0x24;
        cpu.ecx = at(app, cpu.edx + 4);
        cpu.edi = at(app, cpu.edx + 0x10);
        cpu.eax = at(app, cpu.edx + 0x1c);
        cpu.ecx += cpu.edi;
        cpu.edx = at(app, cpu.edx + 0x28);
        cpu.eax += cpu.ecx;
        cpu.edx += cpu.eax;
        cpu.edx = reg32(s(cpu.edx) >> 2);
        cpu.eax = at(app, cpu.esi + 4);
        cpu.edx -= cpu.eax;
        cpu.eax = at(app, b - 4);
        put(app, cpu.eax, cpu.edx);
    }
    cpu.edx = cpu.ebx;
    cpu.eax = cpu.edx;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41ff10: the items of the place on the track eax's block, [+0x5a0]. */
void sectionItemCount(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, cpu.eax);
    cpu.eax = at(app, cpu.eax + 0x5a0);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41ff20: the item edx of the place on the track eax's block (20 bytes
 * each, [+0x5a4]): its first 12 bytes to ebx, its word at 0xc to [ecx] --
 * halved where its kind (0x13's low nibble) is 1, doubled where it is 2.
 * 0, or -1 past the items. */
void sectionItem(App* app, CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 b = cpu.ebp;
    cpu.eax = at(app, cpu.eax);
    if (s(cpu.edx) >= s(at(app, cpu.eax + 0x5a0)))
        cpu.eax = 0xffffffff;
    else
    {
        cpu.eax = at(app, cpu.eax + 0x5a4);
        put(app, b - 4, cpu.eax);
        cpu.eax = cpu.edx * 4;
        cpu.eax += cpu.edx;
        cpu.edi = at(app, b - 4);
        cpu.eax <<= 2;
        cpu.eax += cpu.edi;
        cpu.edi = cpu.ebx;
        cpu.esi = cpu.eax;
        moveWords(app, cpu, 3);
        cpu.bx = half(app, cpu.eax + 0xc);
        putHalf(app, cpu.ecx, cpu.bx);
        cpu.al = byte(app, cpu.eax + 0x13);
        cpu.al &= 0xf;
        if (cpu.al == 1)
        {
            cpu.bx = reg16(x86::sreg16(cpu.bx) >> 1);
            putHalf(app, cpu.ecx, cpu.bx);
        }
        cpu.eax = cpu.edx * 4;
        cpu.eax += cpu.edx;
        cpu.edx = at(app, b - 4);
        cpu.eax <<= 2;
        cpu.eax += cpu.edx;
        cpu.al = byte(app, cpu.eax + 0x13);
        cpu.al &= 0xf;
        if (cpu.al == 2)
        {
            cpu.si = half(app, cpu.ecx);
            cpu.esi += cpu.esi;
            putHalf(app, cpu.ecx, cpu.si);
        }
        cpu.eax = 0;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41ffa0: the lanes' span edx and shift eax, for lanePosition. */
void setLaneOffsets(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    put(app, kLaneSpan, cpu.edx);
    put(app, kLaneShift, cpu.eax);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x41ffd0: a place in a lane, to [edx], from the place on the track eax:
 * the node half the lanes' span (0x5dd930) on from its node, then back to a
 * multiple of the span, round the track's ends; that node's place moved out
 * to its side by its width -- right (ebx set: the nibble at 0x13 by the byte
 * at 0x1b, along the vector at 0x18) or left (the nibble at 0x13's top by
 * the byte at 0x17) -- and along its vector at 0x10 by the shift (0x5dd934).
 * The node in eax. */
void lanePosition(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x14;
    const reg32 b = cpu.ebp;
    cpu.ecx = cpu.eax;
    put(app, b - 4, cpu.edx);
    put(app, b - 8, cpu.ebx);
    cpu.eax = at(app, kLaneSpan);
    cpu.edx = cpu.eax;
    cpu.edx = reg32(s(cpu.edx) >> 0x1f);
    cpu.eax -= cpu.edx;
    cpu.eax = reg32(s(cpu.eax) >> 1);
    if (s(cpu.eax) >= 0)
    {
        cpu.edx = at(app, cpu.ecx + 0x10);
        cpu.ecx = at(app, kNodeCount);
        cpu.eax += cpu.edx;
        if (s(cpu.eax) >= s(cpu.ecx))
            cpu.eax -= cpu.ecx;
    }
    else
    {
        cpu.eax += at(app, cpu.ecx + 0x10);
        if (s(cpu.eax) < 0)
            cpu.eax += at(app, kNodeCount);
    }
    cpu.ebx = cpu.eax;
    cpu.edx = cpu.ebx;
    cpu.edx = 0u - cpu.edx;
    cpu.ecx = at(app, kLaneSpan);
    cpu.eax = cpu.edx;
    cpu.edx = reg32(s(cpu.edx) >> 0x1f);
    idiv(cpu, cpu.ecx);
    cpu.eax = cpu.ebx + cpu.edx;
    if (s(cpu.edx) >= 0)
    {
        cpu.ebx = at(app, kNodeCount);
        if (s(cpu.eax) >= s(cpu.ebx))
            cpu.eax -= cpu.ebx;
    }
    else if (s(cpu.eax) < 0)
        cpu.eax += at(app, kNodeCount);
    cpu.ecx = cpu.eax;
    cpu.ebx = cpu.eax * 8;
    cpu.ebx += cpu.eax;
    cpu.eax = at(app, kNodes);
    cpu.ebx <<= 2;
    cpu.eax += cpu.ebx;
    cpu.edi = b - 0x14;
    cpu.esi = cpu.eax;
    cpu.edx = at(app, b - 8);
    moveWords(app, cpu, 3);
    if (cpu.edx != 0)
    {
        cpu.esi = at(app, cpu.eax + 0x15);
        cpu.edx = 0;
        cpu.esi = reg32(s(cpu.esi) >> 0x18);
        cpu.dl = byte(app, cpu.eax + 0x1b);
        cpu.esi <<= 9;
        cpu.edi = cpu.edx;
        cpu.al = byte(app, cpu.eax + 0x13);
        cpu.edi <<= 0xf;
        cpu.al &= 0xf;
        cpu.eax &= 0xff;
        cpu.eax *= cpu.edi;
        cpu.edx = cpu.esi;
        cpu.esi = at(app, b - 0x14);
        fixedMul(cpu);
        cpu.esi += cpu.eax;
        cpu.eax = at(app, kNodes);
        cpu.edx = 0;
        put(app, b - 0x14, cpu.esi);
        cpu.esi = at(app, cpu.ebx + cpu.eax + 0x16);
        cpu.dl = byte(app, cpu.ebx + cpu.eax + 0x1b);
        cpu.esi = reg32(s(cpu.esi) >> 0x18);
        cpu.edi = cpu.edx;
        cpu.al = byte(app, cpu.ebx + cpu.eax + 0x13);
        cpu.esi <<= 9;
        cpu.edi <<= 0xf;
        cpu.al &= 0xf;
        cpu.eax &= 0xff;
        cpu.eax *= cpu.edi;
        cpu.edx = cpu.esi;
        cpu.edi = at(app, b - 0x10);
        fixedMul(cpu);
        cpu.edi += cpu.eax;
        cpu.eax = at(app, kNodes);
        cpu.ebx += cpu.eax;
        put(app, b - 0x10, cpu.edi);
        cpu.edx = at(app, cpu.ebx + 0x17);
        cpu.eax = 0;
        cpu.edx = reg32(s(cpu.edx) >> 0x18);
        cpu.al = byte(app, cpu.ebx + 0x1b);
        cpu.edx <<= 9;
        cpu.esi = cpu.eax;
        cpu.al = byte(app, cpu.ebx + 0x13);
        cpu.esi <<= 0xf;
        cpu.al &= 0xf;
        cpu.eax &= 0xff;
        cpu.eax *= cpu.esi;
        fixedMul(cpu);
        put(app, b - 0xc, at(app, b - 0xc) + cpu.eax);
    }
    else
    {
        cpu.edx = at(app, cpu.eax + 0x15);
        cpu.esi = byte(app, cpu.eax + 0x13);
        cpu.edx = reg32(s(cpu.edx) >> 0x18);
        cpu.esi = reg32(s(cpu.esi) >> 4);
        cpu.al = byte(app, cpu.eax + 0x17);
        cpu.edx <<= 9;
        cpu.esi &= 0xf;
        cpu.eax &= 0xff;
        cpu.eax <<= 0xf;
        cpu.eax *= cpu.esi;
        cpu.esi = at(app, b - 0x14);
        fixedMul(cpu);
        cpu.esi -= cpu.eax;
        cpu.eax = at(app, kNodes);
        put(app, b - 0x14, cpu.esi);
        cpu.edx = at(app, cpu.ebx + cpu.eax + 0x16);
        cpu.esi = byte(app, cpu.ebx + cpu.eax + 0x13);
        cpu.edx = reg32(s(cpu.edx) >> 0x18);
        cpu.esi = reg32(s(cpu.esi) >> 4);
        cpu.al = byte(app, cpu.ebx + cpu.eax + 0x17);
        cpu.edx <<= 9;
        cpu.esi &= 0xf;
        cpu.eax &= 0xff;
        cpu.eax <<= 0xf;
        cpu.eax *= cpu.esi;
        cpu.edi = at(app, b - 0x10);
        fixedMul(cpu);
        cpu.edi -= cpu.eax;
        cpu.eax = at(app, kNodes);
        cpu.ebx += cpu.eax;
        put(app, b - 0x10, cpu.edi);
        cpu.edx = at(app, cpu.ebx + 0x17);
        cpu.eax = 0;
        cpu.edx = reg32(s(cpu.edx) >> 0x18);
        cpu.al = byte(app, cpu.ebx + 0x13);
        cpu.edx <<= 9;
        cpu.eax = reg32(s(cpu.eax) >> 4);
        cpu.bl = byte(app, cpu.ebx + 0x17);
        cpu.eax &= 0xf;
        cpu.ebx &= 0xff;
        cpu.ebx <<= 0xf;
        cpu.eax *= cpu.ebx;
        fixedMul(cpu);
        put(app, b - 0xc, at(app, b - 0xc) - cpu.eax);
    }
    // Along the node by the shift.
    cpu.eax = cpu.ecx * 8;
    cpu.eax += cpu.ecx;
    cpu.edx = at(app, kNodes);
    cpu.eax <<= 2;
    cpu.eax += cpu.edx;
    cpu.edx = at(app, cpu.eax + 0xd);
    cpu.edx = reg32(s(cpu.edx) >> 0x18);
    cpu.ebx = at(app, kLaneShift);
    cpu.edx <<= 9;
    cpu.edx *= cpu.ebx;
    cpu.esi = at(app, b - 0x14);
    cpu.esi += cpu.edx;
    cpu.edx = at(app, cpu.eax + 0xe);
    cpu.edx = reg32(s(cpu.edx) >> 0x18);
    cpu.edx <<= 9;
    cpu.edx *= cpu.ebx;
    cpu.eax = at(app, cpu.eax + 0xf);
    put(app, b - 0x14, cpu.esi);
    cpu.eax = reg32(s(cpu.eax) >> 0x18);
    cpu.esi = cpu.ebx;
    cpu.eax <<= 9;
    cpu.eax *= cpu.esi;
    cpu.edi = at(app, b - 0xc);
    cpu.ebx = at(app, b - 0x10);
    cpu.esi = b - 0x14;
    cpu.edi += cpu.eax;
    cpu.ebx += cpu.edx;
    put(app, b - 0xc, cpu.edi);
    cpu.edi = at(app, b - 4);
    put(app, b - 0x10, cpu.ebx);
    cpu.eax = cpu.ecx;
    moveWords(app, cpu, 3);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x420230: the frame of the place on the track eax, to edx (nine words:
 * the side, the up and the ahead vectors, the side the up's cross the
 * ahead's) -- found on the road (+0x64 2, a strip) its normal's words
 * doubled, else the node's bytes << 9 -- and its quad's first corner (or the
 * node's place, without a strip or off an end) copied to esi.  esi in eax. */
void positionFrame(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x24;
    const reg32 b = cpu.ebp;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.esi;
    cpu.edi = cpu.edx;
    cpu.esi = cpu.eax;
    cpu.eax = at(app, cpu.eax + 0x64) == 2 ? 1 : 0;
    cpu.ebx = at(app, cpu.esi + 0x14);
    cpu.edx = cpu.eax;
    if (cpu.ebx != 0 && at(app, cpu.esi + 0x6c) == 0)
    {
        const auto fast = [&] { return cpu.edx != 0 && at(app, cpu.esi + 0x14) != 0; };
        // The normal's word at `word`, doubled.
        const auto normal = [&](reg32 word) {
            cpu.eax = at(app, cpu.esi + 0x1c);
            cpu.eax = at(app, cpu.eax + word);
            cpu.eax = reg32(s(cpu.eax) >> 0x10);
            cpu.eax += cpu.eax;
        };
        // The node's byte at `offset` + 3, << 9, the nodes' table in ebx.
        const auto nodeByte = [&](reg32 offset) {
            cpu.eax = at(app, cpu.esi + 0x10);
            put(app, b - 0x18, cpu.eax);
            cpu.ebx = at(app, b - 0x18);
            cpu.eax <<= 3;
            cpu.eax += cpu.ebx;
            cpu.ebx = at(app, kNodes);
            cpu.eax <<= 2;
            cpu.eax += cpu.ebx;
            cpu.eax = at(app, cpu.eax + offset);
            cpu.eax = reg32(s(cpu.eax) >> 0x18);
            cpu.eax <<= 9;
        };
        // The same, the node's offset in ebx.
        const auto nodeByteToo = [&](reg32 offset) {
            cpu.eax = at(app, cpu.esi + 0x10);
            put(app, b - 0x18, cpu.eax);
            cpu.ebx = at(app, b - 0x18);
            cpu.eax <<= 3;
            cpu.eax += cpu.ebx;
            cpu.eax <<= 2;
            put(app, b - 0x18, cpu.eax);
            cpu.ebx = at(app, b - 0x18);
            cpu.eax = at(app, kNodes);
            cpu.eax += cpu.ebx;
            cpu.eax = at(app, cpu.eax + offset);
            cpu.eax = reg32(s(cpu.eax) >> 0x18);
            cpu.eax <<= 9;
        };
        if (fast())
        {
            cpu.eax = at(app, cpu.esi + 0x1c);
            cpu.eax = reg32(sreg32(x86::sreg16(half(app, cpu.eax))));
            cpu.eax += cpu.eax;
        }
        else
            nodeByte(0xd);
        put(app, b - 8, cpu.eax);
        cpu.eax = at(app, b - 8);
        put(app, cpu.edi + 0xc, cpu.eax);
        if (fast())
            normal(0);
        else
            nodeByte(0xe);
        put(app, b - 4, cpu.eax);
        cpu.eax = at(app, b - 4);
        put(app, cpu.edi + 0x10, cpu.eax);
        if (fast())
            normal(2);
        else
            nodeByteToo(0xf);
        put(app, b - 0xc, cpu.eax);
        cpu.eax = at(app, b - 0xc);
        put(app, cpu.edi + 0x14, cpu.eax);
        if (fast())
            normal(4);
        else
            nodeByte(0x11);
        put(app, b - 0x14, cpu.eax);
        cpu.eax = at(app, b - 0x14);
        put(app, cpu.edi + 0x18, cpu.eax);
        if (fast())
            normal(6);
        else
            nodeByteToo(0x12);
        put(app, b - 0x10, cpu.eax);
        cpu.eax = at(app, b - 0x10);
        put(app, cpu.edi + 0x1c, cpu.eax);
        if (fast())
            normal(8);
        else
        {
            cpu.edx = at(app, cpu.esi + 0x10);
            cpu.eax = cpu.edx * 8;
            cpu.eax += cpu.edx;
            cpu.edx = at(app, kNodes);
            cpu.eax = at(app, cpu.edx + cpu.eax * 4 + 0x13);
            cpu.eax = reg32(s(cpu.eax) >> 0x18);
            cpu.eax <<= 9;
        }
        put(app, cpu.edi + 0x20, cpu.eax);
        // The side: the up's cross the ahead's.
        cpu.edx = cpu.eax;
        cpu.eax = at(app, cpu.edi + 0x10);
        fixedMul(cpu);
        cpu.ebx = cpu.eax;
        cpu.edx = at(app, cpu.edi + 0x1c);
        cpu.eax = at(app, cpu.edi + 0x14);
        fixedMul(cpu);
        cpu.edx = at(app, cpu.edi + 0x18);
        cpu.ebx -= cpu.eax;
        cpu.eax = at(app, cpu.edi + 0x14);
        put(app, cpu.edi, cpu.ebx);
        fixedMul(cpu);
        cpu.ebx = cpu.eax;
        cpu.edx = at(app, cpu.edi + 0x20);
        cpu.eax = at(app, cpu.edi + 0xc);
        fixedMul(cpu);
        cpu.edx = at(app, cpu.edi + 0x1c);
        cpu.ebx -= cpu.eax;
        cpu.eax = at(app, cpu.edi + 0xc);
        put(app, cpu.edi + 4, cpu.ebx);
        fixedMul(cpu);
        cpu.ebx = cpu.eax;
        cpu.edx = at(app, cpu.edi + 0x18);
        cpu.eax = at(app, cpu.edi + 0x10);
        fixedMul(cpu);
        cpu.ebx -= cpu.eax;
        put(app, cpu.edi + 8, cpu.ebx);
        cpu.edi = b - 0x24;
        cpu.esi += 0x24;
        moveWords(app, cpu, 3);
        cpu.esi = b - 0x24;
        cpu.edi = cpu.ecx;
        moveWords(app, cpu, 3);
        cpu.eax = cpu.ecx;
        return leave();
    }
    // No strip: the node's bytes, and its place.
    cpu.edx = at(app, cpu.esi + 0x10);
    cpu.eax = cpu.edx * 8;
    cpu.edx += cpu.eax;
    cpu.eax = at(app, kNodes);
    cpu.edx <<= 2;
    cpu.edx = at(app, cpu.edx + cpu.eax + 0x15);
    cpu.edx = reg32(s(cpu.edx) >> 0x18);
    cpu.edx <<= 9;
    put(app, cpu.edi, cpu.edx);
    static const reg32 offsets[][2] = {{0x16, 4}, {0x17, 8}, {0xd, 0xc}, {0xe, 0x10}, {0xf, 0x14},
                                       {0x11, 0x18}, {0x12, 0x1c}, {0x13, 0x20}};
    for (const auto& o : offsets)
    {
        cpu.edx = at(app, cpu.esi + 0x10);
        put(app, b - 0x18, cpu.edx);
        cpu.ebx = at(app, b - 0x18);
        cpu.edx <<= 3;
        cpu.edx += cpu.ebx;
        cpu.edx = at(app, cpu.eax + cpu.edx * 4 + o[0]);
        cpu.edx = reg32(s(cpu.edx) >> 0x18);
        cpu.edx <<= 9;
        put(app, cpu.edi + o[1], cpu.edx);
    }
    cpu.esi = at(app, cpu.esi + 0x10);
    cpu.edx = cpu.esi * 8;
    cpu.edx += cpu.esi;
    cpu.edx <<= 2;
    cpu.edi = b - 0x24;
    cpu.esi = cpu.eax + cpu.edx;
    moveWords(app, cpu, 3);
    cpu.esi = b - 0x24;
    cpu.edi = cpu.ecx;
    moveWords(app, cpu, 3);
    cpu.eax = cpu.ecx;
    leave();
}

/* 0x4205a0: the place eax (three floats) snapped to the road: made 16.16,
 * placed on the track (placeOnTrack, into the frame), and its place there
 * written back as floats; the place on the track (0x78 bytes) copied to
 * esi, which is eax. */
void snapToTrack(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x8c;
    cpu.ebp -= 0x82;
    const reg32 b = cpu.ebp;
    x86::FPU& fpu = cpu.fpu;
    put(app, b + 0x7a, cpu.esi);
    cpu.ecx = cpu.eax;
    cpu.eax = fistpPop(app, cpu, fp::mul(f32(app, cpu.eax), f64(app, kFixedToo)));
    put(app, b + 0x6e, cpu.eax);
    cpu.eax = fistpPop(app, cpu, fp::mul(f32(app, cpu.ecx + 4), f64(app, kFixedToo)));
    put(app, b + 0x72, cpu.eax);
    {
        const Float z = fp::mul(f32(app, cpu.ecx + 8), f64(app, kFixedToo));
        cpu.esi = b - 0xa;
        cpu.eax = fistpPop(app, cpu, z);
    }
    put(app, b + 0x76, cpu.eax);
    cpu.eax = b + 0x6e;
    if (!direct(app, cpu, placeOnTrack))
        return;
    {
        const Float k = f64(app, kUnfixedThree);
        cpu.eax = at(app, b + 0x6e);
        put(app, b + 0x7e, cpu.eax);
        putF32(app, cpu.ecx, fp::mul(i32(app, b + 0x7e), k));
        cpu.eax = at(app, b + 0x72);
        put(app, b + 0x7e, cpu.eax);
        putF32(app, cpu.ecx + 4, fp::mul(i32(app, b + 0x7e), k));
        cpu.eax = at(app, b + 0x76);
        put(app, b + 0x7e, cpu.eax);
        const Float z = fp::mul(k, i32(app, b + 0x7e));
        cpu.edi = at(app, b + 0x7a);
        cpu.esi = b - 0xa;
        putF32(app, cpu.ecx + 8, z);
    }
    cpu.ecx = 0x1e;
    moveWords(app, cpu);
    cpu.eax = at(app, b + 0x7a);
    cpu.esp = cpu.ebp + 0x82;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x420650: the place eax (16.16) put on the track near node ebx: a place
 * on the track made afresh in the frame (resetPosition), brought there
 * (updatePosition, edx followed or not), its frame taken (positionFrame), and
 * the height of the surface there (0x4969c0, of the frame, the corner and the
 * place) to the place's y.  The place on the track copied to esi, which is
 * eax. */
void placeOnTrack(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xb0;
    cpu.ebp -= 0x82;
    const reg32 b = cpu.ebp;
    put(app, b + 0x7e, cpu.esi);
    cpu.ecx = cpu.eax;
    put(app, b + 0x7a, cpu.edx);
    cpu.edx = 0x78;
    cpu.eax = b - 0x2e;
    cpu.edi = b - 0x2a;
    if (!byAddress(app, cpu, kFillZero))
        return;
    cpu.edx = b - 0x2e;
    cpu.eax = cpu.ebx;
    cpu.esi = cpu.ecx;
    if (!direct(app, cpu, resetPosition))
        return;
    cpu.ebx = at(app, b + 0x7a);
    cpu.edx = b - 0x2e;
    cpu.eax = cpu.ecx;
    moveWords(app, cpu, 3);
    if (!direct(app, cpu, updatePosition))
        return;
    cpu.edx = b + 0x4a;
    cpu.eax = b - 0x2e;
    cpu.esi = b + 0x6e;
    if (!direct(app, cpu, positionFrame))
        return;
    cpu.edx = at(app, cpu.ecx + 8);
    push(app, cpu, cpu.edx);
    cpu.ebx = at(app, cpu.ecx + 4);
    push(app, cpu, cpu.ebx);
    cpu.esi = at(app, cpu.ecx);
    push(app, cpu, cpu.esi);
    cpu.edi = at(app, b + 0x76);
    push(app, cpu, cpu.edi);
    cpu.eax = at(app, b + 0x72);
    push(app, cpu, cpu.eax);
    cpu.edx = at(app, b + 0x6e);
    cpu.esi = cpu.ecx;
    push(app, cpu, cpu.edx);
    cpu.eax = b + 0x4a;
    cpu.edi = b - 0x2a;
    if (!byAddress(app, cpu, kOnSurface))
        return;
    put(app, cpu.ecx + 4, cpu.eax);
    moveWords(app, cpu, 3);
    cpu.ecx = 0x1e;
    cpu.edi = at(app, b + 0x7e);
    cpu.esi = b - 0x2e;
    moveWords(app, cpu);
    cpu.eax = at(app, b + 0x7e);
    cpu.esp = cpu.ebp + 0x82;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4206f0: the light where the place on the track eax is (edx set and on
 * a strip): its quad's four vertices' colours (its block's +0x74) averaged,
 * byte by byte, opaque; else 0xff7f7f7f. */
void positionColour(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 b = cpu.ebp;
    if (cpu.edx == 0 || at(app, cpu.eax + 0x14) == 0)
        cpu.eax = 0xff7f7f7f;
    else
    {
        cpu.edx = at(app, cpu.eax);
        cpu.ecx = at(app, cpu.eax + 0x14);
        cpu.esi = at(app, cpu.edx + 0x74);
        cpu.ebx = at(app, cpu.ecx + 2);
        cpu.eax = reg32(sreg32(x86::sreg16(half(app, cpu.ecx))));
        cpu.ebx = reg32(s(cpu.ebx) >> 0x10);
        cpu.edx = at(app, cpu.esi + cpu.eax * 4);
        cpu.eax = at(app, cpu.ecx);
        cpu.ebx = at(app, cpu.esi + cpu.ebx * 4);
        cpu.eax = reg32(s(cpu.eax) >> 0x10);
        cpu.ecx = at(app, cpu.ecx + 4);
        cpu.eax = at(app, cpu.esi + cpu.eax * 4);
        cpu.ecx = reg32(s(cpu.ecx) >> 0x10);
        cpu.edi = cpu.eax;
        cpu.ecx = at(app, cpu.esi + cpu.ecx * 4);
        cpu.esi = cpu.edx;
        cpu.edi &= 0xff0000;
        cpu.esi &= 0xff0000;
        cpu.esi += cpu.edi;
        cpu.edi = cpu.ebx;
        cpu.edi &= 0xff0000;
        cpu.edi += cpu.esi;
        cpu.esi = cpu.ecx;
        cpu.esi &= 0xff0000;
        cpu.esi += cpu.edi;
        cpu.esi = reg32(s(cpu.esi) >> 2);
        cpu.edi = cpu.eax;
        cpu.esi &= 0xff0000;
        cpu.edi &= 0xff00;
        cpu.eax &= 0xff;
        put(app, b - 4, cpu.esi);
        cpu.esi = cpu.edx;
        cpu.esi &= 0xff00;
        cpu.edx &= 0xff;
        cpu.esi += cpu.edi;
        cpu.eax += cpu.edx;
        cpu.edi = cpu.ebx;
        cpu.edx = cpu.ebx;
        cpu.edi &= 0xff00;
        cpu.edx &= 0xff;
        cpu.edx += cpu.eax;
        cpu.eax = cpu.ecx;
        cpu.edi += cpu.esi;
        cpu.esi = cpu.ecx;
        cpu.eax &= 0xff;
        cpu.esi &= 0xff00;
        cpu.edx += cpu.eax;
        cpu.eax = at(app, b - 4);
        cpu.esi += cpu.edi;
        cpu.edx = reg32(s(cpu.edx) >> 2);
        cpu.esi = reg32(s(cpu.esi) >> 2);
        cpu.eax |= 0xff000000;
        cpu.esi &= 0xff00;
        cpu.edx &= 0xff;
        cpu.eax |= cpu.esi;
        cpu.eax |= cpu.edx;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4207e0: the kind of the polygon the place on the track eax is on (the
 * byte at 1 of its +0x18), or 1 without one; edx set and no strip, +0x74 -1
 * and 1. */
void positionSurface(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (cpu.edx != 0 && at(app, cpu.eax + 0x14) == 0)
    {
        put(app, cpu.eax + 0x74, 0xffffffff);
        cpu.eax = 1;
    }
    else
    {
        cpu.ecx = at(app, cpu.eax + 0x18);
        if (cpu.ecx != 0)
        {
            cpu.al = byte(app, cpu.ecx + 1);
            cpu.eax &= 0xff;
        }
        else
            cpu.eax = 1;
    }
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x420840: what is under a car at node eax across lane edx, by the track
 * ([0x6fd4ac], the targets of nfs3.exe's table at 0x420814): the nodes of
 * each track's special ground -- 0x10, 0x11 or 0x12 -- or 0. */
void surfaceUnder(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = at(app, kTrack);
    cpu.ebx = 0;
    const sreg32 node = s(cpu.eax), lane = s(cpu.edx);
    // node in (from, to), as the code's cmp eax, from; jle; cmp eax, to; jl.
    const auto in = [&](sreg32 from, sreg32 to) { return node > from && node < to; };
    reg32 result = cpu.ebx;
    switch (cpu.ecx <= 8 ? cpu.ecx : 1)
    {
    case 0:  // 0x42085d
    {
        if (in(0x25a, 0x260))
        {
            result = 0x10;
            break;
        }
        bool found = false;
        if (lane == 2)
            found = in(0x21d, 0x227);
        else if (lane == 3)
            found = in(0x215, 0x21e) || in(0x234, 0x238);
        else if (lane == 5)
        {
            if (in(0x22d, 0x235))
            {
                result = 0x11;
                break;
            }
        }
        else if (lane == 6)
            found = in(0x22d, 0x235);
        else if (lane == 7)
            found = in(0x1f1, 0x1f8) || in(0x212, 0x216);
        else if (lane == 8)
            found = in(0x1f7, 0x209) || in(0x20f, 0x213);
        if (!found && lane <= 4)
        {
            found = in(0x1f2, 0x209) || in(0x20f, 0x216) || in(0x237, 0x240);
            if (!found && lane <= 3)
            {
                found = in(0x212, 0x22c);
                if (!found && lane <= 2)
                    found = in(0x208, 0x210);
            }
        }
        if (!found && lane >= 5)
        {
            found = in(0x21d, 0x227);
            if (!found && lane >= 6)
            {
                found = in(0x208, 0x210) || in(0x215, 0x21e) || in(0x226, 0x22c) || in(0x234, 0x238);
                if (!found && lane >= 7)
                {
                    found = in(0x22b, 0x22e) || in(0x237, 0x240);
                    if (!found && lane >= 9)
                        found = in(0x22d, 0x235);
                }
            }
        }
        if (found)
            result = 0x12;
        break;
    }
    case 2:  // 0x420a83
        if (lane == 4 && (in(0x3cd, 0x3d7) || in(0x3b7, 0x3c1) || in(0x40b, 0x417)))
            result = 0x11;
        break;
    case 3:  // 0x420acc
        if (lane == 3)
        {
            if (node == 0x557 || node == 0x556 || node == 0x553 || node == 0x554)
                result = 0x11;
        }
        else if (in(0x41a, 0x42c))
            result = 0x12;
        break;
    case 6:  // 0x420b16
        if (lane == 4 && (in(0x424, 0x42e) || in(0x43a, 0x444) || in(0x478, 0x484)))
            result = 0x11;
        break;
    case 7:  // 0x420b5c
        if (lane == 3 && (node == 0x643 || node == 0x644))
            result = 0x11;
        break;
    case 8:  // 0x420b80
        if (node > 0x3c3 || node < 0xd6)
            result = 0x10;
        else if (lane > 7 && lane < 0xb && in(0x305, 0x35b))
            result = 0x12;
        else if (lane == 2 && in(0x2a4, 0x2e1))
            result = 0x11;
        else if ((lane == 9 || lane == 0xb) && in(0x364, 0x37d))
            result = 0x11;
        break;
    default:  // 1, 4, 5 (0x420bde) and past the table
        break;
    }
    cpu.eax = result;
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

namespace
{

/* The node's tunnel kind, its byte at 0xe's low nibble asked first (else
 * the high one), 2 meaning a tunnel: eax 1 or 0, dl the byte. */
void tunnelOf(App* app, CPU& cpu, reg32 node)
{
    cpu.dl = byte(app, node + 0xe);
    if (cpu.dl & 0xf)
    {
        cpu.al = cpu.dl;
        cpu.al &= 0xf;
        cpu.eax = cpu.al == 2 ? 1 : 0;
    }
    else
    {
        cpu.al = cpu.dl;
        cpu.eax &= 0xff;
        cpu.eax = reg32(s(cpu.eax) >> 4);
        cpu.eax &= 0xf;
        cpu.eax = cpu.eax == 2 ? 1 : 0;
    }
}

}  // namespace

/* 0x420bf0 (stdcall, four words: the place's x, y and z, 16.16, its node):
 * whether the place is in a tunnel: which way of the node it is (its place
 * less the node's, 0x4e0000, dotted with its ahead vector, 0x4e01f0) picks
 * which of the node's nibbles at 0xe is asked first; 2 a tunnel.  1 or 0. */
void inTunnel(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x28;
    const reg32 b = cpu.ebp;
    x86::FPU& fpu = cpu.fpu;
    cpu.edx = at(app, b + 0x24);
    cpu.esi = cpu.edx * 8;
    cpu.esi += cpu.edx;
    cpu.eax = at(app, kNodes);
    cpu.esi <<= 2;
    putF32(app, b - 0x1c, fp::mul(i32(app, cpu.esi + cpu.eax), f64(app, kUnfixedFour)));
    putF32(app, b - 0x18, fp::mul(i32(app, cpu.esi + cpu.eax + 4), f64(app, kUnfixedFour)));
    {
        const Float z = fp::mul(i32(app, cpu.esi + cpu.eax + 8), f64(app, kUnfixedFour));
        cpu.ecx = b - 0x10;
        cpu.ebx = b - 0x1c;
        cpu.edx = b + 0x18;
        cpu.eax = 1;
        putF32(app, b - 0x14, z);
    }
    if (!byAddress(app, cpu, kLessAnother))
        return;
    cpu.eax = at(app, kNodes);
    cpu.edx = at(app, cpu.esi + cpu.eax + 0x15);
    cpu.edx = reg32(s(cpu.edx) >> 0x18);
    cpu.edx <<= 9;
    put(app, b - 4, cpu.edx);
    putF32(app, b - 0x28, fp::mul(i32(app, b - 4), f64(app, kUnfixedFour)));
    cpu.edx = at(app, cpu.esi + cpu.eax + 0x16);
    cpu.edx = reg32(s(cpu.edx) >> 0x18);
    cpu.edx <<= 9;
    put(app, b - 4, cpu.edx);
    putF32(app, b - 0x24, fp::mul(i32(app, b - 4), f64(app, kUnfixedFour)));
    cpu.eax = at(app, cpu.esi + cpu.eax + 0x17);
    cpu.eax = reg32(s(cpu.eax) >> 0x18);
    cpu.eax <<= 9;
    put(app, b - 4, cpu.eax);
    {
        const Float z = fp::mul(i32(app, b - 4), f64(app, kUnfixedFour));
        cpu.edx = b - 0x10;
        cpu.eax = b - 0x28;
        putF32(app, b - 0x20, z);
    }
    if (!byAddress(app, cpu, kDot))
        return;
    {
        const Float dot = fpu.st(0);
        const reg16 compared = status(cpu, Float(0.0), dot);
        fpu.count -= 1;
        if (compared & kBelow)
        {
            // Ahead of the node: the low nibble first.
            cpu.eax = at(app, kNodes);
            cpu.eax += cpu.esi;
            tunnelOf(app, cpu, cpu.eax);
        }
        else
        {
            cpu.eax = at(app, kNodes);
            cpu.esi += cpu.eax;
            cpu.eax = 0;
            cpu.al = byte(app, cpu.esi + 0xe);
            cpu.eax = reg32(s(cpu.eax) >> 4);
            if (cpu.al & 0xf)
            {
                cpu.eax &= 0xf;
                cpu.eax = cpu.eax == 2 ? 1 : 0;
            }
            else
            {
                cpu.al = byte(app, cpu.esi + 0xe);
                cpu.al &= 0xf;
                cpu.eax = cpu.al == 2 ? 1 : 0;
            }
        }
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4 + 0x10;
}

/* 0x420d60: whether the car eax is in a tunnel two nodes on (or back) from
 * its node (0x4217e0): its place (+8, floats, made 16.16) less its node's,
 * dotted with the node's ahead vector, picks which nibble is asked first;
 * its heading (+0x2c, the same) along the node's vector at 0x14 (the track
 * reversed at 0x6fd4c0 turning it) picks two on or two back.  1 or 0. */
void carFacingWay(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x30;
    const reg32 b = cpu.ebp;
    x86::FPU& fpu = cpu.fpu;
    cpu.edx = cpu.eax;
    cpu.eax = at(app, cpu.eax + 4);
    if (!byAddress(app, cpu, kCarNode))
        return;
    const auto fixed = [&](reg32 at) { return fp::mul(f32(app, cpu.edx + at), f64(app, kFixedThree)); };
    {
        const Float v = fixed(8);
        cpu.ecx = cpu.eax;
        put(app, b - 4, cpu.eax);
        cpu.eax = fistpPop(app, cpu, v);
    }
    {
        const Float v = fixed(0xc);
        put(app, b - 0x10, cpu.eax);
        cpu.eax = fistpPop(app, cpu, v);
    }
    {
        const Float v = fixed(0x10);
        put(app, b - 0x18, cpu.eax);
        cpu.eax = fistpPop(app, cpu, v);
    }
    {
        const Float v = fixed(0x2c);
        put(app, b - 0x14, cpu.eax);
        cpu.eax = fistpPop(app, cpu, v);
    }
    {
        const Float v = fixed(0x30);
        cpu.ebx = 0;
        cpu.esi = cpu.eax;
        cpu.eax = fistpPop(app, cpu, v);
    }
    {
        const Float v = fixed(0x34);
        put(app, b - 8, cpu.ebx);
        cpu.edi = cpu.eax;
        cpu.eax = fistpPop(app, cpu, v);
    }
    cpu.ebx = cpu.eax;
    cpu.eax = cpu.ecx;
    cpu.eax <<= 3;
    cpu.ecx += cpu.eax;
    cpu.eax = at(app, kNodes);
    cpu.ecx <<= 2;
    cpu.edx = at(app, b - 0x10);
    cpu.edx -= at(app, cpu.ecx + cpu.eax);
    put(app, b - 0x24, cpu.edx);
    cpu.edx = at(app, b - 0x18);
    cpu.edx -= at(app, cpu.ecx + cpu.eax + 4);
    put(app, b - 0x20, cpu.edx);
    cpu.edx = at(app, b - 0x14);
    cpu.edx -= at(app, cpu.ecx + cpu.eax + 8);
    put(app, b - 0x1c, cpu.edx);
    cpu.edx = at(app, cpu.ecx + cpu.eax + 0x15);
    cpu.edx = reg32(s(cpu.edx) >> 0x18);
    cpu.edx <<= 9;
    put(app, b - 0x30, cpu.edx);
    cpu.edx = at(app, cpu.ecx + cpu.eax + 0x16);
    cpu.edx = reg32(s(cpu.edx) >> 0x18);
    cpu.eax = at(app, cpu.ecx + cpu.eax + 0x17);
    cpu.eax = reg32(s(cpu.eax) >> 0x18);
    cpu.edx <<= 9;
    cpu.eax <<= 9;
    put(app, b - 0x2c, cpu.edx);
    put(app, b - 0x28, cpu.eax);
    // Which way of the node: the place's difference dotted with its ahead.
    cpu.edx = at(app, b - 0x24);
    cpu.eax = at(app, b - 0x30);
    fixedMul(cpu);
    cpu.edx = at(app, b - 0x20);
    put(app, b - 0x14, cpu.eax);
    cpu.eax = at(app, b - 0x2c);
    fixedMul(cpu);
    cpu.edx = at(app, b - 0x14);
    cpu.edx += cpu.eax;
    cpu.eax = at(app, b - 0x28);
    put(app, b - 0x10, cpu.edx);
    cpu.edx = at(app, b - 0x1c);
    fixedMul(cpu);
    cpu.edx = at(app, b - 0x10);
    cpu.edx += cpu.eax;
    put(app, b - 0xc, cpu.edx);
    // The heading along the node's vector at 0x14.
    cpu.eax = at(app, kNodes);
    cpu.eax = at(app, cpu.ecx + cpu.eax + 0x11);
    cpu.eax = reg32(s(cpu.eax) >> 0x18);
    cpu.edx = cpu.eax;
    cpu.eax = cpu.esi;
    fixedMul(cpu);
    cpu.esi = cpu.eax;
    cpu.eax = at(app, kNodes);
    cpu.edx = at(app, cpu.ecx + cpu.eax + 0x12);
    cpu.edx = reg32(s(cpu.edx) >> 0x18);
    cpu.eax = cpu.edi;
    fixedMul(cpu);
    cpu.esi += cpu.eax;
    cpu.eax = at(app, kNodes);
    cpu.edx = at(app, cpu.ecx + cpu.eax + 0x13);
    cpu.edx = reg32(s(cpu.edx) >> 0x18);
    cpu.eax = cpu.ebx;
    fixedMul(cpu);
    cpu.eax += cpu.esi;
    const bool reversed = at(app, kReversed) != 0;
    bool back = (!reversed && s(cpu.eax) < 0) || (reversed && s(cpu.eax) > 0) || at(app, b - 8) != 0;
    if (!back)
    {
        cpu.eax = 2;
        cpu.edx = at(app, b - 4);
        cpu.ecx = at(app, kNodeCount);
        cpu.eax += cpu.edx;
        if (s(cpu.eax) >= s(cpu.ecx))
            cpu.eax -= cpu.ecx;
    }
    else
    {
        cpu.eax = 0xfffffffe;
        cpu.eax += at(app, b - 4);
        if (s(cpu.eax) < 0)
            cpu.eax += at(app, kNodeCount);
    }
    cpu.edx = cpu.eax;
    cpu.eax <<= 3;
    cpu.eax += cpu.edx;
    cpu.edx = at(app, b - 0xc);
    cpu.eax <<= 2;
    if (s(cpu.edx) > 0)
    {
        cpu.edx = at(app, kNodes);
        cpu.eax += cpu.edx;
        tunnelOf(app, cpu, cpu.eax);
    }
    else
    {
        cpu.edx = at(app, kNodes);
        cpu.eax += cpu.edx;
        cpu.edx = 0;
        cpu.dl = byte(app, cpu.eax + 0xe);
        cpu.edx = reg32(s(cpu.edx) >> 4);
        if (cpu.dl & 0xf)
        {
            cpu.eax = cpu.edx;
            cpu.eax &= 0xf;
            cpu.eax = cpu.eax == 2 ? 1 : 0;
        }
        else
        {
            cpu.al = byte(app, cpu.eax + 0xe);
            cpu.al &= 0xf;
            cpu.eax = cpu.al == 2 ? 1 : 0;
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

/* 0x4211c0: the kind of the track at the car eax's node (+0x1c) -- the byte
 * at 3 of its entry in its block; the sum of its speeds' sizes (+0xa4,
 * +0xac) * 65536 is worked out, fistp'd, and dropped: the node itself. */
void carSectionKind(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 b = cpu.ebp;
    x86::FPU& fpu = cpu.fpu;
    cpu.edx = cpu.eax;
    cpu.ecx = cpu.eax;
    if (status(cpu, Float(0.0), f32(app, cpu.eax + 0xa4)) & kBelow)
    {
        cpu.eax = at(app, cpu.edx + 0xa4);
        put(app, b - 4, cpu.eax);
    }
    else
        putF32(app, b - 4, -f32(app, cpu.edx + 0xa4));
    if (status(cpu, Float(0.0), f32(app, cpu.ecx + 0xac)) & kBelow)
    {
        cpu.eax = at(app, cpu.ecx + 0xac);
        put(app, b - 8, cpu.eax);
    }
    else
        putF32(app, b - 8, -f32(app, cpu.ecx + 0xac));
    cpu.eax = fistpPop(app, cpu, fp::mul(fp::add(f32(app, b - 4), f32(app, b - 8)), f64(app, kFixedFive)));
    cpu.eax = 0;
    if (at(app, cpu.edx + 0x998) != 0)
        cpu.eax = 0u - cpu.eax;
    if (s(cpu.eax) >= 0)
    {
        cpu.esi = at(app, cpu.ecx + 0x1c);
        cpu.edi = at(app, kNodeCount);
        cpu.eax += cpu.esi;
        if (s(cpu.eax) >= s(cpu.edi))
            cpu.eax -= cpu.edi;
    }
    else
    {
        cpu.eax += at(app, cpu.ecx + 0x1c);
        if (s(cpu.eax) < 0)
            cpu.eax += at(app, kNodeCount);
    }
    cpu.ecx = cpu.eax;
    cpu.edx = cpu.ecx;
    cpu.edx = reg32(s(cpu.edx) >> 3);
    cpu.eax = cpu.edx * 4;
    cpu.eax -= cpu.edx;
    cpu.eax <<= 3;
    cpu.eax -= cpu.edx;
    cpu.eax <<= 6;
    cpu.eax += kBlocks;
    cpu.edx = at(app, cpu.eax + 0x58c);
    cpu.edi = at(app, cpu.eax + 0x584);
    cpu.eax = cpu.ecx;
    cpu.eax -= cpu.edi;
    cpu.al = byte(app, cpu.edx + cpu.eax * 8 + 3);
    cpu.eax &= 0xff;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace trackdata
}  // namespace nfs3hp
