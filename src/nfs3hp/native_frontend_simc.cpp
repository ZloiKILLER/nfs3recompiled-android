#include "native_frontend.h"
#include <nfs3hp.h>
#include <fpu.h>
#include <cstring>
#include <initializer_list>

/* What was left generated of the simulation above 0x4b7000, decompiled, and the
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
 * Watcom's callers may go on with them. */
namespace nfs3hp
{

float zoomHalfWidth(win32::WinApplication* app);  // tools/apply_widescreen.py, nfs3hp_main.cpp

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

/* idiv of a sign-extended dividend: both results, as the generated code has them. */
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
constexpr reg32 kSquareRoot = 0x4ea920;         // a float on the stack, its root onto the x87's
constexpr reg32 kSlerp = 0x4b71e0, kQuatFromMatrix = 0x4b73c0, kMatrixFromQuat = 0x4b7590;
constexpr reg32 kTranspose = 0x4e0110, kMultiply = 0x4e0280;
constexpr reg32 kMatrixSubtract = 0x4b7720, kMatrixAdd = 0x4b7780, kMatrixScale = 0x4b77e0;
constexpr reg32 kSineCosine = 0x4ea970;

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
constexpr reg32 kPlayers = 0x6fd3b0;
constexpr reg32 kRaceClock = 0x7d3684;
constexpr reg32 kClockStart = 0x200;            // the clock of a replay's first frame

constexpr reg32 kSwitchCamera = 0x422610, kPickChaseNode = 0x421490;
constexpr reg32 kKeyPressed = 0x48f8d0;
constexpr reg32 kPanelHighlight = 0x4bd3f0;     // the highlight on the button in eax
constexpr reg32 kPanelPointer = 0x4bd4d0;
constexpr reg32 kMenuKey = 0x451960, kLeaveReplay = 0x4c4d80, kDirectorAfterPanel = 0x426680;
constexpr reg32 kSetCamera = 0x4bdb60, kNextCamera = 0x4bdbd0, kShotOver = 0x4bdc70, kStartShot = 0x4bdcc0;
constexpr reg32 kBusiest = 0x4bdd10, kNearestSpot = 0x423d90;

/* The panel's scan codes. */
constexpr reg32 kKeyLeft = 0x4b, kKeyRight = 0x4d, kKeyDown = 0x50, kKeyEnter = 0x1c, kKeyUp = 0x48;

/* The director takes the camera over (the panel's camera kCameraTelevision), for
 * each player: every camera allowed, a shot of the first of the list.  What the last
 * switch left in eax. */
reg32 startTelevision(Guest& guest, reg32 players)
{
    reg32 left = 0;
    for (reg32 player = 0; player < players; ++player)
    {
        const reg32 record = player * kDirectorBytes;
        guest.setDword(kDirectorOn + record, 1);
        guest.setDword(kDirectorNext + record, 0);
        guest.call(kStartShot, args({ player, guest.dword(kTvCameras) }));
        for (reg32 kind = 0; kind < kTvCameraKinds; ++kind)
            guest.setDword(kDirectorAllowed + record + 4 * kind, 1);
        const reg32 camera = guest.dword(kTvCameras + 4 * guest.dword(kDirectorNext + record));
        left = guest.call(kSwitchCamera, args({ camera, player }));
    }
    return left;
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
void simPoseBlend(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 16;
    static fe::Site site{ "sub_4b70f0", 12, kFrame, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 quatA = cpu.eax, quatB = cpu.edx, placeA = cpu.ebx, placeB = cpu.ecx;
        const reg32 entry = guest.frame() + kFrame;
        const Float t = fldAt(guest, entry + 4);
        const reg32 quatOut = guest.dword(entry + 8), placeOut = guest.dword(entry + 12);
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
        return guest.eax();
    });
}

/* sub_4b71e0: quaternion slerp: eax and edx mixed by the float on the stack, into
 * the pointer after it.  The second is negated first when they are more than a
 * quarter turn apart, and two closer than kNear in cosine are mixed linearly. */
void simQuatSlerp(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 0x50;
    constexpr reg32 kNear = 0x53ff60;
    static fe::Site site{ "sub_4b71e0", 8, kFrame, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 from = cpu.eax, to = cpu.edx;
        const reg32 entry = guest.frame() + kFrame;
        const reg32 out = guest.dword(entry + 8);
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
        reg32 left = withStatus(cpu.eax, status);
        double fromWeight, toWeight;
        if (above(status))
        {
            cpu.eax = left;
            guest.fpu().push(cosine);
            left = guest.call(kArcCosine);
            const Float angle = guest.fpu().take();
            const Float sine = fp::sin(angle);
            const Float t = fldAt(guest, entry + 4);
            const Float sineFrom = fp::sin(fp::mul(fp::sub(1.0, t), angle));
            const Float sineTo = fp::sin(fp::mul(angle, t));
            const Float reciprocal = fp::div(1.0, sine);
            fromWeight = fp::mul(sineFrom, reciprocal);
            toWeight = fp::mul(reciprocal, sineTo);
        }
        else
        {
            const double t = fldAt(guest, entry + 4);
            fromWeight = fp::sub(1.0, t);
            toWeight = t;
            guest.leaveEdx(highWordOf(t));
        }
        for (reg32 i = 0; i < 3; ++i)
        {
            const Float a = fp::mul(fldAt(guest, from + 4 * i), fromWeight);
            fstpAt(guest, out + 4 * i, fp::add(a, fp::mul(target(i), toWeight)));
        }
        const Float a = fp::mul(fromWeight, fldAt(guest, from + 12));
        fstpAt(guest, out + 12, fp::add(a, fp::mul(toWeight, target(3))));
        return left;
    });
}

/* sub_4b7300: the quaternion at edx negated if its negative is nearer the one at eax:
 * the squared length of the sum against that of the difference.  The signs go byte
 * by byte, not through the x87. */
void simQuatNearer(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 0x30;
    static fe::Site site{ "sub_4b7300", 0, kFrame, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 a = cpu.eax, b = cpu.edx;
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
        reg32 left = withStatus(cpu.eax, status);
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
    });
}

/* sub_4b73c0: the 3x3 matrix at eax made the quaternion at edx: by its trace when
 * that is above zero, otherwise from its largest diagonal element. */
void simQuatFromMatrix(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 0x58;
    constexpr reg32 kHalf = 0x53ff68;
    static const reg32 kNextAxis[3] = { 1, 2, 0 };  // the table at 0x4b6a1c
    static fe::Site site{ "sub_4b73c0", 0, kFrame, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 matrix = cpu.eax, out = cpu.edx;
        const auto element = [&](reg32 row, reg32 column) { return matrix + 12 * row + 4 * column; };
        const auto rootOf = [&](Float value) {
            guest.call(kSquareRoot, args({}, { bitsOf(float(value)) }));
            return guest.fpu().take();
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
    });
}

/* sub_4b7590: the quaternion at eax made the 3x3 matrix at edx. */
void simMatrixFromQuat(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 0x38;
    constexpr reg32 kTwo = 0x53ff6c;
    static fe::Site site{ "sub_4b7590", 0, kFrame, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 quat = cpu.eax, out = cpu.edx;
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
        return guest.eax();
    });
}

/* sub_4b7680: two 3x3 matrices, eax and edx, slerped through quaternions by the
 * float on the stack, into the pointer after it. */
void simMatrixSlerp(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 0x3c;
    static fe::Site site{ "sub_4b7680", 8, kFrame, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 from = cpu.eax, to = cpu.edx;
        const reg32 entry = guest.frame() + kFrame;
        const reg32 mixed = guest.frame(), toQuat = guest.frame() + 0x10, fromQuat = guest.frame() + 0x20;
        const reg32 out = guest.dword(entry + 8);
        guest.call(kQuatFromMatrix, args({ from, fromQuat }));
        guest.call(kQuatFromMatrix, args({ to, toQuat }));
        guest.call(kSlerp, args({ fromQuat, toQuat }, { guest.dword(entry + 4), mixed }));
        return guest.call(kMatrixFromQuat, args({ mixed, out }));
    });
}

/* sub_4b7720: the 3x3 matrix at ebx is eax minus edx. */
void simMatrixSubtract(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b7720", 0, 4, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        for (reg32 at = 0; at < 36; at += 4)
            fstpAt(guest, cpu.ebx + at, fp::sub(fldAt(guest, cpu.eax + at), fldAt(guest, cpu.edx + at)));
        return guest.eax();
    });
}

/* sub_4b7780: the 3x3 matrix at ebx is eax plus edx. */
void simMatrixAdd(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b7780", 0, 4, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        for (reg32 at = 0; at < 36; at += 4)
            fstpAt(guest, cpu.ebx + at, fp::add(fldAt(guest, cpu.eax + at), fldAt(guest, cpu.edx + at)));
        return guest.eax();
    });
}

/* sub_4b77e0: the 3x3 matrix at eax times the float on the stack, into the pointer
 * after it. */
void simMatrixScale(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 8;
    static fe::Site site{ "sub_4b77e0", 8, kFrame, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 entry = guest.frame() + kFrame;
        const reg32 out = guest.dword(entry + 8);
        const Float factor = fldAt(guest, entry + 4);
        for (reg32 at = 0; at < 32; at += 4)
            fstpAt(guest, out + at, fp::mul(fldAt(guest, cpu.eax + at), factor));
        fstpAt(guest, out + 32, fp::mul(factor, fldAt(guest, cpu.eax + 32)));
        return guest.eax();
    });
}

/* sub_4b7840: the 3x3 matrix at eax made orthonormal again: M = M (M'M)^-1/2, the
 * root by edx terms -- 10 at the least -- of its binomial series, with coefficients
 * at 0x55fdcc. */
void simMatrixOrthonormalise(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 0xd4;
    constexpr reg32 kIdentity = 0x564498;
    constexpr reg32 kCoefficients = 0x55fdcc;
    constexpr reg32 kWords = 9;
    static fe::Site site{ "sub_4b7840", 0, kFrame, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 matrix = cpu.eax;
        const reg32 terms = sreg32(cpu.edx) > 10 ? cpu.edx : 10;
        const reg32 identity = guest.frame(), term = guest.frame() + 0x24, excess = guest.frame() + 0x48;
        const reg32 power = guest.frame() + 0x6c, sum = guest.frame() + 0x90;
        const float limit = float(Float(sreg32(terms)));

        guest.call(kTranspose, args({ matrix, term, term }));
        guest.call(kMultiply, args({ term, matrix, term }));
        copyWords(guest, identity, kIdentity, kWords);
        guest.call(kMatrixSubtract, args({ term, identity, excess }));
        copyWords(guest, power, kIdentity, kWords);
        copyWords(guest, sum, power, kWords);
        for (reg32 i = 1; below(fp::compare(Float(sreg32(i)), Float(limit))); ++i)
        {
            guest.call(kMultiply, args({ power, excess, power }));
            guest.call(kMatrixScale, args({ power, term, sum }, { guest.dword(kCoefficients + 4 * i), term }));
            guest.call(kMatrixAdd, args({ sum, term, sum }));
        }
        return guest.call(kMultiply, args({ matrix, sum, matrix }));
    });
}

/* sub_4b79c0: the 3x3 matrix at eax turned a quarter: the second row is the third,
 * the third the second's negative. */
void simMatrixQuarterTurn(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4b79c0", 0, 12, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 matrix = cpu.eax;
        for (reg32 at = 0; at < 12; at += 4)
        {
            const Float second = fldAt(guest, matrix + 12 + at);
            const Float third = fldAt(guest, matrix + 24 + at);
            fstpAt(guest, matrix + 12 + at, third);
            fstpAt(guest, matrix + 24 + at, -second);
        }
        return guest.eax();
    });
}

/* sub_4ea680: the 3x3 matrix at eax turned about its first row, by the angle on the
 * stack, into the pointer after it, which may be the matrix itself. */
void simTurnAboutFirstRow(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 0x44;
    static fe::Site site{ "sub_4ea680", 8, kFrame, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 entry = guest.frame() + kFrame;
        const reg32 out = guest.dword(entry + 8);
        reg32 source = cpu.eax;
        if (source == out)
        {
            copyWords(guest, guest.frame(), out, 9);
            source = guest.frame();
        }
        guest.call(kSineCosine, args({ entry + 4, guest.frame() + 0x24, guest.frame() + 0x28 }));
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
    });
}

/* sub_4ea820: the 3x3 matrix at eax turned about its last row, as sub_4ea680. */
void simTurnAboutLastRow(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 0x44;
    static fe::Site site{ "sub_4ea820", 8, kFrame, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 entry = guest.frame() + kFrame;
        const reg32 out = guest.dword(entry + 8);
        reg32 source = cpu.eax;
        if (source == out)
        {
            copyWords(guest, guest.frame(), out, 9);
            source = guest.frame();
        }
        guest.call(kSineCosine, args({ entry + 4, guest.frame() + 0x24, guest.frame() + 0x28 }));
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
    });
}

/* sub_4ea920: the square root of the float on the stack, onto the x87's. */
void simSquareRoot(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 4;
    static fe::Site site{ "sub_4ea920", 4, kFrame, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const Float root = fp::sqrt(fldAt(guest, guest.frame() + kFrame + 4));
        cpu.fpu.count += 1;
        cpu.fpu.st(0) = root;
        return guest.eax();
    });
}

/* ---------------------------------------------------------------------------
 * A view's pass
 * ------------------------------------------------------------------------- */

/* sub_4bc470: a view's pass: eax the pass (0 or 1 draw), edx, ebx, ecx and the word
 * on the stack what the view's record keeps, the frame drawn through it. */
void simViewPass(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kView = 0x7a3d14;
    constexpr reg32 kChooseView = 0x421450, kSetMirrored = 0x4349a0, kAmbientSounds = 0x41bb50;
    constexpr reg32 kRaceFrame = 0x4bc160, kDrawMirrored = 0x4349d0, kPassStart = 0x4bbd70;
    constexpr reg32 kFirstShare = 0x560094, kSecondShare = 0x560098;
    constexpr float kFourFifths = 0.8f, kOneFifth = 0.2f;
    static fe::Site site{ "sub_4bc470", 4, 12, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 pass = cpu.eax;
        const reg32 entry = guest.frame() + 12;
        guest.call(kPassStart);
        guest.setDword(kView, pass);
        guest.setDword(kView + 4, cpu.edx);
        const reg32 kept = guest.dword(entry + 4);
        guest.setDword(kView + 0x68, cpu.ebx);
        guest.setDword(kView + 0x70, kept);
        guest.setDword(kView + 0x6c, cpu.ecx);
        guest.call(kChooseView, args({ kView, 0 }));
        guest.call(kSetMirrored, args({ kView }));
        if (pass == 0)
        {
            guest.leaveEcx(bitsOf(kOneFifth));
            guest.setDword(kFirstShare, bitsOf(kFourFifths));
            guest.setDword(kSecondShare, bitsOf(kOneFifth));
        }
        else if (pass == 1)
        {
            guest.leaveEbx(bitsOf(kOneFifth));
            guest.setDword(kFirstShare, bitsOf(kOneFifth));
            guest.setDword(kSecondShare, 0);
        }
        else
        {
            return guest.eax();
        }
        guest.call(kAmbientSounds, args({ kView }));
        guest.call(kRaceFrame, args({ pass }));
        guest.call(kChooseView, args({ kView, 2 }));
        const reg32 first = guest.dword(0x7a3cf0), second = guest.dword(0x7a3cf4);
        return guest.call(kDrawMirrored, args({ kView, first }, { first, second }));
    });
}

/* sub_4dbde0: the projection of the view at eax: the first's own, or each player's,
 * and a racing zoom camera's scales (the port's zoomHalfWidth keeps pixels square on
 * a wide view). */
void simSetView(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFov = 0x4dbcc0, kSetProjection = 0x4bed20;
    constexpr reg32 kMirrored = 0x6fd4c4;
    constexpr reg32 kCarZoom = 0x54;
    constexpr reg32 kScaleX = 0x56009c, kScaleY = 0x5600a0, kHalfHeight = 0x5600b0, kAspect = 0x54972c;
    static fe::Site site{ "sub_4dbde0", 0, 20, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 view = cpu.eax;
        if (guest.dword(view) == 1)
        {
            const reg32 notMirrored = guest.dword(kMirrored) == 0 ? 1 : 0;
            const reg32 fov = guest.call(kFov);
            return guest.call(kSetProjection, args({ 0x7cdb48, fov, notMirrored }));
        }
        const bool split = guest.dword(kPlayers) == 1;
        const reg32 mirrored = guest.dword(kMirrored);
        const reg32 fov = guest.call(kFov);
        const reg32 viewport = split ? 0x7cda80 + (guest.dword(view + 4) << 5) : 0x7cdb88;
        guest.call(kSetProjection, args({ viewport, fov, mirrored }));

        reg32 car = carOf(guest, guest.dword(view + 4));
        if ((guest.dword(car + kCarZoom) & 0x7fffffffu) == 0)
            return car;
        car = carOf(guest, guest.dword(view + 4));
        const Float scaleX = fp::mul(Float(zoomHalfWidth(app)), fldAt(guest, car + kCarZoom));
        car = carOf(guest, guest.dword(view + 4));
        fstpAt(guest, kScaleX, scaleX);
        const Float scaleY = fp::mul(fp::mul(-fldAt(guest, kHalfHeight), fldAt(guest, car + kCarZoom)),
                                     loadDouble(guest, kAspect));
        const reg32 flip = guest.dword(kMirrored);
        fstpAt(guest, kScaleY, scaleY);
        if (flip != 0)
            guest.setByte(kScaleX + 3, reg8(guest.byte(kScaleX + 3) ^ 0x80));
        return car;
    });
}

/* sub_4d5720: nothing of its own: the race frame's end, sub_434a90. */
void simRaceFrameEnd(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d5720", 0, 4, nullptr, 0, kRestoreNone };
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return guest.call(0x434a90); });
}

/* ---------------------------------------------------------------------------
 * The replay's panel
 * ------------------------------------------------------------------------- */

/* sub_4bd4d0: the pointer on the replay's panel: inside it the button under it is
 * highlighted; a press inside, no sooner than 0x20 ticks after the last, is the
 * buttons' word; anything else 0. */
void simPanelPointer(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 0x54;
    constexpr reg32 kPanelLeft = 0x7497d8, kPanelTop = 0x7497dc, kPanelRight = 0x7497e0, kPanelBottom = 0x7497e4;
    constexpr reg32 kFirstEnd = 0x540198, kSecondEnd = 0x5401a0, kThirdEnd = 0x5401a8;
    constexpr reg32 kReadMouse = 0x435310, kHotSpot = 0x4dab10;
    constexpr reg32 kTicks = 0x79c1a4, kNextPress = 0x55febc;
    constexpr reg32 kPressGap = 0x20;
    static fe::Site site{ "sub_4bd4d0", 0, kFrame, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const auto floatOfInteger = [](reg32 value) { return float(Float(sreg32(value))); };
        const float left = floatOfInteger(guest.dword(kPanelLeft));
        const float right = floatOfInteger(guest.dword(kPanelRight));
        const float top = floatOfInteger(guest.dword(kPanelTop));
        const float bottom = floatOfInteger(guest.dword(kPanelBottom));
        const reg32 buttons = guest.frame() + 0xc, mouseX = guest.frame() + 0x10, mouseY = guest.frame() + 0x14;
        const reg32 hotX = guest.frame() + 0x18, hotY = guest.frame() + 0x1c;
        guest.call(kReadMouse, args({ buttons, mouseX, mouseY, 0 }));
        guest.call(kHotSpot, args({ hotX, hotY }));
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
    });
}

/* sub_4bd640: the replay's panel each frame: a camera asked for last frame made
 * current for each player, then one key: left and right move the highlight, down
 * takes the button a step back -- the camera before, the speed below -- and Enter,
 * up or a click a step on, or leaves the replay; both toggle the pause. */
void simPanelKeys(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kDirectorWanted = 0x7d36a8;
    static fe::Site site{ "sub_4bd640", 0, 24, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const auto pressed = [&](reg32 key) { return guest.call(kKeyPressed, args({ key, 1 })); };
        const reg32 players = guest.dword(kPlayers) == 1 ? 2 : 1;
        reg32 left = guest.eax();

        const reg32 wanted = guest.dword(kPanelCameraWanted);
        if (sreg32(wanted) > 0 && guest.dword(kPanelCameraChanged) != 0)
        {
            guest.setDword(kPanelCamera, wanted);
            for (reg32 player = 0; player < players; ++player)
            {
                guest.setDword(kDirectorOn + player * kDirectorBytes, 0);
                guest.call(kSwitchCamera, args({ guest.dword(kPanelCameras + 4 * guest.dword(kPanelCamera)), player }));
                left = guest.dword(kPanelCamera);
                if (guest.dword(kPanelCameras + 4 * left) == kCameraChase)
                    left = guest.call(kPickChaseNode, args({ player }));
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
            left = guest.call(kPanelHighlight, args({ guest.dword(kPanelButton) }));
        }
        else if (pressed(kKeyRight) != 0)
        {
            const reg32 button = guest.dword(kPanelButton);
            if (sreg32(button) < 3)
                guest.setDword(kPanelButton, button + 1);
            left = guest.call(kPanelHighlight, args({ guest.dword(kPanelButton) }));
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
                    left = startTelevision(guest, players);
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
        else if (pressed(kKeyEnter) != 0 || pressed(kKeyUp) != 0 || guest.call(kPanelPointer) != 0)
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
                    left = startTelevision(guest, players);
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
                guest.call(kMenuKey, args({ 1 }));
                left = guest.call(kLeaveReplay);
            }
        }
        else
        {
            left = 0;
        }

        if (guest.dword(kDirectorWanted) != 0)
            left = guest.call(kDirectorAfterPanel, args({ 0 }));
        return left;
    });
}

/* ---------------------------------------------------------------------------
 * The television director
 * ------------------------------------------------------------------------- */

/* sub_4bdb60: player eax's camera made edx, and a new shot begun whatever it was. */
void simTvSetCamera(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bdb60", 0, 16, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 player = cpu.eax, camera = cpu.edx;
        guest.call(kStartShot, args({ player, camera }));
        guest.leaveEdx(guest.dword(kReplayCars + 4 * player));
        const reg32 current = cameraOfCar(guest, player);
        if (current == camera)
            return current;
        reg32 left = guest.call(kSwitchCamera, args({ camera, player }));
        if (camera == kCameraChase)
            left = guest.call(kPickChaseNode, args({ player }));
        return left;
    });
}

/* sub_4bdbd0: player eax's next shot: with the director on, the next camera of the
 * list that is allowed; otherwise a new shot of the camera the player's car has. */
void simTvNextCamera(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bdbd0", 0, 24, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 player = cpu.eax;
        const reg32 record = player * kDirectorBytes;
        if (guest.dword(kDirectorOn + record) == 0)
            return guest.call(kStartShot, args({ player, cameraOfCar(guest, player) }));
        reg32 camera;
        do
        {
            if (app->contextWanted())
                app->yieldContext(cpu);
            const reg32 next = divide(guest.dword(kDirectorNext + record) + 1, kTvCameraCount).remainder;
            guest.setDword(kDirectorNext + record, next);
            camera = guest.dword(kTvCameras + 4 * next);
        } while (guest.dword(kDirectorAllowed + record + 4 * camera) == 0);
        return guest.call(kSetCamera, args({ player, camera }));
    });
}

/* sub_4bdc70: 1 when player eax's shot has run its length. */
void simTvShotOver(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bdc70", 0, 16, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 record = cpu.eax * kDirectorBytes;
        const reg32 elapsed = guest.dword(kRaceClock) - kClockStart - guest.dword(kDirectorShotStart + record);
        return reg32(sreg32(elapsed) > sreg32(guest.dword(kDirectorShotLength + record)) ? 1 : 0);
    });
}

/* sub_4bdcc0: player eax's shot begins now, as long as camera edx's are. */
void simTvStartShot(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4bdcc0", 0, 12, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 player = cpu.eax, camera = cpu.edx;
        const reg32 record = player * kDirectorBytes;
        guest.setDword(kDirectorShotStart + record, guest.dword(kRaceClock) - kClockStart);
        guest.setDword(kDirectorShotLength + record, camera == kCameraShortShot ? 0xa0 : 0x140);
        guest.leaveEdx(player);
        return record;
    });
}

/* sub_4bdd10: the largest of ebx rows of column eax of the replay's table of what
 * happens when, from row edx on: the two counters left where they stop. */
void simTvBusiest(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kEvents = 0x7c3f48;
    constexpr reg32 kRows = 600;
    static fe::Site site{ "sub_4bdd10", 0, 16, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 column = cpu.eax;
        reg32 row = cpu.edx, count = cpu.ebx;
        reg32 largest = 0;
        for (; sreg32(row) < sreg32(kRows) && sreg32(count) > 0; --count, ++row)
        {
            const reg32 value = guest.dword(kEvents + (row << 6) + column * 4);
            if (sreg32(value) > sreg32(largest))
                largest = value;
        }
        guest.leaveEdx(row);
        guest.leaveEbx(count);
        return largest;
    });
}

/* sub_4bdd70: the director's choice for player eax each frame: by the busiest thing
 * coming in the replay's table, some cameras ruled out, a new shot if the one running
 * has run out or its camera may no longer be used. */
void simTvDirect(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kEventColumn = 0x6fd4f8;
    constexpr reg32 kTicksPerRow = 0x78;
    constexpr reg32 kRows = 600;
    constexpr reg32 kNoSpot = 0xffffffff;
    constexpr reg32 kCarRecord = 4, kCarNoChase = 0x998;
    static fe::Site site{ "sub_4bdd70", 0, 32, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 player = cpu.eax;
        const reg32 record = player * kDirectorBytes;
        const reg32 clock = guest.dword(kRaceClock) - kClockStart;
        const reg32 row = divide(sreg32(clock) < 0 ? 0 : clock, kTicksPerRow).quotient;
        const reg32 over = guest.call(kShotOver, args({ player }));
        const reg32 column = guest.dword(kEventColumn);
        if (sreg32(row) >= sreg32(kRows))
            return column;
        for (reg32 kind = 0; kind < kTvCameraKinds; ++kind)
            guest.setDword(kDirectorAllowed + record + 4 * kind, 1);
        const reg32 rows = divide(guest.dword(kDirectorShotLength + record), kTicksPerRow).quotient;
        const reg32 busiest = guest.call(kBusiest, args({ column, row, rows }));

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
            const reg32 spot = guest.call(kNearestSpot, args({ player }));
            if (spot == kNoSpot && over == 0)
                return spot;
            return guest.call(kSetCamera, args({ player, kCameraShortShot }));
        }
        default:
        {
            if (over == 0)
                return event;
            const reg32 car = carOf(guest, player);
            if (guest.dword(guest.dword(car + kCarRecord) + kCarNoChase) != 0)
                guest.setDword(kDirectorAllowed + record + 4 * kCameraChase, 0);
            return guest.call(kNextCamera, args({ player }));
        }
        }
        const reg32 camera = guest.dword(kTvCameras + 4 * guest.dword(kDirectorNext + record));
        if (guest.dword(kDirectorAllowed + record + 4 * camera) != 0 && over == 0)
            return camera;
        return guest.call(kNextCamera, args({ player }));
    });
}

/* ---------------------------------------------------------------------------
 * Small helpers
 * ------------------------------------------------------------------------- */

/* sub_4be000: al 1 for a digit or a sign, 0 for anything else; the rest of eax
 * as it was. */
void simIsNumberCharacter(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4be000", 0, 4, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg8 character = reg8(guest.eax());
        const bool number = (character >= '0' && character <= '9') || character == '-' || character == '+';
        return reg32((guest.eax() & 0xffffff00u) | (number ? 1u : 0u));
    });
}

/* sub_4be1a0: 0 on one half-tick of every 32 / eax (the clock's, edx further on),
 * 1 on the others. */
void simOffBeat(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4be1a0", 0, 16, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 rate = cpu.eax, offset = cpu.edx;
        const sreg32 clock = sreg32(guest.dword(kRaceClock));
        const reg32 tick = reg32((clock - (clock >> 31)) >> 1) + offset;
        const reg32 period = divide(0x20, rate).quotient;
        const reg32 remainder = divide(tick, period).remainder;
        guest.leaveEdx(remainder);
        return reg32(remainder != 0 ? 1 : 0);
    });
}

/* The sirens: a record of 80 bytes per car that has one on. */
constexpr reg32 kSirens = 0x7dd850;
constexpr reg32 kSirenBytes = 80;

/* sub_4c9dd0: siren eax on, at track node edx, on side ebx (past the sixth the far
 * row of the node's heights), from clock ecx: its two ends and its normal. */
void simSirenOn(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 0x5c;
    constexpr reg32 kUp = 0x4c9a70;       // 0, 1, 0
    constexpr reg32 kNodes = 0x5dd948;
    constexpr reg32 kNodeBytes = 36;
    constexpr reg32 kNodeEnd = 0x4c9bc0;  // esi, three floats: one end of the node's side
    constexpr reg32 kNormalise = 0x4972f0, kCross = 0x4e0160;
    constexpr reg32 kHeightTimes = 0x540368, kHeightUnit = 0x540370, kHalf = 0x540360;
    static fe::Site site{ "sub_4c9dd0", 0, kFrame, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 index = cpu.eax, node = cpu.edx, side = cpu.ebx, clock = cpu.ecx;
        const reg32 up = guest.frame(), line = guest.frame() + 0xc;
        const reg32 farEnd = guest.frame() + 0x18, nearEnd = guest.frame() + 0x24;
        copyWords(guest, up, kUp, 3);
        const reg32 nodeAt = guest.dword(kNodes) + node * kNodeBytes;
        const reg32 raised = reg32(guest.byte(nodeAt + (sreg32(side) < 7 ? 0x17 : 0x1b))) << 15;

        guest.setEsi(nearEnd);
        guest.call(kNodeEnd, args({ node, side, 0 }));
        const reg32 siren = kSirens + index * kSirenBytes;
        copyWords(guest, siren + 0x28, nearEnd, 3);
        guest.setEsi(farEnd);
        guest.call(kNodeEnd, args({ node, side, 1 }));
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

        guest.call(kNormalise, args({ line, up, 1 }));
        guest.fpu().take();
        guest.leaveEcx(siren);
        return guest.call(kCross, args({ line, up, siren + 0x3c }));
    });
}

/* sub_4c9f50: siren eax off. */
void simSirenOff(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4c9f50", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 offset = cpu.eax * kSirenBytes;
        guest.setDword(kSirens + offset, 0);
        return offset;
    });
}

/* sub_4ea570: the angle of (edx, eax) in 16-bit fixed point, 0x10000 to the turn:
 * an octant's from the table at 0x5679bc, by the 32-bit ratio of the smaller to the
 * larger, interpolated. */
void simAngle(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 16;
    constexpr reg32 kShiftLeft = 0x503b00, kDivide64 = 0x503760;
    constexpr reg32 kArcTangents = 0x5679bc;
    static fe::Site site{ "sub_4ea570", 0, kFrame, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        reg32 y = cpu.eax, x = cpu.edx;
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
            guest.leaveEdx(x);
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
            guest.setEsi(wide);
            guest.call(kShiftLeft, args({ y, 0x20 }));
            cpu.esp = wide - 4;
            const reg32 ratio = guest.call(kDivide64, args({}, { guest.dword(wide), guest.dword(wide + 4), x }));
            const reg32 entry = (ratio >> 24) * 4;
            const reg32 fraction = (ratio >> 8) & 0xffff;
            const reg32 from = guest.dword(kArcTangents + entry);
            const reg32 step = ((guest.dword(kArcTangents + entry + 4) - from) * fraction) >> 16;
            guest.leaveEdx(step);
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
    });
}

/* sub_4ea654: how many entries the pack at eax has. */
void simPackCount(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ea654", 0, 0, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return guest.dword(cpu.eax + 8); });
}

/* sub_4ea658: entry edx of the pack at eax, or 0 past its end. */
void simPackEntry(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4ea658", 0, 0, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 pack = cpu.eax, entry = cpu.edx;
        if (entry >= guest.dword(pack + 8))
            return reg32(0);
        return reg32(pack + guest.dword(pack + entry * 8 + 0x14));
    });
}

/* ---------------------------------------------------------------------------
 * The view widget
 * ------------------------------------------------------------------------- */

/* sub_43bcc0: where on the car's interior picture its map starts, by car eax: the
 * car's record says, 0x80 without one. */
void viewInteriorMapX(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kCarRecord = 0x439a50;
    constexpr reg32 kNoCar = 0x80;
    static fe::Site site{ "sub_43bcc0", 0, 4, nullptr, 0, kRestoreNone };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 record = guest.call(kCarRecord);
        return record == 0 ? kNoCar : reg32(guest.word(record + 0x38c));
    });
}

/* sub_465320: the widget's table's +0x10: the item at eax, shown now, placed: its
 * view of 320 by 192 from where the item stands. */
void viewLaidOut(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kShownNow = 0x44a660;
    static fe::Site site{ "sub_465320", 0, 12, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 item = cpu.eax;
        if (guest.call(kShownNow, args({ item, 0x21 })) != 0)
        {
            guest.setWord(item + 0x1e, 0x140);
            guest.setWord(item + 0x20, 0xc0);
            guest.setWord(item + 0x1c, guest.word(item + 6));
            guest.setWord(item + 0x1a, guest.word(item + 8));
        }
        return reg32(0);
    });
}

/* sub_465360: the widget's table's +4: nothing. */
void viewNoEntry(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_465360", 0, 4, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return guest.eax(); });
}

/* sub_465370: the widget's table's +0: the showcase's car's interior picture loaded
 * and the view set back; on the credits' screen the picture by its number. */
void viewLoadCar(win32::WinApplication* app, x86::CPU& cpu)
{
    constexpr reg32 kFrame = 0x270;
    constexpr reg32 kCar = 0x664798, kCarShown = 0x678940;
    constexpr reg32 kCarFolder = 0x4392b0, kCarFileStem = 0x439290, kCarFlag = 0x4395c0;
    constexpr reg32 kSprintf = 0x4df690, kMenuName = 0x44ae00, kStrcmp = 0x4ee310;
    constexpr reg32 kMakeTiles = 0x464700, kMapX = 0x43bcc0, kTrace = 0x42d510;
    constexpr reg32 kFileFormat = 0x53ab3c, kCredits = 0x53ab48, kNumberedFormat = 0x53ab54, kFolderFormat = 0x53ab60;
    constexpr reg32 kTraceFile = 0x53ab78, kTraceFormat = 0x53ab9c;
    constexpr reg32 kInteriorRoot = 0x7a3234, kCarsRoot = 0x7a28b4;
    constexpr reg32 kPanBase = 0x55a438, kMapOffsetX = 0x55a43c, kMapOffsetY = 0x55a440;
    constexpr reg32 kPanX = 0x678954, kPanY = 0x678958;
    constexpr float kFullScale = 1.0f;
    static fe::Site site{ "sub_465370", 0, kFrame, nullptr, 0, fe::kRestoreAll };
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 file = guest.frame(), folder = guest.frame() + 0x12c;
        const reg32 carFolder = guest.call(kCarFolder, args({ guest.dword(kCar) }));
        guest.setDword(kCarShown, guest.dword(kCar));
        const reg32 stem = guest.call(kCarFileStem, args({ guest.dword(kCar) }));
        guest.call(kSprintf, args({}, { file, kFileFormat, stem }));
        const reg32 menu = guest.call(kMenuName);
        if (guest.call(kStrcmp, args({ menu, kCredits })) == 0)
        {
            guest.call(kSprintf, args({}, { file, kNumberedFormat, guest.dword(kCar) }));
            guest.copyString(folder, kInteriorRoot);
        }
        if (guest.call(kCarFlag, args({ guest.dword(kCar) })) != 0)
            guest.call(kSprintf, args({}, { folder, kFolderFormat, kCarsRoot, carFolder }));
        else
            guest.copyString(folder, kInteriorRoot);
        guest.call(kMakeTiles, args({ file, folder, cpu.ebx, bitsOf(kFullScale) }));
        guest.setDword(kPanBase, bitsOf(kFullScale));
        guest.setDword(kMapOffsetY, 0);
        const reg32 mapX = guest.call(kMapX, args({ guest.dword(kCar) }));
        guest.setDword(kMapOffsetX, mapX);
        const reg32 left = guest.call(kTrace, args({}, { 0x5a, 2, kTraceFormat, kTraceFile, 0x214, mapX }));
        guest.setDword(kPanY, 0);
        guest.setDword(kPanX, bitsOf(kFullScale));
        return left;
    });
}

}

}
