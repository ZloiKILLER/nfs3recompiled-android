#include "native_frontend_rendd.h"
#include "native_collide.h"
#include "native_game.h"
#include "native_load_carmodels.h"
#include "native_watcom.h"
#include "game_settings.h"
#include "native_carstate.h"
#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_loopa.h"
#include "native_frontend_renda.h"
#include "native_frontend_rendc.h"
#include "native_frontend_rende.h"
#include "native_frontend_sima.h"
#include "native_frontend_wina.h"
#include "native_trackdata.h"
#include "native_vertices.h"
#include "thrash_calls.h"
#include "native_render.h"
#include <nfs3hp.h>
#include <cstdint>
#include <cstring>

/* The thunks defined below under their qualified names. */
namespace nfs3hp
{
namespace frontend
{
nfs::u32 rendFreeBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendChooseDriver(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendReadDriverFeatures(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendFreePlayerModel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendFreeCarModel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendShadowCorners(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendCarShadow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendCarLights(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendBodyShake(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendFrontWheelTurn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendRearWheelTurn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendBodyLean(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendTrackLitColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendReplayPose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendCarStates(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendCarShade(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendCarAverageColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendCarFarthest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendCarFar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendCarNear(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendCarSimpleNear(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendDrawCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendDetailInUse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendLowerDetail(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendFitDetailBudget(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendCars(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendArenaRewind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendArenaCreate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendArenaFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendArenaTake(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendArenaHere(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendEmptyList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendListOfOne(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendJoinLists(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendOpenListStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendOpenListAppend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendOpenListTake(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendOpenListClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendFlushAndList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendDrawListPlain(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendDrawListAt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendDrawListMirrored(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendFillReciprocals(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendSetClipBounds(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendSetProjection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 rendProjectPoints(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}
}

/* The race's cars as the renderer sees them, decompiled:
 *
 *   the frame's arena   the block drawing allocates from, rewound each pass, and
 *                       the linked lists of records drawn from it (sub_4bbd70 to
 *                       sub_4bc0b0);
 *   the cars            the pass over them all (sub_4bbba0), the detail each may
 *                       have within the budget (sub_4bbad0), one car drawn at its
 *                       detail (sub_4bb4c0, sub_4ba960 and its neighbours), its
 *                       wheels' turn (sub_4b9140, sub_4b9280), its lean (sub_4b93c0),
 *                       its lights (sub_4b8740), its shadow on the road (sub_4b81a0),
 *                       its colours (sub_4ba350, sub_4ba810, sub_4b9550);
 *   the replay's pose   the recorded car drawn as a ghost (sub_4b9df0);
 *   the projection      its numbers (sub_4bf1b0, sub_4bf260) and the points through
 *                       it (sub_4bf370), and the table of reciprocals (sub_4bf110).
 *
 * The cars' state stays in guest memory, where the game reads it.  Only what is handed
 * to a callee by address is kept in the frame: the rest of the game's frame
 * is plain locals here.  A value on
 * the x87 is a double, as the game's code holds it, and each operation rounds
 * through fp:: as it does, in its order and operand order: a float's 24 bits.
 *
 * The registers no caller reads are left as the game's code leaves them, which
 * is what leaveEbx and the others are doing here.
 *
 * Plan step 5: each function a typed core in frontend::rendd (native_frontend_rendd.h), in a
 * frame of its own where the game's function has one (fe::Frame: what it calls runs at the stack
 * addresses it did), and a thunk under the game's name for the calls by address.  A core of
 * this round's three files (rendd, rende, loada) calls another as C++ (guest::direct), and the
 * typed cores of other files too (fe::direct); the rest of the game still by address. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

// Port (the port's vertex hooks change): only watches how much of the arena a pass used.
bool arenaReset(nfs::App* app, nfs::ThreadState& ts);
// Port (the port's car detail change): the frame a car's wheels last moved, per car and per use.
nfs::u32 carWheelStamp(nfs::App* app, nfs::u32 car, nfs::u32 which);

namespace
{

using fe::args;
using fe::Guest;
using nfs::u32;
using nfs::s32;

/* Guest::call as a call through guest::pointerCall: `arguments`' registers in eax, edx, ebx and ecx, `esi` in esi,
 * its stack words pushed in order, esp back after; Terminated when the game stopped in it.  `floatResult`, when given,
 * takes the float the callee gives back. */
inline u32 callAt(Guest& guest, u32 function, const fe::Arguments& arguments = fe::Arguments(), u32 esi = 0,
                    double* floatResult = nullptr)
{
    nfs::ThreadState& ts = guest.ts();
    nfs::Args registers;
    u32* const into[4] = { &registers.eax, &registers.edx, &registers.ebx, &registers.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *into[i] = arguments.registers[i];
    registers.esi = esi;
    const u32 esp = ts.sp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        ts.sp -= 4;
        guest.setDword(ts.sp, arguments.stack[i]);
    }
    const std::optional<u32> result = guest::pointerCall(guest.app(), ts, function, registers);
    if (!result)
        throw fe::Terminated();
    ts.sp = esp;
    if (floatResult)
        *floatResult = registers.floatResult;  // what the callee leaves on the x87's top, as a float result
    return *result;
}

// ---------------------------------------------------------------------------
// The x87 as the game's code holds it
// ---------------------------------------------------------------------------

// fp::compare's word, as a jump after sahf reads it: ja, and jb (true unordered).
inline bool isAbove(nfs::u16 fsw) { return fsw == 0; }
inline bool isBelow(nfs::u16 fsw) { return (fsw & 0x100) != 0; }

// fld dword, fld qword.
inline double floatOf(const Guest& guest, u32 address) { return double(guest.floatAt(address)); }
inline double doubleOf(const Guest& guest, u32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// A float as it goes through an integer register, and back.
inline u32 bitsOf(float value)
{
    u32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}
inline float floatFrom(u32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}
// A double built from its two halves, as the code moves constants into a frame.
inline double doubleFrom(u32 low, u32 high)
{
    const std::uint64_t bits = std::uint64_t(low) | (std::uint64_t(high) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

inline s32 fistp(double value) { return fp::toInteger<s32>(value); }

// A value from the x87 as the game keeps a byte: below 0 and past 255 cut, fistp each
// time.  The first from the stack, the others from where it was stored as a float.
inline u32 clampedByte(double first, float stored)
{
    if (fistp(first) < 0)
        return 0;
    if (fistp(double(stored)) > 0xff)
        return 0xff;
    return u32(fistp(double(stored)));
}

inline void copyDwords(Guest& guest, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* A colour's four bytes times a fraction of 0x10000, as the game's macro has it; ebx
 * left with what it leaves there. */
inline u32 scaledColour(nfs::ThreadState& ts, u32 colour, u32 factor)
{
    if (s32(factor) >= 0x10000)
        factor = 0xffff;
    const u32 by = (factor >> 8) & 0xff;
    const u32 redBlue = (colour & 0x00ff00ff) * by;
    const u32 alphaGreen = ((colour & 0xff00ff00) >> 8) * by;
    const u32 ebx = (redBlue >> 8) & 0x00ff00ff;
    return (alphaGreen & 0xff00ff00) | ebx;
}

/* The clip code of a vertex on the screen, from its reciprocal depth and position as
 * integers: 0x10 behind or too near, else 8 and 4 above and below, 1 and 2 left and right.
 * The floats compared as their bits, as the game does; a negative one is out. */
struct ClipBounds
{
    s32 nearest, top, bottom, left, right;
};

inline ClipBounds clipBounds(const Guest& guest)
{
    return { s32(guest.dword(0x7d34f8)), s32(guest.dword(0x7d3504)), s32(guest.dword(0x7d3500)),
             s32(guest.dword(0x7d34fc)), s32(guest.dword(0x7d350c)) };
}

inline std::uint8_t clipCode(const ClipBounds& bounds, s32 inverseDepth, s32 y, s32 x)
{
    if (inverseDepth < 0 || inverseDepth >= bounds.nearest)
        return 0x10;
    std::uint8_t code = 0;
    if (y < 0 || y < bounds.top)
        code |= 8;
    else if (y > bounds.bottom)
        code |= 4;
    if (x < 0 || x < bounds.left)
        code |= 1;
    else if (x > bounds.right)
        code |= 2;
    return code;
}

// What a zero reciprocal is kept from: the smallest depth the game divides by.
constexpr u32 kSmallestDepth = 0x37800080;

// ---------------------------------------------------------------------------
// The game's functions and data, by address
// ---------------------------------------------------------------------------

constexpr u32 kLog = 0x401010;               // the game's printf to its log
constexpr u32 kCompareNoCase = 0x4eb390;     // eax, edx, ebx the most
constexpr u32 kEndModelBatch = 0x49d640;
constexpr u32 kTrackPointAt = 0x420650;      // eax the point, edx 1, ebx the segment, esi the record it fills
constexpr u32 kTransformPoints = 0x4e0430;   // eax how many, edx them, ebx a matrix, ecx a place; the stack where to
constexpr u32 kRotationY = 0x4dfde0;
constexpr u32 kRotationZ = 0x4dfe40;
constexpr u32 kMultiplyMatrices = 0x4e0280;  // eax by edx to ebx
constexpr u32 kGunFire = 0x42cff0;           // eax the car: how far through its flash, on the x87
constexpr u32 kFlare = 0x492370;             // eax a view; the stack the point, colour, size...
constexpr u32 kLightFlare = 0x491bc0;        // likewise, a record of the road's light back
constexpr u32 kRingFlare = 0x492980;
constexpr u32 kDrawModelPart = 0x49de20;     // eax the model, edx a part, ebx a part's matrix, ecx its offset
constexpr u32 kRgbToHsv = 0x49c880;          // eax to edx, three dwords
constexpr u32 kHsvToRgb = 0x49ca50;
constexpr u32 kNightColour = 0x4b97b0;       // eax the car, edx its colour
constexpr u32 kCarVisible = 0x4bb5d0;        // eax a view, edx a car, ebx where its distance goes: its detail, -1 if none
constexpr u32 kDrawCarDetail = 0x4bae00;
constexpr u32 kDrawListAtDepth = 0x4c7790;
constexpr u32 kDrawListSoftware = 0x4349d0;
constexpr u32 kFlushVertices = 0x4c79a0;
constexpr u32 kListVertices = 0x4c7750;

// This group's own still called by address: through a table, one whose edx the caller reads, and the
// lights, which read their frame's leftovers.
constexpr u32 kCarLights = 0x4b8740;
constexpr u32 kCarFarthest = 0x4ba960;
constexpr u32 kCarFar = 0x4baa50;
constexpr u32 kCarNear = 0x4bab70;
constexpr u32 kFreeBlock = 0x4b5700;

// The arena: the block, the next free byte, its size; and the open list on it.
constexpr u32 kArena = 0x55fe34;
constexpr u32 kArenaNext = 0x55fe38;
constexpr u32 kArenaSize = 0x7a3d04;
constexpr u32 kOpenListTail = 0x7a3d08;
constexpr u32 kOpenListHead = 0x7a3d0c;

constexpr u32 kFrameCount = 0x7d3684;
constexpr u32 kWheelStamp = 0x55fe28;
constexpr u32 kDriverOnCard = 0x7a3a60;      // 1 but on the software renderers
constexpr u32 kDepthBias = 0x7dcfe0;
constexpr u32 kSun = 0x55d2a8;
constexpr u32 kCarTable = 0x5efac8;          // the cars
constexpr u32 kCarCount = 0x5efd9c;
constexpr u32 kPlayerCars = 0x5efa48;        // the players' cars
constexpr u32 kReplayModel = 0x55fe24;
constexpr u32 kReplayShowing = 0x55d000;     // 1 when the replay's pose is drawn
constexpr u32 kPlayerModel = 0x55fe2c;
constexpr u32 kDetailDistances = 0x55fdf4;   // per setting 16 bytes: three distances and the last
constexpr u32 kDetailBudget = 0x4b7a50;      // in the code: three dwords, per setting
constexpr u32 kAmbientBlue = 0x6ff9f8;       // three bytes
constexpr u32 kRenderState = 0x563a90;       // a record; at +4 the light's flag
constexpr u32 kBackFaceFlip = 0x554e4c;
constexpr u32 kShadowTexture = 0x8b4898;
constexpr u32 kSegmentCount = 0x5dd940;
constexpr u32 kSegmentLights = 0x571370;     // every 8 segments 0x5c0 bytes: at +0x5b8 a count, at +0x5bc the lights

// The projection's numbers.
constexpr u32 kScaleX = 0x56009c;
constexpr u32 kScaleY = 0x5600a0;
constexpr u32 kCentreX = 0x5600a4;
constexpr u32 kCentreY = 0x5600a8;
constexpr u32 kHalfWidth = 0x5600ac;
constexpr u32 kHalfHeight = 0x5600b0;

// A car, as these read it.
constexpr u32 kCarSegment = 0x1c;
constexpr u32 kCarSpeedLow = 0x20;
constexpr u32 kCarPlace = 0x98;
constexpr u32 kCarAxes = 0xc0;               // a matrix of 9 floats
constexpr u32 kCarForward = 0xd8;
constexpr u32 kCarVelocityUnit = 0xa4;
constexpr u32 kCarHalfSize = 0x114;
constexpr u32 kCarFlags = 0x200;
constexpr u32 kCarModel = 0x8a4;
constexpr u32 kCarDetail = 0x8b4;
constexpr u32 kCarDepth = 0x8b8;
constexpr u32 kCarHasCop = 0x734;
constexpr u32 kCarLightsOn = 0x660;
constexpr u32 kCarBraking = 0x509;
constexpr std::uint8_t kFlagCop = 0x20;
constexpr std::uint8_t kFlagHidden = 0x40;

// A model.
constexpr u32 kModelParts = 0xf8;
constexpr u32 kModelLightCount = 0x34;
constexpr u32 kModelLights = 0x38;           // points of 12 bytes
constexpr u32 kModelLightNames = 0xa04;      // names of 0x40 bytes, each letter meaning something

// A track surface no shadow falls on.
constexpr std::uint8_t kSurfaceNoShadow = 0xe;

}

// ---------------------------------------------------------------------------
// Starting and ending
// ---------------------------------------------------------------------------

/* sub_4b5700: the block eax freed, if there is one. */
nfs::u32 frontend::rendd::freeBlock(App* app, ThreadState& ts, u32 block)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    // What the call leaves in eax, or the block, 0.
    u32 left = block;
    if (block != 0)
        left = *fe::direct<0x4e1890, ea::purgeBlock>(app, ts, block);
    return left;
}

nfs::u32 frontend::rendFreeBlock(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b5700", 0, 0, nullptr, 0, 0 };
    const u32 block = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::freeBlock(app, ts, block); });
}

/* sub_4b5720: which driver of the eleven (their names at 0x55fd68) the registry names, and
 * over that the switches of the command line (0x55eb2c, two bytes), to *eax. */
nfs::u32 frontend::rendd::chooseDriver(App* app, ThreadState& ts, u32 chosen)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x5c);
    constexpr u32 kDriverNames = 0x55fd68;
    constexpr u32 kDriverNameCount = 11;
    constexpr u32 kSwitches = 0x55eb2c;
    const u32 name = guest.frame();
    fe::direct<0x4b64c0, frontend::wina::thrashDescription>(app, ts, 0x7a3a74u);
    // The game's code hands it 0 in edx too, which it does not read.
    fe::direct<0x4b6400, frontend::wina::thrashDevice>(app, ts, name);
    u32 driver = 0;
    for (; driver < kDriverNameCount; ++driver)
    {
        // The count lives just past the name's 0x40 bytes: its low byte ends a name that fills them.
        guest.setDword(name + 0x40, driver);
        const u32 length = guest.length(name);
        if (fe::direct<kCompareNoCase, watcom::clib::strncmp>(guest.app(), guest.ts(), guest.dword(kDriverNames + 4 * driver), name, length) == 0)
            break;
    }
    if (driver == kDriverNameCount)
        fe::direct<0x4b5e50, frontend::wina::fatalMessage>(app, ts, 0u);
    guest.setDword(chosen, driver);
    const std::uint8_t low = guest.byte(kSwitches);
    const u32 left = (driver & 0xffff00ff) | (u32(low) << 8);
    if (low & 0x04)
        guest.setDword(chosen, 3);
    else if (low & 0x08)
        guest.setDword(chosen, 4);
    else if (low & 0x10)
        guest.setDword(chosen, 5);
    else if (low & 0x20)
        guest.setDword(chosen, 6);
    else if (low & 0x40)
        guest.setDword(chosen, 7);
    else if (low & 0x80)
        guest.setDword(chosen, 8);
    else
    {
        const std::uint8_t high = guest.byte(kSwitches + 1);
        if (high & 0x01)
            guest.setDword(chosen, 9);
        else if (high & 0x02)
            guest.setDword(chosen, 10);
        else if (low & 0x01)
            guest.setDword(chosen, 1);
        else if (low & 0x02)
            guest.setDword(chosen, 2);
    }
    return left;
}

nfs::u32 frontend::rendChooseDriver(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b5720", 0, 0, nullptr, 0 };
    const u32 chosen = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::chooseDriver(app, ts, chosen); });
}

/* sub_4b5940: which of the driver's five features answer (THRASH_setstate's pointer, by a
 * table of its two arguments at 0x55fd94), a bit each to 0x7a3a5c. */
nfs::u32 frontend::rendd::readDriverFeatures(App* app, ThreadState& ts, u32 first)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x18);
    constexpr u32 kFeatures = 0x7a3a5c;
    constexpr u32 kFeatureQueries = 0x55fd94;
    guest.setDword(kFeatures, 0);
    u32 bit = first;
    for (u32 i = 0; i < 5; ++i)
    {
        const std::optional<u32> answer = guest::driver<thrash::kThrashSetState, &port::driver::setState>(
            app, ts, guest.dword(kFeatureQueries + 8 * i), guest.dword(kFeatureQueries + 8 * i + 4));
        if (!answer)
            throw fe::Terminated();
        bit = (*answer != 0 ? 1u : 0u) << i;
        guest.setDword(kFeatures, guest.dword(kFeatures) | bit);
    }
    return bit;
}

nfs::u32 frontend::rendReadDriverFeatures(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b5940", 0, 0, nullptr, 0 };
    const u32 first = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::readDriverFeatures(app, ts, first); });
}

/* sub_4b7d00: the player's model freed, as a race ends, and the models' batch ended. */
nfs::u32 frontend::rendd::freePlayerModel(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xc);
    const u32 model = guest.dword(kPlayerModel);
    if (model != 0)
    {
        fe::direct<0x49d5a0, frontend::rendc::modelFree>(app, ts, model);
        guest.setDword(kPlayerModel, 0);   // ecx, its 0 kept across the call
    }
    return callAt(guest, kEndModelBatch);
}

nfs::u32 frontend::rendFreePlayerModel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b7d00", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::freePlayerModel(app, ts); });
}

/* sub_4b7eb0: car eax's model freed, and no car left pointing at it; the replay's model too
 * when this is the player's and the replay is showing. */
nfs::u32 frontend::rendd::freeCarModel(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    u32 left = car;
    const u32 player = guest.dword(kPlayerCars + 4 * gameSettings(guest).localPlayer);
    const u32 model = guest.dword(car + kCarModel);
    if (car == player && guest.dword(kReplayShowing) == 1)
    {
        const u32 replay = guest.dword(kReplayModel);
        if (replay != 0)
        {
            fe::direct<0x49d5a0, frontend::rendc::modelFree>(app, ts, replay);
            guest.setDword(kReplayModel, 0);
            left = 0;
        }
    }
    if (model != 0)
    {
        fe::direct<0x49d5a0, frontend::rendc::modelFree>(app, ts, model);
        for (left = 0; s32(left) < s32(guest.dword(kCarCount)); ++left)
        {
            const u32 other = guest.dword(kCarTable + 4 * left);
            if (model == guest.dword(other + kCarModel))
                guest.setDword(other + kCarModel, 0);
        }
    }
    return left;
}

nfs::u32 frontend::rendFreeCarModel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b7eb0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::freeCarModel(app, ts, car); });
}

// ---------------------------------------------------------------------------
// The shadow
// ---------------------------------------------------------------------------

/* sub_4b7f80: the four corners of car eax's shadow to edx, 12 bytes each, in the world: its
 * half size turned by its axes, each corner's height the road's under it (sub_4968a0), or
 * for a car in detail the track's own below it unless that is a surface with no shadow. */
nfs::u32 frontend::rendd::shadowCorners(App* app, ThreadState& ts, u32 car, u32 corners)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xc4);
    constexpr u32 kToFixed = 0x53ffb0;       // a double, 65536
    constexpr u32 kFromFixed = 0x53ffb8;     // a double, 1/65536
    const u32 record = guest.frame();        // what sub_420650 fills
    const u32 fixedPoint = record + 0x90;    // the corner it is given, as fixed point
    const u32 halfX = guest.dword(car + kCarHalfSize), halfY = guest.dword(car + kCarHalfSize + 4);
    const u32 halfZ = guest.dword(car + kCarHalfSize + 8);
    const u32 minusX = bitsOf(float(-double(floatFrom(halfX))));
    const u32 minusY = bitsOf(float(-double(floatFrom(halfY))));
    const u32 minusZ = bitsOf(float(-double(floatFrom(halfZ))));
    guest.setDword(corners, minusX);
    guest.setDword(corners + 4, minusY);
    guest.setDword(corners + 8, halfZ);
    guest.setDword(corners + 0xc, halfX);
    guest.setDword(corners + 0x10, minusY);
    guest.setDword(corners + 0x14, halfZ);
    guest.setDword(corners + 0x18, halfX);
    guest.setDword(corners + 0x1c, minusY);
    guest.setDword(corners + 0x20, minusZ);
    guest.setDword(corners + 0x24, minusX);
    guest.setDword(corners + 0x28, minusY);
    guest.setDword(corners + 0x2c, minusZ);
    fe::direct<0x4e0430, vertices::transformPointList>(app, ts, 4, corners, car + kCarAxes, car + kCarPlace, corners);
    u32 left = car;   // set again on every pass below
    for (u32 i = 0; i < 4; ++i)
    {
        const u32 corner = corners + 12 * i;
        const float height = fe::direct<0x4968a0, vertices::groundHeightAt>(
            app, ts, car + 0x124, guest.floatAt(car + 0x148), guest.floatAt(car + 0x14c), guest.floatAt(car + 0x150),
            guest.floatAt(corner), guest.floatAt(corner + 4), guest.floatAt(corner + 8));
        guest.setFloat(corner + 4, height);
        left = car;
        if (s32(guest.dword(car + kCarDetail)) >= 2)
            continue;
        const double toFixed = doubleOf(guest, kToFixed);
        u32 x = u32(fistp(fp::mul(floatOf(guest, corner), toFixed)));
        u32 y = u32(fistp(fp::mul(floatOf(guest, corner + 4), toFixed)));
        u32 z = u32(fistp(fp::mul(floatOf(guest, corner + 8), toFixed)));
        guest.setDword(fixedPoint, x);
        guest.setDword(fixedPoint + 4, y);
        guest.setDword(fixedPoint + 8, z);
        fe::direct<kTrackPointAt, trackdata::placeAt>(guest.app(), guest.ts(), fixedPoint, 1, guest.dword(car + kCarSegment), record);
        left = record;  // eax as it leaves it: the record it filled
        const u32 below = guest.dword(record + 0x14), surface = guest.dword(record + 0x18);
        if (below == 0 || surface == 0)
            continue;
        left = guest.byte(surface + 1);
        if ((left & 0x3f) == 0)
            continue;
        left = guest.byte(surface + 1);
        if ((left & 0x3f) == kSurfaceNoShadow)
        {
            left = 0;
            continue;
        }
        const u32 trackY = guest.dword(fixedPoint + 4);
        if (s32(y + 0x4000) > s32(trackY) && s32(trackY) > s32(y))
        {
            x = guest.dword(fixedPoint);
            y = trackY;
            z = guest.dword(fixedPoint + 8);
        }
        const double fromFixed = doubleOf(guest, kFromFixed);
        const double worldX = fp::mul(double(s32(x)), fromFixed);
        const double worldY = fp::mul(double(s32(y)), fromFixed);
        const double worldZ = fp::mul(fromFixed, double(s32(z)));
        guest.setFloat(corner, float(worldX));
        guest.setFloat(corner + 4, float(worldY));
        guest.setFloat(corner + 8, float(worldZ));
        left = z;
    }
    return left;
}

nfs::u32 frontend::rendShadowCorners(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b7f80", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 car = args.eax, corners = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::shadowCorners(app, ts, car, corners); });
}

/* sub_4b81a0: car eax's shadow in view edx, a quad of the arena on the view's list: its
 * corners (sub_4b7f80), when the sun shows moved along the car's edges as far as the height
 * asks, raised a little, projected, and dropped when off the screen or facing away. */
nfs::u32 frontend::rendd::carShadow(App* app, ThreadState& ts, u32 car, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xf4);
    constexpr u32 kCornerPairs = 0x4b7a30;   // in the code: each corner's two neighbours
    constexpr u32 kRaise = 0x53ffc0;
    constexpr u32 kShadowLength = 0x118;     // of a car
    constexpr u32 kCarLean = 0xd0;
    const u32 frame = guest.frame();
    const u32 corners = frame;               // as they stand
    const u32 moved = frame + 0x30;          // as the sun moves them
    const u32 edge = frame + 0x80;           // also the point in the view
    const u32 offset = frame + 0x8c;
    const u32 from = frame + 0x98;
    const u32 to = frame + 0xa4;
    const u32 list = frame + 0xb0;
    const u32 slope = frame + 0xb8;          // what sub_4e0210 hands back
    const u32 vertexSlot = frame + 0xbc;
    const u32 quadSlot = frame + 0xc0;
    guest.setDword(quadSlot, 0);
    if (guest.dword(car + 0x184) != 0 || guest.dword(car + kCarSpeedLow) == 0)
        return car;
    const bool sunShows = gameSettings(guest).night == 0 && gameSettings(guest).weather == 0 && guest.dword(view) != 1;
    guest::direct<0x4b7f80, shadowCorners>(app, ts, car, corners);
    for (u32 i = 0; i < 4; ++i)
    {
        const u32 corner = corners + 12 * i, copy = moved + 12 * i;
        const double x = floatOf(guest, corner), y = floatOf(guest, corner + 4), z = floatOf(guest, corner + 8);
        guest.setFloat(copy, float(x));
        guest.setFloat(copy + 4, float(y));
        guest.setFloat(copy + 8, float(z));
    }
    if (sunShows)
    {
        for (u32 i = 0; i < 4; ++i)
        {
            copyDwords(guest, from, corners + 12 * i, 3);
            for (u32 side = 0; side < 2; ++side)
            {
                const u32 neighbour = guest.dword(kCornerPairs + 8 * i + 4 * side);
                copyDwords(guest, to, corners + 12 * neighbour, 3);
                fe::direct<0x4e0000, vertices::offsetVectorList<false>>(app, ts, 1, from, to, edge);
                // The game's code hands it the sun in ebx and the slope in ecx too, which the next takes.
                fe::direct<0x4e01c0, vertices::makeUnitVector>(app, ts, edge, edge);
                fe::direct<0x4e0210, vertices::dotProductsOf>(app, ts, 1, edge, kSun, slope);
                const float along = guest.floatAt(slope);
                const double length = isAbove(fp::compare(0.0, double(along))) ? floatOf(guest, car + kShadowLength) : 0.0;
                const double by = fp::mul(-double(along), length);
                fe::direct<0x4e0050, vertices::scaleVectorsBy>(app, ts, 1, edge, float(by), offset);
                fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, ts, 1, moved + 12 * i, offset, moved + 12 * i);
            }
        }
    }
    const double raise = floatOf(guest, kRaise);
    for (u32 i = 0; i < 4; ++i)
    {
        const u32 y = moved + 12 * i + 4;
        guest.setFloat(y, float(fp::add(floatOf(guest, y), raise)));
    }
    if (guest::direct<0x4bbde0, arenaTake>(app, ts, vertexSlot, 0x80u) == 0)
        return 0;
    if (guest::direct<0x4bbde0, arenaTake>(app, ts, quadSlot, 0x84u) == 0)
        return 0;
    const u32 vertices = guest.dword(vertexSlot);
    const u32 axes = view + 0x44;
    const u32 lean = isAbove(fp::compare(0.0, floatOf(guest, car + kCarLean))) ? 1 : 0;
    for (u32 i = 0; i < 4; ++i)
    {
        const u32 corner = moved + 12 * i;
        const double x = floatOf(guest, corner), y = floatOf(guest, corner + 4), z = floatOf(guest, corner + 8);
        double row0 = fp::mul(x, floatOf(guest, axes));
        double row1 = fp::mul(x, floatOf(guest, axes + 4));
        double row2 = fp::mul(x, floatOf(guest, axes + 8));
        const double y0 = fp::mul(y, floatOf(guest, axes + 0xc));
        const double y1 = fp::mul(y, floatOf(guest, axes + 0x10));
        const double y2 = fp::mul(y, floatOf(guest, axes + 0x14));
        row0 = fp::add(row0, y0);
        row1 = fp::add(row1, y1);
        row2 = fp::add(row2, y2);
        const double z0 = fp::mul(z, floatOf(guest, axes + 0x18));
        const double z1 = fp::mul(z, floatOf(guest, axes + 0x1c));
        const double z2 = fp::mul(z, floatOf(guest, axes + 0x20));
        row0 = fp::add(row0, z0);
        row1 = fp::add(row1, z1);
        row2 = fp::add(row2, z2);
        guest.setFloat(edge, float(row0));
        guest.setFloat(edge + 4, float(row1));
        guest.setFloat(edge + 8, float(row2));
        fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, ts, 1, edge, view + 0x38, edge);
        if ((guest.dword(edge + 8) & 0x7fffffff) == 0)
            guest.setDword(edge + 8, kSmallestDepth);
        const float inverse = float(fp::div(1.0, floatOf(guest, edge + 8)));
        const double scaleX = fp::mul(floatOf(guest, kScaleX), double(inverse));
        const double scaleY = fp::mul(floatOf(guest, kScaleY), double(inverse));
        const double screenY = fp::mul(scaleY, floatOf(guest, edge + 4));
        const double screenX = fp::mul(scaleX, floatOf(guest, edge));
        const u32 vertex = vertices + (i << 5);
        guest.setDword(vertex + 0xc, bitsOf(inverse));
        guest.setFloat(vertex, float(fp::add(screenX, floatOf(guest, kCentreX))));
        guest.setDword(vertex + 8, guest.dword(edge + 8));
        guest.setFloat(vertex + 4, float(fp::add(screenY, floatOf(guest, kCentreY))));
        const std::uint8_t code = clipCode(clipBounds(guest), s32(bitsOf(inverse)), s32(guest.dword(vertex + 4)), s32(guest.dword(vertex)));
        guest.setByte(vertex + 0x14, code);
    }
    const std::uint8_t codes0 = guest.byte(vertices + 0x14), codes1 = guest.byte(vertices + 0x34), codes2 = guest.byte(vertices + 0x54);
    const std::uint8_t codes3 = guest.byte(vertices + 0x74);
    const u32 someCodes = u32(codes0) | codes1 | codes2;
    if (((someCodes | codes3) & 0x10) == 0)
    {
        const double v0x = floatOf(guest, vertices), v0y = floatOf(guest, vertices + 4);
        const double v1x = floatOf(guest, vertices + 0x20), v1y = floatOf(guest, vertices + 0x24);
        const double v2x = floatOf(guest, vertices + 0x40), v2y = floatOf(guest, vertices + 0x44);
        const double v3x = floatOf(guest, vertices + 0x60), v3y = floatOf(guest, vertices + 0x64);
        const u32 flip = guest.dword(kBackFaceFlip);
        const double a1 = fp::mul(fp::sub(v2x, v1x), fp::sub(v0y, v1y));
        const double b1 = fp::mul(fp::sub(v2y, v1y), fp::sub(v0x, v1x));
        u32 faces = 0;
        if (((isAbove(fp::compare(b1, a1)) ? 1u : 0u) ^ flip) != 0)
        {
            const double a2 = fp::mul(fp::sub(v3x, v2x), fp::sub(v0y, v2y));
            const double b2 = fp::mul(fp::sub(v3y, v2y), fp::sub(v0x, v2x));
            if (((isAbove(fp::compare(b2, a2)) ? 1u : 0u) ^ flip) != 0)
                faces = 1;
        }
        if ((faces ^ lean) != 0)
            return faces ^ lean;
    }
    const u32 quad = guest.dword(quadSlot);
    guest.setWord(quad + 4, 0);
    guest.setDword(quad, 0);
    guest.setWord(quad + 6, 0);
    guest.setDword(quad + 8, vertices);
    guest.setDword(quad + 0xc, vertices + 0x20);
    guest.setDword(quad + 0x10, vertices + 0x40);
    guest.setDword(quad + 0x14, vertices + 0x60);
    guest.setDword(quad + 0x18, guest.dword(kShadowTexture));
    guest.setDword(vertices + 0x70, 0x6fffffff);
    guest.setDword(vertices + 0x50, 0x6fffffff);
    guest.setDword(vertices + 0x30, 0x6fffffff);
    guest.setDword(vertices + 0x10, 0x6fffffff);
    guest.setDword(quad + 0x1c, 0);
    guest::direct<0x4bbe90, listOfOne>(app, ts, quad, list);
    const double depth = fp::sub(floatOf(guest, car + kCarDepth), floatOf(guest, kDepthBias));
    // The list in edx, as the call before leaves it.
    return guest::direct<0x4bc0b0, drawListMirrored>(app, ts, view, list, bitsOf(float(depth)));
}

nfs::u32 frontend::rendCarShadow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b81a0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 car = args.eax, view = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::carShadow(app, ts, car, view); });
}

// ---------------------------------------------------------------------------
// A car's lights and its parts' turns
// ---------------------------------------------------------------------------

/* sub_4b8740: the flares of car edx's lights in view eax, its body at matrix ebx and place
 * ecx, the stack a size and its colour: each of the model's lights, by the letters of its
 * name, a tail light by its brake, a head light by the night, a cop's by the flash, the
 * flares through sub_492370 and the road's light they throw on a list drawn at the car's depth.
 *
 * What the game's code leaves in its frame from before, the size and colour a light of
 * an unknown kind finds, is read from where it would be. */
nfs::u32 frontend::rendd::carLights(App* app, ThreadState& ts, u32 view, u32 car, u32 matrix, u32 place, u32 flareSize, u32 paint)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x128);
    constexpr u32 kPaintWhole = 0x53ffc8;      // a double, 1/765
    constexpr u32 kPaintPart = 0x53ffd0;       // a double, 0.3
    constexpr u32 kDimmest = 0x53ffd8;         // a float, 0.7
    constexpr u32 kRingAfter = 0x53ffe0;       // a double, 30: time past which the lights' ring shows
    constexpr u32 kRingOffset = 0x53ffe8;      // a float, -30
    constexpr u32 kRingScale = 0x53ffec;       // a float, 1/30
    constexpr u32 kHalf = 0x53fff0;            // a double
    constexpr u32 kFullAlpha = 0x540000;       // a float, 228
    constexpr u32 kSpinScale = 0x53fff8;       // a double, 1/2048
    constexpr u32 kFlashScale = 0x540004;      // a float, 2
    constexpr u32 kTwo = 0x540008;             // a double
    constexpr u32 kQuarterDouble = 0x540010;   // a double
    constexpr u32 kFour = 0x540018;            // a double
    constexpr u32 kQuarter = 0x540020;         // a float
    constexpr u32 kSizeScale = 0x540028;       // a double, 0.8
    constexpr u32 kByteScale = 0x540030;       // a float, 255
    constexpr u32 kOneAndAHalf = 0x540038;     // a double
    constexpr u32 kByteScaleDouble = 0x540040; // a double, 255
    constexpr u32 kHaloDim = 0xa666;           // of 0x10000
    const u32 frame = guest.frame();
    const u32 world = frame;                   // the light in the world
    const u32 toViewer = frame + 0xc;
    const u32 lights = frame + 0x30;           // the road's lights, a list
    const u32 streak = frame + 0x68;           // a list of one
    const u32 halo = frame + 0x90;             // likewise

    float facing = guest.floatAt(frame + 0xfc);
    u32 colour = guest.dword(frame + 0x100);
    u32 size = guest.dword(frame + 0x104);
    float flash = guest.floatAt(frame + 0x10c);

    guest::direct<0x4bbe70, emptyList>(app, ts, lights);
    const float time = float(double(s32(fe::direct<0x470a60, frontend::renda::fogAmount>(app, ts))));
    const u32 model = guest.dword(car + kCarModel);
    const std::uint8_t flags = guest.byte(car + kCarFlags);
    if (flags & kFlagHidden)
    {
        return model;
    }

    // How dark the paint is: the lights show less on a pale car.
    const u32 paintSum = ((paint >> 16) & 0xff) + ((paint >> 8) & 0xff) + (paint & 0xff);
    const double paintWhole = fp::mul(double(s32(paintSum)), doubleOf(guest, kPaintWhole));
    const double paintPart = fp::mul(paintWhole, doubleOf(guest, kPaintPart));
    const double shadeValue = fp::sub(1.0, paintPart);
    const float shade = float(shadeValue);
    double bright;
    if (isBelow(fp::compare(shadeValue, floatOf(guest, kDimmest))))
        bright = doubleFrom(0x66666666, 0x3fe66666);
    else
    {
        const double whole = double(shade);
        bright = isBelow(fp::compare(1.0, whole)) ? 1.0 : whole;
    }
    const double glow = double(float(bright));

    // Long enough into the race, the lights' ring turns with the car's speed.
    u32 ringColour = 0;
    u32 ringAngle = 0;
    if (isAbove(fp::compare(double(time), doubleOf(guest, kRingAfter))))
    {
        u32 frames = guest.dword(kFrameCount) - guest.dword(car + 0x948);
        const double speedDot = fe::direct<0x4e01f0, vertices::dotProduct>(app, ts, car + kCarVelocityUnit, car + kCarForward);
        const double past = fp::add(double(time), floatOf(guest, kRingOffset));
        const double spinWhole = fp::add(speedDot, 1.0);
        const double fadeIn = fp::mul(past, floatOf(guest, kRingScale));
        const s32 spin = fistp(fe::direct<0x4dfd56, watcom::clib::chop>(app, ts, spinWhole));
        const float faded = float(fadeIn);
        float ring = 0.0f;
        if (s32(bitsOf(faded)) >= 0x3f800000 || isBelow(fp::compare(0.0, double(faded))))
        {
            const float again = float(fp::mul(fp::add(double(time), floatOf(guest, kRingOffset)), floatOf(guest, kRingScale)));
            ring = s32(bitsOf(again)) >= 0x3f800000 ? floatFrom(0x3f800000) : again;
        }
        float alpha = ring;
        if (guest.dword(car + kCarLightsOn) == 1)
            alpha = float(fp::mul(double(ring), doubleOf(guest, kHalf)));
        if (s32(frames) < 0)
            frames = 0;
        frames *= u32(spin);
        const double alphaByte = fp::mul(double(alpha), floatOf(guest, kFullAlpha));
        const u32 turned = guest.dword(car + 0x944) + frames;
        guest.setDword(car + 0x944, turned);
        guest.setDword(car + 0x944, turned & 0x7ff);
        const double angle = fp::mul(double(s32(guest.dword(car + 0x944))), doubleOf(guest, kSpinScale));
        const float alphaStored = float(alphaByte);
        guest.setDword(car + 0x948, guest.dword(kFrameCount));
        ringAngle = bitsOf(float(angle));
        ringColour = ((clampedByte(double(alphaStored), alphaStored) & 0xff) << 24) | 0xffffff;
    }

    // A cop's flash, and which side of it is lit.
    std::uint8_t flashSide = 0;
    if ((guest.byte(car + kCarFlags) & kFlagCop) && guest.dword(car + kCarHasCop) != 0)
    {
        double through = 0.0;
        callAt(guest, kGunFire, args({ car }), 0, &through);
        flash = float(through);
        flashSide = isAbove(fp::compare(through, doubleOf(guest, kHalf))) ? 0x45 : 0x4f;
        const double doubled = fp::mul(double(flash), floatOf(guest, kFlashScale));
        flash = float(doubled);
        if (isBelow(fp::compare(1.0, doubled)))
            flash = float(fp::sub(doubleOf(guest, kTwo), doubled));
    }

    for (u32 i = 0;; ++i)
    {
        if (s32(i) >= s32(guest.dword(guest.dword(car + kCarModel) + kModelLightCount)))
            break;
        // By address: what it leaves in ebx goes on to the end, which does not restore ebx.
        fe::direct<0x4e0430, vertices::transformPointList>(app, ts, 1, model + kModelLights + 12 * i, matrix, place, world);
        const u32 name = model + (i << 6);
        const u32 x = guest.dword(world), y = guest.dword(world + 4), z = guest.dword(world + 8);

        // How squarely the light faces the viewer.
        const auto facingViewer = [&]() {
            guest.setFloat(toViewer, float(fp::sub(floatOf(guest, view + 8), double(floatFrom(x)))));
            guest.setFloat(toViewer + 4, float(fp::sub(floatOf(guest, view + 0xc), double(floatFrom(y)))));
            guest.setFloat(toViewer + 8, float(fp::sub(floatOf(guest, view + 0x10), double(floatFrom(z)))));
            fe::direct<0x4972f0, vertices::normaliseVector>(app, ts, toViewer);
            // The game's code hands it the vector in edx too, which the product takes as its second.
            return fe::direct<0x4e01f0, vertices::dotProduct>(app, ts, car + kCarForward, toViewer);
        };

        const std::uint8_t side = guest.byte(name + kModelLightNames + 1);
        if (side == 'R')
        {
            const double behind = -facingViewer();
            facing = float(behind);
            if (isBelow(fp::compare(0.0, behind)))
            {
                double strength;
                if (isAbove(fp::compare(behind, doubleOf(guest, kQuarterDouble))))
                {
                    strength = 1.0;
                }
                else
                    strength = fp::mul(behind, doubleOf(guest, kFour));
                const float strengthStored = float(strength);
                const float least = isAbove(fp::compare(floatOf(guest, kQuarter), double(facing))) ? floatFrom(0x3e800000) : facing;
                size = bitsOf(float(fp::mul(fp::mul(double(least), doubleOf(guest, kSizeScale)), glow)));
                const float brake = guest.byte(car + kCarBraking) != 0 ? floatFrom(0x43460000) : floatFrom(0x43000000);
                const double alpha = fp::mul(fp::mul(double(brake), double(strengthStored)), glow);
                colour = ((clampedByte(alpha, float(alpha)) & 0xff) << 24) | 0xff0000;
            }
        }
        else if (side == 'F')
        {
            const double ahead = facingViewer();
            facing = float(ahead);
            if (isBelow(fp::compare(0.0, ahead)))
            {
                double strength;
                if (isAbove(fp::compare(ahead, doubleOf(guest, kQuarterDouble))))
                {
                    strength = 1.0;
                }
                else
                    strength = fp::mul(ahead, doubleOf(guest, kFour));
                const float strengthStored = float(strength);
                double wide = 1.5;
                if (guest.dword(car + kCarLightsOn) != 2)
                {
                    wide = 1.0;
                }
                const double sized = fp::mul(double(facing), doubleOf(guest, kSizeScale));
                const double alphaWhole = fp::mul(double(strengthStored), floatOf(guest, kByteScale));
                const double widened = fp::mul(sized, wide);
                const double alpha = fp::mul(alphaWhole, glow);
                const double sizeValue = fp::mul(widened, glow);
                const float alphaStored = float(alpha);
                size = bitsOf(float(sizeValue));
                colour = ((clampedByte(alpha, alphaStored) & 0xff) << 24) | 0xffffff;
            }
        }

        const std::uint8_t kind = guest.byte(name + kModelLightNames);
        if (kind == 'T')
        {
            if ((guest.dword(car + kCarLightsOn) != 0 || guest.byte(car + kCarBraking) != 0)
                && isBelow(fp::compare(0.0, double(facing))))
            {
                callAt(guest, kFlare, args({ view }, { x, y, z, colour, size, flareSize, 0 }));
                if (guest.byte(name + kModelLightNames + 4) != 'N')
                {
                    const u32 dimmed = scaledColour(ts, colour, kHaloDim);
                    const u32 record = callAt(guest, kLightFlare, args({ view }, { x, y, z, dimmed, guest.dword(car + kCarSegment), size, car + kCarAxes, 0 }));
                    if (record != 0)
                    {
                        guest::direct<0x4bbe90, listOfOne>(app, ts, record, halo);
                        const u32 head = guest.dword(halo), tail = guest.dword(halo + 4);
                        guest::direct<0x4bbec0, joinLists>(app, ts, lights, guest.dword(lights), guest.dword(lights + 4), head,
                                                           tail);
                    }
                }
            }
        }
        if (kind == 'H' && (guest.dword(car + kCarLightsOn) != 0 || flashSide == guest.byte(name + kModelLightNames + 3)))
        {
            if (isBelow(fp::compare(0.0, double(facing))))
            {
                callAt(guest, kFlare, args({ view }, { x, y, z, colour, size, flareSize, 0 }));
                if (guest.byte(name + kModelLightNames + 4) != 'N')
                {
                    const u32 dimmed = scaledColour(ts, colour, kHaloDim);
                    const u32 record = callAt(guest, kLightFlare, args({ view }, { x, y, z, dimmed, guest.dword(car + kCarSegment), size, car + kCarAxes, 1 }));
                    if (record != 0)
                    {
                        guest::direct<0x4bbe90, listOfOne>(app, ts, record, streak);
                        const u32 head = guest.dword(streak), tail = guest.dword(streak + 4);
                        guest::direct<0x4bbec0, joinLists>(app, ts, lights, guest.dword(lights), guest.dword(lights + 4), head,
                                                           tail);
                    }
                }
            }
            if (isAbove(fp::compare(double(time), doubleOf(guest, kRingAfter))))
            {
                callAt(guest, kRingFlare, args({ view }, { x, y, z, car + kCarAxes, ringColour, 0x40c00000, 0x3dcccccd, 0x3f400000, ringAngle, 0xbecccccd }));
            }
        }
        if (kind == 'S' && (guest.byte(car + kCarFlags) & kFlagCop) && guest.dword(car + kCarHasCop) != 0)
        {
            u32 alphaByte, rgb;
            if (guest.byte(name + kModelLightNames + 2) == 'L')
            {
                const float least = isAbove(fp::compare(floatOf(guest, kQuarter), double(flash))) ? floatFrom(0x3e800000) : flash;
                const double alphaWhole = fp::mul(double(flash), floatOf(guest, kByteScale));
                const double sized = fp::mul(double(least), doubleOf(guest, kOneAndAHalf));
                const double alpha = fp::mul(alphaWhole, glow);
                const double sizeValue = fp::mul(sized, glow);
                const float alphaStored = float(alpha);
                size = bitsOf(float(sizeValue));
                alphaByte = clampedByte(alpha, alphaStored);
                rgb = guest.dword(car + 0x7a4);
            }
            else
            {
                const double other = fp::sub(1.0, double(flash));
                const double least = isAbove(fp::compare(doubleOf(guest, kQuarterDouble), other)) ? 0.25 : other;
                const double rest = fp::sub(1.0, double(flash));
                const double sized = fp::mul(least, doubleOf(guest, kOneAndAHalf));
                const double alphaWhole = fp::mul(rest, doubleOf(guest, kByteScaleDouble));
                const double sizeValue = fp::mul(sized, glow);
                const double alpha = fp::mul(glow, alphaWhole);
                size = bitsOf(float(sizeValue));
                alphaByte = clampedByte(alpha, float(alpha));
                rgb = guest.dword(car + 0x7a8);
            }
            colour = (alphaByte << 24) | (rgb & 0xffffff);
            callAt(guest, kFlare, args({ view }, { x, y, z, colour, size, 0, 0xbf400000 }));
        }
    }

    const u32 head = guest.dword(lights), tail = guest.dword(lights + 4);
    guest::direct<0x4c7380, rende::polygonsSorted>(app, ts, lights, head, tail);
    const double depth = fp::sub(floatOf(guest, car + kCarDepth), floatOf(guest, kDepthBias));
    // The lights in edx, as the call before leaves it.
    return guest::direct<0x4bc0b0, drawListMirrored>(app, ts, view, lights, bitsOf(float(depth)));
}

nfs::u32 frontend::rendCarLights(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b8740", 8, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 flareSize = app->getMemory<u32>(ts.sp + 4);
    const u32 paint = app->getMemory<u32>(ts.sp + 8);
    const u32 view = args.eax, car = args.edx, matrix = args.ebx, place = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::carLights(app, ts, view, car, matrix, place, flareSize, paint); });
}

/* sub_4b90e0: how much of a shake car eax's body has this frame, at random around 0 by its
 * roughness; edx which way, along or across.  Once a frame (nfs3hp::carWheelStamp's). */
nfs::u32 frontend::rendd::bodyShake(App* app, ThreadState& ts, u32 car, u32 across)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xc);
    constexpr u32 kHalfFloat = 0x540048;
    if (carWheelStamp(app, car, across ? 3u : 2u) == guest.dword(kFrameCount))
        return car;
    // eax (the car) as the code leaves it: the random fraction's call keeps it.
    const double fraction = double(*fe::direct<0x4eb230, ea::randomFractionOf>(app, ts));
    const u32 roughness = car + 0x930;
    const double random = fp::mul(fraction, floatOf(guest, roughness));
    const double centre = fp::mul(floatOf(guest, roughness), floatOf(guest, kHalfFloat));
    guest.setFloat(car + (across ? 0x940 : 0x93c), float(fp::sub(random, centre)));
    return car;
}

nfs::u32 frontend::rendBodyShake(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b90e0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const u32 car = args.eax, across = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::bodyShake(app, ts, car, across); });
}

namespace
{

/* What a wheel turns this frame, -0.5 to 0.5: `steer` cut at both ends. */
inline float cutToHalf(const Guest& guest, double steer, u32 lowest, u32 highest)
{
    const float stored = float(steer);
    if (isBelow(fp::compare(steer, doubleOf(guest, lowest))))
        return floatFrom(0xbf000000);
    if (isAbove(fp::compare(double(stored), floatOf(guest, highest))))
        return floatFrom(0x3f000000);
    return stored;
}

/* A turn kept between its two ends: the end's own number added from above, 1 from below,
 * until it is. */
inline void wrapTurn(Guest& guest, u32 turn, u32 lowest)
{
    for (;;)
    {
        const double value = floatOf(guest, turn);
        if (!isBelow(fp::compare(1.0, value)))
            break;
        guest.setFloat(turn, float(fp::add(value, doubleOf(guest, lowest))));
    }
    for (;;)
    {
        const double value = floatOf(guest, turn);
        if (!isBelow(fp::compare(value, doubleOf(guest, lowest))))
            break;
        guest.setFloat(turn, float(fp::add(1.0, value)));
    }
}

}

/* sub_4b9140: a front wheel's matrix of car eax to edx, turned by the car's speed this frame
 * and steered when ebx asks. */
nfs::u32 frontend::rendd::frontWheelTurn(App* app, ThreadState& ts, u32 car, u32 out, u32 steered)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x80);
    constexpr u32 kSpeedScale = 0x54004c;    // a float
    constexpr u32 kSpinScale = 0x540050;     // a double
    constexpr u32 kLowest = 0x540058;        // a double, -0.5
    constexpr u32 kHighest = 0x540060;       // a float, 0.5
    constexpr u32 kPerFrame = 0x540068;      // a double
    constexpr u32 kWrap = 0x540070;          // a double, -1
    constexpr u32 kSteerScale = 0x540078;    // a double
    const u32 rotation = guest.frame();
    const u32 steering = rotation + 0x24;
    const u32 turn = car + 0x938;
    const double speed = fp::mul(floatOf(guest, car + 0x4f4), floatOf(guest, kSpeedScale));
    const u32 stamp = carWheelStamp(app, car, 0u);
    const double spin = fp::mul(double(s32(guest.dword(car + 0x598))), doubleOf(guest, kSpinScale));
    const u32 frames = guest.dword(kFrameCount) - stamp;
    const float cut = cutToHalf(guest, fp::add(speed, spin), kLowest, kHighest);
    if ((bitsOf(cut) & 0x7fffffff) != 0 && s32(frames) > 0 && (guest.byte(car + 0x59c) & 1) == 0)
    {
        const double by = fp::mul(fp::mul(double(s32(frames)), doubleOf(guest, kPerFrame)), double(cut));
        guest.setFloat(turn, float(fp::add(by, floatOf(guest, turn))));
    }
    wrapTurn(guest, turn, kWrap);
    const u32 rotated = *fe::direct<0x4dfd80, ea::rotateAboutX>(app, ts, rotation, turn);
    if (steered == 0)
    {
        copyDwords(guest, out, rotation, 9);
        return rotated;
    }
    const u32 steer = guest.dword(car + 0x51c);
    if (steer == 0)
    {
        copyDwords(guest, out, rotation, 9);
        return 0;
    }
    const double angle = fp::mul(double(s32(steer * 36)), doubleOf(guest, kSteerScale));
    callAt(guest, kRotationY, args({ steering, steering }, { bitsOf(float(angle)) }));
    // The product's other factor in edx and where it goes in ebx, as the rotation before leaves and sets them.
    return callAt(guest, kMultiplyMatrices, args({ rotation, steering, out }));
}

nfs::u32 frontend::rendFrontWheelTurn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b9140", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 car = args.eax, out = args.edx, steered = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::frontWheelTurn(app, ts, car, out, steered); });
}

/* sub_4b9280: a rear wheel's matrix of car eax to edx: only turned, and less when it is
 * holding on (a car's +0x50d). */
nfs::u32 frontend::rendd::rearWheelTurn(App* app, ThreadState& ts, u32 car, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x40);
    constexpr u32 kSpeedScale = 0x540080;    // a float
    constexpr u32 kSpinScale = 0x540088;     // a double
    constexpr u32 kLowest = 0x540090;        // a double, -0.5
    constexpr u32 kHighest = 0x540098;       // a float, 0.5
    constexpr u32 kHoldLowest = 0x5400a0;    // a double, -0.05
    constexpr u32 kHoldHighest = 0x5400a8;   // a float, 0.05
    constexpr u32 kPerFrame = 0x5400b0;      // a double
    constexpr u32 kWrap = 0x5400b8;          // a double, -1
    const u32 turn = car + 0x934;
    const double speed = fp::mul(floatOf(guest, car + 0x4f4), floatOf(guest, kSpeedScale));
    const u32 stamp = carWheelStamp(app, car, 1u);
    const double spin = fp::mul(double(s32(guest.dword(car + 0x594))), doubleOf(guest, kSpinScale));
    const u32 frames = guest.dword(kFrameCount) - stamp;
    const float cut = cutToHalf(guest, fp::add(speed, spin), kLowest, kHighest);
    float by = cut;
    if (guest.byte(car + 0x50d) != 0)
    {
        if (isBelow(fp::compare(double(cut), doubleOf(guest, kHoldLowest))))
            by = floatFrom(0xbd4ccccd);
        else if (isAbove(fp::compare(double(cut), floatOf(guest, kHoldHighest))))
            by = floatFrom(0x3d4ccccd);
    }
    if ((bitsOf(by) & 0x7fffffff) != 0 && s32(frames) > 0 && (guest.byte(car + 0x59c) & 2) == 0)
    {
        const double step = fp::mul(fp::mul(double(s32(frames)), doubleOf(guest, kPerFrame)), double(by));
        guest.setFloat(turn, float(fp::add(step, floatOf(guest, turn))));
    }
    wrapTurn(guest, turn, kWrap);
    return *fe::direct<0x4dfd80, ea::rotateAboutX>(app, ts, out, turn);
}

nfs::u32 frontend::rendRearWheelTurn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b9280", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const u32 car = args.eax, out = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::rearWheelTurn(app, ts, car, out); });
}

/* sub_4b93c0: the body of car eax leaning, its matrix to edx: its two leans (+0x920, +0x924)
 * cut to their ends; edx turned by 0x564498 too, and ebx given the turn before, when the car
 * is rolling over (+0x10c). */
nfs::u32 frontend::rendd::bodyLean(App* app, ThreadState& ts, u32 car, u32 out, u32 before)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x98);
    constexpr u32 kAirborne = 0x5400c0;      // a double
    constexpr u32 kPitchHigh = 0x5400c8;     // a double, 0.25
    constexpr u32 kPitchLow = 0x5400d0;      // a double, -0.25
    constexpr u32 kRollHigh = 0x5400d8;      // a double, 0.3
    constexpr u32 kRollLow = 0x5400e0;       // a double, -0.3
    constexpr u32 kRollOver = 0x5400e8;      // a double
    constexpr u32 kRollMatrix = 0x564498;
    const u32 yawed = guest.frame();
    const u32 pitched = yawed + 0x24;
    const u32 pitch = car + 0x920, roll = car + 0x924;
    if ((guest.byte(car + kCarFlags) & 1) && guest.dword(car + 0x2c4) == 3
        && isBelow(fp::compare(floatOf(guest, car + 0xb8), doubleOf(guest, kAirborne))))
        guest.setDword(pitch, 0);
    const double pitchNow = floatOf(guest, pitch);
    double pitchCut;
    if (isBelow(fp::compare(pitchNow, doubleOf(guest, kPitchHigh))) && !isAbove(fp::compare(pitchNow, doubleOf(guest, kPitchLow))))
        pitchCut = -0.25;
    else
    {
        const double again = floatOf(guest, pitch);
        pitchCut = isBelow(fp::compare(again, doubleOf(guest, kPitchHigh))) ? again : 0.25;
    }
    const double rollNow = floatOf(guest, roll);
    guest.setFloat(pitch, float(pitchCut));
    double rollCut;
    if (isBelow(fp::compare(rollNow, doubleOf(guest, kRollHigh))) && !isAbove(fp::compare(rollNow, doubleOf(guest, kRollLow))))
        rollCut = doubleFrom(0x33333333, 0xbfd33333);
    else
    {
        const double again = floatOf(guest, roll);
        rollCut = isBelow(fp::compare(again, doubleOf(guest, kRollHigh))) ? again : doubleFrom(0x33333333, 0x3fd33333);
    }
    guest.setFloat(roll, float(rollCut));
    callAt(guest, kRotationZ, args({ yawed, pitched }, { guest.dword(roll) }));
    fe::direct<0x4dfd80, ea::rotateAboutX>(app, ts, pitched, pitch);
    const u32 multiplied = callAt(guest, kMultiplyMatrices, args({ yawed, pitched, out }));
    const nfs::u16 fsw = fp::compare(floatOf(guest, car + 0x10c), doubleOf(guest, kRollOver));
    if (!isBelow(fsw))
        return (multiplied & 0xffff0000) | fsw;
    const u32 turned = callAt(guest, kMultiplyMatrices, args({ out, before, before }));
    copyDwords(guest, out, kRollMatrix, 9);
    return turned;
}

nfs::u32 frontend::rendBodyLean(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b93c0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 car = args.eax, out = args.edx, before = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::bodyLean(app, ts, car, out, before); });
}

/* sub_4b9550: colour edx lit by the track's lights around car eax: what each light within
 * reach of the seven segments around it adds, its average on top of the colour's channels. */
nfs::u32 frontend::rendd::trackLitColour(App* app, ThreadState& ts, u32 car, u32 colour)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x4c);
    constexpr u32 kToFixed = 0x5400f0;       // a double
    constexpr s32 kLightReach = 0x100000;
    const u32 fixedPlace = guest.frame();
    const double toFixed = doubleOf(guest, kToFixed);
    guest.setDword(fixedPlace, u32(fistp(fp::mul(floatOf(guest, car + kCarPlace), toFixed))));
    guest.setDword(fixedPlace + 4, u32(fistp(fp::mul(floatOf(guest, car + kCarPlace + 4), toFixed))));
    guest.setDword(fixedPlace + 8, u32(fistp(fp::mul(floatOf(guest, car + kCarPlace + 8), toFixed))));
    u32 sumRed = 0, sumGreen = 0, sumBlue = 0, seen = 0;
    for (s32 near_ = -3; near_ < 4; ++near_)
    {
        u32 segment = guest.dword(car + kCarSegment) + u32(near_);
        if (near_ < 0)
        {
            if (s32(segment) < 0)
                segment += guest.dword(kSegmentCount);
        }
        else if (s32(segment) >= s32(guest.dword(kSegmentCount)))
            segment -= guest.dword(kSegmentCount);
        const u32 block = kSegmentLights + u32(s32(segment) >> 3) * 0x5c0;
        const u32 count = guest.dword(block + 0x5b8);
        if (count == 0)
            continue;
        u32 light = guest.dword(block + 0x5bc);
        for (u32 left = count; left-- != 0; light += 0x10)
        {
            const u32 lit = fe::direct<0x491120, frontend::rendc::blinkColour>(app, ts, u32(guest.word(light + 0xc)));
            if (lit == 0)
                continue;
            // The light's place, in edx since before the call above.
            const s32 away =
                s32(fe::direct<0x496550, frontend::rendc::approximateDistance>(app, ts, fixedPlace, light));
            if (away >= kLightReach)
                continue;
            const u32 weight = 0x10000 - u32(away >> 4);
            const u32 red = u32(s32(weight * u32((s32(lit) >> 16) & 0xff)) >> 16);
            const u32 green = u32(s32(weight * u32((s32(lit) >> 8) & 0xff)) >> 16);
            const u32 blue = u32(s32((lit & 0xff) * weight) >> 16);
            sumRed += red;
            sumBlue += blue;
            sumGreen += green;
            ++seen;
        }
    }
    if (seen == 0)
        return colour;
    const auto channel = [](u32 from, u32 sum, u32 count) -> u32 {
        const s32 value = s32((from & 0xff) + u32(s32(sum) / s32(count)));
        return value < 0 ? 0u : value > 0xff ? 0xffu : u32(value);
    };
    const u32 red = channel(u32(s32(colour) >> 16), sumRed, seen);
    const u32 green = channel(u32(s32(colour) >> 8), sumGreen, seen);
    const u32 blue = channel(colour, sumBlue, seen);
    return 0xff000000 | ((red & 0xff) << 16) | ((green & 0xff) << 8) | (blue & 0xff);
}

nfs::u32 frontend::rendTrackLitColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b9550", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 car = args.eax, colour = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::trackLitColour(app, ts, car, colour); });
}

// ---------------------------------------------------------------------------
// The replay's pose
// ---------------------------------------------------------------------------

/* sub_4b9df0: the recorded car (0x55fe24's model, sub_477d90's pose) in view eax beside car
 * edx, the player's, as its ghost, unless it stands in the car; at the detail its distance
 * gives by the setting, with its own batch of models. */
nfs::u32 frontend::rendd::replayPose(App* app, ThreadState& ts, u32 view, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x9c);
    constexpr u32 kFromFixed = 0x540100;     // a double, 1/65536
    constexpr u32 kNearCar = 0x540108;       // a float: nearer the car than this and it stands in it
    constexpr u32 kNearAxis = 0x540110;      // a double: and over its axes
    const u32 frame = guest.frame();
    const u32 fixedAxes = frame;             // 9 fixed point, as recorded
    const u32 axes = frame + 0x24;           // 9 floats
    const u32 fromCamera = frame + 0x48;
    const u32 fixedPlace = frame + 0x54;     // 3 fixed point
    const u32 place = frame + 0x60;          // 3 floats
    const u32 list = frame + 0x6c;
    u32 lightOn = guest.dword(guest.dword(kRenderState) + 4);
    const u32 got = fe::direct<0x477d90, frontend::loopa::playbackPose>(app, ts, fixedPlace, fixedAxes);
    if (got == 0)
        return got;
    const u32 model = guest.dword(kReplayModel);
    if (model == 0 || s32(guest.dword(model + kModelParts)) <= 0)
        return got;
    const double fromFixed = doubleOf(guest, kFromFixed);
    for (u32 i = 0; i < 9; ++i)
        guest.setFloat(axes + 4 * i, float(fp::mul(double(s32(guest.dword(fixedAxes + 4 * i))), fromFixed)));
    for (u32 i = 0; i < 3; ++i)
        guest.setFloat(place + 4 * i, float(fp::mul(double(s32(guest.dword(fixedPlace + 4 * i))), fromFixed)));

    // Hidden when it stands where the car does, turned as it is.  `measured` is what the last
    // measure left in eax, which the answers below keep the high half of.
    u32 measured = 0;
    const auto distance = [&](u32 from, u32 to) {
        const frontend::sima::FloatLeft quick = fe::direct<0x4965b0, frontend::sima::quickDistanceOf>(app, ts, from, to);
        measured = quick.eax;
        return quick.value;
    };
    if (isBelow(fp::compare(distance(car + kCarPlace, place), floatOf(guest, kNearCar)))
        && isBelow(fp::compare(distance(car + kCarAxes, axes), doubleOf(guest, kNearAxis)))
        && isBelow(fp::compare(distance(car + 0xcc, axes + 0xc), doubleOf(guest, kNearAxis))))
    {
        const nfs::u16 fsw = fp::compare(distance(car + kCarForward, axes + 0x18), doubleOf(guest, kNearAxis));
        if (isBelow(fsw))
            return (measured & 0xffff0000) | fsw;
    }
    const float away = float(distance(place, view + 8));
    {
        const nfs::u16 fsw = fp::compare(-floatOf(guest, car + 0x11c), double(away));
        if (isAbove(fsw))
            return (measured & 0xffff0000) | fsw;
    }
    const u32 distances = gameSettings(guest).detailLevel << 4;
    {
        const nfs::u16 fsw = fp::compare(double(away), floatOf(guest, kDetailDistances + distances + 0xc));
        if (isAbove(fsw))
            return (measured & 0xffff0000) | fsw;
    }
    u32 detail = 0;
    if (isAbove(fp::compare(double(away), floatOf(guest, kDetailDistances + distances))))
        detail = 1;
    if (isAbove(fp::compare(double(away), floatOf(guest, kDetailDistances + distances + 4))))
        detail = 2;
    if (isAbove(fp::compare(double(away), floatOf(guest, kDetailDistances + distances + 8))))
        detail = 3;

    fe::direct<0x49c410, frontend::rendc::objectsBegin>(app, ts);
    callAt(guest, kEndModelBatch);
    fe::direct<0x49d750, frontend::rendc::modelSetShading>(app, ts, gameSettings(guest).useSpecular != 0 ? 1u : 0u);
    fe::direct<0x49d760, frontend::rendc::modelSetLight>(app, ts, kSun);
    // edx too, as the game's code hands it: sub_49d800 below reads it.
    fe::direct<0x49d8f0, frontend::rendc::modelSetColour>(app, ts, 0xffffffffu);
    fe::direct<0x49d800, render::glareColoursSet>(app, ts, 0x7f7f7f7f, 0xffffffff);
    fe::direct<0x49d720, frontend::rendc::modelSetAlphas>(app, ts, 0x8fu, 0x4fu);
    if (gameSettings(guest).useLights == 0)
        lightOn = 0;
    else if (lightOn != 0)
    {
        fe::direct<0x4e0000, vertices::offsetVectorList<false>>(app, ts, 1, view + 8, place, fromCamera);
        fe::direct<0x49d780, frontend::rendc::modelSetEnvironment>(app, ts, fromCamera);
    }

    const auto part = [&](u32 which, u32 offset, u32 flag) {
        fe::direct<0x49de20, frontend::rendc::modelTransformPart>(app, ts, model, which, 0u, model + offset, axes, place,
                                                                  view + 0x44, view + 0x38, flag);
    };
    switch (detail)
    {
    case 0:
        part(0, 0xfc, lightOn);
        part(1, 0x108, 0);
        part(2, 0x114, 0);
        part(3, 0x120, 0);
        part(4, 0x12c, 0);
        break;
    case 1:
        part(5, 0x138, lightOn);
        part(6, 0x144, 0);
        part(7, 0x150, 0);
        part(8, 0x15c, 0);
        part(9, 0x168, 0);
        break;
    case 2:
        part(0xa, 0x174, lightOn);
        break;
    default:
        part(0xb, 0x180, 0);
        break;
    }
    fe::direct<0x49d720, frontend::rendc::modelSetAlphas>(app, ts, 0xffu, 0x7fu);
    fe::direct<0x49c470, frontend::rendc::objectsEnd>(app, ts, list);
    if (guest.dword(kDriverOnCard) == 0)
    {
        const u32 head = guest.dword(list), tail = guest.dword(list + 4);
        guest::direct<0x4c7380, rende::polygonsSorted>(app, ts, list, head, tail);
    }
    fe::direct<0x4e0430, vertices::transformPointList>(app, ts, 1, place, view + 0x44, view + 0x38, place);
    const double depth = fp::sub(floatOf(guest, place + 8), floatOf(guest, kDepthBias));
    return fe::direct<0x49c420, frontend::rendc::objectsDraw>(app, ts, view, list, bitsOf(float(depth)));
}

nfs::u32 frontend::rendReplayPose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b9df0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 view = args.eax, car = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::replayPose(app, ts, view, car); });
}

// ---------------------------------------------------------------------------
// One car
// ---------------------------------------------------------------------------

/* sub_4ba320: the state models draw in: the ambient by the setting, the sun, the outline's
 * colours ebx and edx, as the call hands it them (the car). */
nfs::u32 frontend::rendd::carStates(App* app, ThreadState& ts, u32 view, u32 car, u32 outline)
{
    static_cast<void>(view);   // in eax, not read
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 4);
    fe::direct<0x49d750, frontend::rendc::modelSetShading>(app, ts, gameSettings(guest).useSpecular != 0 ? 1u : 0u);
    fe::direct<0x49d760, frontend::rendc::modelSetLight>(app, ts, kSun);
    // The game's code hands it the car in edx too, which it does not read: left there for the caller.
    return fe::direct<0x49d8f0, frontend::rendc::modelSetColour>(app, ts, outline);
}

nfs::u32 frontend::rendCarStates(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ba320", 0, 0, nullptr, 0, 0 };
    const u32 view = args.eax, car = args.edx, outline = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::carStates(app, ts, view, car, outline); });
}

/* sub_4ba350: the specular colours of car edx's paint, ebx, in view eax, and the light's flag
 * back: nothing when view is a mirror's (*eax 1) or the lights are off, and less the longer a
 * tunnel it is in. */
nfs::u32 frontend::rendd::carShade(App* app, ThreadState& ts, u32 view, u32 car, u32 paint)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x94);
    const auto over256 = [](u32 value) { return u32((s32(value) + (s32(value) < 0 ? 0xff : 0)) >> 8); };
    const auto over512 = [](u32 value) { return u32((s32(value) + (s32(value) < 0 ? 0x1ff : 0)) >> 9); };
    const auto cutByte = [](u32 value) -> u32 { return s32(value) < 0 ? 0u : s32(value) > 0xff ? 0xffu : value; };
    const auto cutBright = [](u32 value) -> u32 { return s32(value) < 0x7f ? 0x7fu : s32(value) > 0xff ? 0xffu : value; };
    const u32 frame = guest.frame();
    const u32 paintRgb = frame;              // red, green, blue, in dwords
    const u32 mixedHsv = frame + 0x10;
    const u32 paintHsv = frame + 0x20;
    const u32 hsvIn = frame + 0x30;
    const u32 mixedRgb = frame + 0x40;       // the ambient's, then with the paint's
    const u32 rgbOut = frame + 0x50;
    const u32 fromLight = frame + 0x60;
    const u32 segment = guest.dword(car + kCarSegment);
    const u32 place = car + kCarPlace;
    const u32 flag = guest.dword(guest.dword(kRenderState) + 4);
    if (guest.dword(view) == 1)
        return 0;
    if (gameSettings(guest).useLights == 0)
        return 0;
    u32 specular = 0x100, ambientShare = 0x100;
    const auto inTunnel = [&](u32 at) {
        return *fe::direct<0x420bf0, trackdata::placeInTunnel>(app, ts, place, at) != 0;
    };
    if (inTunnel(segment))
    {
        u32 reach = 1;
        for (;;)
        {
            u32 ahead = segment + reach;
            const u32 count = guest.dword(kSegmentCount);
            if (s32(ahead) >= s32(count))
                ahead -= count;
            u32 behind = segment - reach;
            if (s32(behind) < 0)
                behind += guest.dword(kSegmentCount);
            if (!inTunnel(ahead) || !inTunnel(behind))
                break;
            if (++reach >= 8)
                break;
        }
        specular = 0x100 - (reach << 4);
        ambientShare = 0x100 - (reach << 5);
    }
    fe::direct<0x4e0000, vertices::offsetVectorList<false>>(app, ts, 1, view + 8, place, fromLight);
    fe::direct<0x49d780, frontend::rendc::modelSetEnvironment>(app, ts, fromLight);
    const u32 red = u32(s32(paint) >> 16) & 0xff, green = u32(s32(paint) >> 8) & 0xff, blue = paint & 0xff;
    guest.setDword(paintRgb, red);
    guest.setDword(paintRgb + 4, green);
    guest.setDword(paintRgb + 8, blue);
    fe::direct<0x49c880, load::colourToHsv>(guest.app(), guest.ts(), paintRgb, paintHsv);
    guest.setDword(mixedRgb, guest.byte(kAmbientBlue + 2));
    guest.setDword(mixedRgb + 4, guest.byte(kAmbientBlue + 1));
    guest.setDword(mixedRgb + 8, guest.byte(kAmbientBlue));
    if (inTunnel(segment))
    {
        const u32 paintShare = 0x100 - ambientShare;
        const auto mix = [&](u32 own, u32 ambient) {
            const u32 fromPaint = over256(own * paintShare);
            return over256(ambient * ambientShare) + fromPaint;
        };
        guest.setDword(mixedRgb, mix(red, guest.dword(mixedRgb)));
        guest.setDword(mixedRgb + 4, mix(green, guest.dword(mixedRgb + 4)));
        guest.setDword(mixedRgb + 8, mix(blue, guest.dword(mixedRgb + 8)));
    }
    fe::direct<0x49c880, load::colourToHsv>(guest.app(), guest.ts(), mixedRgb, mixedHsv);
    const u32 hue = guest.dword(mixedHsv), saturation = guest.dword(mixedHsv + 4), value = guest.dword(mixedHsv + 8);
    const u32 paintValue = guest.dword(paintHsv + 8);
    const u32 wear = guest.dword(car + 0x8a8 + 8);
    guest.setDword(hsvIn, cutByte(hue));
    guest.setDword(hsvIn + 4, cutByte(over256(saturation * 96)));
    const u32 first = cutBright(over512(value * paintValue) + 0x80 - u32(s32(wear) >> 2));
    guest.setDword(hsvIn + 8, first);
    fe::direct<0x49ca50, load::hsvToColour>(guest.app(), guest.ts(), hsvIn, rgbOut);
    const u32 colourOne = ((first & 0xff) << 24) | ((guest.dword(rgbOut) & 0xff) << 16) | ((guest.dword(rgbOut + 4) & 0xff) << 8)
                            | (guest.dword(rgbOut + 8) & 0xff);
    guest.setDword(hsvIn, cutByte(hue));
    guest.setDword(hsvIn + 4, cutByte(u32(s32(saturation) >> 1)));
    const u32 second = cutBright(over512(value * paintValue) + 0x80);
    guest.setDword(hsvIn + 8, second);
    fe::direct<0x49ca50, load::hsvToColour>(guest.app(), guest.ts(), hsvIn, rgbOut);
    const u32 colourTwo = ((second & 0xff) << 24) | ((guest.dword(rgbOut) & 0xff) << 16) | ((guest.dword(rgbOut + 4) & 0xff) << 8)
                            | (guest.dword(rgbOut + 8) & 0xff);
    const u32 factor = specular << 8;
    const u32 scaledOne = scaledColour(ts, colourOne, factor);
    const u32 scaledTwo = scaledColour(ts, colourTwo, factor);
    fe::direct<0x49d800, render::glareColoursSet>(app, ts, scaledOne, scaledTwo);
    return flag;
}

nfs::u32 frontend::rendCarShade(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ba350", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 view = args.eax, car = args.edx, paint = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::carShade(app, ts, view, car, paint); });
}

/* sub_4ba810: car eax's colour, the average of the road's at its four wheels, the place's
 * own for those not on it. */
nfs::u32 frontend::rendd::carAverageColour(App* app, ThreadState& ts, u32 car)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x20);
    const u32 own = fe::direct<0x4206f0, trackdata::placeColour>(app, ts, car + 0xc, 1u);
    const auto wheel = [&](u32 place) -> u32 {
        if (guest.dword(car + place + 0x14) == 0 || guest.dword(car + place + 0x18) == 0)
            return own;
        return fe::direct<0x4206f0, trackdata::placeColour>(app, ts, car + place, 1u);
    };
    const u32 a = wheel(0x300);
    const u32 b = wheel(0x378);
    const u32 c = wheel(0x3f0);
    const u32 d = wheel(0x468);
    const auto average = [&](unsigned shift) {
        const u32 sum = ((a >> shift) & 0xff) + ((b >> shift) & 0xff) + ((c >> shift) & 0xff) + ((d >> shift) & 0xff);
        return u32(s32(sum) >> 2) & 0xff;
    };
    return 0xff000000 | (average(16) << 16) | (average(8) << 8) | average(0);
}

nfs::u32 frontend::rendCarAverageColour(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ba810", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::carAverageColour(app, ts, car); });
}

namespace
{

/* A car's axes to `matrix` in the frame, its place to `place`, for a part to draw it by: the
 * height raised by the float at `raise`, as each detail's own has. */
inline void carFrame(Guest& guest, u32 car, u32 matrix, u32 place, u32 raise)
{
    copyDwords(guest, matrix, car + kCarAxes, 9);
    copyDwords(guest, place, car + kCarPlace, 3);
    guest.setFloat(place + 4, float(fp::add(floatOf(guest, place + 4), floatOf(guest, raise))));
}

/* sub_49de20 called as the game's code calls it, `arguments` in the registers and pushed: sub_4b8740
 * after it reads what is left in its frame, which a C++ call (fe::direct), pushing none of the five
 * stack arguments, would leave 20 bytes up -- and the registers it leaves.  Its depth back.  The call is made
 * as Guest::call makes it, rendc's modelTransformPartCalled in place of the jump by address, which
 * leaves everything as the call does. */
double partByAddress(Guest& guest, const fe::Arguments& arguments)
{
    nfs::ThreadState& ts = guest.ts();
    const u32 esp = ts.sp;
    // The five stack arguments pushed.
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        ts.sp -= 4;
        guest.setDword(ts.sp, arguments.stack[i]);
    }
    const frontend::rendc::Nearest nearest = guest::direct<0x49de20, frontend::rendc::modelTransformPart>(
        guest.app(), ts, arguments.registers[0], arguments.registers[1], arguments.registers[2], arguments.registers[3],
        arguments.stack[0], arguments.stack[1], arguments.stack[2], arguments.stack[3], arguments.stack[4]);
    ts.sp = esp;
    if (ts.terminate)
        throw fe::Terminated();
    return nearest.depth;
}

/* The whole car as one part of its model; its depth, what sub_4b8740 sizes by, kept.  By address
 * (partByAddress). */
inline u32 drawWhole(Guest& guest, u32 model, u32 view, u32 part, u32 offset, u32 matrix, u32 place, u32 flag)
{
    return bitsOf(float(partByAddress(guest, args({ model, part, 0, model + offset },
                                                  { matrix, place, view + 0x44, view + 0x38, flag }))));
}

}

/* sub_4ba960: a car at its least detail: its whole in one, and its lights. */
nfs::u32 frontend::rendd::carFarthest(App* app, ThreadState& ts, u32 car, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x58);
    constexpr u32 kRaise = 0x540118;
    const u32 model = guest.dword(car + kCarModel);
    const u32 matrix = guest.frame(), place = matrix + 0x24;
    const u32 colour = fe::direct<0x4206f0, trackdata::placeColour>(app, ts, car + 0xc, 1u);
    if (model == 0 || s32(guest.dword(model + kModelParts)) <= 0xb)
        return colour;
    guest::direct<0x4ba320, carStates>(app, ts, view, car, colour);
    carFrame(guest, car, matrix, place, kRaise);
    const u32 size = drawWhole(guest, model, view, 0xb, 0x180, matrix, place, 0);
    // By address: sub_4b8740 reads what is left in its frame from before (an unknown light's size,
    // colour, facing), which a C++ call, pushing no arguments, would move 8 bytes up.
    return guest::direct<0x4b8740, carLights>(app, ts, view, car, matrix, place, size, colour);
}

nfs::u32 frontend::rendCarFarthest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ba960", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 car = args.eax, view = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::carFarthest(app, ts, car, view); });
}

/* sub_4baa50: a car far away: its whole, its lights and its shadow. */
nfs::u32 frontend::rendd::carFar(App* app, ThreadState& ts, u32 car, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x5c);
    constexpr u32 kRaise = 0x54011c;
    const u32 model = guest.dword(car + kCarModel);
    const u32 matrix = guest.frame(), place = matrix + 0x24;
    u32 colour = fe::direct<0x4206f0, trackdata::placeColour>(app, ts, car + 0xc, 1u);
    if (model == 0 || s32(guest.dword(model + kModelParts)) <= 0xa)
        return colour;
    if (gameSettings(guest).night != 0)
        colour = *fe::direct<0x4b97b0, render::nightColourOf>(app, ts, car, colour);
    const u32 shade = guest::direct<0x4ba350, carShade>(app, ts, view, car, colour);
    guest::direct<0x4ba320, carStates>(app, ts, view, car, colour);
    carFrame(guest, car, matrix, place, kRaise);
    const u32 size = drawWhole(guest, model, view, 0xa, 0x174, matrix, place, shade);
    // By address: sub_4b8740 reads what is left in its frame from before (an unknown light's size,
    // colour, facing), which a C++ call, pushing no arguments, would move 8 bytes up.
    guest::direct<0x4b8740, carLights>(app, ts, view, car, matrix, place, size, colour);
    return guest::direct<0x4b81a0, carShadow>(app, ts, car, view);
}

nfs::u32 frontend::rendCarFar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4baa50", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 car = args.eax, view = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::carFar(app, ts, car, view); });
}

/* sub_4bab70: a car near: body, a police bar if it has one, four wheels each of its own
 * turn, then the lights by its lean, and the shadow. */
nfs::u32 frontend::rendd::carNear(App* app, ThreadState& ts, u32 car, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0xc8);
    constexpr u32 kRaise = 0x540120;
    constexpr u32 kWheelsHidden = 0x99c;     // of a car: a bit for each wheel
    const u32 frame = guest.frame();
    const u32 axes = frame;
    const u32 lean = frame + 0x24;
    const u32 wheel = frame + 0x48;
    const u32 bodyTurn = frame + 0x6c;
    const u32 place = frame + 0x90;
    const u32 model = guest.dword(car + kCarModel);
    u32 colour = guest::direct<0x4ba810, carAverageColour>(app, ts, car);
    if (model == 0 || s32(guest.dword(model + kModelParts)) <= 5)
        return colour;
    if (gameSettings(guest).night != 0)
        colour = *fe::direct<0x4b97b0, render::nightColourOf>(app, ts, car, colour);
    guest::direct<0x4ba320, carStates>(app, ts, view, car, colour);
    const u32 shade = guest::direct<0x4ba350, carShade>(app, ts, view, car, colour);
    carFrame(guest, car, axes, place, kRaise);
    guest::direct<0x4b93c0, bodyLean>(app, ts, car, lean, axes);
    // By address (partByAddress): sub_4b8740 at the end reads what is left in its frame, which a
    // C++ call, pushing none of the five stack arguments, would leave 20 bytes up.
    const auto part = [&](u32 which, u32 offset, u32 turn, u32 flag) {
        return partByAddress(guest, args({ model, which, turn, model + offset }, { axes, place, view + 0x44, view + 0x38, flag }));
    };
    const u32 size = bitsOf(float(part(5, 0x138, lean, shade)));
    if (guest.dword(car + kCarLightsOn) != 0)
        part(0xc, 0x18c, lean, shade);
    guest::direct<0x4b9140, frontWheelTurn>(app, ts, car, wheel, 0u);
    if ((guest.byte(car + kWheelsHidden) & 1) == 0)
        part(6, 0x144, wheel, 0);
    if ((guest.byte(car + kWheelsHidden) & 2) == 0)
        part(7, 0x150, wheel, 0);
    guest::direct<0x4b9280, rearWheelTurn>(app, ts, car, wheel);
    if ((guest.byte(car + kWheelsHidden) & 8) == 0)
        part(8, 0x15c, wheel, 0);
    if ((guest.byte(car + kWheelsHidden) & 4) == 0)
        part(9, 0x168, wheel, 0);
    // The game's code hands it the place in ecx too, which the lights take.
    fe::direct<0x4e0280, vertices::multiplyMatrix>(app, ts, lean, axes, bodyTurn);
    // By address: sub_4b8740 reads what is left in its frame from before (an unknown light's size,
    // colour, facing), which a C++ call, pushing no arguments, would move 8 bytes up.
    // The place in ecx, as the call before leaves it.
    guest::direct<0x4b8740, carLights>(app, ts, view, car, bodyTurn, place, size, colour);
    return guest::direct<0x4b81a0, carShadow>(app, ts, car, view);
}

nfs::u32 frontend::rendCarNear(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bab70", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 car = args.eax, view = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::carNear(app, ts, car, view); });
}

/* sub_4bb110: traffic's car at its nearest: one part, its lights and its shadow. */
nfs::u32 frontend::rendd::carSimpleNear(App* app, ThreadState& ts, u32 car, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x58);
    constexpr u32 kRaise = 0x540128;
    const u32 model = guest.dword(car + kCarModel);
    const u32 matrix = guest.frame(), place = matrix + 0x24;
    const u32 colour = fe::direct<0x4206f0, trackdata::placeColour>(app, ts, car + 0xc, 1u);
    if (model == 0 || s32(guest.dword(model + kModelParts)) <= 5)
        return colour;
    guest::direct<0x4ba320, carStates>(app, ts, view, car, colour);
    carFrame(guest, car, matrix, place, kRaise);
    const u32 size = drawWhole(guest, model, view, 5, 0x180, matrix, place, 0);
    // By address: sub_4b8740 reads what is left in its frame from before (an unknown light's size,
    // colour, facing), which a C++ call, pushing no arguments, would move 8 bytes up.
    guest::direct<0x4b8740, carLights>(app, ts, view, car, matrix, place, size, colour);
    return guest::direct<0x4b81a0, carShadow>(app, ts, car, view);
}

nfs::u32 frontend::rendCarSimpleNear(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bb110", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 car = args.eax, view = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::carSimpleNear(app, ts, car, view); });
}

/* sub_4bb4c0: car eax in view edx, in its own batch of models, the detail ebx brought down
 * to what its model has: a detail's draw for a full model, or the simple ones'. */
nfs::u32 frontend::rendd::drawCar(App* app, ThreadState& ts, u32 car, u32 view, u32 detail)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x18);
    const u32 list = guest.frame();
    const u32 model = guest.dword(car + kCarModel);
    fe::direct<0x49c410, frontend::rendc::objectsBegin>(app, ts);
    callAt(guest, kEndModelBatch);
    const auto hasParts = [&](s32 parts) { return model != 0 && s32(guest.dword(model + kModelParts)) > parts; };
    if (detail == 3 && !hasParts(0xb))
        detail = 2;
    if (detail == 2 && !hasParts(0xa))
        detail = 1;
    if (detail == 1 && !hasParts(5))
        detail = 0;
    if (detail <= 3)
    {
        if (hasParts(0xb))
        {
            static const u32 kFull[] = { kDrawCarDetail, kCarNear, kCarFar, kCarFarthest };
            callAt(guest, kFull[detail], args({ car, view }));
        }
        else if (detail == 3)
            guest::direct<0x4bb110, carSimpleNear>(app, ts, car, view);
        else
            fe::direct<0x4bb200, render::mediumCarLightsDrawn>(app, ts, car, view);
    }
    fe::direct<0x49c470, frontend::rendc::objectsEnd>(app, ts, list);
    if (guest.dword(kDriverOnCard) == 0)
    {
        const u32 head = guest.dword(list);
        guest::direct<0x4c7380, rende::polygonsSorted>(app, ts, list, head, guest.dword(list + 4));
    }
    const double depth = fp::sub(floatOf(guest, car + kCarDepth), floatOf(guest, kDepthBias));
    return fe::direct<0x49c420, frontend::rendc::objectsDraw>(app, ts, view, list, bitsOf(float(depth)));
}

nfs::u32 frontend::rendDrawCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bb4c0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 car = args.eax, view = args.edx, detail = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::drawCar(app, ts, car, view, detail); });
}

// ---------------------------------------------------------------------------
// All the cars
// ---------------------------------------------------------------------------

/* sub_4bba10: how much of the budget the cars' details use: 4 less the detail of each car drawn. */
nfs::u32 frontend::rendd::detailInUse(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    u32 used = 0;
    for (u32 i = 0; s32(i) < s32(guest.dword(kCarCount)); ++i)
    {
        const u32 detail = guest.dword(guest.dword(kCarTable + 4 * i) + kCarDetail);
        if (s32(detail) >= 0)
            used += 4 - detail;
    }
    return used;
}

nfs::u32 frontend::rendDetailInUse(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bba10", 0, 0, nullptr, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::detailInUse(app, ts); });
}

/* sub_4bba50: the farthest car but the one numbered eax at detail edx brought to detail ebx;
 * 0 when there is none. */
nfs::u32 frontend::rendd::lowerDetail(App* app, ThreadState& ts, u32 keep, u32 detail, u32 lowered)
{
    Guest guest(app, ts);
    u32 farthest = 0;
    s32 found = -1;
    u32 i = 0;
    for (; s32(i) < s32(guest.dword(kCarCount)); ++i)
    {
        const u32 car = guest.dword(kCarTable + 4 * i);
        if (i == keep)
            continue;
        if (!isAbove(fp::compare(floatOf(guest, car + kCarDepth), double(floatFrom(farthest)))))
            continue;
        if (guest.dword(car + kCarDetail) != detail)
            continue;
        farthest = guest.dword(car + kCarDepth);
        found = s32(i);
    }
    if (found < 0)
        return 0;
    guest.setDword(guest.dword(kCarTable + 4 * u32(found)) + kCarDetail, lowered);
    return 1;
}

nfs::u32 frontend::rendLowerDetail(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bba50", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 keep = args.eax, detail = args.edx, lowered = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::lowerDetail(app, ts, keep, detail, lowered); });
}

/* sub_4bbad0: the cars' details cut to what the setting's budget allows, the camera eax's own
 * car spared; with a car asking for the second view, every such car's by the setting. */
nfs::u32 frontend::rendd::fitDetailBudget(App* app, ThreadState& ts, u32 camera, u32 ecx)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x24);
    constexpr u32 kWantsBothViews = 0x9a4;
    const u32 ownCar = fe::direct<0x422ba0, carstate::carOfCamera>(app, ts, guest.dword(camera + 4));
    u32 own = ecx;  // as it came: kept when no car is the camera's
    for (u32 i = 0; s32(i) < s32(guest.dword(kCarCount)); ++i)
    {
        const u32 car = guest.dword(kCarTable + 4 * i);
        if (fe::direct<0x422ba0, carstate::carOfCamera>(app, ts, guest.dword(camera + 4)) == car)
            own = i;
    }
    u32 left;   // what the last call leaves in eax
    for (;;)
    {
        const u32 setting = gameSettings(guest).detailLevel;
        const u32 used = guest::direct<0x4bba10, detailInUse>(app, ts);
        left = used;
        if (s32(used) <= s32(guest.dword(kDetailBudget + 4 * setting)))
            break;
        left = guest::direct<0x4bba50, lowerDetail>(app, ts, own, 1u, 2u);
        if (left != 0)
            continue;
        left = guest::direct<0x4bba50, lowerDetail>(app, ts, own, 0u, 1u);
        if (left == 0)
            break;
    }
    if (guest.dword(ownCar + kWantsBothViews) == 0)
        return left;
    u32 i = 0;
    for (; s32(i) < s32(guest.dword(kCarCount)); ++i)
    {
        const u32 car = guest.dword(kCarTable + 4 * i);
        if (guest.dword(car + kWantsBothViews) != 0)
            guest.setDword(car + kCarDetail, gameSettings(guest).detailLevel != 0 ? 1 : 0);
    }
    return i;
}

nfs::u32 frontend::rendFitDetailBudget(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bbad0", 0, 0, nullptr, 0 };
    const u32 camera = args.eax, ecx = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::fitDetailBudget(app, ts, camera, ecx); });
}

/* sub_4bbba0: every car in view eax: each one's detail by its distance (sub_4bb5d0), fitted
 * to the budget, what goes with each, each drawn when it shows, a car of half the frames every
 * other frame; the replay's pose, the particles; and the frame noted as the one wheels moved on. */
nfs::u32 frontend::rendd::cars(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x18);
    constexpr u32 kEveryOtherFrame = 0x7d36a0;
    for (u32 i = 0; s32(i) < s32(guest.dword(kCarCount)); ++i)
    {
        const u32 car = guest.dword(kCarTable + 4 * i);
        guest.setDword(car + kCarDepth, 0);
        guest.setDword(car + kCarDetail, callAt(guest, kCarVisible, args({ view, car, car + kCarDepth })));
    }
    guest::direct<0x4bbad0, fitDetailBudget>(app, ts, view, 0xffffffffu);
    for (u32 i = 0; s32(i) < s32(guest.dword(kCarCount)); ++i)
    {
        const u32 car = guest.dword(kCarTable + 4 * i);
        const u32 detail = guest.dword(car + kCarDetail);
        if (s32(detail) <= -1)
            continue;
        if (guest.dword(car + 0x8e4) != 0 && guest.dword(car + 0x8e8) != 0)
            fe::direct<0x4b9bc0, render::carLightsDrawn>(app, ts, car, view);
        if (guest.byte(car + 0x89) == 0 || guest.byte(car + 0x88) != 0)
            continue;
        if (guest.dword(car + 0x174) != 0)
        {
            if (guest.dword(kFrameCount) != guest.dword(car + 0x8c0))
                guest.setDword(car + 0x8bc, guest.dword(car + 0x8bc) + 1);
            guest.setDword(car + 0x8c0, guest.dword(kFrameCount));
            if ((guest.byte(car + 0x8bc) & 1) != 0 && guest.dword(kEveryOtherFrame) == 0)
                continue;
        }
        guest::direct<0x4bb4c0, drawCar>(app, ts, car, view, detail);
    }
    if (guest.dword(kReplayShowing) == 1)
        guest::direct<0x4b9df0, replayPose>(app, ts, view, guest.dword(kPlayerCars + 4 * gameSettings(guest).localPlayer));
    guest::direct<0x4c9f70, rende::blinkingQuads>(app, ts, view);
    const u32 now = guest.dword(kFrameCount);
    guest.setDword(kWheelStamp, now);
    return now;
}

nfs::u32 frontend::rendCars(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bbba0", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::cars(app, ts, view); });
}

// ---------------------------------------------------------------------------
// The frame's arena and its lists
// ---------------------------------------------------------------------------

/* sub_4bbd70: the arena back to its start, as a pass begins. */
nfs::u32 frontend::rendd::arenaRewind(App* app, ThreadState& ts)
{
    arenaReset(app, ts);
    Guest guest(app, ts);
    const u32 start = guest.dword(kArena);
    guest.setDword(kArenaNext, start);
    return start;
}

nfs::u32 frontend::rendArenaRewind(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bbd70", 0, 0, nullptr, 0, 0 };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::arenaRewind(app, ts); });
}

/* sub_4bbd80: the arena made, its next byte aligned to 32, 100 bytes of its size kept back. */
nfs::u32 frontend::rendd::arenaCreate(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    const u32 block = *fe::direct<0x4b56e0, render::transformBufferAllocated>(app, ts, kArenaSize);
    guest.setDword(kArena, block);
    const u32 next = block + 0x1f;
    guest.setDword(kArenaNext, next);
    const std::uint8_t aligned = guest.byte(kArenaNext) & 0xe0;
    guest.setDword(kArenaSize, guest.dword(kArenaSize) - 0x64);
    guest.setByte(kArenaNext, aligned);
    return (next & 0xffff00ff) | (u32(aligned) << 8);
}

nfs::u32 frontend::rendArenaCreate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bbd80", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::arenaCreate(app, ts); });
}

/* sub_4bbdc0: the arena freed. */
nfs::u32 frontend::rendd::arenaFree(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    const u32 freed = guest::direct<0x4b5700, freeBlock>(app, ts, guest.dword(kArena));
    guest.setDword(kArena, 0);  // the edx the freeing is handed and leaves
    return freed;
}

nfs::u32 frontend::rendArenaFree(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bbdc0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::arenaFree(app, ts); });
}

/* sub_4bbde0: edx bytes of the arena to *eax, the next aligned to 32; 1, or 0 and the log's
 * line when it has run out. */
nfs::u32 frontend::rendd::arenaTake(App* app, ThreadState& ts, u32 slot, u32 bytes)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    constexpr u32 kOutOfArena = 0x540148;    // "Render_GetTm() raM out of raN\n"
    const u32 taken = guest.dword(kArenaNext);
    guest.setDword(slot, taken);
    const u32 past = taken + bytes;
    guest.setDword(kArenaNext, past + 0x1f);
    guest.setByte(kArenaNext, guest.byte(kArenaNext) & 0xe0);
    const u32 used = guest.dword(kArenaNext) - guest.dword(kArena);
    if (s32(used) <= s32(guest.dword(kArenaSize)))
        return 1;
    fe::direct<kLog, ea::error>(guest.app(), guest.ts(), kOutOfArena, guest::Words({}));
    return 0;
}

nfs::u32 frontend::rendArenaTake(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bbde0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 slot = args.eax, bytes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::arenaTake(app, ts, slot, bytes); });
}

/* sub_4bbe40: where the arena is now, to *eax; 1, or 0 when it has run out. */
nfs::u32 frontend::rendd::arenaHere(App* app, ThreadState& ts, u32 slot)
{
    Guest guest(app, ts);
    const u32 here = guest.dword(kArenaNext);
    guest.setDword(slot, here);
    const u32 start = guest.dword(kArena);
    return s32(here - start) <= s32(guest.dword(kArenaSize)) ? 1 : 0;
}

nfs::u32 frontend::rendArenaHere(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bbe40", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 slot = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::arenaHere(app, ts, slot); });
}

/* A list is two dwords in the caller's frame: its first record and its last.  These leave esi
 * past their own frame's copy, as their movsd does. */

/* sub_4bbe70: *esi an empty list; esi its own address in eax. */
nfs::u32 frontend::rendd::emptyList(App* app, ThreadState& ts, u32 list)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    guest.setDword(list, 0);
    guest.setDword(list + 4, 0);
    return list;
}

nfs::u32 frontend::rendEmptyList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bbe70", 0, 0, nullptr, 0, fe::kRestoreEdx | fe::kRestoreEdi };
    const u32 list = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::emptyList(app, ts, list); });
}

/* sub_4bbe90: *esi the list of the one record eax, 0 for none. */
nfs::u32 frontend::rendd::listOfOne(App* app, ThreadState& ts, u32 record, u32 list)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x14);
    if (record != 0)
        guest.setDword(record, 0);
    guest.setDword(list, record);
    guest.setDword(list + 4, record);
    return list;
}

nfs::u32 frontend::rendListOfOne(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bbe90", 0, 0, nullptr, 0, fe::kRestoreEdx | fe::kRestoreEdi };
    const u32 record = args.eax, list = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::listOfOne(app, ts, record, list); });
}

/* sub_4bbec0: *esi the two lists on the stack joined, an empty one giving the other. */
nfs::u32 frontend::rendd::joinLists(App* app, ThreadState& ts, u32 list, u32 firstHead, u32 firstTail, u32 secondHead, u32 secondTail)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x18);
    u32 head = secondHead, tail = secondTail;
    if (firstHead != 0)
    {
        head = firstHead;
        tail = firstTail;
        if (secondHead != 0)
        {
            guest.setDword(firstTail, secondHead);
            tail = secondTail;
        }
    }
    guest.setDword(list, head);
    guest.setDword(list + 4, tail);
    return list;
}

nfs::u32 frontend::rendJoinLists(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bbec0", 0x10, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEdi };
    const u32 firstHead = app->getMemory<u32>(ts.sp + 4), firstTail = app->getMemory<u32>(ts.sp + 8);
    const u32 secondHead = app->getMemory<u32>(ts.sp + 0xc), secondTail = app->getMemory<u32>(ts.sp + 0x10);
    const u32 list = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::joinLists(app, ts, list, firstHead, firstTail, secondHead, secondTail); });
}

/* sub_4bbf20: the open list empty. */
nfs::u32 frontend::rendd::openListStart(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    guest.setDword(kOpenListTail, 0);
    guest.setDword(kOpenListHead, 0);
    return 0;  // nothing: the thunk leaves eax as it came
}

nfs::u32 frontend::rendOpenListStart(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bbf20", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { rendd::openListStart(app, ts); return args.eax; });
}

/* sub_4bbf40: the list on the stack put on the end of the open one. */
nfs::u32 frontend::rendd::openListAppend(App* app, ThreadState& ts, u32 head, u32 tail)
{
    return openListAppendKeeping(app, ts, head, tail, 0);
}

nfs::u32 frontend::rendd::openListAppendKeeping(App* app, ThreadState& ts, u32 head, u32 tail, u32 eax)
{
    Guest guest(app, ts);
    if (head == 0 || tail == 0)
        return eax;
    if (guest.dword(kOpenListHead) == 0)
        guest.setDword(kOpenListHead, head);
    else
        guest.setDword(guest.dword(kOpenListTail), head);
    guest.setDword(kOpenListTail, tail);
    return tail;
}

nfs::u32 frontend::rendOpenListAppend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bbf40", 8, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 head = app->getMemory<u32>(ts.sp + 4), tail = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::openListAppendKeeping(app, ts, head, tail, args.eax); });
}

/* sub_4bbf80: edx bytes of the arena to *eax, on the end of the open list; 1, or 0 when the
 * arena has run out. */
nfs::u32 frontend::rendd::openListTake(App* app, ThreadState& ts, u32 slot, u32 bytes)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 8);
    if (guest::direct<0x4bbde0, arenaTake>(app, ts, slot, bytes) == 0)
        return 0;
    if (guest.dword(kOpenListHead) == 0)
        guest.setDword(kOpenListHead, guest.dword(slot));
    else
    {
        const u32 tail = guest.dword(kOpenListTail);
        guest.setDword(tail, guest.dword(slot));
    }
    guest.setDword(kOpenListTail, guest.dword(slot));
    return 1;
}

nfs::u32 frontend::rendOpenListTake(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bbf80", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 slot = args.eax, bytes = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::openListTake(app, ts, slot, bytes); });
}

/* sub_4bbfc0: the open list closed, to *esi. */
nfs::u32 frontend::rendd::openListClose(App* app, ThreadState& ts, u32 list)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x10);
    const u32 head = guest.dword(kOpenListHead), tail = guest.dword(kOpenListTail);
    if (tail != 0)
        guest.setDword(tail, 0);
    guest.setDword(list, head);
    guest.setDword(list + 4, tail);
    return list;
}

nfs::u32 frontend::rendOpenListClose(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bbfc0", 0, 0, nullptr, 0, fe::kRestoreEdi };
    const u32 list = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::openListClose(app, ts, list); });
}

/* sub_4bc000: the vertices of mesh eax flushed and listed, the list to *esi. */
nfs::u32 frontend::rendd::flushAndList(App* app, ThreadState& ts, u32 mesh, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x18);
    const u32 list = guest.frame();
    // esi is where the flush puts what it gathers, and the listing keeps it
    // eax as the code has it at the call: the mesh it came with, which the flush reads as deep (non-zero: the far
    // buckets too, where the trees and fences are).
    callAt(guest, kFlushVertices, args({ mesh }), list);
    callAt(guest, kListVertices, args({ mesh }), list);
    const u32 from = list;  // the esi the listing keeps
    guest.setDword(out, guest.dword(from));
    guest.setDword(out + 4, guest.dword(from + 4));
    return out;
}

nfs::u32 frontend::rendFlushAndList(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bc000", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const u32 mesh = args.eax, out = args.esi;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::flushAndList(app, ts, mesh, out); });
}

namespace
{

/* The list emptied through the frame's copy, as the three below end; what the call leaves in eax. */
inline u32 emptyThroughFrame(nfs::App* app, nfs::ThreadState& ts, Guest& guest, u32 list)
{
    const u32 copy = guest.frame();
    const u32 left = guest::direct<0x4bbe70, frontend::rendd::emptyList>(app, ts, copy);
    guest.setDword(list, guest.dword(copy));
    guest.setDword(list + 4, guest.dword(copy + 4));
    return left;
}

}

/* sub_4bc030: the list edx drawn by sub_4349d0 in view eax, and left empty. */
nfs::u32 frontend::rendd::drawListPlain(App* app, ThreadState& ts, u32 view, u32 list)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x1c);
    const u32 tail = guest.dword(list + 4), head = guest.dword(list);
    fe::direct<kDrawListSoftware, collide::listDrawnMirrored>(guest.app(), guest.ts(), view, head);
    return emptyThroughFrame(app, ts, guest, list);
}

nfs::u32 frontend::rendDrawListPlain(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bc030", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 view = args.eax, list = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::drawListPlain(app, ts, view, list); });
}

/* sub_4bc060: the list edx drawn in view eax, by the card's driver at the depth on the stack,
 * and left empty. */
nfs::u32 frontend::rendd::drawListAt(App* app, ThreadState& ts, u32 view, u32 list, u32 depth)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x18);
    if (guest.dword(kDriverOnCard) == 0)
        callAt(guest, kDrawListAtDepth, args({ guest.dword(list) }, { depth }));
    else
    {
        const u32 head = guest.dword(list), tail = guest.dword(list + 4);
        fe::direct<kDrawListSoftware, collide::listDrawnMirrored>(guest.app(), guest.ts(), view, head);
    }
    return emptyThroughFrame(app, ts, guest, list);
}

nfs::u32 frontend::rendDrawListAt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bc060", 4, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 depth = app->getMemory<u32>(ts.sp + 4);
    const u32 view = args.eax, list = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::drawListAt(app, ts, view, list, depth); });
}

/* sub_4bc0b0: the list edx drawn in view eax, by the card's driver as a list of the depth on
 * the stack, and left empty. */
nfs::u32 frontend::rendd::drawListMirrored(App* app, ThreadState& ts, u32 view, u32 list, u32 depth)
{
    Guest guest(app, ts);
    const fe::Frame ownFrame(guest, ts, 0x1c);
    const u32 head = guest.dword(list), tail = guest.dword(list + 4);
    if (guest.dword(kDriverOnCard) == 0)
    {
        guest::direct<0x4c7400, rende::depthBucketAppend>(app, ts, view, head, tail, depth);
    }
    else
    {
        fe::direct<kDrawListSoftware, collide::listDrawnMirrored>(guest.app(), guest.ts(), view, head);
    }
    return emptyThroughFrame(app, ts, guest, list);
}

nfs::u32 frontend::rendDrawListMirrored(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bc0b0", 4, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 depth = app->getMemory<u32>(ts.sp + 4);
    const u32 view = args.eax, list = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::drawListMirrored(app, ts, view, list, depth); });
}

// ---------------------------------------------------------------------------
// The projection
// ---------------------------------------------------------------------------

/* sub_4bf110: the table of reciprocals' mantissas at 0x7d30d8, 256 of them, each with its
 * exponent's bits set. */
nfs::u32 frontend::rendd::fillReciprocals(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    constexpr u32 kReciprocals = 0x7d30d8;
    for (u32 i = 0; i < 0x100; ++i)
    {
        const float value = floatFrom((i << 15) | 0x3f804000);
        const u32 inverse = bitsOf(float(fp::div(1.0, double(value))));
        guest.setDword(kReciprocals + 4 * i, (inverse & 0x7fffff) | 0x7e800000);
    }
    return 0x100;
}

nfs::u32 frontend::rendFillReciprocals(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bf110", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::fillReciprocals(app, ts); });
}

/* sub_4bf1b0: the numbers of the clip: the view's left, top, right and bottom, then its two
 * on the stack, near and far; and every vertex of the cache marked unused. */
nfs::u32 frontend::rendd::setClipBounds(App* app, ThreadState& ts, u32 left, u32 top, u32 right, u32 bottom, u32 nearest, u32 farthest)
{
    Guest guest(app, ts);
    constexpr u32 kVertexCacheCount = 0xc8;
    constexpr u32 kVertexCacheUsed = 0x7cf4c4;
    guest.setDword(0x7d34f4, left);
    guest.setDword(0x7d34ec, right);
    guest.setDword(0x7d34e0, top);
    guest.setDword(0x7d34dc, bottom);
    guest.setDword(0x7d34e8, farthest);
    guest.setDword(0x7d34f0, nearest);
    guest.setDword(0x7d34fc, left);
    guest.setDword(0x7d350c, right);
    guest.setDword(0x7d3504, top);
    guest.setDword(0x7d3500, bottom);
    guest.setDword(0x7d34e4, 0x3f800000);
    guest.setDword(0x7d34d8, farthest);
    guest.setDword(0x7d34f8, nearest);
    for (u32 i = 0; i < kVertexCacheCount; ++i)
        guest.setDword(kVertexCacheUsed + (i << 5), 0);
    return kVertexCacheCount;
}

nfs::u32 frontend::rendSetClipBounds(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bf1b0", 0x18, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 left = app->getMemory<u32>(ts.sp + 4), top = app->getMemory<u32>(ts.sp + 8);
    const u32 right = app->getMemory<u32>(ts.sp + 0xc), bottom = app->getMemory<u32>(ts.sp + 0x10);
    const u32 nearest = app->getMemory<u32>(ts.sp + 0x14), farthest = app->getMemory<u32>(ts.sp + 0x18);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::setClipBounds(app, ts, left, top, right, bottom, nearest, farthest); });
}

/* sub_4bf260: the projection's scale and centre: the window's corner and size on the stack, the
 * fields of view next (0 for none, then the size itself), mirrored when the last is set. */
nfs::u32 frontend::rendd::setProjection(App* app, ThreadState& ts, u32 cornerXBits, u32 cornerYBits, u32 widthBits,
                                          u32 heightBits, u32 fovX, u32 fovY, u32 mirrored)
{
    Guest guest(app, ts);
    constexpr u32 kHalfFloat = 0x5401c0;
    constexpr u32 kDegrees = 0x5401c4;       // to radians, by half: a float
    const double half = floatOf(guest, kHalfFloat);
    const double width = double(floatFrom(widthBits)), height = double(floatFrom(heightBits));
    const double halfWidth = fp::mul(width, half);
    const double halfHeight = fp::mul(half, height);
    guest.setDword(kScaleX, widthBits);
    guest.setFloat(kHalfWidth, float(halfWidth));
    guest.setFloat(kHalfHeight, float(halfHeight));
    guest.setFloat(kCentreX, float(fp::add(floatOf(guest, kHalfWidth), double(floatFrom(cornerXBits)))));
    guest.setFloat(kCentreY, float(fp::add(halfHeight, double(floatFrom(cornerYBits)))));
    double scaleY;
    if ((fovX & 0x7fffffff) == 0)
        scaleY = fp::mul(height, fp::div(width, height));
    else
    {
        const double degrees = floatOf(guest, kDegrees);
        const float halfAngle = float(fp::mul(double(floatFrom(fovX)), degrees));
        double scaleX;
        if ((fovY & 0x7fffffff) == 0)
        {
            const double inverse = fp::div(1.0, fp::tan(double(halfAngle)));
            const double aspect = fp::mul(height, fp::div(width, height));
            scaleX = fp::mul(width, inverse);
            scaleY = fp::mul(inverse, aspect);
        }
        else
        {
            const double tangentX = fp::tan(double(halfAngle));
            const double tangentY = fp::tan(fp::mul(double(floatFrom(fovY)), degrees));
            scaleX = fp::div(width, tangentX);
            scaleY = fp::div(height, tangentY);
        }
        guest.setFloat(kScaleX, float(scaleX));
    }
    guest.setFloat(kScaleY, float(scaleY));
    const std::uint8_t flipped = guest.byte(kScaleY + 3) ^ 0x80;
    const double finalX = fp::mul(floatOf(guest, kScaleX), half);
    guest.setByte(kScaleY + 3, flipped);
    const double finalY = fp::mul(half, floatOf(guest, kScaleY));
    guest.setFloat(kScaleX, float(finalX));
    guest.setFloat(kScaleY, float(finalY));
    if (mirrored != 0)
        guest.setByte(kScaleX + 3, guest.byte(kScaleX + 3) ^ 0x80);
    return (widthBits & 0xffff00ff) | (u32(flipped) << 8);
}

nfs::u32 frontend::rendSetProjection(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bf260", 0x1c, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 cornerX = app->getMemory<u32>(ts.sp + 4), cornerY = app->getMemory<u32>(ts.sp + 8);
    const u32 width = app->getMemory<u32>(ts.sp + 0xc), height = app->getMemory<u32>(ts.sp + 0x10);
    const u32 fovX = app->getMemory<u32>(ts.sp + 0x14), fovY = app->getMemory<u32>(ts.sp + 0x18);
    const u32 mirrored = app->getMemory<u32>(ts.sp + 0x1c);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        return rendd::setProjection(app, ts, cornerX, cornerY, width, height, fovX, fovY, mirrored);
    });
}

/* sub_4bf370: eax points at edx, 12 bytes each, projected to the vertices at ebx, 32 bytes each:
 * x and y on the screen, z, its reciprocal, the clip code.  A depth of 0 is kept from dividing.
 * The last point takes the reciprocal of the one before it: the look ahead stops one short. */
nfs::u32 frontend::rendd::projectPoints(App* app, ThreadState& ts, u32 count, u32 points, u32 vertices)
{
    Guest guest(app, ts);
    if (s32(count) <= 0)
        return count;
    u32 point = points, vertex = vertices;
    if ((guest.dword(point + 8) & 0x7fffffff) == 0)
        guest.setDword(point + 8, kSmallestDepth);
    const double scaleX = floatOf(guest, kScaleX), scaleY = floatOf(guest, kScaleY);
    const double centreX = floatOf(guest, kCentreX), centreY = floatOf(guest, kCentreY);
    const ClipBounds bounds = clipBounds(guest);
    u32 inverse = bitsOf(float(fp::div(1.0, floatOf(guest, point + 8))));
    for (u32 left = count; left != 0; --left)
    {
        const u32 own = inverse;
        const double inverseValue = double(floatFrom(own));
        const double perX = fp::mul(scaleX, inverseValue);
        const double perY = fp::mul(scaleY, inverseValue);
        const double y = fp::mul(perY, floatOf(guest, point + 4));
        const double x = fp::mul(perX, floatOf(guest, point));
        const u32 screenX = bitsOf(float(fp::add(x, centreX)));
        const u32 screenY = bitsOf(float(fp::add(y, centreY)));
        guest.setDword(vertex + 0xc, own);
        guest.setDword(vertex, screenX);
        guest.setDword(vertex + 8, guest.dword(point + 8));
        guest.setDword(vertex + 4, screenY);
        if (s32(left - 2) > 0)
        {
            if ((guest.dword(point + 0x14) & 0x7fffffff) == 0)
                guest.setDword(point + 0x14, kSmallestDepth);
            inverse = bitsOf(float(fp::div(1.0, floatOf(guest, point + 0x14))));
        }
        guest.setByte(vertex + 0x14, clipCode(bounds, s32(own), s32(screenY), s32(screenX)));
        point += 0xc;
        vertex += 0x20;
    }
    return point;
}

nfs::u32 frontend::rendProjectPoints(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bf370", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 count = args.eax, points = args.edx, vertices = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return rendd::projectPoints(app, ts, count, points, vertices); });
}

}
