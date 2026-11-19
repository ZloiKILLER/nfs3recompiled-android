#include "game_settings.h"
#include "native_aidrive.h"
#include "native_collide.h"
#include "native_controls_input.h"
#include "native_controls_queue.h"
#include "native_controls_race.h"
#include "native_ea_files.h"
#include "native_ea_memory.h"
#include "native_frontend_loopb.h"
#include "native_frontend_menue.h"
#include "native_frontend_sima.h"
#include "native_frontend_simb.h"
#include "native_frontend_simd.h"
#include "native_game.h"
#include "native_graphics.h"
#include "native_handling.h"
#include "native_guest.h"
#include "native_raceload.h"
#include "native_racestart.h"
#include "native_track.h"
#include "native_trackdata.h"
#include "native_vertices.h"
#include "native_watcom.h"
#include "thrash_calls.h"
#include "native_render.h"

#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>

/* The game's collisions -- car with car, car with the track's walls, car with
 * the track's objects -- the COLL file of a track's objects, and the drawing
 * code that follows them in nfs3.exe: render states, the software
 * renderer's blend, the polygon lists' drawers, the race's mouse pointer, and
 * the text lines of the race's messages.  The game's code calls each by the name it has here.
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
 * Each is a typed core -- named parameters, a result, C++ locals -- with a
 * thunk under the game's name for the dispatch and the callers still on
 * registers, which leaves eax and the x87's stack as the game's code does (a
 * fcomp's status word in ax where it shows), the registers it was handed no
 * argument in as the caller had them.  A callee is handed its argument
 * registers, every other register kept across the call.  The x87's
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
namespace decompiled
{
void depthOffsetsSet(nfs::App* app);  // native_decompiled.cpp's, 0x4c7a60
}

namespace collide
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using nfs::Float;
using nfs::u16;
using nfs::u32;
using nfs::u8;
using nfs::s32;
using App = nfs::App;
using ThreadState = nfs::ThreadState;
using i32 = std::int32_t;

// The two cars' frames, as findContact and its tests share them.
constexpr u32 kAxisDots = 0x5efebc;        // nine floats: the first car's axes . the second's
constexpr u32 kSecondExtents = 0x5efee0;   // nine: those times the second car's half sizes
constexpr u32 kFirstExtents = 0x5efe90;    // nine: times the first car's
constexpr u32 kBetween = 0x5efe60;         // six: the line between them in either frame
constexpr u32 kContactNormal = 0x5efe78;   // three: contactNormal's
constexpr u32 kContactPoint = 0x5efe84;    // three: where the tests found the contact
constexpr u32 kCornerX = 0x5eff04, kCornerY = 0x5efeb4, kCornerZ = 0x5efeb8;
// The cars tested against each other this frame (addCarToList), and their count.
constexpr u32 kCarList = 0x5efe10, kCarCount = 0x5537bc;
// The track's polygons' records: 0x24 bytes each.
constexpr u32 kPolygonRecords = 0x5dd948;
// A car's fields as the collisions read them.
constexpr u32 kCarPosition = 0x98, kCarForward = 0xc0, kCarRight = 0xd8, kCarHalfLength = 0x114;
constexpr u32 kCarHalfWidth = 0x11c, kCarRadius = 0x120, kCarFlags = 0x200;
// A car's fields the walls and the objects read and set: its track position, its speed,
// how near it is tested, the collision's sound (its loudness, kind, position).
constexpr u32 kCarPlace = 0xc, kCarSpeedSize = 0xb8, kCarObjectReach = 0x15c;
constexpr u32 kCarSoundLoudness = 0x168, kCarSoundExtra = 0x16c, kCarSoundKind = 0x170, kCarSoundAt = 0x178;
// The x87's constants.
constexpr u32 kFifteen = 0x5373fc, kQuarter = 0x537404, kFixedScale = 0x53740c;
constexpr u32 kWallScale = 0x537414, kWallMarginRace = 0x53741c, kWallMarginOther = 0x537424;  // -2, -0.6
constexpr u32 kSpeedStep = 0x53742c, kToFixed = 0x537434, kPolygonScale = 0x53743c;
constexpr u32 kObjectScale = 0x537444, kObjectQuarter = 0x53744c, kObjectTwo = 0x537450;
constexpr u32 kObjectToFixed = 0x537454, kObjectHalf = 0x53745c, kObjectSpeed = 0x537464;
constexpr u32 kDepthNear = 0x537474, kDepthFar = 0x53747c, kPointerSize = 0x53748c;
// The wall test's tables in nfs3.exe's code: three zero floats twice, the
// four edges' bits.
constexpr u32 kZeroVector = 0x42dd30, kZeroVector2 = 0x42dd3c, kEdgeBits = 0x42dd48;

// What is called by address though another typed file's, for what it leaves
// in the registers beyond what its core gives back: the boxes' tests, the
// cars' response (its eax is collideCars'), a vector normalized where its eax
// is againstWalls', the line between the cars (vectors less another: its ebx
// and ecx are pushed into the tests' frames).
constexpr u32 kFaceInFirst = 0x42dd60, kFaceInSecond = 0x42e010;
constexpr u32 kEdgeInFirst = 0x42e2c0, kEdgeInSecond = 0x42e4f0;
constexpr u32 kCarsRespond = 0x42f280;
constexpr u32 kSubtractVectors = 0x4e0000;

/* A loop's safepoint, where the game's code has one: the guest's context
 * handed over when another thread waits for it. */
void safepoint(App* app, nfs::ThreadState& ts)
{
    if (app->contextWanted())
        app->yieldContext(ts);
}

/* A native_frontend_* file's typed core called as C++ (guest::direct): nothing (false) when the
 * game stopped in it (its calls into the game throw fe::Terminated), every register but eax kept.
 * A u32 result is the eax the function leaves, set to it; after any other, eax is left alone. */
template <u32 Address, auto Core, typename... Args>
auto feCore(App* app, ThreadState& ts, Args... args)
{
    const auto run = [&] {
        return guest::direct<Address, Core>(app, ts, args...);
    };
    using Result = decltype(run());
    const auto spoil = [&] {};
    if constexpr (std::is_void_v<Result>)
    {
        try
        {
            run();
        }
        catch (const fe::Terminated&)
        {
            return false;
        }
        spoil();
        return !ts.terminate;
    }
    else
    {
        try
        {
            const Result result = run();
            if constexpr (std::is_same_v<Result, nfs::u32>)
                (void)result;  // a menu core's result: the eax it leaves is no longer set (no caller reads it)
            else
                spoil();
            return ts.terminate ? std::optional<Result>() : std::optional<Result>(result);
        }
        catch (const fe::Terminated&)
        {
            return std::optional<Result>();
        }
    }
}

u16 half(App* app, u32 address)
{
    return app->getMemory<u16>(address);
}

void putHalf(App* app, u32 address, u16 value)
{
    app->getMemory<u16>(address) = value;
}

Float loadFloat(App* app, u32 address)
{
    return Float(app->getMemory<float>(address));
}

void storeFloat(App* app, u32 address, const Float& value)
{
    app->getMemory<float>(address) = float(value);
}

Float loadDouble(App* app, u32 address)
{
    return Float(app->getMemory<double>(address));
}

/* A float from its bits. */
Float floatAt(u32 bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return Float(f);
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
constexpr u16 kC0 = 0x100, kC3 = 0x4000;

u16 fcom(const Float& a, const Float& b)
{
    return fp::compare(a, b);
}

/* A float's size either way, as the collisions work it out: its bits when 0
 * is below it (a NaN too, fcom's below), else it negated. */
float positive(float value)
{
    return fBelow(0.0, Float(value)) ? value : float(-Float(value));
}

/* The bits of a float. */
u32 bitsOf(float value)
{
    u32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

/* The dot product (vertices', 0x4e01f0) of the vectors at a and b, as the
 * x87 holds it. */
std::optional<Float> dot(App* app, ThreadState& ts, u32 a, u32 b)
{
    const double product = guest::direct<0x4e01f0, vertices::dotProduct>(app, ts, a, b);
    if (ts.terminate)
        return std::nullopt;
    return Float(product);
}

/* The registers a function of the game's pushes on entry, written where it
 * pushes them -- below esp, where only what runs later reads them.  For the
 * car-to-car functions: raceload::carsCollide, which collideCars calls after
 * them, reads its cross product's vector before the cross product
 * (0x4e0160, which writes nothing) sets it, so it reads the words pushCarsApart
 * and findContact pushed there. */
struct Pushed
{
    Pushed(std::initializer_list<u32> values) : values(values) {}
    std::initializer_list<u32> values;
    void write(App* app, const ThreadState& ts) const
    {
        u32 to = ts.sp;
        for (const u32 value : values)
            put(app, to -= 4, value);
    }
};

/* Room on the guest's stack for what a callee is handed the address of: the
 * game's frame (`pushed` bytes of registers under esp on entry, then
 * `bytes`), esp under it while the core runs and back after. */
class GuestFrame
{
public:
    GuestFrame(ThreadState& ts, u32 pushed, u32 bytes) : ts(ts), esp(ts.sp), base(ts.sp - pushed)
    {
        ts.sp = base - bytes;
    }
    ~GuestFrame() { ts.sp = esp; }
    u32 operator-(u32 offset) const { return base - offset; }

private:
    ThreadState& ts;
    const u32 esp;
    const u32 base;
};

/* The corner test at `test` (0x42dd60, 0x42e010, 0x42e2c0 or 0x42e4f0) of the box a against b, the three signs
 * the registers and the stack's word hand it: 1 inside, else 0. */
std::optional<u32> cornerTested(App* app, ThreadState& ts, u32 test, u32 a, u32 b, u32 first, u32 second,
                                  u32 third)
{
    switch (test)
    {
    case kFaceInFirst:
        return guest::direct<0x42dd60, raceload::cornerIn>(app, ts, a, b, std::int32_t(first), std::int32_t(second),
                                                         std::int32_t(third))
                   ? 1u
                   : 0u;
    case kFaceInSecond:
        return guest::direct<0x42e010, raceload::cornerInBack>(app, ts, a, b, std::int32_t(first),
                                                             std::int32_t(second), std::int32_t(third))
                   ? 1u
                   : 0u;
    case kEdgeInFirst:
        return guest::direct<0x42e2c0, raceload::cornerIn2>(app, ts, a, b, std::int32_t(first), std::int32_t(second),
                                                          std::int32_t(third))
                   ? 1u
                   : 0u;
    default:
        return guest::direct<0x42e4f0, raceload::cornerInBack2>(app, ts, a, b, std::int32_t(first),
                                                              std::int32_t(second), std::int32_t(third))
                   ? 1u
                   : 0u;
    }
}

/* A call by address (a callee with no typed core) as the code makes it: `registers` into eax, edx,
 * ebx and ecx in turn, `words` pushed (the first lowest) and taken off after it; every register but
 * eax as it was before after it.  eax as the callee leaves it; nullopt when the game stopped. */
std::optional<u32> callWith(App* app, ThreadState& ts, u32 target, std::initializer_list<u32> registers,
                              std::initializer_list<u32> words = {})
{
    return guest::pointerCall(app, ts, target, registers, words);
}

}  // namespace

nfs::u32 findContact(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 contactNormal(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 pushCarsApart(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 separateCars(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 polygonEdgeNormal(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 clearOfWall(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carAgainstWalls(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 collideCars(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 clearCarList(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 objectInReach(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carAgainstObjects(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 addCarToList(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 carCollisions(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 distanceSquared(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 nearestPoint(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 openCollFile(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 closeCollFile(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 collFileSize(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 readCollFile(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 collFileCount(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 collChunk(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 playerControls(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 forgetRenderStates(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 statesForText(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 statesForPictures(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 statesForHud(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 statesForScreens(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 passLayerNothing(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 passSecondLayer(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 passFirstLayer(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 makeBlendTables(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 blendSpan(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 subdivisionDepth(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 drawTexturedTriangles(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 drawFlatTriangles(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 drawTrianglesByZ(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 drawQuadsByZ(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 drawFlatTrianglesAgain(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 drawColouredTriangles(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 stateRecord(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 drawRecords(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 raceTextFrame(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 setMirrored(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 drawMirroredRecords(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 countFrame(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 raceViewDistance(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 raceFrameEnd(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 raceFrameNothing(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 raceEndNothing(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 raceStartDrawing(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 raceStartNothing(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 drawPointer(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 openMessages(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 closeMessages(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 clearAllMessages(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 clearMessages(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 clearMessageLine(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 addMessageText(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 addMessageCharacter(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 copyMessageLines(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 messagesScrolled(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 messagesNothing(App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 seedHash(App* app, nfs::ThreadState& ts, nfs::Args& args);

/* Whether, and how, the boxes of cars a and b touch -- the first's six
 * faces' centres and then its eight corners are tried, each point's signs in
 * ebx, ecx and on the stack (the tests are stdcall, ret 4): 1 a face's
 * centre in the first car's box, 2 in the second's, 3 a corner in the
 * first's, 4 in the second's; 0 none.  The registers the code pushes (ebx,
 * ecx, esi, edi, ebp, `pushed`) written where it pushes them: see Pushed. */
std::optional<u32> contactKind(App* app, ThreadState& ts, u32 a, u32 b, const Pushed& pushed)
{
    struct Probe
    {
        u32 test, pushed, ebx, ecx, kind;
    };
    static const Probe probes[] = {
        {kFaceInFirst, 0, 1, 0, 1}, {kFaceInFirst, 0, 0, 1, 1}, {kFaceInFirst, 1, 0, 0, 1},
        {kFaceInFirst, 0, u32(-1), 0, 1}, {kFaceInFirst, 0, 0, u32(-1), 1}, {kFaceInFirst, u32(-1), 0, 0, 1},
        {kFaceInSecond, 0, 1, 0, 2}, {kFaceInSecond, 0, 0, 1, 2}, {kFaceInSecond, 1, 0, 0, 2},
        {kFaceInSecond, 0, u32(-1), 0, 2}, {kFaceInSecond, 0, 0, u32(-1), 2}, {kFaceInSecond, u32(-1), 0, 0, 2},
        {kEdgeInFirst, 1, 1, 1, 3}, {kEdgeInFirst, u32(-1), 1, 1, 3},
        {kEdgeInFirst, 1, 1, u32(-1), 3}, {kEdgeInFirst, u32(-1), 1, u32(-1), 3},
        {kEdgeInFirst, 1, u32(-1), 1, 3}, {kEdgeInFirst, u32(-1), u32(-1), 1, 3},
        {kEdgeInFirst, 1, u32(-1), u32(-1), 3}, {kEdgeInFirst, u32(-1), u32(-1), u32(-1), 3},
        {kEdgeInSecond, 1, 1, 1, 4}, {kEdgeInSecond, u32(-1), 1, 1, 4},
        {kEdgeInSecond, 1, 1, u32(-1), 4}, {kEdgeInSecond, u32(-1), 1, u32(-1), 4},
        {kEdgeInSecond, 1, u32(-1), 1, 4}, {kEdgeInSecond, u32(-1), u32(-1), 1, 4},
        {kEdgeInSecond, 1, u32(-1), u32(-1), 4}, {kEdgeInSecond, u32(-1), u32(-1), u32(-1), 4},
    };
    pushed.write(app, ts);
    const GuestFrame frame(ts, 0x14, 0);  // as deep as the code's
    for (const Probe& probe : probes)
    {
        const std::optional<u32> hit = cornerTested(app, ts, probe.test, a, b, probe.ebx, probe.ecx, probe.pushed);
        if (!hit)
            return std::nullopt;
        if (*hit != 0)
            return probe.kind;
    }
    return 0;
}

/* 0x42f550: contactKind of car eax and car edx. */
nfs::u32 findContact(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> kind =
        contactKind(app, ts, args.eax, args.edx, {args.ebx, args.ecx, args.esi, args.edi, args.ebp});
    if (!kind)
        return args.eax;
    args.eax = *kind;
    ts.sp += 4;
    return args.eax;
}

namespace
{

/* |a . b|, as contactNormal works it out: the dot product once to see its
 * sign, again for the value, turned round if 0 is not below the first, as a
 * float. */
std::optional<float> absoluteDot(App* app, ThreadState& ts, u32 a, u32 b)
{
    const std::optional<Float> first = dot(app, ts, a, b);
    if (!first)
        return std::nullopt;
    const bool zeroBelow = fBelow(0.0, *first);
    const std::optional<Float> again = dot(app, ts, a, b);
    if (!again)
        return std::nullopt;
    return float(zeroBelow ? *again : -*again);
}

/* A vector of three floats in the guest's memory turned round: each word's
 * sign flipped, its top byte at a time, in the order the code writes them. */
void turnRound(App* app, u32 vector, std::initializer_list<u32> order)
{
    for (const u32 word : order)
        putByte(app, vector + 4 * word + 3, byte(app, vector + 4 * word + 3) ^ 0x80);
}

// A car's speed (three floats), as contactNormal reads it; its axes.
constexpr u32 kCarSpeed = 0xa4, kCarUp = 0xcc;

/* The normal of the contact findContact found between cars a and b, into
 * kContactNormal, in the frame of `car` -- b, or a for `second` 0 -- from
 * the speeds of `mover` (the other): when they move fast apart (the largest
 * of their speeds' differences, each a float, above 15), the axis of `car`
 * along which the mover's speed goes most (each axis turned round first by
 * its corner offset's sign, in the frame, where the dot products are handed
 * it); else the axis of `car` whose box the contact point is least inside,
 * turned round by its offset's sign.  An axis is turned for `second` where 0
 * is below the offset (a NaN too), else where 0 is above it.  What the code
 * leaves in eax: the normal's third word, or what the sign bytes went
 * through. */
std::optional<u32> normalOfContact(App* app, ThreadState& ts, u32 a, u32 b, u32 second)
{
    const bool flag = second != 0;
    const u32 mover = flag ? a : b, car = flag ? b : a;
    float differences[3];
    for (u32 i = 0; i < 3; ++i)
        differences[i] = positive(
            float(fp::sub(loadFloat(app, mover + kCarSpeed + 4 * i), loadFloat(app, car + kCarSpeed + 4 * i))));
    float largest = differences[0];
    if (fAbove(differences[1], largest))
        largest = differences[1];
    if (fAbove(differences[2], largest))
        largest = differences[2];
    const auto turn = [&](u32 offset) {
        const Float o = loadFloat(app, offset);
        return flag ? fBelow(0.0, o) : fAbove(0.0, o);
    };
    const auto copy = [&](u32 to, u32 from) {
        for (u32 i = 0; i < 12; i += 4)
            put(app, to + i, at(app, from + i));
    };
    if (fAbove(largest, loadDouble(app, kFifteen)))
    {
        const GuestFrame frame(ts, 0x10, 0x7c);
        const u32 speed = frame - 0x7c, forward = frame - 0x64, up = frame - 0x58, right = frame - 0x70;
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
        const std::optional<float> alongForward = absoluteDot(app, ts, forward, speed);
        if (!alongForward)
            return std::nullopt;
        const std::optional<float> alongUp = absoluteDot(app, ts, up, speed);
        if (!alongUp)
            return std::nullopt;
        const std::optional<float> alongRight = absoluteDot(app, ts, right, speed);
        if (!alongRight)
            return std::nullopt;
        u32 chosen = right;
        if (fAbove(*alongForward, *alongUp) && fAbove(*alongForward, *alongRight))
            chosen = forward;
        else if (fAbove(*alongUp, *alongRight))
            chosen = up;
        copy(kContactNormal, chosen);
        return at(app, kContactNormal + 8);
    }
    // How far inside each of car's half sizes the corner is.
    float inside[3];
    static const u32 offsets[] = {kCornerX, kCornerY, kCornerZ};
    for (u32 i = 0; i < 3; ++i)
    {
        const bool above = fAbove(0.0, loadFloat(app, offsets[i]));
        const Float size = loadFloat(app, car + kCarHalfLength + 4 * i);
        inside[i] = float(above ? fp::add(size, loadFloat(app, offsets[i])) : fp::sub(size, loadFloat(app, offsets[i])));
    }
    u32 axis = 2;
    if (fBelow(inside[0], inside[1]) && fBelow(inside[0], inside[2]))
        axis = 0;
    else if (fBelow(inside[1], inside[2]))
        axis = 1;
    copy(kContactNormal, car + kCarForward + 0xc * axis);
    if (!turn(offsets[axis]))
        return 0;
    // The sign bytes of axis 1 go through al and ah.
    const auto flipped = [&](u32 word) { return u32(byte(app, kContactNormal + 4 * word + 3) ^ 0x80); };
    if (axis == 1)
    {
        const u32 low = flipped(1), high = flipped(2);
        turnRound(app, kContactNormal, {1, 0, 2});
        return high << 8 | low;
    }
    turnRound(app, kContactNormal, {0, 2, 1});
    return 1;
}

}  // namespace

/* 0x42f8c0: normalOfContact for car eax and car edx, by ebx. */
nfs::u32 contactNormal(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = normalOfContact(app, ts, args.eax, args.edx, args.ebx);
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}


// A car's share of the masses, as the collisions weigh it.
constexpr u32 kCarMassShare = 0xb0;

/* Cars a and b pushed apart along the direction `direction` (three floats'
 * bits, normalized first in the frame, 0x4972f0), each by a quarter of the
 * direction times the other's share of their masses -- the first back, the
 * second on (0x4e0050, 0x4dffb0).  The vectors are the frame's, as the code
 * hands them over; the registers it pushes (ebx, ecx, esi, ebp, `pushed`)
 * are written where it pushes them, and the length of `on` where it keeps
 * it: what the caller calls next (0x42f280) reads one of those words as a
 * local of its own it never sets.  False when the game stopped. */
bool pushApart(App* app, ThreadState& ts, u32 a, u32 b, const u32 (&direction)[3], const Pushed& pushed)
{
    pushed.write(app, ts);
    const GuestFrame frame(ts, 0x10, 0x30);
    const u32 along = frame - 0x24, back = frame - 0x18, on = frame - 0x30;
    for (u32 i = 0; i < 3; ++i)
        put(app, along + 4 * i, direction[i]);
    // Its length the code keeps on the x87's stack until an fstp st(0): never used.
    guest::direct<0x4972f0, vertices::normaliseVector>(app, ts, along);
    if (ts.terminate)
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
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, ts, 1, along, float(-backBy), back);
    if (ts.terminate)
        return false;
    guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, ts, 1, along, onBy, on);
    if (ts.terminate)
        return false;
    guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, ts, 1, a + kCarPosition, back, a + kCarPosition);
    if (ts.terminate)
        return false;
    guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, ts, 1, b + kCarPosition, on, b + kCarPosition);
    return !ts.terminate;
}

/* 0x42fe10: pushApart for car eax and car edx along the direction on the
 * stack; 1; ret 0xc. */
nfs::u32 pushCarsApart(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 direction[3] = {at(app, ts.sp + 4), at(app, ts.sp + 8), at(app, ts.sp + 0xc)};
    if (!pushApart(app, ts, args.eax, args.edx, direction, {args.ebx, args.ecx, args.esi, args.ebp}))
        return args.eax;
    args.eax = 1;
    ts.sp += 4 + 0xc;
    return args.eax;
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
std::optional<u32> separateTwo(App* app, ThreadState& ts, u32 a, u32 b)
{
    const GuestFrame frame(ts, 0x14, 0x1c);
    const u32 line = frame - 0x1c;
    // The nine dot products of the first car's axes with the second's.
    for (u32 row = 0; row < 3; ++row)
        for (u32 column = 0; column < 3; ++column)
        {
            const std::optional<Float> d = dot(app, ts, a + kCarForward + 0xc * row, b + kCarForward + 0xc * column);
            if (!d)
                return std::nullopt;
            storeFloat(app, kAxisDots + 0xc * row + 4 * column, *d);
        }
    // Those times the second car's half sizes (p) and the first's (q), in
    // the code's order, stored as the code's fxch and fstp leave them.
    const auto axisDot = [&](u32 i) { return loadFloat(app, kAxisDots + 4 * i); };
    Float p[9], q[9];
    for (u32 i = 0; i < 7; ++i)
        p[i] = fp::mul(axisDot(i), loadFloat(app, b + kCarHalfLength + 4 * (i % 3)));
    storeFloat(app, kSecondExtents + 4, p[1]);
    p[7] = fp::mul(axisDot(7), loadFloat(app, b + 0x118));
    storeFloat(app, kSecondExtents + 8, p[2]);
    p[8] = fp::mul(axisDot(8), loadFloat(app, b + 0x11c));
    storeFloat(app, kSecondExtents + 0xc, p[3]);
    for (u32 i = 0; i < 6; ++i)
    {
        q[i] = fp::mul(axisDot(i), loadFloat(app, a + kCarHalfLength + 4 * (i / 3)));
        storeFloat(app, i < 5 ? kSecondExtents + 0x10 + 4 * i : kFirstExtents, i < 5 ? p[4 + i] : q[0]);
    }
    q[6] = fp::mul(axisDot(6), loadFloat(app, a + 0x11c));
    storeFloat(app, kFirstExtents + 4, q[1]);
    q[7] = fp::mul(axisDot(7), loadFloat(app, a + 0x11c));
    storeFloat(app, kFirstExtents + 8, q[2]);
    q[8] = fp::mul(axisDot(8), loadFloat(app, a + 0x11c));
    for (u32 i = 3; i < 9; ++i)
        storeFloat(app, kFirstExtents + 4 * i, q[i]);
    storeFloat(app, kSecondExtents, p[0]);
    u32 result = 0;
    for (u32 pushes = 0;;)
    {
        // The line from the first car to the second, into either frame; what
        // the call leaves in ebx and ecx the code pushes in contactKind's.
        // (the subtraction leaves ebx and ecx as handed)
        const u32 leftEbx = a + kCarPosition, leftEcx = line;
        guest::direct<0x4e0000, vertices::offsetVectorList<false>>(app, ts, 1, b + kCarPosition, a + kCarPosition, line);
        if (ts.terminate)
            return std::nullopt;
        static const u32 axes[] = {0xc0, 0xcc, 0xd8};
        for (u32 i = 0; i < 6; ++i)
        {
            const std::optional<Float> d = dot(app, ts, (i < 3 ? a : b) + axes[i % 3], line);
            if (!d)
                return std::nullopt;
            storeFloat(app, kBetween + 4 * i, *d);
        }
        std::optional<u32> kind;
        {
            const GuestFrame slot(ts, 0, 4);  // the call's return address, as the code has it
            kind = contactKind(app, ts, a, b, {leftEbx, leftEcx, a, b, frame - 0});
        }
        if (!kind)
            return std::nullopt;
        if (*kind == 0)
            return result;
        safepoint(app, ts);
        if (*kind == 3 || *kind == 4)
        {
            const GuestFrame slot(ts, 0, 4);
            if (!normalOfContact(app, ts, a, b, *kind == 4 ? 1 : 0))
                return std::nullopt;
        }
        ++pushes;
        const u32 direction[3] = {at(app, line), at(app, line + 4), at(app, line + 8)};
        {
            // Its three words as the code pushes them, and the return address's slot.
            Pushed{direction[2], direction[1], direction[0]}.write(app, ts);
            const GuestFrame slot(ts, 0, 4 + 0xc);
            if (!pushApart(app, ts, a, b, direction, {pushes, direction[0], a, frame - 0}))
                return std::nullopt;
        }
        result = 1;
        if (s32(pushes) >= 4)
            return result;
    }
}

}  // namespace

/* 0x42fee0: separateTwo for car eax and car edx. */
nfs::u32 separateCars(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> r = separateTwo(app, ts, args.eax, args.edx);
    if (!r)
        return args.eax;
    args.eax = *r;
    ts.sp += 4;
    return args.eax;
}

/* The fixed point's multiply, as the code makes it -- imul; shl edx, 16;
 * shr eax, 16; adc eax, edx -- of a by b, rounded by the bit shr shifts
 * out. */
u32 fixedTimes(u32 a, u32 b)
{
    const std::int64_t product = std::int64_t(s32(a)) * s32(b);
    const u32 low = u32(product);
    const u32 high = u32(std::uint64_t(product) >> 32) << 16;
    return (low >> 16) + high + ((low >> 15) & 1);
}

// The corners of a track position's polygon, 16.16 (x, y, z each).
constexpr u32 kPlaceCorners = 0x24;

/* The wall along edge `edge` (1, 2, 4 or 8) of the track position at
 * `place`, as the code works it out in 16.16: the edge's run and rise (from
 * the polygon's corners) and 0 between them, normalized in the frame
 * (0x4970c0), its cross product with up (0, 1, 0) -- each term a 16.16
 * multiply, the first by edx as the normalizing leaves it -- into floats at
 * `out` (the third term first).  Another edge: the frame's run and rise as
 * they were on the guest's stack.  The second term in 16.16, as the code
 * leaves it in eax. */
std::optional<u32> edgeNormal(App* app, ThreadState& ts, u32 edge, u32 place, u32 out)
{
    const GuestFrame frame(ts, 0x10, 0x28);
    const u32 vector = frame - 0x10;
    put(app, vector + 4, 0);
    const auto corner = [&](u32 offset) { return at(app, place + offset); };
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
    // The first multiply is by the edx the call leaves, which is the 0 handed over (the core never
    // touches edx).
    if (!feCore<0x4970c0, frontend::simd::fixedNormalise>(app, ts, vector))
        return std::nullopt;
    const u32 leftEdx = 0;
    constexpr u32 kOne = 0x10000;  // up's y; its x and z 0
    const u32 x = at(app, vector), y = at(app, vector + 4), z = at(app, vector + 8);
    const u32 first = fixedTimes(y, leftEdx) - fixedTimes(z, kOne);
    const u32 second = fixedTimes(z, 0) - fixedTimes(x, 0);
    const u32 third = fixedTimes(x, kOne) - fixedTimes(y, 0);
    const Float scale = loadDouble(app, kFixedScale);
    const Float thirdScaled = fp::mul(Float(s32(third)), scale);
    const Float firstScaled = fp::mul(Float(s32(first)), scale);
    const Float secondScaled = fp::mul(scale, Float(s32(second)));
    storeFloat(app, out + 8, thirdScaled);
    storeFloat(app, out, firstScaled);
    storeFloat(app, out + 4, secondScaled);
    return second;
}

/* 0x430210: edgeNormal of edge eax of the track position at ebx into edx. */
nfs::u32 polygonEdgeNormal(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> second = edgeNormal(app, ts, args.eax, args.ebx, args.edx);
    if (!second)
        return args.eax;
    args.eax = *second;
    ts.sp += 4;
    return args.eax;
}

// A track position's record, as clearOfWall reads it: its polygon's index
// (+0x10), whether its corners are its own (+0x14, else the polygon's
// record's), its polygon (+0x18, 0 none), something of its own (+0x6c).
constexpr u32 kPlacePolygon = 0x10, kPlaceOwnCorners = 0x14, kPlaceHasPolygon = 0x18, kPlaceThing = 0x6c;
constexpr u32 kPolygonRecordBytes = 0x24;
constexpr u32 kCarY = 0x9c, kCarStage = 0x164;

/* Whether car `car` is clear of the wall of the track position `place`: 1
 * for no polygon there, a polygon with something of its own, of kind 0 or
 * 14; else whether the car is below the corners' lowest (its four corners,
 * or the polygon record's first four times when the corners are not its
 * own) plus the margin -- -0.6, -2 for a car with a stage (+0x164).  The lowest is kept
 * as a float, as the code stores it, each against the next as doubles. */
std::optional<u32> clear(App* app, ThreadState& ts, u32 car, u32 place)
{
    const GuestFrame frame(ts, 0x14, 0x30);  // as deep as the code's
    const u32 kind = guest::direct<0x4207e0, trackdata::placeSurface>(app, ts, place, 1) & 0x3f;
    if (ts.terminate)
        return std::nullopt;
    const u32 polygon = at(app, place + kPlaceHasPolygon);
    if (polygon == 0)
        return 1;
    const u32 corners = place + 0x54;
    if (corners == 0 || at(app, place + kPlaceThing) != 0 || kind == 0 || kind == 0xe)
        return 1;
    const u32 carY = at(app, car + kCarY);
    float lowest = 0;
    for (u32 i = 0; i < 4; ++i)
    {
        u32 from;
        if (at(app, place + kPlaceOwnCorners) == 0)
        {
            safepoint(app, ts);
            from = at(app, place + kPlacePolygon) * kPolygonRecordBytes + at(app, kPolygonRecords);
        }
        else
            from = i * 0xc + place + kPlaceCorners;
        safepoint(app, ts);
        const Float height = fp::mul(Float(s32(at(app, from + 4))), loadDouble(app, kWallScale));
        if (i == 0 || fAbove(Float(lowest), height))
            lowest = float(height);
    }
    const u32 margin = half(app, car + kCarStage) != 0 ? kWallMarginRace : kWallMarginOther;
    const Float top = fp::add(Float(lowest), loadDouble(app, margin));
    return fBelow(floatAt(carY), top) ? 1u : 0u;
}

/* 0x430350: clear of car eax against the track position edx. */
nfs::u32 clearOfWall(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> c = clear(app, ts, args.eax, args.edx);
    if (!c)
        return args.eax;
    args.eax = *c;
    ts.sp += 4;
    return args.eax;
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
// All as C++ (guest::direct; the turn simb's, feCore) but the jolt, whose eax
// againstWalls leaves.
constexpr u32 kWallJolt = 0x4aa4f4;
constexpr u32 kCarSlice = 0x1c, kCarTurn = 0xe4, kCarHalfHeight = 0x118;
constexpr u32 kPlaceWords = 0x1e;  // a track position's record, copied whole

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
std::optional<u32> againstWalls(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x18, 0x23c);
    const u32 f = frame - 0x82;
    const u32 normal = f + 0x12, step = f - 0x12, axes = f - 0x6a, turned = f - 0x46, turnWord = f + 0x5a;
    const u32 edgesOut = f + 0x5e, corners = f - 0xca, moved = f + 0x42, fixed = f + 0x1e;
    const u32 place = f - 0x1ba, neighbour = f - 0x142, edgeOut = f + 0x4e, surface = f - 6;
    const u32 polygonNormal = f + 6, toCar = f + 0x36;
    const auto copyWords = [&](u32 to, u32 from, u32 words) {
        for (u32 w = 0; w < words; ++w)
            put(app, to + 4 * w, at(app, from + 4 * w));
    };
    copyWords(normal, kZeroVector, 3);
    copyWords(step, kZeroVector2, 3);
    put(app, edgesOut, 0);
    copyWords(axes, car + kCarForward, 9);
    put(app, turnWord, 0);
    // A thirty-second of the speed, and of the turn onto the stack.
    std::optional<u32> turnedAny;
    {
        const Float by = loadFloat(app, kSpeedStep);
        storeFloat(app, step, fp::mul(loadFloat(app, car + kCarSpeed), by));
        storeFloat(app, step + 4, fp::mul(loadFloat(app, car + kCarSpeed + 4), by));
        storeFloat(app, step + 8, fp::mul(loadFloat(app, car + kCarSpeed + 8), by));
        const float turnZ = float(fp::mul(loadFloat(app, car + kCarTurn + 8), by));
        const float turnY = float(fp::mul(loadFloat(app, car + kCarTurn + 4), by));
        const float turnX = float(fp::mul(by, loadFloat(app, car + kCarTurn)));
        turnedAny = feCore<0x4a2cc0, frontend::simb::turnMatrix>(app, ts, turned, bitsOf(turnX), bitsOf(turnY),
                                                                bitsOf(turnZ), turnWord);
        if (!turnedAny)
            return std::nullopt;
    }
    if (*turnedAny != 0)
    {
        guest::direct<0x4e0280, vertices::multiplyMatrix>(app, ts, axes, turned, axes);
        if (ts.terminate)
            return std::nullopt;
    }
    // The box's corners in the car's frame, in the code's order.
    const auto copy = [&](u32 to, u32 from) { put(app, to, at(app, car + from)); };
    const auto negated = [&](u32 to, u32 from) { storeFloat(app, to, -loadFloat(app, car + from)); };
    const u32 c = corners;
    u32 count;
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
    guest::direct<0x4e0430, vertices::transformPointList>(app, ts, i32(count), corners, axes, car + kCarPosition,
                                                          corners);
    if (ts.terminate)
        return std::nullopt;
    for (u32 i = 0;; ++i)
    {
        safepoint(app, ts);
        if (s32(i) >= s32(count))
            return i;
        // The corner moved on by the speed's step, in 16.16, followed from
        // the car's track position (copied).
        const u32 corner = corners + i * 0xc;
        copyWords(place, car + kCarPlace, kPlaceWords);
        copyWords(moved, corner, 3);
        guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, ts, 1, moved, step, moved);
        if (ts.terminate)
            return std::nullopt;
        const Float toFixed = loadDouble(app, kToFixed);
        put(app, fixed, u32(fp::toInteger<s32>(fp::mul(loadFloat(app, moved), toFixed))));
        const Float y = fp::mul(loadFloat(app, moved + 4), toFixed);
        u32 crossedWalls = 0;
        put(app, fixed + 4, u32(fp::toInteger<s32>(y)));
        put(app, fixed + 8, u32(fp::toInteger<s32>(fp::mul(loadFloat(app, moved + 8), toFixed))));
        copyWords(place + 4, fixed, 3);
        if (!guest::direct<0x41f6e0, trackdata::bringPlace>(app, ts, fixed, place, 1))
            return std::nullopt;
        const auto clearAt = [&](u32 at) -> std::optional<u32> {
            const GuestFrame slot(ts, 0, 4);  // the call's return address, as the code has it
            return clear(app, ts, car, at);
        };
        const std::optional<u32> clearHere = clearAt(place);
        if (!clearHere)
            return std::nullopt;
        if (*clearHere != 0)
        {
            // Clear of its own polygon's walls: the edges it crossed lead to
            // neighbours, looked up and tested in turn.
            copyWords(neighbour, place, kPlaceWords);
            const u32 crossed =
                guest::direct<0x41fd60, trackdata::edgesPast>(app, ts, fixed, car + kCarPlace, place, edgesOut).flags;
            if (ts.terminate)
                return std::nullopt;
            const u32 edges[4] = {at(app, kEdgeBits), at(app, kEdgeBits + 4), at(app, kEdgeBits + 8),
                                    at(app, kEdgeBits + 0xc)};
            put(app, normal + 8, 0);
            put(app, normal + 4, 0);
            put(app, normal, 0);
            if (crossed != 0)
            {
                float walls = 0;
                for (u32 e = 0;; ++e)
                {
                    if (e != 0)
                    {
                        safepoint(app, ts);
                        if (e >= 4)
                            break;
                    }
                    if ((crossed & edges[e]) == 0)
                        continue;
                    const u32 leads =
                        guest::direct<0x41f870, trackdata::matchQuad>(app, ts, edges[e], car + kCarPlace, neighbour).found;
                    if (ts.terminate)
                        return std::nullopt;
                    if (leads != 0)
                    {
                        const std::optional<u32> clearThere = clearAt(neighbour);
                        if (!clearThere)
                            return std::nullopt;
                        if (*clearThere == 0)
                            continue;
                    }
                    {
                        const GuestFrame slot(ts, 0, 4);
                        if (!edgeNormal(app, ts, edges[e], car + kCarPlace, edgeOut))
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
        const float normalLength = guest::direct<0x4972f0, vertices::normaliseVector>(app, ts, normal);
        if (ts.terminate)
            return std::nullopt;
        const std::optional<u32> normalized = normal;  // eax as it was handed
        const Float length = Float(normalLength);
        const u16 word = fcom(0.0, length);
        if (word & kC3)
            return (*normalized & 0xffff0000) | word;
        // The wall's corner by the edges' bits: its polygon's, scaled, for
        // the surface.
        copyWords(surface, corner, 3);
        const u8 bits = u8(crossedWalls);
        const u32 wallCorner = (bits & 4)   ? ((bits & 1) ? 0x3c : 0x30)
                                 : (bits & 8) ? ((bits & 1) ? 0x48 : 0x54)
                                              : ((bits & 1) ? 0x3c : 0x30);
        {
            const Float scale = loadDouble(app, kPolygonScale);
            storeFloat(app, surface, fp::mul(Float(s32(at(app, car + wallCorner))), scale));
            storeFloat(app, surface + 4, fp::mul(Float(s32(at(app, car + wallCorner + 4))), scale));
            storeFloat(app, surface + 8, fp::mul(scale, Float(s32(at(app, car + wallCorner + 8)))));
        }
        const std::optional<bool> hitIt = guest::direct<0x42ee10, raceload::hit>(app, ts, car, corner, normal, surface);
        if (!hitIt)
            return std::nullopt;
        if (!*hitIt)
            continue;
        // The car's polygon's centre and normal (its record's bytes 0x18 to
        // 0x1a, signed, times 0x200), against the line to the car.
        const u32 records = at(app, kPolygonRecords);
        const auto record = [&] { return records + at(app, car + kCarSlice) * kPolygonRecordBytes; };
        {
            const Float scale = loadDouble(app, kPolygonScale);
            const Float x = fp::mul(Float(s32(at(app, record()))), scale);
            const Float y3 = fp::mul(Float(s32(at(app, record() + 4))), scale);
            const Float z = fp::mul(Float(s32(at(app, record() + 8))), scale);
            const float centreX = float(x), centreY = float(y3), centreZ = float(z);
            storeFloat(app, toCar, fp::sub(loadFloat(app, car + kCarPosition), Float(centreX)));
            storeFloat(app, toCar + 4, fp::sub(loadFloat(app, car + kCarY), Float(centreY)));
            storeFloat(app, toCar + 8, fp::sub(loadFloat(app, car + kCarPosition + 8), Float(centreZ)));
            for (u32 k = 0; k < 3; ++k)
            {
                const s32 n = s32(u32(s32(at(app, record() + 0x15 + k)) >> 24) << 9);
                storeFloat(app, polygonNormal + 4 * k, k < 2 ? fp::mul(Float(n), scale) : fp::mul(scale, Float(n)));
            }
        }
        const std::optional<Float> side = dot(app, ts, polygonNormal, toCar);
        if (!side)
            return std::nullopt;
        const float sideKept = float(*side);
        const u8 kinds = byte(app, at(app, kPolygonRecords) + at(app, car + kCarSlice) * kPolygonRecordBytes + 0xe);
        const u32 kind = fAbove(0.0, Float(sideKept)) ? (kinds >> 4) & 0xf : kinds & 0xf;
        put(app, car + kCarSoundKind, kind | 0x40000);
        if ((byte(app, car + kCarFlags) & 2) && !guest::direct<0x406160, aidrive::reverseIfTipped>(app, ts, car, normal))
            return std::nullopt;
        if (!(byte(app, car + kCarFlags) & 4))
            return car;
        // (the code's matching of the revs to the speed leaves the car in eax)
        if (!guest::direct<0x4aa4f4, handling::matchToSpeed>(app, ts, car))
            return std::nullopt;
        return car;
    }
}

}  // namespace

/* 0x4304b0: againstWalls for car eax. */
nfs::u32 carAgainstWalls(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> r = againstWalls(app, ts, args.eax);
    if (!r)
        return args.eax;
    args.eax = *r;
    ts.sp += 4;
    return args.eax;
}

namespace
{

/* Cars a and b collided: taken apart (separateCars) up to 33 times while
 * they still touch, and if they touched at all their response worked out
 * at the contact's point (0x42f280, the cars in eax and edx, the point in
 * ebx).  What the code leaves in eax: the response's, or separateCars' last
 * result where they never touched. */
std::optional<u32> collide(App* app, ThreadState& ts, u32 a, u32 b)
{
    const GuestFrame frame(ts, 0x14, 0);  // separateCars' frame is handed to its callees
    bool touched = false;
    u32 last = 0;
    for (int tries = 0; tries < 33; ++tries)
    {
        safepoint(app, ts);
        std::optional<u32> r;
        {
            const GuestFrame slot(ts, 0, 4);  // the call's return address, as the code has it
            r = separateTwo(app, ts, a, b);
        }
        if (!r)
            return std::nullopt;
        last = *r;
        if (last == 0)
            break;
        touched = true;
    }
    if (touched)
        return guest::direct<0x42f280, raceload::respond>(app, ts, a, b, kContactPoint);
    return last;
}

}  // namespace

/* 0x430c80: collide for car eax and car edx. */
nfs::u32 collideCars(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = collide(app, ts, args.eax, args.edx);
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* 0x430cd0: no car tested yet this frame. */
nfs::u32 clearCarList(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    put(app, kCarCount, 0);
    ts.sp += 4;
    return args.eax;
}

/* Whether the point at `point` (x and z; y taken as the car's) is within
 * `reach` of car `car`'s box along its forward and right axes: the point less
 * the car's position into the vector at `offset` (y 0), then 1, or 0. */
std::optional<u32> inReach(App* app, ThreadState& ts, u32 car, u32 point, u32 reachBits, u32 offset)
{
    const GuestFrame frame(ts, 0xc, 0x10);  // as deep as the code's
    storeFloat(app, offset, fp::sub(loadFloat(app, point), loadFloat(app, car + kCarPosition)));
    storeFloat(app, offset + 8, fp::sub(loadFloat(app, point + 8), loadFloat(app, car + kCarPosition + 8)));
    put(app, offset + 4, 0);
    for (const auto& axis : {std::pair<u32, u32>{kCarForward, kCarHalfLength}, {kCarRight, kCarHalfWidth}})
    {
        const std::optional<Float> along = dot(app, ts, car + axis.first, offset);
        if (!along)
            return std::nullopt;
        const float size = positive(float(*along));
        if (fBelow(fp::add(loadFloat(app, car + axis.second), floatAt(reachBits)), Float(size)))
            return 0;
    }
    return 1;
}

/* 0x430ce0: inReach for car eax, the point at edx, the reach and the
 * offset's vector on the stack; ret 8. */
nfs::u32 objectInReach(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> r =
        inReach(app, ts, args.eax, args.edx, at(app, ts.sp + 4), at(app, ts.sp + 8));
    if (!r)
        return args.eax;
    args.eax = *r;
    ts.sp += 4 + 8;
    return args.eax;
}

/* Car `car` added to this frame's list, and tested against those before it:
 * by their radii along y, x and z (each difference and the radii's sum
 * floats, as the code stores them), collideCars for those near.  What the
 * code leaves in eax: the count at first, then a difference's bits kept as
 * it was, or collideCars' result. */
std::optional<u32> addToList(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0x1c);  // as deep as the code's, for collide's callees
    u32 eax = at(app, kCarCount);
    put(app, eax * 4 + kCarList, car);
    for (u32 i = 0;; ++i)
    {
        safepoint(app, ts);
        if (s32(i) >= s32(at(app, kCarCount)))
            break;
        const u32 other = at(app, i * 4 + kCarList);
        const float radii = float(fp::add(loadFloat(app, car + kCarRadius), loadFloat(app, other + kCarRadius)));
        bool near = true;
        for (const u32 along : {0xa0u, 0x98u, 0x9cu})
        {
            const float d = float(fp::sub(loadFloat(app, car + along), loadFloat(app, other + along)));
            // Each fcomp's status word into ax, as fnstsw leaves it.
            const bool zeroBelow = (fcom(0.0, Float(d)) & kC0) != 0;
            if (zeroBelow)
                eax = bitsOf(d);
            const float size = zeroBelow ? d : float(-Float(d));
            const u16 word = fcom(Float(size), Float(radii));
            eax = (eax & 0xffff0000) | word;
            if (!(word & kC0))
            {
                near = false;
                break;
            }
        }
        if (!near)
            continue;
        const GuestFrame slot(ts, 0, 4);  // the call's return address, as the code has it
        const std::optional<u32> collided = collide(app, ts, car, other);
        if (!collided)
            return std::nullopt;
        eax = *collided;
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
constexpr u32 kSoundBumped = 0x6000a, kSoundKnocked = 0x6000b, kSoundLoud = 0x41700000;  // 15.0

/* Car `car` against the track's objects near it: those close by a rough
 * distance (the larger of the x and z differences and a quarter of the
 * smaller, each a float), twice as big as the car's reach and within its
 * box's (inReach) are hit -- the jolt; for 1 the car pushed out along the
 * line from the object, by its size, its response and the sound's halved,
 * the bump's sound for the car's flag 2; for 2 above a speed of 2, and below
 * 0, the sound's.  The vectors the calls are handed are the frame's.  What
 * the code leaves in eax: the objects' count, at least 0. */
std::optional<u32> againstObjects(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x18, 0x6c);
    const u32 size = frame - 4, position = frame - 0x48, point = frame - 0x3c, offset = frame - 0x54;
    const u32 away = frame - 0x6c, scaled = frame - 0x60;
    const u32 count = guest::direct<0x41ff10, trackdata::itemCount>(app, ts, car + kCarPlace);
    if (ts.terminate)
        return std::nullopt;
    for (u32 i = 0;; ++i)
    {
        safepoint(app, ts);
        if (s32(i) >= s32(count))
            return i;
        guest::direct<0x41ff20, trackdata::takeItem>(app, ts, car + kCarPlace, i, position, size);
        if (ts.terminate)
            return std::nullopt;
        // Its size (its word, times 0x100) and position, as floats.
        const s32 sizeFixed = s32(u32(s32(at(app, size - 2)) >> 16) << 8);
        const Float scale = loadDouble(app, kObjectScale);
        const Float sizeScaled = fp::mul(Float(sizeFixed), scale);
        const Float x = fp::mul(Float(s32(at(app, position))), scale);
        const Float y = fp::mul(Float(s32(at(app, position + 4))), scale);
        const Float z = fp::mul(scale, Float(s32(at(app, position + 8))));
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
        std::optional<u32> within;
        {
            const GuestFrame slot(ts, 0, 4 + 8);  // its two words and the return address
            within = inReach(app, ts, car, point, bitsOf(objectSize), offset);
        }
        if (!within)
            return std::nullopt;
        if (*within == 0)
            continue;
        const std::optional<u32> moving = guest::direct<0x41d350, track::objectMoving>(app, ts, car, i);
        if (!moving)
            return std::nullopt;
        const u32 how = *moving;
        // The jolt, by the offset's z and x in 16.16.
        const Float toFixed = loadDouble(app, kObjectToFixed);
        const u32 joltZ = u32(fp::toInteger<s32>(fp::mul(loadFloat(app, offset + 8), toFixed)));
        const u32 joltX = u32(fp::toInteger<s32>(fp::mul(loadFloat(app, offset), toFixed)));
        if (!guest::direct<0x471440, controls::bump>(app, ts, car, joltX, how))
            return std::nullopt;
        if (how == 1)
        {
            // Pushed out of it, by its size, along the line from it.
            put(app, away + 4, 0);
            storeFloat(app, away, fp::sub(loadFloat(app, car + kCarPosition), loadFloat(app, point)));
            storeFloat(app, away + 8, fp::sub(loadFloat(app, car + kCarPosition + 8), loadFloat(app, point + 8)));
            // Its length the code drops (fstp st(0)).
            guest::direct<0x4972f0, vertices::normaliseVector>(app, ts, away);
            if (ts.terminate)
                return std::nullopt;
            guest::direct<0x4e0050, vertices::scaleVectorsBy>(app, ts, i32(how), away, objectSize, scaled);
            if (ts.terminate)
                return std::nullopt;
            guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, ts, i32(how), point, scaled, scaled);
            if (ts.terminate)
                return std::nullopt;
            {
                // The word the game's code pushes for it (0), above the callee's frame.
                const GuestFrame word(ts, 0, 4);
                const u32 reach = word - 4;
                put(app, reach, 0);
                if (!guest::direct<0x42efe0, raceload::boxHits>(app, ts, car, away, scaled, reach))
                    return std::nullopt;
            }
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
                guest::direct<0x406120, aidrive::reverseWhenSlowDone>(app, ts, car);
                if (ts.terminate)
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
            for (u32 w = 0; w < 12; w += 4)
                put(app, car + kCarSoundAt + w, at(app, car + kCarPosition + w));
        }
        else if (s32(how) < 0)
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

/* 0x430da0: againstObjects for car eax. */
nfs::u32 carAgainstObjects(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> r = againstObjects(app, ts, args.eax);
    if (!r)
        return args.eax;
    args.eax = *r;
    ts.sp += 4;
    return args.eax;
}

/* 0x4310c0: addToList for car eax. */
nfs::u32 addCarToList(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> r = addToList(app, ts, args.eax);
    if (!r)
        return args.eax;
    args.eax = *r;
    ts.sp += 4;
    return args.eax;
}

// A car's collisions' fields: whether it is out of the race (+0x88) and in
// it (+0x89), its countdown before it collides again (+0x174), what it
// collides with (+0x188: 0x40 the track's objects, 0x80 its walls).
constexpr u32 kCarOut = 0x88, kCarIn = 0x89, kCarCountdown = 0x174, kCarCollides = 0x188;

/* A car's collisions for the frame: its countdown on; for a car in the race
 * the other cars (once the countdown is out), the track's objects (by the
 * flag 0x40, likewise) and walls (0x80), then 0x4a0140 (sima's).  What the
 * code leaves in eax: that one's, or the car for a car out of the race;
 * nullopt when the game stopped. */
std::optional<u32> frameCollisions(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x14, 0);  // as deep as the code's, for its callees' frames
    const u32 countdown = at(app, car + kCarCountdown);
    if (s32(countdown) > 0)
        put(app, car + kCarCountdown, countdown - 1);
    if (byte(app, car + kCarOut) != 0 || byte(app, car + kCarIn) == 0)
        return car;
    const bool settled = at(app, car + kCarCountdown) == 0;
    if (settled)
    {
        const GuestFrame slot(ts, 0, 4);  // the call's return address, as the code has it
        if (!addToList(app, ts, car))
            return std::nullopt;
    }
    if ((byte(app, car + kCarCollides) & 0x40) && settled)
    {
        const GuestFrame slot(ts, 0, 4);  // the call's return address, as the code has it
        if (!againstObjects(app, ts, car))
            return std::nullopt;
    }
    if (byte(app, car + kCarCollides) & 0x80)
    {
        const GuestFrame slot(ts, 0, 4);
        if (!againstWalls(app, ts, car))
            return std::nullopt;
    }
    return feCore<0x4a0140, frontend::sima::carOntoRoad>(app, ts, car);
}

/* 0x4311d0 (a car's +0x618): frameCollisions for car eax. */
nfs::u32 carCollisions(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> left = frameCollisions(app, ts, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    ts.sp += 4;
    return args.eax;
}

/* The square of the distance between the points (ax, ay, az) and (bx, by,
 * bz), in integers as they wrap. */
u32 squaredDistance(u32 ax, u32 ay, u32 az, u32 bx, u32 by, u32 bz)
{
    const u32 dx = ax - bx, dy = ay - by, dz = az - bz;
    return dz * dz + (dy * dy + dx * dx);
}

/* 0x431250: squaredDistance of (eax, edx, ebx) and (ecx, and the two on the
 * stack); ret 8. */
nfs::u32 distanceSquared(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = squaredDistance(args.eax, args.edx, args.ebx, args.ecx, at(app, ts.sp + 4), at(app, ts.sp + 8));
    ts.sp += 4 + 8;
    return args.eax;
}

// The points nearestPoint searches: x, y, z and the value, 0x10 bytes each.
constexpr u32 kPoints = 0x5537c4, kPointCount = 0x167, kPointBytes = 0x10;

/* The value of the point nearest (x, y, z) among the points; the first of
 * those as near as another (compared signed, as they wrap), the search over
 * at a distance of 0. */
u32 nearest(App* app, u32 x, u32 y, u32 z)
{
    const auto distance = [&](u32 i) {
        const u32 p = kPoints + i * kPointBytes;
        return squaredDistance(x, y, z, at(app, p), at(app, p + 4), at(app, p + 8));
    };
    u32 best = 0;
    s32 bestSquare = s32(distance(0));
    for (u32 i = 1; s32(i) < s32(kPointCount); ++i)
    {
        const u32 square = distance(i);
        if (s32(square) < bestSquare)
        {
            best = i;
            bestSquare = s32(square);
            if (square == 0)
                break;
        }
    }
    return at(app, kPoints + best * kPointBytes + 0xc);
}

/* 0x431280: nearest to (eax, edx, ebx). */
nfs::u32 nearestPoint(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = nearest(app, args.eax, args.edx, args.ebx);
    ts.sp += 4;
    return args.eax;
}

// The track's COLL file, its objects: its handle, its 16 bytes' header
// ('COLL', its version 11, its size, its chunks' count).
constexpr u32 kCollHandle = 0x5eff18, kCollHeader = 0x5eff08;
constexpr u32 kCollVersion = 0x5eff0c, kCollSize = 0x5eff10, kCollCount = 0x5eff14;
constexpr u32 kCollMagic = 0x53746c;
constexpr u32 kCollHeaderBytes = 0x10, kCollVersionWanted = 0xb;
// EA's files (native_ea_files.h) are called as C++.

/* The COLL file `handle` closed, its handle 0.  What the close gives back,
 * as the code leaves it in eax; nullopt when the game stopped. */
std::optional<u32> closeColl(App* app, ThreadState& ts, u32 handle)
{
    const std::optional<u32> closed = guest::direct<0x4e2da0, ea::closeChecked>(app, ts, handle);
    if (closed)
        put(app, kCollHandle, 0);
    return closed;
}

/* The COLL file named `name` opened (one open before closed first): its
 * header read and checked -- 'COLL', version 11.  The handle, or 0 with the
 * file closed (its handle 0). */
std::optional<u32> openColl(App* app, ThreadState& ts, u32 name)
{
    const GuestFrame frame(ts, 0x14, 8);  // the open's two words
    if (at(app, kCollHandle) != 0 && !closeColl(app, ts, at(app, kCollHandle)))
        return std::nullopt;
    if (!guest::direct<0x4e28d0, ea::openToRead>(app, ts, name, kCollHandle, frame - 8, frame - 4))
        return std::nullopt;
    const std::optional<u32> read =
        guest::direct<0x4e2de0, ea::readChecked>(app, ts, at(app, kCollHandle), kCollHeader, kCollHeaderBytes);
    if (!read)
        return std::nullopt;
    bool good = false;
    if (*read != 0)
    {
        const u32 differs = guest::direct<0x4eb390, watcom::clib::strncmp>(app, ts, kCollHeader, kCollMagic, 4u);
        if (ts.terminate)
            return std::nullopt;
        good = differs == 0 && at(app, kCollVersion) == kCollVersionWanted;
    }
    if (good)
        return at(app, kCollHandle);
    if (!guest::direct<0x4e2da0, ea::closeChecked>(app, ts, at(app, kCollHandle)))
        return std::nullopt;
    put(app, kCollHandle, 0);
    return 0;
}

/* The COLL file `handle` past its header read into `to`, its size less the
 * header's.  False when the game stopped. */
bool readColl(App* app, ThreadState& ts, u32 handle, u32 to)
{
    if (!guest::direct<0x4e2e60, ea::seekChecked>(app, ts, handle, kCollHeaderBytes))
        return false;
    return guest::direct<0x4e2de0, ea::readChecked>(app, ts, handle, to, at(app, kCollSize) - kCollHeaderBytes)
        .has_value();
}

/* 0x431320: openColl for the name at eax. */
nfs::u32 openCollFile(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> handle = openColl(app, ts, args.eax);
    if (!handle)
        return args.eax;
    args.eax = *handle;
    ts.sp += 4;
    return args.eax;
}

/* 0x4313b0: closeColl for eax. */
nfs::u32 closeCollFile(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> closed = closeColl(app, ts, args.eax);
    if (!closed)
        return args.eax;
    args.eax = *closed;
    ts.sp += 4;
    return args.eax;
}

/* The COLL file's size, its header's. */
u32 collSize(App* app)
{
    return at(app, kCollSize);
}

/* 0x4313d0: collSize. */
nfs::u32 collFileSize(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = collSize(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x4313e0: readColl of the COLL file eax into edx; 1. */
nfs::u32 readCollFile(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (!readColl(app, ts, args.eax, args.edx))
        return args.eax;
    args.eax = 1;
    ts.sp += 4;
    return args.eax;
}

/* The COLL file's chunks' count, its header's. */
u32 collCount(App* app)
{
    return at(app, kCollCount);
}

/* 0x431410: collCount. */
nfs::u32 collFileCount(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = collCount(app);
    ts.sp += 4;
    return args.eax;
}

/* Chunk `chunk` of the COLL file's data at `data`: data plus its offset in
 * the table at data. */
u32 collChunkAt(App* app, u32 data, u32 chunk)
{
    return data + at(app, data + chunk * 4);
}

/* 0x431420: collChunkAt for eax and edx. */
nfs::u32 collChunk(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = collChunkAt(app, args.eax, args.edx);
    ts.sp += 4;
    return args.eax;
}

// The controls' state for the frame: its word at 0x79c4a4 (the steering in
// its top byte), the pedals' bytes (+4, +5), the buttons (+6: 2 the reset or
// the pause, 4 and 8 the clutch's and the like, 0x40 the view, 0x80 the
// horn).
constexpr u32 kControls = 0x79c4a5;
constexpr u32 kRaceEntryBytes = 0x6c;
// A car's fields the controls set.
constexpr u32 kCarThrottleAsked = 0x504, kCarBrakeAsked = 0x505, kCarGearAsked = 0x506, kCarGear = 0x50b;
constexpr u32 kCarButton4 = 0x50d, kCarButton8 = 0x50e, kCarButton80 = 0x512, kCarSteeringAsked = 0x518;
constexpr u32 kCarPlayer = 0x1f0, kCarHandlingRecord = 0x528, kCarOwnControls = 0x5fc;
constexpr u32 kCarView = 0x660;

/* The player's car `car`'s controls for the frame -- 0x405740 first while a
 * replay, a demo or the lane votes say so, the car's own (+0x5fc), then the
 * controls' state into the car: the steering and pedals' bytes, the gear
 * asked for (stepped through 0x48f710 or 0x48f6e0 as the car's gearbox is
 * automatic or not), the buttons as 0 or 1, the view stepped (of three), and
 * the reset (0x405220, edx 1, for the car's flag 0x20) or the pause
 * (0x429100).  What the code leaves in eax, or nullopt when the game
 * stopped. */
std::optional<u32> controls(App* app, ThreadState& ts, u32 car)
{
    const GuestFrame frame(ts, 0x10, 0);  // as deep as the code's
    if (s32(at(app, 0x5efd98)) > 0 || s32(at(app, 0x5efd88)) > 0 || gameSettings(app).raceOptionLaneVotes != 0)
    {
        if (!guest::direct<0x405740, aidrive::driveCop>(app, ts, car))
            return std::nullopt;
    }
    if (!callWith(app, ts, at(app, car + kCarOwnControls), {car}))
        return std::nullopt;
    putByte(app, car + kCarThrottleAsked, byte(app, kControls + 4));
    putByte(app, car + kCarBrakeAsked, byte(app, kControls + 5));
    put(app, car + kCarSteeringAsked, u32(s32(at(app, kControls)) >> 24));
    const u32 player = at(app, car + kCarPlayer);
    u8 gear;
    if (gameSettings(app).entryAt(player * kRaceEntryBytes).transmission == 1)
        gear = guest::direct<0x48f710, controls::stepChoiceOfThree>(app, ts, byte(app, car + kCarGear));
    else
        gear = guest::direct<0x48f6e0, controls::stepChoice>(app, ts, byte(app, car + kCarGear),
                                                             at(app, at(app, car + kCarHandlingRecord) + 8));
    if (ts.terminate)
        return std::nullopt;
    putByte(app, car + kCarGearAsked, gear);
    putByte(app, car + kCarButton4, (byte(app, kControls + 6) & 4) ? 1 : 0);
    putByte(app, car + kCarButton8, (byte(app, kControls + 6) & 8) ? 1 : 0);
    u32 eax = (byte(app, kControls + 6) & 0x80) ? 1 : 0;
    putByte(app, car + kCarButton80, u8(eax));
    if (byte(app, kControls + 6) & 0x40)
    {
        const u32 view = at(app, car + kCarView) + 1;
        put(app, car + kCarView, view);
        eax = u32(s32(view) / 3);
        put(app, car + kCarView, u32(s32(view) % 3));
    }
    if (byte(app, kControls + 6) & 2)
    {
        const std::optional<u32> left = (byte(app, car + kCarFlags) & 0x20)
                                              ? guest::direct<0x405220, aidrive::jumpBehind>(app, ts, car, true)
                                              : guest::direct<0x429100, racestart::resetCarOf>(app, ts, car);
        if (!left)
            return std::nullopt;
        eax = *left;
    }
    return eax;
}

/* 0x431430 (a car's +0x600): controls for car eax. */
nfs::u32 playerControls(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> eax = controls(app, ts, args.eax);
    if (!eax)
        return args.eax;
    args.eax = *eax;
    ts.sp += 4;
    return args.eax;
}

// The render states the game keeps (setRenderState's cache), -1 for none.
constexpr u32 kStateTexture = 0x554e54, kStateAlpha = 0x554e58, kStateDepthBias = 0x554e5c;
// The render states' numbers the drawers set themselves.
constexpr u32 kTextureState = 1, kFogColourState = 3, kAlphaTestState = 0x68, kDepthBiasState = 0x15;
// The driver's depth for the HUD (0 off the card), the card's, the race's depth bias.
constexpr u32 kHudDepth = 0x7a3a60, kCardDepth = 0x7a3a68, kRaceDepthBias = 0x7a3a64;

/* The cached render states forgotten (-1), so that each is set again. */
void forgetStates(App* app)
{
    for (const u32 state : {0x554e58u, 0x554e5cu, 0x554e60u, 0x554e64u, 0x554e68u, 0x554e6cu, 0x554e54u})
        put(app, state, 0xffffffff);
}

/* 0x4318c0: forgetStates; every register as it was. */
nfs::u32 forgetRenderStates(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    forgetStates(app);
    ts.sp += 4;
    return args.eax;
}

namespace
{

/* graphics::setRenderState (state, value), as C++: its core leaves eax its
 * answer, as the code does.  False when the game stopped. */
bool setRenderState(App* app, ThreadState& ts, u32 state, u32 value)
{
    return guest::direct<0x431900, graphics::sendState>(app, ts, state, value);
}

/* The render states set in turn.  False when the game stopped. */
bool setStates(App* app, ThreadState& ts, std::initializer_list<std::pair<u32, u32>> states)
{
    for (const auto& s : states)
        if (!setRenderState(app, ts, s.first, s.second))
            return false;
    return true;
}

/* The render-state functions' thunk: the core, eax as the last call
 * leaves it. */
nfs::u32 statesThunk(App* app, ThreadState& ts, nfs::Args& args, bool (*core)(App*, ThreadState&))
{
    if (!core(app, ts))
        return args.eax;
    ts.sp += 4;
    return args.eax;
}

}  // namespace

/* The render states for 2D text: states forgotten, then 0xd, 0x15, 0x68, 2
 * and 4 all 0. */
bool textStates(App* app, ThreadState& ts)
{
    forgetStates(app);
    return setStates(app, ts, {{0xd, 0}, {0x15, 0}, {0x68, 0}, {2, 0}, {4, 0}});
}

namespace
{

/* The same states, 4 first. */
bool pictureStates(App* app, ThreadState& ts)
{
    forgetStates(app);
    return setStates(app, ts, {{4, 0}, {0xd, 0}, {0x15, 0}, {0x68, 0}, {2, 0}});
}

}  // namespace

/* The render states for the race's HUD: the driver's depth (the card's while
 * the text is projected, else 0), the depth offsets, states forgotten, then 4
 * that depth, 0xd 1, 0x15 the depth bias, 0x68 and 2 0 -- each value read
 * after the call before it, as the code reads it. */
bool hudStates(App* app, ThreadState& ts)
{
    put(app, kHudDepth, gameSettings(app).projectedText != 0 ? at(app, kCardDepth) : 0);
    guest::direct<0x4c7a60, decompiled::depthOffsetsSet>(app, ts);
    if (ts.terminate)
        return false;
    forgetStates(app);
    return setRenderState(app, ts, 4, at(app, kHudDepth)) && setRenderState(app, ts, 0xd, 1)
           && setRenderState(app, ts, 0x15, at(app, kRaceDepthBias)) && setStates(app, ts, {{0x68, 0}, {2, 0}});
}

/* The render states for the 2D screens: states forgotten, then 0xd, 0x15,
 * 0x68 0, 2 1, 4 0, 0x2c2 1. */
bool screenStates(App* app, ThreadState& ts)
{
    forgetStates(app);
    return setStates(app, ts, {{0xd, 0}, {0x15, 0}, {0x68, 0}, {2, 1}, {4, 0}, {0x2c2, 1}});
}

/* 0x431aa0: textStates. */
nfs::u32 statesForText(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return statesThunk(app, ts, args, textStates);
}

/* 0x431af0: pictureStates. */
nfs::u32 statesForPictures(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return statesThunk(app, ts, args, pictureStates);
}

/* 0x431b40: hudStates. */
nfs::u32 statesForHud(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return statesThunk(app, ts, args, hudStates);
}

/* 0x431bc0: screenStates. */
nfs::u32 statesForScreens(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return statesThunk(app, ts, args, screenStates);
}

/* A view pass's layer that draws nothing: what its callers hand it (eax how many quads, edx the
 * layer) goes unread. */
void layerNothing(App*, u32, u32)
{
}

/* 0x431c20: layerNothing (an empty function). */
nfs::u32 passLayerNothing(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    layerNothing(app, args.eax, args.edx);
    ts.sp += 4;
    return args.eax;
}

/* The main pass's second and first layers: nothing; the layer its callers hand it (eax) goes
 * unread. */
void secondLayerNothing(App*, u32)
{
}

void firstLayerNothing(App*, u32)
{
}

/* 0x431c30: secondLayerNothing (an empty function). */
nfs::u32 passSecondLayer(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    secondLayerNothing(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x431c40: firstLayerNothing (an empty function). */
nfs::u32 passFirstLayer(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    firstLayerNothing(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

// The software renderer's blend: its tables of a channel times another (5
// bits by 5 at 0x5f1798, 6 by 5 at 0x5f0798, a word each) and the pixel
// format's masks and shifts (0x554e70 on: blue, green, red's masks, 0,
// green's and red's shifts), for 15 bits a pixel (0x56438c 0xf) or 16.
constexpr u32 kBlendMasks = 0x554e70, kPixelBits = 0x56438c;
constexpr u32 kBlueMask = kBlendMasks, kGreenMask = kBlendMasks + 4, kRedMask = kBlendMasks + 8;
constexpr u32 kGreenShift = kBlendMasks + 0x10, kRedShift = kBlendMasks + 0x14;
// The tables blendSpan reads each channel through.
constexpr u32 kBlueTable = 0x5f1f98, kGreenTable = 0x5f1fa4, kRedTable = 0x5f1f9c;

/* The software renderer's blend tables -- a times b over 31, rounded up, to
 * 31 (63 for green's) -- and the pixel format's masks and shifts.  The last
 * quotient, which the code leaves in eax. */
u32 fillBlendTables(App* app)
{
    struct Table
    {
        i32 columns, most;
        u32 at;
    };
    static const Table tables[] = {{0x20, 0x1f, 0x5f1798}, {0x40, 0x3f, 0x5f0798}};
    i32 quotient = 0;
    for (const Table& t : tables)
    {
        u32 entry = t.at;
        for (i32 a = 0; a < 0x20; ++a)
            for (i32 b = 0; b < t.columns; ++b)
            {
                quotient = (a * b + 0x1e) / 0x1f;
                putHalf(app, entry, u16(quotient > t.most ? t.most : quotient));
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
    return u32(quotient);
}

/* 0x431c50: fillBlendTables. */
nfs::u32 makeBlendTables(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = fillBlendTables(app);
    ts.sp += 4;
    return args.eax;
}

// A span's record, as blendSpan reads it: the texture's rows, the texel's
// mask, the screen's pixel and the span's last, u and v (16.16 and 15
// bits) and their steps.
constexpr u32 kSpanRows = 0, kSpanMask = 8, kSpanPixel = 0xc, kSpanLast = 0x10, kSpanU = 0x14, kSpanV = 0x18;
constexpr u32 kSpanStepU = 0x1c, kSpanStepV = 0x20;

/* The span at `span` blended into the screen, a pixel at a time: each
 * texel's channels through the blend tables added to the screen pixel's,
 * each to its most -- compared signed, as the code does. */
void blendInto(App* app, u32 span)
{
    const auto channel = [&](u32 table, u32 mask, u32 shift, u32 texel, u32 pixel) {
        const u32 index = u32(s32(texel & mask) >> shift) * 2;
        const u32 value = u32(half(app, at(app, table) + index)) << shift;
        const u32 sum = value + (pixel & mask);
        return s32(sum) > s32(mask) ? mask : sum;
    };
    while (at(app, span + kSpanLast) >= at(app, span + kSpanPixel))
    {
        const u32 u = at(app, span + kSpanU);
        const u32 texelAt = ((at(app, span + kSpanV) & at(app, span + kSpanMask)) + u) >> 15;
        const u32 texel = at(app, at(app, span + kSpanRows) + texelAt * 4);
        const u32 nextU = u + at(app, span + kSpanStepU);
        const u32 nextV = at(app, span + kSpanV) + at(app, span + kSpanStepV);
        const u32 pixel = half(app, at(app, span + kSpanPixel));
        put(app, span + kSpanV, nextV);
        const u32 blueTable = at(app, kBlueTable);
        put(app, span + kSpanU, nextU);
        // Blue's is not shifted: its table is read at the texel's blue twice.
        u32 blue = u32(half(app, blueTable + (at(app, kBlueMask) & texel) * 2)) + (pixel & at(app, kBlueMask));
        if (s32(blue) > s32(at(app, kBlueMask)))
            blue = at(app, kBlueMask);
        const u32 green = channel(kGreenTable, at(app, kGreenMask), byte(app, kGreenShift) & 0x1f, texel, pixel);
        const u32 red = channel(kRedTable, at(app, kRedMask), byte(app, kRedShift) & 0x1f, texel, pixel);
        const u32 to = at(app, span + kSpanPixel);
        put(app, span + kSpanPixel, to + 2);
        putHalf(app, to, u16(green | blue | red));
    }
}

/* 0x431d50 (handed to the software renderer as its blend): blendInto for the
 * span at eax. */
nfs::u32 blendSpan(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    blendInto(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

// A corner's depth, as the drawers' corners have it; the depths a quad is
// subdivided within (doubles).
constexpr u32 kCornerDepth = 8;

/* How finely the quad of the corners a, b, c and d is to be subdivided, by
 * the nearest of their depths: 0 beyond kDepthNear, 1 beyond kDepthFar,
 * else 2.  The nearest is kept as its bits (a NaN taken, as fcom's below
 * says for it) and goes to a double, exactly. */
u32 subdivisions(App* app, u32 a, u32 b, u32 c, u32 d)
{
    u32 nearest = at(app, a + kCornerDepth);
    for (const u32 corner : {b, c, d})
        if (fBelow(loadFloat(app, corner + kCornerDepth), floatAt(nearest)))
            nearest = at(app, corner + kCornerDepth);
    if (fAbove(floatAt(nearest), loadDouble(app, kDepthNear)))
        return 0;
    if (fAbove(floatAt(nearest), loadDouble(app, kDepthFar)))
        return 1;
    return 2;
}

/* 0x432690: subdivisions of the corners eax, edx, ebx and ecx. */
nfs::u32 subdivisionDepth(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = subdivisions(app, args.eax, args.edx, args.ebx, args.ecx);
    ts.sp += 4;
    return args.eax;
}

// A polygon list's record, as the drawers read it: the next (+0), its kind
// (+4, a word: 0 a quad, 1 a triangle), its flags (+6: 1 the alpha test, 2
// its corners' texture coordinates kept, 4 no depth bias, 0x10 drawn by z),
// its corners (+8 on), its texture (+0x18: the handle +4, the coordinates
// from +8, its flags +0x28, 8 the software blend's).
constexpr u32 kRecordKind = 4, kRecordFlags = 6, kRecordCorners = 8, kRecordTexture = 0x18;
constexpr u32 kTextureHandle = 4, kTextureFlags = 0x28;
constexpr u32 kCornerU = 0x18, kCornerV = 0x1c;  // a corner's texture coordinates
constexpr u32 kKindQuad = 0, kKindTriangle = 1;
constexpr u32 kDrawQuadZ = 0x4c20e0;       // the quad eax, edx, ebx, ecx drawn by z (render::quadByZDrawn)

namespace
{

/* THRASH_setstate (state, value) through its pointer (thrash_calls.h), the
 * value kept at `kept` first.  False when the game stopped. */
bool setThrashState(App* app, ThreadState& ts, u32 state, u32 value, u32 kept)
{
    put(app, kept, value);
    return guest::driver<thrash::kThrashSetState, &port::driver::setState>(app, ts, state, value).has_value();
}

/* The drawers' alpha test, on for `on`, set only where it changes. */
bool alphaState(App* app, ThreadState& ts, bool on)
{
    const u32 value = on ? 1 : 0;
    if (at(app, kStateAlpha) == value)
        return true;
    return setThrashState(app, ts, kAlphaTestState, value, kStateAlpha);
}

/* The drawers' texture: the record's texture's handle, or none, set only
 * where it changes.  `texture` as the code read it before the coordinates'
 * copy; the handle from the record's texture read again. */
bool textureState(App* app, ThreadState& ts, u32 record, u32 texture)
{
    if (texture != 0)
    {
        const u32 handle = at(app, at(app, record + kRecordTexture) + kTextureHandle);
        if (at(app, kStateTexture) != handle)
            return setThrashState(app, ts, kTextureState, handle, kStateTexture);
        return true;
    }
    if (at(app, kStateTexture) == 0)
        return true;
    return setThrashState(app, ts, kTextureState, 0, kStateTexture);
}

/* The drawers' depth bias: none for the record's flag 4, else the race's;
 * set only where it changes. */
bool depthBiasState(App* app, ThreadState& ts, u8 flags)
{
    const u32 bias = (flags & 4) ? 0 : at(app, kRaceDepthBias);
    if (at(app, kStateDepthBias) == bias)
        return true;
    return setThrashState(app, ts, kDepthBiasState, bias, kStateDepthBias);
}

/* A drawer's draw call, as C++: the triangle (0x4c11b0), the triangle by z (0x4c1aa0), the quad
 * (0x4c12e0), the flat triangle (0x4c0b20); false when the game stopped. */
bool drawTriangle(App* app, ThreadState& ts, u32 a, u32 b, u32 c)
{
    guest::direct<0x4c11b0, vertices::clipTriangleCore>(app, ts, a, b, c);
    return !ts.terminate;
}

bool drawTriangleByZ(App* app, ThreadState& ts, u32 a, u32 b, u32 c)
{
    guest::direct<0x4c1aa0, vertices::clipTriangleByZCore>(app, ts, a, b, c);
    return !ts.terminate;
}

bool drawQuad(App* app, ThreadState& ts, u32 a, u32 b, u32 c, u32 d)
{
    guest::direct<0x4c12e0, vertices::clipQuadCore>(app, ts, a, b, c, d);
    return !ts.terminate;
}

/* The quad by z (0x4c20e0, render::quadByZ: by address, it has no typed core). */
bool drawQuadByZ(App* app, ThreadState& ts, u32 a, u32 b, u32 c, u32 d)
{
    return guest::direct<kDrawQuadZ, render::quadByZDrawn>(app, ts, a, b, c, d).has_value();
}

bool drawFlat(App* app, ThreadState& ts, u32 a, u32 b)
{
    return guest::direct<0x4c0b20, render::lineDrawn>(app, ts, a, b).has_value();
}

/* The next record of a drawer's run: 0 at the list's end, the record itself
 * of another kind; nullopt to go on with it. */
std::optional<u32> runEnds(App* app, u32 next, u32 kind)
{
    if (next == 0 || half(app, next + kRecordKind) != kind)
        return next;
    return std::nullopt;
}

/* A drawer's thunk: the record at eax, the next in eax; ebx, ecx and edx
 * as the caller had them (the drawers push and pop them). */
nfs::u32 drawer(App* app, ThreadState& ts, nfs::Args& args, std::optional<u32> (*core)(App*, ThreadState&, u32))
{
    const std::optional<u32> next = core(app, ts, args.eax);
    if (!next)
        return args.eax;
    args.eax = *next;
    ts.sp += 4;
    return args.eax;
}

}  // namespace

/* The textured triangles from `record` on, while they are triangles: the
 * alpha test by the flags, the texture's coordinates into the three corners
 * unless flag 2 (the first's as words, the second's as floats, the third's
 * as words, each from the record's texture read again), the texture, the
 * software blend's colour for a texture's flag 8, the triangle drawn.  The
 * next record, or nullopt when the game stopped. */
std::optional<u32> texturedTriangles(App* app, ThreadState& ts, u32 record)
{
    for (;;)
    {
        safepoint(app, ts);
        const u32 first = at(app, record + kRecordCorners), second = at(app, record + kRecordCorners + 4);
        const u32 third = at(app, record + kRecordCorners + 8);
        const u8 flags = byte(app, record + kRecordFlags);
        if (!alphaState(app, ts, (flags & 1) != 0))
            return std::nullopt;
        const u32 texture = at(app, record + kRecordTexture);
        const auto from = [&](u32 offset) { return at(app, record + kRecordTexture) + offset; };
        if (texture != 0 && !(flags & 2))
        {
            put(app, first + kCornerU, at(app, from(8)));
            put(app, first + kCornerV, at(app, from(0xc)));
            storeFloat(app, second + kCornerU, loadFloat(app, from(0x10)));
            storeFloat(app, second + kCornerV, loadFloat(app, from(0x14)));
            put(app, third + kCornerU, at(app, from(0x18)));
            put(app, third + kCornerV, at(app, from(0x1c)));
        }
        if (!textureState(app, ts, record, texture))
            return std::nullopt;
        const u32 now = at(app, record + kRecordTexture);
        if (now != 0 && (byte(app, now + kTextureFlags) & 8)
            && !guest::direct<0x432390, graphics::softwareBlendColour>(app, ts, at(app, first + 0x10)))
            return std::nullopt;
        if (!drawTriangle(app, ts, first, second, third))
            return std::nullopt;
        if (const auto next = runEnds(app, at(app, record), kKindTriangle))
            return next;
        record = at(app, record);
    }
}

/* The flat triangle at `record`: no texture, the alpha test by its flags,
 * drawn (its corners +8 and +0xc).  The next record. */
std::optional<u32> flatTriangle(App* app, ThreadState& ts, u32 record)
{
    const u32 first = at(app, record + kRecordCorners), second = at(app, record + kRecordCorners + 4);
    if (at(app, kStateTexture) != 0 && !setThrashState(app, ts, kTextureState, 0, kStateTexture))
        return std::nullopt;
    if (!alphaState(app, ts, (byte(app, record + kRecordFlags) & 1) != 0))
        return std::nullopt;
    if (!drawFlat(app, ts, first, second))
        return std::nullopt;
    return at(app, record);
}

/* The triangles from `record` on, while they are triangles: as
 * texturedTriangles, the coordinates the first's as floats, the others' as
 * words, no software blend, the depth bias by flag 4, drawn by z for flag
 * 0x10. */
std::optional<u32> trianglesByZ(App* app, ThreadState& ts, u32 record)
{
    for (;;)
    {
        safepoint(app, ts);
        const u32 first = at(app, record + kRecordCorners), second = at(app, record + kRecordCorners + 4);
        const u32 third = at(app, record + kRecordCorners + 8);
        const u8 flags = byte(app, record + kRecordFlags);
        if (!alphaState(app, ts, (flags & 1) != 0))
            return std::nullopt;
        const u32 texture = at(app, record + kRecordTexture);
        const auto from = [&](u32 offset) { return at(app, record + kRecordTexture) + offset; };
        if (texture != 0 && !(flags & 2))
        {
            storeFloat(app, first + kCornerU, loadFloat(app, from(8)));
            storeFloat(app, first + kCornerV, loadFloat(app, from(0xc)));
            put(app, second + kCornerU, at(app, from(0x10)));
            put(app, second + kCornerV, at(app, from(0x14)));
            put(app, third + kCornerU, at(app, from(0x18)));
            put(app, third + kCornerV, at(app, from(0x1c)));
        }
        if (!textureState(app, ts, record, texture) || !depthBiasState(app, ts, flags))
            return std::nullopt;
        if (!((flags & 0x10) ? drawTriangleByZ(app, ts, first, second, third)
                             : drawTriangle(app, ts, first, second, third)))
            return std::nullopt;
        if (const auto next = runEnds(app, at(app, record), kKindTriangle))
            return next;
        record = at(app, record);
    }
}

/* The quads from `record` on, while they are quads: as trianglesByZ, four
 * corners, the first's and third's coordinates as floats. */
std::optional<u32> quadsByZ(App* app, ThreadState& ts, u32 record)
{
    for (;;)
    {
        safepoint(app, ts);
        const u32 first = at(app, record + kRecordCorners), third = at(app, record + kRecordCorners + 8);
        const u32 second = at(app, record + kRecordCorners + 4), fourth = at(app, record + kRecordCorners + 0xc);
        const u8 flags = byte(app, record + kRecordFlags);
        if (!alphaState(app, ts, (flags & 1) != 0))
            return std::nullopt;
        const u32 texture = at(app, record + kRecordTexture);
        const auto from = [&](u32 offset) { return at(app, record + kRecordTexture) + offset; };
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
        if (!textureState(app, ts, record, texture) || !depthBiasState(app, ts, flags))
            return std::nullopt;
        if (!((flags & 0x10) ? drawQuadByZ(app, ts, first, second, third, fourth)
                             : drawQuad(app, ts, first, second, third, fourth)))
            return std::nullopt;
        if (const auto next = runEnds(app, at(app, record), kKindQuad))
            return next;
        record = at(app, record);
    }
}

/* 0x433530 (a polygon list's drawer, the software renderers' and
 * Direct3D's): texturedTriangles. */
nfs::u32 drawTexturedTriangles(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return drawer(app, ts, args, texturedTriangles);
}

/* 0x433660 (a polygon list's drawer): flatTriangle. */
nfs::u32 drawFlatTriangles(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return drawer(app, ts, args, flatTriangle);
}

/* 0x4336e0 (a polygon list's drawer, the driver's): trianglesByZ. */
nfs::u32 drawTrianglesByZ(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return drawer(app, ts, args, trianglesByZ);
}

/* 0x433840 (a polygon list's drawer, the driver's): quadsByZ. */
nfs::u32 drawQuadsByZ(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return drawer(app, ts, args, quadsByZ);
}

/* 0x4339d0 (a polygon list's drawer, the driver's): flatTriangle, the same
 * code again. */
nfs::u32 drawFlatTrianglesAgain(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return drawer(app, ts, args, flatTriangle);
}

/* The triangles with colours of their own from `record` on -- the driver's
 * on the card (0x434380) or the other (0x433e30), tail-called with the
 * record in eax.  What it leaves in eax, the next record; nullopt when the
 * game stopped. */
std::optional<u32> ownColourTriangles(App* app, ThreadState& ts, u32 record)
{
    if (at(app, kHudDepth) != 0)
    {
        const u32 next = guest::direct<0x434380, vertices::submitPolygonsCore>(app, ts, record);
        if (ts.terminate)
            return std::nullopt;
        return next;
    }
    return guest::pointerCall(app, ts, 0x433e30, {record});  // render::colouredTriangles: no typed core
}

/* 0x4347b0 (a polygon list's drawer): ownColourTriangles for the record in
 * eax. */
nfs::u32 drawColouredTriangles(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> next = ownColourTriangles(app, ts, args.eax);
    if (!next)
        return args.eax;
    args.eax = *next;
    ts.sp += 4;
    return args.eax;
}

/* A render state's record: state +8, value +0xc set.  The next record. */
std::optional<u32> stateFromRecord(App* app, ThreadState& ts, u32 record)
{
    if (!setRenderState(app, ts, at(app, record + 8), at(app, record + 0xc)))
        return std::nullopt;
    return at(app, record);
}

/* 0x434850 (a polygon list's drawer): stateFromRecord. */
nfs::u32 stateRecord(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> next = stateFromRecord(app, ts, args.eax);
    if (!next)
        return args.eax;
    args.eax = *next;
    ts.sp += 4;
    return args.eax;
}

// The polygon lists' drawers, by a record's kind.
constexpr u32 kDrawers = 0x5f0780;

/* The polygon list from `record`: state 0xd 1, then each record handed to
 * the drawer of its kind (called by address: the table's), which gives back
 * the next.  eax as the last call leaves it (0 after a drawer, the state's
 * answer for an empty list).  False when the game stopped. */
bool drawList(App* app, ThreadState& ts, u32 record)
{
    if (!setRenderState(app, ts, 0xd, 1))
        return false;
    while (record != 0)
    {
        safepoint(app, ts);
        const u32 kind = u32(s32(at(app, record + 2)) >> 16);
        const std::optional<u32> next = callWith(app, ts, at(app, kind * 4 + kDrawers), {record});
        if (!next)
            return false;
        record = *next;
    }
    safepoint(app, ts);
    return true;
}

/* 0x434950: drawList from eax. */
nfs::u32 drawRecords(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (!drawList(app, ts, args.eax))
        return args.eax;
    ts.sp += 4;
    return args.eax;
}

/* 0x434990: the race's text for the frame (0x4bed70, loopb's, with 0x7cdac0); eax as it leaves
 * it. */
nfs::u32 raceTextFrame(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const std::optional<u32> clipped = feCore<0x4bed70, frontend::loopb::useClip>(app, ts, 0x7cdac0u);
    if (!clipped)
        return args.eax;
    args.eax = *clipped;
    ts.sp += 4;
    return args.eax;
}

// Whether the view is drawn mirrored.
constexpr u32 kViewMirrored = 0x554e4c;

/* Whether the view at `view` is mirrored (its first word 1) against the
 * mirrored screen, into kViewMirrored; that. */
u32 setViewMirrored(App* app, u32 view)
{
    const u32 mirrored = (at(app, view) == 1 ? 1 : 0) ^ gameSettings(app).mirrored;
    put(app, kViewMirrored, mirrored);
    return mirrored;
}

/* 0x4349a0: setViewMirrored for the view at eax. */
nfs::u32 setMirrored(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = setViewMirrored(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x4349d0: setViewMirrored for the view at eax, then the polygon list on
 * the stack's first word drawn (drawList); ret 8. */
nfs::u32 drawMirroredRecords(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 list = at(app, ts.sp + 4);
    setViewMirrored(app, args.eax);
    if (!drawList(app, ts, list))
        return args.eax;
    ts.sp += 4 + 8;
    return args.eax;
}

// The frames drawn.
constexpr u32 kFramesDrawn = 0x5f1fa8;

/* 0x434a70: the frames drawn counted. */
nfs::u32 countFrame(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    put(app, kFramesDrawn, at(app, kFramesDrawn) + 1);
    ts.sp += 4;
    return args.eax;
}

/* 0x434a80: the race's view distance (0x41d610 with the settings'; by
 * address: the eax it leaves is this one's, and track's core gives nothing
 * back). */
nfs::u32 raceViewDistance(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 distance = gameSettings(app).viewDistance;
    guest::direct<0x41d610, track::chooseViewDistance>(app, ts, distance);
    if (ts.terminate)
        return args.eax;
    args.eax = distance;  // it leaves eax as handed
    ts.sp += 4;
    return args.eax;
}

// The race's fog colour: blue, green, red bytes.
constexpr u32 kFogColour = 0x6ff9f8;

/* The race's fog colour set: its bytes as blue, green and red, alpha 0xff. */
bool fogColour(App* app, ThreadState& ts)
{
    const u32 colour = 0xff000000 | u32(byte(app, kFogColour + 2)) << 16 | u32(byte(app, kFogColour + 1)) << 8
                         | byte(app, kFogColour);
    return setRenderState(app, ts, kFogColourState, colour);
}

/* 0x434a90: fogColour. */
nfs::u32 raceFrameEnd(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return statesThunk(app, ts, args, fogColour);
}

/* 0x434ae0: nothing at the race's frame (an empty function). */
nfs::u32 raceFrameNothing(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    return args.eax;
}

/* 0x434af0: nothing at the race's end (an empty function). */
nfs::u32 raceEndNothing(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    return args.eax;
}

/* 0x434b00: the drawing made ready at the race's start: the reciprocals'
 * table (0x4eb3c0), the polygon lists' drawers (0x434870, by address: the
 * eax it leaves is this one's, and render's core gives nothing back). */
nfs::u32 raceStartDrawing(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (!guest::direct<0x4eb3c0, graphics::reciprocalsMade>(app, ts)
        || !guest::direct<0x434870, render::polygonDrawersSet>(app, ts))
        return args.eax;
    ts.sp += 4;
    return args.eax;
}

/* 0x434b10: nothing at the race's start (an empty function). */
nfs::u32 raceStartNothing(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    return args.eax;
}

// The screen's clip edges, as a float's bits: the far depth, left, right,
// top and bottom.
constexpr u32 kClipFar = 0x7d34f8, kClipLeft = 0x7d34fc, kClipRight = 0x7d350c, kClipTop = 0x7d3504;
constexpr u32 kClipBottom = 0x7d3500;
// A drawn corner, 0x20 bytes: x, y, depth, w, its colour, its clip code (a
// byte), its texture's u and v.
constexpr u32 kVertexX = 0, kVertexY = 4, kVertexZ = 8, kVertexW = 0xc, kVertexColour = 0x10;
constexpr u32 kVertexClip = 0x14, kVertexU = 0x18, kVertexV = 0x1c;
constexpr u32 kJustUnderOne = 0x3f7fff00, kWhite = 0xffffffff;
// The mouse's position (its four words to eax, edx, ebx and ecx), the view's
// corner (0x4dab10, menue's: to eax and edx), a quad drawn (its corners eax,
// edx, ebx, ecx).
// The pointers' records (0x2c bytes: the texture, then the corners' u and
// v), which is shown.
constexpr u32 kPointers = 0x8b41dc, kPointer = 0x563d3c, kPointerRecordBytes = 0x2c;

namespace
{

/* A corner's clip code (as native_graphics.cpp's shapeClipCode has it): 0x10
 * for its w at or past the far edge, else 8 above or 4 below, 1 left of or 2
 * right of the screen's rectangle; each a float's bits compared as integers,
 * a negative one as short. */
u8 clipCodeOf(App* app, u32 corner)
{
    const u32 w = at(app, corner + kVertexW);
    if ((w & 0x80000000) || s32(w) >= s32(at(app, kClipFar)))
        return 0x10;
    u8 code = 0;
    const u32 y = at(app, corner + kVertexY);
    if ((y & 0x80000000) || s32(y) < s32(at(app, kClipTop)))
        code |= 8;
    else if (s32(y) > s32(at(app, kClipBottom)))
        code |= 4;
    const u32 x = at(app, corner + kVertexX);
    if ((x & 0x80000000) || s32(x) < s32(at(app, kClipLeft)))
        code |= 1;
    else if (s32(x) > s32(at(app, kClipRight)))
        code |= 2;
    return code;
}

/* The mouse's pointer drawn in the race: where the mouse is, less the view's
 * corner, as a 32 by 32 quad of four corners in the frame (where the calls
 * are handed them) -- at depth 0, w just under 1, white -- its texture's
 * coordinates and the texture from the pointer's record, each corner's clip
 * code, the texture set (state 1) and the quad drawn.  False when the game
 * stopped. */
bool pointer(App* app, ThreadState& ts)
{
    const GuestFrame frame(ts, 0x18, 0x9c);
    const u32 f = frame - 0x82;
    const u32 mouse = f + 0x6e, view = f + 0x66;
    const u32 corners[4] = {f - 0x1a, f + 6, f + 0x26, f + 0x46};
    if (!guest::direct<0x435310, controls::mouseRead>(app, ts, mouse, mouse + 4, mouse + 8, mouse + 0xc))
        return false;
    if (!feCore<0x4dab10, frontend::menue::pointerHotSpot>(app, ts, view, view + 4))
        return false;
    // The corners' x and y: the mouse's, and 32 on, each a float as stored.
    const float left = float(Float(s32(at(app, mouse + 4) - at(app, view))));
    storeFloat(app, corners[3] + kVertexX, left);
    for (const u32 c : {3, 2, 1, 0})
        put(app, corners[c] + kVertexW, kJustUnderOne);
    for (const u32 c : {2, 1, 0})
        put(app, corners[c] + kVertexX, at(app, corners[3] + kVertexX));
    const Float size = loadFloat(app, kPointerSize);
    storeFloat(app, corners[1] + kVertexX, fp::add(loadFloat(app, corners[3] + kVertexX), size));
    const float top = float(Float(s32(at(app, mouse + 8) - at(app, view + 4))));
    storeFloat(app, corners[3] + kVertexY, top);
    for (const u32 c : {2, 1, 0})
        put(app, corners[c] + kVertexY, at(app, corners[3] + kVertexY));
    storeFloat(app, corners[2] + kVertexY, fp::add(size, loadFloat(app, corners[3] + kVertexY)));
    put(app, corners[2] + kVertexX, at(app, corners[1] + kVertexX));
    put(app, corners[3] + kVertexY, at(app, corners[2] + kVertexY));
    // The pointer's record: its texture, then each corner's u and v.
    const u32 record = kPointers + at(app, kPointer) * kPointerRecordBytes;
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
    for (const u32 corner : corners)
        putByte(app, corner + kVertexClip, clipCodeOf(app, corner));
    if (!setRenderState(app, ts, kTextureState, at(app, kPointers + at(app, kPointer) * kPointerRecordBytes)))
        return false;
    return drawQuad(app, ts, corners[0], corners[1], corners[2], corners[3]);
}

}  // namespace

/* 0x434b60: pointer. */
nfs::u32 drawPointer(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    if (!pointer(app, ts))
        return args.eax;
    ts.sp += 4;
    return args.eax;
}

// The race's messages (chat): their channels and lines in a block of
// 0x24c8 bytes (0x554f04) -- the line's length (+0), the lines a channel
// keeps (+4), the channels (+8), its kind (+0xc), the copy's start (+0x10),
// scrolling (+0x20), and from +0x28 a channel of 0x494 bytes each (its
// line kept +0, its first +4, its next +8, from +0xc lines of 0x74 bytes:
// where its text is +0, +4, its length +8, its text from +0xc) -- the
// lines copied out into another of 0x3f8 (0x554f08: its count, then lines
// of 0x65 bytes), and whether they are in use (0x554f0c).
constexpr u32 kMessages = 0x554f04, kMessageCopy = 0x554f08, kMessagesOn = 0x554f0c;
constexpr u32 kMessagesBytes = 0x24c8, kCopyBytes = 0x3f8;
constexpr u32 kLineLength = 0, kLinesKept = 4, kChannelCount = 8, kMessagesKind = 0xc, kCopyStart = 0x10;
constexpr u32 kScrolling = 0x20, kChannels = 0x28;
constexpr u32 kChannelBytes = 0x494, kChannelKept = 0, kChannelFirst = 4, kChannelNext = 8, kChannelLines = 0xc;
constexpr u32 kLineBytes = 0x74, kLineStart = 0, kLineFrom = 4, kLineCount = 8, kLineText = 0xc;
constexpr u32 kCopyLineBytes = 0x65;
constexpr u32 kMessagesAdded = 0x5f3fd8;  // the texts added, counted

namespace
{

/* The game's allocate (0x4e1620, ea::reservememadr): `bytes`, of no name, aligned to 0x10. */
std::optional<u32> allocate(App* app, ThreadState& ts, u32 bytes)
{
    constexpr u32 kNoName = 0x537490, kAlignment = 0x10;
    return guest::direct<0x4e1620, ea::reserved>(app, ts, kNoName, bytes, kAlignment);
}

/* The game's fill with zeros (0x4e070c): `bytes` at `block`. */
bool fillZero(App* app, ThreadState& ts, u32 block, u32 bytes)
{
    guest::direct<0x4e070c, ea::clearBytes>(app, ts, block, bytes);
    return !ts.terminate;
}

/* The game's memmove (0x4ea4f0, ea::memmove): `bytes` from `from` to `to`. */
bool moveMemory(App* app, ThreadState& ts, u32 from, u32 to, u32 bytes)
{
    return guest::direct<0x4ea4f0, ea::moveMemory>(app, ts, to, from, bytes);
}

/* The game's free (0x4e1890, ea::purgememadr): what it gives back. */
std::optional<u32> freeBlock(App* app, ThreadState& ts, u32 block)
{
    return guest::direct<0x4e1890, ea::purgeBlock>(app, ts, block);
}

/* idiv: `value` (its sign extended) divided by `by`, the remainder. */
u32 remainder(u32 value, u32 by)
{
    return u32(std::int64_t(s32(value)) % s32(by));
}

/* The thunk of a messages' function: the result in eax. */
nfs::u32 returns(ThreadState& ts, nfs::Args& args, const std::optional<u32>& result)
{
    if (!result)
        return args.eax;
    args.eax = *result;
    ts.sp += 4;
    return args.eax;
}

}  // namespace

/* The messages' blocks allocated and cleared, their settings from the eight
 * words at `settings` (the line's length and the lines kept made larger by
 * its +0xc, the second if the lines kept are more than 1), the copy's count
 * the lines kept as given; in use.  The copy's block, as the code leaves it
 * in eax. */
std::optional<u32> openChannels(App* app, ThreadState& ts, u32 settings)
{
    u32 length = at(app, settings + 8), lines = at(app, settings + 4);
    const u32 more = at(app, settings + 0xc);
    if (more != 0)
    {
        if (s32(lines) > 1)
            lines += more;
        else
            length += more;
    }
    const std::optional<u32> messages = allocate(app, ts, kMessagesBytes);
    if (!messages)
        return std::nullopt;
    put(app, kMessages, *messages);
    if (!fillZero(app, ts, *messages, kMessagesBytes)
        || !moveMemory(app, ts, settings, at(app, kMessages) + kChannelCount, 0x20))
        return std::nullopt;
    const u32 block = at(app, kMessages);
    put(app, block + kLinesKept, lines);
    put(app, block + kLineLength, length);
    const std::optional<u32> copy = allocate(app, ts, kCopyBytes);
    if (!copy)
        return std::nullopt;
    put(app, kMessageCopy, *copy);
    if (!fillZero(app, ts, *copy, kCopyBytes))
        return std::nullopt;
    const u32 copyAt = at(app, kMessageCopy);
    put(app, copyAt, at(app, settings + 4));
    put(app, kMessagesOn, 1);
    return copyAt;
}

/* The messages out of use, their blocks freed.  What the code leaves in eax:
 * what the last free gives back, or `eax` as the caller had it where there
 * is nothing to free; nullopt when the game stopped. */
std::optional<u32> closeChannels(App* app, ThreadState& ts, u32 eax)
{
    const u32 messages = at(app, kMessages);
    put(app, kMessagesOn, 0);
    std::optional<u32> left = eax;
    if (messages != 0 && !(left = freeBlock(app, ts, messages)))
        return std::nullopt;
    const u32 copy = at(app, kMessageCopy);
    if (copy != 0 && !(left = freeBlock(app, ts, copy)))
        return std::nullopt;
    put(app, kMessages, 0);
    put(app, kMessageCopy, 0);
    return left;
}

namespace
{

/* Where line `line` of channel `channel` is. */
u32 lineAt(App* app, u32 channel, u32 line)
{
    return at(app, kMessages) + kChannels + channel * kChannelBytes + kChannelLines + line * kLineBytes;
}

/* Line `line` of channel `channel` cleared: its words and its text's first
 * byte 0. */
void clearLine(App* app, u32 channel, u32 line)
{
    const u32 to = lineAt(app, channel, line);
    put(app, to + kLineStart, 0);
    put(app, to + kLineFrom, 0);
    put(app, to + kLineCount, 0);
    putByte(app, to + kLineText, 0);
}

/* Channel `channel` cleared while the messages are in use: its lines, its
 * kept, first and next line 0 -- out of use the while.  What the code leaves
 * in eax: the messages' block, or the channel out of use. */
u32 clearChannel(App* app, u32 channel)
{
    const u32 on = at(app, kMessagesOn);
    if (on == 0)
        return channel;
    put(app, kMessagesOn, 0);
    for (u32 line = 0; s32(line) < s32(at(app, at(app, kMessages) + kLinesKept)); ++line)
        clearLine(app, channel, line);
    const u32 messages = at(app, kMessages);
    const u32 to = messages + channel * kChannelBytes + kChannels;
    put(app, to + kChannelKept, 0);
    put(app, to + kChannelFirst, 0);
    put(app, kMessagesOn, on);
    put(app, to + kChannelNext, 0);
    return messages;
}

}  // namespace

/* Every channel cleared.  The messages' block, as the code leaves it in eax. */
u32 clearAllChannels(App* app)
{
    for (u32 channel = 0; s32(channel) < s32(at(app, at(app, kMessages) + kChannelCount)); ++channel)
        clearChannel(app, channel);
    return at(app, kMessages);
}

namespace
{

/* What a channel's scrolling does: nothing but read the messages' block. */
u32 channelScrolled(App* app)
{
    return at(app, kMessages);
}

/* The character `character` added to channel `channel`'s next line,
 * while the messages are in use: at the line's length, for messages of kind
 * 1, the line starts again and its +4 moves on (of the line's length plus
 * one); for another kind the next line taken (cleared) and the first moved
 * on as they meet, scrolled, and the kept the first.  Its text ended after
 * it.  1, or 0 out of use. */
u32 addCharacter(App* app, u32 channel, u8 character)
{
    if (at(app, kMessagesOn) == 0)
        return 0;
    const u32 messages = at(app, kMessages);
    const u32 offset = channel * kChannelBytes;
    const u32 channelAt = offset + messages + kChannels;
    const u32 line = messages + kChannels + offset + kChannelLines + at(app, channelAt + kChannelNext) * kLineBytes;
    const u32 count = at(app, line + kLineCount);
    put(app, line + kLineCount, count + 1);
    putByte(app, line + kLineText + count, character);
    if (at(app, messages + kMessagesKind) == 1)
    {
        if (s32(at(app, messages + kLineLength)) < s32(at(app, line + kLineCount)))
            put(app, line + kLineCount, 0);
        const u32 from = at(app, line + kLineFrom);
        if (from == at(app, line + kLineCount))
        {
            put(app, line + kLineFrom, from + 1);
            put(app, line + kLineFrom, remainder(from + 1, at(app, at(app, kMessages) + kLineLength) + 1));
        }
        put(app, line + kLineStart, at(app, line + kLineFrom));
    }
    else
    {
        if (at(app, messages + kLineLength) == at(app, line + kLineCount))
        {
            const u32 next = at(app, channelAt + kChannelNext) + 1;
            put(app, channelAt + kChannelNext, next);
            const u32 taken = remainder(next, at(app, messages + kLinesKept));
            put(app, channelAt + kChannelNext, taken);
            clearLine(app, channel, taken);
            const u32 block = at(app, kMessages);
            const u32 now = offset + block + kChannels;
            const u32 first = at(app, now + kChannelFirst);
            if (at(app, now + kChannelNext) == first)
            {
                put(app, now + kChannelFirst, first + 1);
                put(app, now + kChannelFirst, remainder(first + 1, at(app, block + kLinesKept)));
            }
        }
        if (at(app, at(app, kMessages) + kScrolling) != 0)
            channelScrolled(app);
        const u32 now = at(app, kMessages) + channel * kChannelBytes + kChannels;
        put(app, now + kChannelKept, at(app, now + kChannelFirst));
    }
    putByte(app, line + kLineText + at(app, line + kLineCount), 0);
    return 1;
}

}  // namespace

/* The text at `text` added to channel `channel` a character at a time, up
 * to the line's length, the rest of the line filled with 0s; the additions
 * counted.  1, or 0 out of use or when a character could not be added. */
u32 addText(App* app, u32 channel, u32 text)
{
    if (at(app, kMessagesOn) == 0)
        return 0;
    put(app, kMessagesAdded, at(app, kMessagesAdded) + 1);
    u32 added = 0;
    for (; byte(app, text) != 0 && s32(added) < s32(at(app, at(app, kMessages) + kLineLength)); ++added, ++text)
        if (addCharacter(app, channel, byte(app, text)) == 0)
            return 0;
    for (; s32(added) < s32(at(app, at(app, kMessages) + kLineLength)); ++added)
        if (addCharacter(app, channel, 0) == 0)
            return 0;
    return 1;
}

namespace
{

/* Channel `channel`'s lines copied out: for a copy of one line, as many of
 * the channel's text bytes from the copy's start as fit, in one or two
 * pieces; else each of its lines from the kept one on (of the lines kept,
 * round), 0x65 bytes apart, by the code's inlined strcpy.  The copy's block. */
std::optional<u32> copyChannel(App* app, ThreadState& ts, u32 channel)
{
    const u32 offset = channel * kChannelBytes;
    const u32 count = at(app, at(app, kMessageCopy));
    if (count == 1)
    {
        const u32 messages = at(app, kMessages);
        const u32 wanted = at(app, messages + kCopyStart) + count;
        const u32 start = at(app, offset + messages + kChannels + kChannelLines + kLineStart);
        u32 piece = wanted;
        if (s32(wanted) < s32(start + wanted))
            piece = at(app, messages + kLineLength) - start + 1;
        const u32 text = at(app, kMessages) + kChannels + offset + kChannelLines + kLineText;
        if (!moveMemory(app, ts, text + start, at(app, kMessageCopy) + 4, piece))
            return std::nullopt;
        if (s32(piece) < s32(wanted)
            && !moveMemory(app, ts, at(app, kMessages) + kChannels + offset + kChannelLines + kLineText,
                           at(app, kMessageCopy) + 4 + piece, wanted - piece))
            return std::nullopt;
    }
    else
    {
        const u32 kept = at(app, offset + at(app, kMessages) + kChannels + kChannelKept);
        for (u32 i = 0; s32(i) < s32(at(app, at(app, kMessageCopy))); ++i)
        {
            const u32 messages = at(app, kMessages);
            const u32 line = remainder(kept + i, at(app, messages + kLinesKept));
            u32 from = channel * kChannelBytes + messages + kChannels + kChannelLines + line * kLineBytes + kLineText;
            u32 to = at(app, kMessageCopy) + 4 + i * kCopyLineBytes;
            for (;;)
            {
                const u8 c = byte(app, from++);
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
nfs::u32 openMessages(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return returns(ts, args, openChannels(app, ts, args.eax));
}

/* 0x436cf0: closeChannels, eax as the caller had it. */
nfs::u32 closeMessages(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return returns(ts, args, closeChannels(app, ts, args.eax));
}

/* 0x436d40: clearAllChannels. */
nfs::u32 clearAllMessages(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = clearAllChannels(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x436d60: clearChannel for channel eax. */
nfs::u32 clearMessages(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = clearChannel(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* 0x436dc0: clearLine for line edx of channel eax; eax the channel's lines,
 * as the code leaves it. */
nfs::u32 clearMessageLine(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    const u32 channel = args.eax, line = args.edx;
    const u32 lines = at(app, kMessages) + kChannels + channel * kChannelBytes + kChannelLines;
    clearLine(app, channel, line);
    args.eax = lines;
    ts.sp += 4;
    return args.eax;
}

/* 0x436e10: addText of the text at edx to channel eax. */
nfs::u32 addMessageText(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = addText(app, args.eax, args.edx);
    ts.sp += 4;
    return args.eax;
}

/* 0x436e90: addCharacter of dl to channel eax. */
nfs::u32 addMessageCharacter(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = addCharacter(app, args.eax, u8(args.edx));
    ts.sp += 4;
    return args.eax;
}

/* 0x437080: copyChannel of channel eax. */
nfs::u32 copyMessageLines(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    return returns(ts, args, copyChannel(app, ts, args.eax));
}

/* 0x4371a0: channelScrolled (the code compares the block's kind with 1, for
 * flags no caller reads). */
nfs::u32 messagesScrolled(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = channelScrolled(app);
    ts.sp += 4;
    return args.eax;
}

/* 0x4371c0: nothing (an empty function). */
nfs::u32 messagesNothing(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    ts.sp += 4;
    return args.eax;
}

// The hash's seed: the whole product, its low 16 bits, the odd seed.
constexpr u32 kHashProduct = 0x554f14, kHashSeed16 = 0x554f10, kHashOdd = 0x554f18;

/* A hash's seed from `seed`: it made odd and three times it made odd,
 * multiplied eight times by the first, kept to 16 bits but the last.  The
 * odd three times, as the code leaves it in eax. */
u32 seedTheHash(App* app, u32 seed)
{
    const u32 odd = (seed & 1) ? seed : seed + 1;
    u32 triple = seed * 3;
    if (!(triple & 1))
        ++triple;
    u32 product = (odd * triple) & 0xffff;
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
nfs::u32 seedHash(App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    args.eax = seedTheHash(app, args.eax);
    ts.sp += 4;
    return args.eax;
}

/* The typed cores the game's other code calls (native_collide.h), under the game's addresses. */
bool picturesStated(App* app, ThreadState& ts)
{
    return pictureStates(app, ts);
}

std::optional<u32> messageLinesCopied(App* app, ThreadState& ts, u32 channel)
{
    return copyChannel(app, ts, channel);
}

std::optional<u32> raceTextOfFrame(App* app, ThreadState& ts)
{
    return feCore<0x4bed70, frontend::loopb::useClip>(app, ts, 0x7cdac0u);
}

void frameCounted(App* app)
{
    put(app, kFramesDrawn, at(app, kFramesDrawn) + 1);
}

bool raceStartPrepared(App* app, ThreadState& ts)
{
    return guest::direct<0x4eb3c0, graphics::reciprocalsMade>(app, ts)
           && guest::direct<0x434870, render::polygonDrawersSet>(app, ts);
}

void carListCleared(App* app)
{
    put(app, kCarCount, 0);
}

bool listDrawnMirrored(App* app, ThreadState& ts, u32 view, u32 list)
{
    setViewMirrored(app, view);
    return drawList(app, ts, list);
}

}  // namespace collide
}  // namespace nfs3hp
