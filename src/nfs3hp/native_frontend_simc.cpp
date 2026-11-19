#include "native_controls_queue.h"
#include "native_frontend_simc.h"
#include "native_track.h"
#include "native_carstate.h"
#include "native_watcom.h"
#include "game_settings.h"
#include "native_controls_input.h"
#include "native_collide.h"
#include "native_frontend.h"
#include "native_frontend_fx.h"
#include "native_ea_support.h"
#include "native_frontend_loop.h"
#include "native_frontend_loopb.h"
#include "native_frontend_menue.h"
#include "native_frontend_rendd.h"
#include "native_frontend_rende.h"
#include "native_frontend_showcase.h"
#include "native_frontend_sima.h"
#include "native_frontend_simd.h"
#include "native_frontend_view.h"
#include "native_frontend_wina.h"
#include "native_load_cardb.h"
#include "native_vertices.h"
#include "port_text.h"
#include "native_graphics.h"
#include <nfs3hp.h>
#include <game_float.h>
#include <cstring>
#include <initializer_list>

/* What was left of the simulation above 0x4b7000, decompiled, and the
 * pan-and-zoom widget's last small functions:
 *
 *   quaternions and matrices  a car's pose mixed and slerped, quaternion to matrix
 *                             and back, nine-float matrices added, scaled and made
 *                             orthonormal again, turned about an axis;
 *   the view pass             a view's record filled and the race's frame drawn
 *                             through it, and the view's projection set;
 *   the replay's panel        the pointer on its four buttons and its keys;
 *   the television director   which camera follows the action in a replay;
 *   small helpers             the sirens' records, the fixed-point angle, a pack's
 *                             directory, the square root, a frame beat;
 *   the view widget           its load, its laid-out state, its empty entry.
 *
 * All of the game's state stays in guest memory.  The x87's arithmetic goes through
 * fp:: in the game's order, operand for operand, each value through a float where
 * the game stores it in one and reads it back: replays stay the same to the bit.
 * What each function leaves in eax and in the registers it does not save is kept:
 * Watcom's callers may go on with them.
 *
 * Plan step 5: each function a typed core in frontend::simc (native_frontend_simc.h), in
 * a frame of its own where it calls or hands out its frame (fe::Frame), and a thunk
 * under the game's name for the calls by address.  A core of the simulation's files calls
 * another as C++ (guest::direct), and the typed cores of other files too (fe::direct); the
 * rest of the game still by address. */
namespace nfs3hp
{

float zoomHalfWidth(nfs::App* app);  // the port's widescreen change, nfs3hp_main.cpp

namespace
{

using fe::args;
using fe::Guest;
using nfs::Float;
using nfs::u16;
using nfs::u32;
using nfs::u8;
using nfs::s32;
using nfs::s64;

constexpr std::uint8_t kRestoreNone = 0;

/* fld dword, fstp dword. */
Float fldAt(const Guest& guest, u32 address)
{
    return Float(guest.floatAt(address));
}

void fstpAt(Guest& guest, u32 address, Float value)
{
    guest.setFloat(address, float(value));
}

/* fld qword: one of the game's constants. */
Float loadDouble(const Guest& guest, u32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* A float's bits, as mov dword moves it and push hands it on. */
u32 bitsOf(float value)
{
    u32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

u32 highWordOf(double value)
{
    std::uint64_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    return u32(bits >> 32);
}

/* What a jump reads of fcom's status word after sahf: jb (true unordered), ja. */
bool below(u16 status)
{
    return (status & 0x0100) != 0;
}

bool above(u16 status)
{
    return (status & 0x4100) == 0;
}

/* fnstsw ax: the word in ax, the rest of eax as it was. */
u32 withStatus(u32 eax, u16 status)
{
    return (eax & 0xffff0000u) | status;
}

void copyWords(Guest& guest, u32 to, u32 from, u32 count)
{
    for (u32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* idiv of a sign-extended dividend: both results, as the game's code has them. */
struct Division
{
    u32 quotient;
    u32 remainder;
};

Division divide(u32 dividend, u32 divisor)
{
    const s64 wide = s64(s32(dividend));
    const s32 by = s32(divisor);
    return { u32(wide / by), u32(wide % by) };
}

/* The game's functions these call. */
constexpr u32 kArcCosine = 0x4f9482;          // st(0) in, its arc cosine out

/* The car a player's view is on: the replay's if there is one, or the race's. */
constexpr u32 kReplayCars = 0x5e10b8, kRaceCars = 0x5e10b0;

u32 carOf(const Guest& guest, u32 player)
{
    const u32 replay = guest.dword(kReplayCars + 4 * player);
    return replay != 0 ? replay : guest.dword(kRaceCars + 4 * player);
}

/* The camera a player's car has, 0 without a car. */
u32 cameraOfCar(const Guest& guest, u32 player)
{
    const u32 car = carOf(guest, player);
    return car != 0 ? guest.dword(car) : 0;
}

/* The television director's record per player, and the replay panel's state. */
constexpr u32 kDirectorBytes = 0x74;
constexpr u32 kDirectorOn = 0x7cd968;         // the director has the camera
constexpr u32 kDirectorShotStart = 0x7cd96c;
constexpr u32 kDirectorNext = 0x7cd970;       // where it is in kTvCameras
constexpr u32 kDirectorShotLength = 0x7cd974;
constexpr u32 kDirectorAllowed = 0x7cd978;    // kTvCameraKinds words: this camera may be picked
constexpr u32 kTvCameraKinds = 25;
constexpr u32 kTvCameras = 0x55fe68;          // the order they come in
constexpr u32 kTvCameraCount = 21;
constexpr u32 kPanelCameras = 0x55fe44;       // what the panel's camera button goes through
constexpr u32 kPanelCameraCount = 9;
constexpr u32 kCameraTelevision = 15;
constexpr u32 kCameraChase = 9;
constexpr u32 kCameraShortShot = 18;

constexpr u32 kPanelPaused = 0x7cd948;        // a byte
constexpr u32 kPanelSpeed = 0x7cd94c;         // 0 to 3
constexpr u32 kPanelCamera = 0x7cd954;
constexpr u32 kPanelButton = 0x7cd958;        // 0 to 3, the one highlighted
constexpr u32 kPanelRepeat = 0x7cd95c;        // frames before the next key counts
constexpr u32 kPanelCameraChanged = 0x7cd960;
constexpr u32 kPanelCameraWanted = 0x7cd964;
constexpr u32 kRaceClock = 0x7d3684;
constexpr u32 kClockStart = 0x200;            // the clock of a replay's first frame

constexpr u32 kSwitchCamera = 0x422610, kPickChaseNode = 0x421490;
constexpr u32 kKeyPressed = 0x48f8d0;
constexpr u32 kNearestSpot = 0x423d90;

/* The panel's scan codes. */
constexpr u32 kKeyLeft = 0x4b, kKeyRight = 0x4d, kKeyDown = 0x50, kKeyEnter = 0x1c, kKeyUp = 0x48;

/* The director takes the camera over (the panel's camera kCameraTelevision), for
 * each player: every camera allowed, a shot of the first of the list.  What the last
 * switch left in eax. */
u32 startTelevision(nfs::App* app, nfs::ThreadState& ts, Guest& guest, u32 players)
{
    u32 left = 0;
    for (u32 player = 0; player < players; ++player)
    {
        const u32 record = player * kDirectorBytes;
        guest.setDword(kDirectorOn + record, 1);
        guest.setDword(kDirectorNext + record, 0);
        guest::direct<0x4bdcc0, frontend::simc::tvStartShot>(app, ts, player, guest.dword(kTvCameras));
        for (u32 kind = 0; kind < kTvCameraKinds; ++kind)
            guest.setDword(kDirectorAllowed + record + 4 * kind, 1);
        const u32 camera = guest.dword(kTvCameras + 4 * guest.dword(kDirectorNext + record));
        left = *fe::direct<kSwitchCamera, carstate::cameraSwitched>(guest.app(), guest.ts(), camera, player);
    }
    return left;
}

/* A float of those bits, as fld dword reads the stack's word. */
Float fromBits(u32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

}

namespace frontend
{

/* ---------------------------------------------------------------------------
 * Quaternions and matrices
 * ------------------------------------------------------------------------- */

/* sub_4b70f0: a pose mixed between two: the quaternions at eax and edx and the
 * places at ebx and ecx, by the float on the stack, into the two out pointers after
 * it.  Plain linear mixes, each read after the write before it. */
nfs::u32 simc::poseBlend(App* app, ThreadState& ts, u32 quatA, u32 quatB, u32 placeA, u32 placeB, u32 tBits,
                           u32 quatOut, u32 placeOut)
{
    Guest guest(app, ts);
    const Float t = fromBits(tBits);
    const Float s = fp::sub(1.0, t);
    for (u32 i = 0; i < 4; ++i)
    {
        const Float a = fp::mul(fldAt(guest, quatA + 4 * i), s);
        fstpAt(guest, quatOut + 4 * i, fp::add(a, fp::mul(fldAt(guest, quatB + 4 * i), t)));
    }
    for (u32 i = 0; i < 2; ++i)
    {
        const Float a = fp::mul(fldAt(guest, placeA + 4 * i), s);
        fstpAt(guest, placeOut + 4 * i, fp::add(a, fp::mul(fldAt(guest, placeB + 4 * i), t)));
    }
    const Float a = fp::mul(s, fldAt(guest, placeA + 8));
    fstpAt(guest, placeOut + 8, fp::add(a, fp::mul(t, fldAt(guest, placeB + 8))));
    return quatA;
}

nfs::u32 simPoseBlend(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b70f0", 12, 0, nullptr, 0, fe::kRestoreAll };
    const u32 quatA = args.eax, quatB = args.edx, placeA = args.ebx, placeB = args.ecx;
    const u32 t = guest::at(app, ts.sp + 4), quatOut = guest::at(app, ts.sp + 8), placeOut = guest::at(app, ts.sp + 12);
    return fe::runThunk(app, ts, args, site,
                 [&](Guest&) { return simc::poseBlend(app, ts, quatA, quatB, placeA, placeB, t, quatOut, placeOut); });
}

/* sub_4b71e0: quaternion slerp: eax and edx mixed by the float on the stack, into
 * the pointer after it.  The second is negated first when they are more than a
 * quarter turn apart, and two closer than kNear in cosine are mixed linearly. */
nfs::u32 simc::quatSlerp(App* app, ThreadState& ts, u32 from, u32 to, u32 tBits, u32 out)
{
    constexpr u32 kNear = 0x53ff60;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x50);
    Float dot = fp::mul(fldAt(guest, from + 4), fldAt(guest, to + 4));
    dot = fp::add(dot, fp::mul(fldAt(guest, from), fldAt(guest, to)));
    dot = fp::add(dot, fp::mul(fldAt(guest, from + 8), fldAt(guest, to + 8)));
    dot = fp::add(dot, fp::mul(fldAt(guest, from + 12), fldAt(guest, to + 12)));
    double cosine = dot;
    const bool negated = above(fp::compare(0.0, cosine));
    float flipped[4] = {};
    if (negated)
    {
        for (u32 i = 0; i < 4; ++i)
            flipped[i] = float(-fldAt(guest, to + 4 * i));
        cosine = -cosine;
    }
    const auto target = [&](u32 i) { return negated ? Float(flipped[i]) : fldAt(guest, to + 4 * i); };

    const u16 status = fp::compare(fp::sub(1.0, cosine), loadDouble(guest, kNear));
    u32 left = withStatus(from, status);
    double fromWeight, toWeight;
    if (above(status))
    {
        nfs::Args arc;
        arc.floatIn[0] = cosine;
        arc.floatsIn = 1;
        left = fx::callAt(guest, kArcCosine, args({ left }), &arc);
        const Float angle = arc.floatResult;
        const Float sine = fp::sin(angle);
        const Float t = fromBits(tBits);
        const Float sineFrom = fp::sin(fp::mul(fp::sub(1.0, t), angle));
        const Float sineTo = fp::sin(fp::mul(angle, t));
        const Float reciprocal = fp::div(1.0, sine);
        fromWeight = fp::mul(sineFrom, reciprocal);
        toWeight = fp::mul(reciprocal, sineTo);
    }
    else
    {
        const double t = fromBits(tBits);
        fromWeight = fp::sub(1.0, t);
        toWeight = t;
    }
    for (u32 i = 0; i < 3; ++i)
    {
        const Float a = fp::mul(fldAt(guest, from + 4 * i), fromWeight);
        fstpAt(guest, out + 4 * i, fp::add(a, fp::mul(target(i), toWeight)));
    }
    const Float a = fp::mul(fromWeight, fldAt(guest, from + 12));
    fstpAt(guest, out + 12, fp::add(a, fp::mul(toWeight, target(3))));
    return left;
}

nfs::u32 simQuatSlerp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b71e0", 8, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 from = args.eax, to = args.edx, t = guest::at(app, ts.sp + 4), out = guest::at(app, ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::quatSlerp(app, ts, from, to, t, out); });
}

/* sub_4b7300: the quaternion at edx negated if its negative is nearer the one at eax:
 * the squared length of the sum against that of the difference.  The signs go byte
 * by byte, not through the x87. */
nfs::u32 simc::quatNearer(App* app, ThreadState& ts, u32 a, u32 b)
{
    Guest guest(app, ts);
    /* A value by itself, the second factor through its float, as the game keeps it. */
    const auto squared = [](Float value) { return fp::mul(value, Float(float(value))); };
    const auto sumOf = [&](u32 offset) { return squared(fp::add(fldAt(guest, a + offset), fldAt(guest, b + offset))); };
    const auto differenceOf = [&](u32 offset) { return squared(fp::sub(fldAt(guest, a + offset), fldAt(guest, b + offset))); };
    Float differences = fp::add(differenceOf(4), differenceOf(0));
    Float sums = fp::add(sumOf(4), sumOf(0));
    sums = fp::add(sums, sumOf(8));
    differences = fp::add(differences, differenceOf(8));
    sums = fp::add(sums, sumOf(12));
    differences = fp::add(differences, differenceOf(12));
    const u16 status = fp::compare(sums, differences);
    u32 left = withStatus(a, status);
    if (below(status))
    {
        const u8 highest = u8(guest.byte(b + 3) ^ 0x80);
        guest.setByte(b + 7, u8(guest.byte(b + 7) ^ 0x80));
        guest.setByte(b + 15, u8(guest.byte(b + 15) ^ 0x80));
        guest.setByte(b + 3, highest);
        guest.setByte(b + 11, u8(guest.byte(b + 11) ^ 0x80));
        left = (left & 0xffff00ffu) | (u32(highest) << 8);
    }
    return left;
}

nfs::u32 simQuatNearer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b7300", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 a = args.eax, b = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::quatNearer(app, ts, a, b); });
}

/* sub_4b73c0: the 3x3 matrix at eax made the quaternion at edx: by its trace when
 * that is above zero, otherwise from its largest diagonal element. */
nfs::u32 simc::quatFromMatrix(App* app, ThreadState& ts, u32 matrix, u32 out)
{
    constexpr u32 kHalf = 0x53ff68;
    static const u32 kNextAxis[3] = { 1, 2, 0 };  // the table at 0x4b6a1c
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x58);
    const auto element = [&](u32 row, u32 column) { return matrix + 12 * row + 4 * column; };
    const auto rootOf = [&](Float value) {
        return Float(guest::direct<0x4ea920, squareRootOf>(app, ts, bitsOf(float(value))));
    };

    const Float sum = fp::add(fp::add(fldAt(guest, element(0, 0)), fldAt(guest, element(1, 1))), fldAt(guest, element(2, 2)));
    const float trace = float(sum);
    const u16 status = fp::compare(0.0, Float(trace));
    if (below(status))
    {
        const float root = float(rootOf(fp::add(1.0, Float(trace))));
        const Float half = fldAt(guest, kHalf);
        const Float halfOverRoot = fp::div(half, Float(root));
        fstpAt(guest, out + 12, fp::mul(half, Float(root)));
        const float scale = float(halfOverRoot);
        fstpAt(guest, out, fp::mul(fp::sub(fldAt(guest, element(1, 2)), fldAt(guest, element(2, 1))), Float(scale)));
        fstpAt(guest, out + 4, fp::mul(fp::sub(fldAt(guest, element(2, 0)), fldAt(guest, element(0, 2))), Float(scale)));
        fstpAt(guest, out + 8, fp::mul(fp::sub(fldAt(guest, element(0, 1)), fldAt(guest, element(1, 0))), Float(scale)));
        return matrix + 12;
    }

    u32 i = 0;
    if (above(fp::compare(fldAt(guest, element(1, 1)), fldAt(guest, element(0, 0)))))
        i = 1;
    if (above(fp::compare(fldAt(guest, element(2, 2)), fldAt(guest, element(i, i)))))
        i = 2;
    const u32 j = kNextAxis[i], k = kNextAxis[j];
    const Float others = fp::add(fldAt(guest, element(j, j)), fldAt(guest, element(k, k)));
    const Float root = rootOf(fp::add(fp::sub(fldAt(guest, element(i, i)), others), 1.0));
    float scale = float(root);
    float quat[3];
    quat[i] = float(fp::mul(root, fldAt(guest, kHalf)));
    if ((bitsOf(scale) & 0x7fffffffu) != 0)
        scale = float(fp::div(fldAt(guest, kHalf), Float(scale)));
    const Float s = Float(scale);
    const float w = float(fp::mul(fp::sub(fldAt(guest, element(j, k)), fldAt(guest, element(k, j))), s));
    quat[j] = float(fp::mul(fp::add(fldAt(guest, element(i, j)), fldAt(guest, element(j, i))), s));
    quat[k] = float(fp::mul(s, fp::add(fldAt(guest, element(i, k)), fldAt(guest, element(k, i)))));
    guest.setDword(out, bitsOf(quat[0]));
    guest.setDword(out + 4, bitsOf(quat[1]));
    guest.setDword(out + 8, bitsOf(quat[2]));
    guest.setDword(out + 12, bitsOf(w));
    return bitsOf(w);
}

nfs::u32 simQuatFromMatrix(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b73c0", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 matrix = args.eax, out = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::quatFromMatrix(app, ts, matrix, out); });
}

/* sub_4b7590: the quaternion at eax made the 3x3 matrix at edx. */
nfs::u32 simc::matrixFromQuat(App* app, ThreadState& ts, u32 quat, u32 out)
{
    constexpr u32 kTwo = 0x53ff6c;
    Guest guest(app, ts);
    const Float two = fldAt(guest, kTwo);
    const Float x = fldAt(guest, quat), y = fldAt(guest, quat + 4), z = fldAt(guest, quat + 8), w = fldAt(guest, quat + 12);
    const Float twoX = fp::mul(x, two);
    const float twoY = float(fp::mul(y, two));
    const float twoZ = float(fp::mul(two, z));
    const float wx = float(fp::mul(w, twoX));
    const float wy = float(fp::mul(w, Float(twoY)));
    const float wz = float(fp::mul(w, Float(twoZ)));
    const float xx = float(fp::mul(twoX, x));
    const Float xy = fp::mul(x, Float(twoY));
    const float xz = float(fp::mul(x, Float(twoZ)));
    const float yy = float(fp::mul(y, Float(twoY)));
    const Float yz = fp::mul(y, Float(twoZ));
    const Float zz = fp::mul(z, Float(twoZ));

    const Float oneLessYy = fp::sub(1.0, Float(yy));
    const Float oneLessXx = fp::sub(1.0, Float(xx));
    fstpAt(guest, out + 4, fp::add(xy, Float(wz)));
    fstpAt(guest, out + 8, fp::sub(Float(xz), Float(wy)));
    fstpAt(guest, out + 12, fp::sub(Float(float(xy)), Float(wz)));
    fstpAt(guest, out + 20, fp::add(yz, Float(wx)));
    fstpAt(guest, out + 24, fp::add(Float(xz), Float(wy)));
    fstpAt(guest, out + 28, fp::sub(Float(float(yz)), Float(wx)));
    fstpAt(guest, out, fp::sub(oneLessYy, zz));
    fstpAt(guest, out + 16, fp::sub(oneLessXx, zz));
    fstpAt(guest, out + 32, fp::sub(oneLessXx, Float(yy)));
    return quat;
}

nfs::u32 simMatrixFromQuat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b7590", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 quat = args.eax, out = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::matrixFromQuat(app, ts, quat, out); });
}

/* sub_4b7680: two 3x3 matrices, eax and edx, slerped through quaternions by the
 * float on the stack, into the pointer after it. */
nfs::u32 simc::matrixSlerp(App* app, ThreadState& ts, u32 from, u32 to, u32 t, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x3c);
    const u32 mixed = guest.frame(), toQuat = guest.frame() + 0x10, fromQuat = guest.frame() + 0x20;
    guest::direct<0x4b73c0, quatFromMatrix>(app, ts, from, fromQuat);
    guest::direct<0x4b73c0, quatFromMatrix>(app, ts, to, toQuat);
    guest::direct<0x4b71e0, quatSlerp>(app, ts, fromQuat, toQuat, t, mixed);
    // `out` in edx stays there after it, as the code leaves it.
    return guest::direct<0x4b7590, matrixFromQuat>(app, ts, mixed, out);
}

nfs::u32 simMatrixSlerp(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b7680", 8, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const u32 from = args.eax, to = args.edx, t = guest::at(app, ts.sp + 4), out = guest::at(app, ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::matrixSlerp(app, ts, from, to, t, out); });
}

/* sub_4b7720: the 3x3 matrix at ebx is eax minus edx. */
nfs::u32 simc::matrixSubtract(App* app, ThreadState& ts, u32 a, u32 b, u32 out)
{
    Guest guest(app, ts);
    for (u32 at = 0; at < 36; at += 4)
        fstpAt(guest, out + at, fp::sub(fldAt(guest, a + at), fldAt(guest, b + at)));
    return a;
}

nfs::u32 simMatrixSubtract(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b7720", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 a = args.eax, b = args.edx, out = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::matrixSubtract(app, ts, a, b, out); });
}

/* sub_4b7780: the 3x3 matrix at ebx is eax plus edx. */
nfs::u32 simc::matrixAdd(App* app, ThreadState& ts, u32 a, u32 b, u32 out)
{
    Guest guest(app, ts);
    for (u32 at = 0; at < 36; at += 4)
        fstpAt(guest, out + at, fp::add(fldAt(guest, a + at), fldAt(guest, b + at)));
    return a;
}

nfs::u32 simMatrixAdd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b7780", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 a = args.eax, b = args.edx, out = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::matrixAdd(app, ts, a, b, out); });
}

/* sub_4b77e0: the 3x3 matrix at eax times the float on the stack, into the pointer
 * after it. */
nfs::u32 simc::matrixScale(App* app, ThreadState& ts, u32 matrix, u32 factorBits, u32 out)
{
    Guest guest(app, ts);
    const Float factor = fromBits(factorBits);
    for (u32 at = 0; at < 32; at += 4)
        fstpAt(guest, out + at, fp::mul(fldAt(guest, matrix + at), factor));
    fstpAt(guest, out + 32, fp::mul(factor, fldAt(guest, matrix + 32)));
    return matrix;
}

nfs::u32 simMatrixScale(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b77e0", 8, 0, nullptr, 0, fe::kRestoreAll };
    const u32 matrix = args.eax, factor = guest::at(app, ts.sp + 4), out = guest::at(app, ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::matrixScale(app, ts, matrix, factor, out); });
}

/* sub_4b7840: the 3x3 matrix at eax made orthonormal again: M = M (M'M)^-1/2, the
 * root by edx terms -- 10 at the least -- of its binomial series, with coefficients
 * at 0x55fdcc. */
nfs::u32 simc::matrixOrthonormalise(App* app, ThreadState& ts, u32 matrix, u32 termsAsked)
{
    constexpr u32 kIdentity = 0x564498;
    constexpr u32 kCoefficients = 0x55fdcc;
    constexpr u32 kWords = 9;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0xd4);
    const u32 terms = s32(termsAsked) > 10 ? termsAsked : 10;
    const u32 identity = guest.frame(), term = guest.frame() + 0x24, excess = guest.frame() + 0x48;
    const u32 power = guest.frame() + 0x6c, sum = guest.frame() + 0x90;
    const float limit = float(Float(s32(terms)));

    // The game's code hands the transpose `term` in ebx too, which it does not read.
    guest::direct<0x4e0110, simd::transpose>(app, ts, matrix, term);
    fe::direct<0x4e0280, vertices::multiplyMatrix>(app, ts, term, matrix, term);
    copyWords(guest, identity, kIdentity, kWords);
    guest::direct<0x4b7720, matrixSubtract>(app, ts, term, identity, excess);
    copyWords(guest, power, kIdentity, kWords);
    copyWords(guest, sum, power, kWords);
    for (u32 i = 1; below(fp::compare(Float(s32(i)), Float(limit))); ++i)
    {
        fe::direct<0x4e0280, vertices::multiplyMatrix>(app, ts, power, excess, power);
        // The game's code hands it `term` and `sum` in edx and ebx too, which it does not read.
        guest::direct<0x4b77e0, matrixScale>(app, ts, power, guest.dword(kCoefficients + 4 * i), term);
        guest::direct<0x4b7780, matrixAdd>(app, ts, sum, term, sum);
    }
    return fe::direct<0x4e0280, vertices::multiplyMatrixEax>(app, ts, matrix, sum, matrix);
}

nfs::u32 simMatrixOrthonormalise(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b7840", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 matrix = args.eax, terms = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::matrixOrthonormalise(app, ts, matrix, terms); });
}

/* sub_4b79c0: the 3x3 matrix at eax turned a quarter: the second row is the third,
 * the third the second's negative. */
nfs::u32 simc::matrixQuarterTurn(App* app, ThreadState& ts, u32 matrix)
{
    Guest guest(app, ts);
    for (u32 at = 0; at < 12; at += 4)
    {
        const Float second = fldAt(guest, matrix + 12 + at);
        const Float third = fldAt(guest, matrix + 24 + at);
        fstpAt(guest, matrix + 12 + at, third);
        fstpAt(guest, matrix + 24 + at, -second);
    }
    return matrix;
}

nfs::u32 simMatrixQuarterTurn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4b79c0", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 matrix = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::matrixQuarterTurn(app, ts, matrix); });
}

/* sub_4ea680: the 3x3 matrix at eax turned about its first row, by the angle on the
 * stack (handed on by its address), into the pointer after it, which may be the matrix
 * itself. */
nfs::u32 simc::turnAboutFirstRow(App* app, ThreadState& ts, u32 matrix, u32 angleAt, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x44);
    u32 source = matrix;
    if (source == out)
    {
        copyWords(guest, guest.frame(), out, 9);
        source = guest.frame();
    }
    guest::direct<0x4ea970, simd::sineCosine>(app, ts, angleAt, guest.frame() + 0x24, guest.frame() + 0x28);
    const Float sine = fldAt(guest, guest.frame() + 0x24), cosine = fldAt(guest, guest.frame() + 0x28);
    guest.setDword(out, guest.dword(source));
    guest.setDword(out + 4, guest.dword(source + 4));
    const u32 left = guest.dword(source + 8);
    guest.setDword(out + 8, left);
    for (u32 at = 0; at < 12; at += 4)
    {
        const Float a = fp::mul(fldAt(guest, source + 12 + at), cosine);
        fstpAt(guest, out + 12 + at, fp::sub(a, fp::mul(fldAt(guest, source + 24 + at), sine)));
    }
    for (u32 at = 0; at < 12; at += 4)
    {
        const Float a = fp::mul(fldAt(guest, source + 24 + at), cosine);
        fstpAt(guest, out + 24 + at, fp::add(a, fp::mul(fldAt(guest, source + 12 + at), sine)));
    }
    return left;
}

nfs::u32 simTurnAboutFirstRow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ea680", 8, 0, nullptr, 0, fe::kRestoreAll };
    const u32 matrix = args.eax, angleAt = ts.sp + 4, out = guest::at(app, ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::turnAboutFirstRow(app, ts, matrix, angleAt, out); });
}

/* sub_4ea820: the 3x3 matrix at eax turned about its last row, as sub_4ea680. */
nfs::u32 simc::turnAboutLastRow(App* app, ThreadState& ts, u32 matrix, u32 angleAt, u32 out)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x44);
    u32 source = matrix;
    if (source == out)
    {
        copyWords(guest, guest.frame(), out, 9);
        source = guest.frame();
    }
    guest::direct<0x4ea970, simd::sineCosine>(app, ts, angleAt, guest.frame() + 0x24, guest.frame() + 0x28);
    const Float sine = fldAt(guest, guest.frame() + 0x24), cosine = fldAt(guest, guest.frame() + 0x28);
    for (u32 at = 0; at < 12; at += 4)
    {
        const Float a = fp::mul(fldAt(guest, source + at), cosine);
        fstpAt(guest, out + at, fp::sub(a, fp::mul(fldAt(guest, source + 12 + at), sine)));
    }
    for (u32 at = 0; at < 12; at += 4)
    {
        const Float a = fp::mul(fldAt(guest, source + 12 + at), cosine);
        fstpAt(guest, out + 12 + at, fp::add(a, fp::mul(fldAt(guest, source + at), sine)));
    }
    guest.setDword(out + 24, guest.dword(source + 24));
    guest.setDword(out + 28, guest.dword(source + 28));
    const u32 left = guest.dword(source + 32);
    guest.setDword(out + 32, left);
    return left;
}

nfs::u32 simTurnAboutLastRow(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ea820", 8, 0, nullptr, 0, fe::kRestoreAll };
    const u32 matrix = args.eax, angleAt = ts.sp + 4, out = guest::at(app, ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::turnAboutLastRow(app, ts, matrix, angleAt, out); });
}

/* sub_4ea920: the square root of the float on the stack, onto the x87's. */
double simc::squareRootOf(App*, u32 value)
{
    return fp::sqrt(fromBits(value));
}

nfs::u32 simSquareRoot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ea920", 4, 0, nullptr, 0, fe::kRestoreAll };
    const u32 eax = args.eax, value = guest::at(app, ts.sp + 4);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        args.floatSet = true;
        args.floatResult = simc::squareRootOf(app, value);
        return eax;
    });
}

/* ---------------------------------------------------------------------------
 * A view's pass
 * ------------------------------------------------------------------------- */

/* sub_4bc470: a view's pass: eax the pass (0 or 1 draw), edx, ebx, ecx and the word
 * on the stack what the view's record keeps, the frame drawn through it. */
nfs::u32 simc::viewPass(App* app, ThreadState& ts, u32 pass, u32 record4, u32 record68, u32 record6c,
                          u32 record70)
{
    constexpr u32 kView = 0x7a3d14;
    constexpr u32 kChooseView = 0x421450, kSetMirrored = 0x4349a0, kAmbientSounds = 0x41bb50;
    constexpr u32 kRaceFrame = 0x4bc160, kDrawMirrored = 0x4349d0;
    constexpr u32 kFirstShare = 0x560094, kSecondShare = 0x560098;
    constexpr float kFourFifths = 0.8f, kOneFifth = 0.2f;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 12);
    fe::direct<0x4bbd70, rendd::arenaRewind>(app, ts);
    guest.setDword(kView, pass);
    guest.setDword(kView + 4, record4);
    guest.setDword(kView + 0x68, record68);
    guest.setDword(kView + 0x70, record70);
    guest.setDword(kView + 0x6c, record6c);
    fe::direct<kChooseView, carstate::cameraViewChosen>(guest.app(), guest.ts(), kView, 0);
    fe::direct<kSetMirrored, collide::setViewMirrored>(guest.app(), guest.ts(), kView);
    if (pass == 0)
    {
        guest.setDword(kFirstShare, bitsOf(kFourFifths));
        guest.setDword(kSecondShare, bitsOf(kOneFifth));
    }
    else if (pass == 1)
    {
        guest.setDword(kFirstShare, bitsOf(kOneFifth));
        guest.setDword(kSecondShare, 0);
    }
    else
    {
        return 0;
    }
    fe::direct<kAmbientSounds, track::ambientAround>(guest.app(), guest.ts(), kView);
    fe::direct<0x4bc160, graphics::raceFrameDrawn>(app, ts, pass);
    fe::direct<kChooseView, carstate::cameraViewChosen>(guest.app(), guest.ts(), kView, 2);
    const u32 first = guest.dword(0x7a3cf0), second = guest.dword(0x7a3cf4);
    (void)second;
    fe::direct<kDrawMirrored, collide::listDrawnMirrored>(app, ts, kView, first);
    return kView;
}

nfs::u32 simViewPass(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bc470", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 pass = args.eax, record4 = args.edx, record68 = args.ebx, record6c = args.ecx;
    const u32 record70 = guest::at(app, ts.sp + 4);
    return fe::runThunk(app, ts, args, site,
                 [&](Guest&) { return simc::viewPass(app, ts, pass, record4, record68, record6c, record70); });
}

/* sub_4dbde0: the projection of the view at eax: the first's own, or each player's,
 * and a racing zoom camera's scales (the port's zoomHalfWidth keeps pixels square on
 * a wide view). */
nfs::u32 simc::setView(App* app, ThreadState& ts, u32 view)
{
    constexpr u32 kCarZoom = 0x54;
    constexpr u32 kScaleX = 0x56009c, kScaleY = 0x5600a0, kHalfHeight = 0x5600b0, kAspect = 0x54972c;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 20);
    if (guest.dword(view) == 1)
    {
        const u32 notMirrored = gameSettings(guest).mirrored == 0 ? 1 : 0;
        const u32 fov = fe::direct<0x4dbcc0, rende::defaultFieldOfView>(app, ts);
        return fe::direct<0x4bed20, loopb::useWithProjection>(app, ts, 0x7cdb48, fov, notMirrored);
    }
    const bool split = gameSettings(guest).playMode == 1;
    const u32 mirrored = gameSettings(guest).mirrored;
    const u32 fov = fe::direct<0x4dbcc0, rende::defaultFieldOfView>(app, ts);
    const u32 viewport = split ? 0x7cda80 + (guest.dword(view + 4) << 5) : 0x7cdb88;
    fe::direct<0x4bed20, loopb::useWithProjection>(app, ts, viewport, fov, mirrored);

    u32 car = carOf(guest, guest.dword(view + 4));
    if ((guest.dword(car + kCarZoom) & 0x7fffffffu) == 0)
        return car;
    car = carOf(guest, guest.dword(view + 4));
    const Float scaleX = fp::mul(Float(zoomHalfWidth(app)), fldAt(guest, car + kCarZoom));
    car = carOf(guest, guest.dword(view + 4));
    fstpAt(guest, kScaleX, scaleX);
    const Float scaleY = fp::mul(fp::mul(-fldAt(guest, kHalfHeight), fldAt(guest, car + kCarZoom)),
                                 loadDouble(guest, kAspect));
    const u32 flip = gameSettings(guest).mirrored;
    fstpAt(guest, kScaleY, scaleY);
    if (flip != 0)
        guest.setByte(kScaleX + 3, u8(guest.byte(kScaleX + 3) ^ 0x80));
    return car;
}

nfs::u32 simSetView(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4dbde0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::setView(app, ts, view); });
}

/* sub_4d5720: nothing of its own: the race frame's end, sub_434a90, every register as it
 * came; by address, for what it leaves in eax is this function's. */
nfs::u32 simc::raceFrameEnd(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    return fe::direct<0x434a90, collide::fogColour>(app, ts) ? 1 : 0;
}

nfs::u32 simRaceFrameEnd(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d5720", 0, 0, nullptr, 0, kRestoreNone };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::raceFrameEnd(app, ts); });
}

/* ---------------------------------------------------------------------------
 * The replay's panel
 * ------------------------------------------------------------------------- */

/* sub_4bd4d0: the pointer on the replay's panel: inside it the button under it is
 * highlighted; a press inside, no sooner than 0x20 ticks after the last, is the
 * buttons' word; anything else 0. */
nfs::u32 simc::panelPointer(App* app, ThreadState& ts)
{
    constexpr u32 kPanelLeft = 0x7497d8, kPanelTop = 0x7497dc, kPanelRight = 0x7497e0, kPanelBottom = 0x7497e4;
    constexpr u32 kFirstEnd = 0x540198, kSecondEnd = 0x5401a0, kThirdEnd = 0x5401a8;
    constexpr u32 kTicks = 0x79c1a4, kNextPress = 0x55febc;
    constexpr u32 kPressGap = 0x20;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x54);
    const auto floatOfInteger = [](u32 value) { return float(Float(s32(value))); };
    const float left = floatOfInteger(guest.dword(kPanelLeft));
    const float right = floatOfInteger(guest.dword(kPanelRight));
    const float top = floatOfInteger(guest.dword(kPanelTop));
    const float bottom = floatOfInteger(guest.dword(kPanelBottom));
    const u32 buttons = guest.frame() + 0xc, mouseX = guest.frame() + 0x10, mouseY = guest.frame() + 0x14;
    const u32 hotX = guest.frame() + 0x18, hotY = guest.frame() + 0x1c;
    fe::direct<0x435310, controls::mouseRead>(app, ts, buttons, mouseX, mouseY, 0);
    fe::direct<0x4dab10, menue::pointerHotSpot>(app, ts, hotX, hotY);
    const float x = floatOfInteger(guest.dword(mouseX) - guest.dword(hotX) + 7);
    const float y = floatOfInteger(guest.dword(mouseY) - guest.dword(hotY) + 1);
    const bool inside = !below(fp::compare(Float(x), Float(left))) && !above(fp::compare(Float(x), Float(right)))
                        && !below(fp::compare(Float(y), Float(top))) && !above(fp::compare(Float(y), Float(bottom)));
    if (inside)
    {
        const Float width = fp::sub(Float(right), Float(left));
        const Float first = fp::add(fp::mul(width, loadDouble(guest, kFirstEnd)), Float(left));
        const float second = float(fp::add(fp::mul(width, loadDouble(guest, kSecondEnd)), Float(left)));
        const float third = float(fp::add(Float(left), fp::mul(width, loadDouble(guest, kThirdEnd))));
        u32 button = 3;
        if (below(fp::compare(Float(x), first)))
            button = 0;
        else if (below(fp::compare(Float(x), Float(second))))
            button = 1;
        else if (below(fp::compare(Float(x), Float(third))))
            button = 2;
        guest.setDword(kPanelButton, button);
    }
    if (guest.dword(buttons) != 0 && inside)
    {
        const u32 ticks = guest.dword(kTicks);
        if (s32(ticks) > s32(guest.dword(kNextPress)))
        {
            guest.setDword(kNextPress, ticks + kPressGap);
            return guest.dword(buttons);
        }
    }
    return u32(0);
}

nfs::u32 simPanelPointer(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bd4d0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::panelPointer(app, ts); });
}

/* sub_4bd640: the replay's panel each frame: a camera asked for last frame made
 * current for each player, then one key: left and right move the highlight, down
 * takes the button a step back -- the camera before, the speed below -- and Enter,
 * up or a click a step on, or leaves the replay; both toggle the pause. */
nfs::u32 simc::panelKeys(App* app, ThreadState& ts)
{
    constexpr u32 kDirectorWanted = 0x7d36a8;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24);
    const auto pressed = [&](u32 key) { return fe::direct<0x48f8d0, controls::keyDownNow>(app, ts, key, 1); };
    const u32 players = gameSettings(guest).playMode == 1 ? 2 : 1;
    u32 left = 0;

    const u32 wanted = guest.dword(kPanelCameraWanted);
    if (s32(wanted) > 0 && guest.dword(kPanelCameraChanged) != 0)
    {
        guest.setDword(kPanelCamera, wanted);
        for (u32 player = 0; player < players; ++player)
        {
            guest.setDword(kDirectorOn + player * kDirectorBytes, 0);
            fe::direct<kSwitchCamera, carstate::cameraSwitched>(guest.app(), guest.ts(), guest.dword(kPanelCameras + 4 * guest.dword(kPanelCamera)), player);
            left = guest.dword(kPanelCamera);
            if (guest.dword(kPanelCameras + 4 * left) == kCameraChase)
                left = fe::direct<kPickChaseNode, carstate::chaseNodePicked>(guest.app(), guest.ts(), player);
        }
    }

    const u32 repeat = guest.dword(kPanelRepeat);
    guest.setDword(kPanelCameraChanged, 0);
    if (s32(repeat) > 0)
    {
        guest.setDword(kPanelRepeat, repeat - 1);
    }
    else if (pressed(kKeyLeft) != 0)
    {
        const u32 button = guest.dword(kPanelButton);
        if (s32(button) > 0)
            guest.setDword(kPanelButton, button - 1);
        left = fe::direct<0x4bd3f0, loopb::pointerAtChoice>(app, ts, guest.dword(kPanelButton));
    }
    else if (pressed(kKeyRight) != 0)
    {
        const u32 button = guest.dword(kPanelButton);
        if (s32(button) < 3)
            guest.setDword(kPanelButton, button + 1);
        left = fe::direct<0x4bd3f0, loopb::pointerAtChoice>(app, ts, guest.dword(kPanelButton));
    }
    else if (pressed(kKeyDown) != 0)
    {
        const u32 button = guest.dword(kPanelButton);
        guest.setDword(kPanelRepeat, 8);
        left = button;
        if (button == 0)
        {
            const u32 previous = guest.dword(kPanelCamera) - 1;
            guest.setDword(kPanelCameraChanged, 1);
            guest.setDword(kPanelCamera, previous);
            if (s32(previous) < 0)
                guest.setDword(kPanelCamera, kPanelCameraCount - 1);
            const u32 camera = guest.dword(kPanelCamera);
            guest.setDword(kPanelCameraWanted, camera);
            left = camera;
            if (guest.dword(kPanelCameras + 4 * camera) == kCameraTelevision)
                left = startTelevision(app, ts, guest, players);
        }
        else if (button == 1)
        {
            const u32 speed = guest.dword(kPanelSpeed) - 1;
            guest.setDword(kPanelSpeed, speed);
            if (s32(speed) < 0)
                guest.setDword(kPanelSpeed, 3);
        }
        else if (button == 2)
        {
            guest.setByte(kPanelPaused, u8(guest.byte(kPanelPaused) ^ 1));
        }
    }
    else if (pressed(kKeyEnter) != 0 || pressed(kKeyUp) != 0 || guest::direct<0x4bd4d0, panelPointer>(app, ts) != 0)
    {
        const u32 button = guest.dword(kPanelButton);
        guest.setDword(kPanelRepeat, 8);
        left = button;
        if (button == 0)
        {
            const Division next = divide(guest.dword(kPanelCamera) + 1, kPanelCameraCount);
            guest.setDword(kPanelCameraChanged, 1);
            guest.setDword(kPanelCamera, next.remainder);
            const u32 camera = guest.dword(kPanelCameras + 4 * next.remainder);
            guest.setDword(kPanelCameraWanted, next.remainder);
            left = next.quotient;
            if (camera == kCameraTelevision)
                left = startTelevision(app, ts, guest, players);
        }
        else if (button == 1)
        {
            const Division next = divide(guest.dword(kPanelSpeed) + 1, 4);
            guest.setDword(kPanelSpeed, next.remainder);
            left = next.quotient;
        }
        else if (button == 2)
        {
            guest.setByte(kPanelPaused, u8(guest.byte(kPanelPaused) ^ 1));
        }
        else if (button == 3)
        {
            guest.setDword(kPanelRepeat, 0x18);
            fe::direct<0x451960, port::text::menuKeyOf>(app, ts, ts.sp - 4, 1);
            // The game's code leaves the key in eax for it, which it does not read.
            left = fe::direct<0x4c4d80, loopb::stop>(app, ts);
        }
    }
    else
    {
        left = 0;
    }

    // The game's code hands it 0 in eax, which it does not read: there as the code has it.
    if (guest.dword(kDirectorWanted) != 0)
    {
        left = guest::direct<0x426680, sima::camerasStep>(app, ts);
    }
    return left;
}

nfs::u32 simPanelKeys(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bd640", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::panelKeys(app, ts); });
}

/* ---------------------------------------------------------------------------
 * The television director
 * ------------------------------------------------------------------------- */

/* sub_4bdb60: player eax's camera made edx, and a new shot begun whatever it was. */
nfs::u32 simc::tvSetCamera(App* app, ThreadState& ts, u32 player, u32 camera)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 16);
    guest::direct<0x4bdcc0, tvStartShot>(app, ts, player, camera);
    const u32 current = cameraOfCar(guest, player);
    if (current == camera)
        return current;
    u32 left = *fe::direct<kSwitchCamera, carstate::cameraSwitched>(guest.app(), guest.ts(), camera, player);
    if (camera == kCameraChase)
        left = fe::direct<kPickChaseNode, carstate::chaseNodePicked>(guest.app(), guest.ts(), player);
    return left;
}

nfs::u32 simTvSetCamera(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bdb60", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdi };
    const u32 player = args.eax, camera = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::tvSetCamera(app, ts, player, camera); });
}

/* sub_4bdbd0: player eax's next shot: with the director on, the next camera of the
 * list that is allowed; otherwise a new shot of the camera the player's car has. */
nfs::u32 simc::tvNextCamera(App* app, ThreadState& ts, u32 player)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 24);
    const u32 record = player * kDirectorBytes;
    if (guest.dword(kDirectorOn + record) == 0)
        return guest::direct<0x4bdcc0, tvStartShot>(app, ts, player, cameraOfCar(guest, player));
    u32 camera;
    do
    {
        if (app->contextWanted())
            app->yieldContext(ts);
        const u32 next = divide(guest.dword(kDirectorNext + record) + 1, kTvCameraCount).remainder;
        guest.setDword(kDirectorNext + record, next);
        camera = guest.dword(kTvCameras + 4 * next);
    } while (guest.dword(kDirectorAllowed + record + 4 * camera) == 0);
    return guest::direct<0x4bdb60, tvSetCamera>(app, ts, player, camera);
}

nfs::u32 simTvNextCamera(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bdbd0", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 player = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::tvNextCamera(app, ts, player); });
}

/* sub_4bdc70: 1 when player eax's shot has run its length. */
nfs::u32 simc::tvShotOver(App* app, ThreadState& ts, u32 player)
{
    Guest guest(app, ts);
    const u32 record = player * kDirectorBytes;
    const u32 elapsed = guest.dword(kRaceClock) - kClockStart - guest.dword(kDirectorShotStart + record);
    return u32(s32(elapsed) > s32(guest.dword(kDirectorShotLength + record)) ? 1 : 0);
}

nfs::u32 simTvShotOver(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bdc70", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 player = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::tvShotOver(app, ts, player); });
}

/* sub_4bdcc0: player eax's shot begins now, as long as camera edx's are; edx left the player. */
nfs::u32 simc::tvStartShot(App* app, ThreadState& ts, u32 player, u32 camera)
{
    Guest guest(app, ts);
    const u32 record = player * kDirectorBytes;
    guest.setDword(kDirectorShotStart + record, guest.dword(kRaceClock) - kClockStart);
    guest.setDword(kDirectorShotLength + record, camera == kCameraShortShot ? 0xa0 : 0x140);
    return record;
}

nfs::u32 simTvStartShot(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bdcc0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const u32 player = args.eax, camera = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::tvStartShot(app, ts, player, camera); });
}

/* sub_4bdd10: the largest of ebx rows of column eax of the replay's table of what
 * happens when, from row edx on: the two counters left where they stop. */
nfs::u32 simc::tvBusiest(App* app, ThreadState& ts, u32 column, u32 row, u32 count)
{
    constexpr u32 kEvents = 0x7c3f48;
    constexpr u32 kRows = 600;
    Guest guest(app, ts);
    u32 largest = 0;
    for (; s32(row) < s32(kRows) && s32(count) > 0; --count, ++row)
    {
        const u32 value = guest.dword(kEvents + (row << 6) + column * 4);
        if (s32(value) > s32(largest))
            largest = value;
    }
    return largest;
}

nfs::u32 simTvBusiest(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bdd10", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 column = args.eax, row = args.edx, count = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::tvBusiest(app, ts, column, row, count); });
}

/* sub_4bdd70: the director's choice for player eax each frame: by the busiest thing
 * coming in the replay's table, some cameras ruled out, a new shot if the one running
 * has run out or its camera may no longer be used. */
nfs::u32 simc::tvDirect(App* app, ThreadState& ts, u32 player)
{
    constexpr u32 kTicksPerRow = 0x78;
    constexpr u32 kRows = 600;
    constexpr u32 kNoSpot = 0xffffffff;
    constexpr u32 kCarRecord = 4, kCarNoChase = 0x998;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 32);
    const u32 record = player * kDirectorBytes;
    const u32 clock = guest.dword(kRaceClock) - kClockStart;
    const u32 row = divide(s32(clock) < 0 ? 0 : clock, kTicksPerRow).quotient;
    const u32 over = guest::direct<0x4bdc70, tvShotOver>(app, ts, player);
    const u32 column = gameSettings(guest).viewedPlayer;
    if (s32(row) >= s32(kRows))
        return column;
    for (u32 kind = 0; kind < kTvCameraKinds; ++kind)
        guest.setDword(kDirectorAllowed + record + 4 * kind, 1);
    const u32 rows = divide(guest.dword(kDirectorShotLength + record), kTicksPerRow).quotient;
    // Its edx and ebx (where it stopped) are read by nothing here.
    const u32 busiest = guest::direct<0x4bdd10, tvBusiest>(app, ts, column, row, rows);

    const auto rule = [&](std::initializer_list<u32> kinds) {
        for (const u32 kind : kinds)
            guest.setDword(kDirectorAllowed + record + 4 * kind, 0);
    };
    const u32 event = busiest - 3;
    switch (event)
    {
    case 0:
        rule({ 1 });
        break;
    case 1:
        rule({ 10, 12, 13, 11 });
        break;
    case 2:
        rule({ 10, 12, 13, 7 });
        break;
    case 3:
        rule({ 8, 18, 11, 7, 10 });
        break;
    case 4:
        rule({ 11, 10, 12, 13, 9 });
        break;
    case 6:
    {
        const u32 spot = *fe::direct<kNearestSpot, carstate::spotNearestTo>(guest.app(), guest.ts(), player);
        if (spot == kNoSpot && over == 0)
            return spot;
        return guest::direct<0x4bdb60, tvSetCamera>(app, ts, player, kCameraShortShot);
    }
    default:
    {
        if (over == 0)
            return event;
        const u32 car = carOf(guest, player);
        if (guest.dword(guest.dword(car + kCarRecord) + kCarNoChase) != 0)
            guest.setDword(kDirectorAllowed + record + 4 * kCameraChase, 0);
        return guest::direct<0x4bdbd0, tvNextCamera>(app, ts, player);
    }
    }
    const u32 camera = guest.dword(kTvCameras + 4 * guest.dword(kDirectorNext + record));
    if (guest.dword(kDirectorAllowed + record + 4 * camera) != 0 && over == 0)
        return camera;
    return guest::direct<0x4bdbd0, tvNextCamera>(app, ts, player);
}

nfs::u32 simTvDirect(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4bdd70", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 player = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::tvDirect(app, ts, player); });
}

/* ---------------------------------------------------------------------------
 * Small helpers
 * ------------------------------------------------------------------------- */

/* sub_4be000: al 1 for a digit or a sign, 0 for anything else; the rest of eax
 * as it was. */
nfs::u32 simc::isNumberCharacter(App*, ThreadState&, u32 eax)
{
    const u8 character = u8(eax);
    const bool number = (character >= '0' && character <= '9') || character == '-' || character == '+';
    return u32((eax & 0xffffff00u) | (number ? 1u : 0u));
}

nfs::u32 simIsNumberCharacter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4be000", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::isNumberCharacter(app, ts, eax); });
}

/* sub_4be1a0: 0 on one half-tick of every 32 / eax (the clock's, edx further on),
 * 1 on the others. */
nfs::u32 simc::offBeat(App* app, ThreadState& ts, u32 rate, u32 offset)
{
    Guest guest(app, ts);
    const s32 clock = s32(guest.dword(kRaceClock));
    const u32 tick = u32((clock - (clock >> 31)) >> 1) + offset;
    const u32 period = divide(0x20, rate).quotient;
    const u32 remainder = divide(tick, period).remainder;
    return u32(remainder != 0 ? 1 : 0);
}

nfs::u32 simOffBeat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4be1a0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 rate = args.eax, offset = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::offBeat(app, ts, rate, offset); });
}

/* The sirens: a record of 80 bytes per car that has one on. */
constexpr u32 kSirens = 0x7dd850;
constexpr u32 kSirenBytes = 80;

/* sub_4c9dd0: siren eax on, at track node edx, on side ebx (past the sixth the far
 * row of the node's heights), from clock ecx: its two ends and its normal. */
nfs::u32 simc::sirenOn(App* app, ThreadState& ts, u32 index, u32 node, u32 side, u32 clock)
{
    constexpr u32 kUp = 0x4c9a70;       // 0, 1, 0
    constexpr u32 kNodes = 0x5dd948;
    constexpr u32 kNodeBytes = 36;
    constexpr u32 kHeightTimes = 0x540368, kHeightUnit = 0x540370, kHalf = 0x540360;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x5c);
    const u32 up = guest.frame(), line = guest.frame() + 0xc;
    const u32 farEnd = guest.frame() + 0x18, nearEnd = guest.frame() + 0x24;
    copyWords(guest, up, kUp, 3);
    const u32 nodeAt = guest.dword(kNodes) + node * kNodeBytes;
    const u32 raised = u32(guest.byte(nodeAt + (s32(side) < 7 ? 0x17 : 0x1b))) << 15;

    // One end of the node's side, into esi's floats.
    guest::direct<0x4c9bc0, simd::placeInLane>(app, ts, node, side, 0, nearEnd);
    const u32 siren = kSirens + index * kSirenBytes;
    copyWords(guest, siren + 0x28, nearEnd, 3);
    guest::direct<0x4c9bc0, simd::placeInLane>(app, ts, node, side, 1, farEnd);
    copyWords(guest, siren + 0x1c, farEnd, 3);

    const Float height = fp::mul(fp::mul(Float(s32(raised)), loadDouble(guest, kHeightTimes)), loadDouble(guest, kHeightUnit));
    copyWords(guest, siren + 4, siren + 0x28, 3);
    const Float half = loadDouble(guest, kHalf);
    fstpAt(guest, siren + 8, fp::add(fldAt(guest, siren + 8), half));
    copyWords(guest, siren + 0x10, siren + 0x1c, 3);
    guest.setDword(siren, 1);
    const Float farY = fldAt(guest, siren + 0x14), farX = fldAt(guest, siren + 0x10), farZ = fldAt(guest, siren + 0x18);
    fstpAt(guest, siren + 0x48, height);
    guest.setDword(siren + 0x34, node);
    const Float lineX = fp::sub(farX, fldAt(guest, siren + 4));
    const Float lineZ = fp::sub(farZ, fldAt(guest, siren + 0xc));
    fstpAt(guest, line, lineX);
    fstpAt(guest, line + 8, lineZ);
    guest.setDword(siren + 0x38, side);
    const Float raisedY = fp::add(half, farY);
    guest.setDword(siren + 0x4c, clock);
    fstpAt(guest, siren + 0x14, raisedY);
    fstpAt(guest, line + 4, fp::sub(raisedY, fldAt(guest, siren + 8)));

    // The game's code hands it `up` and 1 in edx and ebx too, which it does not read.
    fe::direct<0x4972f0, vertices::normaliseVector>(app, ts, line);
    // Its three registers stay as they are handed, as the code leaves them.
    return guest::direct<0x4e0160, simd::cross>(app, ts, line, up, siren + 0x3c);
}

nfs::u32 simSirenOn(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c9dd0", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 index = args.eax, node = args.edx, side = args.ebx, clock = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::sirenOn(app, ts, index, node, side, clock); });
}

/* sub_4c9f50: siren eax off. */
nfs::u32 simc::sirenOff(App* app, ThreadState& ts, u32 index)
{
    Guest guest(app, ts);
    const u32 offset = index * kSirenBytes;
    guest.setDword(kSirens + offset, 0);
    return offset;
}

nfs::u32 simSirenOff(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4c9f50", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 index = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::sirenOff(app, ts, index); });
}

/* sub_4ea570: the angle of (edx, eax) in 16-bit fixed point, 0x10000 to the turn:
 * an octant's from the table at 0x5679bc, by the 32-bit ratio of the smaller to the
 * larger, interpolated. */
nfs::u32 simc::angle(App* app, ThreadState& ts, u32 y, u32 x)
{
    constexpr u32 kDivide64 = 0x503760;
    constexpr u32 kArcTangents = 0x5679bc;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 16);
    u32 octant = 0;
    if (s32(x) < 0)
    {
        octant = 2;
        x = 0 - x;
    }
    if (s32(y) < 0)
    {
        y = 0 - y;
        octant |= 4;
    }
    u32 angle;
    if (y == x)
    {
        angle = 0x2000;
    }
    else
    {
        if (s32(x) < s32(y))
        {
            const u32 larger = y;
            octant |= 1;
            y = x;
            x = larger;
        }
        const u32 wide = guest.frame();
        // Where the game's two pushes before these calls have the stack.
        ts.sp = wide - 8;
        // The game's code hands it the 64 bits' address in esi: its `out`.
        fe::direct<0x503b00, wina::shiftTo64>(app, ts, y, 0x20, wide);
        ts.sp = wide - 4;
        const u32 ratio = fx::callAt(guest, kDivide64, args({}, { guest.dword(wide), guest.dword(wide + 4), x }));
        const u32 entry = (ratio >> 24) * 4;
        const u32 fraction = (ratio >> 8) & 0xffff;
        const u32 from = guest.dword(kArcTangents + entry);
        const u32 step = ((guest.dword(kArcTangents + entry + 4) - from) * fraction) >> 16;
        angle = from + step;
    }
    switch (octant)
    {
    case 1:
        return u32(0x4000 - angle);
    case 2:
        return u32(0x8000 - angle);
    case 3:
        return u32(angle + 0x4000);
    case 4:
        return u32(0 - angle);
    case 5:
        return u32(angle - 0x4000);
    case 6:
        return u32(angle - 0x8000);
    case 7:
        return u32(0 - angle - 0x4000);
    default:
        return angle;
    }
}

nfs::u32 simAngle(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ea570", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 y = args.eax, x = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::angle(app, ts, y, x); });
}

/* sub_4ea654: how many entries the pack at eax has. */
nfs::u32 simc::packCount(App* app, ThreadState& ts, u32 pack)
{
    Guest guest(app, ts);
    return guest.dword(pack + 8);
}

nfs::u32 simPackCount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ea654", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 pack = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::packCount(app, ts, pack); });
}

/* sub_4ea658: entry edx of the pack at eax, or 0 past its end. */
nfs::u32 simc::packEntry(App* app, ThreadState& ts, u32 pack, u32 entry)
{
    Guest guest(app, ts);
    if (entry >= guest.dword(pack + 8))
        return u32(0);
    return u32(pack + guest.dword(pack + entry * 8 + 0x14));
}

nfs::u32 simPackEntry(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4ea658", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 pack = args.eax, entry = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::packEntry(app, ts, pack, entry); });
}

/* ---------------------------------------------------------------------------
 * The view widget
 * ------------------------------------------------------------------------- */

/* sub_43bcc0: where on the car's interior picture its map starts, by car eax: the
 * car's record says, 0x80 without one. */
nfs::u32 simc::interiorMapX(App* app, ThreadState& ts, u32 car)
{
    constexpr u32 kNoCar = 0x80;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 4);
    const u32 record = fe::direct<0x439a50, load::recordAt>(app, ts, car);
    return record == 0 ? kNoCar : u32(guest.word(record + 0x38c));
}

nfs::u32 viewInteriorMapX(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_43bcc0", 0, 0, nullptr, 0, kRestoreNone };
    const u32 car = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::interiorMapX(app, ts, car); });
}

/* sub_465320: the widget's table's +0x10: the item at eax, shown now, placed: its
 * view of 320 by 192 from where the item stands. */
nfs::u32 simc::viewLaidOut(App* app, ThreadState& ts, u32 item)
{
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 12);
    // The game's code hands it 0x21 in edx too, which it does not read: there as the code has it.
    if (fe::direct<0x44a660, loop::shownNow>(app, ts, item) != 0)
    {
        guest.setWord(item + 0x1e, 0x140);
        guest.setWord(item + 0x20, 0xc0);
        guest.setWord(item + 0x1c, guest.word(item + 6));
        guest.setWord(item + 0x1a, guest.word(item + 8));
    }
    return u32(0);
}

nfs::u32 viewLaidOut(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_465320", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 item = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::viewLaidOut(app, ts, item); });
}

/* sub_465360: the widget's table's +4: nothing. */
nfs::u32 simc::viewNoEntry(App*, ThreadState&, u32 eax)
{
    return eax;
}

nfs::u32 viewNoEntry(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_465360", 0, 0, nullptr, 0, fe::kRestoreAll };
    const u32 eax = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::viewNoEntry(app, ts, eax); });
}

/* sub_465370: the widget's table's +0: the showcase's car's interior picture loaded
 * and the view set back; on the credits' screen the picture by its number. */
nfs::u32 simc::viewLoadCar(App* app, ThreadState& ts)
{
    constexpr u32 kCar = 0x664798, kCarShown = 0x678940;
    constexpr u32 kSprintf = 0x4df690, kStrcmp = 0x4ee310;
    constexpr u32 kFileFormat = 0x53ab3c, kCredits = 0x53ab48, kNumberedFormat = 0x53ab54, kFolderFormat = 0x53ab60;
    constexpr u32 kTraceFile = 0x53ab78, kTraceFormat = 0x53ab9c;
    constexpr u32 kInteriorRoot = 0x7a3234, kCarsRoot = 0x7a28b4;
    constexpr u32 kPanBase = 0x55a438, kMapOffsetX = 0x55a43c, kMapOffsetY = 0x55a440;
    constexpr u32 kPanX = 0x678954, kPanY = 0x678958;
    constexpr float kFullScale = 1.0f;
    Guest guest(app, ts);
    const fe::Frame locals(guest, ts, 0x270);
    const u32 file = guest.frame(), folder = guest.frame() + 0x12c;
    const u32 carFolder = *fe::direct<0x4392b0, load::folderOf>(app, ts, guest.dword(kCar));
    guest.setDword(kCarShown, guest.dword(kCar));
    const u32 stem = fe::direct<0x439290, showcase::carFileStem>(app, ts, guest.dword(kCar));
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), file, kFileFormat, guest::Words{stem});
    const u32 menu = fe::direct<0x44ae00, loop::currentMenuName>(app, ts);
    if (fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.ts(), menu, kCredits) == 0)
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), file, kNumberedFormat, guest::Words{guest.dword(kCar)});
        guest.copyString(folder, kInteriorRoot);
    }
    if (*fe::direct<0x4395c0, load::flag4Of>(guest.app(), guest.ts(), guest.dword(kCar)) != 0)
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.ts(), folder, kFolderFormat, guest::Words{kCarsRoot, carFolder});
    else
        guest.copyString(folder, kInteriorRoot);
    // The game's code hands it the ebx it has and 1.0 in ecx too, which it does not read: there as
    // the code has them.
    fe::direct<0x464700, view::makeTiles>(app, ts, file, folder);
    guest.setDword(kPanBase, bitsOf(kFullScale));
    guest.setDword(kMapOffsetY, 0);
    const u32 mapX = guest::direct<0x43bcc0, interiorMapX>(app, ts, guest.dword(kCar));
    guest.setDword(kMapOffsetX, mapX);
    // The trace (compiled out) with its words pushed as the code pushes them, though it reads
    // none; it leaves eax as it came, the map's x, which is this function's.
    const u32 esp = ts.sp;
    const u32 words[] = { 0x5a, 2, kTraceFormat, kTraceFile, 0x214, mapX };
    for (u32 i = 0; i < 6; ++i)
        guest.setDword(esp - 24 + 4 * i, words[i]);
    ts.sp = esp - 24;
    fe::direct<0x42d510, ea::traceOff>(app, ts);
    ts.sp = esp;
    guest.setDword(kPanY, 0);
    guest.setDword(kPanX, bitsOf(kFullScale));
    return mapX;
}

nfs::u32 viewLoadCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_465370", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return simc::viewLoadCar(app, ts); });
}

}

}
