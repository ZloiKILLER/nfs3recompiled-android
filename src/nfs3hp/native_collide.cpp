#include "game_settings.h"
#include "native_aidrive.h"
#include "native_collide.h"
#include "native_controls_input.h"
#include "native_controls_queue.h"
#include "native_ea_files.h"
#include "native_ea_memory.h"
#include "native_graphics.h"
#include "native_guest.h"
#include "native_raceload.h"
#include "native_racestart.h"
#include "native_track.h"
#include "native_trackdata.h"
#include "native_vertices.h"

#include <cstdint>
#include <cstring>
#include <optional>

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
 * Each is a typed core -- named parameters, a result, C++ locals -- with an
 * adapter under the game's name for the dispatch and the callers still on
 * registers, which leaves eax, ebx, ecx, edx and the x87's stack as the
 * game's code does (a fcomp's status word in ax where it shows).  The x87's
 * arithmetic goes through fp::add, sub, mul and div in the code's order, with
 * its operands in the order the code has them, each float the code stores
 * rounded there.  Other typed files' functions are called as C++
 * (guest::direct, their headers) but where what a callee leaves in the
 * registers, which its core does not give back, is what a function here
 * leaves or hands on; every other function of the game's, and the
 * renderer's through the pointers the game keeps, is called by address, its
 * arguments in the registers and on the guest's stack.  What a callee is
 * handed the address of stays in the guest's memory, on the guest's stack as deep as
 * the code has it (GuestFrame), so that the callees see the same addresses;
 * a core calling a core makes room for the return address's slot as the
 * call does.  The game's code runs with the direction flag clear: its movsd
 * are plain copies here. */
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
using i32 = std::int32_t;

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
// A car's fields as the collisions read them.
constexpr reg32 kCarPosition = 0x98, kCarForward = 0xc0, kCarRight = 0xd8, kCarHalfLength = 0x114;
constexpr reg32 kCarHalfWidth = 0x11c, kCarRadius = 0x120, kCarFlags = 0x200;
// A car's fields the walls and the objects read and set: its track position, its speed,
// how near it is tested, the collision's sound (its loudness, kind, position).
constexpr reg32 kCarPlace = 0xc, kCarSpeedSize = 0xb8, kCarObjectReach = 0x15c;
constexpr reg32 kCarSoundLoudness = 0x168, kCarSoundExtra = 0x16c, kCarSoundKind = 0x170, kCarSoundAt = 0x178;
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

// What is called by address though another typed file's, for what it leaves
// in the registers beyond what its core gives back: the boxes' tests (the edx
// the last leaves is findContact's), the cars' response (its eax and edx are
// collideCars'), a vector normalized where its eax is againstWalls', vectors
// added where they are pushApart's last call, the line between the cars
// (vectors less another: its ebx and ecx are pushed into the tests' frames).
constexpr reg32 kFaceInFirst = 0x42dd60, kFaceInSecond = 0x42e010;
constexpr reg32 kEdgeInFirst = 0x42e2c0, kEdgeInSecond = 0x42e4f0;
constexpr reg32 kCarsRespond = 0x42f280;
constexpr reg32 kNormalize = 0x4972f0, kAddVectors = 0x4dffb0;
constexpr reg32 kSubtractVectors = 0x4e0000;

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

/* A float from its bits. */
Float floatAt(reg32 bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return Float(f);
}

/* What a callee left on the x87's stack, taken off it as the generated code
 * does: st(0) read, the stack one shorter. */
Float popTop(CPU& cpu)
{
    const Float value = cpu.fpu.st(0);
    cpu.fpu.count -= 1;
    return value;
}

/* fcom's answers as the code's jumps read them: "below" (C0) for less or
 * unordered, "above" for neither C0 nor C3 -- greater, never a NaN. */
bool fBelow(const Float& a, const Float& b)
{
    return (fp::compare(a, b) & 0x100) != 0;
}

bool fAbove(const Float& a, const Float& b)
{
    return fp::compare(a, b) == 0;
}

/* fcom where the code reads the status word after (fnstsw ax): the x87's
 * status word as it leaves it, which the caller puts into ax. */
constexpr reg16 kC0 = 0x100, kC3 = 0x4000;

reg16 fcom(CPU& cpu, const Float& a, const Float& b)
{
    cpu.fpu.compare(a, b);
    return cpu.fpu.status.word;
}

/* A float's size either way, as the collisions work it out: its bits when 0
 * is below it (a NaN too, fcom's below), else it negated. */
float positive(float value)
{
    return fBelow(0.0, Float(value)) ? value : float(-Float(value));
}

/* The bits of a float. */
reg32 bitsOf(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* The dot product (vertices', 0x4e01f0) of the vectors at a and b, as the
 * x87 holds it. */
std::optional<Float> dot(App* app, CPU& cpu, reg32 a, reg32 b)
{
    const double product = guest::direct<0x4e01f0, vertices::dotProduct>(app, cpu, a, b);
    if (cpu.terminate)
        return std::nullopt;
    return Float(product);
}

/* The registers a function of the game's pushes on entry, written where it
 * pushes them -- below esp, where only what runs later reads them.  For the
 * car-to-car functions: raceload::carsCollide, which collideCars calls after
 * them, reads its cross product's vector before the check's stand-in for the
 * cross product (0x4e0160, which writes nothing) sets it -- so in the check
 * it reads the words pushCarsApart and findContact pushed there. */
struct Pushed
{
    Pushed(std::initializer_list<reg32> values) : values(values) {}
    std::initializer_list<reg32> values;
    void write(App* app, const CPU& cpu) const
    {
        reg32 to = cpu.esp;
        for (const reg32 value : values)
            put(app, to -= 4, value);
    }
};

/* Room on the guest's stack for what a callee is handed the address of: the
 * game's frame (`pushed` bytes of registers under esp on entry, then
 * `bytes`), esp under it while the core runs and back after. */
class GuestFrame
{
public:
    GuestFrame(CPU& cpu, reg32 pushed, reg32 bytes) : cpu(cpu), esp(cpu.esp), base(cpu.esp - pushed)
    {
        cpu.esp = base - bytes;
    }
    ~GuestFrame() { cpu.esp = esp; }
    reg32 operator-(reg32 offset) const { return base - offset; }

private:
    CPU& cpu;
    const reg32 esp;
    const reg32 base;
};

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

/* Whether, and how, the boxes of cars a and b touch -- the first's six
 * faces' centres and then its eight corners are tried, each point's signs in
 * ebx, ecx and on the stack (the tests are stdcall, ret 4): 1 a face's
 * centre in the first car's box, 2 in the second's, 3 a corner in the
 * first's, 4 in the second's; 0 none.  The registers the code pushes (ebx,
 * ecx, esi, edi, ebp, `pushed`) written where it pushes them: see Pushed. */
std::optional<reg32> contactKind(App* app, CPU& cpu, reg32 a, reg32 b, const Pushed& pushed)
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
    pushed.write(app, cpu);
    const GuestFrame frame(cpu, 0x14, 0);  // as deep as the code's
    for (const Probe& probe : probes)
    {
        cpu.eax = a;
        cpu.edx = b;
        cpu.ebx = probe.ebx;
        cpu.ecx = probe.ecx;
        if (!guest::call(app, cpu, probe.test, {probe.pushed}))
            return std::nullopt;
        if (cpu.eax != 0)
            return probe.kind;
    }
    return 0;
}

/* 0x42f550: contactKind of car eax and car edx; edx as the last test leaves
 * it. */
void findContact(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx;
    const std::optional<reg32> kind =
        contactKind(app, cpu, cpu.eax, cpu.edx, {cpu.ebx, cpu.ecx, cpu.esi, cpu.edi, cpu.ebp});
    if (!kind)
        return;
    cpu.eax = *kind;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.esp += 4;
}

namespace
{

/* |a . b|, as contactNormal works it out: the dot product once to see its
 * sign, again for the value, turned round if 0 is not below the first, as a
 * float. */
std::optional<float> absoluteDot(App* app, CPU& cpu, reg32 a, reg32 b)
{
    const std::optional<Float> first = dot(app, cpu, a, b);
    if (!first)
        return std::nullopt;
    const bool zeroBelow = fBelow(0.0, *first);
    const std::optional<Float> again = dot(app, cpu, a, b);
    if (!again)
        return std::nullopt;
    return float(zeroBelow ? *again : -*again);
}

/* A vector of three floats in the guest's memory turned round: each word's
 * sign flipped, its top byte at a time, in the order the code writes them. */
void turnRound(App* app, reg32 vector, std::initializer_list<reg32> order)
{
    for (const reg32 word : order)
        putByte(app, vector + 4 * word + 3, byte(app, vector + 4 * word + 3) ^ 0x80);
}

/* What contactNormal's code leaves in eax, ebx and edx. */
struct NormalLeft
{
    reg32 eax, ebx, edx;
};

// A car's speed (three floats), as contactNormal reads it; its axes.
constexpr reg32 kCarSpeed = 0xa4, kCarUp = 0xcc;

/* The normal of the contact findContact found between cars a and b, into
 * kContactNormal, in the frame of `car` -- b, or a for `second` 0 -- from
 * the speeds of `mover` (the other): when they move fast apart (the largest
 * of their speeds' differences, each a float, above 15), the axis of `car`
 * along which the mover's speed goes most (each axis turned round first by
 * its corner offset's sign, in the frame, where the dot products are handed
 * it); else the axis of `car` whose box the contact point is least inside,
 * turned round by its offset's sign.  An axis is turned for `second` where 0
 * is below the offset (a NaN too), else where 0 is above it.  `ebx` the
 * caller's, for what the code leaves in it. */
std::optional<NormalLeft> normalOfContact(App* app, CPU& cpu, reg32 a, reg32 b, reg32 second, reg32 ebx)
{
    const bool flag = second != 0;
    const reg32 mover = flag ? a : b, car = flag ? b : a;
    float differences[3];
    for (reg32 i = 0; i < 3; ++i)
        differences[i] = positive(
            float(fp::sub(loadFloat(app, mover + kCarSpeed + 4 * i), loadFloat(app, car + kCarSpeed + 4 * i))));
    float largest = differences[0];
    if (fAbove(differences[1], largest))
        largest = differences[1];
    if (fAbove(differences[2], largest))
        largest = differences[2];
    const auto turn = [&](reg32 offset) {
        const Float o = loadFloat(app, offset);
        return flag ? fBelow(0.0, o) : fAbove(0.0, o);
    };
    const auto copy = [&](reg32 to, reg32 from) {
        for (reg32 i = 0; i < 12; i += 4)
            put(app, to + i, at(app, from + i));
    };
    if (fAbove(largest, loadDouble(app, kFifteen)))
    {
        const GuestFrame frame(cpu, 0x10, 0x7c);
        const reg32 speed = frame - 0x7c, forward = frame - 0x64, up = frame - 0x58, right = frame - 0x70;
        copy(speed, mover + kCarSpeed);
        copy(forward, car + kCarForward);
        copy(up, car + kCarUp);
        copy(right, car + kCarRight);
        if (turn(kCornerX))
            turnRound(app, forward, {1, 0, 2});
        if (turn(kCornerY))
            turnRound(app, up, {0, 2, 1});
        if (turn(kCornerZ))
            turnRound(app, right, {1, 0, 2});
        const std::optional<float> alongForward = absoluteDot(app, cpu, forward, speed);
        if (!alongForward)
            return std::nullopt;
        const std::optional<float> alongUp = absoluteDot(app, cpu, up, speed);
        if (!alongUp)
            return std::nullopt;
        const std::optional<float> alongRight = absoluteDot(app, cpu, right, speed);
        if (!alongRight)
            return std::nullopt;
        reg32 chosen = right;
        if (fAbove(*alongForward, *alongUp) && fAbove(*alongForward, *alongRight))
            chosen = forward;
        else if (fAbove(*alongUp, *alongRight))
            chosen = up;
        copy(kContactNormal, chosen);
        // edx the speed's vector, as the code handed it to the last dot product.
        return NormalLeft{at(app, kContactNormal + 8), ebx, speed};
    }
    // How far inside each of car's half sizes the corner is.
    float inside[3];
    static const reg32 offsets[] = {kCornerX, kCornerY, kCornerZ};
    for (reg32 i = 0; i < 3; ++i)
    {
        const bool above = fAbove(0.0, loadFloat(app, offsets[i]));
        const Float size = loadFloat(app, car + kCarHalfLength + 4 * i);
        inside[i] = float(above ? fp::add(size, loadFloat(app, offsets[i])) : fp::sub(size, loadFloat(app, offsets[i])));
    }
    reg32 axis = 2;
    if (fBelow(inside[0], inside[1]) && fBelow(inside[0], inside[2]))
        axis = 0;
    else if (fBelow(inside[1], inside[2]))
        axis = 1;
    copy(kContactNormal, car + kCarForward + 0xc * axis);
    if (!turn(offsets[axis]))
        return NormalLeft{0, ebx, car};
    // The sign bytes go through bl, cl and bh; al, ch and ah; dl, bl and dh.
    const auto flipped = [&](reg32 word) { return reg32(byte(app, kContactNormal + 4 * word + 3) ^ 0x80); };
    if (axis == 0)
    {
        const reg32 low = flipped(0), high = flipped(1);
        turnRound(app, kContactNormal, {0, 2, 1});
        return NormalLeft{1, (ebx & 0xffff0000) | high << 8 | low, car};
    }
    if (axis == 1)
    {
        const reg32 low = flipped(1), high = flipped(2);
        turnRound(app, kContactNormal, {1, 0, 2});
        return NormalLeft{high << 8 | low, ebx, car};
    }
    const reg32 low = flipped(0), high = flipped(1), third = flipped(2);
    turnRound(app, kContactNormal, {0, 2, 1});
    return NormalLeft{1, (ebx & 0xffffff00) | third, (car & 0xffff0000) | high << 8 | low};
}

}  // namespace

/* 0x42f8c0: normalOfContact for car eax and car edx, by ebx. */
void contactNormal(App* app, CPU& cpu)
{
    const reg32 ecx = cpu.ecx;
    const std::optional<NormalLeft> n = normalOfContact(app, cpu, cpu.eax, cpu.edx, cpu.ebx, cpu.ebx);
    if (!n)
        return;
    cpu.eax = n->eax;
    cpu.ebx = n->ebx;
    cpu.edx = n->edx;
    cpu.ecx = ecx;
    cpu.esp += 4;
}


// A car's share of the masses, as the collisions weigh it.
constexpr reg32 kCarMassShare = 0xb0;

/* Cars a and b pushed apart along the direction `direction` (three floats'
 * bits, normalized first in the frame, 0x4972f0), each by a quarter of the
 * direction times the other's share of their masses -- the first back, the
 * second on (0x4e0050, 0x4dffb0).  The vectors are the frame's, as the code
 * hands them over; the registers it pushes (ebx, ecx, esi, ebp, `pushed`)
 * are written where it pushes them, and the length of `on` where it keeps
 * it: what the caller calls next (0x42f280) reads one of those words as a
 * local of its own it never sets.  False when the game stopped. */
bool pushApart(App* app, CPU& cpu, reg32 a, reg32 b, const reg32 (&direction)[3], const Pushed& pushed)
{
    pushed.write(app, cpu);
    const GuestFrame frame(cpu, 0x10, 0x30);
    const reg32 along = frame - 0x24, back = frame - 0x18, on = frame - 0x30;
    for (reg32 i = 0; i < 3; ++i)
        put(app, along + 4 * i, direction[i]);
    // Its length the code keeps on the x87's stack until an fstp st(0): never used.
    guest::direct<0x4972f0, vertices::normaliseVector>(app, cpu, along);
    if (cpu.terminate)
        return false;
    const Float sum = fp::add(loadFloat(app, a + kCarMassShare), loadFloat(app, b + kCarMassShare));
    const Float second = loadFloat(app, b + kCarMassShare);
    const Float reciprocal = fp::div(1.0, sum);
    const Float secondShare = fp::mul(second, reciprocal);
    const Float firstShare = fp::mul(reciprocal, loadFloat(app, a + kCarMassShare));
    const Float quarter = loadDouble(app, kQuarter);
    const Float backBy = fp::mul(secondShare, quarter);
    const float onBy = float(fp::mul(quarter, firstShare));
    put(app, frame - 0xc, bitsOf(onBy));
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, 1, along, float(-backBy), back);
    if (cpu.terminate)
        return false;
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, 1, along, onBy, on);
    if (cpu.terminate)
        return false;
    guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, a + kCarPosition, back, a + kCarPosition);
    if (cpu.terminate)
        return false;
    // By address: the registers it leaves are pushCarsApart's and separateCars'.
    cpu.eax = 1;
    cpu.edx = cpu.ecx = b + kCarPosition;
    cpu.ebx = on;
    return guest::call(app, cpu, kAddVectors);
}

/* 0x42fe10: pushApart for car eax and car edx along the direction on the
 * stack; 1, edx as the last call leaves it; ret 0xc. */
void pushCarsApart(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx;
    const reg32 direction[3] = {at(app, cpu.esp + 4), at(app, cpu.esp + 8), at(app, cpu.esp + 0xc)};
    if (!pushApart(app, cpu, cpu.eax, cpu.edx, direction, {cpu.ebx, cpu.ecx, cpu.esi, cpu.ebp}))
        return;
    cpu.eax = 1;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.esp += 4 + 0xc;
}

namespace
{

/* Two cars, a and b, whose boxes may overlap, taken apart: the frames
 * findContact's tests share worked out (kAxisDots, kSecondExtents,
 * kFirstExtents), then up to four times the line between the cars (in the
 * frame, where the calls are handed it) into either car's frame (kBetween),
 * contactKind, and for a contact the normal (for a corner) and the cars
 * pushed apart along the line.  pushApart's last result, or 0 for no
 * contact the first time. */
std::optional<reg32> separateTwo(App* app, CPU& cpu, reg32 a, reg32 b)
{
    const GuestFrame frame(cpu, 0x14, 0x1c);
    const reg32 line = frame - 0x1c;
    // The nine dot products of the first car's axes with the second's.
    for (reg32 row = 0; row < 3; ++row)
        for (reg32 column = 0; column < 3; ++column)
        {
            const std::optional<Float> d = dot(app, cpu, a + kCarForward + 0xc * row, b + kCarForward + 0xc * column);
            if (!d)
                return std::nullopt;
            storeFloat(app, kAxisDots + 0xc * row + 4 * column, *d);
        }
    // Those times the second car's half sizes (p) and the first's (q), in
    // the code's order, stored as the code's fxch and fstp leave them.
    const auto axisDot = [&](reg32 i) { return loadFloat(app, kAxisDots + 4 * i); };
    Float p[9], q[9];
    for (reg32 i = 0; i < 7; ++i)
        p[i] = fp::mul(axisDot(i), loadFloat(app, b + kCarHalfLength + 4 * (i % 3)));
    storeFloat(app, kSecondExtents + 4, p[1]);
    p[7] = fp::mul(axisDot(7), loadFloat(app, b + 0x118));
    storeFloat(app, kSecondExtents + 8, p[2]);
    p[8] = fp::mul(axisDot(8), loadFloat(app, b + 0x11c));
    storeFloat(app, kSecondExtents + 0xc, p[3]);
    for (reg32 i = 0; i < 6; ++i)
    {
        q[i] = fp::mul(axisDot(i), loadFloat(app, a + kCarHalfLength + 4 * (i / 3)));
        storeFloat(app, i < 5 ? kSecondExtents + 0x10 + 4 * i : kFirstExtents, i < 5 ? p[4 + i] : q[0]);
    }
    q[6] = fp::mul(axisDot(6), loadFloat(app, a + 0x11c));
    storeFloat(app, kFirstExtents + 4, q[1]);
    q[7] = fp::mul(axisDot(7), loadFloat(app, a + 0x11c));
    storeFloat(app, kFirstExtents + 8, q[2]);
    q[8] = fp::mul(axisDot(8), loadFloat(app, a + 0x11c));
    for (reg32 i = 3; i < 9; ++i)
        storeFloat(app, kFirstExtents + 4 * i, q[i]);
    storeFloat(app, kSecondExtents, p[0]);
    reg32 result = 0;
    for (reg32 pushes = 0;;)
    {
        // The line from the first car to the second, into either frame.
        cpu.eax = 1;
        cpu.edx = b + kCarPosition;
        cpu.ebx = a + kCarPosition;
        cpu.ecx = line;
        if (!guest::call(app, cpu, kSubtractVectors))
            return std::nullopt;
        static const reg32 axes[] = {0xc0, 0xcc, 0xd8};
        for (reg32 i = 0; i < 6; ++i)
        {
            const std::optional<Float> d = dot(app, cpu, (i < 3 ? a : b) + axes[i % 3], line);
            if (!d)
                return std::nullopt;
            storeFloat(app, kBetween + 4 * i, *d);
        }
        std::optional<reg32> kind;
        {
            const GuestFrame slot(cpu, 0, 4);  // the call's return address, as the code has it
            kind = contactKind(app, cpu, a, b, {cpu.ebx, cpu.ecx, a, b, frame - 0});
        }
        if (!kind)
            return std::nullopt;
        if (*kind == 0)
            return result;
        safepoint(app, cpu);
        if (*kind == 3 || *kind == 4)
        {
            const GuestFrame slot(cpu, 0, 4);
            if (!normalOfContact(app, cpu, a, b, *kind == 4 ? 1 : 0, cpu.ebx))
                return std::nullopt;
        }
        ++pushes;
        const reg32 direction[3] = {at(app, line), at(app, line + 4), at(app, line + 8)};
        {
            // Its three words as the code pushes them, and the return address's slot.
            Pushed{direction[2], direction[1], direction[0]}.write(app, cpu);
            const GuestFrame slot(cpu, 0, 4 + 0xc);
            if (!pushApart(app, cpu, a, b, direction, {pushes, direction[0], a, frame - 0}))
                return std::nullopt;
        }
        result = 1;
        if (sreg32(pushes) >= 4)
            return result;
    }
}

}  // namespace

/* 0x42fee0: separateTwo for car eax and car edx; edx as the last call leaves
 * it. */
void separateCars(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx;
    const std::optional<reg32> r = separateTwo(app, cpu, cpu.eax, cpu.edx);
    if (!r)
        return;
    cpu.eax = *r;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.esp += 4;
}

/* The fixed point's multiply, as the code makes it -- imul; shl edx, 16;
 * shr eax, 16; adc eax, edx -- of a by b, rounded by the bit shr shifts
 * out: the product, and edx as the code leaves it. */
struct Fixed
{
    reg32 value, high;
};

Fixed fixedTimes(reg32 a, reg32 b)
{
    const std::int64_t product = std::int64_t(sreg32(a)) * sreg32(b);
    const reg32 low = reg32(product);
    const reg32 high = reg32(std::uint64_t(product) >> 32) << 16;
    return {(low >> 16) + high + ((low >> 15) & 1), high};
}

/* What edgeNormal leaves in the registers: the normal's second and third
 * terms in 16.16 (eax and ebx), the last multiply's edx. */
struct EdgeNormal
{
    reg32 second, third, high;
};

// The corners of a track position's polygon, 16.16 (x, y, z each).
constexpr reg32 kPlaceCorners = 0x24;

/* The wall along edge `edge` (1, 2, 4 or 8) of the track position at
 * `place`, as the code works it out in 16.16: the edge's run and rise (from
 * the polygon's corners) and 0 between them, normalized in the frame
 * (0x4970c0), its cross product with up (0, 1, 0) -- each term a 16.16
 * multiply, the first by edx as the normalizing leaves it -- into floats at
 * `out` (the third term first).  Another edge: the frame's run and rise as
 * they were on the guest's stack. */
std::optional<EdgeNormal> edgeNormal(App* app, CPU& cpu, reg32 edge, reg32 place, reg32 out)
{
    const GuestFrame frame(cpu, 0x10, 0x28);
    const reg32 vector = frame - 0x10;
    put(app, vector + 4, 0);
    const auto corner = [&](reg32 offset) { return at(app, place + offset); };
    switch (edge)
    {
    case 1:
        put(app, vector + 8, corner(0x44) - corner(0x38));
        put(app, vector, corner(0x3c) - corner(0x30));
        break;
    case 2:
        put(app, vector + 8, corner(0x2c) - corner(0x50));
        put(app, vector, corner(0x24) - corner(0x48));
        break;
    case 4:
        put(app, vector + 8, corner(0x38) - corner(0x2c));
        put(app, vector, corner(0x30) - corner(0x24));
        break;
    case 8:
        put(app, vector + 8, corner(0x50) - corner(0x44));
        put(app, vector, corner(0x48) - corner(0x3c));
        break;
    default:
        break;
    }
    cpu.eax = vector;
    cpu.edx = 0;
    if (!guest::call(app, cpu, 0x4970c0))
        return std::nullopt;
    constexpr reg32 kOne = 0x10000;  // up's y; its x and z 0
    const reg32 x = at(app, vector), y = at(app, vector + 4), z = at(app, vector + 8);
    const reg32 first = fixedTimes(y, cpu.edx).value - fixedTimes(z, kOne).value;
    const reg32 second = fixedTimes(z, 0).value - fixedTimes(x, 0).value;
    const Fixed last = fixedTimes(y, 0);
    const reg32 third = fixedTimes(x, kOne).value - last.value;
    const Float scale = loadDouble(app, kFixedScale);
    const Float thirdScaled = fp::mul(Float(sreg32(third)), scale);
    const Float firstScaled = fp::mul(Float(sreg32(first)), scale);
    const Float secondScaled = fp::mul(scale, Float(sreg32(second)));
    storeFloat(app, out + 8, thirdScaled);
    storeFloat(app, out, firstScaled);
    storeFloat(app, out + 4, secondScaled);
    return EdgeNormal{second, third, last.high};
}

/* 0x430210: edgeNormal of edge eax of the track position at ebx into edx;
 * eax, ebx and edx as the code leaves them. */
void polygonEdgeNormal(App* app, CPU& cpu)
{
    const std::optional<EdgeNormal> n = edgeNormal(app, cpu, cpu.eax, cpu.ebx, cpu.edx);
    if (!n)
        return;
    cpu.eax = n->second;
    cpu.ebx = n->third;
    cpu.edx = n->high;
    cpu.esp += 4;
}

// A track position's record, as clearOfWall reads it: its polygon's index
// (+0x10), whether its corners are its own (+0x14, else the polygon's
// record's), its polygon (+0x18, 0 none), something of its own (+0x6c).
constexpr reg32 kPlacePolygon = 0x10, kPlaceOwnCorners = 0x14, kPlaceHasPolygon = 0x18, kPlaceThing = 0x6c;
constexpr reg32 kPolygonRecordBytes = 0x24;
constexpr reg32 kCarY = 0x9c, kCarStage = 0x164;

/* What clear leaves: its result, and edx as the code leaves it. */
struct Clear
{
    reg32 result, edx;
};

/* Whether car `car` is clear of the wall of the track position `place`: 1
 * for no polygon there, a polygon with something of its own, of kind 0 or
 * 14; else whether the car is below the corners' lowest (its four corners,
 * or the polygon record's first four times when the corners are not its
 * own) plus the margin -- -0.6, -2 for a car with a stage (+0x164).  The lowest is kept
 * as a float, as the code stores it, each against the next as doubles. */
std::optional<Clear> clear(App* app, CPU& cpu, reg32 car, reg32 place)
{
    const GuestFrame frame(cpu, 0x14, 0x30);  // as deep as the code's
    const reg32 kind = guest::direct<0x4207e0, trackdata::placeSurface>(app, cpu, place, 1) & 0x3f;
    if (cpu.terminate)
        return std::nullopt;
    const reg32 polygon = at(app, place + kPlaceHasPolygon);
    if (polygon == 0)
        return Clear{1, polygon};
    const reg32 corners = place + 0x54;
    if (corners == 0 || at(app, place + kPlaceThing) != 0 || kind == 0 || kind == 0xe)
        return Clear{1, corners};
    const reg32 carY = at(app, car + kCarY);
    float lowest = 0;
    for (reg32 i = 0; i < 4; ++i)
    {
        reg32 from;
        if (at(app, place + kPlaceOwnCorners) == 0)
        {
            safepoint(app, cpu);
            from = at(app, place + kPlacePolygon) * kPolygonRecordBytes + at(app, kPolygonRecords);
        }
        else
            from = i * 0xc + place + kPlaceCorners;
        safepoint(app, cpu);
        const Float height = fp::mul(Float(sreg32(at(app, from + 4))), loadDouble(app, kWallScale));
        if (i == 0 || fAbove(Float(lowest), height))
            lowest = float(height);
    }
    const reg32 margin = half(app, car + kCarStage) != 0 ? kWallMarginRace : kWallMarginOther;
    const Float top = fp::add(Float(lowest), loadDouble(app, margin));
    return Clear{fBelow(floatAt(carY), top) ? 1u : 0u, 4};
}

/* 0x430350: clear of car eax against the track position edx; edx as the
 * code leaves it. */
void clearOfWall(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx;
    const std::optional<Clear> c = clear(app, cpu, cpu.eax, cpu.edx);
    if (!c)
        return;
    cpu.eax = c->result;
    cpu.edx = c->edx;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.esp += 4;
}

// What carAgainstWalls calls: the car's turn applied to its axes (0x4a2cc0,
// eax the turn's out, the turn's three floats and a word's address on the
// stack; then 0x4e0280, the axes times it), the corners into the world
// (0x4e0430: eax of them at edx, by the axes at ebx, from ecx; the corners'
// address on the stack), a place on the track followed (0x41f6e0: from eax
// with the record at edx, ebx 1), the edges a place is past (0x41fd60), the
// quad an edge leads to (0x41f870), the wall's surface (0x42ee10: the car,
// its corner, the normal, the surface's corner), the scrape's sound
// (0x406160) and a jolt (0x4aa4f4).
// All but the turn as C++ (guest::direct) -- and but the jolt, whose eax
// againstWalls leaves.
constexpr reg32 kTurn = 0x4a2cc0;
constexpr reg32 kWallJolt = 0x4aa4f4;
constexpr reg32 kCarSlice = 0x1c, kCarTurn = 0xe4, kCarHalfHeight = 0x118;
constexpr reg32 kPlaceWords = 0x1e;  // a track position's record, copied whole

namespace
{

/* Car `car` against the track's walls: its box's corners -- four at its
 * middle's height without a stage, eight else -- into the world, each moved
 * on by a thirty-second of the car's speed (and its turn), put into 16.16
 * and followed on the track; the first corner past a wall (clear 0) of its
 * polygon or of the neighbours the edges it crossed lead to gives the walls'
 * normals' mean (flat), or the line back to the car, normalized; then the
 * wall's surface, the sound's kind (the polygon's byte 0xe, a half by the
 * side of the wall the car is on), the scrape's sound and a jolt by the
 * car's flags.  Everything the calls are handed is the frame's.  What the
 * code leaves in eax. */
std::optional<reg32> againstWalls(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x18, 0x23c);
    const reg32 f = frame - 0x82;
    const reg32 normal = f + 0x12, step = f - 0x12, axes = f - 0x6a, turned = f - 0x46, turnWord = f + 0x5a;
    const reg32 edgesOut = f + 0x5e, corners = f - 0xca, moved = f + 0x42, fixed = f + 0x1e;
    const reg32 place = f - 0x1ba, neighbour = f - 0x142, edgeOut = f + 0x4e, surface = f - 6;
    const reg32 polygonNormal = f + 6, toCar = f + 0x36;
    const auto copyWords = [&](reg32 to, reg32 from, reg32 words) {
        for (reg32 w = 0; w < words; ++w)
            put(app, to + 4 * w, at(app, from + 4 * w));
    };
    copyWords(normal, kZeroVector, 3);
    copyWords(step, kZeroVector2, 3);
    put(app, edgesOut, 0);
    copyWords(axes, car + kCarForward, 9);
    put(app, turnWord, 0);
    // A thirty-second of the speed, and of the turn onto the stack.
    {
        const Float by = loadFloat(app, kSpeedStep);
        storeFloat(app, step, fp::mul(loadFloat(app, car + kCarSpeed), by));
        storeFloat(app, step + 4, fp::mul(loadFloat(app, car + kCarSpeed + 4), by));
        storeFloat(app, step + 8, fp::mul(loadFloat(app, car + kCarSpeed + 8), by));
        const float turnZ = float(fp::mul(loadFloat(app, car + kCarTurn + 8), by));
        const float turnY = float(fp::mul(loadFloat(app, car + kCarTurn + 4), by));
        const float turnX = float(fp::mul(by, loadFloat(app, car + kCarTurn)));
        cpu.eax = turned;
        if (!guest::call(app, cpu, kTurn, {bitsOf(turnX), bitsOf(turnY), bitsOf(turnZ), turnWord}))
            return std::nullopt;
    }
    if (cpu.eax != 0)
    {
        guest::direct<0x4e0280, vertices::multiplyMatrix>(app, cpu, axes, turned, axes);
        if (cpu.terminate)
            return std::nullopt;
    }
    // The box's corners in the car's frame, in the code's order.
    const auto copy = [&](reg32 to, reg32 from) { put(app, to, at(app, car + from)); };
    const auto negated = [&](reg32 to, reg32 from) { storeFloat(app, to, -loadFloat(app, car + from)); };
    const reg32 c = corners;
    reg32 count;
    if (half(app, car + kCarStage) == 0)
    {
        put(app, c + 4, 0);
        negated(c, kCarHalfLength);
        copy(c + 8, kCarHalfWidth);
        copy(c + 0xc, kCarHalfLength);
        put(app, c + 0x10, 0);
        copy(c + 0x14, kCarHalfWidth);
        copy(c + 0x18, kCarHalfLength);
        put(app, c + 0x1c, 0);
        count = 4;
        negated(c + 0x20, kCarHalfWidth);
        put(app, c + 0x28, 0);
        negated(c + 0x24, kCarHalfLength);
        negated(c + 0x2c, kCarHalfWidth);
    }
    else
    {
        negated(c, kCarHalfLength);
        copy(c + 4, kCarHalfHeight);
        copy(c + 8, kCarHalfWidth);
        copy(c + 0xc, kCarHalfLength);
        copy(c + 0x10, kCarHalfHeight);
        copy(c + 0x14, kCarHalfWidth);
        copy(c + 0x18, kCarHalfLength);
        copy(c + 0x1c, kCarHalfHeight);
        negated(c + 0x20, kCarHalfWidth);
        negated(c + 0x24, kCarHalfLength);
        copy(c + 0x28, kCarHalfHeight);
        negated(c + 0x2c, kCarHalfWidth);
        negated(c + 0x30, kCarHalfLength);
        negated(c + 0x34, kCarHalfHeight);
        copy(c + 0x38, kCarHalfWidth);
        copy(c + 0x3c, kCarHalfLength);
        negated(c + 0x40, kCarHalfHeight);
        copy(c + 0x44, kCarHalfWidth);
        copy(c + 0x48, kCarHalfLength);
        count = 8;
        negated(c + 0x4c, kCarHalfHeight);
        negated(c + 0x50, kCarHalfWidth);
        negated(c + 0x54, kCarHalfLength);
        negated(c + 0x58, kCarHalfHeight);
        negated(c + 0x5c, kCarHalfWidth);
    }
    // Into the world, by the car's axes and from its position.
    guest::direct<0x4e0430, vertices::transformPointList>(app, cpu, i32(count), corners, axes, car + kCarPosition,
                                                          corners);
    if (cpu.terminate)
        return std::nullopt;
    for (reg32 i = 0;; ++i)
    {
        safepoint(app, cpu);
        if (sreg32(i) >= sreg32(count))
            return i;
        // The corner moved on by the speed's step, in 16.16, followed from
        // the car's track position (copied).
        const reg32 corner = corners + i * 0xc;
        copyWords(place, car + kCarPlace, kPlaceWords);
        copyWords(moved, corner, 3);
        guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, moved, step, moved);
        if (cpu.terminate)
            return std::nullopt;
        const Float toFixed = loadDouble(app, kToFixed);
        put(app, fixed, reg32(fp::toInteger<sreg32>(fp::mul(loadFloat(app, moved), toFixed))));
        const Float y = fp::mul(loadFloat(app, moved + 4), toFixed);
        reg32 crossedWalls = 0;
        put(app, fixed + 4, reg32(fp::toInteger<sreg32>(y)));
        put(app, fixed + 8, reg32(fp::toInteger<sreg32>(fp::mul(loadFloat(app, moved + 8), toFixed))));
        copyWords(place + 4, fixed, 3);
        if (!guest::direct<0x41f6e0, trackdata::bringPlace>(app, cpu, fixed, place, 1))
            return std::nullopt;
        const auto clearAt = [&](reg32 at) -> std::optional<reg32> {
            const GuestFrame slot(cpu, 0, 4);  // the call's return address, as the code has it
            const std::optional<Clear> r = clear(app, cpu, car, at);
            if (!r)
                return std::nullopt;
            return r->result;
        };
        const std::optional<reg32> clearHere = clearAt(place);
        if (!clearHere)
            return std::nullopt;
        if (*clearHere != 0)
        {
            // Clear of its own polygon's walls: the edges it crossed lead to
            // neighbours, looked up and tested in turn.
            copyWords(neighbour, place, kPlaceWords);
            const reg32 crossed =
                guest::direct<0x41fd60, trackdata::edgesPast>(app, cpu, fixed, car + kCarPlace, place, edgesOut).flags;
            if (cpu.terminate)
                return std::nullopt;
            const reg32 edges[4] = {at(app, kEdgeBits), at(app, kEdgeBits + 4), at(app, kEdgeBits + 8),
                                    at(app, kEdgeBits + 0xc)};
            put(app, normal + 8, 0);
            put(app, normal + 4, 0);
            put(app, normal, 0);
            if (crossed != 0)
            {
                float walls = 0;
                for (reg32 e = 0;; ++e)
                {
                    if (e != 0)
                    {
                        safepoint(app, cpu);
                        if (e >= 4)
                            break;
                    }
                    if ((crossed & edges[e]) == 0)
                        continue;
                    const reg32 leads =
                        guest::direct<0x41f870, trackdata::matchQuad>(app, cpu, edges[e], car + kCarPlace, neighbour).found;
                    if (cpu.terminate)
                        return std::nullopt;
                    if (leads != 0)
                    {
                        const std::optional<reg32> clearThere = clearAt(neighbour);
                        if (!clearThere)
                            return std::nullopt;
                        if (*clearThere == 0)
                            continue;
                    }
                    {
                        const GuestFrame slot(cpu, 0, 4);
                        if (!edgeNormal(app, cpu, edges[e], car + kCarPlace, edgeOut))
                            return std::nullopt;
                    }
                    // The count and the sum of the normals.
                    const Float x = loadFloat(app, normal), y2 = loadFloat(app, normal + 4);
                    const Float z = loadFloat(app, normal + 8);
                    const Float counted = fp::add(1.0, Float(walls));
                    const Float sumX = fp::add(x, loadFloat(app, edgeOut));
                    const Float sumY = fp::add(y2, loadFloat(app, edgeOut + 4));
                    const Float sumZ = fp::add(z, loadFloat(app, edgeOut + 8));
                    crossedWalls |= edges[e];
                    walls = float(counted);
                    storeFloat(app, normal, sumX);
                    storeFloat(app, normal + 4, sumY);
                    storeFloat(app, normal + 8, sumZ);
                }
                if ((bitsOf(walls) & 0x7fffffff) != 0)
                {
                    // Their mean, flat (y 0).
                    const Float x = loadFloat(app, normal);
                    const Float reciprocal = fp::div(1.0, Float(walls));
                    const Float meanX = fp::mul(x, reciprocal);
                    const Float meanY = fp::mul(loadFloat(app, normal + 4), reciprocal);
                    const Float meanZ = fp::mul(reciprocal, loadFloat(app, normal + 8));
                    storeFloat(app, normal, meanX);
                    storeFloat(app, normal + 4, meanY);
                    storeFloat(app, normal + 8, meanZ);
                    put(app, normal + 4, 0);
                }
                else
                {
                    // No wall's normal: back towards the car.
                    storeFloat(app, normal, fp::sub(loadFloat(app, car + kCarPosition), loadFloat(app, moved)));
                    storeFloat(app, normal + 4, fp::sub(loadFloat(app, car + kCarY), loadFloat(app, moved + 4)));
                    storeFloat(app, normal + 8, fp::sub(loadFloat(app, car + kCarPosition + 8), loadFloat(app, moved + 8)));
                    crossedWalls = crossed;
                }
            }
        }
        if (crossedWalls == 0)
            continue;
        // By address: the eax it leaves is againstWalls' where the length is 0.
        cpu.eax = normal;
        if (!guest::call(app, cpu, kNormalize))
            return std::nullopt;
        const reg16 word = fcom(cpu, 0.0, cpu.fpu.st(0));
        popTop(cpu);
        if (word & kC3)
            return (cpu.eax & 0xffff0000) | word;
        // The wall's corner by the edges' bits: its polygon's, scaled, for
        // the surface.
        copyWords(surface, corner, 3);
        const reg8 bits = reg8(crossedWalls);
        const reg32 wallCorner = (bits & 4)   ? ((bits & 1) ? 0x3c : 0x30)
                                 : (bits & 8) ? ((bits & 1) ? 0x48 : 0x54)
                                              : ((bits & 1) ? 0x3c : 0x30);
        {
            const Float scale = loadDouble(app, kPolygonScale);
            storeFloat(app, surface, fp::mul(Float(sreg32(at(app, car + wallCorner))), scale));
            storeFloat(app, surface + 4, fp::mul(Float(sreg32(at(app, car + wallCorner + 4))), scale));
            storeFloat(app, surface + 8, fp::mul(scale, Float(sreg32(at(app, car + wallCorner + 8)))));
        }
        const std::optional<bool> hitIt = guest::direct<0x42ee10, raceload::hit>(app, cpu, car, corner, normal, surface);
        if (!hitIt)
            return std::nullopt;
        if (!*hitIt)
            continue;
        // The car's polygon's centre and normal (its record's bytes 0x18 to
        // 0x1a, signed, times 0x200), against the line to the car.
        const reg32 records = at(app, kPolygonRecords);
        const auto record = [&] { return records + at(app, car + kCarSlice) * kPolygonRecordBytes; };
        {
            const Float scale = loadDouble(app, kPolygonScale);
            const Float x = fp::mul(Float(sreg32(at(app, record()))), scale);
            const Float y3 = fp::mul(Float(sreg32(at(app, record() + 4))), scale);
            const Float z = fp::mul(Float(sreg32(at(app, record() + 8))), scale);
            const float centreX = float(x), centreY = float(y3), centreZ = float(z);
            storeFloat(app, toCar, fp::sub(loadFloat(app, car + kCarPosition), Float(centreX)));
            storeFloat(app, toCar + 4, fp::sub(loadFloat(app, car + kCarY), Float(centreY)));
            storeFloat(app, toCar + 8, fp::sub(loadFloat(app, car + kCarPosition + 8), Float(centreZ)));
            for (reg32 k = 0; k < 3; ++k)
            {
                const sreg32 n = sreg32(reg32(sreg32(at(app, record() + 0x15 + k)) >> 24) << 9);
                storeFloat(app, polygonNormal + 4 * k, k < 2 ? fp::mul(Float(n), scale) : fp::mul(scale, Float(n)));
            }
        }
        const std::optional<Float> side = dot(app, cpu, polygonNormal, toCar);
        if (!side)
            return std::nullopt;
        const float sideKept = float(*side);
        const reg8 kinds = byte(app, at(app, kPolygonRecords) + at(app, car + kCarSlice) * kPolygonRecordBytes + 0xe);
        const reg32 kind = fAbove(0.0, Float(sideKept)) ? (kinds >> 4) & 0xf : kinds & 0xf;
        put(app, car + kCarSoundKind, kind | 0x40000);
        cpu.eax = kind | 0x40000;
        if ((byte(app, car + kCarFlags) & 2) && !guest::direct<0x406160, aidrive::reverseIfTipped>(app, cpu, car, normal))
            return std::nullopt;
        cpu.eax = car;
        if ((byte(app, car + kCarFlags) & 4) && !guest::call(app, cpu, kWallJolt))
            return std::nullopt;
        return cpu.eax;
    }
}

}  // namespace

/* 0x4304b0: againstWalls for car eax; every other register as it was. */
void carAgainstWalls(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
    const std::optional<reg32> r = againstWalls(app, cpu, cpu.eax);
    if (!r)
        return;
    cpu.eax = *r;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esp += 4;
}

namespace
{

/* What collide leaves: whether the cars touched at all, and separateCars'
 * last result. */
struct Collided
{
    bool touched;
    reg32 last;
};

/* Cars a and b collided: taken apart (separateCars) up to 33 times while
 * they still touch, and if they touched at all their response worked out
 * at the contact's point (0x42f280, the cars in eax and edx, the point in
 * ebx). */
std::optional<Collided> collide(App* app, CPU& cpu, reg32 a, reg32 b)
{
    const GuestFrame frame(cpu, 0x14, 0);  // separateCars' frame is handed to its callees
    Collided c{false, 0};
    for (int tries = 0; tries < 33; ++tries)
    {
        safepoint(app, cpu);
        std::optional<reg32> r;
        {
            const GuestFrame slot(cpu, 0, 4);  // the call's return address, as the code has it
            r = separateTwo(app, cpu, a, b);
        }
        if (!r)
            return std::nullopt;
        c.last = *r;
        if (c.last == 0)
            break;
        c.touched = true;
    }
    if (c.touched)
    {
        cpu.eax = a;
        cpu.edx = b;
        cpu.ebx = kContactPoint;
        if (!guest::call(app, cpu, kCarsRespond))
            return std::nullopt;
    }
    return c;
}

}  // namespace

/* 0x430c80: collide for car eax and car edx; eax and edx the last result
 * when they never touched, else as the response leaves them. */
void collideCars(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx;
    const std::optional<Collided> c = collide(app, cpu, cpu.eax, cpu.edx);
    if (!c)
        return;
    if (!c->touched)
        cpu.eax = cpu.edx = c->last;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.esp += 4;
}

/* 0x430cd0: no car tested yet this frame. */
void clearCarList(App* app, CPU& cpu)
{
    put(app, kCarCount, 0);
    cpu.esp += 4;
}

namespace
{

}  // namespace

/* Whether the point at `point` (x and z; y taken as the car's) is within
 * `reach` of car `car`'s box along its forward and right axes: the point less
 * the car's position into the vector at `offset` (y 0), then 1, or 0. */
std::optional<reg32> inReach(App* app, CPU& cpu, reg32 car, reg32 point, reg32 reachBits, reg32 offset)
{
    const GuestFrame frame(cpu, 0xc, 0x10);  // as deep as the code's
    storeFloat(app, offset, fp::sub(loadFloat(app, point), loadFloat(app, car + kCarPosition)));
    storeFloat(app, offset + 8, fp::sub(loadFloat(app, point + 8), loadFloat(app, car + kCarPosition + 8)));
    put(app, offset + 4, 0);
    for (const auto& axis : {std::pair<reg32, reg32>{kCarForward, kCarHalfLength}, {kCarRight, kCarHalfWidth}})
    {
        const std::optional<Float> along = dot(app, cpu, car + axis.first, offset);
        if (!along)
            return std::nullopt;
        cpu.edx = offset;  // as the code's call left it: the offset it handed in edx
        const float size = positive(float(*along));
        if (fBelow(fp::add(loadFloat(app, car + axis.second), floatAt(reachBits)), Float(size)))
            return 0;
    }
    return 1;
}

/* 0x430ce0: inReach for car eax, the point at edx, the reach and the
 * offset's vector on the stack; ret 8.  edx as the last call leaves it. */
void objectInReach(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx;
    const std::optional<reg32> r =
        inReach(app, cpu, cpu.eax, cpu.edx, at(app, cpu.esp + 4), at(app, cpu.esp + 8));
    if (!r)
        return;
    cpu.eax = *r;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.esp += 4 + 8;
}

/* Car `car` added to this frame's list, and tested against those before it:
 * by their radii along y, x and z (each difference and the radii's sum
 * floats, as the code stores them), collideCars for those near.  What the
 * code leaves in eax: the count at first, then a difference's bits kept as
 * it was, or collideCars' result. */
std::optional<reg32> addToList(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x10, 0x1c);  // as deep as the code's, for collide's callees
    reg32 eax = at(app, kCarCount);
    put(app, eax * 4 + kCarList, car);
    for (reg32 i = 0;; ++i)
    {
        safepoint(app, cpu);
        if (sreg32(i) >= sreg32(at(app, kCarCount)))
            break;
        const reg32 other = at(app, i * 4 + kCarList);
        const float radii = float(fp::add(loadFloat(app, car + kCarRadius), loadFloat(app, other + kCarRadius)));
        bool near = true;
        for (const reg32 along : {0xa0u, 0x98u, 0x9cu})
        {
            const float d = float(fp::sub(loadFloat(app, car + along), loadFloat(app, other + along)));
            // Each fcomp's status word into ax, as fnstsw leaves it.
            const bool zeroBelow = (fcom(cpu, 0.0, Float(d)) & kC0) != 0;
            if (zeroBelow)
                eax = bitsOf(d);
            const float size = zeroBelow ? d : float(-Float(d));
            const reg16 word = fcom(cpu, Float(size), Float(radii));
            eax = (eax & 0xffff0000) | word;
            if (!(word & kC0))
            {
                near = false;
                break;
            }
        }
        if (!near)
            continue;
        const GuestFrame slot(cpu, 0, 4);  // the call's return address, as the code has it
        if (!collide(app, cpu, car, other))
            return std::nullopt;
        eax = cpu.eax;
    }
    put(app, kCarCount, at(app, kCarCount) + 1);
    return eax;
}

// The track's objects near a car: their count (eax the car's track
// position), each (eax the position, edx its index, its position in 16.16
// to ebx, its size's word to ecx), how one is hit (eax the car, edx the
// index: 1 pushed back, 2 a small one knocked, below 0 something else), the
// jolt (eax the car, edx how, ecx and ebx the offset's z and x in 16.16).
// (0x41ff10, 0x41ff20, 0x41d350; the bump's sound, eax the car, 0x406120 --
// called as C++, guest::direct.)  The jolt by address: its core pushes the
// registers its caller holds into its frame -- ecx, the offset's z, no
// argument of its.  The response by address too: the car eax pushed by the
// vector edx at ebx, stdcall, a 0 pushed -- its frame, whose addresses it
// hands on, lies under that word, where a call as C++ would not put it.
constexpr reg32 kObjectJolt = 0x471440, kObjectResponse = 0x42efe0;
constexpr reg32 kSoundBumped = 0x6000a, kSoundKnocked = 0x6000b, kSoundLoud = 0x41700000;  // 15.0

/* Car `car` against the track's objects near it: those close by a rough
 * distance (the larger of the x and z differences and a quarter of the
 * smaller, each a float), twice as big as the car's reach and within its
 * box's (inReach) are hit -- the jolt; for 1 the car pushed out along the
 * line from the object, by its size, its response and the sound's halved,
 * the bump's sound for the car's flag 2; for 2 above a speed of 2, and below
 * 0, the sound's.  The vectors the calls are handed are the frame's.  What
 * the code leaves in eax: the objects' count, at least 0. */
std::optional<reg32> againstObjects(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x18, 0x6c);
    const reg32 size = frame - 4, position = frame - 0x48, point = frame - 0x3c, offset = frame - 0x54;
    const reg32 away = frame - 0x6c, scaled = frame - 0x60;
    const reg32 count = guest::direct<0x41ff10, trackdata::itemCount>(app, cpu, car + kCarPlace);
    if (cpu.terminate)
        return std::nullopt;
    for (reg32 i = 0;; ++i)
    {
        safepoint(app, cpu);
        if (sreg32(i) >= sreg32(count))
            return i;
        guest::direct<0x41ff20, trackdata::takeItem>(app, cpu, car + kCarPlace, i, position, size);
        if (cpu.terminate)
            return std::nullopt;
        // Its size (its word, times 0x100) and position, as floats.
        const sreg32 sizeFixed = sreg32(reg32(sreg32(at(app, size - 2)) >> 16) << 8);
        const Float scale = loadDouble(app, kObjectScale);
        const Float sizeScaled = fp::mul(Float(sizeFixed), scale);
        const Float x = fp::mul(Float(sreg32(at(app, position))), scale);
        const Float y = fp::mul(Float(sreg32(at(app, position + 4))), scale);
        const Float z = fp::mul(scale, Float(sreg32(at(app, position + 8))));
        storeFloat(app, point, x);
        storeFloat(app, point + 4, y);
        storeFloat(app, point + 8, z);
        const float objectSize = float(sizeScaled);
        const float dx = positive(float(fp::sub(x, loadFloat(app, car + kCarPosition))));
        const float dz = positive(float(fp::sub(loadFloat(app, point + 8), loadFloat(app, car + kCarPosition + 8))));
        // The rough distance: the larger and a quarter of the smaller.
        const bool xLarger = fAbove(dx, dz);
        const float rough =
            float(fp::add(fp::mul(xLarger ? dz : dx, loadFloat(app, kObjectQuarter)), xLarger ? dx : dz));
        if (!fAbove(fp::add(objectSize, loadFloat(app, car + kCarRadius)), rough))
            continue;
        if (!fAbove(fp::mul(objectSize, loadFloat(app, kObjectTwo)), loadFloat(app, car + kCarObjectReach)))
            continue;
        std::optional<reg32> within;
        {
            const GuestFrame slot(cpu, 0, 4 + 8);  // its two words and the return address
            within = inReach(app, cpu, car, point, bitsOf(objectSize), offset);
        }
        if (!within)
            return std::nullopt;
        if (*within == 0)
            continue;
        const std::optional<reg32> moving = guest::direct<0x41d350, track::objectMoving>(app, cpu, car, i);
        if (!moving)
            return std::nullopt;
        const reg32 how = *moving;
        // The jolt, by the offset's z and x in 16.16.
        const Float toFixed = loadDouble(app, kObjectToFixed);
        const reg32 joltZ = reg32(fp::toInteger<sreg32>(fp::mul(loadFloat(app, offset + 8), toFixed)));
        const reg32 joltX = reg32(fp::toInteger<sreg32>(fp::mul(loadFloat(app, offset), toFixed)));
        cpu.eax = car;
        cpu.edx = how;
        cpu.ebx = joltX;
        cpu.ecx = joltZ;
        if (!guest::call(app, cpu, kObjectJolt))
            return std::nullopt;
        if (how == 1)
        {
            // Pushed out of it, by its size, along the line from it.
            put(app, away + 4, 0);
            storeFloat(app, away, fp::sub(loadFloat(app, car + kCarPosition), loadFloat(app, point)));
            storeFloat(app, away + 8, fp::sub(loadFloat(app, car + kCarPosition + 8), loadFloat(app, point + 8)));
            // Its length the code drops (fstp st(0)).
            guest::direct<0x4972f0, vertices::normaliseVector>(app, cpu, away);
            if (cpu.terminate)
                return std::nullopt;
            guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, i32(how), away, objectSize, scaled);
            if (cpu.terminate)
                return std::nullopt;
            guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, i32(how), point, scaled, scaled);
            if (cpu.terminate)
                return std::nullopt;
            cpu.eax = car;
            cpu.edx = away;
            cpu.ebx = scaled;
            if (!guest::call(app, cpu, kObjectResponse, {0}))
                return std::nullopt;
            if ((at(app, car + kCarSoundLoudness) & 0x7fffffff) != 0)
            {
                const Float loudness = loadFloat(app, car + kCarSoundLoudness);
                put(app, car + kCarSoundKind, kSoundBumped);
                const Float halved = fp::mul(loudness, loadFloat(app, kObjectHalf));
                const Float px = loadFloat(app, car + kCarPosition);
                const Float py = loadFloat(app, car + kCarPosition + 4);
                const Float pz = loadFloat(app, car + kCarPosition + 8);
                storeFloat(app, car + kCarSoundAt, px);
                storeFloat(app, car + kCarSoundAt + 4, py);
                storeFloat(app, car + kCarSoundAt + 8, pz);
                storeFloat(app, car + kCarSoundLoudness, halved);
            }
            if (byte(app, car + kCarFlags) & 2)
            {
                guest::direct<0x406120, aidrive::reverseWhenSlowDone>(app, cpu, car);
                if (cpu.terminate)
                    return std::nullopt;
            }
        }
        else if (how == 2)
        {
            if (!fAbove(loadFloat(app, car + kCarSpeedSize), loadDouble(app, kObjectSpeed)))
                continue;
            put(app, car + kCarSoundKind, kSoundKnocked);
            put(app, car + kCarSoundExtra, 0);
            put(app, car + kCarSoundLoudness, kSoundLoud);
            for (reg32 w = 0; w < 12; w += 4)
                put(app, car + kCarSoundAt + w, at(app, car + kCarPosition + w));
        }
        else if (sreg32(how) < 0)
        {
            put(app, car + kCarSoundLoudness, kSoundLoud);
            put(app, car + kCarSoundKind, kSoundBumped);
            put(app, car + kCarSoundExtra, 0);
            const Float px = loadFloat(app, car + kCarPosition);
            const Float py = loadFloat(app, car + kCarPosition + 4);
            const Float pz = loadFloat(app, car + kCarPosition + 8);
            storeFloat(app, car + kCarSoundAt, px);
            storeFloat(app, car + kCarSoundAt + 4, py);
            storeFloat(app, car + kCarSoundAt + 8, pz);
        }
    }
}

/* 0x430da0: againstObjects for car eax; every other register as it was. */
void carAgainstObjects(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
    const std::optional<reg32> r = againstObjects(app, cpu, cpu.eax);
    if (!r)
        return;
    cpu.eax = *r;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esp += 4;
}

/* 0x4310c0: addToList for car eax; ebx, ecx and edx as the caller had them. */
void addCarToList(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
    const std::optional<reg32> r = addToList(app, cpu, cpu.eax);
    if (!r)
        return;
    cpu.eax = *r;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esp += 4;
}

// A car's collisions' fields: whether it is out of the race (+0x88) and in
// it (+0x89), its countdown before it collides again (+0x174), what it
// collides with (+0x188: 0x40 the track's objects, 0x80 its walls).
constexpr reg32 kCarOut = 0x88, kCarIn = 0x89, kCarCountdown = 0x174, kCarCollides = 0x188;
constexpr reg32 kAfterCollisions = 0x4a0140;  // what follows a car's collisions, eax the car

/* A car's collisions for the frame: its countdown on; for a car in the race
 * the other cars (once the countdown is out), the track's objects (by the
 * flag 0x40, likewise) and walls (0x80), then kAfterCollisions.  False when
 * the game stopped; eax as the code leaves it. */
bool frameCollisions(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x14, 0);  // as deep as the code's, for its callees' frames
    const reg32 countdown = at(app, car + kCarCountdown);
    if (sreg32(countdown) > 0)
        put(app, car + kCarCountdown, countdown - 1);
    if (byte(app, car + kCarOut) != 0 || byte(app, car + kCarIn) == 0)
        return true;
    const bool settled = at(app, car + kCarCountdown) == 0;
    if (settled)
    {
        const GuestFrame slot(cpu, 0, 4);  // the call's return address, as the code has it
        if (!addToList(app, cpu, car))
            return false;
    }
    if ((byte(app, car + kCarCollides) & 0x40) && settled)
    {
        const GuestFrame slot(cpu, 0, 4);  // the call's return address, as the code has it
        if (!againstObjects(app, cpu, car))
            return false;
    }
    if (byte(app, car + kCarCollides) & 0x80)
    {
        const GuestFrame slot(cpu, 0, 4);
        if (!againstWalls(app, cpu, car))
            return false;
    }
    cpu.eax = car;
    return guest::call(app, cpu, kAfterCollisions);
}

/* 0x4311d0 (a car's +0x618): frameCollisions for car eax; every other
 * register as it was. */
void carCollisions(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
    if (!frameCollisions(app, cpu, cpu.eax))
        return;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esp += 4;
}

/* What squaredDistance leaves: the square, and the parts the code leaves in
 * ecx (z's difference) and edx (x's and y's squares summed). */
struct Squared
{
    reg32 square, dz, xy;
};

/* The square of the distance between the points (ax, ay, az) and (bx, by,
 * bz), in integers as they wrap. */
Squared squaredDistance(reg32 ax, reg32 ay, reg32 az, reg32 bx, reg32 by, reg32 bz)
{
    const reg32 dx = ax - bx, dy = ay - by, dz = az - bz;
    const reg32 xy = dy * dy + dx * dx;
    return {dz * dz + xy, dz, xy};
}

/* 0x431250: squaredDistance of (eax, edx, ebx) and (ecx, and the two on the
 * stack); ret 8. */
void distanceSquared(App* app, CPU& cpu)
{
    const Squared s = squaredDistance(cpu.eax, cpu.edx, cpu.ebx, cpu.ecx, at(app, cpu.esp + 4), at(app, cpu.esp + 8));
    cpu.eax = s.square;
    cpu.ecx = s.dz;
    cpu.edx = s.xy;
    cpu.esp += 4 + 8;
}

// The points nearestPoint searches: x, y, z and the value, 0x10 bytes each.
constexpr reg32 kPoints = 0x5537c4, kPointCount = 0x167, kPointBytes = 0x10;

/* The value of the point nearest (x, y, z) among the points; the first of
 * those as near as another (compared signed, as they wrap), the search over
 * at a distance of 0. */
Nearest nearest(App* app, reg32 x, reg32 y, reg32 z)
{
    const auto distance = [&](reg32 i) {
        const reg32 p = kPoints + i * kPointBytes;
        return squaredDistance(x, y, z, at(app, p), at(app, p + 4), at(app, p + 8));
    };
    Squared d = distance(0);
    reg32 best = 0;
    sreg32 bestSquare = sreg32(d.square);
    for (reg32 i = 1; sreg32(i) < sreg32(kPointCount); ++i)
    {
        d = distance(i);
        if (sreg32(d.square) < bestSquare)
        {
            best = i;
            bestSquare = sreg32(d.square);
            if (d.square == 0)
                break;
        }
    }
    return {at(app, kPoints + best * kPointBytes + 0xc), d.xy};
}

/* 0x431280: nearest to (eax, edx, ebx). */
void nearestPoint(App* app, CPU& cpu)
{
    const Nearest n = nearest(app, cpu.eax, cpu.edx, cpu.ebx);
    cpu.eax = n.value;
    cpu.edx = n.edx;
    cpu.esp += 4;
}

// The track's COLL file, its objects: its handle, its 16 bytes' header
// ('COLL', its version 11, its size, its chunks' count).
constexpr reg32 kCollHandle = 0x5eff18, kCollHeader = 0x5eff08;
constexpr reg32 kCollVersion = 0x5eff0c, kCollSize = 0x5eff10, kCollCount = 0x5eff14;
constexpr reg32 kCollMagic = 0x53746c;
constexpr reg32 kCollHeaderBytes = 0x10, kCollVersionWanted = 0xb;
// EA's files (native_ea_files.h) are called as C++, but readColl's read: the edx it leaves is
// readCollFile's.
constexpr reg32 kFileRead = 0x4e2de0;   // the handle eax, to edx, ebx bytes
constexpr reg32 kStrncmp = 0x4eb390;    // eax and edx, ebx characters

/* The COLL file `handle` closed, its handle 0 -- as the code writes it,
 * edx's 0 as the close keeps it; eax what the close gives back.  False when
 * the game stopped. */
bool closeColl(App* app, CPU& cpu, reg32 handle)
{
    const std::optional<reg32> closed = guest::direct<0x4e2da0, ea::closeChecked>(app, cpu, handle);
    if (!closed)
        return false;
    cpu.eax = *closed;
    put(app, kCollHandle, 0);
    return true;
}

/* The COLL file named `name` opened (one open before closed first): its
 * header read and checked -- 'COLL', version 11.  The handle, or 0 with the
 * file closed (its handle 0 as the close keeps ebx's). */
std::optional<reg32> openColl(App* app, CPU& cpu, reg32 name)
{
    const GuestFrame frame(cpu, 0x14, 8);  // the open's two words
    if (at(app, kCollHandle) != 0 && !closeColl(app, cpu, at(app, kCollHandle)))
        return std::nullopt;
    if (!guest::direct<0x4e28d0, ea::openToRead>(app, cpu, name, kCollHandle, frame - 8, frame - 4))
        return std::nullopt;
    const std::optional<reg32> read =
        guest::direct<0x4e2de0, ea::readChecked>(app, cpu, at(app, kCollHandle), kCollHeader, kCollHeaderBytes);
    if (!read)
        return std::nullopt;
    bool good = false;
    if (*read != 0)
    {
        cpu.eax = kCollHeader;
        cpu.edx = kCollMagic;
        cpu.ebx = 4;
        if (!guest::call(app, cpu, kStrncmp))
            return std::nullopt;
        good = cpu.eax == 0 && at(app, kCollVersion) == kCollVersionWanted;
    }
    if (good)
        return at(app, kCollHandle);
    // The handle 0, ebx's as the close keeps it.
    if (!guest::direct<0x4e2da0, ea::closeChecked>(app, cpu, at(app, kCollHandle)))
        return std::nullopt;
    put(app, kCollHandle, 0);
    return 0;
}

/* The COLL file `handle` past its header read into `to`, its size less the
 * header's.  False when the game stopped. */
bool readColl(App* app, CPU& cpu, reg32 handle, reg32 to)
{
    if (!guest::direct<0x4e2e60, ea::seekChecked>(app, cpu, handle, kCollHeaderBytes))
        return false;
    cpu.eax = handle;
    cpu.edx = to;
    cpu.ebx = at(app, kCollSize) - kCollHeaderBytes;
    return guest::call(app, cpu, kFileRead);
}

/* 0x431320: openColl for the name at eax. */
void openCollFile(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
    const std::optional<reg32> handle = openColl(app, cpu, cpu.eax);
    if (!handle)
        return;
    cpu.eax = *handle;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esp += 4;
}

/* 0x4313b0: closeColl for eax; edx as the caller had it. */
void closeCollFile(App* app, CPU& cpu)
{
    const reg32 edx = cpu.edx;
    if (!closeColl(app, cpu, cpu.eax))
        return;
    cpu.edx = edx;
    cpu.esp += 4;
}

/* The COLL file's size, its header's. */
reg32 collSize(App* app)
{
    return at(app, kCollSize);
}

/* 0x4313d0: collSize. */
void collFileSize(App* app, CPU& cpu)
{
    cpu.eax = collSize(app);
    cpu.esp += 4;
}

/* 0x4313e0: readColl of the COLL file eax into edx; 1, edx as the read
 * leaves it. */
void readCollFile(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx;
    if (!readColl(app, cpu, cpu.eax, cpu.edx))
        return;
    cpu.eax = 1;
    cpu.ebx = ebx;
    cpu.esp += 4;
}

/* The COLL file's chunks' count, its header's. */
reg32 collCount(App* app)
{
    return at(app, kCollCount);
}

/* 0x431410: collCount. */
void collFileCount(App* app, CPU& cpu)
{
    cpu.eax = collCount(app);
    cpu.esp += 4;
}

/* Chunk `chunk` of the COLL file's data at `data`: data plus its offset in
 * the table at data. */
reg32 collChunkAt(App* app, reg32 data, reg32 chunk)
{
    return data + at(app, data + chunk * 4);
}

/* 0x431420: collChunkAt for eax and edx. */
void collChunk(App* app, CPU& cpu)
{
    cpu.eax = collChunkAt(app, cpu.eax, cpu.edx);
    cpu.esp += 4;
}

// The controls' state for the frame: its word at 0x79c4a4 (the steering in
// its top byte), the pedals' bytes (+4, +5), the buttons (+6: 2 the reset or
// the pause, 4 and 8 the clutch's and the like, 0x40 the view, 0x80 the
// horn).
constexpr reg32 kControls = 0x79c4a5;
constexpr reg32 kRaceEntryBytes = 0x6c;
// A car's fields the controls set.
constexpr reg32 kCarThrottleAsked = 0x504, kCarBrakeAsked = 0x505, kCarGearAsked = 0x506, kCarGear = 0x50b;
constexpr reg32 kCarButton4 = 0x50d, kCarButton8 = 0x50e, kCarButton80 = 0x512, kCarSteeringAsked = 0x518;
constexpr reg32 kCarPlayer = 0x1f0, kCarHandlingRecord = 0x528, kCarOwnControls = 0x5fc;
constexpr reg32 kCarView = 0x660;

/* The player's car `car`'s controls for the frame -- 0x405740 first while a
 * replay, a demo or the lane votes say so, the car's own (+0x5fc), then the
 * controls' state into the car: the steering and pedals' bytes, the gear
 * asked for (stepped through 0x48f710 or 0x48f6e0 as the car's gearbox is
 * automatic or not), the buttons as 0 or 1, the view stepped (of three), and
 * the reset (0x405220, edx 1, for the car's flag 0x20) or the pause
 * (0x429100).  What the code leaves in eax, or nullopt when the game
 * stopped. */
std::optional<reg32> controls(App* app, CPU& cpu, reg32 car)
{
    const GuestFrame frame(cpu, 0x10, 0);  // as deep as the code's
    if (sreg32(at(app, 0x5efd98)) > 0 || sreg32(at(app, 0x5efd88)) > 0 || gameSettings(app).raceOptionLaneVotes != 0)
    {
        if (!guest::direct<0x405740, aidrive::driveCop>(app, cpu, car))
            return std::nullopt;
    }
    cpu.eax = car;
    if (!guest::call(app, cpu, at(app, car + kCarOwnControls)))
        return std::nullopt;
    putByte(app, car + kCarThrottleAsked, byte(app, kControls + 4));
    putByte(app, car + kCarBrakeAsked, byte(app, kControls + 5));
    put(app, car + kCarSteeringAsked, reg32(sreg32(at(app, kControls)) >> 24));
    const reg32 player = at(app, car + kCarPlayer);
    reg8 gear;
    if (gameSettings(app).entryAt(player * kRaceEntryBytes).transmission == 1)
        gear = guest::direct<0x48f710, controls::stepChoiceOfThree>(app, cpu, byte(app, car + kCarGear));
    else
        gear = guest::direct<0x48f6e0, controls::stepChoice>(app, cpu, byte(app, car + kCarGear),
                                                             at(app, at(app, car + kCarHandlingRecord) + 8));
    if (cpu.terminate)
        return std::nullopt;
    putByte(app, car + kCarGearAsked, gear);
    putByte(app, car + kCarButton4, (byte(app, kControls + 6) & 4) ? 1 : 0);
    putByte(app, car + kCarButton8, (byte(app, kControls + 6) & 8) ? 1 : 0);
    reg32 eax = (byte(app, kControls + 6) & 0x80) ? 1 : 0;
    putByte(app, car + kCarButton80, reg8(eax));
    if (byte(app, kControls + 6) & 0x40)
    {
        const reg32 view = at(app, car + kCarView) + 1;
        put(app, car + kCarView, view);
        eax = reg32(sreg32(view) / 3);
        put(app, car + kCarView, reg32(sreg32(view) % 3));
    }
    if (byte(app, kControls + 6) & 2)
    {
        const std::optional<reg32> left = (byte(app, car + kCarFlags) & 0x20)
                                              ? guest::direct<0x405220, aidrive::jumpBehind>(app, cpu, car, true)
                                              : guest::direct<0x429100, racestart::resetCarOf>(app, cpu, car);
        if (!left)
            return std::nullopt;
        eax = *left;
    }
    return eax;
}

/* 0x431430 (a car's +0x600): controls for car eax; ebx, ecx and edx as the
 * caller had them. */
void playerControls(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
    const std::optional<reg32> eax = controls(app, cpu, cpu.eax);
    if (!eax)
        return;
    cpu.eax = *eax;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esp += 4;
}

// The render states the game keeps (setRenderState's cache), -1 for none.
constexpr reg32 kStateTexture = 0x554e54, kStateAlpha = 0x554e58, kStateDepthBias = 0x554e5c;
constexpr reg32 kThrashSetState = 0x9ef96c;  // THRASH_setstate's pointer: (state, value), stdcall
// The render states' numbers the drawers set themselves.
constexpr reg32 kTextureState = 1, kFogColourState = 3, kAlphaTestState = 0x68, kDepthBiasState = 0x15;
// The driver's depth for the HUD (0 off the card), the card's, the race's depth bias.
constexpr reg32 kHudDepth = 0x7a3a60, kCardDepth = 0x7a3a68, kRaceDepthBias = 0x7a3a64;
constexpr reg32 kDepthOffsets = 0x4c7a60;  // the depth offsets worked out: no registers

/* The cached render states forgotten (-1), so that each is set again. */
void forgetStates(App* app)
{
    for (const reg32 state : {0x554e58u, 0x554e5cu, 0x554e60u, 0x554e64u, 0x554e68u, 0x554e6cu, 0x554e54u})
        put(app, state, 0xffffffff);
}

/* 0x4318c0: forgetStates; every register as it was. */
void forgetRenderStates(App* app, CPU& cpu)
{
    forgetStates(app);
    cpu.esp += 4;
}

namespace
{

/* graphics::setRenderState (state, value), as C++: its core leaves eax its
 * answer, as the code does.  False when the game stopped. */
bool setRenderState(App* app, CPU& cpu, reg32 state, reg32 value)
{
    return guest::direct<0x431900, graphics::sendState>(app, cpu, state, value);
}

/* The render states set in turn.  False when the game stopped. */
bool setStates(App* app, CPU& cpu, std::initializer_list<std::pair<reg32, reg32>> states)
{
    for (const auto& s : states)
        if (!setRenderState(app, cpu, s.first, s.second))
            return false;
    return true;
}

/* The render-state functions' adapter: the core, edx as the caller had it
 * (their code pushes and pops it), eax as the last call leaves it. */
void keepingEdx(App* app, CPU& cpu, bool (*core)(App*, CPU&))
{
    const reg32 edx = cpu.edx;
    if (!core(app, cpu))
        return;
    cpu.edx = edx;
    cpu.esp += 4;
}

}  // namespace

/* The render states for 2D text: states forgotten, then 0xd, 0x15, 0x68, 2
 * and 4 all 0. */
bool textStates(App* app, CPU& cpu)
{
    forgetStates(app);
    return setStates(app, cpu, {{0xd, 0}, {0x15, 0}, {0x68, 0}, {2, 0}, {4, 0}});
}

namespace
{

/* The same states, 4 first. */
bool pictureStates(App* app, CPU& cpu)
{
    forgetStates(app);
    return setStates(app, cpu, {{4, 0}, {0xd, 0}, {0x15, 0}, {0x68, 0}, {2, 0}});
}

}  // namespace

/* The render states for the race's HUD: the driver's depth (the card's while
 * the text is projected, else 0), the depth offsets, states forgotten, then 4
 * that depth, 0xd 1, 0x15 the depth bias, 0x68 and 2 0 -- each value read
 * after the call before it, as the code reads it. */
bool hudStates(App* app, CPU& cpu)
{
    put(app, kHudDepth, gameSettings(app).projectedText != 0 ? at(app, kCardDepth) : 0);
    if (!guest::call(app, cpu, kDepthOffsets))
        return false;
    forgetStates(app);
    return setRenderState(app, cpu, 4, at(app, kHudDepth)) && setRenderState(app, cpu, 0xd, 1)
           && setRenderState(app, cpu, 0x15, at(app, kRaceDepthBias)) && setStates(app, cpu, {{0x68, 0}, {2, 0}});
}

/* The render states for the 2D screens: states forgotten, then 0xd, 0x15,
 * 0x68 0, 2 1, 4 0, 0x2c2 1. */
bool screenStates(App* app, CPU& cpu)
{
    forgetStates(app);
    return setStates(app, cpu, {{0xd, 0}, {0x15, 0}, {0x68, 0}, {2, 1}, {4, 0}, {0x2c2, 1}});
}

/* 0x431aa0: textStates. */
void statesForText(App* app, CPU& cpu)
{
    keepingEdx(app, cpu, textStates);
}

/* 0x431af0: pictureStates. */
void statesForPictures(App* app, CPU& cpu)
{
    keepingEdx(app, cpu, pictureStates);
}

/* 0x431b40: hudStates. */
void statesForHud(App* app, CPU& cpu)
{
    keepingEdx(app, cpu, hudStates);
}

/* 0x431bc0: screenStates. */
void statesForScreens(App* app, CPU& cpu)
{
    keepingEdx(app, cpu, screenStates);
}

/* A view pass's layer that draws nothing: what its callers hand it (eax how many quads, edx the
 * layer) goes unread. */
void layerNothing(App*, reg32, reg32)
{
}

/* 0x431c20: layerNothing (an empty function). */
void passLayerNothing(App* app, CPU& cpu)
{
    layerNothing(app, cpu.eax, cpu.edx);
    cpu.esp += 4;
}

/* The main pass's second and first layers: nothing; the layer its callers hand it (eax) goes
 * unread. */
void secondLayerNothing(App*, reg32)
{
}

void firstLayerNothing(App*, reg32)
{
}

/* 0x431c30: secondLayerNothing (an empty function). */
void passSecondLayer(App* app, CPU& cpu)
{
    secondLayerNothing(app, cpu.eax);
    cpu.esp += 4;
}

/* 0x431c40: firstLayerNothing (an empty function). */
void passFirstLayer(App* app, CPU& cpu)
{
    firstLayerNothing(app, cpu.eax);
    cpu.esp += 4;
}

// The software renderer's blend: its tables of a channel times another (5
// bits by 5 at 0x5f1798, 6 by 5 at 0x5f0798, a word each) and the pixel
// format's masks and shifts (0x554e70 on: blue, green, red's masks, 0,
// green's and red's shifts), for 15 bits a pixel (0x56438c 0xf) or 16.
constexpr reg32 kBlendMasks = 0x554e70, kPixelBits = 0x56438c;
constexpr reg32 kBlueMask = kBlendMasks, kGreenMask = kBlendMasks + 4, kRedMask = kBlendMasks + 8;
constexpr reg32 kGreenShift = kBlendMasks + 0x10, kRedShift = kBlendMasks + 0x14;
// The tables blendSpan reads each channel through.
constexpr reg32 kBlueTable = 0x5f1f98, kGreenTable = 0x5f1fa4, kRedTable = 0x5f1f9c;

/* The software renderer's blend tables -- a times b over 31, rounded up, to
 * 31 (63 for green's) -- and the pixel format's masks and shifts.  The last
 * quotient, which the code leaves in eax. */
reg32 fillBlendTables(App* app)
{
    struct Table
    {
        i32 columns, most;
        reg32 at;
    };
    static const Table tables[] = {{0x20, 0x1f, 0x5f1798}, {0x40, 0x3f, 0x5f0798}};
    i32 quotient = 0;
    for (const Table& t : tables)
    {
        reg32 entry = t.at;
        for (i32 a = 0; a < 0x20; ++a)
            for (i32 b = 0; b < t.columns; ++b)
            {
                quotient = (a * b + 0x1e) / 0x1f;
                putHalf(app, entry, reg16(quotient > t.most ? t.most : quotient));
                entry += 2;
            }
    }
    const bool fifteen = at(app, kPixelBits) == 0xf;
    put(app, kBlueMask, 0x1f);
    put(app, kGreenMask, fifteen ? 0x3e0 : 0x7e0);
    put(app, kBlendMasks + 0xc, 0);
    put(app, kRedMask, fifteen ? 0x7c00 : 0xf800);
    put(app, kGreenShift, 5);
    put(app, kRedShift, fifteen ? 0xa : 0xb);
    return reg32(quotient);
}

/* 0x431c50: fillBlendTables. */
void makeBlendTables(App* app, CPU& cpu)
{
    cpu.eax = fillBlendTables(app);
    cpu.esp += 4;
}

// A span's record, as blendSpan reads it: the texture's rows, the texel's
// mask, the screen's pixel and the span's last, u and v (16.16 and 15
// bits) and their steps.
constexpr reg32 kSpanRows = 0, kSpanMask = 8, kSpanPixel = 0xc, kSpanLast = 0x10, kSpanU = 0x14, kSpanV = 0x18;
constexpr reg32 kSpanStepU = 0x1c, kSpanStepV = 0x20;

/* The span at `span` blended into the screen, a pixel at a time: each
 * texel's channels through the blend tables added to the screen pixel's,
 * each to its most -- compared signed, as the code does. */
void blendInto(App* app, reg32 span)
{
    const auto channel = [&](reg32 table, reg32 mask, reg32 shift, reg32 texel, reg32 pixel) {
        const reg32 index = reg32(sreg32(texel & mask) >> shift) * 2;
        const reg32 value = reg32(half(app, at(app, table) + index)) << shift;
        const reg32 sum = value + (pixel & mask);
        return sreg32(sum) > sreg32(mask) ? mask : sum;
    };
    while (at(app, span + kSpanLast) >= at(app, span + kSpanPixel))
    {
        const reg32 u = at(app, span + kSpanU);
        const reg32 texelAt = ((at(app, span + kSpanV) & at(app, span + kSpanMask)) + u) >> 15;
        const reg32 texel = at(app, at(app, span + kSpanRows) + texelAt * 4);
        const reg32 nextU = u + at(app, span + kSpanStepU);
        const reg32 nextV = at(app, span + kSpanV) + at(app, span + kSpanStepV);
        const reg32 pixel = half(app, at(app, span + kSpanPixel));
        put(app, span + kSpanV, nextV);
        const reg32 blueTable = at(app, kBlueTable);
        put(app, span + kSpanU, nextU);
        // Blue's is not shifted: its table is read at the texel's blue twice.
        reg32 blue = reg32(half(app, blueTable + (at(app, kBlueMask) & texel) * 2)) + (pixel & at(app, kBlueMask));
        if (sreg32(blue) > sreg32(at(app, kBlueMask)))
            blue = at(app, kBlueMask);
        const reg32 green = channel(kGreenTable, at(app, kGreenMask), byte(app, kGreenShift) & 0x1f, texel, pixel);
        const reg32 red = channel(kRedTable, at(app, kRedMask), byte(app, kRedShift) & 0x1f, texel, pixel);
        const reg32 to = at(app, span + kSpanPixel);
        put(app, span + kSpanPixel, to + 2);
        putHalf(app, to, reg16(green | blue | red));
    }
}

/* 0x431d50 (handed to the software renderer as its blend): blendInto for the
 * span at eax. */
void blendSpan(App* app, CPU& cpu)
{
    blendInto(app, cpu.eax);
    cpu.esp += 4;
}

// A corner's depth, as the drawers' corners have it; the depths a quad is
// subdivided within (doubles).
constexpr reg32 kCornerDepth = 8;

/* How finely the quad of the corners a, b, c and d is to be subdivided, by
 * the nearest of their depths: 0 beyond kDepthNear, 1 beyond kDepthFar,
 * else 2.  The nearest is kept as its bits (a NaN taken, as fcom's below
 * says for it) and goes to a double, exactly. */
reg32 subdivisions(App* app, reg32 a, reg32 b, reg32 c, reg32 d)
{
    reg32 nearest = at(app, a + kCornerDepth);
    for (const reg32 corner : {b, c, d})
        if (fBelow(loadFloat(app, corner + kCornerDepth), floatAt(nearest)))
            nearest = at(app, corner + kCornerDepth);
    if (fAbove(floatAt(nearest), loadDouble(app, kDepthNear)))
        return 0;
    if (fAbove(floatAt(nearest), loadDouble(app, kDepthFar)))
        return 1;
    return 2;
}

/* 0x432690: subdivisions of the corners eax, edx, ebx and ecx. */
void subdivisionDepth(App* app, CPU& cpu)
{
    cpu.eax = subdivisions(app, cpu.eax, cpu.edx, cpu.ebx, cpu.ecx);
    cpu.esp += 4;
}

// A polygon list's record, as the drawers read it: the next (+0), its kind
// (+4, a word: 0 a quad, 1 a triangle), its flags (+6: 1 the alpha test, 2
// its corners' texture coordinates kept, 4 no depth bias, 0x10 drawn by z),
// its corners (+8 on), its texture (+0x18: the handle +4, the coordinates
// from +8, its flags +0x28, 8 the software blend's).
constexpr reg32 kRecordKind = 4, kRecordFlags = 6, kRecordCorners = 8, kRecordTexture = 0x18;
constexpr reg32 kTextureHandle = 4, kTextureFlags = 0x28;
constexpr reg32 kCornerU = 0x18, kCornerV = 0x1c;  // a corner's texture coordinates
constexpr reg32 kKindQuad = 0, kKindTriangle = 1;
constexpr reg32 kDrawTriangle = 0x4c11b0;    // the triangle eax, edx, ebx drawn
constexpr reg32 kDrawTriangleZ = 0x4c1aa0;   // by z
constexpr reg32 kDrawQuad = 0x4c12e0;        // the quad eax, edx, ebx, ecx drawn
constexpr reg32 kDrawQuadZ = 0x4c20e0;       // by z
constexpr reg32 kDrawFlat = 0x4c0b20;        // a flat triangle, eax and edx

namespace
{

/* THRASH_setstate (state, value) through its pointer, the value kept at
 * `kept` first.  False when the game stopped. */
bool setThrashState(App* app, CPU& cpu, reg32 state, reg32 value, reg32 kept)
{
    put(app, kept, value);
    return guest::call(app, cpu, at(app, kThrashSetState), {state, value});
}

/* The drawers' alpha test, on for `on`, set only where it changes. */
bool alphaState(App* app, CPU& cpu, bool on)
{
    const reg32 value = on ? 1 : 0;
    if (at(app, kStateAlpha) == value)
        return true;
    return setThrashState(app, cpu, kAlphaTestState, value, kStateAlpha);
}

/* The drawers' texture: the record's texture's handle, or none, set only
 * where it changes.  `texture` as the code read it before the coordinates'
 * copy; the handle from the record's texture read again. */
bool textureState(App* app, CPU& cpu, reg32 record, reg32 texture)
{
    if (texture != 0)
    {
        const reg32 handle = at(app, at(app, record + kRecordTexture) + kTextureHandle);
        if (at(app, kStateTexture) != handle)
            return setThrashState(app, cpu, kTextureState, handle, kStateTexture);
        return true;
    }
    if (at(app, kStateTexture) == 0)
        return true;
    return setThrashState(app, cpu, kTextureState, 0, kStateTexture);
}

/* The drawers' depth bias: none for the record's flag 4, else the race's;
 * set only where it changes. */
bool depthBiasState(App* app, CPU& cpu, reg8 flags)
{
    const reg32 bias = (flags & 4) ? 0 : at(app, kRaceDepthBias);
    if (at(app, kStateDepthBias) == bias)
        return true;
    return setThrashState(app, cpu, kDepthBiasState, bias, kStateDepthBias);
}

/* A drawer's draw call: the corners in eax, edx, ebx and ecx. */
bool drawCorners(App* app, CPU& cpu, reg32 target, reg32 a, reg32 b, reg32 c, reg32 d)
{
    cpu.eax = a;
    cpu.edx = b;
    cpu.ebx = c;
    cpu.ecx = d;
    return guest::call(app, cpu, target);
}

/* The next record of a drawer's run: 0 at the list's end, the record itself
 * of another kind; nullopt to go on with it. */
std::optional<reg32> runEnds(App* app, reg32 next, reg32 kind)
{
    if (next == 0 || half(app, next + kRecordKind) != kind)
        return next;
    return std::nullopt;
}

/* A drawer's adapter: the record at eax, the next in eax; ebx, ecx and edx
 * as the caller had them (the drawers push and pop them). */
void drawer(App* app, CPU& cpu, std::optional<reg32> (*core)(App*, CPU&, reg32))
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
    const std::optional<reg32> next = core(app, cpu, cpu.eax);
    if (!next)
        return;
    cpu.eax = *next;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esp += 4;
}

}  // namespace

/* The textured triangles from `record` on, while they are triangles: the
 * alpha test by the flags, the texture's coordinates into the three corners
 * unless flag 2 (the first's as words, the second's as floats, the third's
 * as words, each from the record's texture read again), the texture, the
 * software blend's colour for a texture's flag 8, the triangle drawn.  The
 * next record, or nullopt when the game stopped. */
std::optional<reg32> texturedTriangles(App* app, CPU& cpu, reg32 record)
{
    for (;;)
    {
        safepoint(app, cpu);
        const reg32 first = at(app, record + kRecordCorners), second = at(app, record + kRecordCorners + 4);
        const reg32 third = at(app, record + kRecordCorners + 8);
        const reg8 flags = byte(app, record + kRecordFlags);
        if (!alphaState(app, cpu, (flags & 1) != 0))
            return std::nullopt;
        const reg32 texture = at(app, record + kRecordTexture);
        const auto from = [&](reg32 offset) { return at(app, record + kRecordTexture) + offset; };
        if (texture != 0 && !(flags & 2))
        {
            put(app, first + kCornerU, at(app, from(8)));
            put(app, first + kCornerV, at(app, from(0xc)));
            storeFloat(app, second + kCornerU, loadFloat(app, from(0x10)));
            storeFloat(app, second + kCornerV, loadFloat(app, from(0x14)));
            put(app, third + kCornerU, at(app, from(0x18)));
            put(app, third + kCornerV, at(app, from(0x1c)));
        }
        if (!textureState(app, cpu, record, texture))
            return std::nullopt;
        const reg32 now = at(app, record + kRecordTexture);
        if (now != 0 && (byte(app, now + kTextureFlags) & 8)
            && !guest::direct<0x432390, graphics::softwareBlendColour>(app, cpu, at(app, first + 0x10)))
            return std::nullopt;
        if (!drawCorners(app, cpu, kDrawTriangle, first, second, third, cpu.ecx))
            return std::nullopt;
        if (const auto next = runEnds(app, at(app, record), kKindTriangle))
            return next;
        record = at(app, record);
    }
}

/* The flat triangle at `record`: no texture, the alpha test by its flags,
 * drawn (its corners +8 and +0xc).  The next record. */
std::optional<reg32> flatTriangle(App* app, CPU& cpu, reg32 record)
{
    const reg32 first = at(app, record + kRecordCorners), second = at(app, record + kRecordCorners + 4);
    if (at(app, kStateTexture) != 0 && !setThrashState(app, cpu, kTextureState, 0, kStateTexture))
        return std::nullopt;
    if (!alphaState(app, cpu, (byte(app, record + kRecordFlags) & 1) != 0))
        return std::nullopt;
    cpu.eax = first;
    cpu.edx = second;
    if (!guest::call(app, cpu, kDrawFlat))
        return std::nullopt;
    return at(app, record);
}

/* The triangles from `record` on, while they are triangles: as
 * texturedTriangles, the coordinates the first's as floats, the others' as
 * words, no software blend, the depth bias by flag 4, drawn by z for flag
 * 0x10. */
std::optional<reg32> trianglesByZ(App* app, CPU& cpu, reg32 record)
{
    for (;;)
    {
        safepoint(app, cpu);
        const reg32 first = at(app, record + kRecordCorners), second = at(app, record + kRecordCorners + 4);
        const reg32 third = at(app, record + kRecordCorners + 8);
        const reg8 flags = byte(app, record + kRecordFlags);
        if (!alphaState(app, cpu, (flags & 1) != 0))
            return std::nullopt;
        const reg32 texture = at(app, record + kRecordTexture);
        const auto from = [&](reg32 offset) { return at(app, record + kRecordTexture) + offset; };
        if (texture != 0 && !(flags & 2))
        {
            storeFloat(app, first + kCornerU, loadFloat(app, from(8)));
            storeFloat(app, first + kCornerV, loadFloat(app, from(0xc)));
            put(app, second + kCornerU, at(app, from(0x10)));
            put(app, second + kCornerV, at(app, from(0x14)));
            put(app, third + kCornerU, at(app, from(0x18)));
            put(app, third + kCornerV, at(app, from(0x1c)));
        }
        if (!textureState(app, cpu, record, texture) || !depthBiasState(app, cpu, flags))
            return std::nullopt;
        if (!drawCorners(app, cpu, (flags & 0x10) ? kDrawTriangleZ : kDrawTriangle, first, second, third, cpu.ecx))
            return std::nullopt;
        if (const auto next = runEnds(app, at(app, record), kKindTriangle))
            return next;
        record = at(app, record);
    }
}

/* The quads from `record` on, while they are quads: as trianglesByZ, four
 * corners, the first's and third's coordinates as floats. */
std::optional<reg32> quadsByZ(App* app, CPU& cpu, reg32 record)
{
    for (;;)
    {
        safepoint(app, cpu);
        const reg32 first = at(app, record + kRecordCorners), third = at(app, record + kRecordCorners + 8);
        const reg32 second = at(app, record + kRecordCorners + 4), fourth = at(app, record + kRecordCorners + 0xc);
        const reg8 flags = byte(app, record + kRecordFlags);
        if (!alphaState(app, cpu, (flags & 1) != 0))
            return std::nullopt;
        const reg32 texture = at(app, record + kRecordTexture);
        const auto from = [&](reg32 offset) { return at(app, record + kRecordTexture) + offset; };
        if (texture != 0 && !(flags & 2))
        {
            storeFloat(app, first + kCornerU, loadFloat(app, from(8)));
            storeFloat(app, first + kCornerV, loadFloat(app, from(0xc)));
            put(app, second + kCornerU, at(app, from(0x10)));
            put(app, second + kCornerV, at(app, from(0x14)));
            storeFloat(app, third + kCornerU, loadFloat(app, from(0x18)));
            storeFloat(app, third + kCornerV, loadFloat(app, from(0x1c)));
            put(app, fourth + kCornerU, at(app, from(0x20)));
            put(app, fourth + kCornerV, at(app, from(0x24)));
        }
        if (!textureState(app, cpu, record, texture) || !depthBiasState(app, cpu, flags))
            return std::nullopt;
        if (!drawCorners(app, cpu, (flags & 0x10) ? kDrawQuadZ : kDrawQuad, first, second, third, fourth))
            return std::nullopt;
        if (const auto next = runEnds(app, at(app, record), kKindQuad))
            return next;
        record = at(app, record);
    }
}

/* 0x433530 (a polygon list's drawer, the software renderers' and
 * Direct3D's): texturedTriangles. */
void drawTexturedTriangles(App* app, CPU& cpu)
{
    drawer(app, cpu, texturedTriangles);
}

/* 0x433660 (a polygon list's drawer): flatTriangle. */
void drawFlatTriangles(App* app, CPU& cpu)
{
    drawer(app, cpu, flatTriangle);
}

/* 0x4336e0 (a polygon list's drawer, the driver's): trianglesByZ. */
void drawTrianglesByZ(App* app, CPU& cpu)
{
    drawer(app, cpu, trianglesByZ);
}

/* 0x433840 (a polygon list's drawer, the driver's): quadsByZ. */
void drawQuadsByZ(App* app, CPU& cpu)
{
    drawer(app, cpu, quadsByZ);
}

/* 0x4339d0 (a polygon list's drawer, the driver's): flatTriangle, the same
 * code again. */
void drawFlatTrianglesAgain(App* app, CPU& cpu)
{
    drawer(app, cpu, flatTriangle);
}

/* The triangles with colours of their own from `record` on -- the driver's
 * on the card (0x434380) or the other (0x433e30), tail-called with the
 * record in eax.  What it leaves in eax, the next record; nullopt when the
 * game stopped. */
std::optional<reg32> ownColourTriangles(App* app, CPU& cpu, reg32 record)
{
    cpu.eax = record;
    if (!guest::call(app, cpu, at(app, kHudDepth) != 0 ? 0x434380 : 0x433e30))
        return std::nullopt;
    return cpu.eax;
}

/* 0x4347b0 (a polygon list's drawer): ownColourTriangles for the record in
 * eax; the other registers as the callee leaves them. */
void drawColouredTriangles(App* app, CPU& cpu)
{
    const std::optional<reg32> next = ownColourTriangles(app, cpu, cpu.eax);
    if (!next)
        return;
    cpu.eax = *next;
    cpu.esp += 4;
}

/* A render state's record: state +8, value +0xc set.  The next record. */
std::optional<reg32> stateFromRecord(App* app, CPU& cpu, reg32 record)
{
    if (!setRenderState(app, cpu, at(app, record + 8), at(app, record + 0xc)))
        return std::nullopt;
    return at(app, record);
}

/* 0x434850 (a polygon list's drawer): stateFromRecord; edx as the caller had
 * it. */
void stateRecord(App* app, CPU& cpu)
{
    const reg32 edx = cpu.edx;
    const std::optional<reg32> next = stateFromRecord(app, cpu, cpu.eax);
    if (!next)
        return;
    cpu.eax = *next;
    cpu.edx = edx;
    cpu.esp += 4;
}

// The polygon lists' drawers, by a record's kind.
constexpr reg32 kDrawers = 0x5f0780;

/* The polygon list from `record`: state 0xd 1, then each record handed to
 * the drawer of its kind (called by address: the table's), which gives back
 * the next.  eax as the last call leaves it.  False when the game stopped. */
bool drawList(App* app, CPU& cpu, reg32 record)
{
    if (!setRenderState(app, cpu, 0xd, 1))
        return false;
    while (record != 0)
    {
        safepoint(app, cpu);
        const reg32 kind = reg32(sreg32(at(app, record + 2)) >> 16);
        cpu.eax = record;
        if (!guest::call(app, cpu, at(app, kind * 4 + kDrawers)))
            return false;
        record = cpu.eax;
    }
    safepoint(app, cpu);
    return true;
}

/* 0x434950: drawList from eax; ecx and edx as the caller had them. */
void drawRecords(App* app, CPU& cpu)
{
    const reg32 ecx = cpu.ecx, edx = cpu.edx;
    if (!drawList(app, cpu, cpu.eax))
        return;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esp += 4;
}

/* 0x434990: the race's text for the frame (0x4bed70 with 0x7cdac0). */
void raceTextFrame(App* app, CPU& cpu)
{
    cpu.eax = 0x7cdac0;
    if (!guest::call(app, cpu, 0x4bed70))
        return;
    cpu.esp += 4;
}

// Whether the view is drawn mirrored.
constexpr reg32 kViewMirrored = 0x554e4c;

/* Whether the view at `view` is mirrored (its first word 1) against the
 * mirrored screen, into kViewMirrored; that. */
reg32 setViewMirrored(App* app, reg32 view)
{
    const reg32 mirrored = (at(app, view) == 1 ? 1 : 0) ^ gameSettings(app).mirrored;
    put(app, kViewMirrored, mirrored);
    return mirrored;
}

/* 0x4349a0: setViewMirrored for the view at eax. */
void setMirrored(App* app, CPU& cpu)
{
    cpu.eax = setViewMirrored(app, cpu.eax);
    cpu.esp += 4;
}

/* 0x4349d0: setViewMirrored for the view at eax, then the polygon list on
 * the stack's first word drawn (drawList); ret 8.  ecx and edx as the
 * caller had them (pushed and popped here and in drawRecords). */
void drawMirroredRecords(App* app, CPU& cpu)
{
    const reg32 ecx = cpu.ecx, edx = cpu.edx;
    const reg32 list = at(app, cpu.esp + 4);
    setViewMirrored(app, cpu.eax);
    if (!drawList(app, cpu, list))
        return;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esp += 4 + 8;
}

// The frames drawn.
constexpr reg32 kFramesDrawn = 0x5f1fa8;

/* 0x434a70: the frames drawn counted. */
void countFrame(App* app, CPU& cpu)
{
    put(app, kFramesDrawn, at(app, kFramesDrawn) + 1);
    cpu.esp += 4;
}

/* 0x434a80: the race's view distance (0x41d610 with the settings'; by
 * address: the eax it leaves is this one's, and track's core gives nothing
 * back). */
void raceViewDistance(App* app, CPU& cpu)
{
    cpu.eax = gameSettings(app).viewDistance;
    if (!guest::call(app, cpu, 0x41d610))
        return;
    cpu.esp += 4;
}

// The race's fog colour: blue, green, red bytes.
constexpr reg32 kFogColour = 0x6ff9f8;

/* The race's fog colour set: its bytes as blue, green and red, alpha 0xff. */
bool fogColour(App* app, CPU& cpu)
{
    const reg32 colour = 0xff000000 | reg32(byte(app, kFogColour + 2)) << 16 | reg32(byte(app, kFogColour + 1)) << 8
                         | byte(app, kFogColour);
    return setRenderState(app, cpu, kFogColourState, colour);
}

/* 0x434a90: fogColour. */
void raceFrameEnd(App* app, CPU& cpu)
{
    keepingEdx(app, cpu, fogColour);
}

/* 0x434ae0: nothing at the race's frame (an empty function). */
void raceFrameNothing(App*, CPU& cpu)
{
    cpu.esp += 4;
}

/* 0x434af0: nothing at the race's end (an empty function). */
void raceEndNothing(App*, CPU& cpu)
{
    cpu.esp += 4;
}

/* 0x434b00: the drawing made ready at the race's start: the reciprocals'
 * table (0x4eb3c0), the polygon lists' drawers (0x434870, by address: the
 * eax it leaves is this one's, and render's core gives nothing back). */
void raceStartDrawing(App* app, CPU& cpu)
{
    if (!guest::direct<0x4eb3c0, graphics::reciprocalsMade>(app, cpu) || !guest::call(app, cpu, 0x434870))
        return;
    cpu.esp += 4;
}

/* 0x434b10: nothing at the race's start (an empty function). */
void raceStartNothing(App*, CPU& cpu)
{
    cpu.esp += 4;
}

// The screen's clip edges, as a float's bits: the far depth, left, right,
// top and bottom.
constexpr reg32 kClipFar = 0x7d34f8, kClipLeft = 0x7d34fc, kClipRight = 0x7d350c, kClipTop = 0x7d3504;
constexpr reg32 kClipBottom = 0x7d3500;
// A drawn corner, 0x20 bytes: x, y, depth, w, its colour, its clip code (a
// byte), its texture's u and v.
constexpr reg32 kVertexX = 0, kVertexY = 4, kVertexZ = 8, kVertexW = 0xc, kVertexColour = 0x10;
constexpr reg32 kVertexClip = 0x14, kVertexU = 0x18, kVertexV = 0x1c;
constexpr reg32 kJustUnderOne = 0x3f7fff00, kWhite = 0xffffffff;
// The mouse's position (its four words to eax, edx, ebx and ecx), the view's
// corner (to eax and edx), a quad drawn (its corners eax, edx, ebx, ecx).
constexpr reg32 kViewCorner = 0x4dab10;
// The pointers' records (0x2c bytes: the texture, then the corners' u and
// v), which is shown.
constexpr reg32 kPointers = 0x8b41dc, kPointer = 0x563d3c, kPointerRecordBytes = 0x2c;

namespace
{

/* A corner's clip code (as native_graphics.cpp's shapeClipCode has it): 0x10
 * for its w at or past the far edge, else 8 above or 4 below, 1 left of or 2
 * right of the screen's rectangle; each a float's bits compared as integers,
 * a negative one as short. */
reg8 clipCodeOf(App* app, reg32 corner)
{
    const reg32 w = at(app, corner + kVertexW);
    if ((w & 0x80000000) || sreg32(w) >= sreg32(at(app, kClipFar)))
        return 0x10;
    reg8 code = 0;
    const reg32 y = at(app, corner + kVertexY);
    if ((y & 0x80000000) || sreg32(y) < sreg32(at(app, kClipTop)))
        code |= 8;
    else if (sreg32(y) > sreg32(at(app, kClipBottom)))
        code |= 4;
    const reg32 x = at(app, corner + kVertexX);
    if ((x & 0x80000000) || sreg32(x) < sreg32(at(app, kClipLeft)))
        code |= 1;
    else if (sreg32(x) > sreg32(at(app, kClipRight)))
        code |= 2;
    return code;
}

/* The mouse's pointer drawn in the race: where the mouse is, less the view's
 * corner, as a 32 by 32 quad of four corners in the frame (where the calls
 * are handed them) -- at depth 0, w just under 1, white -- its texture's
 * coordinates and the texture from the pointer's record, each corner's clip
 * code, the texture set (state 1) and the quad drawn.  False when the game
 * stopped. */
bool pointer(App* app, CPU& cpu)
{
    const GuestFrame frame(cpu, 0x18, 0x9c);
    const reg32 f = frame - 0x82;
    const reg32 mouse = f + 0x6e, view = f + 0x66;
    const reg32 corners[4] = {f - 0x1a, f + 6, f + 0x26, f + 0x46};
    if (!guest::direct<0x435310, controls::mouseRead>(app, cpu, mouse, mouse + 4, mouse + 8, mouse + 0xc))
        return false;
    cpu.eax = view;
    cpu.edx = view + 4;
    if (!guest::call(app, cpu, kViewCorner))
        return false;
    // The corners' x and y: the mouse's, and 32 on, each a float as stored.
    const float left = float(Float(sreg32(at(app, mouse + 4) - at(app, view))));
    storeFloat(app, corners[3] + kVertexX, left);
    for (const reg32 c : {3, 2, 1, 0})
        put(app, corners[c] + kVertexW, kJustUnderOne);
    for (const reg32 c : {2, 1, 0})
        put(app, corners[c] + kVertexX, at(app, corners[3] + kVertexX));
    const Float size = loadFloat(app, kPointerSize);
    storeFloat(app, corners[1] + kVertexX, fp::add(loadFloat(app, corners[3] + kVertexX), size));
    const float top = float(Float(sreg32(at(app, mouse + 8) - at(app, view + 4))));
    storeFloat(app, corners[3] + kVertexY, top);
    for (const reg32 c : {2, 1, 0})
        put(app, corners[c] + kVertexY, at(app, corners[3] + kVertexY));
    storeFloat(app, corners[2] + kVertexY, fp::add(size, loadFloat(app, corners[3] + kVertexY)));
    put(app, corners[2] + kVertexX, at(app, corners[1] + kVertexX));
    put(app, corners[3] + kVertexY, at(app, corners[2] + kVertexY));
    // The pointer's record: its texture, then each corner's u and v.
    const reg32 record = kPointers + at(app, kPointer) * kPointerRecordBytes;
    put(app, corners[3] + kVertexZ, 0);
    put(app, corners[0] + kVertexU, at(app, record + 4));
    put(app, corners[2] + kVertexZ, 0);
    put(app, corners[0] + kVertexV, at(app, record + 8));
    put(app, corners[1] + kVertexZ, 0);
    put(app, corners[1] + kVertexU, at(app, record + 0xc));
    put(app, corners[0] + kVertexZ, 0);
    put(app, corners[1] + kVertexV, at(app, record + 0x10));
    put(app, corners[3] + kVertexColour, kWhite);
    put(app, corners[2] + kVertexU, at(app, record + 0x14));
    put(app, corners[2] + kVertexColour, kWhite);
    put(app, corners[2] + kVertexV, at(app, record + 0x18));
    put(app, corners[1] + kVertexColour, kWhite);
    put(app, corners[3] + kVertexU, at(app, record + 0x1c));
    put(app, corners[0] + kVertexColour, kWhite);
    put(app, corners[3] + kVertexV, at(app, record + 0x20));
    for (const reg32 corner : corners)
        putByte(app, corner + kVertexClip, clipCodeOf(app, corner));
    cpu.ebx = corners[2];
    cpu.ecx = corners[3];
    if (!setRenderState(app, cpu, kTextureState, at(app, kPointers + at(app, kPointer) * kPointerRecordBytes)))
        return false;
    return drawCorners(app, cpu, kDrawQuad, corners[0], corners[1], corners[2], corners[3]);
}

}  // namespace

/* 0x434b60: pointer; every register but eax as it was. */
void drawPointer(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
    if (!pointer(app, cpu))
        return;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = edx;
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
constexpr reg32 kMessagesBytes = 0x24c8, kCopyBytes = 0x3f8;
constexpr reg32 kLineLength = 0, kLinesKept = 4, kChannelCount = 8, kMessagesKind = 0xc, kCopyStart = 0x10;
constexpr reg32 kScrolling = 0x20, kChannels = 0x28;
constexpr reg32 kChannelBytes = 0x494, kChannelKept = 0, kChannelFirst = 4, kChannelNext = 8, kChannelLines = 0xc;
constexpr reg32 kLineBytes = 0x74, kLineStart = 0, kLineFrom = 4, kLineCount = 8, kLineText = 0xc;
constexpr reg32 kCopyLineBytes = 0x65;
constexpr reg32 kMessagesAdded = 0x5f3fd8;  // the texts added, counted
constexpr reg32 kFillZero = 0x4e070c;

namespace
{

/* The game's allocate (0x4e1620, ea::reservememadr): `bytes`, of no name, aligned to 0x10. */
std::optional<reg32> allocate(App* app, CPU& cpu, reg32 bytes)
{
    constexpr reg32 kNoName = 0x537490, kAlignment = 0x10;
    return guest::direct<0x4e1620, ea::reserved>(app, cpu, kNoName, bytes, kAlignment);
}

/* The game's fill with zeros (0x4e070c): `bytes` at `block`. */
bool fillZero(App* app, CPU& cpu, reg32 block, reg32 bytes)
{
    cpu.eax = block;
    cpu.edx = bytes;
    return guest::call(app, cpu, kFillZero);
}

/* The game's memmove (0x4ea4f0, ea::memmove): `bytes` from `from` to `to`. */
bool moveMemory(App* app, CPU& cpu, reg32 from, reg32 to, reg32 bytes)
{
    return guest::direct<0x4ea4f0, ea::moveMemory>(app, cpu, to, from, bytes);
}

/* The game's free (0x4e1890, ea::purgememadr); eax what it gives back, as the code leaves it. */
bool freeBlock(App* app, CPU& cpu, reg32 block)
{
    const std::optional<reg32> freed = guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
    if (!freed)
        return false;
    cpu.eax = *freed;
    return true;
}

/* idiv: `value` (its sign extended) divided by `by`, the remainder. */
reg32 remainder(reg32 value, reg32 by)
{
    return reg32(std::int64_t(sreg32(value)) % sreg32(by));
}

/* The adapter of a messages' function whose code pushes and pops ebx, ecx
 * and edx: the result in eax. */
void keepingRegisters(CPU& cpu, const std::optional<reg32>& result, reg32 ebx, reg32 ecx, reg32 edx)
{
    if (!result)
        return;
    cpu.eax = *result;
    cpu.ebx = ebx;
    cpu.ecx = ecx;
    cpu.edx = edx;
    cpu.esp += 4;
}

}  // namespace

/* The messages' blocks allocated and cleared, their settings from the eight
 * words at `settings` (the line's length and the lines kept made larger by
 * its +0xc, the second if the lines kept are more than 1), the copy's count
 * the lines kept as given; in use.  The copy's block, as the code leaves it
 * in eax. */
std::optional<reg32> openChannels(App* app, CPU& cpu, reg32 settings)
{
    reg32 length = at(app, settings + 8), lines = at(app, settings + 4);
    const reg32 more = at(app, settings + 0xc);
    if (more != 0)
    {
        if (sreg32(lines) > 1)
            lines += more;
        else
            length += more;
    }
    const std::optional<reg32> messages = allocate(app, cpu, kMessagesBytes);
    if (!messages)
        return std::nullopt;
    put(app, kMessages, *messages);
    if (!fillZero(app, cpu, *messages, kMessagesBytes)
        || !moveMemory(app, cpu, settings, at(app, kMessages) + kChannelCount, 0x20))
        return std::nullopt;
    const reg32 block = at(app, kMessages);
    put(app, block + kLinesKept, lines);
    put(app, block + kLineLength, length);
    const std::optional<reg32> copy = allocate(app, cpu, kCopyBytes);
    if (!copy)
        return std::nullopt;
    put(app, kMessageCopy, *copy);
    if (!fillZero(app, cpu, *copy, kCopyBytes))
        return std::nullopt;
    const reg32 copyAt = at(app, kMessageCopy);
    put(app, copyAt, at(app, settings + 4));
    put(app, kMessagesOn, 1);
    return copyAt;
}

/* The messages out of use, their blocks freed.  eax as the last free leaves
 * it, or as it was. */
bool closeChannels(App* app, CPU& cpu)
{
    const reg32 messages = at(app, kMessages);
    put(app, kMessagesOn, 0);
    if (messages != 0 && !freeBlock(app, cpu, messages))
        return false;
    const reg32 copy = at(app, kMessageCopy);
    if (copy != 0 && !freeBlock(app, cpu, copy))
        return false;
    put(app, kMessages, 0);
    put(app, kMessageCopy, 0);
    return true;
}

namespace
{

/* Where line `line` of channel `channel` is. */
reg32 lineAt(App* app, reg32 channel, reg32 line)
{
    return at(app, kMessages) + kChannels + channel * kChannelBytes + kChannelLines + line * kLineBytes;
}

/* Line `line` of channel `channel` cleared: its words and its text's first
 * byte 0. */
void clearLine(App* app, reg32 channel, reg32 line)
{
    const reg32 to = lineAt(app, channel, line);
    put(app, to + kLineStart, 0);
    put(app, to + kLineFrom, 0);
    put(app, to + kLineCount, 0);
    putByte(app, to + kLineText, 0);
}

/* Channel `channel` cleared while the messages are in use: its lines, its
 * kept, first and next line 0 -- out of use the while.  What the code leaves
 * in eax: the messages' block, or the channel out of use. */
reg32 clearChannel(App* app, reg32 channel)
{
    const reg32 on = at(app, kMessagesOn);
    if (on == 0)
        return channel;
    put(app, kMessagesOn, 0);
    for (reg32 line = 0; sreg32(line) < sreg32(at(app, at(app, kMessages) + kLinesKept)); ++line)
        clearLine(app, channel, line);
    const reg32 messages = at(app, kMessages);
    const reg32 to = messages + channel * kChannelBytes + kChannels;
    put(app, to + kChannelKept, 0);
    put(app, to + kChannelFirst, 0);
    put(app, kMessagesOn, on);
    put(app, to + kChannelNext, 0);
    return messages;
}

}  // namespace

/* Every channel cleared.  The messages' block, as the code leaves it in eax. */
reg32 clearAllChannels(App* app)
{
    for (reg32 channel = 0; sreg32(channel) < sreg32(at(app, at(app, kMessages) + kChannelCount)); ++channel)
        clearChannel(app, channel);
    return at(app, kMessages);
}

namespace
{

/* What a channel's scrolling does: nothing but read the messages' block. */
reg32 channelScrolled(App* app)
{
    return at(app, kMessages);
}

/* The character (edx's low byte) added to channel `channel`'s next line,
 * while the messages are in use: at the line's length, for messages of kind
 * 1, the line starts again and its +4 moves on (of the line's length plus
 * one); for another kind the next line taken (cleared) and the first moved
 * on as they meet, scrolled, and the kept the first.  Its text ended after
 * it.  1, or 0 out of use. */
Added addCharacter(App* app, reg32 channel, reg32 edx)
{
    if (at(app, kMessagesOn) == 0)
        return {0, edx};
    const reg32 messages = at(app, kMessages);
    const reg32 offset = channel * kChannelBytes;
    const reg32 channelAt = offset + messages + kChannels;
    const reg32 line = messages + kChannels + offset + kChannelLines + at(app, channelAt + kChannelNext) * kLineBytes;
    const reg32 count = at(app, line + kLineCount);
    put(app, line + kLineCount, count + 1);
    putByte(app, line + kLineText + count, reg8(edx));
    if (at(app, messages + kMessagesKind) == 1)
    {
        edx = at(app, messages + kLineLength);
        if (sreg32(edx) < sreg32(at(app, line + kLineCount)))
            put(app, line + kLineCount, 0);
        const reg32 from = at(app, line + kLineFrom);
        if (from == at(app, line + kLineCount))
        {
            put(app, line + kLineFrom, from + 1);
            edx = remainder(from + 1, at(app, at(app, kMessages) + kLineLength) + 1);
            put(app, line + kLineFrom, edx);
        }
        put(app, line + kLineStart, at(app, line + kLineFrom));
    }
    else
    {
        if (at(app, messages + kLineLength) == at(app, line + kLineCount))
        {
            const reg32 next = at(app, channelAt + kChannelNext) + 1;
            put(app, channelAt + kChannelNext, next);
            const reg32 taken = remainder(next, at(app, messages + kLinesKept));
            put(app, channelAt + kChannelNext, taken);
            clearLine(app, channel, taken);
            const reg32 block = at(app, kMessages);
            const reg32 now = offset + block + kChannels;
            const reg32 first = at(app, now + kChannelFirst);
            if (at(app, now + kChannelNext) == first)
            {
                put(app, now + kChannelFirst, first + 1);
                put(app, now + kChannelFirst, remainder(first + 1, at(app, block + kLinesKept)));
            }
        }
        if (at(app, at(app, kMessages) + kScrolling) != 0)
            channelScrolled(app);
        const reg32 now = at(app, kMessages) + channel * kChannelBytes + kChannels;
        edx = at(app, now + kChannelFirst);
        put(app, now + kChannelKept, edx);
    }
    putByte(app, line + kLineText + at(app, line + kLineCount), 0);
    return {1, edx};
}

}  // namespace

/* The text at `text` added to channel `channel` a character at a time, up
 * to the line's length, the rest of the line filled with 0s; the additions
 * counted.  1, or 0 out of use or when a character could not be added; edx
 * as the code leaves it. */
Added addText(App* app, reg32 channel, reg32 text)
{
    reg32 edx = at(app, kMessagesOn);
    if (edx == 0)
        return {0, edx};
    put(app, kMessagesAdded, at(app, kMessagesAdded) + 1);
    reg32 added = 0;
    for (; byte(app, text) != 0 && sreg32(added) < sreg32(at(app, at(app, kMessages) + kLineLength)); ++added, ++text)
    {
        const Added a = addCharacter(app, channel, byte(app, text));
        edx = a.edx;
        if (a.result == 0)
            return a;
    }
    for (; sreg32(added) < sreg32(at(app, at(app, kMessages) + kLineLength)); ++added)
    {
        const Added a = addCharacter(app, channel, 0);
        edx = a.edx;
        if (a.result == 0)
            return a;
    }
    return {1, edx};
}

namespace
{

/* Channel `channel`'s lines copied out: for a copy of one line, as many of
 * the channel's text bytes from the copy's start as fit, in one or two
 * pieces; else each of its lines from the kept one on (of the lines kept,
 * round), 0x65 bytes apart, by the code's inlined strcpy.  The copy's block. */
std::optional<reg32> copyChannel(App* app, CPU& cpu, reg32 channel)
{
    const reg32 offset = channel * kChannelBytes;
    const reg32 count = at(app, at(app, kMessageCopy));
    if (count == 1)
    {
        const reg32 messages = at(app, kMessages);
        const reg32 wanted = at(app, messages + kCopyStart) + count;
        const reg32 start = at(app, offset + messages + kChannels + kChannelLines + kLineStart);
        reg32 piece = wanted;
        if (sreg32(wanted) < sreg32(start + wanted))
            piece = at(app, messages + kLineLength) - start + 1;
        const reg32 text = at(app, kMessages) + kChannels + offset + kChannelLines + kLineText;
        if (!moveMemory(app, cpu, text + start, at(app, kMessageCopy) + 4, piece))
            return std::nullopt;
        if (sreg32(piece) < sreg32(wanted)
            && !moveMemory(app, cpu, at(app, kMessages) + kChannels + offset + kChannelLines + kLineText,
                           at(app, kMessageCopy) + 4 + piece, wanted - piece))
            return std::nullopt;
    }
    else
    {
        const reg32 kept = at(app, offset + at(app, kMessages) + kChannels + kChannelKept);
        for (reg32 i = 0; sreg32(i) < sreg32(at(app, at(app, kMessageCopy))); ++i)
        {
            const reg32 messages = at(app, kMessages);
            const reg32 line = remainder(kept + i, at(app, messages + kLinesKept));
            reg32 from = channel * kChannelBytes + messages + kChannels + kChannelLines + line * kLineBytes + kLineText;
            reg32 to = at(app, kMessageCopy) + 4 + i * kCopyLineBytes;
            for (;;)
            {
                const reg8 c = byte(app, from++);
                putByte(app, to++, c);
                if (c == 0)
                    break;
            }
        }
    }
    return at(app, kMessageCopy);
}

}  // namespace

/* 0x436c40: openChannels from the settings at eax. */
void openMessages(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
    keepingRegisters(cpu, openChannels(app, cpu, cpu.eax), ebx, ecx, edx);
}

/* 0x436cf0: closeChannels. */
void closeMessages(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
    if (!closeChannels(app, cpu))
        return;
    keepingRegisters(cpu, cpu.eax, ebx, ecx, edx);
}

/* 0x436d40: clearAllChannels. */
void clearAllMessages(App* app, CPU& cpu)
{
    cpu.eax = clearAllChannels(app);
    cpu.esp += 4;
}

/* 0x436d60: clearChannel for channel eax. */
void clearMessages(App* app, CPU& cpu)
{
    cpu.eax = clearChannel(app, cpu.eax);
    cpu.esp += 4;
}

/* 0x436dc0: clearLine for line edx of channel eax; eax the channel's lines
 * and edx the line's offset among them, as the code leaves them. */
void clearMessageLine(App* app, CPU& cpu)
{
    const reg32 channel = cpu.eax, line = cpu.edx;
    const reg32 lines = at(app, kMessages) + kChannels + channel * kChannelBytes + kChannelLines;
    clearLine(app, channel, line);
    cpu.eax = lines;
    cpu.edx = line * kLineBytes;
    cpu.esp += 4;
}

/* 0x436e10: addText of the text at edx to channel eax. */
void addMessageText(App* app, CPU& cpu)
{
    const Added a = addText(app, cpu.eax, cpu.edx);
    cpu.eax = a.result;
    cpu.edx = a.edx;
    cpu.esp += 4;
}

/* 0x436e90: addCharacter of dl to channel eax. */
void addMessageCharacter(App* app, CPU& cpu)
{
    const Added a = addCharacter(app, cpu.eax, cpu.edx);
    cpu.eax = a.result;
    cpu.edx = a.edx;
    cpu.esp += 4;
}

/* 0x437080: copyChannel of channel eax. */
void copyMessageLines(App* app, CPU& cpu)
{
    const reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
    keepingRegisters(cpu, copyChannel(app, cpu, cpu.eax), ebx, ecx, edx);
}

/* 0x4371a0: channelScrolled (the code compares the block's kind with 1, for
 * flags no caller reads). */
void messagesScrolled(App* app, CPU& cpu)
{
    cpu.eax = channelScrolled(app);
    cpu.esp += 4;
}

/* 0x4371c0: nothing (an empty function). */
void messagesNothing(App*, CPU& cpu)
{
    cpu.esp += 4;
}

// The hash's seed: the whole product, its low 16 bits, the odd seed.
constexpr reg32 kHashProduct = 0x554f14, kHashSeed16 = 0x554f10, kHashOdd = 0x554f18;

/* A hash's seed from `seed`: it made odd and three times it made odd,
 * multiplied eight times by the first, kept to 16 bits but the last.  The
 * odd three times, as the code leaves it in eax. */
reg32 seedTheHash(App* app, reg32 seed)
{
    const reg32 odd = (seed & 1) ? seed : seed + 1;
    reg32 triple = seed * 3;
    if (!(triple & 1))
        ++triple;
    reg32 product = (odd * triple) & 0xffff;
    for (int i = 0; i < 7; ++i)
    {
        product *= odd;
        if (i < 6)
            product &= 0xffff;
    }
    put(app, kHashProduct, product);
    put(app, kHashSeed16, product & 0xffff);
    put(app, kHashOdd, odd);
    return triple;
}

/* 0x4371d0: seedTheHash of eax. */
void seedHash(App* app, CPU& cpu)
{
    cpu.eax = seedTheHash(app, cpu.eax);
    cpu.esp += 4;
}

}  // namespace collide
}  // namespace nfs3hp
