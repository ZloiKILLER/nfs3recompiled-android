#include "native_guest.h"

/* The game's collisions -- car with car, car with the track's walls, car with
 * the track's objects -- the COLL file of a track's objects, and the drawing
 * code that follows them in nfs3.exe: render states, the software
 * renderer's blend, the polygon lists' drawers, the race's mouse pointer, and
 * the text lines of the race's messages.  disassemble_nfs3hp.py binds each
 * address to the function of that name here (COLLIDE_NATIVES), and the
 * generated code calls it by name.  tools/game_checks holds each against its
 * generated code (collide.inc).
 *
 * A car's record, as the collisions read it: its position at 0x98 (three
 * floats), its axes -- forward, up, right, three floats each -- at 0xc0,
 * 0xcc and 0xd8, its box's half sizes at 0x114, 0x118 and 0x11c, its mass's
 * share at 0xb0, its speed at 0xa4 (three floats), the radius it is tested
 * by at 0x120, the track polygon it is on at 0x0c (the polygon's corners,
 * 16.16 fixed points, at 0x24 to 0x5c of the record), its flags at 0x188 and
 * 0x200, and what the collision tells the sound at 0x168 to 0x180.
 *
 * Two cars are held against each other in each one's frame: the dot
 * products of their axes at 0x5efebc (nine floats, a row for each of the
 * first car's axes), those times the second car's half sizes at 0x5efee0 and
 * the first's at 0x5efe90, the line between them in either frame at
 * 0x5efe60; findContact's tests leave the contact's point at 0x5efe84, the
 * corner's offsets at 0x5eff04, 0x5efeb4 and 0x5efeb8, and contactNormal the
 * normal at 0x5efe78.
 *
 * Written as their code is: each push and pop to the guest's stack, each call
 * with the return address's slot, the frame's locals in the guest's memory,
 * the registers as the code leaves them.  The x87's arithmetic goes through
 * cpu.fpu's add, sub, mul and div in the code's order, with its operands in
 * the order the code has them; what the code keeps on the x87's stack across
 * a call or a branch is on cpu.fpu's stack, as the generated code keeps it;
 * a fcomp's status word is left in ax.  Every other function of the game's,
 * and the renderer's through the pointers the game keeps, is called by
 * address. */
namespace nfs3hp
{
namespace collide
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

// The two cars' frames, as findContact and its tests share them.
constexpr reg32 kAxisDots = 0x5efebc;        // nine floats: the first car's axes . the second's
constexpr reg32 kSecondExtents = 0x5efee0;   // nine: those times the second car's half sizes
constexpr reg32 kFirstExtents = 0x5efe90;    // nine: times the first car's
constexpr reg32 kBetween = 0x5efe60;         // six: the line between them in either frame
constexpr reg32 kContactNormal = 0x5efe78;   // three: contactNormal's
constexpr reg32 kContactPoint = 0x5efe84;    // three: where the tests found the contact
constexpr reg32 kCornerX = 0x5eff04, kCornerY = 0x5efeb4, kCornerZ = 0x5efeb8;
// The cars tested against each other this frame (addCarToList), and their count.
constexpr reg32 kCarList = 0x5efe10, kCarCount = 0x5537bc;
// The track's polygons' records: 0x24 bytes each.
constexpr reg32 kPolygonRecords = 0x5dd948;
// The x87's constants.
constexpr reg32 kFifteen = 0x5373fc, kQuarter = 0x537404, kFixedScale = 0x53740c;
constexpr reg32 kWallScale = 0x537414, kWallMarginRace = 0x53741c, kWallMarginOther = 0x537424;  // -2, -0.6
constexpr reg32 kSpeedStep = 0x53742c, kToFixed = 0x537434, kPolygonScale = 0x53743c;
constexpr reg32 kObjectScale = 0x537444, kObjectQuarter = 0x53744c, kObjectTwo = 0x537450;
constexpr reg32 kObjectToFixed = 0x537454, kObjectHalf = 0x53745c, kObjectSpeed = 0x537464;
constexpr reg32 kDepthNear = 0x537474, kDepthFar = 0x53747c, kPointerSize = 0x53748c;
// The wall test's tables in nfs3.exe's code: three zero floats twice, the
// four edges' bits.
constexpr reg32 kZeroVector = 0x42dd30, kZeroVector2 = 0x42dd3c, kEdgeBits = 0x42dd48;

// What is called by address.
constexpr reg32 kFaceInFirst = 0x42dd60, kFaceInSecond = 0x42e010;
constexpr reg32 kEdgeInFirst = 0x42e2c0, kEdgeInSecond = 0x42e4f0;
constexpr reg32 kCarsRespond = 0x42f280;
constexpr reg32 kDot = 0x4e01f0, kNormalize = 0x4972f0, kAddVectors = 0x4dffb0;
constexpr reg32 kSubtractVectors = 0x4e0000, kScaleVectors = 0x4e0050;

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

/* A loop's safepoint, where the generated code has one: the guest's context
 * handed over when another thread waits for it. */
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

Float loadFloat(App* app, reg32 address)
{
    return Float(app->getMemory<float>(address));
}

void storeFloat(App* app, reg32 address, const Float& value)
{
    app->getMemory<float>(address) = float(value);
}

Float loadDouble(App* app, reg32 address)
{
    return Float(app->getMemory<double>(address));
}

void storeDouble(App* app, reg32 address, const Float& value)
{
    app->getMemory<double>(address) = double(value);
}

/* fild dword. */
Float loadInteger(App* app, reg32 address)
{
    return Float(sreg32(at(app, address)));
}

/* fcom(p); fnstsw ax: a compared with b, the status word left in ax; sahf's
 * CF (C0) as `below`, its ZF (C3) as `equal` -- both for an unordered pair. */
struct Compared
{
    bool below, equal;
    bool belowOrEqual() const { return below || equal; }
};

Compared compare(CPU& cpu, const Float& a, const Float& b)
{
    cpu.fpu.compare(a, b);
    cpu.ax = cpu.fpu.status.word;
    return {(cpu.fpu.status.word & 0x100) != 0, (cpu.fpu.status.word & 0x4000) != 0};
}

/* What a callee left on the x87's stack, taken off it as the generated code
 * does: st(0) read, the stack one shorter. */
Float popTop(CPU& cpu)
{
    const Float value = cpu.fpu.st(0);
    cpu.fpu.count -= 1;
    return value;
}

/* A value the code keeps on the x87's stack past a branch or a call. */
void pushTop(CPU& cpu, const Float& value)
{
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = value;
}

/* movsd: a dword from [esi] to es:[edi], both stepped by the direction flag. */
void movsd(App* app, CPU& cpu)
{
    put(app, cpu.ees + cpu.edi, at(app, cpu.esi));
    const reg32 step = cpu.flags.df ? reg32(0) - 4 : 4;
    cpu.edi += step;
    cpu.esi += step;
}

/* rep movsd: ecx dwords. */
void repMovsd(App* app, CPU& cpu)
{
    for (; cpu.ecx != 0; --cpu.ecx)
        movsd(app, cpu);
}

/* The code's 16.16 multiply -- imul edx; shl edx, 16; shr eax, 16; adc eax,
 * edx -- of eax by edx, rounded by the bit shr shifts out; edx as it leaves
 * it. */
void fixedMultiply(CPU& cpu)
{
    const std::int64_t product = std::int64_t(sreg32(cpu.eax)) * sreg32(cpu.edx);
    cpu.eax = reg32(product);
    cpu.edx = reg32(std::uint64_t(product) >> 32);
    cpu.edx <<= 16;
    const reg32 carry = (cpu.eax >> 15) & 1;
    cpu.eax >>= 16;
    cpu.eax += cpu.edx + carry;
}

}  // namespace

void findContact(App* app, CPU& cpu);
void contactNormal(App* app, CPU& cpu);
void pushCarsApart(App* app, CPU& cpu);
void separateCars(App* app, CPU& cpu);
void polygonEdgeNormal(App* app, CPU& cpu);
void clearOfWall(App* app, CPU& cpu);
void carAgainstWalls(App* app, CPU& cpu);
void collideCars(App* app, CPU& cpu);
void clearCarList(App* app, CPU& cpu);
void objectInReach(App* app, CPU& cpu);
void carAgainstObjects(App* app, CPU& cpu);
void addCarToList(App* app, CPU& cpu);
void carCollisions(App* app, CPU& cpu);
void distanceSquared(App* app, CPU& cpu);
void nearestPoint(App* app, CPU& cpu);
void openCollFile(App* app, CPU& cpu);
void closeCollFile(App* app, CPU& cpu);
void collFileSize(App* app, CPU& cpu);
void readCollFile(App* app, CPU& cpu);
void collFileCount(App* app, CPU& cpu);
void collChunk(App* app, CPU& cpu);
void playerControls(App* app, CPU& cpu);
void forgetRenderStates(App* app, CPU& cpu);
void statesForText(App* app, CPU& cpu);
void statesForPictures(App* app, CPU& cpu);
void statesForHud(App* app, CPU& cpu);
void statesForScreens(App* app, CPU& cpu);
void passLayerNothing(App* app, CPU& cpu);
void passSecondLayer(App* app, CPU& cpu);
void passFirstLayer(App* app, CPU& cpu);
void makeBlendTables(App* app, CPU& cpu);
void blendSpan(App* app, CPU& cpu);
void subdivisionDepth(App* app, CPU& cpu);
void drawTexturedTriangles(App* app, CPU& cpu);
void drawFlatTriangles(App* app, CPU& cpu);
void drawTrianglesByZ(App* app, CPU& cpu);
void drawQuadsByZ(App* app, CPU& cpu);
void drawFlatTrianglesAgain(App* app, CPU& cpu);
void drawColouredTriangles(App* app, CPU& cpu);
void stateRecord(App* app, CPU& cpu);
void drawRecords(App* app, CPU& cpu);
void raceTextFrame(App* app, CPU& cpu);
void setMirrored(App* app, CPU& cpu);
void drawMirroredRecords(App* app, CPU& cpu);
void countFrame(App* app, CPU& cpu);
void raceViewDistance(App* app, CPU& cpu);
void raceFrameEnd(App* app, CPU& cpu);
void raceFrameNothing(App* app, CPU& cpu);
void raceEndNothing(App* app, CPU& cpu);
void raceStartDrawing(App* app, CPU& cpu);
void raceStartNothing(App* app, CPU& cpu);
void drawPointer(App* app, CPU& cpu);
void openMessages(App* app, CPU& cpu);
void closeMessages(App* app, CPU& cpu);
void clearAllMessages(App* app, CPU& cpu);
void clearMessages(App* app, CPU& cpu);
void clearMessageLine(App* app, CPU& cpu);
void addMessageText(App* app, CPU& cpu);
void addMessageCharacter(App* app, CPU& cpu);
void copyMessageLines(App* app, CPU& cpu);
void messagesScrolled(App* app, CPU& cpu);
void messagesNothing(App* app, CPU& cpu);
void seedHash(App* app, CPU& cpu);

/* 0x42f550: whether, and how, the boxes of car eax and car edx touch -- the
 * first's six faces' centres and then its eight corners are tried, each
 * point's signs in ebx, ecx and on the stack: 1 a face's centre in the first
 * car's box (0x42dd60), 2 in the second's (0x42e010), 3 a corner in the
 * first's (0x42e2c0), 4 in the second's (0x42e4f0); 0 none. */
void findContact(App* app, CPU& cpu)
{
    struct Probe
    {
        reg32 test, pushed, ebx, ecx, kind;
    };
    static const Probe probes[] = {
        {kFaceInFirst, 0, 1, 0, 1}, {kFaceInFirst, 0, 0, 1, 1}, {kFaceInFirst, 1, 0, 0, 1},
        {kFaceInFirst, 0, reg32(-1), 0, 1}, {kFaceInFirst, 0, 0, reg32(-1), 1}, {kFaceInFirst, reg32(-1), 0, 0, 1},
        {kFaceInSecond, 0, 1, 0, 2}, {kFaceInSecond, 0, 0, 1, 2}, {kFaceInSecond, 1, 0, 0, 2},
        {kFaceInSecond, 0, reg32(-1), 0, 2}, {kFaceInSecond, 0, 0, reg32(-1), 2}, {kFaceInSecond, reg32(-1), 0, 0, 2},
        {kEdgeInFirst, 1, 1, 1, 3}, {kEdgeInFirst, reg32(-1), 1, 1, 3},
        {kEdgeInFirst, 1, 1, reg32(-1), 3}, {kEdgeInFirst, reg32(-1), 1, reg32(-1), 3},
        {kEdgeInFirst, 1, reg32(-1), 1, 3}, {kEdgeInFirst, reg32(-1), reg32(-1), 1, 3},
        {kEdgeInFirst, 1, reg32(-1), reg32(-1), 3}, {kEdgeInFirst, reg32(-1), reg32(-1), reg32(-1), 3},
        {kEdgeInSecond, 1, 1, 1, 4}, {kEdgeInSecond, reg32(-1), 1, 1, 4},
        {kEdgeInSecond, 1, 1, reg32(-1), 4}, {kEdgeInSecond, reg32(-1), 1, reg32(-1), 4},
        {kEdgeInSecond, 1, reg32(-1), 1, 4}, {kEdgeInSecond, reg32(-1), reg32(-1), 1, 4},
        {kEdgeInSecond, 1, reg32(-1), reg32(-1), 4}, {kEdgeInSecond, reg32(-1), reg32(-1), reg32(-1), 4},
    };
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edi = cpu.eax;
    cpu.esi = cpu.edx;
    // Each test takes its third sign off the stack (ret 4); a 0 pushed after
    // a test is its eax, 0.
    for (const Probe& probe : probes)
    {
        push(app, cpu, probe.pushed);
        cpu.ebx = probe.ebx;
        cpu.ecx = probe.ecx;
        cpu.edx = cpu.esi;
        cpu.eax = cpu.edi;
        if (!byAddress(app, cpu, probe.test))
            return;
        if (cpu.eax != 0)
        {
            cpu.eax = probe.kind;
            break;
        }
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

namespace
{

/* contactNormal's: whether an axis is to be turned round, by the sign of
 * its offset at `offset` -- for ebx nonzero when 0 is below it, else when 0
 * is above it; eax 1 or 0. */
bool turnAxis(App* app, CPU& cpu, reg32 offset)
{
    const Compared c = compare(cpu, 0.0, loadFloat(app, offset));
    if (cpu.ebx != 0)
        cpu.eax = c.below ? 1 : 0;
    else
        cpu.eax = c.belowOrEqual() ? 0 : 1;
    return cpu.eax != 0;
}

/* |a . b|, as contactNormal works it out: the dot product (0x4e01f0) once
 * to see its sign, again for the value, turned round if it is not above
 * 0, into the frame's float at `to`.  False when the game stopped. */
bool absoluteDot(App* app, CPU& cpu, reg32 a, reg32 b, reg32 to)
{
    cpu.edx = b;
    cpu.eax = a;
    if (!byAddress(app, cpu, kDot))
        return false;
    const Compared c = compare(cpu, 0.0, cpu.fpu.st(0));
    cpu.fpu.count -= 1;
    cpu.edx = b;
    cpu.eax = a;
    if (!byAddress(app, cpu, kDot))
        return false;
    if (!c.below)
        cpu.fpu.st(0) = -cpu.fpu.st(0);
    storeFloat(app, to, popTop(cpu));
    return true;
}

}  // namespace

/* 0x42f8c0: the normal of the contact findContact found between car eax and
 * car edx, into kContactNormal: when the cars move fast apart (the largest
 * of their speeds' differences above 15), the axis of the second car (the
 * first, for ebx nonzero) along which their speeds' difference goes most --
 * each axis turned round by its corner offset's sign first; else the axis of
 * the second car whose box the contact point is least inside, turned round
 * by its offset's sign.  ebx, edx and eax left as the code leaves them. */
void contactNormal(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x7c;
    const reg32 ebp = cpu.ebp;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.edx;
    if (cpu.ebx != 0)
        cpu.ecx = cpu.eax;
    else
        cpu.edx = cpu.eax;
    // The speeds' differences, made positive, into -0x30, -0x48, -0x44.
    struct Axis
    {
        reg32 speed, difference, positive;
    };
    static const Axis axes[] = {{0xa4, 0x24, 0x30}, {0xa8, 0x14, 0x48}, {0xac, 0x18, 0x44}};
    for (int i = 0; i < 3; ++i)
    {
        const Axis& a = axes[i];
        const Float zero = 0.0;
        const Float difference = fp::sub(loadFloat(app, cpu.ecx + a.speed), loadFloat(app, cpu.edx + a.speed));
        if (i == 1)
        {
            cpu.eax = at(app, ebp - 0x30);
            put(app, ebp - 0x3c, cpu.eax);
        }
        else if (i == 2)
        {
            cpu.eax = at(app, ebp - 0x48);
            put(app, ebp - 0x38, cpu.eax);
        }
        storeFloat(app, ebp - a.difference, difference);
        if (compare(cpu, zero, loadFloat(app, ebp - a.difference)).below)
        {
            cpu.eax = at(app, ebp - a.difference);
            put(app, ebp - a.positive, cpu.eax);
        }
        else
            storeFloat(app, ebp - a.positive, -loadFloat(app, ebp - a.difference));
    }
    // Their largest, into -0xc.
    cpu.eax = at(app, ebp - 0x44);
    put(app, ebp - 8, cpu.eax);
    cpu.eax = at(app, ebp - 0x3c);
    const Float y = loadFloat(app, ebp - 0x38);
    put(app, ebp - 0xc, cpu.eax);
    if (!compare(cpu, y, loadFloat(app, ebp - 0x3c)).belowOrEqual())
    {
        cpu.eax = at(app, ebp - 0x38);
        put(app, ebp - 0xc, cpu.eax);
    }
    if (!compare(cpu, loadFloat(app, ebp - 8), loadFloat(app, ebp - 0xc)).belowOrEqual())
    {
        cpu.eax = at(app, ebp - 8);
        put(app, ebp - 0xc, cpu.eax);
    }
    if (!compare(cpu, loadFloat(app, ebp - 0xc), loadDouble(app, kFifteen)).belowOrEqual())
    {
        // Fast: the speeds' difference at -0x7c, the second car's axes at
        // -0x64, -0x58 and -0x70, each turned round by its offset's sign.
        cpu.edi = ebp - 0x7c;
        cpu.esi = cpu.ecx + 0xa4;
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        cpu.edi = ebp - 0x64;
        cpu.esi = cpu.edx + 0xc0;
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        cpu.edi = ebp - 0x58;
        cpu.esi = cpu.edx + 0xcc;
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        cpu.edi = ebp - 0x70;
        cpu.esi = cpu.edx + 0xd8;
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        if (turnAxis(app, cpu, kCornerX))
        {
            cpu.dl = byte(app, ebp - 0x5d);
            cpu.dl ^= 0x80;
            cpu.ah = byte(app, ebp - 0x61);
            putByte(app, ebp - 0x5d, cpu.dl);
            cpu.dh = byte(app, ebp - 0x59);
            cpu.ah ^= 0x80;
            cpu.dh ^= 0x80;
            putByte(app, ebp - 0x61, cpu.ah);
            putByte(app, ebp - 0x59, cpu.dh);
        }
        if (turnAxis(app, cpu, kCornerY))
        {
            cpu.cl = byte(app, ebp - 0x55);
            cpu.cl ^= 0x80;
            cpu.al = byte(app, ebp - 0x4d);
            putByte(app, ebp - 0x55, cpu.cl);
            cpu.ch = byte(app, ebp - 0x51);
            cpu.al ^= 0x80;
            cpu.ch ^= 0x80;
            putByte(app, ebp - 0x4d, cpu.al);
            putByte(app, ebp - 0x51, cpu.ch);
        }
        if (turnAxis(app, cpu, kCornerZ))
        {
            cpu.dl = byte(app, ebp - 0x69);
            cpu.dl ^= 0x80;
            cpu.ah = byte(app, ebp - 0x6d);
            putByte(app, ebp - 0x69, cpu.dl);
            cpu.dh = byte(app, ebp - 0x65);
            cpu.ah ^= 0x80;
            cpu.dh ^= 0x80;
            putByte(app, ebp - 0x6d, cpu.ah);
            putByte(app, ebp - 0x65, cpu.dh);
        }
        // How much of the speeds' difference goes along each.
        if (!absoluteDot(app, cpu, ebp - 0x64, ebp - 0x7c, ebp - 4))
            return;
        cpu.eax = at(app, ebp - 4);
        put(app, ebp - 0x2c, cpu.eax);
        if (!absoluteDot(app, cpu, ebp - 0x58, ebp - 0x7c, ebp - 0x40))
            return;
        cpu.eax = at(app, ebp - 0x40);
        put(app, ebp - 0x34, cpu.eax);
        if (!absoluteDot(app, cpu, ebp - 0x70, ebp - 0x7c, ebp - 0x10))
            return;
        cpu.eax = at(app, ebp - 0x10);
        put(app, ebp - 0x4c, cpu.eax);
        if (!compare(cpu, loadFloat(app, ebp - 0x2c), loadFloat(app, ebp - 0x34)).belowOrEqual()
            && !compare(cpu, loadFloat(app, ebp - 0x2c), loadFloat(app, ebp - 0x10)).belowOrEqual())
        {
            cpu.eax = at(app, ebp - 0x64);
            put(app, kContactNormal, cpu.eax);
            cpu.eax = at(app, ebp - 0x60);
            put(app, kContactNormal + 4, cpu.eax);
            cpu.eax = at(app, ebp - 0x5c);
        }
        else if (!compare(cpu, loadFloat(app, ebp - 0x34), loadFloat(app, ebp - 0x4c)).belowOrEqual())
        {
            cpu.eax = at(app, ebp - 0x58);
            put(app, kContactNormal, cpu.eax);
            cpu.eax = at(app, ebp - 0x54);
            put(app, kContactNormal + 4, cpu.eax);
            cpu.eax = at(app, ebp - 0x50);
            put(app, kContactNormal + 8, cpu.eax);
            return leave();
        }
        else
        {
            cpu.eax = at(app, ebp - 0x70);
            put(app, kContactNormal, cpu.eax);
            cpu.eax = at(app, ebp - 0x6c);
            put(app, kContactNormal + 4, cpu.eax);
            cpu.eax = at(app, ebp - 0x68);
        }
        put(app, kContactNormal + 8, cpu.eax);
        return leave();
    }
    // Slow: how far inside each of the second car's half sizes the corner
    // is, into -0x20, -0x1c, -0x28.
    struct Inside
    {
        reg32 offset, size, to;
    };
    static const Inside insides[] = {{kCornerX, 0x114, 0x20}, {kCornerY, 0x118, 0x1c}, {kCornerZ, 0x11c, 0x28}};
    for (const Inside& in : insides)
    {
        const bool above = !compare(cpu, 0.0, loadFloat(app, in.offset)).belowOrEqual();
        const Float size = loadFloat(app, cpu.edx + in.size);
        storeFloat(app, ebp - in.to,
                   above ? fp::add(size, loadFloat(app, in.offset)) : fp::sub(size, loadFloat(app, in.offset)));
    }
    reg32 axis, offset;
    if (compare(cpu, loadFloat(app, ebp - 0x20), loadFloat(app, ebp - 0x1c)).below
        && compare(cpu, loadFloat(app, ebp - 0x20), loadFloat(app, ebp - 0x28)).below)
    {
        axis = 0xc0;
        offset = kCornerX;
    }
    else if (compare(cpu, loadFloat(app, ebp - 0x1c), loadFloat(app, ebp - 0x28)).below)
    {
        axis = 0xcc;
        offset = kCornerY;
    }
    else
    {
        axis = 0xd8;
        offset = kCornerZ;
    }
    cpu.edi = kContactNormal;
    cpu.esi = cpu.edx + axis;
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    if (!turnAxis(app, cpu, offset))
        return leave();
    if (axis == 0xc0)
    {
        cpu.bl = byte(app, kContactNormal + 3);
        cpu.bl ^= 0x80;
        cpu.cl = byte(app, kContactNormal + 0xb);
        putByte(app, kContactNormal + 3, cpu.bl);
        cpu.bh = byte(app, kContactNormal + 7);
        cpu.cl ^= 0x80;
        cpu.bh ^= 0x80;
        putByte(app, kContactNormal + 0xb, cpu.cl);
        putByte(app, kContactNormal + 7, cpu.bh);
    }
    else if (axis == 0xcc)
    {
        cpu.al = byte(app, kContactNormal + 7);
        cpu.al ^= 0x80;
        cpu.ch = byte(app, kContactNormal + 3);
        putByte(app, kContactNormal + 7, cpu.al);
        cpu.ah = byte(app, kContactNormal + 0xb);
        cpu.ch ^= 0x80;
        cpu.ah ^= 0x80;
        putByte(app, kContactNormal + 3, cpu.ch);
        putByte(app, kContactNormal + 0xb, cpu.ah);
    }
    else
    {
        cpu.dl = byte(app, kContactNormal + 3);
        cpu.dl ^= 0x80;
        cpu.bl = byte(app, kContactNormal + 0xb);
        putByte(app, kContactNormal + 3, cpu.dl);
        cpu.dh = byte(app, kContactNormal + 7);
        cpu.bl ^= 0x80;
        cpu.dh ^= 0x80;
        putByte(app, kContactNormal + 0xb, cpu.bl);
        putByte(app, kContactNormal + 7, cpu.dh);
    }
    leave();
}

/* 0x42fe10: car eax and car edx pushed apart along the direction on the
 * stack (three floats, normalized first, 0x4972f0), each by a quarter of
 * the direction times the other's share of their masses (0xb0) -- the first
 * back, the second on (0x4e0050, 0x4dffb0).  1; ret 0xc. */
void pushCarsApart(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x30;
    const reg32 ebp = cpu.ebp;
    cpu.ecx = cpu.eax;
    cpu.esi = cpu.edx;
    cpu.eax = at(app, ebp + 0x14);
    put(app, ebp - 0x24, cpu.eax);
    cpu.eax = at(app, ebp + 0x18);
    put(app, ebp - 0x20, cpu.eax);
    cpu.eax = at(app, ebp + 0x1c);
    put(app, ebp - 0x1c, cpu.eax);
    cpu.eax = ebp - 0x24;
    if (!byAddress(app, cpu, kNormalize))
        return;
    // The direction's length stays on the x87's stack until the fstp st(0).
    const Float sum = fp::add(loadFloat(app, cpu.ecx + 0xb0), loadFloat(app, cpu.edx + 0xb0));
    const Float second = loadFloat(app, cpu.edx + 0xb0);
    const Float reciprocal = fp::div(1.0, sum);
    const Float secondShare = fp::mul(second, reciprocal);
    const Float firstShare = fp::mul(reciprocal, loadFloat(app, cpu.ecx + 0xb0));
    const Float quarter = loadDouble(app, kQuarter);
    const Float back = fp::mul(secondShare, quarter);
    cpu.eax = ebp - 0x18;
    const Float on = fp::mul(quarter, firstShare);
    push(app, cpu, cpu.eax);
    cpu.esp -= 4;
    cpu.edx = ebp - 0x24;
    cpu.eax = 1;
    storeFloat(app, cpu.esp, -back);
    storeFloat(app, ebp - 0xc, on);
    cpu.fpu.count -= 1;
    if (!byAddress(app, cpu, kScaleVectors))
        return;
    cpu.eax = ebp - 0x30;
    push(app, cpu, cpu.eax);
    cpu.edx = ebp - 0x24;
    push(app, cpu, at(app, ebp - 0xc));
    cpu.eax = 1;
    cpu.ebx = ebp - 0x18;
    if (!byAddress(app, cpu, kScaleVectors))
        return;
    cpu.edx = cpu.ecx + 0x98;
    cpu.eax = 1;
    cpu.ecx = cpu.edx;
    if (!byAddress(app, cpu, kAddVectors))
        return;
    cpu.ebx = ebp - 0x30;
    cpu.edx = cpu.esi + 0x98;
    cpu.eax = 1;
    cpu.ecx = cpu.edx;
    if (!byAddress(app, cpu, kAddVectors))
        return;
    cpu.eax = 1;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4 + 0xc;
}

/* 0x42fee0: two cars, eax and edx, whose boxes may overlap, taken apart: the
 * frames findContact's tests share worked out (kAxisDots, kSecondExtents,
 * kFirstExtents), then up to four times the line between the cars into
 * either frame (kBetween), findContact, and for a contact the normal
 * (contactNormal, for a corner) and the cars pushed apart along the line
 * between them (pushCarsApart).  pushCarsApart's last result, or 0 for no
 * contact the first time. */
void separateCars(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x1c;
    const reg32 ebp = cpu.ebp;
    cpu.esi = cpu.eax;
    cpu.edi = cpu.edx;
    cpu.eax = cpu.edi;
    cpu.ebx = cpu.esi + 0xc0;
    cpu.edx = 0;
    cpu.eax += 0xc0;
    put(app, ebp - 0xc, cpu.edx);
    put(app, ebp - 4, cpu.eax);
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ebx;
    // The nine dot products of the first car's axes with the second's.
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.eax = cpu.edi;
    cpu.eax += 0xcc;
    storeFloat(app, kAxisDots, popTop(cpu));
    put(app, ebp - 0x10, cpu.eax);
    cpu.edx = cpu.eax;
    cpu.eax = cpu.ebx;
    cpu.ecx = cpu.edi + 0xd8;
    if (!byAddress(app, cpu, kDot))
        return;
    cpu.edx = cpu.ecx;
    cpu.eax = cpu.ebx;
    storeFloat(app, kAxisDots + 4, popTop(cpu));
    if (!byAddress(app, cpu, kDot))
        return;
    for (reg32 row = 1; row < 3; ++row)
    {
        cpu.ebx = cpu.esi + 0xc0 + 0xc * row;
        cpu.edx = at(app, ebp - 4);
        cpu.eax = cpu.ebx;
        storeFloat(app, kAxisDots + 0xc * row - 4, popTop(cpu));
        if (!byAddress(app, cpu, kDot))
            return;
        cpu.edx = at(app, ebp - 0x10);
        cpu.eax = cpu.ebx;
        storeFloat(app, kAxisDots + 0xc * row, popTop(cpu));
        if (!byAddress(app, cpu, kDot))
            return;
        cpu.edx = cpu.ecx;
        cpu.eax = cpu.ebx;
        storeFloat(app, kAxisDots + 0xc * row + 4, popTop(cpu));
        if (!byAddress(app, cpu, kDot))
            return;
    }
    storeFloat(app, kAxisDots + 0x20, popTop(cpu));
    // Those times the second car's half sizes (p) and the first's (q), in
    // the code's order, stored as the code's fxch and fstp leave them.
    const auto dot = [&](reg32 i) { return loadFloat(app, kAxisDots + 4 * i); };
    Float p[9], q[9];
    for (reg32 i = 0; i < 7; ++i)
        p[i] = fp::mul(dot(i), loadFloat(app, cpu.edi + 0x114 + 4 * (i % 3)));
    storeFloat(app, kSecondExtents + 4, p[1]);
    p[7] = fp::mul(dot(7), loadFloat(app, cpu.edi + 0x118));
    storeFloat(app, kSecondExtents + 8, p[2]);
    p[8] = fp::mul(dot(8), loadFloat(app, cpu.edi + 0x11c));
    storeFloat(app, kSecondExtents + 0xc, p[3]);
    for (reg32 i = 0; i < 6; ++i)
    {
        q[i] = fp::mul(dot(i), loadFloat(app, cpu.esi + 0x114 + 4 * (i / 3)));
        storeFloat(app, i < 5 ? kSecondExtents + 0x10 + 4 * i : kFirstExtents, i < 5 ? p[4 + i] : q[0]);
    }
    q[6] = fp::mul(dot(6), loadFloat(app, cpu.esi + 0x11c));
    storeFloat(app, kFirstExtents + 4, q[1]);
    q[7] = fp::mul(dot(7), loadFloat(app, cpu.esi + 0x11c));
    cpu.ecx = 0;
    storeFloat(app, kFirstExtents + 8, q[2]);
    q[8] = fp::mul(dot(8), loadFloat(app, cpu.esi + 0x11c));
    put(app, ebp - 8, cpu.ecx);
    for (reg32 i = 3; i < 9; ++i)
        storeFloat(app, kFirstExtents + 4 * i, q[i]);
    storeFloat(app, kSecondExtents, p[0]);
    const auto leave = [&] {
        cpu.eax = at(app, ebp - 0xc);
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    for (;;)
    {
        // The line from the first car to the second, into either frame.
        cpu.ecx = ebp - 0x1c;
        cpu.eax = 1;
        cpu.ebx = cpu.esi + 0x98;
        cpu.edx = cpu.edi + 0x98;
        if (!byAddress(app, cpu, kSubtractVectors))
            return;
        static const reg32 axes[] = {0xc0, 0xcc, 0xd8};
        for (int i = 0; i < 6; ++i)
        {
            cpu.edx = ebp - 0x1c;
            cpu.eax = (i < 3 ? cpu.esi : cpu.edi) + axes[i % 3];
            if (i > 0)
                storeFloat(app, kBetween + 4 * (i - 1), popTop(cpu));
            if (!byAddress(app, cpu, kDot))
                return;
        }
        cpu.edx = cpu.edi;
        cpu.eax = cpu.esi;
        storeFloat(app, kBetween + 0x14, popTop(cpu));
        if (!direct(app, cpu, findContact))
            return;
        if (cpu.eax == 0)
            return leave();
        safepoint(app, cpu);
        if (cpu.eax == 3)
        {
            cpu.edx = cpu.edi;
            cpu.eax = cpu.esi;
            cpu.ebx = 0;
            if (!direct(app, cpu, contactNormal))
                return;
        }
        else if (cpu.eax == 4)
        {
            cpu.ebx = 1;
            cpu.edx = cpu.edi;
            cpu.eax = cpu.esi;
            if (!direct(app, cpu, contactNormal))
                return;
        }
        cpu.eax = at(app, ebp - 0x14);
        cpu.edx = at(app, ebp - 0x18);
        push(app, cpu, cpu.eax);
        cpu.ecx = at(app, ebp - 0x1c);
        cpu.ebx = at(app, ebp - 8);
        push(app, cpu, cpu.edx);
        ++cpu.ebx;
        cpu.eax = cpu.esi;
        push(app, cpu, cpu.ecx);
        cpu.edx = cpu.edi;
        put(app, ebp - 8, cpu.ebx);
        if (!direct(app, cpu, pushCarsApart))
            return;
        put(app, ebp - 0xc, cpu.eax);
        if (sreg32(at(app, ebp - 8)) >= 4)
            return leave();
    }
}

/* 0x430210: the wall along edge eax (1, 2, 4 or 8) of the track polygon at
 * ebx, as the code works it out in 16.16: the edge's run and rise (from the
 * polygon's corners at ebx + 0x24 on) into -0x10 and -8, the vector they
 * make with 0 between them normalized (0x4970c0), its cross product with
 * up (0, 1, 0) -- each term a 16.16 multiply -- into floats at edx (as the
 * fstps leave them: the third term at +8, the first at +0, the second at
 * +4).  Another eax: the frame's -0x10 and -8 as they were. */
void polygonEdgeNormal(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x28;
    const reg32 ebp = cpu.ebp;
    cpu.ecx = cpu.edx;
    cpu.edx = 0;
    put(app, ebp - 0x28, cpu.edx);
    put(app, ebp - 0xc, cpu.edx);
    put(app, ebp - 0x20, cpu.edx);
    put(app, ebp - 0x24, 0x10000);
    if (cpu.eax == 1 || cpu.eax == 2 || cpu.eax == 4 || cpu.eax == 8)
    {
        if (cpu.eax == 1)
        {
            cpu.esi = at(app, cpu.ebx + 0x38);
            cpu.eax = at(app, cpu.ebx + 0x44);
            cpu.eax -= cpu.esi;
            put(app, ebp - 8, cpu.eax);
            cpu.eax = at(app, cpu.ebx + 0x3c);
            cpu.edi = at(app, cpu.ebx + 0x30);
            cpu.eax -= cpu.edi;
        }
        else if (cpu.eax == 2)
        {
            cpu.edx = at(app, cpu.ebx + 0x50);
            cpu.eax = at(app, cpu.ebx + 0x2c);
            cpu.eax -= cpu.edx;
            put(app, ebp - 8, cpu.eax);
            cpu.eax = at(app, cpu.ebx + 0x24);
            cpu.esi = at(app, cpu.ebx + 0x48);
            cpu.eax -= cpu.esi;
        }
        else if (cpu.eax == 4)
        {
            cpu.edi = at(app, cpu.ebx + 0x2c);
            cpu.eax = at(app, cpu.ebx + 0x38);
            cpu.eax -= cpu.edi;
            put(app, ebp - 8, cpu.eax);
            cpu.eax = at(app, cpu.ebx + 0x30);
            cpu.edx = at(app, cpu.ebx + 0x24);
            cpu.eax -= cpu.edx;
        }
        else
        {
            cpu.esi = at(app, cpu.ebx + 0x44);
            cpu.eax = at(app, cpu.ebx + 0x50);
            cpu.eax -= cpu.esi;
            put(app, ebp - 8, cpu.eax);
            cpu.eax = at(app, cpu.ebx + 0x48);
            cpu.edi = at(app, cpu.ebx + 0x3c);
            cpu.eax -= cpu.edi;
        }
        put(app, ebp - 0x10, cpu.eax);
    }
    cpu.eax = ebp - 0x10;
    cpu.edx = at(app, ebp - 0x20);
    if (!byAddress(app, cpu, 0x4970c0))
        return;
    // The cross product, a 16.16 multiply a term (edx the callee's, 0).
    cpu.eax = at(app, ebp - 0xc);
    fixedMultiply(cpu);
    cpu.edx = at(app, ebp - 0x24);
    cpu.ebx = cpu.eax;
    cpu.eax = at(app, ebp - 8);
    fixedMultiply(cpu);
    cpu.edx = at(app, ebp - 0x28);
    cpu.ebx -= cpu.eax;
    cpu.eax = at(app, ebp - 8);
    put(app, ebp - 0x1c, cpu.ebx);
    fixedMultiply(cpu);
    cpu.edx = at(app, ebp - 0x20);
    cpu.ebx = cpu.eax;
    cpu.eax = at(app, ebp - 0x10);
    fixedMultiply(cpu);
    cpu.edx = at(app, ebp - 0x24);
    cpu.ebx -= cpu.eax;
    cpu.eax = at(app, ebp - 0x10);
    put(app, ebp - 0x18, cpu.ebx);
    fixedMultiply(cpu);
    cpu.edx = at(app, ebp - 0x28);
    cpu.ebx = cpu.eax;
    cpu.eax = at(app, ebp - 0xc);
    fixedMultiply(cpu);
    cpu.ebx -= cpu.eax;
    put(app, ebp - 4, cpu.ebx);
    // To floats, each through the frame's -4.
    cpu.eax = at(app, ebp - 0x1c);
    const Float scale = loadDouble(app, kFixedScale);
    const Float third = fp::mul(loadInteger(app, ebp - 4), scale);
    put(app, ebp - 4, cpu.eax);
    cpu.eax = at(app, ebp - 0x18);
    const Float first = loadInteger(app, ebp - 4);
    put(app, ebp - 4, cpu.eax);
    const Float firstScaled = fp::mul(first, scale);
    const Float second = fp::mul(scale, loadInteger(app, ebp - 4));
    put(app, ebp - 0x14, cpu.ebx);
    storeFloat(app, cpu.ecx + 8, third);
    storeFloat(app, cpu.ecx, firstScaled);
    storeFloat(app, cpu.ecx + 4, second);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x430350: whether car eax is clear of the wall of the track position edx
 * (a copy of the car's +0xc record, as carAgainstWalls makes it): 1 for no
 * polygon there, a polygon with something of its own (+0x6c), of kind 0 or
 * 14 (0x4207e0), or the car (+0x9c) not below the polygon's corners'
 * highest (its four corners at +0x24 on, or those of the polygon record
 * +0x10 names when +0x14 is 0) less 0.6 -- 2 for +0x164 nonzero; else 0. */
void clearOfWall(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x30;
    const reg32 ebp = cpu.ebp;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ebx = cpu.eax;
    cpu.ecx = cpu.edx;
    cpu.edx = 1;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, 0x4207e0))
        return;
    cpu.edx = at(app, cpu.ecx + 0x18);
    cpu.eax &= 0x3f;
    if (cpu.edx == 0 || (cpu.edx = cpu.ecx + 0x54) == 0)
    {
        cpu.eax = 1;
        return leave();
    }
    cpu.esi = at(app, cpu.ecx + 0x6c);
    if (cpu.esi != 0 || cpu.eax == 0 || cpu.eax == 0xe)
    {
        cpu.eax = 1;
        return leave();
    }
    put(app, ebp - 8, cpu.esi);
    cpu.eax = at(app, cpu.ebx + 0x9c);
    cpu.edx = 0;
    put(app, ebp - 0xc, cpu.eax);
    // The corners' heights: the highest into -8.
    for (;;)
    {
        if (at(app, cpu.ecx + 0x14) == 0)
        {
            safepoint(app, cpu);
            cpu.esi = at(app, cpu.ecx + 0x10);
            cpu.eax = cpu.esi * 8;
            cpu.eax += cpu.esi;
            cpu.esi = at(app, kPolygonRecords);
            cpu.eax <<= 2;
            cpu.eax += cpu.esi;
        }
        else
        {
            cpu.eax = cpu.edx * 4;
            cpu.eax -= cpu.edx;
            cpu.eax <<= 2;
            cpu.eax += cpu.ecx;
            cpu.eax += 0x24;
        }
        safepoint(app, cpu);
        cpu.edi = ebp - 0x30;
        cpu.esi = cpu.eax;
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        cpu.eax = at(app, ebp - 0x2c);
        put(app, ebp - 4, cpu.eax);
        const Float height = fp::mul(loadInteger(app, ebp - 4), loadDouble(app, kWallScale));
        Float highest;
        if (cpu.edx == 0)
            highest = height;
        else
        {
            const Float before = loadFloat(app, ebp - 8);
            storeDouble(app, ebp - 0x14, before);
            storeDouble(app, ebp - 0x24, height);
            if (!compare(cpu, before, loadDouble(app, ebp - 0x24)).belowOrEqual())
            {
                cpu.eax = at(app, ebp - 0x24);
                put(app, ebp - 0x1c, cpu.eax);
                cpu.eax = at(app, ebp - 0x20);
            }
            else
            {
                cpu.eax = at(app, ebp - 0x14);
                put(app, ebp - 0x1c, cpu.eax);
                cpu.eax = at(app, ebp - 0x10);
            }
            put(app, ebp - 0x18, cpu.eax);
            highest = loadDouble(app, ebp - 0x1c);
        }
        storeFloat(app, ebp - 8, highest);
        ++cpu.edx;
        if (sreg32(cpu.edx) >= 4)
            break;
    }
    const reg32 margin = half(app, cpu.ebx + 0x164) != 0 ? kWallMarginRace : kWallMarginOther;
    const Float top = fp::add(loadFloat(app, ebp - 8), loadDouble(app, margin));
    cpu.eax = compare(cpu, loadFloat(app, ebp - 0xc), top).below ? 1 : 0;
    leave();
}

/* 0x4304b0: car eax against the track's walls: its box's corners -- four at
 * its middle's height for +0x164 0, eight else -- into the world
 * (0x4e0430), each moved on by a thirty-second of the car's speed and its
 * turn (0x4a2cc0, 0x4e0280), put into 16.16 and looked up on the track
 * (0x41f6e0); the first corner past a wall (clearOfWall 0) of its polygon or
 * of the neighbours the edges it crossed lead to (0x41fd60, 0x41f870) gives
 * the walls' normals summed (polygonEdgeNormal), or the line back to the
 * car, normalized (0x4972f0); the wall's surface (its polygon's normal,
 * 0x42ee10), the sound's kind into +0x170 (the polygon's byte 0xe, a half by
 * the side of the wall the car is on), the scrape's sound (0x406160) and a
 * jolt (0x4aa4f4) by the car's flags (+0x200). */
void carAgainstWalls(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x23c;
    cpu.ebp -= 0x82;
    const reg32 f = cpu.ebp;
    const auto car = [&] { return at(app, f + 0x7a); };
    const auto leave = [&] {
        cpu.esp = f + 0x82;
        cpu.ebp = pop(app, cpu);
        cpu.edi = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    put(app, f + 0x7a, cpu.eax);
    cpu.edi = f + 0x12;
    cpu.esi = kZeroVector;
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    cpu.edi = f - 0x12;
    cpu.esi = kZeroVector2;
    cpu.ecx = 9;
    cpu.edx = 0;
    movsd(app, cpu);
    movsd(app, cpu);
    movsd(app, cpu);
    put(app, f + 0x5e, cpu.edx);
    cpu.edi = f - 0x6a;
    cpu.esi = cpu.eax + 0xc0;
    repMovsd(app, cpu);
    put(app, f + 0x5a, cpu.edx);
    // A thirty-second of the speed into -0x12, of the turn onto the stack.
    {
        const Float step = loadFloat(app, kSpeedStep);
        storeFloat(app, f - 0x12, fp::mul(loadFloat(app, cpu.eax + 0xa4), step));
        storeFloat(app, f - 0xe, fp::mul(loadFloat(app, cpu.eax + 0xa8), step));
        const Float z = loadFloat(app, cpu.eax + 0xac);
        cpu.eax = f + 0x5a;
        const Float zStep = fp::mul(z, step);
        push(app, cpu, cpu.eax);
        cpu.eax = car();
        storeFloat(app, f - 0xa, zStep);
        const Float turnZ = fp::mul(loadFloat(app, cpu.eax + 0xec), step);
        cpu.esp -= 4;
        storeFloat(app, cpu.esp, turnZ);
        const Float turnY = fp::mul(loadFloat(app, cpu.eax + 0xe8), step);
        cpu.esp -= 4;
        storeFloat(app, cpu.esp, turnY);
        const Float turnX = fp::mul(step, loadFloat(app, cpu.eax + 0xe4));
        cpu.esp -= 4;
        cpu.eax = f - 0x46;
        storeFloat(app, cpu.esp, turnX);
    }
    if (!byAddress(app, cpu, 0x4a2cc0))
        return;
    if (cpu.eax != 0)
    {
        cpu.ebx = f - 0x6a;
        cpu.edx = f - 0x46;
        cpu.eax = f - 0x6a;
        if (!byAddress(app, cpu, 0x4e0280))
            return;
    }
    // The box's corners in the car's frame, at -0xca.
    cpu.eax = car();
    if (half(app, cpu.eax + 0x164) == 0)
    {
        const Float x = loadFloat(app, cpu.eax + 0x114);
        cpu.esi = 0;
        put(app, f - 0xc6, cpu.esi);
        storeFloat(app, f - 0xca, -x);
        cpu.eax = at(app, cpu.eax + 0x11c);
        put(app, f - 0xc2, cpu.eax);
        cpu.eax = car();
        cpu.eax = at(app, cpu.eax + 0x114);
        put(app, f - 0xbe, cpu.eax);
        cpu.eax = car();
        put(app, f - 0xba, cpu.esi);
        cpu.eax = at(app, cpu.eax + 0x11c);
        put(app, f - 0xb6, cpu.eax);
        cpu.eax = car();
        cpu.eax = at(app, cpu.eax + 0x114);
        cpu.ebx = 4;
        put(app, f - 0xb2, cpu.eax);
        cpu.eax = car();
        put(app, f - 0xae, cpu.esi);
        put(app, f + 0x6a, cpu.ebx);
        storeFloat(app, f - 0xaa, -loadFloat(app, cpu.eax + 0x11c));
        const Float x2 = loadFloat(app, cpu.eax + 0x114);
        put(app, f - 0xa2, cpu.esi);
        storeFloat(app, f - 0xa6, -x2);
        storeFloat(app, f - 0x9e, -loadFloat(app, cpu.eax + 0x11c));
    }
    else
    {
        storeFloat(app, f - 0xca, -loadFloat(app, cpu.eax + 0x114));
        cpu.eax = at(app, cpu.eax + 0x118);
        put(app, f - 0xc6, cpu.eax);
        static const reg32 copied[][2] = {{0x11c, 0xc2}, {0x114, 0xbe}, {0x118, 0xba}, {0x11c, 0xb6},
                                          {0x114, 0xb2}, {0x118, 0xae}};
        for (const auto& c : copied)
        {
            cpu.eax = car();
            cpu.eax = at(app, cpu.eax + c[0]);
            put(app, f - c[1], cpu.eax);
        }
        cpu.eax = car();
        storeFloat(app, f - 0xaa, -loadFloat(app, cpu.eax + 0x11c));
        storeFloat(app, f - 0xa6, -loadFloat(app, cpu.eax + 0x114));
        cpu.eax = at(app, cpu.eax + 0x118);
        put(app, f - 0xa2, cpu.eax);
        cpu.eax = car();
        storeFloat(app, f - 0x9e, -loadFloat(app, cpu.eax + 0x11c));
        storeFloat(app, f - 0x9a, -loadFloat(app, cpu.eax + 0x114));
        storeFloat(app, f - 0x96, -loadFloat(app, cpu.eax + 0x118));
        cpu.eax = at(app, cpu.eax + 0x11c);
        put(app, f - 0x92, cpu.eax);
        cpu.eax = car();
        cpu.eax = at(app, cpu.eax + 0x114);
        put(app, f - 0x8e, cpu.eax);
        cpu.eax = car();
        storeFloat(app, f - 0x8a, -loadFloat(app, cpu.eax + 0x118));
        cpu.eax = at(app, cpu.eax + 0x11c);
        put(app, f - 0x86, cpu.eax);
        cpu.eax = car();
        cpu.eax = at(app, cpu.eax + 0x114);
        cpu.ecx = 8;
        put(app, f - 0x82, cpu.eax);
        cpu.eax = car();
        put(app, f + 0x6a, cpu.ecx);
        storeFloat(app, f - 0x7e, -loadFloat(app, cpu.eax + 0x118));
        storeFloat(app, f - 0x7a, -loadFloat(app, cpu.eax + 0x11c));
        storeFloat(app, f - 0x76, -loadFloat(app, cpu.eax + 0x114));
        storeFloat(app, f - 0x72, -loadFloat(app, cpu.eax + 0x118));
        storeFloat(app, f - 0x6e, -loadFloat(app, cpu.eax + 0x11c));
    }
    // Into the world, by the car's axes and from its position.
    cpu.eax = f - 0xca;
    cpu.ecx = car();
    cpu.ebx = f - 0x6a;
    cpu.edx = f - 0xca;
    push(app, cpu, cpu.eax);
    cpu.ecx += 0x98;
    cpu.eax = at(app, f + 0x6a);
    if (!byAddress(app, cpu, 0x4e0430))
        return;
    cpu.ebx = 0;
    put(app, f + 0x72, cpu.ebx);
    for (;; put(app, f + 0x72, at(app, f + 0x72) + 1))
    {
        safepoint(app, cpu);
        cpu.eax = at(app, f + 0x72);
        if (sreg32(cpu.eax) >= sreg32(at(app, f + 0x6a)))
            return leave();
        // The corner moved on by the speed's step into +0x42, in 16.16 at
        // +0x1e, looked up from the car's track position (copied to -0x1ba).
        cpu.eax = car();
        cpu.edx = at(app, f + 0x72);
        cpu.esi = cpu.eax + 0xc;
        cpu.eax = cpu.edx * 4;
        cpu.eax -= cpu.edx;
        cpu.ecx = 0x1e;
        cpu.edx = cpu.eax * 4;
        cpu.edi = f - 0x1ba;
        cpu.ebx = f - 0x12;
        cpu.eax = at(app, cpu.edx + f - 0xca);
        repMovsd(app, cpu);
        put(app, f + 0x42, cpu.eax);
        cpu.eax = at(app, cpu.edx + f - 0xc6);
        cpu.ecx = f + 0x42;
        put(app, f + 0x46, cpu.eax);
        cpu.eax = at(app, cpu.edx + f - 0xc2);
        cpu.edx = f + 0x42;
        put(app, f + 0x4a, cpu.eax);
        cpu.eax = 1;
        if (!byAddress(app, cpu, kAddVectors))
            return;
        const Float toFixed = loadDouble(app, kToFixed);
        cpu.esp -= 4;
        put(app, cpu.esp, reg32(fp::toInteger<sreg32>(fp::mul(loadFloat(app, f + 0x42), toFixed))));
        cpu.eax = pop(app, cpu);
        const Float y = fp::mul(loadFloat(app, f + 0x46), toFixed);
        put(app, f + 0x1e, cpu.eax);
        cpu.esp -= 4;
        put(app, cpu.esp, reg32(fp::toInteger<sreg32>(y)));
        cpu.eax = pop(app, cpu);
        const Float z = fp::mul(loadFloat(app, f + 0x4a), toFixed);
        cpu.edi = 0;
        cpu.esi = f + 0x1e;
        put(app, f + 0x76, cpu.edi);
        cpu.edi = f - 0x1b6;
        cpu.ebx = 1;
        cpu.edx = f - 0x1ba;
        put(app, f + 0x22, cpu.eax);
        cpu.esp -= 4;
        put(app, cpu.esp, reg32(fp::toInteger<sreg32>(z)));
        cpu.eax = pop(app, cpu);
        put(app, f + 0x26, cpu.eax);
        cpu.eax = f + 0x1e;
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        if (!byAddress(app, cpu, 0x41f6e0))
            return;
        cpu.edx = f - 0x1ba;
        cpu.eax = car();
        if (!direct(app, cpu, clearOfWall))
            return;
        if (cpu.eax != 0)
        {
            // Clear of its own polygon's walls: the edges it crossed (+0x5e)
            // lead to neighbours, looked up and tested in turn.
            cpu.ecx = 0x1e;
            cpu.edi = f - 0x142;
            cpu.esi = f - 0x1ba;
            cpu.edx = car();
            cpu.ebx = f - 0x1ba;
            cpu.eax = f + 0x1e;
            repMovsd(app, cpu);
            cpu.edx += 0xc;
            cpu.ecx = f + 0x5e;
            cpu.edi = f - 0x22;
            cpu.esi = kEdgeBits;
            if (!byAddress(app, cpu, 0x41fd60))
                return;
            put(app, f + 0x62, cpu.eax);
            cpu.edx = 0;
            movsd(app, cpu);
            movsd(app, cpu);
            movsd(app, cpu);
            movsd(app, cpu);
            put(app, f + 0x1a, cpu.edx);
            put(app, f + 0x16, cpu.edx);
            put(app, f + 0x12, cpu.edx);
            if (cpu.eax != 0)
            {
                put(app, f + 0x6e, cpu.edx);
                for (cpu.ecx = 0;; ++cpu.ecx)
                {
                    if (cpu.ecx != 0)
                    {
                        safepoint(app, cpu);
                        if (sreg32(cpu.ecx) >= 4)
                            break;
                    }
                    cpu.eax = cpu.ecx * 4;
                    cpu.edx = at(app, f + 0x62);
                    cpu.edi = at(app, cpu.eax + f - 0x22);
                    if ((cpu.edx & cpu.edi) == 0)
                        continue;
                    cpu.edx = car();
                    cpu.ebx = f - 0x142;
                    cpu.eax = cpu.edi;
                    cpu.edx += 0xc;
                    if (!byAddress(app, cpu, 0x41f870))
                        return;
                    if (cpu.eax != 0)
                    {
                        cpu.edx = f - 0x142;
                        cpu.eax = car();
                        if (!direct(app, cpu, clearOfWall))
                            return;
                        if (cpu.eax == 0)
                            continue;
                    }
                    cpu.ebx = car();
                    cpu.edx = f + 0x4e;
                    cpu.eax = at(app, f + cpu.ecx * 4 - 0x22);
                    cpu.ebx += 0xc;
                    if (!direct(app, cpu, polygonEdgeNormal))
                        return;
                    // The count (+0x6e) and the sum (+0x12) of the normals.
                    const Float x = loadFloat(app, f + 0x12);
                    const Float y = loadFloat(app, f + 0x16);
                    const Float z = loadFloat(app, f + 0x1a);
                    cpu.edx = at(app, f + 0x76);
                    cpu.eax = at(app, f + cpu.ecx * 4 - 0x22);
                    const Float count = fp::add(1.0, loadFloat(app, f + 0x6e));
                    const Float sumX = fp::add(x, loadFloat(app, f + 0x4e));
                    const Float sumY = fp::add(y, loadFloat(app, f + 0x52));
                    const Float sumZ = fp::add(z, loadFloat(app, f + 0x56));
                    cpu.edx |= cpu.eax;
                    storeFloat(app, f + 0x6e, count);
                    storeFloat(app, f + 0x12, sumX);
                    storeFloat(app, f + 0x16, sumY);
                    storeFloat(app, f + 0x1a, sumZ);
                    put(app, f + 0x76, cpu.edx);
                }
                if ((at(app, f + 0x6e) & 0x7fffffff) != 0)
                {
                    // Their mean, flat (y 0).
                    const Float x = loadFloat(app, f + 0x12);
                    const Float reciprocal = fp::div(1.0, loadFloat(app, f + 0x6e));
                    const Float meanX = fp::mul(x, reciprocal);
                    const Float meanY = fp::mul(loadFloat(app, f + 0x16), reciprocal);
                    const Float meanZ = fp::mul(reciprocal, loadFloat(app, f + 0x1a));
                    cpu.ebx = 0;
                    storeFloat(app, f + 0x12, meanX);
                    storeFloat(app, f + 0x16, meanY);
                    storeFloat(app, f + 0x1a, meanZ);
                    put(app, f + 0x16, cpu.ebx);
                }
                else
                {
                    // No wall's normal: back towards the car.
                    cpu.eax = car();
                    storeFloat(app, f + 0x12, fp::sub(loadFloat(app, cpu.eax + 0x98), loadFloat(app, f + 0x42)));
                    storeFloat(app, f + 0x16, fp::sub(loadFloat(app, cpu.eax + 0x9c), loadFloat(app, f + 0x46)));
                    const Float z = fp::sub(loadFloat(app, cpu.eax + 0xa0), loadFloat(app, f + 0x4a));
                    cpu.eax = at(app, f + 0x62);
                    storeFloat(app, f + 0x1a, z);
                    put(app, f + 0x76, cpu.eax);
                }
            }
        }
        if (at(app, f + 0x76) == 0)
            continue;
        cpu.eax = f + 0x12;
        if (!byAddress(app, cpu, kNormalize))
            return;
        const bool none = compare(cpu, 0.0, cpu.fpu.st(0)).equal;
        cpu.fpu.count -= 1;
        if (none)
            return leave();
        // The corner at -6, the wall's side by the edges' bits (+0x76): its
        // polygon's corner, scaled, for the surface.
        cpu.edx = at(app, f + 0x72);
        cpu.eax = cpu.edx * 4;
        cpu.eax -= cpu.edx;
        cpu.edi = f - 6;
        cpu.esi = f + cpu.eax * 4 - 0xca;
        cpu.ah = byte(app, f + 0x76);
        movsd(app, cpu);
        movsd(app, cpu);
        movsd(app, cpu);
        reg32 corner;
        if (cpu.ah & 4)
            corner = (cpu.ah & 1) ? 0x3c : 0x30;
        else if (cpu.ah & 8)
            corner = (cpu.ah & 1) ? 0x48 : 0x54;
        else
            corner = (cpu.ah & 1) ? 0x3c : 0x30;
        cpu.eax = car();
        {
            const Float scale = loadDouble(app, kPolygonScale);
            storeFloat(app, f - 6, fp::mul(loadInteger(app, cpu.eax + corner), scale));
            storeFloat(app, f - 2, fp::mul(loadInteger(app, cpu.eax + corner + 4), scale));
            storeFloat(app, f + 2, fp::mul(scale, loadInteger(app, cpu.eax + corner + 8)));
        }
        cpu.eax = at(app, f + 0x72);
        cpu.edx = cpu.eax * 4;
        cpu.ecx = f - 6;
        cpu.edx -= cpu.eax;
        cpu.eax = f - 0xca;
        cpu.edx <<= 2;
        cpu.ebx = f + 0x12;
        cpu.edx += cpu.eax;
        cpu.eax = car();
        if (!byAddress(app, cpu, 0x42ee10))
            return;
        if (cpu.eax == 0)
            continue;
        // The car's polygon's centre and normal (its record's bytes 0x15 to
        // 0x17, signed), against the line to the car.
        cpu.eax = car();
        cpu.edx = at(app, cpu.eax + 0x1c);
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.edx = at(app, kPolygonRecords);
        {
            const Float scale = loadDouble(app, kPolygonScale);
            const Float x = fp::mul(loadInteger(app, cpu.edx + cpu.eax * 4), scale);
            const Float y = fp::mul(loadInteger(app, cpu.edx + cpu.eax * 4 + 4), scale);
            const Float zFixed = loadInteger(app, cpu.edx + cpu.eax * 4 + 8);
            cpu.eax = car();
            const Float z = fp::mul(zFixed, scale);
            const Float carX = loadFloat(app, cpu.eax + 0x98);
            storeFloat(app, f + 0x2a, x);
            const Float toX = fp::sub(carX, loadFloat(app, f + 0x2a));
            storeFloat(app, f + 0x2e, y);
            storeFloat(app, f + 0x36, toX);
            const Float toY = fp::sub(loadFloat(app, cpu.eax + 0x9c), loadFloat(app, f + 0x2e));
            storeFloat(app, f + 0x32, z);
            storeFloat(app, f + 0x3a, toY);
            storeFloat(app, f + 0x3e, fp::sub(loadFloat(app, cpu.eax + 0xa0), loadFloat(app, f + 0x32)));
            for (reg32 i = 0; i < 3; ++i)
            {
                cpu.ecx = at(app, cpu.eax + 0x1c);
                cpu.eax = cpu.ecx * 8;
                cpu.eax += cpu.ecx;
                cpu.eax = at(app, cpu.edx + cpu.eax * 4 + 0x15 + i);
                cpu.eax = reg32(sreg32(cpu.eax) >> 0x18);
                cpu.eax <<= 9;
                put(app, f + 0x7e, cpu.eax);
                if (i < 2)
                {
                    const Float n = fp::mul(loadInteger(app, f + 0x7e), scale);
                    cpu.eax = car();
                    storeFloat(app, f + 6 + 4 * i, n);
                }
                else
                {
                    const Float n = fp::mul(scale, loadInteger(app, f + 0x7e));
                    cpu.edx = f + 0x36;
                    cpu.eax = f + 6;
                    storeFloat(app, f + 0xe, n);
                }
            }
        }
        if (!byAddress(app, cpu, kDot))
            return;
        storeFloat(app, f + 0x66, cpu.fpu.st(0));
        const Compared side = compare(cpu, 0.0, loadFloat(app, f + 0x66));
        cpu.fpu.count -= 1;
        cpu.eax = car();
        cpu.edx = at(app, cpu.eax + 0x1c);
        cpu.eax = cpu.edx * 8;
        cpu.eax += cpu.edx;
        cpu.edx = at(app, kPolygonRecords);
        // (ja then jbe: the code's mov eax, 4 after them is never reached.)
        if (!side.belowOrEqual())
        {
            cpu.al = byte(app, cpu.edx + cpu.eax * 4 + 0xe);
            cpu.eax &= 0xff;
            cpu.eax = reg32(sreg32(cpu.eax) >> 4);
            cpu.eax &= 0xf;
        }
        else
        {
            cpu.al = byte(app, cpu.edx + cpu.eax * 4 + 0xe);
            cpu.al &= 0xf;
            cpu.eax &= 0xff;
        }
        cpu.edx = car();
        cpu.eax |= 0x40000;
        cpu.cl = byte(app, cpu.edx + 0x200);
        put(app, cpu.edx + 0x170, cpu.eax);
        if (cpu.cl & 2)
        {
            cpu.edx = f + 0x12;
            cpu.eax = car();
            if (!byAddress(app, cpu, 0x406160))
                return;
        }
        cpu.eax = car();
        if ((byte(app, cpu.eax + 0x200) & 4) && !byAddress(app, cpu, 0x4aa4f4))
            return;
        return leave();
    }
}

/* 0x430c80: car eax and car edx collided: taken apart (separateCars) up to
 * 33 times while they still touch, and if they touched at all their
 * response worked out at the contact's point (0x42f280). */
void collideCars(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.esi = cpu.edx;
    cpu.ebx = 0x20;
    cpu.edi = 0;
    for (;;)
    {
        safepoint(app, cpu);
        cpu.edx = cpu.esi;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, separateCars))
            return;
        cpu.edx = cpu.eax;
        if (cpu.eax != 0)
            cpu.edi = 1;
        if (cpu.edx == 0)
            break;
        --cpu.ebx;
        if (cpu.ebx == reg32(-1))
            break;
    }
    if (cpu.edi != 0)
    {
        cpu.ebx = kContactPoint;
        cpu.edx = cpu.esi;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kCarsRespond))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x430cd0: no car tested yet this frame (kCarCount 0). */
void clearCarList(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 0;
    put(app, kCarCount, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x430ce0: whether the point at edx (x and z; y taken as the car's) is
 * within the reach on the stack (a float) of car eax's box, along its
 * forward and right axes: the point less the car's position into the
 * vector at the stack's second word, then 1, or 0; ret 8. */
void objectInReach(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    const reg32 ebp = cpu.ebp;
    const auto leave = [&] {
        cpu.esp = cpu.ebp;
        cpu.ebp = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4 + 8;
    };
    cpu.ebx = at(app, ebp + 0x14);
    cpu.ecx = cpu.eax;
    const Float x = fp::sub(loadFloat(app, cpu.edx), loadFloat(app, cpu.eax + 0x98));
    cpu.eax += 0xc0;
    storeFloat(app, cpu.ebx, x);
    const Float z = fp::sub(loadFloat(app, cpu.edx + 8), loadFloat(app, cpu.eax - 0x20));
    cpu.edx = cpu.ebx;
    storeFloat(app, cpu.ebx + 8, z);
    put(app, cpu.ebx + 4, 0);
    // |the offset . each axis| against the box's half size and the reach.
    static const reg32 axes[][4] = {{0, 0x114, 8, 0x10}, {0xd8, 0x11c, 4, 0xc}};
    for (int i = 0; i < 2; ++i)
    {
        if (i == 1)
        {
            cpu.eax = cpu.ecx + 0xd8;
            cpu.edx = cpu.ebx;
        }
        if (!byAddress(app, cpu, kDot))
            return;
        const reg32 along = ebp - axes[i][2], positive = ebp - axes[i][3];
        storeFloat(app, along, cpu.fpu.st(0));
        const bool above = compare(cpu, 0.0, loadFloat(app, along)).below;
        cpu.fpu.count -= 1;
        if (above)
        {
            cpu.eax = at(app, along);
            put(app, positive, cpu.eax);
        }
        else
            storeFloat(app, positive, -loadFloat(app, along));
        const Float reach = fp::add(loadFloat(app, cpu.ecx + axes[i][1]), loadFloat(app, ebp + 0x10));
        if (compare(cpu, reach, loadFloat(app, positive)).below)
        {
            cpu.eax = 0;
            return leave();
        }
    }
    cpu.eax = 1;
    leave();
}

/* 0x430da0: car eax against the track's objects near it (0x41ff10 their
 * count, 0x41ff20 each: its position in 16.16 and its size): those close
 * by a rough distance (the larger of the x and z differences and a quarter
 * of the smaller), twice as big as the car's +0x15c and within its box's
 * reach (objectInReach) are hit -- 0x41d350 says how (1 pushed back, 2 a
 * small one knocked, below 0 something else), the jolt (0x471440); for 1
 * the car pushed out (0x4e0050, 0x4dffb0), its response (0x42efe0) and the
 * sound's (+0x168 to +0x180), the bump's sound (0x406120); for 2 above a
 * speed of 2 and below 0 the sound's. */
void carAgainstObjects(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x6c;
    const reg32 ebp = cpu.ebp;
    const auto car = [&] { return at(app, ebp - 0x10); };
    put(app, ebp - 0x10, cpu.eax);
    cpu.eax += 0xc;
    if (!byAddress(app, cpu, 0x41ff10))
        return;
    cpu.edx = cpu.eax;
    cpu.edx ^= cpu.eax;
    put(app, ebp - 0xc, cpu.eax);
    put(app, ebp - 0x14, cpu.edx);
    for (;; put(app, ebp - 0x14, at(app, ebp - 0x14) + 1))
    {
        safepoint(app, cpu);
        cpu.eax = at(app, ebp - 0x14);
        if (sreg32(cpu.eax) >= sreg32(at(app, ebp - 0xc)))
            break;
        cpu.eax = car();
        cpu.ecx = ebp - 4;
        cpu.ebx = ebp - 0x48;
        cpu.edx = at(app, ebp - 0x14);
        cpu.eax += 0xc;
        if (!byAddress(app, cpu, 0x41ff20))
            return;
        // Its size (the word at -4, times 0x100) and position, as floats.
        cpu.eax = at(app, ebp - 6);
        cpu.eax = reg32(sreg32(cpu.eax) >> 0x10);
        cpu.eax <<= 8;
        put(app, ebp - 8, cpu.eax);
        cpu.eax = at(app, ebp - 0x48);
        const Float sizeFixed = loadInteger(app, ebp - 8);
        put(app, ebp - 8, cpu.eax);
        const Float scale = loadDouble(app, kObjectScale);
        const Float size = fp::mul(sizeFixed, scale);
        const Float xFixed = loadInteger(app, ebp - 8);
        cpu.eax = at(app, ebp - 0x44);
        const Float x = fp::mul(xFixed, scale);
        put(app, ebp - 8, cpu.eax);
        cpu.eax = at(app, ebp - 0x40);
        const Float yFixed = loadInteger(app, ebp - 8);
        put(app, ebp - 8, cpu.eax);
        const Float y = fp::mul(yFixed, scale);
        const Float z = fp::mul(scale, loadInteger(app, ebp - 8));
        const Float zero = 0.0;
        storeFloat(app, ebp - 0x3c, x);
        storeFloat(app, ebp - 0x38, y);
        cpu.eax = car();
        storeFloat(app, ebp - 0x34, z);
        const Float dx = fp::sub(x, loadFloat(app, cpu.eax + 0x98));
        storeFloat(app, ebp - 0x1c, size);
        storeFloat(app, ebp - 0x28, dx);
        if (compare(cpu, zero, loadFloat(app, ebp - 0x28)).below)
        {
            cpu.eax = at(app, ebp - 0x28);
            put(app, ebp - 0x30, cpu.eax);
        }
        else
            storeFloat(app, ebp - 0x30, -loadFloat(app, ebp - 0x28));
        cpu.eax = at(app, ebp - 0x30);
        put(app, ebp - 0x20, cpu.eax);
        cpu.eax = car();
        const Float dz = fp::sub(loadFloat(app, ebp - 0x34), loadFloat(app, cpu.eax + 0xa0));
        storeFloat(app, ebp - 0x24, dz);
        if (compare(cpu, 0.0, loadFloat(app, ebp - 0x24)).below)
        {
            cpu.eax = at(app, ebp - 0x24);
            put(app, ebp - 0x18, cpu.eax);
        }
        else
            storeFloat(app, ebp - 0x18, -loadFloat(app, ebp - 0x24));
        // The rough distance: the larger and a quarter of the smaller.
        const bool xLarger = !compare(cpu, loadFloat(app, ebp - 0x20), loadFloat(app, ebp - 0x18)).belowOrEqual();
        const reg32 smaller = xLarger ? ebp - 0x18 : ebp - 0x20, larger = xLarger ? ebp - 0x20 : ebp - 0x18;
        storeFloat(app, ebp - 0x2c, fp::add(fp::mul(loadFloat(app, smaller), loadFloat(app, kObjectQuarter)),
                                                loadFloat(app, larger)));
        cpu.eax = car();
        const Float near = fp::add(loadFloat(app, ebp - 0x1c), loadFloat(app, cpu.eax + 0x120));
        if (compare(cpu, near, loadFloat(app, ebp - 0x2c)).belowOrEqual())
            continue;
        const Float twice = fp::mul(loadFloat(app, ebp - 0x1c), loadFloat(app, kObjectTwo));
        cpu.eax = car();
        if (compare(cpu, twice, loadFloat(app, cpu.eax + 0x15c)).belowOrEqual())
            continue;
        cpu.eax = ebp - 0x54;
        push(app, cpu, cpu.eax);
        cpu.edx = ebp - 0x3c;
        cpu.eax = car();
        push(app, cpu, at(app, ebp - 0x1c));
        if (!direct(app, cpu, objectInReach))
            return;
        if (cpu.eax == 0)
            continue;
        cpu.edx = at(app, ebp - 0x14);
        cpu.eax = car();
        if (!byAddress(app, cpu, 0x41d350))
            return;
        // The jolt, by the offset's x and z in 16.16.
        const Float toFixed = loadDouble(app, kObjectToFixed);
        const Float joltX = fp::mul(loadFloat(app, ebp - 0x4c), toFixed);
        cpu.esi = cpu.eax;
        cpu.esp -= 4;
        put(app, cpu.esp, reg32(fp::toInteger<sreg32>(joltX)));
        cpu.eax = pop(app, cpu);
        const Float joltZ = fp::mul(loadFloat(app, ebp - 0x54), toFixed);
        cpu.edx = cpu.esi;
        cpu.ecx = cpu.eax;
        cpu.esp -= 4;
        put(app, cpu.esp, reg32(fp::toInteger<sreg32>(joltZ)));
        cpu.eax = pop(app, cpu);
        cpu.ebx = cpu.eax;
        cpu.eax = car();
        if (!byAddress(app, cpu, 0x471440))
            return;
        if (cpu.esi == 1)
        {
            // Pushed out of it, by its size, along the line from it.
            cpu.ebx = 0;
            cpu.eax = car();
            put(app, ebp - 0x68, cpu.ebx);
            storeFloat(app, ebp - 0x6c, fp::sub(loadFloat(app, cpu.eax + 0x98), loadFloat(app, ebp - 0x3c)));
            const Float awayZ = fp::sub(loadFloat(app, cpu.eax + 0xa0), loadFloat(app, ebp - 0x34));
            cpu.eax = ebp - 0x6c;
            storeFloat(app, ebp - 0x64, awayZ);
            if (!byAddress(app, cpu, kNormalize))
                return;
            cpu.eax = ebp - 0x60;
            cpu.edx = ebp - 0x6c;
            push(app, cpu, cpu.eax);
            cpu.ecx = ebp - 0x60;
            cpu.ebx = ebp - 0x60;
            push(app, cpu, at(app, ebp - 0x1c));
            cpu.eax = cpu.esi;
            cpu.fpu.count -= 1;  // fstp st(0): the length
            if (!byAddress(app, cpu, kScaleVectors))
                return;
            cpu.edx = ebp - 0x3c;
            cpu.eax = cpu.esi;
            if (!byAddress(app, cpu, kAddVectors))
                return;
            cpu.ebx = ebp - 0x60;
            cpu.edx = ebp - 0x6c;
            cpu.eax = car();
            push(app, cpu, 0);
            if (!byAddress(app, cpu, 0x42efe0))
                return;
            cpu.eax = car();
            if ((at(app, cpu.eax + 0x168) & 0x7fffffff) != 0)
            {
                const Float loudness = loadFloat(app, cpu.eax + 0x168);
                put(app, cpu.eax + 0x170, 0x6000a);
                const Float halved = fp::mul(loudness, loadFloat(app, kObjectHalf));
                const Float px = loadFloat(app, cpu.eax + 0x98);
                const Float py = loadFloat(app, cpu.eax + 0x9c);
                const Float pz = loadFloat(app, cpu.eax + 0xa0);
                storeFloat(app, cpu.eax + 0x178, px);
                storeFloat(app, cpu.eax + 0x17c, py);
                storeFloat(app, cpu.eax + 0x180, pz);
                storeFloat(app, cpu.eax + 0x168, halved);
            }
            cpu.eax = car();
            if ((byte(app, cpu.eax + 0x200) & 2) && !byAddress(app, cpu, 0x406120))
                return;
        }
        else if (cpu.esi == 2)
        {
            cpu.eax = car();
            if (compare(cpu, loadFloat(app, cpu.eax + 0xb8), loadDouble(app, kObjectSpeed)).belowOrEqual())
                continue;
            cpu.eax = car();
            put(app, cpu.eax + 0x170, 0x6000b);
            put(app, cpu.eax + 0x16c, 0);
            cpu.edi = cpu.eax + 0x178;
            cpu.esi = cpu.eax + 0x98;
            put(app, cpu.eax + 0x168, 0x41700000);
            movsd(app, cpu);
            movsd(app, cpu);
            movsd(app, cpu);
        }
        else if (sreg32(cpu.esi) < 0)
        {
            cpu.eax = car();
            put(app, cpu.eax + 0x168, 0x41700000);
            put(app, cpu.eax + 0x170, 0x6000a);
            put(app, cpu.eax + 0x16c, 0);
            const Float px = loadFloat(app, cpu.eax + 0x98);
            const Float py = loadFloat(app, cpu.eax + 0x9c);
            const Float pz = loadFloat(app, cpu.eax + 0xa0);
            storeFloat(app, cpu.eax + 0x178, px);
            storeFloat(app, cpu.eax + 0x17c, py);
            storeFloat(app, cpu.eax + 0x180, pz);
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

/* 0x4310c0: car eax added to this frame's list (kCarList, kCarCount), and
 * tested against those before it: by their radii (+0x120) along each of
 * y, x and z, collideCars for those that are near. */
void addCarToList(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x1c;
    const reg32 ebp = cpu.ebp;
    cpu.ecx = cpu.eax;
    cpu.eax = at(app, kCarCount);
    cpu.ebx = 0;
    put(app, cpu.eax * 4 + kCarList, cpu.ecx);
    for (;; ++cpu.ebx)
    {
        safepoint(app, cpu);
        cpu.edx = at(app, kCarCount);
        if (sreg32(cpu.ebx) >= sreg32(cpu.edx))
            break;
        const Float zero = 0.0;
        const Float radius = loadFloat(app, cpu.ecx + 0x120);
        cpu.edx = at(app, cpu.ebx * 4 + kCarList);
        const Float y = loadFloat(app, cpu.ecx + 0xa0);
        const Float radii = fp::add(radius, loadFloat(app, cpu.edx + 0x120));
        const Float dy = fp::sub(y, loadFloat(app, cpu.edx + 0xa0));
        storeFloat(app, ebp - 0x10, radii);
        storeFloat(app, ebp - 4, dy);
        if (compare(cpu, zero, loadFloat(app, ebp - 4)).below)
        {
            cpu.eax = at(app, ebp - 4);
            put(app, ebp - 0x18, cpu.eax);
        }
        else
            storeFloat(app, ebp - 0x18, -loadFloat(app, ebp - 4));
        if (!compare(cpu, loadFloat(app, ebp - 0x18), loadFloat(app, ebp - 0x10)).below)
            continue;
        static const reg32 others[][3] = {{0x98, 0x1c, 0xc}, {0x9c, 0x14, 8}};
        bool near = true;
        for (const auto& o : others)
        {
            const Float d = fp::sub(loadFloat(app, cpu.ecx + o[0]), loadFloat(app, cpu.edx + o[0]));
            storeFloat(app, ebp - o[1], d);
            if (compare(cpu, 0.0, loadFloat(app, ebp - o[1])).below)
            {
                cpu.eax = at(app, ebp - o[1]);
                put(app, ebp - o[2], cpu.eax);
            }
            else
                storeFloat(app, ebp - o[2], -loadFloat(app, ebp - o[1]));
            if (!compare(cpu, loadFloat(app, ebp - o[2]), loadFloat(app, ebp - 0x10)).below)
            {
                near = false;
                break;
            }
        }
        if (!near)
            continue;
        cpu.eax = cpu.ecx;
        if (!direct(app, cpu, collideCars))
            return;
    }
    cpu.ecx = cpu.edx + 1;
    put(app, kCarCount, cpu.ecx);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4311d0 (a car's +0x618): a car's collisions for the frame -- its
 * countdown (+0x174) on; for a car in the race (+0x88 0, +0x89 nonzero):
 * the other cars (addCarToList, once the countdown is out), the track's
 * objects (by +0x188's bit 0x40, likewise) and walls (bit 0x80), then
 * 0x4a0140. */
void carCollisions(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.ebx = at(app, cpu.eax + 0x174);
    cpu.ecx = 1;
    if (sreg32(cpu.ebx) > 0)
    {
        cpu.esi = cpu.ebx;
        cpu.esi -= cpu.ecx;
        put(app, cpu.eax + 0x174, cpu.esi);
    }
    if (byte(app, cpu.edx + 0x88) == 0 && byte(app, cpu.edx + 0x89) != 0)
    {
        if (at(app, cpu.edx + 0x174) != 0)
            cpu.ecx = 0;
        else if (cpu.ecx != 0)
        {
            cpu.eax = cpu.edx;
            if (!direct(app, cpu, addCarToList))
                return;
        }
        if ((byte(app, cpu.edx + 0x188) & 0x40) && cpu.ecx != 0)
        {
            cpu.eax = cpu.edx;
            if (!direct(app, cpu, carAgainstObjects))
                return;
        }
        if (byte(app, cpu.edx + 0x188) & 0x80)
        {
            cpu.eax = cpu.edx;
            if (!direct(app, cpu, carAgainstWalls))
                return;
        }
        cpu.eax = cpu.edx;
        if (!byAddress(app, cpu, 0x4a0140))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x431250: the square of the distance between the points (eax, edx, ebx)
 * and (ecx, and the two on the stack), in integers as they wrap; ret 8. */
void distanceSquared(App* app, CPU& cpu)
{
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax -= cpu.ecx;
    cpu.eax *= cpu.eax;
    cpu.edx -= at(app, cpu.ebp + 0xc);
    cpu.edx *= cpu.edx;
    cpu.esi = at(app, cpu.ebp + 0x10);
    cpu.ecx = cpu.ebx;
    cpu.ecx -= cpu.esi;
    cpu.edx += cpu.eax;
    cpu.eax = cpu.ecx;
    cpu.eax *= cpu.ecx;
    cpu.eax += cpu.edx;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.esp += 4 + 8;
}

/* 0x431280: the value of the point nearest (eax, edx, ebx) among the 0x167
 * points at 0x5537c4 (x, y, z and the value, 0x10 bytes each); the first of
 * those as near as another, and the search over at a distance of 0. */
void nearestPoint(App* app, CPU& cpu)
{
    constexpr reg32 kPoints = 0x5537c4;
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x10;
    const reg32 ebp = cpu.ebp;
    cpu.edi = cpu.eax;
    put(app, ebp - 8, cpu.edx);
    put(app, ebp - 0x10, cpu.ebx);
    cpu.ecx = at(app, kPoints + 8);
    cpu.edx = 0;
    cpu.ebx = at(app, kPoints + 4);
    put(app, ebp - 0xc, cpu.edx);
    push(app, cpu, cpu.ecx);
    cpu.edx = at(app, ebp - 8);
    cpu.ecx = at(app, kPoints);
    push(app, cpu, cpu.ebx);
    cpu.ebx = at(app, ebp - 0x10);
    cpu.esi = 1;
    if (!direct(app, cpu, distanceSquared))
        return;
    put(app, ebp - 4, cpu.eax);
    for (;; ++cpu.esi)
    {
        if (cpu.esi != 1)
        {
            safepoint(app, cpu);
            if (sreg32(cpu.esi) >= 0x167)
                break;
        }
        cpu.eax = cpu.esi << 4;
        cpu.edx = at(app, cpu.eax + kPoints + 8);
        cpu.ebx = at(app, ebp - 0x10);
        push(app, cpu, cpu.edx);
        cpu.ecx = at(app, cpu.eax + kPoints + 4);
        cpu.edx = at(app, ebp - 8);
        push(app, cpu, cpu.ecx);
        cpu.ecx = at(app, cpu.eax + kPoints);
        cpu.eax = cpu.edi;
        if (!direct(app, cpu, distanceSquared))
            return;
        if (sreg32(cpu.eax) < sreg32(at(app, ebp - 4)))
        {
            put(app, ebp - 0xc, cpu.esi);
            put(app, ebp - 4, cpu.eax);
            if (cpu.eax == 0)
                cpu.esi = 0x167;
        }
    }
    cpu.eax = at(app, ebp - 0xc);
    cpu.eax <<= 4;
    cpu.eax = at(app, cpu.eax + kPoints + 0xc);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

// The track's COLL file, its objects: its handle, its 16 bytes' header
// ('COLL', its version 11, its size, its chunks' count).
constexpr reg32 kCollHandle = 0x5eff18, kCollHeader = 0x5eff08;
constexpr reg32 kCollVersion = 0x5eff0c, kCollSize = 0x5eff10, kCollCount = 0x5eff14;
constexpr reg32 kCollMagic = 0x53746c;

/* 0x431320: the COLL file named eax opened (one open before closed first,
 * closeCollFile): its header read and checked -- 'COLL' (strncmp, 0x4eb390),
 * version 11.  The handle, or 0 with the file closed. */
void openCollFile(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 ebp = cpu.ebp;
    cpu.esi = cpu.eax;
    cpu.edx = at(app, kCollHandle);
    if (cpu.edx != 0)
    {
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, closeCollFile))
            return;
    }
    cpu.ecx = ebp - 4;
    cpu.ebx = ebp - 8;
    cpu.edx = kCollHandle;
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, 0x4e28d0))
        return;
    cpu.ebx = 0x10;
    cpu.edx = kCollHeader;
    cpu.eax = at(app, kCollHandle);
    if (!byAddress(app, cpu, 0x4e2de0))
        return;
    bool good = false;
    if (cpu.eax != 0)
    {
        cpu.ebx = 4;
        cpu.edx = kCollMagic;
        cpu.eax = kCollHeader;
        if (!byAddress(app, cpu, 0x4eb390))
            return;
        good = cpu.eax == 0 && at(app, kCollVersion) == 0xb;
    }
    if (good)
        cpu.eax = at(app, kCollHandle);
    else
    {
        cpu.eax = at(app, kCollHandle);
        cpu.ebx = 0;
        if (!byAddress(app, cpu, 0x4e2da0))
            return;
        put(app, kCollHandle, cpu.ebx);
        cpu.eax = 0;
    }
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4313b0: the COLL file eax closed (0x4e2da0), its handle 0. */
void closeCollFile(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 0;
    if (!byAddress(app, cpu, 0x4e2da0))
        return;
    put(app, kCollHandle, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4313d0: the COLL file's size, its header's. */
void collFileSize(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, kCollSize);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4313e0: the COLL file eax past its header (0x4e2e60) read into edx
 * (0x4e2de0), its size less the header's 16 bytes.  1. */
void readCollFile(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.esi = cpu.edx;
    cpu.edx = 0x10;
    if (!byAddress(app, cpu, 0x4e2e60))
        return;
    cpu.ebx = at(app, kCollSize);
    cpu.edx = cpu.esi;
    cpu.eax = cpu.ecx;
    cpu.ebx -= 0x10;
    if (!byAddress(app, cpu, 0x4e2de0))
        return;
    cpu.eax = 1;
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x431410: the COLL file's chunks' count, its header's. */
void collFileCount(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, kCollCount);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x431420: chunk edx of the COLL file's data eax: eax plus its offset in
 * the table at eax. */
void collChunk(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax += at(app, cpu.eax + cpu.edx * 4);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x431430 (a car's +0x600): the player's car's controls for the frame --
 * 0x405740 first while a replay, a demo or 0x6fd494 says so, the car's own
 * (+0x5fc), then the controls' state at 0x79c4a5 into the car: the
 * steering and pedals' bytes, the gear asked for (stepped through
 * 0x48f710 or 0x48f6e0 as the car's gearbox is automatic or not), the
 * handbrake, horn and the like as 0 or 1, the view stepped (of three), and
 * the reset or the pause (0x405220, 0x429100). */
void playerControls(App* app, CPU& cpu)
{
    constexpr reg32 kControls = 0x79c4a5;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.edx = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.ecx = cpu.eax;
    if (sreg32(at(app, 0x5efd98)) > 0 || sreg32(at(app, 0x5efd88)) > 0 || at(app, 0x6fd494) != 0)
    {
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, 0x405740))
            return;
    }
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, at(app, cpu.ecx + 0x5fc)))
        return;
    cpu.al = byte(app, kControls + 4);
    putByte(app, cpu.ecx + 0x504, cpu.al);
    cpu.al = byte(app, kControls + 5);
    putByte(app, cpu.ecx + 0x505, cpu.al);
    cpu.eax = at(app, kControls);
    cpu.eax = reg32(sreg32(cpu.eax) >> 0x18);
    cpu.edx = at(app, cpu.ecx + 0x1f0);
    put(app, cpu.ecx + 0x518, cpu.eax);
    cpu.eax = cpu.edx * 4;
    cpu.eax -= cpu.edx;
    cpu.eax <<= 2;
    cpu.edx = cpu.eax;
    if (at(app, cpu.edx + cpu.eax * 8 + 0x6fd548) == 1)
    {
        cpu.eax = 0;
        cpu.al = byte(app, cpu.ecx + 0x50b);
        if (!byAddress(app, cpu, 0x48f710))
            return;
    }
    else
    {
        cpu.eax = at(app, cpu.ecx + 0x528);
        cpu.edx = at(app, cpu.eax + 8);
        cpu.eax = 0;
        cpu.al = byte(app, cpu.ecx + 0x50b);
        if (!byAddress(app, cpu, 0x48f6e0))
            return;
    }
    putByte(app, cpu.ecx + 0x506, cpu.al);
    cpu.eax = (byte(app, kControls + 6) & 4) ? 1 : 0;
    cpu.dl = byte(app, kControls + 6);
    putByte(app, cpu.ecx + 0x50d, cpu.al);
    cpu.eax = (cpu.dl & 8) ? 1 : 0;
    cpu.dh = byte(app, kControls + 6);
    putByte(app, cpu.ecx + 0x50e, cpu.al);
    cpu.eax = (cpu.dh & 0x80) ? 1 : 0;
    cpu.bl = byte(app, kControls + 6);
    putByte(app, cpu.ecx + 0x512, cpu.al);
    if (cpu.bl & 0x40)
    {
        cpu.eax = at(app, cpu.ecx + 0x660);
        ++cpu.eax;
        cpu.edx = cpu.eax;
        cpu.ebx = 3;
        cpu.edx = reg32(sreg32(cpu.edx) >> 0x1f);
        put(app, cpu.ecx + 0x660, cpu.eax);
        const sreg32 n = sreg32(cpu.eax);
        cpu.eax = reg32(n / 3);
        cpu.edx = reg32(n % 3);
        put(app, cpu.ecx + 0x660, cpu.edx);
    }
    if (byte(app, kControls + 6) & 2)
    {
        if (byte(app, cpu.ecx + 0x200) & 0x20)
        {
            cpu.edx = 1;
            cpu.eax = cpu.ecx;
            if (!byAddress(app, cpu, 0x405220))
                return;
            return leave();
        }
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, 0x429100))
            return;
    }
    leave();
}

// The render states the game keeps (setRenderState's cache), -1 for none.
constexpr reg32 kStateTexture = 0x554e54, kStateAlpha = 0x554e58, kStateDepthBias = 0x554e5c;
constexpr reg32 kSetRenderState = 0x431900;  // graphics::setRenderState: state eax, value edx
constexpr reg32 kThrashSetState = 0x9ef96c;  // THRASH_setstate's pointer: (state, value), stdcall

/* 0x4318c0: the cached render states forgotten (-1), so that each is set
 * again. */
void forgetRenderStates(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = reg32(-1);
    for (const reg32 state : {0x554e58u, 0x554e5cu, 0x554e60u, 0x554e64u, 0x554e68u, 0x554e6cu, 0x554e54u})
        put(app, state, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

namespace
{

/* The render states set in turn, each (state, value) through
 * graphics::setRenderState.  False when the game stopped. */
bool setStates(App* app, CPU& cpu, std::initializer_list<std::pair<reg32, reg32>> states)
{
    for (const auto& s : states)
    {
        cpu.eax = s.first;
        cpu.edx = s.second;
        if (!byAddress(app, cpu, kSetRenderState))
            return false;
    }
    return true;
}

/* The render-state functions' frame: push edx, push ebp, ..., pop, pop. */
void stateFunctionLeave(App* app, CPU& cpu)
{
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace

/* 0x431aa0: the render states for 2D text: states forgotten, then 0xd,
 * 0x15, 0x68, 2 and 4 all 0. */
void statesForText(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!direct(app, cpu, forgetRenderStates)
        || !setStates(app, cpu, {{0xd, 0}, {0x15, 0}, {0x68, 0}, {2, 0}, {4, 0}}))
        return;
    stateFunctionLeave(app, cpu);
}

/* 0x431af0: the same states, 4 first. */
void statesForPictures(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!direct(app, cpu, forgetRenderStates)
        || !setStates(app, cpu, {{4, 0}, {0xd, 0}, {0x15, 0}, {0x68, 0}, {2, 0}}))
        return;
    stateFunctionLeave(app, cpu);
}

/* 0x431b40: the render states for the race's HUD: the driver's depth (on
 * the card, 0x7a3a60, as 0x7a3a68 says while 0x6fbc34 is set, else 0), the
 * depth offsets (0x4c7a60), states forgotten, then 4 that depth, 0xd 1,
 * 0x15 the depth bias (0x7a3a64), 0x68 and 2 0. */
void statesForHud(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, 0x6fbc34);
    if (cpu.edx != 0)
    {
        cpu.eax = at(app, 0x7a3a68);
        put(app, 0x7a3a60, cpu.eax);
    }
    else
        put(app, 0x7a3a60, cpu.edx);
    if (!byAddress(app, cpu, 0x4c7a60) || !direct(app, cpu, forgetRenderStates))
        return;
    if (!setStates(app, cpu, {{4, at(app, 0x7a3a60)}}) || !setStates(app, cpu, {{0xd, 1}})
        || !setStates(app, cpu, {{0x15, at(app, 0x7a3a64)}}) || !setStates(app, cpu, {{0x68, 0}, {2, 0}}))
        return;
    stateFunctionLeave(app, cpu);
}

/* 0x431bc0: the render states for the 2D screens: states forgotten, then
 * 0xd, 0x15, 0x68 0, 2 1, 4 0, 0x2c2 1. */
void statesForScreens(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!direct(app, cpu, forgetRenderStates)
        || !setStates(app, cpu, {{0xd, 0}, {0x15, 0}, {0x68, 0}, {2, 1}, {4, 0}, {0x2c2, 1}}))
        return;
    stateFunctionLeave(app, cpu);
}

namespace
{

/* push ebp; mov ebp, esp; pop ebp; ret. */
void emptyFunction(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace

/* 0x431c20: a view pass's layer that draws nothing (an empty function). */
void passLayerNothing(App* app, CPU& cpu)
{
    emptyFunction(app, cpu);
}

/* 0x431c30: the main pass's second layer: nothing (an empty function). */
void passSecondLayer(App* app, CPU& cpu)
{
    emptyFunction(app, cpu);
}

/* 0x431c40: the main pass's first layer: nothing (an empty function). */
void passFirstLayer(App* app, CPU& cpu)
{
    emptyFunction(app, cpu);
}

// The software renderer's blend: its tables of a channel times another
// (5 bits by 5 at 0x5f1798, 6 by 5 at 0x5f0798) and the pixel format's
// masks and shifts (0x554e70 on: blue, green, red's masks, 0, green's and
// red's shifts).
constexpr reg32 kBlendMasks = 0x554e70;

/* 0x431c50: the software renderer's blend tables -- a times b over 31,
 * rounded up, to 31 (0x3f for the green's) -- and the pixel format's masks
 * and shifts for 15 bits (0x56438c 0xf) or 16. */
void makeBlendTables(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    struct Table
    {
        reg32 columns, most, at;
    };
    static const Table tables[] = {{0x20, 0x1f, 0x5f1796}, {0x40, 0x3f, 0x5f0796}};
    for (const Table& t : tables)
    {
        cpu.edi = 0;
        cpu.esi = 0;
        for (;;)
        {
            cpu.ebx = 0;
            for (;;)
            {
                safepoint(app, cpu);
                cpu.eax = cpu.esi;
                cpu.eax *= cpu.ebx;
                cpu.edx = cpu.eax + 0x1e;
                cpu.ecx = 0x1f;
                cpu.eax = cpu.edx;
                cpu.edx = reg32(sreg32(cpu.edx) >> 0x1f);
                const sreg32 n = sreg32(cpu.eax);
                cpu.eax = reg32(n / 0x1f);
                cpu.edx = reg32(n % 0x1f);
                cpu.ecx = cpu.eax;
                if (sreg32(cpu.eax) > sreg32(t.most))
                    cpu.ecx = t.most;
                ++cpu.edi;
                ++cpu.ebx;
                putHalf(app, cpu.edi * 2 + t.at, cpu.cx);
                safepoint(app, cpu);
                if (sreg32(cpu.ebx) >= sreg32(t.columns))
                    break;
            }
            ++cpu.esi;
            if (sreg32(cpu.esi) >= 0x20)
                break;
        }
    }
    if (at(app, 0x56438c) == 0xf)
    {
        cpu.ecx = 0x1f;
        cpu.edi = 0x3e0;
        cpu.ebx = 0x7c00;
        cpu.edx = 0xa;
        cpu.esi = 0;
        put(app, kBlendMasks, cpu.ecx);
        cpu.ecx = 5;
    }
    else
    {
        cpu.ebx = 0x1f;
        cpu.edi = 0x7e0;
        cpu.ecx = 5;
        cpu.edx = 0xb;
        cpu.esi = 0;
        put(app, kBlendMasks, cpu.ebx);
        cpu.ebx = 0xf800;
    }
    put(app, kBlendMasks + 4, cpu.edi);
    put(app, kBlendMasks + 0xc, cpu.esi);
    put(app, kBlendMasks + 8, cpu.ebx);
    put(app, kBlendMasks + 0x10, cpu.ecx);
    put(app, kBlendMasks + 0x14, cpu.edx);
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x431d50 (handed to the software renderer as its blend): a span of the
 * record at eax blended into the screen -- from +0xc to +0x10, a pixel at a
 * time, each the texture's texel (its rows at [eax], the coordinates at
 * +0x14 and +0x18 stepped by +0x1c and +0x20, masked by +8) through the
 * blend tables (0x5f1f98, 0x5f1fa4, 0x5f1f9c) added to the screen's pixel,
 * each channel to its most. */
void blendSpan(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 4;
    const reg32 ebp = cpu.ebp;
    for (;;)
    {
        safepoint(app, cpu);
        cpu.ecx = at(app, cpu.eax + 0x10);
        if (cpu.ecx < at(app, cpu.eax + 0xc))
            break;
        cpu.ebx = at(app, cpu.eax + 8);
        cpu.edx = at(app, cpu.eax + 0x18);
        cpu.esi = at(app, cpu.eax + 0x14);
        cpu.edx &= cpu.ebx;
        cpu.edx += cpu.esi;
        cpu.edx >>= 0xf;
        cpu.ebx = cpu.edx * 4;
        cpu.edx = at(app, cpu.eax);
        cpu.edx = at(app, cpu.edx + cpu.ebx);
        cpu.ebx = at(app, cpu.eax + 0x1c);
        cpu.ecx = cpu.esi + cpu.ebx;
        cpu.ebx = at(app, cpu.eax + 0x20);
        cpu.esi = at(app, cpu.eax + 0x18);
        cpu.edi = at(app, cpu.eax + 0xc);
        cpu.esi += cpu.ebx;
        cpu.di = half(app, cpu.edi);
        put(app, cpu.eax + 0x18, cpu.esi);
        // Blue.
        cpu.esi = at(app, kBlendMasks);
        cpu.ebx = at(app, 0x5f1f98);
        cpu.esi &= cpu.edx;
        put(app, cpu.eax + 0x14, cpu.ecx);
        cpu.si = half(app, cpu.ebx + cpu.esi * 2);
        cpu.ebx = 0;
        cpu.ecx = at(app, kBlendMasks);
        cpu.bx = cpu.di;
        cpu.esi &= 0xffff;
        cpu.ebx &= cpu.ecx;
        cpu.esi += cpu.ebx;
        if (sreg32(cpu.esi) > sreg32(cpu.ecx))
            cpu.esi = cpu.ecx;
        // Green.
        cpu.ebx = at(app, kBlendMasks + 4);
        cpu.cl = byte(app, kBlendMasks + 0x10);
        cpu.ebx &= cpu.edx;
        cpu.ebx = reg32(sreg32(cpu.ebx) >> (cpu.cl & 0x1f));
        cpu.ebx += cpu.ebx;
        put(app, ebp - 4, cpu.ebx);
        cpu.ecx = at(app, ebp - 4);
        cpu.ebx = at(app, 0x5f1fa4);
        cpu.ebx += cpu.ecx;
        cpu.bx = half(app, cpu.ebx);
        cpu.cl = byte(app, kBlendMasks + 0x10);
        cpu.ebx &= 0xffff;
        cpu.ebx <<= cpu.cl & 0x1f;
        cpu.ecx = 0;
        cpu.cx = cpu.di;
        cpu.ecx &= at(app, kBlendMasks + 4);
        cpu.ebx += cpu.ecx;
        cpu.ecx = at(app, kBlendMasks + 4);
        if (sreg32(cpu.ebx) > sreg32(cpu.ecx))
            cpu.ebx = cpu.ecx;
        // Red.
        cpu.edx &= at(app, kBlendMasks + 8);
        cpu.cl = byte(app, kBlendMasks + 0x14);
        cpu.edx = reg32(sreg32(cpu.edx) >> (cpu.cl & 0x1f));
        cpu.ecx = at(app, 0x5f1f9c);
        cpu.edx += cpu.edx;
        cpu.edx += cpu.ecx;
        cpu.dx = half(app, cpu.edx);
        cpu.cl = byte(app, kBlendMasks + 0x14);
        cpu.edx &= 0xffff;
        cpu.edx <<= cpu.cl & 0x1f;
        cpu.ecx = at(app, kBlendMasks + 8);
        cpu.edi &= 0xffff;
        cpu.edi &= cpu.ecx;
        cpu.edx += cpu.edi;
        if (sreg32(cpu.edx) > sreg32(cpu.ecx))
            cpu.edx = cpu.ecx;
        cpu.ebx |= cpu.esi;
        cpu.ebx |= cpu.edx;
        cpu.edx = at(app, cpu.eax + 0xc);
        cpu.ecx = cpu.edx + 2;
        put(app, cpu.eax + 0xc, cpu.ecx);
        putHalf(app, cpu.edx, cpu.bx);
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

/* 0x432690: how finely a quad of the corners eax, edx, ebx and ecx is to be
 * subdivided, by the nearest of their depths (+8): 0 beyond 9, 1 beyond 6,
 * else 2. */
void subdivisionDepth(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x18;
    const reg32 ebp = cpu.ebp;
    cpu.eax = at(app, cpu.eax + 8);
    put(app, ebp - 4, cpu.eax);
    cpu.eax = at(app, cpu.edx + 8);
    put(app, ebp - 0xc, cpu.eax);
    cpu.eax = at(app, cpu.ebx + 8);
    put(app, ebp - 8, cpu.eax);
    cpu.eax = at(app, cpu.ecx + 8);
    const Float second = loadFloat(app, ebp - 0xc);
    put(app, ebp - 0x10, cpu.eax);
    if (compare(cpu, second, loadFloat(app, ebp - 4)).below)
    {
        cpu.eax = at(app, ebp - 0xc);
        put(app, ebp - 4, cpu.eax);
    }
    for (const reg32 other : {8u, 0x10u})
        if (compare(cpu, loadFloat(app, ebp - other), loadFloat(app, ebp - 4)).below)
        {
            cpu.eax = at(app, ebp - other);
            put(app, ebp - 4, cpu.eax);
        }
    const Float nearest = loadFloat(app, ebp - 4);
    storeDouble(app, ebp - 0x18, nearest);
    if (!compare(cpu, nearest, loadDouble(app, kDepthNear)).belowOrEqual())
        cpu.eax = 0;
    else if (!compare(cpu, loadDouble(app, ebp - 0x18), loadDouble(app, kDepthFar)).belowOrEqual())
        cpu.eax = 1;
    else
        cpu.eax = 2;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

namespace
{

/* THRASH_setstate (state, value) through its pointer, the value kept at
 * `kept` first, as the drawers set a state that changed. */
bool setThrashState(App* app, CPU& cpu, reg32 state, reg32 value, reg32 kept)
{
    push(app, cpu, value);
    push(app, cpu, state);
    put(app, kept, value);
    return byAddress(app, cpu, at(app, kThrashSetState));
}

/* The drawers' alpha test (state 0x68): on for `on`, kept at kStateAlpha,
 * set only where it changes -- 1 from ecx or edx, 0 from edx or eax, as
 * each drawer's code has its registers (`one`, `zero`). */
bool alphaState(App* app, CPU& cpu, bool on, reg32 x86::CPU::*one, reg32 x86::CPU::*zero)
{
    if (on)
    {
        if (at(app, kStateAlpha) == 1)
            return true;
        cpu.*one = 1;
        return setThrashState(app, cpu, 0x68, cpu.*one, kStateAlpha);
    }
    if (at(app, kStateAlpha) == 0)
        return true;
    cpu.*zero = 0;
    return setThrashState(app, cpu, 0x68, cpu.*zero, kStateAlpha);
}

/* The drawers' texture (state 1): the record's texture's +4, or none. */
bool textureState(App* app, CPU& cpu)
{
    if (cpu.ecx != 0)
    {
        cpu.eax = at(app, cpu.esi + 0x18);
        cpu.edx = at(app, kStateTexture);
        cpu.ecx = at(app, cpu.eax + 4);
        if (cpu.edx != cpu.ecx)
            return setThrashState(app, cpu, 1, cpu.ecx, kStateTexture);
        return true;
    }
    if (at(app, kStateTexture) == 0)
        return true;
    cpu.eax = 0;
    return setThrashState(app, cpu, 1, cpu.eax, kStateTexture);
}

/* The drawers' depth bias (state 0x15): none for the record's flag 4, else
 * the race's (0x7a3a64). */
bool depthBiasState(App* app, CPU& cpu, reg32 flags)
{
    if (byte(app, flags) & 4)
    {
        if (at(app, kStateDepthBias) == 0)
            return true;
        cpu.ecx = 0;
        return setThrashState(app, cpu, 0x15, cpu.ecx, kStateDepthBias);
    }
    cpu.edx = at(app, 0x7a3a64);
    if (cpu.edx == at(app, kStateDepthBias))
        return true;
    return setThrashState(app, cpu, 0x15, cpu.edx, kStateDepthBias);
}

/* A drawer's way out: the next record in eax. */
void drawerLeave(App* app, CPU& cpu)
{
    cpu.eax = cpu.esi;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace

/* 0x433530 (a polygon list's drawer, the software renderers' and
 * Direct3D's): the textured triangles from the record at eax on, while
 * they are of kind 1 (+4): the alpha test by the record's flags (+6, bit
 * 1), the texture's coordinates into the three corners (+8, +0xc, +0x10)
 * unless bit 2, the texture (state 1), the software blend's colour for a
 * texture's flag 8 (0x432390), the triangle drawn (0x4c11b0).  The next
 * record, or 0. */
void drawTexturedTriangles(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 ebp = cpu.ebp;
    cpu.esi = cpu.eax;
    for (;;)
    {
        safepoint(app, cpu);
        cpu.eax = at(app, cpu.esi + 0xc);
        put(app, ebp - 4, cpu.eax);
        cpu.eax = at(app, cpu.esi + 4);
        cpu.eax = reg32(sreg32(cpu.eax) >> 0x10);
        cpu.edi = at(app, cpu.esi + 8);
        put(app, ebp - 8, cpu.eax);
        cpu.ebx = at(app, cpu.esi + 0x10);
        cpu.ah = byte(app, ebp - 8);
        if (!alphaState(app, cpu, (cpu.ah & 1) != 0, &CPU::ecx, &CPU::edx))
            return;
        cpu.ecx = at(app, cpu.esi + 0x18);
        if (cpu.ecx != 0 && !(byte(app, ebp - 8) & 2))
        {
            cpu.eax = cpu.ecx;
            cpu.edx = at(app, cpu.eax + 8);
            put(app, cpu.edi + 0x18, cpu.edx);
            cpu.eax = at(app, cpu.esi + 0x18);
            cpu.edx = at(app, cpu.eax + 0xc);
            put(app, cpu.edi + 0x1c, cpu.edx);
            cpu.eax = at(app, cpu.esi + 0x18);
            const Float u = loadFloat(app, cpu.eax + 0x10);
            cpu.eax = at(app, ebp - 4);
            storeFloat(app, cpu.eax + 0x18, u);
            cpu.eax = at(app, cpu.esi + 0x18);
            const Float v = loadFloat(app, cpu.eax + 0x14);
            cpu.eax = at(app, ebp - 4);
            storeFloat(app, cpu.eax + 0x1c, v);
            cpu.eax = at(app, cpu.esi + 0x18);
            cpu.edx = at(app, cpu.eax + 0x18);
            put(app, cpu.ebx + 0x18, cpu.edx);
            cpu.eax = at(app, cpu.esi + 0x18);
            cpu.edx = at(app, cpu.eax + 0x1c);
            put(app, cpu.ebx + 0x1c, cpu.edx);
        }
        if (!textureState(app, cpu))
            return;
        cpu.edx = at(app, cpu.esi + 0x18);
        if (cpu.edx != 0 && (byte(app, cpu.edx + 0x28) & 8))
        {
            cpu.eax = at(app, cpu.edi + 0x10);
            if (!byAddress(app, cpu, 0x432390))
                return;
        }
        cpu.edx = at(app, ebp - 4);
        cpu.eax = cpu.edi;
        if (!byAddress(app, cpu, 0x4c11b0))
            return;
        cpu.esi = at(app, cpu.esi);
        if (cpu.esi == 0)
            break;
        cpu.ax = half(app, cpu.esi + 4);
        if (cpu.ax != 1)
            break;
    }
    drawerLeave(app, cpu);
}

namespace
{

/* 0x433660 and 0x4339d0, the same code twice: the flat triangle at eax --
 * no texture (state 1), the alpha test by its flags (+6, bit 1), drawn
 * (0x4c0b20, its corners +8 and +0xc).  The next record. */
void flatTriangle(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.edx = at(app, kStateTexture);
    cpu.edi = at(app, cpu.eax + 0xc);
    cpu.esi = at(app, cpu.eax + 8);
    if (cpu.edx != 0)
    {
        cpu.ecx = 0;
        if (!setThrashState(app, cpu, 1, cpu.ecx, kStateTexture))
            return;
    }
    if (!alphaState(app, cpu, (byte(app, cpu.ebx + 6) & 1) != 0, &CPU::edx, &CPU::eax))
        return;
    cpu.edx = cpu.edi;
    cpu.eax = cpu.esi;
    if (!byAddress(app, cpu, 0x4c0b20))
        return;
    cpu.eax = at(app, cpu.ebx);
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace

/* 0x433660 (a polygon list's drawer): flatTriangle. */
void drawFlatTriangles(App* app, CPU& cpu)
{
    flatTriangle(app, cpu);
}

/* 0x4336e0 (a polygon list's drawer, the driver's): the triangles from the
 * record at eax on, while they are of kind 1 (+4): the alpha test (flags
 * +6, bit 1), the texture's coordinates into the corners (+8, +0xc, +0x10)
 * unless bit 2, the texture (state 1), the depth bias (state 0x15) by bit
 * 4, drawn by z (0x4c1aa0, bit 0x10) or not (0x4c11b0).  The next record,
 * or 0. */
void drawTrianglesByZ(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 ebp = cpu.ebp;
    cpu.esi = cpu.eax;
    for (;;)
    {
        safepoint(app, cpu);
        cpu.eax = at(app, cpu.esi + 8);
        put(app, ebp - 8, cpu.eax);
        cpu.eax = at(app, cpu.esi + 4);
        cpu.eax = reg32(sreg32(cpu.eax) >> 0x10);
        cpu.edi = at(app, cpu.esi + 0xc);
        put(app, ebp - 4, cpu.eax);
        cpu.ebx = at(app, cpu.esi + 0x10);
        cpu.ah = byte(app, ebp - 4);
        if (!alphaState(app, cpu, (cpu.ah & 1) != 0, &CPU::ecx, &CPU::edx))
            return;
        cpu.ecx = at(app, cpu.esi + 0x18);
        if (cpu.ecx != 0 && !(byte(app, ebp - 4) & 2))
        {
            cpu.eax = cpu.ecx;
            const Float u = loadFloat(app, cpu.eax + 8);
            cpu.eax = at(app, ebp - 8);
            storeFloat(app, cpu.eax + 0x18, u);
            cpu.eax = at(app, cpu.esi + 0x18);
            const Float v = loadFloat(app, cpu.eax + 0xc);
            cpu.eax = at(app, ebp - 8);
            storeFloat(app, cpu.eax + 0x1c, v);
            cpu.eax = at(app, cpu.esi + 0x18);
            cpu.edx = at(app, cpu.eax + 0x10);
            put(app, cpu.edi + 0x18, cpu.edx);
            cpu.eax = at(app, cpu.esi + 0x18);
            cpu.edx = at(app, cpu.eax + 0x14);
            put(app, cpu.edi + 0x1c, cpu.edx);
            cpu.eax = at(app, cpu.esi + 0x18);
            cpu.edx = at(app, cpu.eax + 0x18);
            put(app, cpu.ebx + 0x18, cpu.edx);
            cpu.eax = at(app, cpu.esi + 0x18);
            cpu.edx = at(app, cpu.eax + 0x1c);
            put(app, cpu.ebx + 0x1c, cpu.edx);
        }
        if (!textureState(app, cpu) || !depthBiasState(app, cpu, ebp - 4))
            return;
        cpu.eax = at(app, ebp - 8);
        cpu.edx = cpu.edi;
        if (!byAddress(app, cpu, (byte(app, ebp - 4) & 0x10) ? 0x4c1aa0 : 0x4c11b0))
            return;
        cpu.esi = at(app, cpu.esi);
        if (cpu.esi == 0)
            break;
        cpu.ax = half(app, cpu.esi + 4);
        if (cpu.ax != 1)
            break;
    }
    drawerLeave(app, cpu);
}

/* 0x433840 (a polygon list's drawer, the driver's): the quads from the
 * record at eax on, while they are of kind 0 (+4): as drawTrianglesByZ,
 * four corners (+8, +0xc, +0x10, +0x14), drawn by z (0x4c20e0) or not
 * (0x4c12e0).  The next record, or 0. */
void drawQuadsByZ(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0xc;
    const reg32 ebp = cpu.ebp;
    cpu.esi = cpu.eax;
    for (;;)
    {
        safepoint(app, cpu);
        cpu.eax = at(app, cpu.esi + 8);
        put(app, ebp - 8, cpu.eax);
        cpu.eax = at(app, cpu.esi + 0x10);
        put(app, ebp - 0xc, cpu.eax);
        cpu.eax = at(app, cpu.esi + 4);
        cpu.eax = reg32(sreg32(cpu.eax) >> 0x10);
        cpu.edi = at(app, cpu.esi + 0xc);
        put(app, ebp - 4, cpu.eax);
        cpu.ebx = at(app, cpu.esi + 0x14);
        cpu.ah = byte(app, ebp - 4);
        if (!alphaState(app, cpu, (cpu.ah & 1) != 0, &CPU::ecx, &CPU::edx))
            return;
        cpu.ecx = at(app, cpu.esi + 0x18);
        if (cpu.ecx != 0 && !(byte(app, ebp - 4) & 2))
        {
            cpu.eax = cpu.ecx;
            const Float u0 = loadFloat(app, cpu.eax + 8);
            cpu.eax = at(app, ebp - 8);
            storeFloat(app, cpu.eax + 0x18, u0);
            cpu.eax = at(app, cpu.esi + 0x18);
            const Float v0 = loadFloat(app, cpu.eax + 0xc);
            cpu.eax = at(app, ebp - 8);
            storeFloat(app, cpu.eax + 0x1c, v0);
            cpu.eax = at(app, cpu.esi + 0x18);
            cpu.edx = at(app, cpu.eax + 0x10);
            put(app, cpu.edi + 0x18, cpu.edx);
            cpu.eax = at(app, cpu.esi + 0x18);
            cpu.edx = at(app, cpu.eax + 0x14);
            put(app, cpu.edi + 0x1c, cpu.edx);
            cpu.eax = at(app, cpu.esi + 0x18);
            const Float u2 = loadFloat(app, cpu.eax + 0x18);
            cpu.eax = at(app, ebp - 0xc);
            storeFloat(app, cpu.eax + 0x18, u2);
            cpu.eax = at(app, cpu.esi + 0x18);
            const Float v2 = loadFloat(app, cpu.eax + 0x1c);
            cpu.eax = at(app, ebp - 0xc);
            storeFloat(app, cpu.eax + 0x1c, v2);
            cpu.eax = at(app, cpu.esi + 0x18);
            cpu.edx = at(app, cpu.eax + 0x20);
            put(app, cpu.ebx + 0x18, cpu.edx);
            cpu.eax = at(app, cpu.esi + 0x18);
            cpu.edx = at(app, cpu.eax + 0x24);
            put(app, cpu.ebx + 0x1c, cpu.edx);
        }
        if (!textureState(app, cpu) || !depthBiasState(app, cpu, ebp - 4))
            return;
        cpu.eax = at(app, ebp - 8);
        cpu.ecx = cpu.ebx;
        cpu.edx = cpu.edi;
        cpu.ebx = at(app, ebp - 0xc);
        if (!byAddress(app, cpu, (byte(app, ebp - 4) & 0x10) ? 0x4c20e0 : 0x4c12e0))
            return;
        cpu.esi = at(app, cpu.esi);
        if (cpu.esi == 0)
            break;
        cpu.ax = half(app, cpu.esi + 4);
        if (cpu.ax != 0)
            break;
    }
    drawerLeave(app, cpu);
}

/* 0x4339d0 (a polygon list's drawer, the driver's): flatTriangle. */
void drawFlatTrianglesAgain(App* app, CPU& cpu)
{
    flatTriangle(app, cpu);
}

/* 0x4347b0 (a polygon list's drawer): the triangles with colours of their
 * own -- the driver's on the card (0x7a3a60, 0x434380) or the other
 * (0x433e30), tail-called. */
void drawColouredTriangles(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!byAddress(app, cpu, at(app, 0x7a3a60) != 0 ? 0x434380 : 0x433e30))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x434850 (a polygon list's drawer): a render state's record -- state +8,
 * value +0xc (setRenderState).  The next record. */
void stateRecord(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.edx = at(app, cpu.eax + 0xc);
    cpu.eax = at(app, cpu.eax + 8);
    if (!byAddress(app, cpu, kSetRenderState))
        return;
    cpu.eax = at(app, cpu.ecx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x434950: a polygon list drawn from its record at eax: state 0xd 1, then
 * each record handed to the drawer of its kind (its word at +4, the
 * drawers' table at 0x5f0780), which gives back the next. */
void drawRecords(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.edx = 1;
    cpu.eax = 0xd;
    if (!byAddress(app, cpu, kSetRenderState))
        return;
    for (;;)
    {
        safepoint(app, cpu);
        if (cpu.ecx == 0)
            break;
        cpu.eax = at(app, cpu.ecx + 2);
        cpu.eax = reg32(sreg32(cpu.eax) >> 0x10);
        cpu.edx = cpu.eax * 4;
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, at(app, cpu.edx + 0x5f0780)))
            return;
        cpu.ecx = cpu.eax;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x434990: the race's text for the frame (0x4bed70 with 0x7cdac0). */
void raceTextFrame(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = 0x7cdac0;
    if (!byAddress(app, cpu, 0x4bed70))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4349a0: whether the view at eax is mirrored ([eax] 1) against the
 * mirrored screen (0x6fd4c4), into 0x554e4c. */
void setMirrored(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, cpu.eax);
    cpu.eax = cpu.edx == 1 ? cpu.edx : 0;
    cpu.edx = at(app, 0x6fd4c4);
    cpu.eax ^= cpu.edx;
    put(app, 0x554e4c, cpu.eax);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4349d0: setMirrored for the view at eax, then the polygon list on the
 * stack's second word drawn (drawRecords); ret 8. */
void drawMirroredRecords(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = at(app, cpu.eax);
    cpu.eax = cpu.edx == 1 ? cpu.edx : 0;
    cpu.edx = at(app, 0x6fd4c4);
    cpu.eax ^= cpu.edx;
    put(app, 0x554e4c, cpu.eax);
    cpu.eax = at(app, cpu.ebp + 0xc);
    if (!direct(app, cpu, drawRecords))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4 + 8;
}

/* 0x434a70: the frames drawn counted (0x5f1fa8). */
void countFrame(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    put(app, 0x5f1fa8, at(app, 0x5f1fa8) + 1);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x434a80: the race's view distance (0x41d610 with 0x6fbc28). */
void raceViewDistance(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, 0x6fbc28);
    if (!byAddress(app, cpu, 0x41d610))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x434a90: the race's fog colour (state 3): its bytes at 0x6ff9f8 as
 * blue, green and red, alpha 0xff. */
void raceFrameEnd(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = 0;
    cpu.al = byte(app, 0x6ff9fa);
    cpu.edx = cpu.eax;
    cpu.eax = 0;
    cpu.edx <<= 0x10;
    cpu.al = byte(app, 0x6ff9f9);
    cpu.edx |= 0xff000000;
    cpu.eax <<= 8;
    cpu.edx |= cpu.eax;
    cpu.eax = 0;
    cpu.al = byte(app, 0x6ff9f8);
    cpu.edx |= cpu.eax;
    cpu.eax = 3;
    if (!byAddress(app, cpu, kSetRenderState))
        return;
    stateFunctionLeave(app, cpu);
}

/* 0x434ae0: nothing at the race's frame (an empty function). */
void raceFrameNothing(App* app, CPU& cpu)
{
    emptyFunction(app, cpu);
}

/* 0x434af0: nothing at the race's end (an empty function). */
void raceEndNothing(App* app, CPU& cpu)
{
    emptyFunction(app, cpu);
}

/* 0x434b00: the drawing made ready at the race's start: the reciprocals'
 * table (0x4eb3c0), the polygon lists' drawers (0x434870). */
void raceStartDrawing(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    if (!byAddress(app, cpu, 0x4eb3c0) || !byAddress(app, cpu, 0x434870))
        return;
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x434b10: nothing at the race's start (an empty function). */
void raceStartNothing(App* app, CPU& cpu)
{
    emptyFunction(app, cpu);
}

namespace
{

/* A corner's clip code (as native_graphics.cpp's shapeClipCode has it):
 * the corner at esi, the code's byte to edi -- 0x10 for its +0xc at or
 * past the far edge (0x7d34f8), else 8 above or 4 below (+4), 1 left of or
 * 2 right of (+0) the screen's rectangle; each a float's bits compared as
 * integers, a negative one as short. */
void clipCode(App* app, CPU& cpu)
{
    cpu.eax = at(app, cpu.esi + 0xc);
    cpu.ecx = at(app, 0x7d34f8);
    if ((cpu.eax & 0x80000000) || sreg32(cpu.eax) >= sreg32(cpu.ecx))
        cpu.ebx = 0x10;
    else
    {
        cpu.eax = at(app, cpu.esi + 4);
        cpu.ebx = 0;
        cpu.ecx = at(app, 0x7d3504);
        cpu.edx = at(app, 0x7d3500);
        if ((cpu.eax & 0x80000000) || sreg32(cpu.eax) < sreg32(cpu.ecx))
            cpu.ebx |= 8;
        else if (sreg32(cpu.eax) > sreg32(cpu.edx))
            cpu.ebx |= 4;
        cpu.eax = at(app, cpu.esi);
        cpu.ecx = at(app, 0x7d34fc);
        cpu.edx = at(app, 0x7d350c);
        if ((cpu.eax & 0x80000000) || sreg32(cpu.eax) < sreg32(cpu.ecx))
            cpu.ebx |= 1;
        else if (sreg32(cpu.eax) > sreg32(cpu.edx))
            cpu.ebx |= 2;
    }
    putByte(app, cpu.edi, cpu.bl);
}

}  // namespace

/* 0x434b60: the mouse's pointer drawn in the race: where the mouse is
 * (0x435310, less the view's corner, 0x4dab10) as a 32 by 32 quad of four
 * corners (0x20 bytes each) in the frame -- at depth 0, w just under 1,
 * white -- its texture's coordinates and the texture from the pointer's
 * record (0x8b41dc + 0x2c times 0x563d3c), each corner's clip code, the
 * texture set (state 1) and the quad drawn (0x4c12e0). */
void drawPointer(App* app, CPU& cpu)
{
    constexpr reg32 kPointers = 0x8b41dc, kPointer = 0x563d3c;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 0x9c;
    cpu.ebp -= 0x82;
    const reg32 f = cpu.ebp;
    cpu.ecx = f + 0x7a;
    cpu.ebx = f + 0x76;
    cpu.edx = f + 0x72;
    cpu.eax = f + 0x6e;
    if (!byAddress(app, cpu, 0x435310))
        return;
    cpu.edx = f + 0x6a;
    cpu.eax = f + 0x66;
    if (!byAddress(app, cpu, 0x4dab10))
        return;
    // The corners' x and y: the mouse's, and 32 on.
    cpu.eax = at(app, f + 0x72);
    cpu.eax -= at(app, f + 0x66);
    cpu.ecx = at(app, f + 0x6a);
    put(app, f + 0x7e, cpu.eax);
    cpu.edx = 0x3f7fff00;
    storeFloat(app, f + 0x46, loadInteger(app, f + 0x7e));
    put(app, f + 0x52, cpu.edx);
    put(app, f + 0x32, cpu.edx);
    put(app, f + 0x12, cpu.edx);
    cpu.eax = at(app, f + 0x46);
    put(app, f - 0xe, cpu.edx);
    put(app, f + 0x26, cpu.eax);
    put(app, f + 6, cpu.eax);
    put(app, f - 0x1a, cpu.eax);
    cpu.eax = at(app, f + 0x76);
    const Float x = loadFloat(app, f + 0x46);
    cpu.eax -= cpu.ecx;
    const Float size = loadFloat(app, kPointerSize);
    const Float right = fp::add(x, size);
    put(app, f + 0x7e, cpu.eax);
    storeFloat(app, f + 6, right);
    storeFloat(app, f + 0x4a, loadInteger(app, f + 0x7e));
    cpu.eax = at(app, f + 0x4a);
    const Float y = loadFloat(app, f + 0x4a);
    put(app, f + 0x2a, cpu.eax);
    put(app, f + 0xa, cpu.eax);
    put(app, f - 0x16, cpu.eax);
    const Float bottom = fp::add(size, y);
    cpu.eax = at(app, f + 6);
    storeFloat(app, f + 0x2a, bottom);
    put(app, f + 0x26, cpu.eax);
    cpu.eax = at(app, f + 0x2a);
    cpu.edx = at(app, kPointer);
    put(app, f + 0x4a, cpu.eax);
    // The pointer's record: 11 words.
    cpu.eax = cpu.edx * 4;
    cpu.eax -= cpu.edx;
    cpu.eax <<= 2;
    cpu.edi = reg32(-1);
    cpu.eax -= cpu.edx;
    cpu.ebx = 0;
    cpu.edx = cpu.eax * 4;
    cpu.esi = f - 0x1a;
    put(app, f + 0x4e, cpu.ebx);
    cpu.eax = at(app, cpu.edx + kPointers + 4);
    put(app, f + 0x2e, cpu.ebx);
    put(app, f - 2, cpu.eax);
    cpu.eax = at(app, cpu.edx + kPointers + 8);
    put(app, f + 0xe, cpu.ebx);
    put(app, f + 2, cpu.eax);
    cpu.eax = at(app, cpu.edx + kPointers + 0xc);
    put(app, f - 0x12, cpu.ebx);
    put(app, f + 0x1e, cpu.eax);
    cpu.eax = at(app, cpu.edx + kPointers + 0x10);
    put(app, f + 0x56, cpu.edi);
    put(app, f + 0x22, cpu.eax);
    cpu.eax = at(app, cpu.edx + kPointers + 0x14);
    put(app, f + 0x36, cpu.edi);
    put(app, f + 0x3e, cpu.eax);
    cpu.eax = at(app, cpu.edx + kPointers + 0x18);
    put(app, f + 0x16, cpu.edi);
    put(app, f + 0x42, cpu.eax);
    cpu.eax = at(app, cpu.edx + kPointers + 0x1c);
    put(app, f - 0xa, cpu.edi);
    put(app, f + 0x5e, cpu.eax);
    cpu.eax = at(app, cpu.edx + kPointers + 0x20);
    cpu.edi = f - 6;
    put(app, f + 0x62, cpu.eax);
    // The corners' clip codes.
    clipCode(app, cpu);
    for (const reg32 corner : {6u, 0x26u, 0x46u})
    {
        cpu.edi = f + corner + 0x14;
        cpu.esi = f + corner;
        clipCode(app, cpu);
    }
    cpu.edx = at(app, kPointer);
    cpu.eax = cpu.edx * 4;
    cpu.eax -= cpu.edx;
    cpu.eax <<= 2;
    cpu.eax -= cpu.edx;
    cpu.ecx = f + 0x46;
    cpu.edx = at(app, cpu.eax * 4 + kPointers);
    cpu.eax = 1;
    cpu.ebx = f + 0x26;
    if (!byAddress(app, cpu, kSetRenderState))
        return;
    cpu.edx = f + 6;
    cpu.eax = f - 0x1a;
    if (!byAddress(app, cpu, 0x4c12e0))
        return;
    cpu.esp = f + 0x82;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

// The race's messages (chat): their channels and lines in a block of
// 0x24c8 bytes (0x554f04) -- the line's length (+0), the lines a channel
// keeps (+4), the channels (+8), its kind (+0xc), the copy's start (+0x10),
// scrolling (+0x20), and from +0x28 a channel of 0x494 bytes each (its
// line kept +0, its first +4, its next +8, from +0xc lines of 0x74 bytes:
// where its text is +0, +4, its length +8, its text from +0xc) -- the
// lines copied out into another of 0x3f8 (0x554f08: its count, then lines
// of 0x65 bytes), and whether they are in use (0x554f0c).
constexpr reg32 kMessages = 0x554f04, kMessageCopy = 0x554f08, kMessagesOn = 0x554f0c;
constexpr reg32 kChannelBytes = 0x494;
constexpr reg32 kAllocate = 0x4e1620, kFree = 0x4e1890, kFillZero = 0x4e070c, kMoveMemory = 0x4ea4f0;

/* 0x436c40: the messages' blocks allocated and cleared (0x4e1620,
 * 0x4e070c), their settings from the eight words at eax (the line's length
 * and the lines kept made larger by +0xc, the second if the lines kept are
 * more than 1), the copy's count from eax's +4; in use. */
void openMessages(App* app, CPU& cpu)
{
    constexpr reg32 kNoName = 0x537490;
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ecx = cpu.eax;
    cpu.edi = at(app, cpu.eax + 8);
    cpu.edx = at(app, cpu.eax + 0xc);
    cpu.esi = at(app, cpu.eax + 4);
    if (cpu.edx != 0)
    {
        if (sreg32(cpu.esi) > 1)
            cpu.esi += cpu.edx;
        else
            cpu.edi += cpu.edx;
    }
    cpu.ebx = 0x10;
    cpu.edx = 0x24c8;
    cpu.eax = kNoName;
    if (!byAddress(app, cpu, kAllocate))
        return;
    cpu.edx = 0x24c8;
    put(app, kMessages, cpu.eax);
    if (!byAddress(app, cpu, kFillZero))
        return;
    cpu.eax = at(app, kMessages);
    cpu.edx = cpu.eax + 8;
    cpu.ebx = 0x20;
    cpu.eax = cpu.ecx;
    if (!byAddress(app, cpu, kMoveMemory))
        return;
    cpu.eax = at(app, kMessages);
    cpu.ebx = 0x10;
    put(app, cpu.eax + 4, cpu.esi);
    cpu.edx = 0x3f8;
    put(app, cpu.eax, cpu.edi);
    cpu.eax = kNoName;
    if (!byAddress(app, cpu, kAllocate))
        return;
    cpu.edx = 0x3f8;
    put(app, kMessageCopy, cpu.eax);
    if (!byAddress(app, cpu, kFillZero))
        return;
    cpu.eax = at(app, kMessageCopy);
    cpu.edx = at(app, cpu.ecx + 4);
    put(app, cpu.eax, cpu.edx);
    put(app, kMessagesOn, 1);
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x436cf0: the messages out of use, their blocks freed (0x4e1890). */
void closeMessages(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = 0;
    cpu.ecx = at(app, kMessages);
    put(app, kMessagesOn, cpu.edx);
    if (cpu.ecx != 0)
    {
        cpu.eax = cpu.ecx;
        if (!byAddress(app, cpu, kFree))
            return;
    }
    cpu.ebx = at(app, kMessageCopy);
    if (cpu.ebx != 0)
    {
        cpu.eax = cpu.ebx;
        if (!byAddress(app, cpu, kFree))
            return;
    }
    cpu.esi = 0;
    put(app, kMessages, cpu.esi);
    put(app, kMessageCopy, cpu.esi);
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x436d40: every channel cleared (clearMessages). */
void clearAllMessages(App* app, CPU& cpu)
{
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    for (cpu.edx = 0;; ++cpu.edx)
    {
        safepoint(app, cpu);
        cpu.eax = at(app, kMessages);
        if (sreg32(cpu.edx) >= sreg32(at(app, cpu.eax + 8)))
            break;
        cpu.eax = cpu.edx;
        if (!direct(app, cpu, clearMessages))
            return;
    }
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x436d60: channel eax cleared while the messages are in use: its lines
 * (clearMessageLine), its kept, first and next line 0 -- out of use the
 * while. */
void clearMessages(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax;
    cpu.esi = at(app, kMessagesOn);
    if (cpu.esi != 0)
    {
        cpu.edx = 0;
        cpu.ecx = 0;
        put(app, kMessagesOn, cpu.edx);
        for (;; ++cpu.ecx)
        {
            safepoint(app, cpu);
            cpu.eax = at(app, kMessages);
            if (sreg32(cpu.ecx) >= sreg32(at(app, cpu.eax + 4)))
                break;
            cpu.edx = cpu.ecx;
            cpu.eax = cpu.ebx;
            if (!direct(app, cpu, clearMessageLine))
                return;
        }
        cpu.ebx *= kChannelBytes;
        put(app, cpu.ebx + cpu.eax + 0x28, 0);
        put(app, cpu.ebx + cpu.eax + 0x2c, 0);
        put(app, kMessagesOn, cpu.esi);
        put(app, cpu.ebx + cpu.eax + 0x30, 0);
    }
    cpu.ebp = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x436dc0: line edx of channel eax cleared: its words and its text's
 * first byte 0. */
void clearMessageLine(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.ebx = cpu.eax * kChannelBytes;
    cpu.ecx = at(app, kMessages);
    cpu.eax = cpu.ecx + 0x28;
    cpu.ebx += cpu.eax;
    cpu.eax = cpu.edx * 8;
    cpu.eax -= cpu.edx;
    cpu.eax <<= 2;
    cpu.edx += cpu.eax;
    cpu.edx <<= 2;
    cpu.eax = cpu.ebx + 0xc;
    put(app, cpu.edx + cpu.eax, 0);
    put(app, cpu.edx + cpu.eax + 4, 0);
    put(app, cpu.edx + cpu.eax + 8, 0);
    putByte(app, cpu.edx + cpu.eax + 0xc, 0);
    cpu.ebp = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x436e10: the text at edx added to channel eax a character at a time
 * (addMessageCharacter), up to the line's length, the rest of the line
 * filled with 0s; the additions counted (0x5f3fd8).  1, or 0 out of use or
 * when a character could not be added. */
void addMessageText(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    const auto leave = [&] {
        cpu.ebp = pop(app, cpu);
        cpu.esi = pop(app, cpu);
        cpu.ecx = pop(app, cpu);
        cpu.ebx = pop(app, cpu);
        cpu.esp += 4;
    };
    cpu.esi = cpu.eax;
    cpu.ebx = cpu.edx;
    cpu.edx = at(app, kMessagesOn);
    cpu.ecx = 0;
    if (cpu.edx == 0)
    {
        cpu.eax = 0;
        return leave();
    }
    put(app, 0x5f3fd8, at(app, 0x5f3fd8) + 1);
    for (;;)
    {
        safepoint(app, cpu);
        if (byte(app, cpu.ebx) == 0)
            break;
        cpu.eax = at(app, kMessages);
        if (sreg32(cpu.ecx) >= sreg32(at(app, cpu.eax)))
            break;
        cpu.edx = 0;
        cpu.eax = cpu.esi;
        cpu.dl = byte(app, cpu.ebx);
        if (!direct(app, cpu, addMessageCharacter))
            return;
        if (cpu.eax == 0)
            return leave();
        ++cpu.ecx;
        ++cpu.ebx;
    }
    for (;;)
    {
        safepoint(app, cpu);
        cpu.eax = at(app, kMessages);
        if (sreg32(cpu.ecx) >= sreg32(at(app, cpu.eax)))
            break;
        cpu.eax = cpu.esi;
        cpu.edx = 0;
        if (!direct(app, cpu, addMessageCharacter))
            return;
        if (cpu.eax == 0)
            return leave();
        ++cpu.ecx;
    }
    cpu.eax = 1;
    leave();
}

namespace
{

/* idiv by a word: edx:eax (eax's sign in edx) divided, as x86 does. */
void divide(CPU& cpu, reg32 by)
{
    const std::int64_t n = std::int64_t(std::uint64_t(cpu.edx) << 32 | cpu.eax);
    const std::int64_t d = sreg32(by);
    cpu.eax = reg32(n / d);
    cpu.edx = reg32(n % d);
}

}  // namespace

/* 0x436e90: the character dl added to channel eax's next line, while the
 * messages are in use: at the line's length, for a channel of kind 1, the
 * line starts again and the next line's first moves on (of the line's
 * length plus one); for another kind the next line taken (clearMessageLine)
 * and the kept and the first moved on as they meet, scrolled
 * (messagesScrolled) and the kept the first.  Its text ended after it.  1,
 * or 0 out of use. */
void addMessageCharacter(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 ebp = cpu.ebp;
    cpu.ebx = cpu.eax;
    putByte(app, ebp - 4, cpu.dl);
    if (at(app, kMessagesOn) == 0)
        cpu.ecx = 0;
    else
    {
        cpu.eax *= kChannelBytes;
        cpu.edi = at(app, kMessages);
        put(app, ebp - 8, cpu.eax);
        cpu.esi = cpu.eax + cpu.edi;
        cpu.edx = at(app, cpu.esi + 0x30);
        cpu.eax = cpu.edx * 8;
        cpu.eax -= cpu.edx;
        cpu.eax <<= 2;
        cpu.ecx = at(app, ebp - 8);
        cpu.edx += cpu.eax;
        cpu.eax = cpu.edi + 0x28;
        cpu.eax += cpu.ecx;
        cpu.edx <<= 2;
        cpu.eax += 0xc;
        cpu.ecx = cpu.eax + cpu.edx;
        cpu.eax = at(app, cpu.ecx + 8);
        cpu.edx = cpu.eax + 1;
        put(app, cpu.ecx + 8, cpu.edx);
        cpu.dl = byte(app, ebp - 4);
        putByte(app, cpu.ecx + cpu.eax + 0xc, cpu.dl);
        if (at(app, cpu.edi + 0xc) == 1)
        {
            cpu.edx = at(app, cpu.edi);
            if (sreg32(cpu.edx) < sreg32(at(app, cpu.ecx + 8)))
                put(app, cpu.ecx + 8, 0);
            cpu.ebx = at(app, cpu.ecx + 4);
            if (cpu.ebx == at(app, cpu.ecx + 8))
            {
                cpu.esi = cpu.ebx + 1;
                cpu.eax = at(app, kMessages);
                put(app, cpu.ecx + 4, cpu.esi);
                cpu.edx = cpu.esi;
                cpu.ebx = at(app, cpu.eax);
                cpu.edx = reg32(sreg32(cpu.edx) >> 0x1f);
                ++cpu.ebx;
                cpu.eax = cpu.esi;
                divide(cpu, cpu.ebx);
                put(app, cpu.ecx + 4, cpu.edx);
            }
            cpu.eax = at(app, cpu.ecx + 4);
            put(app, cpu.ecx, cpu.eax);
        }
        else
        {
            cpu.edx = at(app, cpu.edi);
            if (cpu.edx == at(app, cpu.ecx + 8))
            {
                cpu.eax = at(app, cpu.esi + 0x30);
                ++cpu.eax;
                cpu.edx = cpu.eax;
                put(app, cpu.esi + 0x30, cpu.eax);
                cpu.edx = reg32(sreg32(cpu.edx) >> 0x1f);
                divide(cpu, at(app, cpu.edi + 4));
                cpu.eax = cpu.ebx;
                put(app, cpu.esi + 0x30, cpu.edx);
                if (!direct(app, cpu, clearMessageLine))
                    return;
                cpu.esi = at(app, ebp - 8);
                cpu.edi = at(app, kMessages);
                cpu.esi += cpu.edi;
                cpu.eax = at(app, cpu.esi + 0x30);
                cpu.edx = at(app, cpu.esi + 0x2c);
                if (cpu.eax == cpu.edx)
                {
                    cpu.eax = cpu.edx + 1;
                    cpu.edx = cpu.eax;
                    put(app, cpu.esi + 0x2c, cpu.eax);
                    cpu.edx = reg32(sreg32(cpu.edx) >> 0x1f);
                    divide(cpu, at(app, cpu.edi + 4));
                    put(app, cpu.esi + 0x2c, cpu.edx);
                }
            }
            cpu.eax = at(app, kMessages);
            if (at(app, cpu.eax + 0x20) != 0)
            {
                cpu.eax = cpu.ebx;
                if (!direct(app, cpu, messagesScrolled))
                    return;
            }
            cpu.edx = cpu.ebx * kChannelBytes;
            cpu.eax = at(app, kMessages);
            cpu.eax += cpu.edx;
            cpu.edx = at(app, cpu.eax + 0x2c);
            put(app, cpu.eax + 0x28, cpu.edx);
        }
        cpu.eax = at(app, cpu.ecx + 8);
        putByte(app, cpu.ecx + cpu.eax + 0xc, 0);
        cpu.ecx = 1;
    }
    cpu.eax = cpu.ecx;
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x437080: channel eax's lines copied out (0x554f08, which it gives back):
 * for a copy of one line, as many of the channel's text bytes from the
 * copy's start as fit, in one or two pieces (0x4ea4f0); else each of its
 * lines from the kept one on (of the lines kept, round), 0x65 bytes
 * apart, by the code's inlined strcpy. */
void copyMessageLines(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.esi);
    push(app, cpu, cpu.edi);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.esp -= 8;
    const reg32 ebp = cpu.ebp;
    cpu.ebx = cpu.eax;
    cpu.edx = cpu.ebx * kChannelBytes;
    cpu.eax = at(app, kMessageCopy);
    cpu.ecx = at(app, cpu.eax);
    if (cpu.ecx == 1)
    {
        cpu.eax = at(app, kMessages);
        cpu.esi = at(app, cpu.eax + 0x10);
        cpu.edx = at(app, cpu.edx + cpu.eax + 0x34);
        cpu.esi += cpu.ecx;
        cpu.edi = cpu.edx + cpu.esi;
        cpu.ecx = cpu.esi;
        if (sreg32(cpu.esi) < sreg32(cpu.edi))
        {
            cpu.eax = at(app, cpu.eax);
            cpu.eax -= cpu.edx;
            cpu.ecx = cpu.eax + 1;
        }
        cpu.edi = cpu.ebx * kChannelBytes;
        cpu.eax = at(app, kMessageCopy);
        cpu.eax += 4;
        put(app, ebp - 8, cpu.eax);
        cpu.eax = at(app, kMessages);
        cpu.eax += 0x28;
        cpu.eax += cpu.edi;
        cpu.eax += 0x18;
        cpu.ebx = cpu.ecx;
        cpu.eax += cpu.edx;
        cpu.edx = at(app, ebp - 8);
        if (!byAddress(app, cpu, kMoveMemory))
            return;
        if (sreg32(cpu.ecx) < sreg32(cpu.esi))
        {
            cpu.eax = at(app, kMessageCopy);
            cpu.eax += 4;
            cpu.edx = cpu.eax + cpu.ecx;
            cpu.eax = at(app, kMessages);
            cpu.eax += 0x28;
            cpu.ebx = cpu.esi;
            cpu.eax += cpu.edi;
            cpu.ebx -= cpu.ecx;
            cpu.eax += 0x18;
            if (!byAddress(app, cpu, kMoveMemory))
                return;
        }
    }
    else
    {
        cpu.eax = at(app, kMessages);
        cpu.eax = at(app, cpu.edx + cpu.eax + 0x28);
        cpu.ecx = 0;
        put(app, ebp - 4, cpu.eax);
        for (;; ++cpu.ecx)
        {
            safepoint(app, cpu);
            cpu.edi = at(app, kMessageCopy);
            if (sreg32(cpu.ecx) >= sreg32(at(app, cpu.edi)))
                break;
            cpu.edx = at(app, ebp - 4);
            cpu.edx += cpu.ecx;
            cpu.esi = at(app, kMessages);
            cpu.eax = cpu.edx;
            cpu.edx = reg32(sreg32(cpu.edx) >> 0x1f);
            divide(cpu, at(app, cpu.esi + 4));
            cpu.eax = cpu.edx * 8;
            cpu.eax -= cpu.edx;
            cpu.eax <<= 2;
            cpu.eax += cpu.edx;
            cpu.eax <<= 2;
            put(app, ebp - 8, cpu.eax);
            cpu.eax = cpu.ebx * kChannelBytes;
            cpu.esi += 0x28;
            cpu.eax += cpu.esi;
            cpu.edx = at(app, ebp - 8);
            cpu.eax += 0xc;
            cpu.eax += cpu.edx;
            cpu.esi = cpu.eax + 0xc;
            cpu.eax = cpu.ecx * 0x65;
            cpu.edi += 4;
            cpu.edi += cpu.eax;
            push(app, cpu, cpu.edi);
            for (;;)
            {
                safepoint(app, cpu);
                cpu.al = byte(app, cpu.esi);
                putByte(app, cpu.edi, cpu.al);
                if (cpu.al == 0)
                    break;
                cpu.al = byte(app, cpu.esi + 1);
                cpu.esi += 2;
                putByte(app, cpu.edi + 1, cpu.al);
                cpu.edi += 2;
                if (cpu.al == 0)
                    break;
            }
            cpu.edi = pop(app, cpu);
        }
    }
    cpu.eax = at(app, kMessageCopy);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop(app, cpu);
    cpu.edi = pop(app, cpu);
    cpu.esi = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4371a0: what the messages do when a channel scrolls: nothing but read
 * their block (eax) and compare its kind with 1, for flags no caller
 * reads. */
void messagesScrolled(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.eax = at(app, kMessages);
    cpu.ebp = pop(app, cpu);
    cpu.esp += 4;
}

/* 0x4371c0: nothing (an empty function). */
void messagesNothing(App* app, CPU& cpu)
{
    emptyFunction(app, cpu);
}

/* 0x4371d0: a hash's seed from eax: eax made odd and three times it made
 * odd, multiplied eight times by the first, kept to 16 bits but the last
 * -- the whole into 0x554f14, its low 16 bits into 0x554f10, the odd eax
 * into 0x554f18. */
void seedHash(App* app, CPU& cpu)
{
    push(app, cpu, cpu.ebx);
    push(app, cpu, cpu.ecx);
    push(app, cpu, cpu.edx);
    push(app, cpu, cpu.ebp);
    cpu.ebp = cpu.esp;
    cpu.edx = cpu.eax;
    cpu.eax <<= 2;
    cpu.eax -= cpu.edx;
    if (!(cpu.dl & 1))
        ++cpu.edx;
    cpu.ebx = cpu.edx;
    if (!(cpu.al & 1))
        ++cpu.eax;
    cpu.edx = cpu.ebx;
    cpu.edx *= cpu.eax;
    cpu.edx &= 0xffff;
    for (int i = 0; i < 7; ++i)
    {
        cpu.edx *= cpu.ebx;
        if (i < 6)
            cpu.edx &= 0xffff;
    }
    cpu.ecx = cpu.edx;
    cpu.ecx &= 0xffff;
    put(app, 0x554f14, cpu.edx);
    put(app, 0x554f10, cpu.ecx);
    put(app, 0x554f18, cpu.ebx);
    cpu.ebp = pop(app, cpu);
    cpu.edx = pop(app, cpu);
    cpu.ecx = pop(app, cpu);
    cpu.ebx = pop(app, cpu);
    cpu.esp += 4;
}

}  // namespace collide
}  // namespace nfs3hp
