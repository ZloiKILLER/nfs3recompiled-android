#include "native_frontend_rendd.h"
#include "native_game.h"
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
#include <nfs3hp.h>
#include <cstdint>
#include <cstring>

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
 * to a callee by address is kept in the frame: the rest of the generated code's frame
 * is plain locals here.  A value on
 * the x87 is a double, as the generated code holds it, and each operation rounds
 * through fp:: as it does, in its order and operand order: a float's 24 bits.
 *
 * The registers no caller reads are left as the generated code leaves them, which
 * the check holds them to: that is what leaveEbx and the others are doing here.
 *
 * Plan step 5: each function a typed core in frontend::rendd (native_frontend_rendd.h), in a
 * frame of its own where the game's function has one (fe::Frame: what it calls runs at the stack
 * addresses it did), and an adapter under the game's name for the calls by address.  A core of
 * this round's three files (rendd, rende, loada) calls another as C++ (guest::direct), and the
 * typed cores of other files too (fe::direct); the rest of the game still by address. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

// Port (tools/apply_native_vertices.py): only watches how much of the arena a pass used.
bool arenaReset(win32::WinApplication* app, x86::CPU& cpu);
// Port (tools/apply_car_detail.py): the frame a car's wheels last moved, per car and per use.
x86::reg32 carWheelStamp(win32::WinApplication* app, x86::reg32 car, x86::reg32 which);

namespace
{

using fe::args;
using fe::Guest;
using x86::reg32;
using x86::sreg32;

// ---------------------------------------------------------------------------
// The x87 as the generated code holds it
// ---------------------------------------------------------------------------

// fp::compare's word, as a jump after sahf reads it: ja, and jb (true unordered).
inline bool isAbove(x86::reg16 fsw) { return fsw == 0; }
inline bool isBelow(x86::reg16 fsw) { return (fsw & 0x100) != 0; }

// fld dword, fld qword.
inline double floatOf(const Guest& guest, reg32 address) { return double(guest.floatAt(address)); }
inline double doubleOf(const Guest& guest, reg32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// A float as it goes through an integer register, and back.
inline reg32 bitsOf(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}
inline float floatFrom(reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}
// A double built from its two halves, as the code moves constants into a frame.
inline double doubleFrom(reg32 low, reg32 high)
{
    const std::uint64_t bits = std::uint64_t(low) | (std::uint64_t(high) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

inline sreg32 fistp(double value) { return fp::toInteger<sreg32>(value); }

// A value from the x87 as the game keeps a byte: below 0 and past 255 cut, fistp each
// time.  The first from the stack, the others from where it was stored as a float.
inline reg32 clampedByte(double first, float stored)
{
    if (fistp(first) < 0)
        return 0;
    if (fistp(double(stored)) > 0xff)
        return 0xff;
    return reg32(fistp(double(stored)));
}

inline void copyDwords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* A colour's four bytes times a fraction of 0x10000, as the game's macro has it; ebx
 * left with what it leaves there. */
inline reg32 scaledColour(x86::CPU& cpu, reg32 colour, reg32 factor)
{
    if (sreg32(factor) >= 0x10000)
        factor = 0xffff;
    const reg32 by = (factor >> 8) & 0xff;
    const reg32 redBlue = (colour & 0x00ff00ff) * by;
    const reg32 alphaGreen = ((colour & 0xff00ff00) >> 8) * by;
    cpu.ebx = (redBlue >> 8) & 0x00ff00ff;
    return (alphaGreen & 0xff00ff00) | cpu.ebx;
}

/* The clip code of a vertex on the screen, from its reciprocal depth and position as
 * integers: 0x10 behind or too near, else 8 and 4 above and below, 1 and 2 left and right.
 * The floats compared as their bits, as the game does; a negative one is out. */
struct ClipBounds
{
    sreg32 nearest, top, bottom, left, right;
};

inline ClipBounds clipBounds(const Guest& guest)
{
    return { sreg32(guest.dword(0x7d34f8)), sreg32(guest.dword(0x7d3504)), sreg32(guest.dword(0x7d3500)),
             sreg32(guest.dword(0x7d34fc)), sreg32(guest.dword(0x7d350c)) };
}

inline std::uint8_t clipCode(const ClipBounds& bounds, sreg32 inverseDepth, sreg32 y, sreg32 x)
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
constexpr reg32 kSmallestDepth = 0x37800080;

// ---------------------------------------------------------------------------
// The game's functions and data, by address
// ---------------------------------------------------------------------------

constexpr reg32 kCreateArena = 0x4b56e0;       // eax where its size is
constexpr reg32 kLog = 0x401010;               // the game's printf to its log
constexpr reg32 kCompareNoCase = 0x4eb390;     // eax, edx, ebx the most
constexpr reg32 kEndModelBatch = 0x49d640;
constexpr reg32 kTrackPointAt = 0x420650;      // eax the point, edx 1, ebx the segment, esi the record it fills
constexpr reg32 kTransformPoints = 0x4e0430;   // eax how many, edx them, ebx a matrix, ecx a place; the stack where to
constexpr reg32 kRotationY = 0x4dfde0;
constexpr reg32 kRotationZ = 0x4dfe40;
constexpr reg32 kMultiplyMatrices = 0x4e0280;  // eax by edx to ebx
constexpr reg32 kGunFire = 0x42cff0;           // eax the car: how far through its flash, on the x87
constexpr reg32 kFlare = 0x492370;             // eax a view; the stack the point, colour, size...
constexpr reg32 kLightFlare = 0x491bc0;        // likewise, a record of the road's light back
constexpr reg32 kRingFlare = 0x492980;
constexpr reg32 kSetSpecular = 0x49d800;
constexpr reg32 kDrawModelPart = 0x49de20;     // eax the model, edx a part, ebx a part's matrix, ecx its offset
constexpr reg32 kRgbToHsv = 0x49c880;          // eax to edx, three dwords
constexpr reg32 kHsvToRgb = 0x49ca50;
constexpr reg32 kNightColour = 0x4b97b0;       // eax the car, edx its colour
constexpr reg32 kCarVisible = 0x4bb5d0;        // eax a view, edx a car, ebx where its distance goes: its detail, -1 if none
constexpr reg32 kDrawCarDetail = 0x4bae00;
constexpr reg32 kDrawTrafficDetail = 0x4bb200;
constexpr reg32 kDrawCarExtras = 0x4b9bc0;
constexpr reg32 kDrawListAtDepth = 0x4c7790;
constexpr reg32 kDrawListSoftware = 0x4349d0;
constexpr reg32 kFlushVertices = 0x4c79a0;
constexpr reg32 kListVertices = 0x4c7750;

// This group's own still called by address: through a table, one whose edx the caller reads, and the
// lights, which read their frame's leftovers.
constexpr reg32 kCarLights = 0x4b8740;
constexpr reg32 kCarFarthest = 0x4ba960;
constexpr reg32 kCarFar = 0x4baa50;
constexpr reg32 kCarNear = 0x4bab70;
constexpr reg32 kFreeBlock = 0x4b5700;

// The arena: the block, the next free byte, its size; and the open list on it.
constexpr reg32 kArena = 0x55fe34;
constexpr reg32 kArenaNext = 0x55fe38;
constexpr reg32 kArenaSize = 0x7a3d04;
constexpr reg32 kOpenListTail = 0x7a3d08;
constexpr reg32 kOpenListHead = 0x7a3d0c;

constexpr reg32 kFrameCount = 0x7d3684;
constexpr reg32 kWheelStamp = 0x55fe28;
constexpr reg32 kDriverOnCard = 0x7a3a60;      // 1 but on the software renderers
constexpr reg32 kDepthBias = 0x7dcfe0;
constexpr reg32 kSun = 0x55d2a8;
constexpr reg32 kCarTable = 0x5efac8;          // the cars
constexpr reg32 kCarCount = 0x5efd9c;
constexpr reg32 kPlayerCars = 0x5efa48;        // the players' cars
constexpr reg32 kReplayModel = 0x55fe24;
constexpr reg32 kReplayShowing = 0x55d000;     // 1 when the replay's pose is drawn
constexpr reg32 kPlayerModel = 0x55fe2c;
constexpr reg32 kDetailDistances = 0x55fdf4;   // per setting 16 bytes: three distances and the last
constexpr reg32 kDetailBudget = 0x4b7a50;      // in the code: three dwords, per setting
constexpr reg32 kAmbientBlue = 0x6ff9f8;       // three bytes
constexpr reg32 kRenderState = 0x563a90;       // a record; at +4 the light's flag
constexpr reg32 kBackFaceFlip = 0x554e4c;
constexpr reg32 kShadowTexture = 0x8b4898;
constexpr reg32 kSegmentCount = 0x5dd940;
constexpr reg32 kSegmentLights = 0x571370;     // every 8 segments 0x5c0 bytes: at +0x5b8 a count, at +0x5bc the lights

// The projection's numbers.
constexpr reg32 kScaleX = 0x56009c;
constexpr reg32 kScaleY = 0x5600a0;
constexpr reg32 kCentreX = 0x5600a4;
constexpr reg32 kCentreY = 0x5600a8;
constexpr reg32 kHalfWidth = 0x5600ac;
constexpr reg32 kHalfHeight = 0x5600b0;

// A car, as these read it.
constexpr reg32 kCarSegment = 0x1c;
constexpr reg32 kCarSpeedLow = 0x20;
constexpr reg32 kCarPlace = 0x98;
constexpr reg32 kCarAxes = 0xc0;               // a matrix of 9 floats
constexpr reg32 kCarForward = 0xd8;
constexpr reg32 kCarVelocityUnit = 0xa4;
constexpr reg32 kCarHalfSize = 0x114;
constexpr reg32 kCarFlags = 0x200;
constexpr reg32 kCarModel = 0x8a4;
constexpr reg32 kCarDetail = 0x8b4;
constexpr reg32 kCarDepth = 0x8b8;
constexpr reg32 kCarHasCop = 0x734;
constexpr reg32 kCarLightsOn = 0x660;
constexpr reg32 kCarBraking = 0x509;
constexpr std::uint8_t kFlagCop = 0x20;
constexpr std::uint8_t kFlagHidden = 0x40;

// A model.
constexpr reg32 kModelParts = 0xf8;
constexpr reg32 kModelLightCount = 0x34;
constexpr reg32 kModelLights = 0x38;           // points of 12 bytes
constexpr reg32 kModelLightNames = 0xa04;      // names of 0x40 bytes, each letter meaning something

// A track surface no shadow falls on.
constexpr std::uint8_t kSurfaceNoShadow = 0xe;

}

// ---------------------------------------------------------------------------
// Starting and ending
// ---------------------------------------------------------------------------

/* sub_4b5700: the block eax freed, if there is one. */
x86::reg32 frontend::rendd::freeBlock(App* app, CPU& cpu, reg32 block)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    // What the call leaves in eax, or the block, 0.
    reg32 left = block;
    if (block != 0)
        left = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
    return left;
}

void frontend::rendFreeBlock(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b5700", 0, 0, nullptr, 0, 0 };
    const reg32 block = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::freeBlock(app, cpu, block); });
}

/* sub_4b5720: which driver of the eleven (their names at 0x55fd68) the registry names, and
 * over that the switches of the command line (0x55eb2c, two bytes), to *eax. */
x86::reg32 frontend::rendd::chooseDriver(App* app, CPU& cpu, reg32 chosen)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x5c);
    constexpr reg32 kDriverNames = 0x55fd68;
    constexpr reg32 kDriverNameCount = 11;
    constexpr reg32 kSwitches = 0x55eb2c;
    const reg32 name = guest.frame();
    fe::direct<0x4b64c0, frontend::wina::thrashDescription>(app, cpu, 0x7a3a74u);
    // The game's code hands it 0 in edx too, which it does not read.
    fe::direct<0x4b6400, frontend::wina::thrashDevice>(app, cpu, name);
    reg32 driver = 0;
    for (; driver < kDriverNameCount; ++driver)
    {
        // The count lives just past the name's 0x40 bytes: its low byte ends a name that fills them.
        guest.setDword(name + 0x40, driver);
        const reg32 length = guest.length(name);
        if (fe::direct<kCompareNoCase, watcom::clib::strncmp>(guest.app(), guest.cpu(), guest.dword(kDriverNames + 4 * driver), name, length) == 0)
            break;
    }
    if (driver == kDriverNameCount)
        fe::direct<0x4b5e50, frontend::wina::fatalMessage>(app, cpu, 0u);
    guest.setDword(chosen, driver);
    const std::uint8_t low = guest.byte(kSwitches);
    const reg32 left = (driver & 0xffff00ff) | (reg32(low) << 8);
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

void frontend::rendChooseDriver(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b5720", 0, 0, nullptr, 0 };
    const reg32 chosen = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::chooseDriver(app, cpu, chosen); });
}

/* sub_4b5940: which of the driver's five features answer (THRASH_setstate's pointer, by a
 * table of its two arguments at 0x55fd94), a bit each to 0x7a3a5c. */
x86::reg32 frontend::rendd::readDriverFeatures(App* app, CPU& cpu, reg32 first)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x18);
    constexpr reg32 kFeatures = 0x7a3a5c;
    constexpr reg32 kFeatureQueries = 0x55fd94;
    guest.setDword(kFeatures, 0);
    reg32 bit = first;
    for (reg32 i = 0; i < 5; ++i)
    {
        const std::optional<reg32> answer = guest::driver<thrash::kThrashSetState, &port::driver::setState>(
            app, cpu, guest.dword(kFeatureQueries + 8 * i), guest.dword(kFeatureQueries + 8 * i + 4));
        if (!answer)
            throw fe::Terminated();
        bit = (*answer != 0 ? 1u : 0u) << i;
        guest.setDword(kFeatures, guest.dword(kFeatures) | bit);
    }
    return bit;
}

void frontend::rendReadDriverFeatures(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b5940", 0, 0, nullptr, 0 };
    const reg32 first = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::readDriverFeatures(app, cpu, first); });
}

/* sub_4b7d00: the player's model freed, as a race ends, and the models' batch ended. */
x86::reg32 frontend::rendd::freePlayerModel(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    const reg32 model = guest.dword(kPlayerModel);
    if (model != 0)
    {
        guest.leaveEcx(0);
        fe::direct<0x49d5a0, frontend::rendc::modelFree>(app, cpu, model);
        guest.setDword(kPlayerModel, 0);   // ecx, its 0 kept across the call
    }
    return guest.call(kEndModelBatch);
}

void frontend::rendFreePlayerModel(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b7d00", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::freePlayerModel(app, cpu); });
}

/* sub_4b7eb0: car eax's model freed, and no car left pointing at it; the replay's model too
 * when this is the player's and the replay is showing. */
x86::reg32 frontend::rendd::freeCarModel(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    reg32 left = car;
    const reg32 player = guest.dword(kPlayerCars + 4 * gameSettings(guest).localPlayer);
    const reg32 model = guest.dword(car + kCarModel);
    if (car == player && guest.dword(kReplayShowing) == 1)
    {
        const reg32 replay = guest.dword(kReplayModel);
        if (replay != 0)
        {
            fe::direct<0x49d5a0, frontend::rendc::modelFree>(app, cpu, replay);
            guest.setDword(kReplayModel, 0);
            left = 0;
        }
    }
    if (model != 0)
    {
        fe::direct<0x49d5a0, frontend::rendc::modelFree>(app, cpu, model);
        for (left = 0; sreg32(left) < sreg32(guest.dword(kCarCount)); ++left)
        {
            const reg32 other = guest.dword(kCarTable + 4 * left);
            if (model == guest.dword(other + kCarModel))
                guest.setDword(other + kCarModel, 0);
        }
    }
    return left;
}

void frontend::rendFreeCarModel(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b7eb0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::freeCarModel(app, cpu, car); });
}

// ---------------------------------------------------------------------------
// The shadow
// ---------------------------------------------------------------------------

/* sub_4b7f80: the four corners of car eax's shadow to edx, 12 bytes each, in the world: its
 * half size turned by its axes, each corner's height the road's under it (sub_4968a0), or
 * for a car in detail the track's own below it unless that is a surface with no shadow. */
x86::reg32 frontend::rendd::shadowCorners(App* app, CPU& cpu, reg32 car, reg32 corners)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc4);
    constexpr reg32 kToFixed = 0x53ffb0;       // a double, 65536
    constexpr reg32 kFromFixed = 0x53ffb8;     // a double, 1/65536
    const reg32 record = guest.frame();        // what sub_420650 fills
    const reg32 fixedPoint = record + 0x90;    // the corner it is given, as fixed point
    const reg32 halfX = guest.dword(car + kCarHalfSize), halfY = guest.dword(car + kCarHalfSize + 4);
    const reg32 halfZ = guest.dword(car + kCarHalfSize + 8);
    const reg32 minusX = bitsOf(float(-double(floatFrom(halfX))));
    const reg32 minusY = bitsOf(float(-double(floatFrom(halfY))));
    const reg32 minusZ = bitsOf(float(-double(floatFrom(halfZ))));
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
    fe::direct<0x4e0430, vertices::transformPointList>(app, cpu, 4, corners, car + kCarAxes, car + kCarPlace, corners);
    reg32 left = car;   // set again on every pass below
    for (reg32 i = 0; i < 4; ++i)
    {
        const reg32 corner = corners + 12 * i;
        guest.leaveEdx(corner);
        const float height = fe::direct<0x4968a0, vertices::groundHeightAt>(
            app, cpu, car + 0x124, guest.floatAt(car + 0x148), guest.floatAt(car + 0x14c), guest.floatAt(car + 0x150),
            guest.floatAt(corner), guest.floatAt(corner + 4), guest.floatAt(corner + 8));
        guest.setFloat(corner + 4, height);
        left = car;
        if (sreg32(guest.dword(car + kCarDetail)) >= 2)
            continue;
        const double toFixed = doubleOf(guest, kToFixed);
        reg32 x = reg32(fistp(fp::mul(floatOf(guest, corner), toFixed)));
        reg32 y = reg32(fistp(fp::mul(floatOf(guest, corner + 4), toFixed)));
        reg32 z = reg32(fistp(fp::mul(floatOf(guest, corner + 8), toFixed)));
        guest.setDword(fixedPoint, x);
        guest.setDword(fixedPoint + 4, y);
        guest.setDword(fixedPoint + 8, z);
        guest.setEsi(record);
        left = guest.call(kTrackPointAt, args({ fixedPoint, 1, guest.dword(car + kCarSegment) }));
        const reg32 below = guest.dword(record + 0x14), surface = guest.dword(record + 0x18);
        if (below == 0 || surface == 0)
            continue;
        left = guest.byte(surface + 1);
        if ((left & 0x3f) == 0)
            continue;
        guest.leaveEdx(surface);
        left = guest.byte(surface + 1);
        if ((left & 0x3f) == kSurfaceNoShadow)
        {
            left = 0;
            continue;
        }
        const reg32 trackY = guest.dword(fixedPoint + 4);
        if (sreg32(y + 0x4000) > sreg32(trackY) && sreg32(trackY) > sreg32(y))
        {
            x = guest.dword(fixedPoint);
            y = trackY;
            z = guest.dword(fixedPoint + 8);
        }
        const double fromFixed = doubleOf(guest, kFromFixed);
        const double worldX = fp::mul(double(sreg32(x)), fromFixed);
        const double worldY = fp::mul(double(sreg32(y)), fromFixed);
        const double worldZ = fp::mul(fromFixed, double(sreg32(z)));
        guest.leaveEdx(corner);
        guest.setFloat(corner, float(worldX));
        guest.setFloat(corner + 4, float(worldY));
        guest.setFloat(corner + 8, float(worldZ));
        left = z;
    }
    return left;
}

void frontend::rendShadowCorners(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b7f80", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = cpu.eax, corners = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::shadowCorners(app, cpu, car, corners); });
}

/* sub_4b81a0: car eax's shadow in view edx, a quad of the arena on the view's list: its
 * corners (sub_4b7f80), when the sun shows moved along the car's edges as far as the height
 * asks, raised a little, projected, and dropped when off the screen or facing away. */
x86::reg32 frontend::rendd::carShadow(App* app, CPU& cpu, reg32 car, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xf4);
    constexpr reg32 kCornerPairs = 0x4b7a30;   // in the code: each corner's two neighbours
    constexpr reg32 kRaise = 0x53ffc0;
    constexpr reg32 kShadowLength = 0x118;     // of a car
    constexpr reg32 kCarLean = 0xd0;
    const reg32 frame = guest.frame();
    const reg32 corners = frame;               // as they stand
    const reg32 moved = frame + 0x30;          // as the sun moves them
    const reg32 edge = frame + 0x80;           // also the point in the view
    const reg32 offset = frame + 0x8c;
    const reg32 from = frame + 0x98;
    const reg32 to = frame + 0xa4;
    const reg32 list = frame + 0xb0;
    const reg32 slope = frame + 0xb8;          // what sub_4e0210 hands back
    const reg32 vertexSlot = frame + 0xbc;
    const reg32 quadSlot = frame + 0xc0;
    guest.setDword(quadSlot, 0);
    guest.leaveEdx(0);
    if (guest.dword(car + 0x184) != 0 || guest.dword(car + kCarSpeedLow) == 0)
        return car;
    const bool sunShows = gameSettings(guest).night == 0 && gameSettings(guest).weather == 0 && guest.dword(view) != 1;
    guest::direct<0x4b7f80, shadowCorners>(app, cpu, car, corners);
    for (reg32 i = 0; i < 4; ++i)
    {
        const reg32 corner = corners + 12 * i, copy = moved + 12 * i;
        const double x = floatOf(guest, corner), y = floatOf(guest, corner + 4), z = floatOf(guest, corner + 8);
        guest.setFloat(copy, float(x));
        guest.setFloat(copy + 4, float(y));
        guest.setFloat(copy + 8, float(z));
    }
    guest.leaveEdx(4);
    if (sunShows)
    {
        for (reg32 i = 0; i < 4; ++i)
        {
            copyDwords(guest, from, corners + 12 * i, 3);
            for (reg32 side = 0; side < 2; ++side)
            {
                const reg32 neighbour = guest.dword(kCornerPairs + 8 * i + 4 * side);
                copyDwords(guest, to, corners + 12 * neighbour, 3);
                fe::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, from, to, edge);
                // The game's code hands it the sun in ebx and the slope in ecx too, which the next takes.
                fe::direct<0x4e01c0, vertices::makeUnitVector>(app, cpu, edge, edge);
                fe::direct<0x4e0210, vertices::dotProductsOf>(app, cpu, 1, edge, kSun, slope);
                const float along = guest.floatAt(slope);
                const double length = isAbove(fp::compare(0.0, double(along))) ? floatOf(guest, car + kShadowLength) : 0.0;
                const double by = fp::mul(-double(along), length);
                fe::direct<0x4e0050, vertices::scaleVectorsBy>(app, cpu, 1, edge, float(by), offset);
                fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, moved + 12 * i, offset, moved + 12 * i);
            }
        }
    }
    const double raise = floatOf(guest, kRaise);
    for (reg32 i = 0; i < 4; ++i)
    {
        const reg32 y = moved + 12 * i + 4;
        guest.setFloat(y, float(fp::add(floatOf(guest, y), raise)));
    }
    if (guest::direct<0x4bbde0, arenaTake>(app, cpu, vertexSlot, 0x80u) == 0)
        return 0;
    if (guest::direct<0x4bbde0, arenaTake>(app, cpu, quadSlot, 0x84u) == 0)
        return 0;
    const reg32 vertices = guest.dword(vertexSlot);
    const reg32 axes = view + 0x44;
    const reg32 lean = isAbove(fp::compare(0.0, floatOf(guest, car + kCarLean))) ? 1 : 0;
    for (reg32 i = 0; i < 4; ++i)
    {
        const reg32 corner = moved + 12 * i;
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
        fe::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, edge, view + 0x38, edge);
        if ((guest.dword(edge + 8) & 0x7fffffff) == 0)
            guest.setDword(edge + 8, kSmallestDepth);
        const float inverse = float(fp::div(1.0, floatOf(guest, edge + 8)));
        const double scaleX = fp::mul(floatOf(guest, kScaleX), double(inverse));
        const double scaleY = fp::mul(floatOf(guest, kScaleY), double(inverse));
        const double screenY = fp::mul(scaleY, floatOf(guest, edge + 4));
        const double screenX = fp::mul(scaleX, floatOf(guest, edge));
        const reg32 vertex = vertices + (i << 5);
        guest.setDword(vertex + 0xc, bitsOf(inverse));
        guest.setFloat(vertex, float(fp::add(screenX, floatOf(guest, kCentreX))));
        guest.setDword(vertex + 8, guest.dword(edge + 8));
        guest.setFloat(vertex + 4, float(fp::add(screenY, floatOf(guest, kCentreY))));
        const std::uint8_t code = clipCode(clipBounds(guest), sreg32(bitsOf(inverse)), sreg32(guest.dword(vertex + 4)), sreg32(guest.dword(vertex)));
        guest.setByte(vertex + 0x14, code);
    }
    const std::uint8_t codes0 = guest.byte(vertices + 0x14), codes1 = guest.byte(vertices + 0x34), codes2 = guest.byte(vertices + 0x54);
    const std::uint8_t codes3 = guest.byte(vertices + 0x74);
    const reg32 someCodes = reg32(codes0) | codes1 | codes2;
    guest.leaveEdx(someCodes);
    if (((someCodes | codes3) & 0x10) == 0)
    {
        const double v0x = floatOf(guest, vertices), v0y = floatOf(guest, vertices + 4);
        const double v1x = floatOf(guest, vertices + 0x20), v1y = floatOf(guest, vertices + 0x24);
        const double v2x = floatOf(guest, vertices + 0x40), v2y = floatOf(guest, vertices + 0x44);
        const double v3x = floatOf(guest, vertices + 0x60), v3y = floatOf(guest, vertices + 0x64);
        const reg32 flip = guest.dword(kBackFaceFlip);
        const double a1 = fp::mul(fp::sub(v2x, v1x), fp::sub(v0y, v1y));
        const double b1 = fp::mul(fp::sub(v2y, v1y), fp::sub(v0x, v1x));
        reg32 faces = 0;
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
    const reg32 quad = guest.dword(quadSlot);
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
    guest.setEsi(list);
    guest.leaveEdx(list);
    guest::direct<0x4bbe90, listOfOne>(app, cpu, quad, list);
    const double depth = fp::sub(floatOf(guest, car + kCarDepth), floatOf(guest, kDepthBias));
    // The list in edx, as the call before leaves it.
    return guest::direct<0x4bc0b0, drawListMirrored>(app, cpu, view, list, bitsOf(float(depth)));
}

void frontend::rendCarShadow(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b81a0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = cpu.eax, view = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::carShadow(app, cpu, car, view); });
}

// ---------------------------------------------------------------------------
// A car's lights and its parts' turns
// ---------------------------------------------------------------------------

/* sub_4b8740: the flares of car edx's lights in view eax, its body at matrix ebx and place
 * ecx, the stack a size and its colour: each of the model's lights, by the letters of its
 * name, a tail light by its brake, a head light by the night, a cop's by the flash, the
 * flares through sub_492370 and the road's light they throw on a list drawn at the car's depth.
 *
 * What the generated code leaves in its frame from before, the size and colour a light of
 * an unknown kind finds, is read from where it would be. */
x86::reg32 frontend::rendd::carLights(App* app, CPU& cpu, reg32 view, reg32 car, reg32 matrix, reg32 place, reg32 flareSize, reg32 paint)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x128);
    constexpr reg32 kPaintWhole = 0x53ffc8;      // a double, 1/765
    constexpr reg32 kPaintPart = 0x53ffd0;       // a double, 0.3
    constexpr reg32 kDimmest = 0x53ffd8;         // a float, 0.7
    constexpr reg32 kRingAfter = 0x53ffe0;       // a double, 30: time past which the lights' ring shows
    constexpr reg32 kRingOffset = 0x53ffe8;      // a float, -30
    constexpr reg32 kRingScale = 0x53ffec;       // a float, 1/30
    constexpr reg32 kHalf = 0x53fff0;            // a double
    constexpr reg32 kFullAlpha = 0x540000;       // a float, 228
    constexpr reg32 kSpinScale = 0x53fff8;       // a double, 1/2048
    constexpr reg32 kFlashScale = 0x540004;      // a float, 2
    constexpr reg32 kTwo = 0x540008;             // a double
    constexpr reg32 kQuarterDouble = 0x540010;   // a double
    constexpr reg32 kFour = 0x540018;            // a double
    constexpr reg32 kQuarter = 0x540020;         // a float
    constexpr reg32 kSizeScale = 0x540028;       // a double, 0.8
    constexpr reg32 kByteScale = 0x540030;       // a float, 255
    constexpr reg32 kOneAndAHalf = 0x540038;     // a double
    constexpr reg32 kByteScaleDouble = 0x540040; // a double, 255
    constexpr reg32 kHaloDim = 0xa666;           // of 0x10000
    const reg32 frame = guest.frame();
    const reg32 world = frame;                   // the light in the world
    const reg32 toViewer = frame + 0xc;
    const reg32 lights = frame + 0x30;           // the road's lights, a list
    const reg32 streak = frame + 0x68;           // a list of one
    const reg32 halo = frame + 0x90;             // likewise

    float facing = guest.floatAt(frame + 0xfc);
    reg32 colour = guest.dword(frame + 0x100);
    reg32 size = guest.dword(frame + 0x104);
    float flash = guest.floatAt(frame + 0x10c);

    guest.setEsi(lights);
    guest::direct<0x4bbe70, emptyList>(app, cpu, lights);
    const float time = float(double(sreg32(fe::direct<0x470a60, frontend::renda::fogAmount>(app, cpu))));
    const reg32 model = guest.dword(car + kCarModel);
    const std::uint8_t flags = guest.byte(car + kCarFlags);
    if (flags & kFlagHidden)
    {
        guest.leaveEdx((guest.edx() & 0xffffff00) | flags);
        return model;
    }

    // How dark the paint is: the lights show less on a pale car.
    const reg32 paintSum = ((paint >> 16) & 0xff) + ((paint >> 8) & 0xff) + (paint & 0xff);
    const double paintWhole = fp::mul(double(sreg32(paintSum)), doubleOf(guest, kPaintWhole));
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
    reg32 ringColour = 0;
    reg32 ringAngle = 0;
    if (isAbove(fp::compare(double(time), doubleOf(guest, kRingAfter))))
    {
        reg32 frames = guest.dword(kFrameCount) - guest.dword(car + 0x948);
        const double speedDot = fe::direct<0x4e01f0, vertices::dotProduct>(app, cpu, car + kCarVelocityUnit, car + kCarForward);
        const double past = fp::add(double(time), floatOf(guest, kRingOffset));
        const double spinWhole = fp::add(speedDot, 1.0);
        const double fadeIn = fp::mul(past, floatOf(guest, kRingScale));
        const sreg32 spin = fistp(fe::direct<0x4dfd56, watcom::clib::chop>(app, cpu, spinWhole));
        const float faded = float(fadeIn);
        float ring = 0.0f;
        if (sreg32(bitsOf(faded)) >= 0x3f800000 || isBelow(fp::compare(0.0, double(faded))))
        {
            const float again = float(fp::mul(fp::add(double(time), floatOf(guest, kRingOffset)), floatOf(guest, kRingScale)));
            ring = sreg32(bitsOf(again)) >= 0x3f800000 ? floatFrom(0x3f800000) : again;
        }
        float alpha = ring;
        if (guest.dword(car + kCarLightsOn) == 1)
            alpha = float(fp::mul(double(ring), doubleOf(guest, kHalf)));
        if (sreg32(frames) < 0)
            frames = 0;
        frames *= reg32(spin);
        const double alphaByte = fp::mul(double(alpha), floatOf(guest, kFullAlpha));
        const reg32 turned = guest.dword(car + 0x944) + frames;
        guest.setDword(car + 0x944, turned);
        guest.setDword(car + 0x944, turned & 0x7ff);
        const double angle = fp::mul(double(sreg32(guest.dword(car + 0x944))), doubleOf(guest, kSpinScale));
        const float alphaStored = float(alphaByte);
        guest.setDword(car + 0x948, guest.dword(kFrameCount));
        ringAngle = bitsOf(float(angle));
        ringColour = ((clampedByte(double(alphaStored), alphaStored) & 0xff) << 24) | 0xffffff;
    }

    // A cop's flash, and which side of it is lit.
    std::uint8_t flashSide = 0;
    if ((guest.byte(car + kCarFlags) & kFlagCop) && guest.dword(car + kCarHasCop) != 0)
    {
        guest.call(kGunFire, args({ car }));
        const double through = guest.fpu().pop();
        flash = float(through);
        flashSide = isAbove(fp::compare(through, doubleOf(guest, kHalf))) ? 0x45 : 0x4f;
        const double doubled = fp::mul(double(flash), floatOf(guest, kFlashScale));
        flash = float(doubled);
        if (isBelow(fp::compare(1.0, doubled)))
            flash = float(fp::sub(doubleOf(guest, kTwo), doubled));
    }

    cpu.ebx = 0;
    for (reg32 i = 0;; ++i)
    {
        if (sreg32(i) >= sreg32(guest.dword(guest.dword(car + kCarModel) + kModelLightCount)))
            break;
        // By address: what it leaves in ebx goes on to the end, which does not restore ebx.
        guest.call(kTransformPoints, args({ 1, model + kModelLights + 12 * i, matrix, place }, { world }));
        const reg32 name = model + (i << 6);
        const reg32 x = guest.dword(world), y = guest.dword(world + 4), z = guest.dword(world + 8);

        // How squarely the light faces the viewer.
        const auto facingViewer = [&]() {
            guest.setFloat(toViewer, float(fp::sub(floatOf(guest, view + 8), double(floatFrom(x)))));
            guest.setFloat(toViewer + 4, float(fp::sub(floatOf(guest, view + 0xc), double(floatFrom(y)))));
            guest.setFloat(toViewer + 8, float(fp::sub(floatOf(guest, view + 0x10), double(floatFrom(z)))));
            fe::direct<0x4972f0, vertices::normaliseVector>(app, cpu, toViewer);
            // The game's code hands it the vector in edx too, which the product takes as its second.
            return fe::direct<0x4e01f0, vertices::dotProduct>(app, cpu, car + kCarForward, toViewer);
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
                    cpu.ebx = 0x3ff00000;
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
                    cpu.ebx = 0x3ff00000;
                }
                else
                    strength = fp::mul(ahead, doubleOf(guest, kFour));
                const float strengthStored = float(strength);
                double wide = 1.5;
                if (guest.dword(car + kCarLightsOn) != 2)
                {
                    wide = 1.0;
                    cpu.ebx = 0x3ff00000;
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
                cpu.ebx = y;
                guest.call(kFlare, args({ view }, { x, y, z, colour, size, flareSize, 0 }));
                if (guest.byte(name + kModelLightNames + 4) != 'N')
                {
                    const reg32 dimmed = scaledColour(cpu, colour, kHaloDim);
                    guest.leaveEbx(z);
                    const reg32 record = guest.call(kLightFlare, args({ view }, { x, y, z, dimmed, guest.dword(car + kCarSegment), size, car + kCarAxes, 0 }));
                    if (record != 0)
                    {
                        guest.setEsi(halo);
                        guest::direct<0x4bbe90, listOfOne>(app, cpu, record, halo);
                        const reg32 head = guest.dword(halo), tail = guest.dword(halo + 4);
                        guest.leaveEbx(guest.dword(lights + 4));
                        guest.setEsi(lights);
                        guest::direct<0x4bbec0, joinLists>(app, cpu, lights, guest.dword(lights), guest.dword(lights + 4), head,
                                                           tail);
                    }
                }
            }
        }
        if (kind == 'H' && (guest.dword(car + kCarLightsOn) != 0 || flashSide == guest.byte(name + kModelLightNames + 3)))
        {
            if (isBelow(fp::compare(0.0, double(facing))))
            {
                cpu.ebx = colour;
                guest.call(kFlare, args({ view }, { x, y, z, colour, size, flareSize, 0 }));
                if (guest.byte(name + kModelLightNames + 4) != 'N')
                {
                    const reg32 dimmed = scaledColour(cpu, colour, kHaloDim);
                    const reg32 record = guest.call(kLightFlare, args({ view }, { x, y, z, dimmed, guest.dword(car + kCarSegment), size, car + kCarAxes, 1 }));
                    if (record != 0)
                    {
                        guest.setEsi(streak);
                        guest::direct<0x4bbe90, listOfOne>(app, cpu, record, streak);
                        const reg32 head = guest.dword(streak), tail = guest.dword(streak + 4);
                        guest.leaveEbx(head);
                        guest.setEsi(lights);
                        guest::direct<0x4bbec0, joinLists>(app, cpu, lights, guest.dword(lights), guest.dword(lights + 4), head,
                                                           tail);
                    }
                }
            }
            if (isAbove(fp::compare(double(time), doubleOf(guest, kRingAfter))))
            {
                guest.leaveEbx(y);
                guest.call(kRingFlare, args({ view }, { x, y, z, car + kCarAxes, ringColour, 0x40c00000, 0x3dcccccd, 0x3f400000, ringAngle, 0xbecccccd }));
            }
        }
        if (kind == 'S' && (guest.byte(car + kCarFlags) & kFlagCop) && guest.dword(car + kCarHasCop) != 0)
        {
            reg32 alphaByte, rgb;
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
            guest.leaveEbx(y);
            guest.call(kFlare, args({ view }, { x, y, z, colour, size, 0, 0xbf400000 }));
        }
    }

    const reg32 head = guest.dword(lights), tail = guest.dword(lights + 4);
    guest.leaveEcx(head);
    guest.leaveEdx(lights);
    guest.setEsi(lights);
    guest::direct<0x4c7380, rende::polygonsSorted>(app, cpu, lights, head, tail);
    const double depth = fp::sub(floatOf(guest, car + kCarDepth), floatOf(guest, kDepthBias));
    // The lights in edx, as the call before leaves it.
    return guest::direct<0x4bc0b0, drawListMirrored>(app, cpu, view, lights, bitsOf(float(depth)));
}

void frontend::rendCarLights(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b8740", 8, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 flareSize = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 paint = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 view = cpu.eax, car = cpu.edx, matrix = cpu.ebx, place = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::carLights(app, cpu, view, car, matrix, place, flareSize, paint); });
}

/* sub_4b90e0: how much of a shake car eax's body has this frame, at random around 0 by its
 * roughness; edx which way, along or across.  Once a frame (nfs3hp::carWheelStamp's). */
x86::reg32 frontend::rendd::bodyShake(App* app, CPU& cpu, reg32 car, reg32 across)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc);
    constexpr reg32 kHalfFloat = 0x540048;
    if (carWheelStamp(app, car, across ? 3u : 2u) == guest.dword(kFrameCount))
        return car;
    // eax as the code leaves it: the random fraction's call keeps it.
    const reg32 eax = guest.eax();
    const double fraction = double(*fe::direct<0x4eb230, ea::randomFractionOf>(app, cpu));
    const reg32 roughness = car + 0x930;
    const double random = fp::mul(fraction, floatOf(guest, roughness));
    const double centre = fp::mul(floatOf(guest, roughness), floatOf(guest, kHalfFloat));
    guest.setFloat(car + (across ? 0x940 : 0x93c), float(fp::sub(random, centre)));
    return eax;
}

void frontend::rendBodyShake(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b90e0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const reg32 car = cpu.eax, across = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::bodyShake(app, cpu, car, across); });
}

namespace
{

/* What a wheel turns this frame, -0.5 to 0.5: `steer` cut at both ends. */
inline float cutToHalf(const Guest& guest, double steer, reg32 lowest, reg32 highest)
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
inline void wrapTurn(Guest& guest, reg32 turn, reg32 lowest)
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
x86::reg32 frontend::rendd::frontWheelTurn(App* app, CPU& cpu, reg32 car, reg32 out, reg32 steered)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x80);
    constexpr reg32 kSpeedScale = 0x54004c;    // a float
    constexpr reg32 kSpinScale = 0x540050;     // a double
    constexpr reg32 kLowest = 0x540058;        // a double, -0.5
    constexpr reg32 kHighest = 0x540060;       // a float, 0.5
    constexpr reg32 kPerFrame = 0x540068;      // a double
    constexpr reg32 kWrap = 0x540070;          // a double, -1
    constexpr reg32 kSteerScale = 0x540078;    // a double
    const reg32 rotation = guest.frame();
    const reg32 steering = rotation + 0x24;
    const reg32 turn = car + 0x938;
    const double speed = fp::mul(floatOf(guest, car + 0x4f4), floatOf(guest, kSpeedScale));
    const reg32 stamp = carWheelStamp(app, car, 0u);
    const double spin = fp::mul(double(sreg32(guest.dword(car + 0x598))), doubleOf(guest, kSpinScale));
    const reg32 frames = guest.dword(kFrameCount) - stamp;
    const float cut = cutToHalf(guest, fp::add(speed, spin), kLowest, kHighest);
    if ((bitsOf(cut) & 0x7fffffff) != 0 && sreg32(frames) > 0 && (guest.byte(car + 0x59c) & 1) == 0)
    {
        const double by = fp::mul(fp::mul(double(sreg32(frames)), doubleOf(guest, kPerFrame)), double(cut));
        guest.setFloat(turn, float(fp::add(by, floatOf(guest, turn))));
    }
    wrapTurn(guest, turn, kWrap);
    const reg32 rotated = *fe::direct<0x4dfd80, ea::rotateAboutX>(app, cpu, rotation, turn);
    if (steered == 0)
    {
        copyDwords(guest, out, rotation, 9);
        return rotated;
    }
    const reg32 steer = guest.dword(car + 0x51c);
    if (steer == 0)
    {
        copyDwords(guest, out, rotation, 9);
        return 0;
    }
    const double angle = fp::mul(double(sreg32(steer * 36)), doubleOf(guest, kSteerScale));
    guest.leaveEbx(out);
    guest.call(kRotationY, args({ steering, steering }, { bitsOf(float(angle)) }));
    return guest.call(kMultiplyMatrices, args({ rotation }));
}

void frontend::rendFrontWheelTurn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b9140", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = cpu.eax, out = cpu.edx, steered = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::frontWheelTurn(app, cpu, car, out, steered); });
}

/* sub_4b9280: a rear wheel's matrix of car eax to edx: only turned, and less when it is
 * holding on (a car's +0x50d). */
x86::reg32 frontend::rendd::rearWheelTurn(App* app, CPU& cpu, reg32 car, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x40);
    constexpr reg32 kSpeedScale = 0x540080;    // a float
    constexpr reg32 kSpinScale = 0x540088;     // a double
    constexpr reg32 kLowest = 0x540090;        // a double, -0.5
    constexpr reg32 kHighest = 0x540098;       // a float, 0.5
    constexpr reg32 kHoldLowest = 0x5400a0;    // a double, -0.05
    constexpr reg32 kHoldHighest = 0x5400a8;   // a float, 0.05
    constexpr reg32 kPerFrame = 0x5400b0;      // a double
    constexpr reg32 kWrap = 0x5400b8;          // a double, -1
    const reg32 turn = car + 0x934;
    const double speed = fp::mul(floatOf(guest, car + 0x4f4), floatOf(guest, kSpeedScale));
    const reg32 stamp = carWheelStamp(app, car, 1u);
    const double spin = fp::mul(double(sreg32(guest.dword(car + 0x594))), doubleOf(guest, kSpinScale));
    const reg32 frames = guest.dword(kFrameCount) - stamp;
    const float cut = cutToHalf(guest, fp::add(speed, spin), kLowest, kHighest);
    float by = cut;
    if (guest.byte(car + 0x50d) != 0)
    {
        if (isBelow(fp::compare(double(cut), doubleOf(guest, kHoldLowest))))
            by = floatFrom(0xbd4ccccd);
        else if (isAbove(fp::compare(double(cut), floatOf(guest, kHoldHighest))))
            by = floatFrom(0x3d4ccccd);
    }
    if ((bitsOf(by) & 0x7fffffff) != 0 && sreg32(frames) > 0 && (guest.byte(car + 0x59c) & 2) == 0)
    {
        const double step = fp::mul(fp::mul(double(sreg32(frames)), doubleOf(guest, kPerFrame)), double(by));
        guest.setFloat(turn, float(fp::add(step, floatOf(guest, turn))));
    }
    wrapTurn(guest, turn, kWrap);
    return *fe::direct<0x4dfd80, ea::rotateAboutX>(app, cpu, out, turn);
}

void frontend::rendRearWheelTurn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b9280", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const reg32 car = cpu.eax, out = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::rearWheelTurn(app, cpu, car, out); });
}

/* sub_4b93c0: the body of car eax leaning, its matrix to edx: its two leans (+0x920, +0x924)
 * cut to their ends; edx turned by 0x564498 too, and ebx given the turn before, when the car
 * is rolling over (+0x10c). */
x86::reg32 frontend::rendd::bodyLean(App* app, CPU& cpu, reg32 car, reg32 out, reg32 before)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x98);
    constexpr reg32 kAirborne = 0x5400c0;      // a double
    constexpr reg32 kPitchHigh = 0x5400c8;     // a double, 0.25
    constexpr reg32 kPitchLow = 0x5400d0;      // a double, -0.25
    constexpr reg32 kRollHigh = 0x5400d8;      // a double, 0.3
    constexpr reg32 kRollLow = 0x5400e0;       // a double, -0.3
    constexpr reg32 kRollOver = 0x5400e8;      // a double
    constexpr reg32 kRollMatrix = 0x564498;
    const reg32 yawed = guest.frame();
    const reg32 pitched = yawed + 0x24;
    const reg32 pitch = car + 0x920, roll = car + 0x924;
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
    guest.call(kRotationZ, args({ yawed, pitched }, { guest.dword(roll) }));
    guest.leaveEbx(out);
    fe::direct<0x4dfd80, ea::rotateAboutX>(app, cpu, pitched, pitch);
    const reg32 multiplied = guest.call(kMultiplyMatrices, args({ yawed }));
    const x86::reg16 fsw = fp::compare(floatOf(guest, car + 0x10c), doubleOf(guest, kRollOver));
    if (!isBelow(fsw))
        return (multiplied & 0xffff0000) | fsw;
    const reg32 turned = guest.call(kMultiplyMatrices, args({ out, before, before }));
    copyDwords(guest, out, kRollMatrix, 9);
    return turned;
}

void frontend::rendBodyLean(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b93c0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = cpu.eax, out = cpu.edx, before = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::bodyLean(app, cpu, car, out, before); });
}

/* sub_4b9550: colour edx lit by the track's lights around car eax: what each light within
 * reach of the seven segments around it adds, its average on top of the colour's channels. */
x86::reg32 frontend::rendd::trackLitColour(App* app, CPU& cpu, reg32 car, reg32 colour)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x4c);
    constexpr reg32 kToFixed = 0x5400f0;       // a double
    constexpr sreg32 kLightReach = 0x100000;
    const reg32 fixedPlace = guest.frame();
    const double toFixed = doubleOf(guest, kToFixed);
    guest.setDword(fixedPlace, reg32(fistp(fp::mul(floatOf(guest, car + kCarPlace), toFixed))));
    guest.setDword(fixedPlace + 4, reg32(fistp(fp::mul(floatOf(guest, car + kCarPlace + 4), toFixed))));
    guest.setDword(fixedPlace + 8, reg32(fistp(fp::mul(floatOf(guest, car + kCarPlace + 8), toFixed))));
    reg32 sumRed = 0, sumGreen = 0, sumBlue = 0, seen = 0;
    for (sreg32 near_ = -3; near_ < 4; ++near_)
    {
        reg32 segment = guest.dword(car + kCarSegment) + reg32(near_);
        if (near_ < 0)
        {
            if (sreg32(segment) < 0)
                segment += guest.dword(kSegmentCount);
        }
        else if (sreg32(segment) >= sreg32(guest.dword(kSegmentCount)))
            segment -= guest.dword(kSegmentCount);
        const reg32 block = kSegmentLights + reg32(sreg32(segment) >> 3) * 0x5c0;
        const reg32 count = guest.dword(block + 0x5b8);
        guest.leaveEdx(count);
        if (count == 0)
            continue;
        reg32 light = guest.dword(block + 0x5bc);
        for (reg32 left = count; left-- != 0; light += 0x10)
        {
            guest.leaveEdx(light);
            const reg32 lit = fe::direct<0x491120, frontend::rendc::blinkColour>(app, cpu, reg32(guest.word(light + 0xc)));
            if (lit == 0)
                continue;
            // The light's place, in edx since before the call above.
            const sreg32 away =
                sreg32(fe::direct<0x496550, frontend::rendc::approximateDistance>(app, cpu, fixedPlace, light));
            if (away >= kLightReach)
                continue;
            const reg32 weight = 0x10000 - reg32(away >> 4);
            const reg32 red = reg32(sreg32(weight * reg32((sreg32(lit) >> 16) & 0xff)) >> 16);
            const reg32 green = reg32(sreg32(weight * reg32((sreg32(lit) >> 8) & 0xff)) >> 16);
            const reg32 blue = reg32(sreg32((lit & 0xff) * weight) >> 16);
            sumRed += red;
            sumBlue += blue;
            sumGreen += green;
            guest.leaveEdx(green);
            ++seen;
        }
    }
    if (seen == 0)
        return colour;
    const auto channel = [](reg32 from, reg32 sum, reg32 count) -> reg32 {
        const sreg32 value = sreg32((from & 0xff) + reg32(sreg32(sum) / sreg32(count)));
        return value < 0 ? 0u : value > 0xff ? 0xffu : reg32(value);
    };
    const reg32 red = channel(reg32(sreg32(colour) >> 16), sumRed, seen);
    const reg32 green = channel(reg32(sreg32(colour) >> 8), sumGreen, seen);
    const reg32 blue = channel(colour, sumBlue, seen);
    guest.leaveEdx(red);
    return 0xff000000 | ((red & 0xff) << 16) | ((green & 0xff) << 8) | (blue & 0xff);
}

void frontend::rendTrackLitColour(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b9550", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = cpu.eax, colour = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::trackLitColour(app, cpu, car, colour); });
}

// ---------------------------------------------------------------------------
// The replay's pose
// ---------------------------------------------------------------------------

/* sub_4b9df0: the recorded car (0x55fe24's model, sub_477d90's pose) in view eax beside car
 * edx, the player's, as its ghost, unless it stands in the car; at the detail its distance
 * gives by the setting, with its own batch of models. */
x86::reg32 frontend::rendd::replayPose(App* app, CPU& cpu, reg32 view, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x9c);
    constexpr reg32 kFromFixed = 0x540100;     // a double, 1/65536
    constexpr reg32 kNearCar = 0x540108;       // a float: nearer the car than this and it stands in it
    constexpr reg32 kNearAxis = 0x540110;      // a double: and over its axes
    const reg32 frame = guest.frame();
    const reg32 fixedAxes = frame;             // 9 fixed point, as recorded
    const reg32 axes = frame + 0x24;           // 9 floats
    const reg32 fromCamera = frame + 0x48;
    const reg32 fixedPlace = frame + 0x54;     // 3 fixed point
    const reg32 place = frame + 0x60;          // 3 floats
    const reg32 list = frame + 0x6c;
    reg32 lightOn = guest.dword(guest.dword(kRenderState) + 4);
    const reg32 got = fe::direct<0x477d90, frontend::loopa::playbackPose>(app, cpu, fixedPlace, fixedAxes);
    if (got == 0)
        return got;
    guest.leaveEdx(0);
    const reg32 model = guest.dword(kReplayModel);
    if (model == 0 || sreg32(guest.dword(model + kModelParts)) <= 0)
        return got;
    const double fromFixed = doubleOf(guest, kFromFixed);
    for (reg32 i = 0; i < 9; ++i)
        guest.setFloat(axes + 4 * i, float(fp::mul(double(sreg32(guest.dword(fixedAxes + 4 * i))), fromFixed)));
    for (reg32 i = 0; i < 3; ++i)
        guest.setFloat(place + 4 * i, float(fp::mul(double(sreg32(guest.dword(fixedPlace + 4 * i))), fromFixed)));

    // Hidden when it stands where the car does, turned as it is.  `measured` is what the last
    // measure left in eax, which the answers below keep the high half of.
    reg32 measured = 0;
    const auto distance = [&](reg32 from, reg32 to) {
        guest.leaveEdx(to);   // as the call by address leaves it
        const frontend::sima::FloatLeft quick = fe::direct<0x4965b0, frontend::sima::quickDistanceOf>(app, cpu, from, to);
        measured = quick.eax;
        return quick.value;
    };
    if (isBelow(fp::compare(distance(car + kCarPlace, place), floatOf(guest, kNearCar)))
        && isBelow(fp::compare(distance(car + kCarAxes, axes), doubleOf(guest, kNearAxis)))
        && isBelow(fp::compare(distance(car + 0xcc, axes + 0xc), doubleOf(guest, kNearAxis))))
    {
        const x86::reg16 fsw = fp::compare(distance(car + kCarForward, axes + 0x18), doubleOf(guest, kNearAxis));
        if (isBelow(fsw))
            return (measured & 0xffff0000) | fsw;
    }
    const float away = float(distance(place, view + 8));
    {
        const x86::reg16 fsw = fp::compare(-floatOf(guest, car + 0x11c), double(away));
        if (isAbove(fsw))
            return (measured & 0xffff0000) | fsw;
    }
    const reg32 distances = gameSettings(guest).detailLevel << 4;
    guest.leaveEdx(distances);
    {
        const x86::reg16 fsw = fp::compare(double(away), floatOf(guest, kDetailDistances + distances + 0xc));
        if (isAbove(fsw))
            return (measured & 0xffff0000) | fsw;
    }
    reg32 detail = 0;
    if (isAbove(fp::compare(double(away), floatOf(guest, kDetailDistances + distances))))
        detail = 1;
    if (isAbove(fp::compare(double(away), floatOf(guest, kDetailDistances + distances + 4))))
        detail = 2;
    if (isAbove(fp::compare(double(away), floatOf(guest, kDetailDistances + distances + 8))))
        detail = 3;

    fe::direct<0x49c410, frontend::rendc::objectsBegin>(app, cpu);
    guest.call(kEndModelBatch);
    fe::direct<0x49d750, frontend::rendc::modelSetShading>(app, cpu, gameSettings(guest).useSpecular != 0 ? 1u : 0u);
    fe::direct<0x49d760, frontend::rendc::modelSetLight>(app, cpu, kSun);
    // edx too, as the game's code hands it: sub_49d800 below reads it.
    guest.leaveEdx(0xffffffff);
    fe::direct<0x49d8f0, frontend::rendc::modelSetColour>(app, cpu, 0xffffffffu);
    guest.call(kSetSpecular, args({ 0x7f7f7f7f }));
    fe::direct<0x49d720, frontend::rendc::modelSetAlphas>(app, cpu, 0x8fu, 0x4fu);
    if (gameSettings(guest).useLights == 0)
        lightOn = 0;
    else if (lightOn != 0)
    {
        fe::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, view + 8, place, fromCamera);
        fe::direct<0x49d780, frontend::rendc::modelSetEnvironment>(app, cpu, fromCamera);
    }

    const auto part = [&](reg32 which, reg32 offset, reg32 flag) {
        fe::direct<0x49de20, frontend::rendc::modelTransformPart>(app, cpu, model, which, 0u, model + offset, axes, place,
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
    guest.setEsi(list);
    fe::direct<0x49d720, frontend::rendc::modelSetAlphas>(app, cpu, 0xffu, 0x7fu);
    fe::direct<0x49c470, frontend::rendc::objectsEnd>(app, cpu, list);
    if (guest.dword(kDriverOnCard) == 0)
    {
        const reg32 head = guest.dword(list), tail = guest.dword(list + 4);
        guest.setEsi(list);
        guest::direct<0x4c7380, rende::polygonsSorted>(app, cpu, list, head, tail);
    }
    fe::direct<0x4e0430, vertices::transformPointList>(app, cpu, 1, place, view + 0x44, view + 0x38, place);
    const double depth = fp::sub(floatOf(guest, place + 8), floatOf(guest, kDepthBias));
    guest.leaveEdx(list);   // as the call by address leaves it
    return fe::direct<0x49c420, frontend::rendc::objectsDraw>(app, cpu, view, list, bitsOf(float(depth)));
}

void frontend::rendReplayPose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b9df0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 view = cpu.eax, car = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::replayPose(app, cpu, view, car); });
}

// ---------------------------------------------------------------------------
// One car
// ---------------------------------------------------------------------------

/* sub_4ba320: the state models draw in: the ambient by the setting, the sun, the outline's
 * colours ebx and edx, as the call hands it them (the car). */
x86::reg32 frontend::rendd::carStates(App* app, CPU& cpu, reg32 view, reg32 car, reg32 outline)
{
    static_cast<void>(view);   // in eax, not read
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 4);
    fe::direct<0x49d750, frontend::rendc::modelSetShading>(app, cpu, gameSettings(guest).useSpecular != 0 ? 1u : 0u);
    fe::direct<0x49d760, frontend::rendc::modelSetLight>(app, cpu, kSun);
    // The game's code hands it the car in edx too, which it does not read: left there for the caller.
    guest.leaveEdx(car);
    return fe::direct<0x49d8f0, frontend::rendc::modelSetColour>(app, cpu, outline);
}

void frontend::rendCarStates(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ba320", 0, 0, nullptr, 0, 0 };
    const reg32 view = cpu.eax, car = cpu.edx, outline = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::carStates(app, cpu, view, car, outline); });
}

/* sub_4ba350: the specular colours of car edx's paint, ebx, in view eax, and the light's flag
 * back: nothing when view is a mirror's (*eax 1) or the lights are off, and less the longer a
 * tunnel it is in. */
x86::reg32 frontend::rendd::carShade(App* app, CPU& cpu, reg32 view, reg32 car, reg32 paint)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x94);
    const auto over256 = [](reg32 value) { return reg32((sreg32(value) + (sreg32(value) < 0 ? 0xff : 0)) >> 8); };
    const auto over512 = [](reg32 value) { return reg32((sreg32(value) + (sreg32(value) < 0 ? 0x1ff : 0)) >> 9); };
    const auto cutByte = [](reg32 value) -> reg32 { return sreg32(value) < 0 ? 0u : sreg32(value) > 0xff ? 0xffu : value; };
    const auto cutBright = [](reg32 value) -> reg32 { return sreg32(value) < 0x7f ? 0x7fu : sreg32(value) > 0xff ? 0xffu : value; };
    const reg32 frame = guest.frame();
    const reg32 paintRgb = frame;              // red, green, blue, in dwords
    const reg32 mixedHsv = frame + 0x10;
    const reg32 paintHsv = frame + 0x20;
    const reg32 hsvIn = frame + 0x30;
    const reg32 mixedRgb = frame + 0x40;       // the ambient's, then with the paint's
    const reg32 rgbOut = frame + 0x50;
    const reg32 fromLight = frame + 0x60;
    const reg32 segment = guest.dword(car + kCarSegment);
    const reg32 place = car + kCarPlace;
    const reg32 flag = guest.dword(guest.dword(kRenderState) + 4);
    guest.leaveEdx(0x100);
    cpu.ebx = guest.dword(view);
    if (cpu.ebx == 1)
        return 0;
    if (gameSettings(guest).useLights == 0)
        return 0;
    reg32 specular = 0x100, ambientShare = 0x100;
    const auto inTunnel = [&](reg32 at) {
        return *fe::direct<0x420bf0, trackdata::placeInTunnel>(app, cpu, place, at) != 0;
    };
    if (inTunnel(segment))
    {
        reg32 reach = 1;
        for (;;)
        {
            reg32 ahead = segment + reach;
            const reg32 count = guest.dword(kSegmentCount);
            if (sreg32(ahead) >= sreg32(count))
                ahead -= count;
            reg32 behind = segment - reach;
            if (sreg32(behind) < 0)
                behind += guest.dword(kSegmentCount);
            if (!inTunnel(ahead) || !inTunnel(behind))
                break;
            if (++reach >= 8)
                break;
        }
        specular = 0x100 - (reach << 4);
        ambientShare = 0x100 - (reach << 5);
    }
    fe::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, view + 8, place, fromLight);
    fe::direct<0x49d780, frontend::rendc::modelSetEnvironment>(app, cpu, fromLight);
    const reg32 red = reg32(sreg32(paint) >> 16) & 0xff, green = reg32(sreg32(paint) >> 8) & 0xff, blue = paint & 0xff;
    guest.setDword(paintRgb, red);
    guest.setDword(paintRgb + 4, green);
    guest.setDword(paintRgb + 8, blue);
    guest.call(kRgbToHsv, args({ paintRgb, paintHsv }));
    guest.setDword(mixedRgb, guest.byte(kAmbientBlue + 2));
    guest.setDword(mixedRgb + 4, guest.byte(kAmbientBlue + 1));
    guest.setDword(mixedRgb + 8, guest.byte(kAmbientBlue));
    if (inTunnel(segment))
    {
        const reg32 paintShare = 0x100 - ambientShare;
        const auto mix = [&](reg32 own, reg32 ambient) {
            const reg32 fromPaint = over256(own * paintShare);
            return over256(ambient * ambientShare) + fromPaint;
        };
        guest.setDword(mixedRgb, mix(red, guest.dword(mixedRgb)));
        guest.setDword(mixedRgb + 4, mix(green, guest.dword(mixedRgb + 4)));
        guest.setDword(mixedRgb + 8, mix(blue, guest.dword(mixedRgb + 8)));
    }
    guest.call(kRgbToHsv, args({ mixedRgb, mixedHsv }));
    const reg32 hue = guest.dword(mixedHsv), saturation = guest.dword(mixedHsv + 4), value = guest.dword(mixedHsv + 8);
    const reg32 paintValue = guest.dword(paintHsv + 8);
    const reg32 wear = guest.dword(car + 0x8a8 + 8);
    guest.setDword(hsvIn, cutByte(hue));
    guest.setDword(hsvIn + 4, cutByte(over256(saturation * 96)));
    const reg32 first = cutBright(over512(value * paintValue) + 0x80 - reg32(sreg32(wear) >> 2));
    guest.setDword(hsvIn + 8, first);
    cpu.ebx = first;
    guest.call(kHsvToRgb, args({ hsvIn, rgbOut }));
    const reg32 colourOne = ((first & 0xff) << 24) | ((guest.dword(rgbOut) & 0xff) << 16) | ((guest.dword(rgbOut + 4) & 0xff) << 8)
                            | (guest.dword(rgbOut + 8) & 0xff);
    guest.setDword(hsvIn, cutByte(hue));
    guest.setDword(hsvIn + 4, cutByte(reg32(sreg32(saturation) >> 1)));
    const reg32 second = cutBright(over512(value * paintValue) + 0x80);
    guest.setDword(hsvIn + 8, second);
    cpu.ebx = second;
    guest.call(kHsvToRgb, args({ hsvIn, rgbOut }));
    const reg32 colourTwo = ((second & 0xff) << 24) | ((guest.dword(rgbOut) & 0xff) << 16) | ((guest.dword(rgbOut + 4) & 0xff) << 8)
                            | (guest.dword(rgbOut + 8) & 0xff);
    const reg32 factor = specular << 8;
    const reg32 scaledOne = scaledColour(cpu, colourOne, factor);
    const reg32 scaledTwo = scaledColour(cpu, colourTwo, factor);
    guest.call(kSetSpecular, args({ scaledOne, scaledTwo }));
    return flag;
}

void frontend::rendCarShade(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ba350", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 view = cpu.eax, car = cpu.edx, paint = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::carShade(app, cpu, view, car, paint); });
}

/* sub_4ba810: car eax's colour, the average of the road's at its four wheels, the place's
 * own for those not on it. */
x86::reg32 frontend::rendd::carAverageColour(App* app, CPU& cpu, reg32 car)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x20);
    const reg32 own = fe::direct<0x4206f0, trackdata::placeColour>(app, cpu, car + 0xc, 1u);
    const auto wheel = [&](reg32 place) -> reg32 {
        if (guest.dword(car + place + 0x14) == 0 || guest.dword(car + place + 0x18) == 0)
            return own;
        return fe::direct<0x4206f0, trackdata::placeColour>(app, cpu, car + place, 1u);
    };
    const reg32 a = wheel(0x300);
    const reg32 b = wheel(0x378);
    const reg32 c = wheel(0x3f0);
    const reg32 d = wheel(0x468);
    const auto average = [&](unsigned shift) {
        const reg32 sum = ((a >> shift) & 0xff) + ((b >> shift) & 0xff) + ((c >> shift) & 0xff) + ((d >> shift) & 0xff);
        return reg32(sreg32(sum) >> 2) & 0xff;
    };
    return 0xff000000 | (average(16) << 16) | (average(8) << 8) | average(0);
}

void frontend::rendCarAverageColour(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ba810", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 car = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::carAverageColour(app, cpu, car); });
}

namespace
{

/* A car's axes to `matrix` in the frame, its place to `place`, for a part to draw it by: the
 * height raised by the float at `raise`, as each detail's own has. */
inline void carFrame(Guest& guest, reg32 car, reg32 matrix, reg32 place, reg32 raise)
{
    copyDwords(guest, matrix, car + kCarAxes, 9);
    copyDwords(guest, place, car + kCarPlace, 3);
    guest.setFloat(place + 4, float(fp::add(floatOf(guest, place + 4), floatOf(guest, raise))));
}

/* sub_49de20 called as the game's code calls it, `arguments` in the registers and pushed: sub_4b8740
 * after it reads what is left in its frame, which a C++ call (fe::direct), pushing none of the five
 * stack arguments, would leave 20 bytes up -- and the registers it leaves.  Its depth back.  In the
 * game the call made as Guest::call makes it, rendc's modelTransformPartCalled in place of the jump
 * by address, which leaves everything as the call does; in the checks the call by address itself,
 * the depth taken off the x87's stack. */
double partByAddress(Guest& guest, const fe::Arguments& arguments)
{
    x86::CPU& cpu = guest.cpu();
#ifdef NFS_FRONTEND_CHECKS
    guest.call(kDrawModelPart, arguments);
    return double(cpu.fpu.take());
#else
    reg32* const registers[4] = { &cpu.eax, &cpu.edx, &cpu.ebx, &cpu.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *registers[i] = arguments.registers[i];
    const reg32 esp = cpu.esp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        cpu.esp -= 4;
        guest.setDword(cpu.esp, arguments.stack[i]);
    }
    // The return address's slot, left unwritten as the generated code leaves it.
    cpu.esp -= 4;
    const std::optional<double> depth = frontend::rendc::modelTransformPartCalled(guest.app(), cpu);
    if (!depth)
        throw fe::Terminated();
    cpu.esp = esp;
    return *depth;
#endif
}

/* The whole car as one part of its model; its depth, what sub_4b8740 sizes by, kept.  By address
 * (partByAddress). */
inline reg32 drawWhole(Guest& guest, reg32 model, reg32 view, reg32 part, reg32 offset, reg32 matrix, reg32 place, reg32 flag)
{
    return bitsOf(float(partByAddress(guest, args({ model, part, 0, model + offset },
                                                  { matrix, place, view + 0x44, view + 0x38, flag }))));
}

}

/* sub_4ba960: a car at its least detail: its whole in one, and its lights. */
x86::reg32 frontend::rendd::carFarthest(App* app, CPU& cpu, reg32 car, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x58);
    constexpr reg32 kRaise = 0x540118;
    const reg32 model = guest.dword(car + kCarModel);
    const reg32 matrix = guest.frame(), place = matrix + 0x24;
    const reg32 colour = fe::direct<0x4206f0, trackdata::placeColour>(app, cpu, car + 0xc, 1u);
    guest.leaveEdx(model);
    if (model == 0 || sreg32(guest.dword(model + kModelParts)) <= 0xb)
        return colour;
    guest.leaveEbx(colour);
    guest::direct<0x4ba320, carStates>(app, cpu, view, car, colour);
    carFrame(guest, car, matrix, place, kRaise);
    const reg32 size = drawWhole(guest, model, view, 0xb, 0x180, matrix, place, 0);
    // By address: sub_4b8740 reads what is left in its frame from before (an unknown light's size,
    // colour, facing), which a C++ call, pushing no arguments, would move 8 bytes up.
    return guest.call(kCarLights, args({ view, car, matrix, place }, { size, colour }));
}

void frontend::rendCarFarthest(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ba960", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = cpu.eax, view = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::carFarthest(app, cpu, car, view); });
}

/* sub_4baa50: a car far away: its whole, its lights and its shadow. */
x86::reg32 frontend::rendd::carFar(App* app, CPU& cpu, reg32 car, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x5c);
    constexpr reg32 kRaise = 0x54011c;
    const reg32 model = guest.dword(car + kCarModel);
    const reg32 matrix = guest.frame(), place = matrix + 0x24;
    reg32 colour = fe::direct<0x4206f0, trackdata::placeColour>(app, cpu, car + 0xc, 1u);
    guest.leaveEdx(model);
    if (model == 0 || sreg32(guest.dword(model + kModelParts)) <= 0xa)
        return colour;
    if (gameSettings(guest).night != 0)
        colour = guest.call(kNightColour, args({ car, colour }));
    guest.leaveEbx(colour);
    const reg32 shade = guest::direct<0x4ba350, carShade>(app, cpu, view, car, colour);
    guest.leaveEbx(colour);
    guest::direct<0x4ba320, carStates>(app, cpu, view, car, colour);
    carFrame(guest, car, matrix, place, kRaise);
    const reg32 size = drawWhole(guest, model, view, 0xa, 0x174, matrix, place, shade);
    // By address: sub_4b8740 reads what is left in its frame from before (an unknown light's size,
    // colour, facing), which a C++ call, pushing no arguments, would move 8 bytes up.
    guest.call(kCarLights, args({ view, car, matrix, place }, { size, colour }));
    return guest::direct<0x4b81a0, carShadow>(app, cpu, car, view);
}

void frontend::rendCarFar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4baa50", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = cpu.eax, view = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::carFar(app, cpu, car, view); });
}

/* sub_4bab70: a car near: body, a police bar if it has one, four wheels each of its own
 * turn, then the lights by its lean, and the shadow. */
x86::reg32 frontend::rendd::carNear(App* app, CPU& cpu, reg32 car, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0xc8);
    constexpr reg32 kRaise = 0x540120;
    constexpr reg32 kWheelsHidden = 0x99c;     // of a car: a bit for each wheel
    const reg32 frame = guest.frame();
    const reg32 axes = frame;
    const reg32 lean = frame + 0x24;
    const reg32 wheel = frame + 0x48;
    const reg32 bodyTurn = frame + 0x6c;
    const reg32 place = frame + 0x90;
    const reg32 model = guest.dword(car + kCarModel);
    guest.leaveEdx(model);
    reg32 colour = guest::direct<0x4ba810, carAverageColour>(app, cpu, car);
    if (model == 0 || sreg32(guest.dword(model + kModelParts)) <= 5)
        return colour;
    if (gameSettings(guest).night != 0)
        colour = guest.call(kNightColour, args({ car, colour }));
    guest.leaveEbx(colour);
    guest::direct<0x4ba320, carStates>(app, cpu, view, car, colour);
    guest.leaveEbx(colour);
    const reg32 shade = guest::direct<0x4ba350, carShade>(app, cpu, view, car, colour);
    carFrame(guest, car, axes, place, kRaise);
    guest::direct<0x4b93c0, bodyLean>(app, cpu, car, lean, axes);
    // By address (partByAddress): sub_4b8740 at the end reads what is left in its frame, which a
    // C++ call, pushing none of the five stack arguments, would leave 20 bytes up.
    const auto part = [&](reg32 which, reg32 offset, reg32 turn, reg32 flag) {
        return partByAddress(guest, args({ model, which, turn, model + offset }, { axes, place, view + 0x44, view + 0x38, flag }));
    };
    const reg32 size = bitsOf(float(part(5, 0x138, lean, shade)));
    if (guest.dword(car + kCarLightsOn) != 0)
        part(0xc, 0x18c, lean, shade);
    guest::direct<0x4b9140, frontWheelTurn>(app, cpu, car, wheel, 0u);
    if ((guest.byte(car + kWheelsHidden) & 1) == 0)
        part(6, 0x144, wheel, 0);
    if ((guest.byte(car + kWheelsHidden) & 2) == 0)
        part(7, 0x150, wheel, 0);
    guest::direct<0x4b9280, rearWheelTurn>(app, cpu, car, wheel);
    if ((guest.byte(car + kWheelsHidden) & 8) == 0)
        part(8, 0x15c, wheel, 0);
    if ((guest.byte(car + kWheelsHidden) & 4) == 0)
        part(9, 0x168, wheel, 0);
    // The game's code hands it the place in ecx too, which the lights take.
    fe::direct<0x4e0280, vertices::multiplyMatrix>(app, cpu, lean, axes, bodyTurn);
    // By address: sub_4b8740 reads what is left in its frame from before (an unknown light's size,
    // colour, facing), which a C++ call, pushing no arguments, would move 8 bytes up.
    // The place in ecx, as the call before leaves it.
    guest.call(kCarLights, args({ view, car, bodyTurn, place }, { size, colour }));
    return guest::direct<0x4b81a0, carShadow>(app, cpu, car, view);
}

void frontend::rendCarNear(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bab70", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = cpu.eax, view = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::carNear(app, cpu, car, view); });
}

/* sub_4bb110: traffic's car at its nearest: one part, its lights and its shadow. */
x86::reg32 frontend::rendd::carSimpleNear(App* app, CPU& cpu, reg32 car, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x58);
    constexpr reg32 kRaise = 0x540128;
    const reg32 model = guest.dword(car + kCarModel);
    const reg32 matrix = guest.frame(), place = matrix + 0x24;
    const reg32 colour = fe::direct<0x4206f0, trackdata::placeColour>(app, cpu, car + 0xc, 1u);
    guest.leaveEdx(model);
    if (model == 0 || sreg32(guest.dword(model + kModelParts)) <= 5)
        return colour;
    guest.leaveEbx(colour);
    guest::direct<0x4ba320, carStates>(app, cpu, view, car, colour);
    carFrame(guest, car, matrix, place, kRaise);
    const reg32 size = drawWhole(guest, model, view, 5, 0x180, matrix, place, 0);
    // By address: sub_4b8740 reads what is left in its frame from before (an unknown light's size,
    // colour, facing), which a C++ call, pushing no arguments, would move 8 bytes up.
    guest.call(kCarLights, args({ view, car, matrix, place }, { size, colour }));
    return guest::direct<0x4b81a0, carShadow>(app, cpu, car, view);
}

void frontend::rendCarSimpleNear(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bb110", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = cpu.eax, view = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::carSimpleNear(app, cpu, car, view); });
}

/* sub_4bb4c0: car eax in view edx, in its own batch of models, the detail ebx brought down
 * to what its model has: a detail's draw for a full model, or the simple ones'. */
x86::reg32 frontend::rendd::drawCar(App* app, CPU& cpu, reg32 car, reg32 view, reg32 detail)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x18);
    const reg32 list = guest.frame();
    const reg32 model = guest.dword(car + kCarModel);
    cpu.ebx = model;
    guest.leaveEdx(detail);
    fe::direct<0x49c410, frontend::rendc::objectsBegin>(app, cpu);
    guest.call(kEndModelBatch);
    const auto hasParts = [&](sreg32 parts) { return model != 0 && sreg32(guest.dword(model + kModelParts)) > parts; };
    if (detail == 3 && !hasParts(0xb))
        detail = 2;
    if (detail == 2 && !hasParts(0xa))
        detail = 1;
    if (detail == 1 && !hasParts(5))
        detail = 0;
    guest.leaveEdx(detail);
    if (detail <= 3)
    {
        if (hasParts(0xb))
        {
            static const reg32 kFull[] = { kDrawCarDetail, kCarNear, kCarFar, kCarFarthest };
            guest.call(kFull[detail], args({ car, view }));
        }
        else if (detail == 3)
            guest::direct<0x4bb110, carSimpleNear>(app, cpu, car, view);
        else
            guest.call(kDrawTrafficDetail, args({ car, view }));
    }
    guest.setEsi(list);
    fe::direct<0x49c470, frontend::rendc::objectsEnd>(app, cpu, list);
    if (guest.dword(kDriverOnCard) == 0)
    {
        cpu.ebx = guest.dword(list + 4);
        const reg32 head = guest.dword(list);
        guest.setEsi(list);
        guest::direct<0x4c7380, rende::polygonsSorted>(app, cpu, list, head, cpu.ebx);
    }
    const double depth = fp::sub(floatOf(guest, car + kCarDepth), floatOf(guest, kDepthBias));
    guest.leaveEdx(list);   // as the call by address leaves it
    return fe::direct<0x49c420, frontend::rendc::objectsDraw>(app, cpu, view, list, bitsOf(float(depth)));
}

void frontend::rendDrawCar(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bb4c0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = cpu.eax, view = cpu.edx, detail = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::drawCar(app, cpu, car, view, detail); });
}

// ---------------------------------------------------------------------------
// All the cars
// ---------------------------------------------------------------------------

/* sub_4bba10: how much of the budget the cars' details use: 4 less the detail of each car drawn. */
x86::reg32 frontend::rendd::detailInUse(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    reg32 used = 0;
    for (reg32 i = 0; sreg32(i) < sreg32(guest.dword(kCarCount)); ++i)
    {
        const reg32 detail = guest.dword(guest.dword(kCarTable + 4 * i) + kCarDetail);
        if (sreg32(detail) >= 0)
            used += 4 - detail;
    }
    return used;
}

void frontend::rendDetailInUse(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bba10", 0, 0, nullptr, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::detailInUse(app, cpu); });
}

/* sub_4bba50: the farthest car but the one numbered eax at detail edx brought to detail ebx;
 * 0 when there is none. */
x86::reg32 frontend::rendd::lowerDetail(App* app, CPU& cpu, reg32 keep, reg32 detail, reg32 lowered)
{
    Guest guest(app, cpu);
    reg32 farthest = 0;
    sreg32 found = -1;
    reg32 i = 0;
    for (; sreg32(i) < sreg32(guest.dword(kCarCount)); ++i)
    {
        const reg32 car = guest.dword(kCarTable + 4 * i);
        cpu.ebx = car;
        if (i == keep)
            continue;
        if (!isAbove(fp::compare(floatOf(guest, car + kCarDepth), double(floatFrom(farthest)))))
            continue;
        if (guest.dword(car + kCarDetail) != detail)
            continue;
        farthest = guest.dword(car + kCarDepth);
        found = sreg32(i);
    }
    guest.leaveEdx(i);
    if (found < 0)
        return 0;
    guest.setDword(guest.dword(kCarTable + 4 * reg32(found)) + kCarDetail, lowered);
    return 1;
}

void frontend::rendLowerDetail(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bba50", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 keep = cpu.eax, detail = cpu.edx, lowered = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::lowerDetail(app, cpu, keep, detail, lowered); });
}

/* sub_4bbad0: the cars' details cut to what the setting's budget allows, the camera eax's own
 * car spared; with a car asking for the second view, every such car's by the setting. */
x86::reg32 frontend::rendd::fitDetailBudget(App* app, CPU& cpu, reg32 camera)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x24);
    constexpr reg32 kWantsBothViews = 0x9a4;
    const reg32 ownCar = fe::direct<0x422ba0, carstate::carOfCamera>(app, cpu, guest.dword(camera + 4));
    reg32 own = cpu.ecx;
    for (reg32 i = 0; sreg32(i) < sreg32(guest.dword(kCarCount)); ++i)
    {
        const reg32 car = guest.dword(kCarTable + 4 * i);
        if (fe::direct<0x422ba0, carstate::carOfCamera>(app, cpu, guest.dword(camera + 4)) == car)
            own = i;
    }
    reg32 left;   // what the last call leaves in eax
    for (;;)
    {
        const reg32 setting = gameSettings(guest).detailLevel;
        guest.leaveEdx(setting);
        const reg32 used = guest::direct<0x4bba10, detailInUse>(app, cpu);
        left = used;
        if (sreg32(used) <= sreg32(guest.dword(kDetailBudget + 4 * setting)))
            break;
        guest.leaveEbx(2);
        left = guest::direct<0x4bba50, lowerDetail>(app, cpu, own, 1u, 2u);
        if (left != 0)
            continue;
        guest.leaveEbx(1);
        left = guest::direct<0x4bba50, lowerDetail>(app, cpu, own, 0u, 1u);
        if (left == 0)
            break;
    }
    if (guest.dword(ownCar + kWantsBothViews) == 0)
        return left;
    reg32 i = 0;
    for (; sreg32(i) < sreg32(guest.dword(kCarCount)); ++i)
    {
        const reg32 car = guest.dword(kCarTable + 4 * i);
        if (guest.dword(car + kWantsBothViews) != 0)
            guest.setDword(car + kCarDetail, gameSettings(guest).detailLevel != 0 ? 1 : 0);
    }
    return i;
}

void frontend::rendFitDetailBudget(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bbad0", 0, 0, nullptr, 0 };
    const reg32 camera = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::fitDetailBudget(app, cpu, camera); });
}

/* sub_4bbba0: every car in view eax: each one's detail by its distance (sub_4bb5d0), fitted
 * to the budget, what goes with each, each drawn when it shows, a car of half the frames every
 * other frame; the replay's pose, the particles; and the frame noted as the one wheels moved on. */
x86::reg32 frontend::rendd::cars(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x18);
    constexpr reg32 kEveryOtherFrame = 0x7d36a0;
    for (reg32 i = 0; sreg32(i) < sreg32(guest.dword(kCarCount)); ++i)
    {
        const reg32 car = guest.dword(kCarTable + 4 * i);
        cpu.ecx = car;
        guest.setDword(car + kCarDepth, 0);
        guest.setDword(car + kCarDetail, guest.call(kCarVisible, args({ view, car, car + kCarDepth })));
    }
    guest::direct<0x4bbad0, fitDetailBudget>(app, cpu, view);
    for (reg32 i = 0; sreg32(i) < sreg32(guest.dword(kCarCount)); ++i)
    {
        const reg32 car = guest.dword(kCarTable + 4 * i);
        cpu.ebx = guest.dword(car + kCarDetail);
        if (sreg32(cpu.ebx) <= -1)
            continue;
        if (guest.dword(car + 0x8e4) != 0 && guest.dword(car + 0x8e8) != 0)
            guest.call(kDrawCarExtras, args({ car, view }));
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
        // The detail in ebx, as set above.
        guest::direct<0x4bb4c0, drawCar>(app, cpu, car, view, cpu.ebx);
    }
    if (guest.dword(kReplayShowing) == 1)
        guest::direct<0x4b9df0, replayPose>(app, cpu, view, guest.dword(kPlayerCars + 4 * gameSettings(guest).localPlayer));
    guest::direct<0x4c9f70, rende::blinkingQuads>(app, cpu, view);
    const reg32 now = guest.dword(kFrameCount);
    guest.setDword(kWheelStamp, now);
    return now;
}

void frontend::rendCars(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bbba0", 0, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::cars(app, cpu, view); });
}

// ---------------------------------------------------------------------------
// The frame's arena and its lists
// ---------------------------------------------------------------------------

/* sub_4bbd70: the arena back to its start, as a pass begins. */
x86::reg32 frontend::rendd::arenaRewind(App* app, CPU& cpu)
{
    arenaReset(app, cpu);
    Guest guest(app, cpu);
    const reg32 start = guest.dword(kArena);
    guest.setDword(kArenaNext, start);
    return start;
}

void frontend::rendArenaRewind(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bbd70", 0, 0, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::arenaRewind(app, cpu); });
}

/* sub_4bbd80: the arena made, its next byte aligned to 32, 100 bytes of its size kept back. */
x86::reg32 frontend::rendd::arenaCreate(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    const reg32 block = guest.call(kCreateArena, args({ kArenaSize }));
    guest.setDword(kArena, block);
    const reg32 next = block + 0x1f;
    guest.setDword(kArenaNext, next);
    const std::uint8_t aligned = guest.byte(kArenaNext) & 0xe0;
    guest.setDword(kArenaSize, guest.dword(kArenaSize) - 0x64);
    guest.setByte(kArenaNext, aligned);
    return (next & 0xffff00ff) | (reg32(aligned) << 8);
}

void frontend::rendArenaCreate(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bbd80", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::arenaCreate(app, cpu); });
}

/* sub_4bbdc0: the arena freed. */
x86::reg32 frontend::rendd::arenaFree(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    const reg32 freed = guest.call(kFreeBlock, args({ guest.dword(kArena), 0 }));
    guest.setDword(kArena, guest.edx());
    return freed;
}

void frontend::rendArenaFree(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bbdc0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::arenaFree(app, cpu); });
}

/* sub_4bbde0: edx bytes of the arena to *eax, the next aligned to 32; 1, or 0 and the log's
 * line when it has run out. */
x86::reg32 frontend::rendd::arenaTake(App* app, CPU& cpu, reg32 slot, reg32 bytes)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    constexpr reg32 kOutOfArena = 0x540148;    // "Render_GetTm() raM out of raN\n"
    const reg32 taken = guest.dword(kArenaNext);
    guest.setDword(slot, taken);
    const reg32 past = taken + bytes;
    guest.setDword(kArenaNext, past + 0x1f);
    guest.setByte(kArenaNext, guest.byte(kArenaNext) & 0xe0);
    const reg32 used = guest.dword(kArenaNext) - guest.dword(kArena);
    if (sreg32(used) <= sreg32(guest.dword(kArenaSize)))
        return 1;
    fe::direct<kLog, ea::error>(guest.app(), guest.cpu(), kOutOfArena, guest::Words({}));
    return 0;
}

void frontend::rendArenaTake(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bbde0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 slot = cpu.eax, bytes = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::arenaTake(app, cpu, slot, bytes); });
}

/* sub_4bbe40: where the arena is now, to *eax; 1, or 0 when it has run out. */
x86::reg32 frontend::rendd::arenaHere(App* app, CPU& cpu, reg32 slot)
{
    Guest guest(app, cpu);
    const reg32 here = guest.dword(kArenaNext);
    guest.setDword(slot, here);
    const reg32 start = guest.dword(kArena);
    guest.leaveEdx(start);
    return sreg32(here - start) <= sreg32(guest.dword(kArenaSize)) ? 1 : 0;
}

void frontend::rendArenaHere(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bbe40", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 slot = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::arenaHere(app, cpu, slot); });
}

/* A list is two dwords in the caller's frame: its first record and its last.  These leave esi
 * past their own frame's copy, as their movsd does. */

/* sub_4bbe70: *esi an empty list; esi its own address in eax. */
x86::reg32 frontend::rendd::emptyList(App* app, CPU& cpu, reg32 list)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    guest.setDword(list, 0);
    guest.setDword(list + 4, 0);
    guest.setEsi(guest.frame() + 8);
    return list;
}

void frontend::rendEmptyList(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bbe70", 0, 0, nullptr, 0, fe::kRestoreEdx | fe::kRestoreEdi };
    const reg32 list = cpu.esi;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::emptyList(app, cpu, list); });
}

/* sub_4bbe90: *esi the list of the one record eax, 0 for none. */
x86::reg32 frontend::rendd::listOfOne(App* app, CPU& cpu, reg32 record, reg32 list)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x14);
    if (record != 0)
        guest.setDword(record, 0);
    guest.setDword(list, record);
    guest.setDword(list + 4, record);
    guest.setEsi(guest.frame() + 8);
    return list;
}

void frontend::rendListOfOne(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bbe90", 0, 0, nullptr, 0, fe::kRestoreEdx | fe::kRestoreEdi };
    const reg32 record = cpu.eax, list = cpu.esi;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::listOfOne(app, cpu, record, list); });
}

/* sub_4bbec0: *esi the two lists on the stack joined, an empty one giving the other. */
x86::reg32 frontend::rendd::joinLists(App* app, CPU& cpu, reg32 list, reg32 firstHead, reg32 firstTail, reg32 secondHead, reg32 secondTail)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x18);
    reg32 head = secondHead, tail = secondTail;
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
    guest.setEsi(guest.frame() + 8);
    return list;
}

void frontend::rendJoinLists(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bbec0", 0x10, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx | fe::kRestoreEdi };
    const reg32 firstHead = app->getMemory<reg32>(cpu.esp + 4), firstTail = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 secondHead = app->getMemory<reg32>(cpu.esp + 0xc), secondTail = app->getMemory<reg32>(cpu.esp + 0x10);
    const reg32 list = cpu.esi;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::joinLists(app, cpu, list, firstHead, firstTail, secondHead, secondTail); });
}

/* sub_4bbf20: the open list empty. */
x86::reg32 frontend::rendd::openListStart(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    guest.setDword(kOpenListTail, 0);
    guest.setDword(kOpenListHead, 0);
    return guest.eax();
}

void frontend::rendOpenListStart(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bbf20", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::openListStart(app, cpu); });
}

/* sub_4bbf40: the list on the stack put on the end of the open one. */
x86::reg32 frontend::rendd::openListAppend(App* app, CPU& cpu, reg32 head, reg32 tail)
{
    Guest guest(app, cpu);
    if (head == 0 || tail == 0)
        return guest.eax();
    if (guest.dword(kOpenListHead) == 0)
        guest.setDword(kOpenListHead, head);
    else
        guest.setDword(guest.dword(kOpenListTail), head);
    guest.setDword(kOpenListTail, tail);
    return tail;
}

void frontend::rendOpenListAppend(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bbf40", 8, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 head = app->getMemory<reg32>(cpu.esp + 4), tail = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::openListAppend(app, cpu, head, tail); });
}

/* sub_4bbf80: edx bytes of the arena to *eax, on the end of the open list; 1, or 0 when the
 * arena has run out. */
x86::reg32 frontend::rendd::openListTake(App* app, CPU& cpu, reg32 slot, reg32 bytes)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 8);
    if (guest::direct<0x4bbde0, arenaTake>(app, cpu, slot, bytes) == 0)
        return 0;
    if (guest.dword(kOpenListHead) == 0)
        guest.setDword(kOpenListHead, guest.dword(slot));
    else
    {
        const reg32 tail = guest.dword(kOpenListTail);
        guest.leaveEdx(tail);
        guest.setDword(tail, guest.dword(slot));
    }
    guest.setDword(kOpenListTail, guest.dword(slot));
    return 1;
}

void frontend::rendOpenListTake(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bbf80", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 slot = cpu.eax, bytes = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::openListTake(app, cpu, slot, bytes); });
}

/* sub_4bbfc0: the open list closed, to *esi. */
x86::reg32 frontend::rendd::openListClose(App* app, CPU& cpu, reg32 list)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x10);
    const reg32 head = guest.dword(kOpenListHead), tail = guest.dword(kOpenListTail);
    if (tail != 0)
        guest.setDword(tail, 0);
    guest.setDword(list, head);
    guest.setDword(list + 4, tail);
    guest.setEsi(guest.frame() + 8);
    return list;
}

void frontend::rendOpenListClose(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bbfc0", 0, 0, nullptr, 0, fe::kRestoreEdi };
    const reg32 list = cpu.esi;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::openListClose(app, cpu, list); });
}

/* sub_4bc000: the vertices of mesh eax flushed and listed, the list to *esi. */
x86::reg32 frontend::rendd::flushAndList(App* app, CPU& cpu, reg32 mesh, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x18);
    const reg32 list = guest.frame();
    guest.setEsi(list);
    guest.call(kFlushVertices);
    guest.setEsi(list);
    guest.call(kListVertices, args({ mesh }));
    const reg32 from = cpu.esi;
    guest.setDword(out, guest.dword(from));
    guest.setDword(out + 4, guest.dword(from + 4));
    guest.setEsi(from + 8);
    return out;
}

void frontend::rendFlushAndList(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bc000", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const reg32 mesh = cpu.eax, out = cpu.esi;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::flushAndList(app, cpu, mesh, out); });
}

namespace
{

/* The list emptied through the frame's copy, as the three below end; what the call leaves in eax. */
inline reg32 emptyThroughFrame(win32::WinApplication* app, x86::CPU& cpu, Guest& guest, reg32 list)
{
    const reg32 copy = guest.frame();
    guest.setEsi(copy);
    const reg32 left = guest::direct<0x4bbe70, frontend::rendd::emptyList>(app, cpu, copy);
    guest.setDword(list, guest.dword(copy));
    guest.setDword(list + 4, guest.dword(copy + 4));
    return left;
}

}

/* sub_4bc030: the list edx drawn by sub_4349d0 in view eax, and left empty. */
x86::reg32 frontend::rendd::drawListPlain(App* app, CPU& cpu, reg32 view, reg32 list)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x1c);
    const reg32 tail = guest.dword(list + 4), head = guest.dword(list);
    guest.leaveEcx(tail);
    guest.leaveEbx(head);
    guest.setEsi(guest.frame());
    guest.call(kDrawListSoftware, args({ view }, { head, tail }));
    return emptyThroughFrame(app, cpu, guest, list);
}

void frontend::rendDrawListPlain(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bc030", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 view = cpu.eax, list = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::drawListPlain(app, cpu, view, list); });
}

/* sub_4bc060: the list edx drawn in view eax, by the card's driver at the depth on the stack,
 * and left empty. */
x86::reg32 frontend::rendd::drawListAt(App* app, CPU& cpu, reg32 view, reg32 list, reg32 depth)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x18);
    if (guest.dword(kDriverOnCard) == 0)
        guest.call(kDrawListAtDepth, args({ guest.dword(list) }, { depth }));
    else
    {
        const reg32 head = guest.dword(list), tail = guest.dword(list + 4);
        guest.leaveEbx(tail);
        guest.setEsi(head);
        guest.call(kDrawListSoftware, args({ view }, { head, tail }));
    }
    return emptyThroughFrame(app, cpu, guest, list);
}

void frontend::rendDrawListAt(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bc060", 4, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 depth = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 view = cpu.eax, list = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::drawListAt(app, cpu, view, list, depth); });
}

/* sub_4bc0b0: the list edx drawn in view eax, by the card's driver as a list of the depth on
 * the stack, and left empty. */
x86::reg32 frontend::rendd::drawListMirrored(App* app, CPU& cpu, reg32 view, reg32 list, reg32 depth)
{
    Guest guest(app, cpu);
    const fe::Frame ownFrame(guest, cpu, 0x1c);
    const reg32 head = guest.dword(list), tail = guest.dword(list + 4);
    if (guest.dword(kDriverOnCard) == 0)
    {
        guest.leaveEbx(tail);
        guest.setEsi(head);
        guest::direct<0x4c7400, rende::depthBucketAppend>(app, cpu, view, head, tail, depth);
    }
    else
    {
        guest.leaveEcx(head);
        guest.call(kDrawListSoftware, args({ view }, { head, tail }));
    }
    return emptyThroughFrame(app, cpu, guest, list);
}

void frontend::rendDrawListMirrored(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bc0b0", 4, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 depth = app->getMemory<reg32>(cpu.esp + 4);
    const reg32 view = cpu.eax, list = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::drawListMirrored(app, cpu, view, list, depth); });
}

// ---------------------------------------------------------------------------
// The projection
// ---------------------------------------------------------------------------

/* sub_4bf110: the table of reciprocals' mantissas at 0x7d30d8, 256 of them, each with its
 * exponent's bits set. */
x86::reg32 frontend::rendd::fillReciprocals(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    constexpr reg32 kReciprocals = 0x7d30d8;
    for (reg32 i = 0; i < 0x100; ++i)
    {
        const float value = floatFrom((i << 15) | 0x3f804000);
        const reg32 inverse = bitsOf(float(fp::div(1.0, double(value))));
        guest.setDword(kReciprocals + 4 * i, (inverse & 0x7fffff) | 0x7e800000);
    }
    return 0x100;
}

void frontend::rendFillReciprocals(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bf110", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::fillReciprocals(app, cpu); });
}

/* sub_4bf1b0: the numbers of the clip: the view's left, top, right and bottom, then its two
 * on the stack, near and far; and every vertex of the cache marked unused. */
x86::reg32 frontend::rendd::setClipBounds(App* app, CPU& cpu, reg32 left, reg32 top, reg32 right, reg32 bottom, reg32 nearest, reg32 farthest)
{
    Guest guest(app, cpu);
    constexpr reg32 kVertexCacheCount = 0xc8;
    constexpr reg32 kVertexCacheUsed = 0x7cf4c4;
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
    for (reg32 i = 0; i < kVertexCacheCount; ++i)
        guest.setDword(kVertexCacheUsed + (i << 5), 0);
    return kVertexCacheCount;
}

void frontend::rendSetClipBounds(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bf1b0", 0x18, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 left = app->getMemory<reg32>(cpu.esp + 4), top = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 right = app->getMemory<reg32>(cpu.esp + 0xc), bottom = app->getMemory<reg32>(cpu.esp + 0x10);
    const reg32 nearest = app->getMemory<reg32>(cpu.esp + 0x14), farthest = app->getMemory<reg32>(cpu.esp + 0x18);
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::setClipBounds(app, cpu, left, top, right, bottom, nearest, farthest); });
}

/* sub_4bf260: the projection's scale and centre: the window's corner and size on the stack, the
 * fields of view next (0 for none, then the size itself), mirrored when the last is set. */
x86::reg32 frontend::rendd::setProjection(App* app, CPU& cpu, reg32 cornerXBits, reg32 cornerYBits, reg32 widthBits,
                                          reg32 heightBits, reg32 fovX, reg32 fovY, reg32 mirrored)
{
    Guest guest(app, cpu);
    constexpr reg32 kHalfFloat = 0x5401c0;
    constexpr reg32 kDegrees = 0x5401c4;       // to radians, by half: a float
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
    return (widthBits & 0xffff00ff) | (reg32(flipped) << 8);
}

void frontend::rendSetProjection(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bf260", 0x1c, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 cornerX = app->getMemory<reg32>(cpu.esp + 4), cornerY = app->getMemory<reg32>(cpu.esp + 8);
    const reg32 width = app->getMemory<reg32>(cpu.esp + 0xc), height = app->getMemory<reg32>(cpu.esp + 0x10);
    const reg32 fovX = app->getMemory<reg32>(cpu.esp + 0x14), fovY = app->getMemory<reg32>(cpu.esp + 0x18);
    const reg32 mirrored = app->getMemory<reg32>(cpu.esp + 0x1c);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        return rendd::setProjection(app, cpu, cornerX, cornerY, width, height, fovX, fovY, mirrored);
    });
}

/* sub_4bf370: eax points at edx, 12 bytes each, projected to the vertices at ebx, 32 bytes each:
 * x and y on the screen, z, its reciprocal, the clip code.  A depth of 0 is kept from dividing.
 * The last point takes the reciprocal of the one before it: the look ahead stops one short. */
x86::reg32 frontend::rendd::projectPoints(App* app, CPU& cpu, reg32 count, reg32 points, reg32 vertices)
{
    Guest guest(app, cpu);
    if (sreg32(count) <= 0)
        return count;
    reg32 point = points, vertex = vertices;
    if ((guest.dword(point + 8) & 0x7fffffff) == 0)
        guest.setDword(point + 8, kSmallestDepth);
    const double scaleX = floatOf(guest, kScaleX), scaleY = floatOf(guest, kScaleY);
    const double centreX = floatOf(guest, kCentreX), centreY = floatOf(guest, kCentreY);
    const ClipBounds bounds = clipBounds(guest);
    reg32 inverse = bitsOf(float(fp::div(1.0, floatOf(guest, point + 8))));
    for (reg32 left = count; left != 0; --left)
    {
        const reg32 own = inverse;
        const double inverseValue = double(floatFrom(own));
        const double perX = fp::mul(scaleX, inverseValue);
        const double perY = fp::mul(scaleY, inverseValue);
        const double y = fp::mul(perY, floatOf(guest, point + 4));
        const double x = fp::mul(perX, floatOf(guest, point));
        const reg32 screenX = bitsOf(float(fp::add(x, centreX)));
        const reg32 screenY = bitsOf(float(fp::add(y, centreY)));
        guest.setDword(vertex + 0xc, own);
        guest.setDword(vertex, screenX);
        guest.setDword(vertex + 8, guest.dword(point + 8));
        guest.setDword(vertex + 4, screenY);
        if (sreg32(left - 2) > 0)
        {
            if ((guest.dword(point + 0x14) & 0x7fffffff) == 0)
                guest.setDword(point + 0x14, kSmallestDepth);
            inverse = bitsOf(float(fp::div(1.0, floatOf(guest, point + 0x14))));
        }
        guest.setByte(vertex + 0x14, clipCode(bounds, sreg32(own), sreg32(screenY), sreg32(screenX)));
        point += 0xc;
        vertex += 0x20;
    }
    cpu.ebx = 0xffffffff;
    guest.leaveEdx(vertex);
    return point;
}

void frontend::rendProjectPoints(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bf370", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 count = cpu.eax, points = cpu.edx, vertices = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return rendd::projectPoints(app, cpu, count, points, vertices); });
}

}
