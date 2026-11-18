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
#include <fpu.h>
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

float zoomHalfWidth(win32::WinApplication* app);  // the port's widescreen change, nfs3hp_main.cpp

namespace
{

using fe::args;
using fe::Guest;
using x86::Float;
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;
using x86::sreg64;

constexpr std::uint8_t kRestoreNone = 0;

/* fld dword, fstp dword. */
Float fldAt(const Guest& guest, reg32 address)
{
    return Float(guest.floatAt(address));
}

void fstpAt(Guest& guest, reg32 address, Float value)
{
    guest.setFloat(address, float(value));
}

/* fld qword: one of the game's constants. */
Float loadDouble(const Guest& guest, reg32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* A float's bits, as mov dword moves it and push hands it on. */
reg32 bitsOf(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

reg32 highWordOf(double value)
{
    std::uint64_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    return reg32(bits >> 32);
}

/* What a jump reads of fcom's status word after sahf: jb (true unordered), ja. */
bool below(reg16 status)
{
    return (status & 0x0100) != 0;
}

bool above(reg16 status)
{
    return (status & 0x4100) == 0;
}

/* fnstsw ax: the word in ax, the rest of eax as it was. */
reg32 withStatus(reg32 eax, reg16 status)
{
    return (eax & 0xffff0000u) | status;
}

void copyWords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* idiv of a sign-extended dividend: both results, as the game's code has them. */
struct Division
{
    reg32 quotient;
    reg32 remainder;
};

Division divide(reg32 dividend, reg32 divisor)
{
    const sreg64 wide = sreg64(sreg32(dividend));
    const sreg32 by = sreg32(divisor);
    return { reg32(wide / by), reg32(wide % by) };
}

/* The game's functions these call. */
constexpr reg32 kArcCosine = 0x4f9482;          // st(0) in, its arc cosine out

/* The car a player's view is on: the replay's if there is one, or the race's. */
constexpr reg32 kReplayCars = 0x5e10b8, kRaceCars = 0x5e10b0;

reg32 carOf(const Guest& guest, reg32 player)
{
    const reg32 replay = guest.dword(kReplayCars + 4 * player);
    return replay != 0 ? replay : guest.dword(kRaceCars + 4 * player);
}

/* The camera a player's car has, 0 without a car. */
reg32 cameraOfCar(const Guest& guest, reg32 player)
{
    const reg32 car = carOf(guest, player);
    return car != 0 ? guest.dword(car) : 0;
}

/* The television director's record per player, and the replay panel's state. */
constexpr reg32 kDirectorBytes = 0x74;
constexpr reg32 kDirectorOn = 0x7cd968;         // the director has the camera
constexpr reg32 kDirectorShotStart = 0x7cd96c;
constexpr reg32 kDirectorNext = 0x7cd970;       // where it is in kTvCameras
constexpr reg32 kDirectorShotLength = 0x7cd974;
constexpr reg32 kDirectorAllowed = 0x7cd978;    // kTvCameraKinds words: this camera may be picked
constexpr reg32 kTvCameraKinds = 25;
constexpr reg32 kTvCameras = 0x55fe68;          // the order they come in
constexpr reg32 kTvCameraCount = 21;
constexpr reg32 kPanelCameras = 0x55fe44;       // what the panel's camera button goes through
constexpr reg32 kPanelCameraCount = 9;
constexpr reg32 kCameraTelevision = 15;
constexpr reg32 kCameraChase = 9;
constexpr reg32 kCameraShortShot = 18;

constexpr reg32 kPanelPaused = 0x7cd948;        // a byte
constexpr reg32 kPanelSpeed = 0x7cd94c;         // 0 to 3
constexpr reg32 kPanelCamera = 0x7cd954;
constexpr reg32 kPanelButton = 0x7cd958;        // 0 to 3, the one highlighted
constexpr reg32 kPanelRepeat = 0x7cd95c;        // frames before the next key counts
constexpr reg32 kPanelCameraChanged = 0x7cd960;
constexpr reg32 kPanelCameraWanted = 0x7cd964;
constexpr reg32 kRaceClock = 0x7d3684;
constexpr reg32 kClockStart = 0x200;            // the clock of a replay's first frame

constexpr reg32 kSwitchCamera = 0x422610, kPickChaseNode = 0x421490;
constexpr reg32 kKeyPressed = 0x48f8d0;
constexpr reg32 kNearestSpot = 0x423d90;

/* The panel's scan codes. */
constexpr reg32 kKeyLeft = 0x4b, kKeyRight = 0x4d, kKeyDown = 0x50, kKeyEnter = 0x1c, kKeyUp = 0x48;

/* The director takes the camera over (the panel's camera kCameraTelevision), for
 * each player: every camera allowed, a shot of the first of the list.  What the last
 * switch left in eax. */
reg32 startTelevision(win32::WinApplication* app, x86::CPU& cpu, Guest& guest, reg32 players)
{
    reg32 left = 0;
    for (reg32 player = 0; player < players; ++player)
    {
        const reg32 record = player * kDirectorBytes;
        guest.setDword(kDirectorOn + record, 1);
        guest.setDword(kDirectorNext + record, 0);
        guest::direct<0x4bdcc0, frontend::simc::tvStartShot>(app, cpu, player, guest.dword(kTvCameras));
        for (reg32 kind = 0; kind < kTvCameraKinds; ++kind)
            guest.setDword(kDirectorAllowed + record + 4 * kind, 1);
        const reg32 camera = guest.dword(kTvCameras + 4 * guest.dword(kDirectorNext + record));
        left = *fe::direct<kSwitchCamera, carstate::cameraSwitched>(guest.app(), guest.cpu(), camera, player);
    }
    return left;
}

/* A float of those bits, as fld dword reads the stack's word. */
Float fromBits(reg32 bits)
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
x86::reg32 simc::poseBlend(App* app, CPU& cpu, reg32 quatA, reg32 quatB, reg32 placeA, reg32 placeB, reg32 tBits,
                           reg32 quatOut, reg32 placeOut)
{
    Guest guest(app, cpu);
    const Float t = fromBits(tBits);
    const Float s = fp::sub(1.0, t);
    for (reg32 i = 0; i < 4; ++i)
    {
        const Float a = fp::mul(fldAt(guest, quatA + 4 * i), s);
        fstpAt(guest, quatOut + 4 * i, fp::add(a, fp::mul(fldAt(guest, quatB + 4 * i), t)));
    }
    for (reg32 i = 0; i < 2; ++i)
    {
        const Float a = fp::mul(fldAt(guest, placeA + 4 * i), s);
        fstpAt(guest, placeOut + 4 * i, fp::add(a, fp::mul(fldAt(guest, placeB + 4 * i), t)));
    }
    const Float a = fp::mul(s, fldAt(guest, placeA + 8));
    fstpAt(guest, placeOut + 8, fp::add(a, fp::mul(t, fldAt(guest, placeB + 8))));
    return quatA;
}

x86::reg32 simPoseBlend(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b70f0", 12, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 quatA = args.eax, quatB = args.edx, placeA = args.ebx, placeB = args.ecx;
    const reg32 t = guest::at(app, cpu.esp + 4), quatOut = guest::at(app, cpu.esp + 8), placeOut = guest::at(app, cpu.esp + 12);
    return fe::runThunk(app, cpu, args, site,
                 [&](Guest&) { return simc::poseBlend(app, cpu, quatA, quatB, placeA, placeB, t, quatOut, placeOut); });
}

/* sub_4b71e0: quaternion slerp: eax and edx mixed by the float on the stack, into
 * the pointer after it.  The second is negated first when they are more than a
 * quarter turn apart, and two closer than kNear in cosine are mixed linearly. */
x86::reg32 simc::quatSlerp(App* app, CPU& cpu, reg32 from, reg32 to, reg32 tBits, reg32 out)
{
    constexpr reg32 kNear = 0x53ff60;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x50);
    Float dot = fp::mul(fldAt(guest, from + 4), fldAt(guest, to + 4));
    dot = fp::add(dot, fp::mul(fldAt(guest, from), fldAt(guest, to)));
    dot = fp::add(dot, fp::mul(fldAt(guest, from + 8), fldAt(guest, to + 8)));
    dot = fp::add(dot, fp::mul(fldAt(guest, from + 12), fldAt(guest, to + 12)));
    double cosine = dot;
    const bool negated = above(fp::compare(0.0, cosine));
    float flipped[4] = {};
    if (negated)
    {
        for (reg32 i = 0; i < 4; ++i)
            flipped[i] = float(-fldAt(guest, to + 4 * i));
        cosine = -cosine;
    }
    const auto target = [&](reg32 i) { return negated ? Float(flipped[i]) : fldAt(guest, to + 4 * i); };

    const reg16 status = fp::compare(fp::sub(1.0, cosine), loadDouble(guest, kNear));
    reg32 left = withStatus(from, status);
    double fromWeight, toWeight;
    if (above(status))
    {
        x86::Args arc;
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
    for (reg32 i = 0; i < 3; ++i)
    {
        const Float a = fp::mul(fldAt(guest, from + 4 * i), fromWeight);
        fstpAt(guest, out + 4 * i, fp::add(a, fp::mul(target(i), toWeight)));
    }
    const Float a = fp::mul(fromWeight, fldAt(guest, from + 12));
    fstpAt(guest, out + 12, fp::add(a, fp::mul(toWeight, target(3))));
    return left;
}

x86::reg32 simQuatSlerp(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b71e0", 8, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 from = args.eax, to = args.edx, t = guest::at(app, cpu.esp + 4), out = guest::at(app, cpu.esp + 8);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::quatSlerp(app, cpu, from, to, t, out); });
}

/* sub_4b7300: the quaternion at edx negated if its negative is nearer the one at eax:
 * the squared length of the sum against that of the difference.  The signs go byte
 * by byte, not through the x87. */
x86::reg32 simc::quatNearer(App* app, CPU& cpu, reg32 a, reg32 b)
{
    Guest guest(app, cpu);
    /* A value by itself, the second factor through its float, as the game keeps it. */
    const auto squared = [](Float value) { return fp::mul(value, Float(float(value))); };
    const auto sumOf = [&](reg32 offset) { return squared(fp::add(fldAt(guest, a + offset), fldAt(guest, b + offset))); };
    const auto differenceOf = [&](reg32 offset) { return squared(fp::sub(fldAt(guest, a + offset), fldAt(guest, b + offset))); };
    Float differences = fp::add(differenceOf(4), differenceOf(0));
    Float sums = fp::add(sumOf(4), sumOf(0));
    sums = fp::add(sums, sumOf(8));
    differences = fp::add(differences, differenceOf(8));
    sums = fp::add(sums, sumOf(12));
    differences = fp::add(differences, differenceOf(12));
    const reg16 status = fp::compare(sums, differences);
    reg32 left = withStatus(a, status);
    if (below(status))
    {
        const reg8 highest = reg8(guest.byte(b + 3) ^ 0x80);
        guest.setByte(b + 7, reg8(guest.byte(b + 7) ^ 0x80));
        guest.setByte(b + 15, reg8(guest.byte(b + 15) ^ 0x80));
        guest.setByte(b + 3, highest);
        guest.setByte(b + 11, reg8(guest.byte(b + 11) ^ 0x80));
        left = (left & 0xffff00ffu) | (reg32(highest) << 8);
    }
    return left;
}

x86::reg32 simQuatNearer(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b7300", 0, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 a = args.eax, b = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::quatNearer(app, cpu, a, b); });
}

/* sub_4b73c0: the 3x3 matrix at eax made the quaternion at edx: by its trace when
 * that is above zero, otherwise from its largest diagonal element. */
x86::reg32 simc::quatFromMatrix(App* app, CPU& cpu, reg32 matrix, reg32 out)
{
    constexpr reg32 kHalf = 0x53ff68;
    static const reg32 kNextAxis[3] = { 1, 2, 0 };  // the table at 0x4b6a1c
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x58);
    const auto element = [&](reg32 row, reg32 column) { return matrix + 12 * row + 4 * column; };
    const auto rootOf = [&](Float value) {
        return Float(guest::direct<0x4ea920, squareRootOf>(app, cpu, bitsOf(float(value))));
    };

    const Float sum = fp::add(fp::add(fldAt(guest, element(0, 0)), fldAt(guest, element(1, 1))), fldAt(guest, element(2, 2)));
    const float trace = float(sum);
    const reg16 status = fp::compare(0.0, Float(trace));
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

    reg32 i = 0;
    if (above(fp::compare(fldAt(guest, element(1, 1)), fldAt(guest, element(0, 0)))))
        i = 1;
    if (above(fp::compare(fldAt(guest, element(2, 2)), fldAt(guest, element(i, i)))))
        i = 2;
    const reg32 j = kNextAxis[i], k = kNextAxis[j];
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

x86::reg32 simQuatFromMatrix(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b73c0", 0, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 matrix = args.eax, out = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::quatFromMatrix(app, cpu, matrix, out); });
}

/* sub_4b7590: the quaternion at eax made the 3x3 matrix at edx. */
x86::reg32 simc::matrixFromQuat(App* app, CPU& cpu, reg32 quat, reg32 out)
{
    constexpr reg32 kTwo = 0x53ff6c;
    Guest guest(app, cpu);
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

x86::reg32 simMatrixFromQuat(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b7590", 0, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 quat = args.eax, out = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::matrixFromQuat(app, cpu, quat, out); });
}

/* sub_4b7680: two 3x3 matrices, eax and edx, slerped through quaternions by the
 * float on the stack, into the pointer after it. */
x86::reg32 simc::matrixSlerp(App* app, CPU& cpu, reg32 from, reg32 to, reg32 t, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x3c);
    const reg32 mixed = guest.frame(), toQuat = guest.frame() + 0x10, fromQuat = guest.frame() + 0x20;
    guest::direct<0x4b73c0, quatFromMatrix>(app, cpu, from, fromQuat);
    guest::direct<0x4b73c0, quatFromMatrix>(app, cpu, to, toQuat);
    guest::direct<0x4b71e0, quatSlerp>(app, cpu, fromQuat, toQuat, t, mixed);
    // `out` in edx stays there after it, as the code leaves it.
    return guest::direct<0x4b7590, matrixFromQuat>(app, cpu, mixed, out);
}

x86::reg32 simMatrixSlerp(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b7680", 8, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const reg32 from = args.eax, to = args.edx, t = guest::at(app, cpu.esp + 4), out = guest::at(app, cpu.esp + 8);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::matrixSlerp(app, cpu, from, to, t, out); });
}

/* sub_4b7720: the 3x3 matrix at ebx is eax minus edx. */
x86::reg32 simc::matrixSubtract(App* app, CPU& cpu, reg32 a, reg32 b, reg32 out)
{
    Guest guest(app, cpu);
    for (reg32 at = 0; at < 36; at += 4)
        fstpAt(guest, out + at, fp::sub(fldAt(guest, a + at), fldAt(guest, b + at)));
    return a;
}

x86::reg32 simMatrixSubtract(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b7720", 0, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 a = args.eax, b = args.edx, out = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::matrixSubtract(app, cpu, a, b, out); });
}

/* sub_4b7780: the 3x3 matrix at ebx is eax plus edx. */
x86::reg32 simc::matrixAdd(App* app, CPU& cpu, reg32 a, reg32 b, reg32 out)
{
    Guest guest(app, cpu);
    for (reg32 at = 0; at < 36; at += 4)
        fstpAt(guest, out + at, fp::add(fldAt(guest, a + at), fldAt(guest, b + at)));
    return a;
}

x86::reg32 simMatrixAdd(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b7780", 0, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 a = args.eax, b = args.edx, out = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::matrixAdd(app, cpu, a, b, out); });
}

/* sub_4b77e0: the 3x3 matrix at eax times the float on the stack, into the pointer
 * after it. */
x86::reg32 simc::matrixScale(App* app, CPU& cpu, reg32 matrix, reg32 factorBits, reg32 out)
{
    Guest guest(app, cpu);
    const Float factor = fromBits(factorBits);
    for (reg32 at = 0; at < 32; at += 4)
        fstpAt(guest, out + at, fp::mul(fldAt(guest, matrix + at), factor));
    fstpAt(guest, out + 32, fp::mul(factor, fldAt(guest, matrix + 32)));
    return matrix;
}

x86::reg32 simMatrixScale(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b77e0", 8, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 matrix = args.eax, factor = guest::at(app, cpu.esp + 4), out = guest::at(app, cpu.esp + 8);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::matrixScale(app, cpu, matrix, factor, out); });
}

/* sub_4b7840: the 3x3 matrix at eax made orthonormal again: M = M (M'M)^-1/2, the
 * root by edx terms -- 10 at the least -- of its binomial series, with coefficients
 * at 0x55fdcc. */
x86::reg32 simc::matrixOrthonormalise(App* app, CPU& cpu, reg32 matrix, reg32 termsAsked)
{
    constexpr reg32 kIdentity = 0x564498;
    constexpr reg32 kCoefficients = 0x55fdcc;
    constexpr reg32 kWords = 9;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0xd4);
    const reg32 terms = sreg32(termsAsked) > 10 ? termsAsked : 10;
    const reg32 identity = guest.frame(), term = guest.frame() + 0x24, excess = guest.frame() + 0x48;
    const reg32 power = guest.frame() + 0x6c, sum = guest.frame() + 0x90;
    const float limit = float(Float(sreg32(terms)));

    // The game's code hands the transpose `term` in ebx too, which it does not read.
    guest::direct<0x4e0110, simd::transpose>(app, cpu, matrix, term);
    fe::direct<0x4e0280, vertices::multiplyMatrix>(app, cpu, term, matrix, term);
    copyWords(guest, identity, kIdentity, kWords);
    guest::direct<0x4b7720, matrixSubtract>(app, cpu, term, identity, excess);
    copyWords(guest, power, kIdentity, kWords);
    copyWords(guest, sum, power, kWords);
    for (reg32 i = 1; below(fp::compare(Float(sreg32(i)), Float(limit))); ++i)
    {
        fe::direct<0x4e0280, vertices::multiplyMatrix>(app, cpu, power, excess, power);
        // The game's code hands it `term` and `sum` in edx and ebx too, which it does not read.
        guest::direct<0x4b77e0, matrixScale>(app, cpu, power, guest.dword(kCoefficients + 4 * i), term);
        guest::direct<0x4b7780, matrixAdd>(app, cpu, sum, term, sum);
    }
    return fe::direct<0x4e0280, vertices::multiplyMatrixEax>(app, cpu, matrix, sum, matrix);
}

x86::reg32 simMatrixOrthonormalise(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b7840", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 matrix = args.eax, terms = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::matrixOrthonormalise(app, cpu, matrix, terms); });
}

/* sub_4b79c0: the 3x3 matrix at eax turned a quarter: the second row is the third,
 * the third the second's negative. */
x86::reg32 simc::matrixQuarterTurn(App* app, CPU& cpu, reg32 matrix)
{
    Guest guest(app, cpu);
    for (reg32 at = 0; at < 12; at += 4)
    {
        const Float second = fldAt(guest, matrix + 12 + at);
        const Float third = fldAt(guest, matrix + 24 + at);
        fstpAt(guest, matrix + 12 + at, third);
        fstpAt(guest, matrix + 24 + at, -second);
    }
    return matrix;
}

x86::reg32 simMatrixQuarterTurn(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4b79c0", 0, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 matrix = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::matrixQuarterTurn(app, cpu, matrix); });
}

/* sub_4ea680: the 3x3 matrix at eax turned about its first row, by the angle on the
 * stack (handed on by its address), into the pointer after it, which may be the matrix
 * itself. */
x86::reg32 simc::turnAboutFirstRow(App* app, CPU& cpu, reg32 matrix, reg32 angleAt, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x44);
    reg32 source = matrix;
    if (source == out)
    {
        copyWords(guest, guest.frame(), out, 9);
        source = guest.frame();
    }
    guest::direct<0x4ea970, simd::sineCosine>(app, cpu, angleAt, guest.frame() + 0x24, guest.frame() + 0x28);
    const Float sine = fldAt(guest, guest.frame() + 0x24), cosine = fldAt(guest, guest.frame() + 0x28);
    guest.setDword(out, guest.dword(source));
    guest.setDword(out + 4, guest.dword(source + 4));
    const reg32 left = guest.dword(source + 8);
    guest.setDword(out + 8, left);
    for (reg32 at = 0; at < 12; at += 4)
    {
        const Float a = fp::mul(fldAt(guest, source + 12 + at), cosine);
        fstpAt(guest, out + 12 + at, fp::sub(a, fp::mul(fldAt(guest, source + 24 + at), sine)));
    }
    for (reg32 at = 0; at < 12; at += 4)
    {
        const Float a = fp::mul(fldAt(guest, source + 24 + at), cosine);
        fstpAt(guest, out + 24 + at, fp::add(a, fp::mul(fldAt(guest, source + 12 + at), sine)));
    }
    return left;
}

x86::reg32 simTurnAboutFirstRow(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4ea680", 8, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 matrix = args.eax, angleAt = cpu.esp + 4, out = guest::at(app, cpu.esp + 8);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::turnAboutFirstRow(app, cpu, matrix, angleAt, out); });
}

/* sub_4ea820: the 3x3 matrix at eax turned about its last row, as sub_4ea680. */
x86::reg32 simc::turnAboutLastRow(App* app, CPU& cpu, reg32 matrix, reg32 angleAt, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x44);
    reg32 source = matrix;
    if (source == out)
    {
        copyWords(guest, guest.frame(), out, 9);
        source = guest.frame();
    }
    guest::direct<0x4ea970, simd::sineCosine>(app, cpu, angleAt, guest.frame() + 0x24, guest.frame() + 0x28);
    const Float sine = fldAt(guest, guest.frame() + 0x24), cosine = fldAt(guest, guest.frame() + 0x28);
    for (reg32 at = 0; at < 12; at += 4)
    {
        const Float a = fp::mul(fldAt(guest, source + at), cosine);
        fstpAt(guest, out + at, fp::sub(a, fp::mul(fldAt(guest, source + 12 + at), sine)));
    }
    for (reg32 at = 0; at < 12; at += 4)
    {
        const Float a = fp::mul(fldAt(guest, source + 12 + at), cosine);
        fstpAt(guest, out + 12 + at, fp::add(a, fp::mul(fldAt(guest, source + at), sine)));
    }
    guest.setDword(out + 24, guest.dword(source + 24));
    guest.setDword(out + 28, guest.dword(source + 28));
    const reg32 left = guest.dword(source + 32);
    guest.setDword(out + 32, left);
    return left;
}

x86::reg32 simTurnAboutLastRow(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4ea820", 8, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 matrix = args.eax, angleAt = cpu.esp + 4, out = guest::at(app, cpu.esp + 8);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::turnAboutLastRow(app, cpu, matrix, angleAt, out); });
}

/* sub_4ea920: the square root of the float on the stack, onto the x87's. */
double simc::squareRootOf(App*, reg32 value)
{
    return fp::sqrt(fromBits(value));
}

x86::reg32 simSquareRoot(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4ea920", 4, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 eax = args.eax, value = guest::at(app, cpu.esp + 4);
    return fe::runThunk(app, cpu, args, site, [&](Guest&) {
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
x86::reg32 simc::viewPass(App* app, CPU& cpu, reg32 pass, reg32 record4, reg32 record68, reg32 record6c,
                          reg32 record70)
{
    constexpr reg32 kView = 0x7a3d14;
    constexpr reg32 kChooseView = 0x421450, kSetMirrored = 0x4349a0, kAmbientSounds = 0x41bb50;
    constexpr reg32 kRaceFrame = 0x4bc160, kDrawMirrored = 0x4349d0;
    constexpr reg32 kFirstShare = 0x560094, kSecondShare = 0x560098;
    constexpr float kFourFifths = 0.8f, kOneFifth = 0.2f;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 12);
    fe::direct<0x4bbd70, rendd::arenaRewind>(app, cpu);
    guest.setDword(kView, pass);
    guest.setDword(kView + 4, record4);
    guest.setDword(kView + 0x68, record68);
    guest.setDword(kView + 0x70, record70);
    guest.setDword(kView + 0x6c, record6c);
    fe::direct<kChooseView, carstate::cameraViewChosen>(guest.app(), guest.cpu(), kView, 0);
    fe::direct<kSetMirrored, collide::setViewMirrored>(guest.app(), guest.cpu(), kView);
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
    fe::direct<kAmbientSounds, track::ambientAround>(guest.app(), guest.cpu(), kView);
    fe::direct<0x4bc160, graphics::raceFrameDrawn>(app, cpu, pass);
    fe::direct<kChooseView, carstate::cameraViewChosen>(guest.app(), guest.cpu(), kView, 2);
    const reg32 first = guest.dword(0x7a3cf0), second = guest.dword(0x7a3cf4);
    (void)second;
    fe::direct<kDrawMirrored, collide::listDrawnMirrored>(app, cpu, kView, first);
    return kView;
}

x86::reg32 simViewPass(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bc470", 4, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 pass = args.eax, record4 = args.edx, record68 = args.ebx, record6c = args.ecx;
    const reg32 record70 = guest::at(app, cpu.esp + 4);
    return fe::runThunk(app, cpu, args, site,
                 [&](Guest&) { return simc::viewPass(app, cpu, pass, record4, record68, record6c, record70); });
}

/* sub_4dbde0: the projection of the view at eax: the first's own, or each player's,
 * and a racing zoom camera's scales (the port's zoomHalfWidth keeps pixels square on
 * a wide view). */
x86::reg32 simc::setView(App* app, CPU& cpu, reg32 view)
{
    constexpr reg32 kCarZoom = 0x54;
    constexpr reg32 kScaleX = 0x56009c, kScaleY = 0x5600a0, kHalfHeight = 0x5600b0, kAspect = 0x54972c;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 20);
    if (guest.dword(view) == 1)
    {
        const reg32 notMirrored = gameSettings(guest).mirrored == 0 ? 1 : 0;
        const reg32 fov = fe::direct<0x4dbcc0, rende::defaultFieldOfView>(app, cpu);
        return fe::direct<0x4bed20, loopb::useWithProjection>(app, cpu, 0x7cdb48, fov, notMirrored);
    }
    const bool split = gameSettings(guest).playMode == 1;
    const reg32 mirrored = gameSettings(guest).mirrored;
    const reg32 fov = fe::direct<0x4dbcc0, rende::defaultFieldOfView>(app, cpu);
    const reg32 viewport = split ? 0x7cda80 + (guest.dword(view + 4) << 5) : 0x7cdb88;
    fe::direct<0x4bed20, loopb::useWithProjection>(app, cpu, viewport, fov, mirrored);

    reg32 car = carOf(guest, guest.dword(view + 4));
    if ((guest.dword(car + kCarZoom) & 0x7fffffffu) == 0)
        return car;
    car = carOf(guest, guest.dword(view + 4));
    const Float scaleX = fp::mul(Float(zoomHalfWidth(app)), fldAt(guest, car + kCarZoom));
    car = carOf(guest, guest.dword(view + 4));
    fstpAt(guest, kScaleX, scaleX);
    const Float scaleY = fp::mul(fp::mul(-fldAt(guest, kHalfHeight), fldAt(guest, car + kCarZoom)),
                                 loadDouble(guest, kAspect));
    const reg32 flip = gameSettings(guest).mirrored;
    fstpAt(guest, kScaleY, scaleY);
    if (flip != 0)
        guest.setByte(kScaleX + 3, reg8(guest.byte(kScaleX + 3) ^ 0x80));
    return car;
}

x86::reg32 simSetView(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dbde0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    const reg32 view = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::setView(app, cpu, view); });
}

/* sub_4d5720: nothing of its own: the race frame's end, sub_434a90, every register as it
 * came; by address, for what it leaves in eax is this function's. */
x86::reg32 simc::raceFrameEnd(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    return fe::direct<0x434a90, collide::fogColour>(app, cpu) ? 1 : 0;
}

x86::reg32 simRaceFrameEnd(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4d5720", 0, 0, nullptr, 0, kRestoreNone };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::raceFrameEnd(app, cpu); });
}

/* ---------------------------------------------------------------------------
 * The replay's panel
 * ------------------------------------------------------------------------- */

/* sub_4bd4d0: the pointer on the replay's panel: inside it the button under it is
 * highlighted; a press inside, no sooner than 0x20 ticks after the last, is the
 * buttons' word; anything else 0. */
x86::reg32 simc::panelPointer(App* app, CPU& cpu)
{
    constexpr reg32 kPanelLeft = 0x7497d8, kPanelTop = 0x7497dc, kPanelRight = 0x7497e0, kPanelBottom = 0x7497e4;
    constexpr reg32 kFirstEnd = 0x540198, kSecondEnd = 0x5401a0, kThirdEnd = 0x5401a8;
    constexpr reg32 kTicks = 0x79c1a4, kNextPress = 0x55febc;
    constexpr reg32 kPressGap = 0x20;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x54);
    const auto floatOfInteger = [](reg32 value) { return float(Float(sreg32(value))); };
    const float left = floatOfInteger(guest.dword(kPanelLeft));
    const float right = floatOfInteger(guest.dword(kPanelRight));
    const float top = floatOfInteger(guest.dword(kPanelTop));
    const float bottom = floatOfInteger(guest.dword(kPanelBottom));
    const reg32 buttons = guest.frame() + 0xc, mouseX = guest.frame() + 0x10, mouseY = guest.frame() + 0x14;
    const reg32 hotX = guest.frame() + 0x18, hotY = guest.frame() + 0x1c;
    fe::direct<0x435310, controls::mouseRead>(app, cpu, buttons, mouseX, mouseY, 0);
    fe::direct<0x4dab10, menue::pointerHotSpot>(app, cpu, hotX, hotY);
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
        reg32 button = 3;
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
        const reg32 ticks = guest.dword(kTicks);
        if (sreg32(ticks) > sreg32(guest.dword(kNextPress)))
        {
            guest.setDword(kNextPress, ticks + kPressGap);
            return guest.dword(buttons);
        }
    }
    return reg32(0);
}

x86::reg32 simPanelPointer(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bd4d0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::panelPointer(app, cpu); });
}

/* sub_4bd640: the replay's panel each frame: a camera asked for last frame made
 * current for each player, then one key: left and right move the highlight, down
 * takes the button a step back -- the camera before, the speed below -- and Enter,
 * up or a click a step on, or leaves the replay; both toggle the pause. */
x86::reg32 simc::panelKeys(App* app, CPU& cpu)
{
    constexpr reg32 kDirectorWanted = 0x7d36a8;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24);
    const auto pressed = [&](reg32 key) { return fe::direct<0x48f8d0, controls::keyDownNow>(app, cpu, key, 1); };
    const reg32 players = gameSettings(guest).playMode == 1 ? 2 : 1;
    reg32 left = 0;

    const reg32 wanted = guest.dword(kPanelCameraWanted);
    if (sreg32(wanted) > 0 && guest.dword(kPanelCameraChanged) != 0)
    {
        guest.setDword(kPanelCamera, wanted);
        for (reg32 player = 0; player < players; ++player)
        {
            guest.setDword(kDirectorOn + player * kDirectorBytes, 0);
            fe::direct<kSwitchCamera, carstate::cameraSwitched>(guest.app(), guest.cpu(), guest.dword(kPanelCameras + 4 * guest.dword(kPanelCamera)), player);
            left = guest.dword(kPanelCamera);
            if (guest.dword(kPanelCameras + 4 * left) == kCameraChase)
                left = fe::direct<kPickChaseNode, carstate::chaseNodePicked>(guest.app(), guest.cpu(), player);
        }
    }

    const reg32 repeat = guest.dword(kPanelRepeat);
    guest.setDword(kPanelCameraChanged, 0);
    if (sreg32(repeat) > 0)
    {
        guest.setDword(kPanelRepeat, repeat - 1);
    }
    else if (pressed(kKeyLeft) != 0)
    {
        const reg32 button = guest.dword(kPanelButton);
        if (sreg32(button) > 0)
            guest.setDword(kPanelButton, button - 1);
        left = fe::direct<0x4bd3f0, loopb::pointerAtChoice>(app, cpu, guest.dword(kPanelButton));
    }
    else if (pressed(kKeyRight) != 0)
    {
        const reg32 button = guest.dword(kPanelButton);
        if (sreg32(button) < 3)
            guest.setDword(kPanelButton, button + 1);
        left = fe::direct<0x4bd3f0, loopb::pointerAtChoice>(app, cpu, guest.dword(kPanelButton));
    }
    else if (pressed(kKeyDown) != 0)
    {
        const reg32 button = guest.dword(kPanelButton);
        guest.setDword(kPanelRepeat, 8);
        left = button;
        if (button == 0)
        {
            const reg32 previous = guest.dword(kPanelCamera) - 1;
            guest.setDword(kPanelCameraChanged, 1);
            guest.setDword(kPanelCamera, previous);
            if (sreg32(previous) < 0)
                guest.setDword(kPanelCamera, kPanelCameraCount - 1);
            const reg32 camera = guest.dword(kPanelCamera);
            guest.setDword(kPanelCameraWanted, camera);
            left = camera;
            if (guest.dword(kPanelCameras + 4 * camera) == kCameraTelevision)
                left = startTelevision(app, cpu, guest, players);
        }
        else if (button == 1)
        {
            const reg32 speed = guest.dword(kPanelSpeed) - 1;
            guest.setDword(kPanelSpeed, speed);
            if (sreg32(speed) < 0)
                guest.setDword(kPanelSpeed, 3);
        }
        else if (button == 2)
        {
            guest.setByte(kPanelPaused, reg8(guest.byte(kPanelPaused) ^ 1));
        }
    }
    else if (pressed(kKeyEnter) != 0 || pressed(kKeyUp) != 0 || guest::direct<0x4bd4d0, panelPointer>(app, cpu) != 0)
    {
        const reg32 button = guest.dword(kPanelButton);
        guest.setDword(kPanelRepeat, 8);
        left = button;
        if (button == 0)
        {
            const Division next = divide(guest.dword(kPanelCamera) + 1, kPanelCameraCount);
            guest.setDword(kPanelCameraChanged, 1);
            guest.setDword(kPanelCamera, next.remainder);
            const reg32 camera = guest.dword(kPanelCameras + 4 * next.remainder);
            guest.setDword(kPanelCameraWanted, next.remainder);
            left = next.quotient;
            if (camera == kCameraTelevision)
                left = startTelevision(app, cpu, guest, players);
        }
        else if (button == 1)
        {
            const Division next = divide(guest.dword(kPanelSpeed) + 1, 4);
            guest.setDword(kPanelSpeed, next.remainder);
            left = next.quotient;
        }
        else if (button == 2)
        {
            guest.setByte(kPanelPaused, reg8(guest.byte(kPanelPaused) ^ 1));
        }
        else if (button == 3)
        {
            guest.setDword(kPanelRepeat, 0x18);
            fe::direct<0x451960, port::text::menuKeyOf>(app, cpu, cpu.esp - 4, 1);
            // The game's code leaves the key in eax for it, which it does not read.
            left = fe::direct<0x4c4d80, loopb::stop>(app, cpu);
        }
    }
    else
    {
        left = 0;
    }

    // The game's code hands it 0 in eax, which it does not read: there as the code has it.
    if (guest.dword(kDirectorWanted) != 0)
    {
        left = guest::direct<0x426680, sima::camerasStep>(app, cpu);
    }
    return left;
}

x86::reg32 simPanelKeys(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bd640", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::panelKeys(app, cpu); });
}

/* ---------------------------------------------------------------------------
 * The television director
 * ------------------------------------------------------------------------- */

/* sub_4bdb60: player eax's camera made edx, and a new shot begun whatever it was. */
x86::reg32 simc::tvSetCamera(App* app, CPU& cpu, reg32 player, reg32 camera)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 16);
    guest::direct<0x4bdcc0, tvStartShot>(app, cpu, player, camera);
    const reg32 current = cameraOfCar(guest, player);
    if (current == camera)
        return current;
    reg32 left = *fe::direct<kSwitchCamera, carstate::cameraSwitched>(guest.app(), guest.cpu(), camera, player);
    if (camera == kCameraChase)
        left = fe::direct<kPickChaseNode, carstate::chaseNodePicked>(guest.app(), guest.cpu(), player);
    return left;
}

x86::reg32 simTvSetCamera(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bdb60", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdi };
    const reg32 player = args.eax, camera = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::tvSetCamera(app, cpu, player, camera); });
}

/* sub_4bdbd0: player eax's next shot: with the director on, the next camera of the
 * list that is allowed; otherwise a new shot of the camera the player's car has. */
x86::reg32 simc::tvNextCamera(App* app, CPU& cpu, reg32 player)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24);
    const reg32 record = player * kDirectorBytes;
    if (guest.dword(kDirectorOn + record) == 0)
        return guest::direct<0x4bdcc0, tvStartShot>(app, cpu, player, cameraOfCar(guest, player));
    reg32 camera;
    do
    {
        if (app->contextWanted())
            app->yieldContext(cpu);
        const reg32 next = divide(guest.dword(kDirectorNext + record) + 1, kTvCameraCount).remainder;
        guest.setDword(kDirectorNext + record, next);
        camera = guest.dword(kTvCameras + 4 * next);
    } while (guest.dword(kDirectorAllowed + record + 4 * camera) == 0);
    return guest::direct<0x4bdb60, tvSetCamera>(app, cpu, player, camera);
}

x86::reg32 simTvNextCamera(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bdbd0", 0, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 player = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::tvNextCamera(app, cpu, player); });
}

/* sub_4bdc70: 1 when player eax's shot has run its length. */
x86::reg32 simc::tvShotOver(App* app, CPU& cpu, reg32 player)
{
    Guest guest(app, cpu);
    const reg32 record = player * kDirectorBytes;
    const reg32 elapsed = guest.dword(kRaceClock) - kClockStart - guest.dword(kDirectorShotStart + record);
    return reg32(sreg32(elapsed) > sreg32(guest.dword(kDirectorShotLength + record)) ? 1 : 0);
}

x86::reg32 simTvShotOver(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bdc70", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 player = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::tvShotOver(app, cpu, player); });
}

/* sub_4bdcc0: player eax's shot begins now, as long as camera edx's are; edx left the player. */
x86::reg32 simc::tvStartShot(App* app, CPU& cpu, reg32 player, reg32 camera)
{
    Guest guest(app, cpu);
    const reg32 record = player * kDirectorBytes;
    guest.setDword(kDirectorShotStart + record, guest.dword(kRaceClock) - kClockStart);
    guest.setDword(kDirectorShotLength + record, camera == kCameraShortShot ? 0xa0 : 0x140);
    return record;
}

x86::reg32 simTvStartShot(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bdcc0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const reg32 player = args.eax, camera = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::tvStartShot(app, cpu, player, camera); });
}

/* sub_4bdd10: the largest of ebx rows of column eax of the replay's table of what
 * happens when, from row edx on: the two counters left where they stop. */
x86::reg32 simc::tvBusiest(App* app, CPU& cpu, reg32 column, reg32 row, reg32 count)
{
    constexpr reg32 kEvents = 0x7c3f48;
    constexpr reg32 kRows = 600;
    Guest guest(app, cpu);
    reg32 largest = 0;
    for (; sreg32(row) < sreg32(kRows) && sreg32(count) > 0; --count, ++row)
    {
        const reg32 value = guest.dword(kEvents + (row << 6) + column * 4);
        if (sreg32(value) > sreg32(largest))
            largest = value;
    }
    return largest;
}

x86::reg32 simTvBusiest(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bdd10", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 column = args.eax, row = args.edx, count = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::tvBusiest(app, cpu, column, row, count); });
}

/* sub_4bdd70: the director's choice for player eax each frame: by the busiest thing
 * coming in the replay's table, some cameras ruled out, a new shot if the one running
 * has run out or its camera may no longer be used. */
x86::reg32 simc::tvDirect(App* app, CPU& cpu, reg32 player)
{
    constexpr reg32 kTicksPerRow = 0x78;
    constexpr reg32 kRows = 600;
    constexpr reg32 kNoSpot = 0xffffffff;
    constexpr reg32 kCarRecord = 4, kCarNoChase = 0x998;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 32);
    const reg32 record = player * kDirectorBytes;
    const reg32 clock = guest.dword(kRaceClock) - kClockStart;
    const reg32 row = divide(sreg32(clock) < 0 ? 0 : clock, kTicksPerRow).quotient;
    const reg32 over = guest::direct<0x4bdc70, tvShotOver>(app, cpu, player);
    const reg32 column = gameSettings(guest).viewedPlayer;
    if (sreg32(row) >= sreg32(kRows))
        return column;
    for (reg32 kind = 0; kind < kTvCameraKinds; ++kind)
        guest.setDword(kDirectorAllowed + record + 4 * kind, 1);
    const reg32 rows = divide(guest.dword(kDirectorShotLength + record), kTicksPerRow).quotient;
    // Its edx and ebx (where it stopped) are read by nothing here.
    const reg32 busiest = guest::direct<0x4bdd10, tvBusiest>(app, cpu, column, row, rows);

    const auto rule = [&](std::initializer_list<reg32> kinds) {
        for (const reg32 kind : kinds)
            guest.setDword(kDirectorAllowed + record + 4 * kind, 0);
    };
    const reg32 event = busiest - 3;
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
        const reg32 spot = *fe::direct<kNearestSpot, carstate::spotNearestTo>(guest.app(), guest.cpu(), player);
        if (spot == kNoSpot && over == 0)
            return spot;
        return guest::direct<0x4bdb60, tvSetCamera>(app, cpu, player, kCameraShortShot);
    }
    default:
    {
        if (over == 0)
            return event;
        const reg32 car = carOf(guest, player);
        if (guest.dword(guest.dword(car + kCarRecord) + kCarNoChase) != 0)
            guest.setDword(kDirectorAllowed + record + 4 * kCameraChase, 0);
        return guest::direct<0x4bdbd0, tvNextCamera>(app, cpu, player);
    }
    }
    const reg32 camera = guest.dword(kTvCameras + 4 * guest.dword(kDirectorNext + record));
    if (guest.dword(kDirectorAllowed + record + 4 * camera) != 0 && over == 0)
        return camera;
    return guest::direct<0x4bdbd0, tvNextCamera>(app, cpu, player);
}

x86::reg32 simTvDirect(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bdd70", 0, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 player = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::tvDirect(app, cpu, player); });
}

/* ---------------------------------------------------------------------------
 * Small helpers
 * ------------------------------------------------------------------------- */

/* sub_4be000: al 1 for a digit or a sign, 0 for anything else; the rest of eax
 * as it was. */
x86::reg32 simc::isNumberCharacter(App*, CPU&, reg32 eax)
{
    const reg8 character = reg8(eax);
    const bool number = (character >= '0' && character <= '9') || character == '-' || character == '+';
    return reg32((eax & 0xffffff00u) | (number ? 1u : 0u));
}

x86::reg32 simIsNumberCharacter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4be000", 0, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 eax = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::isNumberCharacter(app, cpu, eax); });
}

/* sub_4be1a0: 0 on one half-tick of every 32 / eax (the clock's, edx further on),
 * 1 on the others. */
x86::reg32 simc::offBeat(App* app, CPU& cpu, reg32 rate, reg32 offset)
{
    Guest guest(app, cpu);
    const sreg32 clock = sreg32(guest.dword(kRaceClock));
    const reg32 tick = reg32((clock - (clock >> 31)) >> 1) + offset;
    const reg32 period = divide(0x20, rate).quotient;
    const reg32 remainder = divide(tick, period).remainder;
    return reg32(remainder != 0 ? 1 : 0);
}

x86::reg32 simOffBeat(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4be1a0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 rate = args.eax, offset = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::offBeat(app, cpu, rate, offset); });
}

/* The sirens: a record of 80 bytes per car that has one on. */
constexpr reg32 kSirens = 0x7dd850;
constexpr reg32 kSirenBytes = 80;

/* sub_4c9dd0: siren eax on, at track node edx, on side ebx (past the sixth the far
 * row of the node's heights), from clock ecx: its two ends and its normal. */
x86::reg32 simc::sirenOn(App* app, CPU& cpu, reg32 index, reg32 node, reg32 side, reg32 clock)
{
    constexpr reg32 kUp = 0x4c9a70;       // 0, 1, 0
    constexpr reg32 kNodes = 0x5dd948;
    constexpr reg32 kNodeBytes = 36;
    constexpr reg32 kHeightTimes = 0x540368, kHeightUnit = 0x540370, kHalf = 0x540360;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x5c);
    const reg32 up = guest.frame(), line = guest.frame() + 0xc;
    const reg32 farEnd = guest.frame() + 0x18, nearEnd = guest.frame() + 0x24;
    copyWords(guest, up, kUp, 3);
    const reg32 nodeAt = guest.dword(kNodes) + node * kNodeBytes;
    const reg32 raised = reg32(guest.byte(nodeAt + (sreg32(side) < 7 ? 0x17 : 0x1b))) << 15;

    // One end of the node's side, into esi's floats.
    guest::direct<0x4c9bc0, simd::placeInLane>(app, cpu, node, side, 0, nearEnd);
    const reg32 siren = kSirens + index * kSirenBytes;
    copyWords(guest, siren + 0x28, nearEnd, 3);
    guest::direct<0x4c9bc0, simd::placeInLane>(app, cpu, node, side, 1, farEnd);
    copyWords(guest, siren + 0x1c, farEnd, 3);

    const Float height = fp::mul(fp::mul(Float(sreg32(raised)), loadDouble(guest, kHeightTimes)), loadDouble(guest, kHeightUnit));
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
    fe::direct<0x4972f0, vertices::normaliseVector>(app, cpu, line);
    // Its three registers stay as they are handed, as the code leaves them.
    return guest::direct<0x4e0160, simd::cross>(app, cpu, line, up, siren + 0x3c);
}

x86::reg32 simSirenOn(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c9dd0", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 index = args.eax, node = args.edx, side = args.ebx, clock = args.ecx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::sirenOn(app, cpu, index, node, side, clock); });
}

/* sub_4c9f50: siren eax off. */
x86::reg32 simc::sirenOff(App* app, CPU& cpu, reg32 index)
{
    Guest guest(app, cpu);
    const reg32 offset = index * kSirenBytes;
    guest.setDword(kSirens + offset, 0);
    return offset;
}

x86::reg32 simSirenOff(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c9f50", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 index = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::sirenOff(app, cpu, index); });
}

/* sub_4ea570: the angle of (edx, eax) in 16-bit fixed point, 0x10000 to the turn:
 * an octant's from the table at 0x5679bc, by the 32-bit ratio of the smaller to the
 * larger, interpolated. */
x86::reg32 simc::angle(App* app, CPU& cpu, reg32 y, reg32 x)
{
    constexpr reg32 kDivide64 = 0x503760;
    constexpr reg32 kArcTangents = 0x5679bc;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 16);
    reg32 octant = 0;
    if (sreg32(x) < 0)
    {
        octant = 2;
        x = 0 - x;
    }
    if (sreg32(y) < 0)
    {
        y = 0 - y;
        octant |= 4;
    }
    reg32 angle;
    if (y == x)
    {
        angle = 0x2000;
    }
    else
    {
        if (sreg32(x) < sreg32(y))
        {
            const reg32 larger = y;
            octant |= 1;
            y = x;
            x = larger;
        }
        const reg32 wide = guest.frame();
        // Where the game's two pushes before these calls have the stack.
        cpu.esp = wide - 8;
        // The game's code hands it the 64 bits' address in esi: its `out`.
        fe::direct<0x503b00, wina::shiftTo64>(app, cpu, y, 0x20, wide);
        cpu.esp = wide - 4;
        const reg32 ratio = fx::callAt(guest, kDivide64, args({}, { guest.dword(wide), guest.dword(wide + 4), x }));
        const reg32 entry = (ratio >> 24) * 4;
        const reg32 fraction = (ratio >> 8) & 0xffff;
        const reg32 from = guest.dword(kArcTangents + entry);
        const reg32 step = ((guest.dword(kArcTangents + entry + 4) - from) * fraction) >> 16;
        angle = from + step;
    }
    switch (octant)
    {
    case 1:
        return reg32(0x4000 - angle);
    case 2:
        return reg32(0x8000 - angle);
    case 3:
        return reg32(angle + 0x4000);
    case 4:
        return reg32(0 - angle);
    case 5:
        return reg32(angle - 0x4000);
    case 6:
        return reg32(angle - 0x8000);
    case 7:
        return reg32(0 - angle - 0x4000);
    default:
        return angle;
    }
}

x86::reg32 simAngle(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4ea570", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 y = args.eax, x = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::angle(app, cpu, y, x); });
}

/* sub_4ea654: how many entries the pack at eax has. */
x86::reg32 simc::packCount(App* app, CPU& cpu, reg32 pack)
{
    Guest guest(app, cpu);
    return guest.dword(pack + 8);
}

x86::reg32 simPackCount(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4ea654", 0, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 pack = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::packCount(app, cpu, pack); });
}

/* sub_4ea658: entry edx of the pack at eax, or 0 past its end. */
x86::reg32 simc::packEntry(App* app, CPU& cpu, reg32 pack, reg32 entry)
{
    Guest guest(app, cpu);
    if (entry >= guest.dword(pack + 8))
        return reg32(0);
    return reg32(pack + guest.dword(pack + entry * 8 + 0x14));
}

x86::reg32 simPackEntry(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4ea658", 0, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 pack = args.eax, entry = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::packEntry(app, cpu, pack, entry); });
}

/* ---------------------------------------------------------------------------
 * The view widget
 * ------------------------------------------------------------------------- */

/* sub_43bcc0: where on the car's interior picture its map starts, by car eax: the
 * car's record says, 0x80 without one. */
x86::reg32 simc::interiorMapX(App* app, CPU& cpu, reg32 car)
{
    constexpr reg32 kNoCar = 0x80;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    const reg32 record = fe::direct<0x439a50, load::recordAt>(app, cpu, car);
    return record == 0 ? kNoCar : reg32(guest.word(record + 0x38c));
}

x86::reg32 viewInteriorMapX(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_43bcc0", 0, 0, nullptr, 0, kRestoreNone };
    const reg32 car = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::interiorMapX(app, cpu, car); });
}

/* sub_465320: the widget's table's +0x10: the item at eax, shown now, placed: its
 * view of 320 by 192 from where the item stands. */
x86::reg32 simc::viewLaidOut(App* app, CPU& cpu, reg32 item)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 12);
    // The game's code hands it 0x21 in edx too, which it does not read: there as the code has it.
    if (fe::direct<0x44a660, loop::shownNow>(app, cpu, item) != 0)
    {
        guest.setWord(item + 0x1e, 0x140);
        guest.setWord(item + 0x20, 0xc0);
        guest.setWord(item + 0x1c, guest.word(item + 6));
        guest.setWord(item + 0x1a, guest.word(item + 8));
    }
    return reg32(0);
}

x86::reg32 viewLaidOut(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_465320", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 item = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::viewLaidOut(app, cpu, item); });
}

/* sub_465360: the widget's table's +4: nothing. */
x86::reg32 simc::viewNoEntry(App*, CPU&, reg32 eax)
{
    return eax;
}

x86::reg32 viewNoEntry(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_465360", 0, 0, nullptr, 0, fe::kRestoreAll };
    const reg32 eax = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::viewNoEntry(app, cpu, eax); });
}

/* sub_465370: the widget's table's +0: the showcase's car's interior picture loaded
 * and the view set back; on the credits' screen the picture by its number. */
x86::reg32 simc::viewLoadCar(App* app, CPU& cpu)
{
    constexpr reg32 kCar = 0x664798, kCarShown = 0x678940;
    constexpr reg32 kSprintf = 0x4df690, kStrcmp = 0x4ee310;
    constexpr reg32 kFileFormat = 0x53ab3c, kCredits = 0x53ab48, kNumberedFormat = 0x53ab54, kFolderFormat = 0x53ab60;
    constexpr reg32 kTraceFile = 0x53ab78, kTraceFormat = 0x53ab9c;
    constexpr reg32 kInteriorRoot = 0x7a3234, kCarsRoot = 0x7a28b4;
    constexpr reg32 kPanBase = 0x55a438, kMapOffsetX = 0x55a43c, kMapOffsetY = 0x55a440;
    constexpr reg32 kPanX = 0x678954, kPanY = 0x678958;
    constexpr float kFullScale = 1.0f;
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 0x270);
    const reg32 file = guest.frame(), folder = guest.frame() + 0x12c;
    const reg32 carFolder = *fe::direct<0x4392b0, load::folderOf>(app, cpu, guest.dword(kCar));
    guest.setDword(kCarShown, guest.dword(kCar));
    const reg32 stem = fe::direct<0x439290, showcase::carFileStem>(app, cpu, guest.dword(kCar));
    fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), file, kFileFormat, guest::Words{stem});
    const reg32 menu = fe::direct<0x44ae00, loop::currentMenuName>(app, cpu);
    if (fe::direct<kStrcmp, watcom::clib::strcmp>(guest.app(), guest.cpu(), menu, kCredits) == 0)
    {
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), file, kNumberedFormat, guest::Words{guest.dword(kCar)});
        guest.copyString(folder, kInteriorRoot);
    }
    if (*fe::direct<0x4395c0, load::flag4Of>(guest.app(), guest.cpu(), guest.dword(kCar)) != 0)
        fe::direct<kSprintf, watcom::clib::sprintf>(guest.app(), guest.cpu(), folder, kFolderFormat, guest::Words{kCarsRoot, carFolder});
    else
        guest.copyString(folder, kInteriorRoot);
    // The game's code hands it the ebx it has and 1.0 in ecx too, which it does not read: there as
    // the code has them.
    fe::direct<0x464700, view::makeTiles>(app, cpu, file, folder);
    guest.setDword(kPanBase, bitsOf(kFullScale));
    guest.setDword(kMapOffsetY, 0);
    const reg32 mapX = guest::direct<0x43bcc0, interiorMapX>(app, cpu, guest.dword(kCar));
    guest.setDword(kMapOffsetX, mapX);
    // The trace (compiled out) with its words pushed as the code pushes them, though it reads
    // none; it leaves eax as it came, the map's x, which is this function's.
    const reg32 esp = cpu.esp;
    const reg32 words[] = { 0x5a, 2, kTraceFormat, kTraceFile, 0x214, mapX };
    for (reg32 i = 0; i < 6; ++i)
        guest.setDword(esp - 24 + 4 * i, words[i]);
    cpu.esp = esp - 24;
    fe::direct<0x42d510, ea::traceOff>(app, cpu);
    cpu.esp = esp;
    guest.setDword(kPanY, 0);
    guest.setDword(kPanX, bitsOf(kFullScale));
    return mapX;
}

x86::reg32 viewLoadCar(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_465370", 0, 0, nullptr, 0, fe::kRestoreAll };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return simc::viewLoadCar(app, cpu); });
}

}

}
