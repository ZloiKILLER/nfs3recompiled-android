#include "native_frontend_sime.h"
#include "port_snd.h"
#include "native_game.h"
#include "native_load_cardb.h"
#include "game_settings.h"
#include "native_carstate.h"
#include "native_ea_memory.h"
#include "native_ea_threads.h"
#include "native_frontend.h"
#include "native_frontend_fx.h"
#include "native_frontend_hudx.h"
#include "native_frontend_loada.h"
#include "native_frontend_menuf.h"
#include "native_frontend_pause.h"
#include "native_frontend_renda.h"
#include "native_frontend_rende.h"
#include "native_frontend_simc.h"
#include "native_frontend_simd.h"
#include "native_graphics.h"
#include "native_load_track.h"
#include "native_vertices.h"
#include "native_render.h"
#include <nfs3hp.h>
#include <fpu.h>
#include <cstdint>
#include <cstring>

/* What a race's effects are made from, and a few small pieces that sat among them,
 * decompiled:
 *
 *   particles      smoke, dust, spray and sparks, 600 of 128 bytes at 0x7ddfd0, by a
 *                  table of 14 kinds at 0x560b64: the slot a new one takes
 *                  (sub_4ca4b0) and the spawn (sub_4ca850); those a dying one hands on
 *                  (sub_4cab60), the sparks of a hit (sub_4cac20), a crash's puff
 *                  (sub_4cae00), the sparkle of a car put back (sub_4caea0) and a
 *                  wheel's dust (sub_4cb010);
 *   drops          what lands on a player's view, 200 of 64 bytes a player at 0x7f0bd0
 *                  (sub_4ca500);
 *   strips         the eight flat quads laid across the road at 0x7dd850 (+0x4c the tick
 *                  each comes on), and which of a car's wheels are over one (sub_4ca160):
 *                  the wheel hits a car keeps at +0x99c, by all appearances the cops'
 *                  spike strips;
 *   skid marks     the block of strips taken and given back (sub_4c70f0, sub_4c7150,
 *                  sub_4c7130), a segment written (sub_4c6c70), the marks of the wheels
 *                  at 0x8b7ba0 cleared and cut (sub_4dbbc0, sub_4dbba0, sub_4db860);
 *   a race's       what is reset as it starts and given back as it ends (sub_4bc8b0,
 *   beginning      sub_4bc950, with sub_4c7b20, sub_4d5730 and sub_4dbbe0, which do
 *   and end        nothing), the pieces of a track loaded with the loading bar
 *                  (sub_4dbbf0);
 *   the fog        its colour and distance where the camera is, from the zones at
 *                  [0x55d278] (sub_478e10);
 *   the views      all the race's view rectangles grown or shrunk (sub_4be970, sub_4bea40);
 *   a quick        the table of inverse roots at 0x79f364 (sub_496310) and a vector
 *   normal         scaled by it, one Newton step (sub_497500);
 *   leftovers      the folders named P and three digits under the cars' folder removed
 *                  (sub_43bcf0).
 *
 * These take the race's random numbers and are drawn on a replay, so every sum is the
 * x87's in the original's order: each fp:: call is an instruction's, its operands in the
 * order the game's code has them, a value that went through a float in memory goes
 * through a float here.  All the game's state stays in guest memory.  What is handed to
 * the game by address lives in the frame, where the game's function had it. */
#if defined(__clang__)
#pragma clang fp contract(off)
#else
#pragma fp_contract(off)
#endif

namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using fx::Arith;
using x86::Float;
using x86::reg16;
using x86::reg32;
using x86::sreg32;

// ---------------------------------------------------------------------------
// The x87 as these functions read it
// ---------------------------------------------------------------------------

/* What a jump after fcom; fnstsw ax; sahf sees of fp::compare's word. */
bool jumpsAbove(reg16 status) { return (status & 0x4100) == 0; }  // ja, not jbe; false unordered
bool jumpsBelow(reg16 status) { return (status & 0x0100) != 0; }  // jb, not jae; true unordered

reg32 bitsOf(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

float floatOf(reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

/* A float's sign changed in its top byte, as sub_4ca160 does it. */
float signFlipped(float value) { return floatOf(bitsOf(value) ^ 0x80000000u); }

/* A double of the game's constants, as fld and fmul qword read it. */
Float doubleAt(const Guest& guest, reg32 address)
{
    const std::uint64_t bits = std::uint64_t(guest.dword(address)) | (std::uint64_t(guest.dword(address + 4)) << 32);
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* fld dword */
Float floatFrom(const Guest& guest, reg32 address) { return Float(guest.floatAt(address)); }

void copyDwords(Guest& guest, reg32 to, reg32 from, reg32 count)
{
    for (reg32 i = 0; i < count; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

/* Words the game's code pushes for a callee that takes their address: put where those pushes put
 * them, below esp, and esp below them while this lives, so the callee's frame lies where the
 * game's does. */
class PushedWords
{
public:
    PushedWords(Guest& guest, x86::CPU& cpu, std::initializer_list<reg32> words)
        : m_cpu(cpu), m_esp(cpu.esp), m_address(cpu.esp - 4 * reg32(words.size()))
    {
        reg32 at = m_address;
        for (const reg32 word : words)
        {
            guest.setDword(at, word);
            at += 4;
        }
        cpu.esp = m_address;
    }
    ~PushedWords() { m_cpu.esp = m_esp; }
    PushedWords(const PushedWords&) = delete;
    PushedWords& operator=(const PushedWords&) = delete;
    reg32 address() const { return m_address; }

private:
    x86::CPU& m_cpu;
    const reg32 m_esp;
    const reg32 m_address;
};


/* Whether a float is either zero, by its bits, as `test x, 0x7fffffff` asks. */
bool isZero(reg32 bits) { return (bits & 0x7fffffffu) == 0; }

/* The original's own absolute value, inline at each use: fldz; fcomp x, and the sign
 * changed only below 0, so that -0 stays -0 and a NaN stays as it is. */
float absolute(float value)
{
    return jumpsAbove(fp::compare(0.0, Float(value))) ? float(-Float(value)) : value;
}

// ---------------------------------------------------------------------------
// The game's functions, by address
// ---------------------------------------------------------------------------

constexpr reg32 kRandom = 0x4e0d88;
constexpr reg32 kFillZero = 0x4e070c;          // eax where, edx the bytes
constexpr reg32 kLoadTrackTextures = 0x41dbc0; // eax the track
constexpr reg32 kLoadHorizon = 0x478340;       // eax the track
constexpr reg32 kRaceViews = 0x41d960;
constexpr reg32 kDropEffect = 0x415010;        // eax the player

// This group's own, by address too: as the callers elsewhere reach them.

// ---------------------------------------------------------------------------
// The game's data
// ---------------------------------------------------------------------------

constexpr reg32 kClock = 0x7d3684;             // the race's ticks
constexpr reg32 kDriverFlags = 0x7a3a58;       // a byte: 1 << the driver's type
constexpr std::uint8_t kSoftwareDrivers = 0x06;

// A kind of particle: 128 bytes at 0x560b64, 14 of them.
constexpr reg32 kKinds = 0x560b64;
constexpr reg32 kKindCount = 0x00;             // how many a spawn makes
constexpr reg32 kKindLife = 0x04;
constexpr reg32 kKindEvery = 0x08;             // spawned on every (this + 1)th tick of two
constexpr reg32 kKindFullSpeed = 0x0c;         // float: the speed its scale is 1 at
constexpr reg32 kKindSpread = 0x10;            // three floats: its speed each way
constexpr reg32 kKindJitter = 0x1c;            // float: how much of the spread is chance
constexpr reg32 kKindSwirl = 0x20;
constexpr reg32 kKindSwirlRate = 0x24;         // float
constexpr reg32 kKindSwirlFade = 0x28;         // float
constexpr reg32 kKindSpin = 0x30;              // float
constexpr reg32 kKindBackSpeed = 0x34;         // float: how fast along the third vector
constexpr reg32 kKindSize = 0x40;              // float
constexpr reg32 kKindGrowth = 0x44;            // float
constexpr reg32 kKindSizeJitter = 0x48;        // float
constexpr reg32 kKindColour = 0x4c;
constexpr reg32 kKindColourJitter = 0x54;      // four bytes
constexpr reg32 kKindFlags = 0x5c;             // a byte
constexpr reg32 kKindDrops = 0x64;             // how many drops a spawn makes, if any
constexpr reg32 kKindDropLife = 0x68;
constexpr reg32 kKindDropFlags = 0x6c;
constexpr reg32 kKindDropSize = 0x70;          // float
constexpr reg32 kKindDropJitter = 0x74;        // float
constexpr reg32 kKindDropColour = 0x78;
constexpr reg32 kKindDropChance = 0x7c;        // float: the share of spawns that land, 0 for all

constexpr reg32 kKindNone = 0;
constexpr reg32 kKindSparks = 3;
constexpr reg32 kKindWithJolt = 4;             // its drops tell sub_415010
constexpr reg32 kKindDrip = 0xa;               // none on the software drivers; its drop runs kind 11's off
constexpr reg32 kKindRunOff = 0xb;
constexpr reg32 kKindNoExtra = 0xc;

reg32 kindAt(reg32 kind) { return kKinds + (kind << 7); }

// A particle: 128 bytes at 0x7ddfd0, 600 of them.
constexpr reg32 kParticles = 0x7ddfd0;
constexpr reg32 kParticleCount = 600;
constexpr reg32 kParticleLife = 0x00;
constexpr reg32 kParticleKind = 0x04;
constexpr reg32 kParticleLifeAtStart = 0x08;
constexpr reg32 kParticlePosition = 0x0c;
constexpr reg32 kParticleVelocity = 0x18;
constexpr reg32 kParticleSize = 0x24;
constexpr reg32 kParticleSizeAtStart = 0x28;
constexpr reg32 kParticleGrowth = 0x2c;
constexpr reg32 kParticleAngle = 0x30;
constexpr reg32 kParticleSpin = 0x34;
constexpr reg32 kParticleCosine = 0x38;
constexpr reg32 kParticleSine = 0x3c;
constexpr reg32 kParticleChance = 0x40;
constexpr reg32 kParticleSwirl = 0x44;
constexpr reg32 kParticleSwirlRate = 0x48;
constexpr reg32 kParticleSwirlFade = 0x4c;
constexpr reg32 kParticleTexture = 0x50;
constexpr reg32 kParticleColour = 0x54;
constexpr reg32 kParticleFade = 0x78;          // four bytes
constexpr reg32 kParticleFlags = 0x7c;         // a byte
constexpr reg32 kParticleMinor = 0x7d;         // a byte: dropped when effects are low
constexpr reg32 kParticleSpawns = 0x7e;        // a byte: how many it hands on as it dies

constexpr reg32 kParticleTextures = 0x7ddd68;  // 0x2c bytes a kind
constexpr reg32 kDropTextures = 0x7ddb00;      // 0x2c bytes a kind
constexpr reg32 kKindTextureBytes = 0x2c;

// The drops on a player's view: 200 of 64 bytes at 0x7f0bd0, 0x3200 bytes a player.
constexpr reg32 kDrops = 0x7f0bd0;
constexpr reg32 kDropsPerPlayer = 0x3200;
constexpr reg32 kDropCount = 200;
constexpr reg32 kDropLife = 0x00;
constexpr reg32 kDropKind = 0x04;
constexpr reg32 kDropCorners = 0x08;           // eight floats
constexpr reg32 kDropColour = 0x28;
constexpr reg32 kDropFlags = 0x2c;
constexpr reg32 kDropTexture = 0x30;
constexpr reg32 kDropX = 0x34, kDropY = 0x38;  // floats: its centre
constexpr reg32 kDropSize = 0x3c;              // float
constexpr reg32 kViewCovered = 0x79c4a0;       // by player: nothing lands on it while set

// A car.
constexpr reg32 kCarNode = 0x1c;
constexpr reg32 kCarPosition = 0x98;
constexpr reg32 kCarVelocity = 0xa4;           // three floats
constexpr reg32 kCarSpeed = 0xb8;              // float
constexpr reg32 kCarMatrix = 0xc0;
constexpr reg32 kCarWheels = 0x304;            // their points, three 16.16 each, kCarWheelBytes apart
constexpr reg32 kCarWheelBytes = 0x78;
constexpr reg32 kCarBackWheelsExtra = 0x594;   // nonzero: wheels 2 and 3 throw more

// ---------------------------------------------------------------------------
// The folders
// ---------------------------------------------------------------------------

constexpr reg32 kCarsFolder = 0x7a28b4;
constexpr reg32 kCharacterKinds = 0x564ef0;    // by the character + 1, as the runtime has it
constexpr std::uint8_t kDigit = 0x20;

bool isDigit(const Guest& guest, reg32 at)
{
    return (guest.byte(kCharacterKinds + ((guest.byte(at) + 1) & 0xff)) & kDigit) != 0;
}

}  // namespace

/* sub_43bcf0: every folder under the cars' folder named P and three digits, removed.
 * What is left in eax is the last answer, which is 0. */
x86::reg32 frontend::sime::removeNumberedFolders(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 20 + 0x12c);
    const reg32 path = guest.frame();
    reg32 name = *fe::direct<0x4386b0, load::firstSubfolderIn>(app, cpu, kCarsFolder);
    while (name != 0)
    {
        if (guest.byte(name) == 'P' && isDigit(guest, name + 1) && isDigit(guest, name + 2) && isDigit(guest, name + 3))
        {
            guest.copyString(path, kCarsFolder);
            fe::direct<0x438340, load::slashAdded>(app, cpu, path);
            guest.copyString(path + guest.length(path), name);
            fe::direct<0x4389d0, load::folderRemoved>(app, cpu, path);
        }
        name = *fe::direct<0x4387e0, load::nextSubfolderFound>(app, cpu);
    }
    return name;
}

namespace
{

// ---------------------------------------------------------------------------
// The fog
// ---------------------------------------------------------------------------

constexpr reg32 kFogBlue = 0x55d26c, kFogGreen = 0x55d26d, kFogRed = 0x55d26e;  // bytes: with no zone
constexpr reg32 kFogDistance = 0x55d270;
constexpr reg32 kFogZoneCount = 0x55d274;
constexpr reg32 kFogZones = 0x55d278;
constexpr reg32 kFogZoneBytes = 0x24;
// A zone: at its first node, its middle and its last, the colour (blue, green, red, a byte
// spare) and the distance, and the three nodes.
constexpr reg32 kZoneStartColour = 0x00, kZoneMiddleColour = 0x04, kZoneEndColour = 0x08;
constexpr reg32 kZoneStartDistance = 0x0c, kZoneMiddleDistance = 0x10, kZoneEndDistance = 0x14;
constexpr reg32 kZoneFirstNode = 0x18, kZoneMiddleNode = 0x1c, kZoneLastNode = 0x20;

constexpr reg32 opaque(reg32 red, reg32 green, reg32 blue)
{
    return (red << 16) | 0xff000000u | (green << 8) | blue;
}

/* A zone's colour and distance at a share of the way from its middle towards one end; they
 * go out rounded towards zero, as __CHP leaves them.  eax's value for the caller, the
 * distance; edx left the colour. */
reg32 fogBetween(Guest& guest, reg32 zone, reg32 otherColour, reg32 otherDistance, const Float& share, reg32 colourOut,
                 reg32 distanceOut)
{
    Arith x87(guest);
    const float stored = float(share);
    const Float rest = fp::sub(1.0, share);
    const auto channel = [&](reg32 at) { return Float(sreg32(guest.byte(at))); };
    const Float red = fp::add(fp::mul(channel(zone + kZoneMiddleColour + 2), rest), fp::mul(channel(zone + otherColour + 2), Float(stored)));
    const Float green = fp::add(fp::mul(channel(zone + kZoneMiddleColour + 1), rest), fp::mul(channel(zone + otherColour + 1), Float(stored)));
    const Float blue = fp::add(fp::mul(channel(zone + kZoneMiddleColour), rest), fp::mul(channel(zone + otherColour), Float(stored)));
    const Float distance = fp::add(fp::mul(rest, Float(sreg32(guest.dword(zone + kZoneMiddleDistance)))),
                                   fp::mul(Float(sreg32(guest.dword(zone + otherDistance))), Float(stored)));
    const reg32 redNow = reg32(x87.truncated(red));
    const reg32 greenNow = reg32(x87.truncated(green));
    const reg32 blueNow = reg32(x87.truncated(blue));
    const reg32 distanceNow = reg32(x87.truncated(distance));
    const reg32 colour = opaque(redNow, greenNow, blueNow);
    guest.setDword(colourOut, colour);
    guest.setDword(distanceOut, distanceNow);
    return distanceNow;
}

}  // namespace

/* sub_478e10: the fog's colour and distance where the view's camera is: eax the view, edx and
 * ebx where they go.  With no zone there the track's own; in one, blended from its middle
 * towards the end the node is on.  eax, ebx and edx are left as the original leaves them. */
x86::reg32 frontend::sime::fogAtCamera(App* app, CPU& cpu, reg32 view, reg32 colourOut, reg32 distanceOut)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 16 + 0x40);
    const reg32 zones = guest.dword(kFogZones);
    // Handed the zones and 0 in edx and ebx too, which it does not read: there as the code has them.
    const sreg32 node = sreg32(fe::direct<0x4217e0, carstate::nodeOfCamera>(app, cpu, guest.dword(view + 4)));
    guest.setDword(colourOut, opaque(guest.byte(kFogRed), guest.byte(kFogGreen), guest.byte(kFogBlue)));
    guest.setDword(distanceOut, guest.dword(kFogDistance));

    reg32 zone = zones;
    sreg32 i = 0;
    for (; i < sreg32(guest.dword(kFogZoneCount)); ++i, zone += kFogZoneBytes)
    {
        const sreg32 first = sreg32(guest.dword(zone + kZoneFirstNode));
        const sreg32 middle = sreg32(guest.dword(zone + kZoneMiddleNode));
        const sreg32 last = sreg32(guest.dword(zone + kZoneLastNode));
        const sreg32 before = sreg32(reg32(middle) - reg32(first));
        const sreg32 after = sreg32(reg32(last) - reg32(middle));
        if (node < first || node > last)
            continue;
        if (node < middle && before != 0)
        {
            const sreg32 left = sreg32(reg32(middle) - reg32(node));
            return fogBetween(guest, zone, kZoneStartColour, kZoneStartDistance, fp::div(Float(left), Float(before)), colourOut, distanceOut);
        }
        if (node > middle && after != 0)
        {
            const sreg32 past = sreg32(reg32(node) - reg32(middle));
            return fogBetween(guest, zone, kZoneEndColour, kZoneEndDistance, fp::div(Float(past), Float(after)), colourOut, distanceOut);
        }
        const reg32 colour = opaque(guest.byte(zone + kZoneMiddleColour + 2), guest.byte(zone + kZoneMiddleColour + 1),
                                    guest.byte(zone + kZoneMiddleColour));
        const reg32 distance = guest.dword(zone + kZoneMiddleDistance);
        guest.setDword(colourOut, colour);
        guest.setDword(distanceOut, distance);
        return distance;
    }
    return reg32(node);
}

namespace
{

// ---------------------------------------------------------------------------
// The quick normal
// ---------------------------------------------------------------------------

constexpr reg32 kRootTable = 0x79f364;         // by a square's low exponent bit and six of its mantissa's
constexpr reg32 kRootTableEntries = 0x80;
constexpr reg32 kRootTableOfOne = 0x79f3a4;    // 1 / sqrt(1): its mantissa would wrap to 0

}  // namespace

/* sub_496310: the top bits of the mantissas of 1 / sqrt(x), for x from 0.5 up to 2, rounded.
 * eax is left the last entry. */
x86::reg32 frontend::sime::makeRootTable(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 12 + 4);
    reg32 entry = 0;
    for (reg32 i = 0; i < kRootTableEntries; ++i)
    {
        const double root = guest::direct<0x4ea920, simc::squareRootOf>(app, cpu, (i << 17) | 0x3f000000u);
        const float inverse = float(fp::div(1.0, root));
        entry = ((bitsOf(inverse) + 0x2000u) >> 15) & 0xff;
        guest.setByte(kRootTable + i, std::uint8_t(entry));
    }
    guest.setByte(kRootTableOfOne, 0xff);
    return entry;
}

namespace
{

/* sub_497500: eax's vector scaled to length 1, to edx, by the table and a step of Newton's. */
void quickNormalInto(Guest& guest, reg32 vector, reg32 out)
{
    constexpr reg32 kThree = 0x53bf3c, kHalf = 0x53bf44;  // doubles
    const Float y = floatFrom(guest, vector + 4);
    const Float squareY = fp::mul(y, Float(y));
    const Float x = floatFrom(guest, vector);
    const Float squareX = fp::mul(x, Float(x));
    const Float sumXY = fp::add(squareY, squareX);
    const Float z = floatFrom(guest, vector + 8);
    const Float squareZ = fp::mul(z, Float(z));
    const float square = float(fp::add(sumXY, squareZ));

    const reg32 bits = bitsOf(square);
    const reg32 mantissa = guest.byte(kRootTable + ((bits >> 17) & 0x7f));
    const reg32 exponent = (0x17cu - ((bits >> 23) & 0xff)) >> 1;
    const Float estimate = Float(floatOf((exponent << 23) | (mantissa << 15)));

    const Float estimateSquared = fp::mul(estimate, Float(estimate));
    const Float product = fp::mul(estimateSquared, Float(square));
    const Float three = fp::sub(doubleAt(guest, kThree), product);
    const Float scaled = fp::mul(three, Float(estimate));
    const Float inverse = fp::mul(scaled, doubleAt(guest, kHalf));

    guest.setFloat(out, float(fp::mul(floatFrom(guest, vector), Float(inverse))));
    guest.setFloat(out + 4, float(fp::mul(floatFrom(guest, vector + 4), Float(inverse))));
    guest.setFloat(out + 8, float(fp::mul(inverse, floatFrom(guest, vector + 8))));
}

// ---------------------------------------------------------------------------
// A race's beginning and end
// ---------------------------------------------------------------------------

}  // namespace

/* sub_4bc8b0: what a race starts from. */
x86::reg32 frontend::sime::effectsReset(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    // (sub_434ae0 is an empty function)
    fx::callAt(guest, 0x421480);
    guest::direct<0x4c7130, skidStripsReset>(app, cpu);
    guest::direct<0x4dbbc0, wheelMarksReset>(app, cpu);
    fe::direct<0x4cb6f0, rende::clearParticles>(app, cpu);
    fe::direct<0x47b390, renda::flashesResetAgain>(app, cpu);
    guest::direct<0x4dcd10, simd::weatherStep>(app, cpu);
    return fe::direct<0x4bc530, menuf::resetRaceMouse>(app, cpu);
}

/* sub_4bc950: and what it gives back. */
x86::reg32 frontend::sime::effectsFree(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    // (sub_434af0 is an empty function)
    // What each leaves in eax goes on to the next, as the code has it there.
    guest::direct<0x4d5730, effectsEndNothing>(app, cpu);
    const reg32 strips = guest::direct<0x4c7150, skidStripsFree>(app, cpu);
    guest::direct<0x4dbbe0, wheelMarksEndEmpty>(app, cpu, strips);
    fe::direct<0x484220, hudx::clearRebuildFlag>(app, cpu);
    const reg32 cleared = fe::direct<0x4cb730, rende::clearParticleStates>(app, cpu);
    return guest::direct<0x4c7b20, effectsEndEmpty>(app, cpu, cleared);
}

/* sub_4dbbf0: a track's textures, horizon, views and what goes with them, the loading bar
 * moved on after each.  Told to stop on the way ([0x55e88c]), 1 at once; else 0. */
x86::reg32 frontend::sime::loadTrackPieces(App* app, CPU& cpu, reg32 track)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    constexpr reg32 kLoadingStopped = 0x55e88c;
    constexpr reg32 kBar40 = 0x42200000, kBar43 = 0x422c0000, kBar58 = 0x42680000, kBar61 = 0x42740000, kBar64 = 0x42800000;
    const auto stopped = [&] {
        fe::direct<0x4a33d0, loada::stopCheck>(app, cpu);
        return guest.dword(kLoadingStopped) != 0;
    };
    // The bar's per cent, the word the code pushes, handed by its address.
    const auto progress = [&](reg32 perCent) {
        const PushedWords pushed(guest, cpu, { perCent });
        fe::direct<0x495230, graphics::loadingProgress>(app, cpu, pushed.address());
    };
    fe::direct<0x41dbc0, load::trackTexturesLoaded>(app, cpu);
    if (stopped())
        return 1;
    progress(kBar40);
    fe::direct<0x478340, load::horizonRead>(app, cpu, track);
    if (stopped())
        return 1;
    progress(kBar43);
    fe::direct<0x41d960, render::raceViewsSet>(app, cpu);
    if (stopped())
        return 1;
    progress(kBar58);
    fe::direct<0x4bc870, loada::raceResetSteps>(app, cpu);
    if (stopped())
        return 1;
    progress(kBar61);
    fe::direct<0x4782c0, loada::horizonStartCall>(app, cpu);
    if (stopped())
        return 1;
    progress(kBar64);
    return 0;
}

namespace
{

// ---------------------------------------------------------------------------
// The view rectangles
// ---------------------------------------------------------------------------

constexpr reg32 kScreenExtent = 0x7cda68;      // what a rectangle's +8 is held between half of and all of
constexpr reg32 kFullView = 0x7cdb88, kTopView = 0x7cda80, kBottomView = 0x7cdaa0;
// A rectangle: its near sides, its far sides, then the two again.
constexpr reg32 kViewNearA = 0x00, kViewNearB = 0x04, kViewFarA = 0x08, kViewFarB = 0x0c;
constexpr reg32 kViewNearA2 = 0x10, kViewNearB2 = 0x14, kViewFarA2 = 0x18, kViewFarB2 = 0x1c;

}  // namespace

/* sub_4bea40: a rectangle grown by edx one way and three quarters of that the other, about
 * its centre, as long as it stays between half the screen and all of it.  eax and edx are
 * left as the original leaves them. */
x86::reg32 frontend::sime::viewResize(App* app, CPU& cpu, reg32 view, reg32 changeWord)
{
    const sreg32 change = sreg32(changeWord);
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 20 + 4);
    const sreg32 across = sreg32(3u * reg32(change)) / 4;
    if (change == 0)
    {
        return reg32(across);
    }
    const sreg32 extent = sreg32(guest.dword(kScreenExtent));
    const sreg32 half = extent / 2;
    const sreg32 farThen = sreg32(guest.dword(view + kViewFarA) + reg32(change));
    const sreg32 farAcross = sreg32(guest.dword(view + kViewFarB) + reg32(across));
    if (farThen < half || farThen > extent || farAcross < 0)
    {
        return reg32(half);
    }
    const reg32 changeHalf = reg32(change / 2);
    guest.setDword(view + kViewNearA, guest.dword(view + kViewNearA) - changeHalf);
    guest.setDword(view + kViewNearA2, guest.dword(view + kViewNearA2) - changeHalf);
    const reg32 acrossHalf = reg32(across / 2);
    guest.setDword(view + kViewNearB2, guest.dword(view + kViewNearB2) - acrossHalf);
    guest.setDword(view + kViewNearB, guest.dword(view + kViewNearB) - acrossHalf);
    const reg32 farNow = guest.dword(view + kViewFarA) + reg32(change);
    guest.setDword(view + kViewFarA, farNow);
    guest.setDword(view + kViewFarB2, guest.dword(view + kViewFarB2) + reg32(across));
    const reg32 farAgainNow = guest.dword(view + kViewFarA2) + reg32(change);
    guest.setDword(view + kViewFarA2, farAgainNow);
    guest.setDword(view + kViewFarB, guest.dword(view + kViewFarB) + reg32(across));
    return farNow;
}

/* sub_4be970: the views a step bigger (eax not negative) or smaller: by 32 on a driver with
 * bit 2 of its flags, else a fortieth of the screen.  The split screen's two keep +0x14 and
 * +0x1c.  A view that shrank asks for its surround to be drawn twice more ([0x7cdba8]).  With
 * [0x6fbc4c] set, only with two players on the machine's count of 1. */
x86::reg32 frontend::sime::viewsResize(App* app, CPU& cpu, reg32 direction)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24);
    constexpr reg32 kSurroundDirty = 0x7cdba8;
    constexpr std::uint8_t kFixedStepDrivers = 0x02;
    reg32 eax = direction;
    sreg32 step = 0x20;
    if ((guest.byte(kDriverFlags) & kFixedStepDrivers) == 0)
    {
        step = sreg32(guest.dword(kScreenExtent)) / 0x28;
        eax = reg32(step);
    }
    if (sreg32(direction) < 0)
        step = sreg32(0u - reg32(step));
    if (gameSettings(guest).wideScreen != 0 && gameSettings(guest).playMode != 1)
        return eax;

    const reg32 oldFar = guest.dword(kFullView + kViewFarA2);
    guest::direct<0x4bea40, viewResize>(app, cpu, kFullView, reg32(step));
    const reg32 topNear = guest.dword(kTopView + kViewNearB2), topFar = guest.dword(kTopView + kViewFarB2);
    guest::direct<0x4bea40, viewResize>(app, cpu, kTopView, reg32(step));
    guest.setDword(kTopView + kViewNearB2, topNear);
    guest.setDword(kTopView + kViewFarB2, topFar);
    const reg32 bottomNear = guest.dword(kBottomView + kViewNearB2), bottomFar = guest.dword(kBottomView + kViewFarB2);
    guest::direct<0x4bea40, viewResize>(app, cpu, kBottomView, reg32(step));
    guest.setDword(kBottomView + kViewNearB2, bottomNear);
    guest.setDword(kBottomView + kViewFarB2, bottomFar);
    const reg32 result = fe::direct<0x4be3d0, pause::fillViewQuads>(app, cpu);
    if (sreg32(oldFar) > sreg32(guest.dword(kFullView + kViewFarA2)))
        guest.setDword(kSurroundDirty, 2);
    return result;
}

namespace
{

// ---------------------------------------------------------------------------
// The skid marks
// ---------------------------------------------------------------------------

constexpr reg32 kSkidStrips = 0x560110;        // the block
constexpr reg32 kSkidStripsName = 0x540220;
constexpr reg32 kSkidStripsBytes = 0xd000;
constexpr reg32 kSkidFirstStripCount = 0x0c;   // the first strip's segments
constexpr reg32 kSkidStripNow = 0x7d5754, kSkidStripsUsed = 0x7d5758, kSkidDrawnAt = 0x7d5750;
constexpr reg32 kWheelMarks = 0x8b7ba0;
constexpr reg32 kWheelMarksBytes = 0xd00;
constexpr reg32 kWheelMarksPerCar = 0xd0;
constexpr reg32 kWheelMarkDwords = 13;
constexpr reg32 kMarkSliding = 0x2c, kMarkFresh = 0x30;

/* sub_4c6c70: a segment of a strip, eax its corners and edx its two alphas: ebx's two sides,
 * then ecx's the other way round; the alphas the stack's low bytes.  What is left in eax is
 * the original's: the corners' address with the last alpha in its low byte. */
}  // namespace

x86::reg32 frontend::sime::skidSegmentPut(App* app, CPU& cpu, reg32 corners, reg32 alphas, reg32 from, reg32 to,
                                          reg32 alphaFrom, reg32 alphaTo)
{
    Guest guest(app, cpu);
    copyDwords(guest, corners, from, 3);
    copyDwords(guest, corners + 0x0c, from + 0x0c, 3);
    copyDwords(guest, corners + 0x18, to + 0x0c, 3);
    copyDwords(guest, corners + 0x24, to, 3);
    guest.setByte(alphas, std::uint8_t(alphaFrom));
    guest.setByte(alphas + 1, std::uint8_t(alphaTo));
    return (corners & 0xffffff00u) | (alphaTo & 0xff);
}

/* sub_4c70f0: the strips' block taken, and none laid yet. */
x86::reg32 frontend::sime::skidStripsTake(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 12);
    const reg32 block = *fe::direct<0x4e1620, ea::reserved>(app, cpu, kSkidStripsName, kSkidStripsBytes, 0);
    guest.setDword(kSkidStrips, block);
    guest.setDword(kSkidStripsUsed, 0);
    guest.setDword(block + kSkidFirstStripCount, 0);
    const reg32 now = guest.dword(kClock);
    guest.setDword(kSkidStripNow, 0);
    guest.setDword(kSkidDrawnAt, now);
    return now;
}

/* sub_4c7150: and given back, if there is one. */
x86::reg32 frontend::sime::skidStripsFree(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 12);
    const reg32 block = guest.dword(kSkidStrips);
    if (block == 0)
        return 0;
    const reg32 result = *fe::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
    guest.setDword(kSkidStrips, 0);
    return result;
}

/* sub_4c7130: both. */
x86::reg32 frontend::sime::skidStripsReset(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    guest::direct<0x4c7150, skidStripsFree>(app, cpu);
    guest::direct<0x4c70f0, skidStripsTake>(app, cpu);
    const reg32 now = guest.dword(kClock);
    guest.setDword(kSkidDrawnAt, now);
    return now;
}

/* sub_4db860: a wheel's mark on a car (eax's low four bits, edx) cut: the next starts afresh.
 * eax is left the wheel's place in dwords. */
x86::reg32 frontend::sime::wheelMarkCut(App* app, CPU& cpu, reg32 car, reg32 wheel)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 12);
    const reg32 row = wheel * kWheelMarkDwords;
    const reg32 mark = kWheelMarks + (car & 0xf) * kWheelMarksPerCar + row * 4;
    guest.setDword(mark + kMarkSliding, 0);
    guest.setDword(mark + kMarkFresh, 0);
    return row;
}

namespace
{

// ---------------------------------------------------------------------------
// The strips across the road
// ---------------------------------------------------------------------------

constexpr reg32 kStrips = 0x7dd850;
constexpr reg32 kStripBytes = 0x50;
constexpr reg32 kStripCount = 8;
constexpr reg32 kStripCorners = 0x04;          // four points of three floats
constexpr reg32 kStripNode = 0x34;
constexpr reg32 kStripOnFrom = 0x4c;           // the tick it comes on
constexpr reg32 kNodeCount = 0x5dd940;

/* A point times the matrix in the frame, the sums as the original's inlined product takes
 * them. */
void rotated(const Guest& guest, reg32 matrix, float x, float y, float z, float out[3])
{
    const auto m = [&](reg32 i) { return floatFrom(guest, matrix + 4 * i); };
    const Float x0 = fp::mul(Float(x), m(0)), x1 = fp::mul(Float(x), m(1)), x2 = fp::mul(Float(x), m(2));
    const Float y0 = fp::mul(Float(y), m(3)), y1 = fp::mul(Float(y), m(4)), y2 = fp::mul(Float(y), m(5));
    const Float a0 = fp::add(x0, y0), a1 = fp::add(x1, y1), a2 = fp::add(x2, y2);
    const Float z0 = fp::mul(Float(z), m(6)), z1 = fp::mul(Float(z), m(7)), z2 = fp::mul(Float(z), m(8));
    out[0] = float(fp::add(a0, z0));
    out[1] = float(fp::add(a1, z1));
    out[2] = float(fp::add(a2, z2));
}

}  // namespace

/* sub_4ca160: the first strip that is on and within a node of car eax with any of the car's
 * four wheels over it: its number to edx's address, and a bit a wheel in eax.  None, 0.  A
 * wheel is over it unless all its corners, seen from the wheel in the car's axes, are beyond
 * half a unit on the same side.  edx is left as the original leaves it. */
x86::reg32 frontend::sime::stripUnderWheels(App* app, CPU& cpu, reg32 car, reg32 out)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 20 + 0x7c);
    constexpr reg32 kFromFixed = 0x540380, kHalf = 0x540388, kMinusHalf = 0x540390;  // doubles
    const reg32 matrix = guest.frame();
    reg32 edx = 0;
    for (reg32 i = 0; i < kStripCount; ++i)
    {
        const reg32 strip = kStrips + i * kStripBytes;
        edx = i;
        if (guest.dword(strip) == 0)
            continue;
        edx = guest.dword(kClock);
        if (sreg32(edx) < sreg32(guest.dword(strip + kStripOnFrom)))
            continue;
        const sreg32 count = sreg32(guest.dword(kNodeCount));
        const sreg32 node = sreg32(guest.dword(car + kCarNode));
        sreg32 before = sreg32(reg32(node) - 1);
        if (before < 0)
            before = sreg32(reg32(before) + reg32(count));
        sreg32 next = sreg32(reg32(node) + 1);
        if (next >= count)
            next = sreg32(reg32(next) - reg32(count));
        edx = reg32(next);
        const sreg32 stripNode = sreg32(guest.dword(strip + kStripNode));
        if (stripNode != before && stripNode != node && next != stripNode)
            continue;

        guest::direct<0x4e0110, simd::transpose>(app, cpu, car + kCarMatrix, matrix);
        reg32 wheels = 0;
        for (reg32 wheel = 0; wheel < 4; ++wheel)
        {
            const reg32 point = car + kCarWheels + wheel * kCarWheelBytes;
            const Float fromFixed = doubleAt(guest, kFromFixed);
            const float x = float(fp::mul(Float(sreg32(guest.dword(point))), fromFixed));
            const float y = float(fp::mul(Float(sreg32(guest.dword(point + 4))), fromFixed));
            const float z = float(fp::mul(fromFixed, Float(sreg32(guest.dword(point + 8)))));
            float wheelInCar[3];
            rotated(guest, matrix, signFlipped(x), signFlipped(y), signFlipped(z), wheelInCar);

            reg32 sides[4];
            for (reg32 corner = 0; corner < 4; ++corner)
            {
                const reg32 at = strip + kStripCorners + corner * 12;
                float seen[3];
                rotated(guest, matrix, guest.floatAt(at), guest.floatAt(at + 4), guest.floatAt(at + 8), seen);
                float from[3];
                for (reg32 axis = 0; axis < 3; ++axis)
                    from[axis] = float(fp::add(Float(seen[axis]), Float(wheelInCar[axis])));
                reg32 side = 0;
                for (reg32 axis = 0; axis < 3; ++axis)
                    if (jumpsAbove(fp::compare(Float(from[axis]), doubleAt(guest, kHalf))))
                        side |= reg32(2) << (axis * 2);
                for (reg32 axis = 0; axis < 3; ++axis)
                    if (jumpsBelow(fp::compare(Float(from[axis]), doubleAt(guest, kMinusHalf))))
                        side |= reg32(1) << (axis * 2);
                sides[corner] = side;
            }
            edx = sides[1];
            if ((sides[0] & sides[1] & sides[2] & sides[3]) == 0)
                wheels |= reg32(1) << wheel;
        }
        if (wheels != 0)
        {
            guest.setDword(out, i);
            return wheels;
        }
    }
    return 0;
}

namespace
{

// ---------------------------------------------------------------------------
// Particles
// ---------------------------------------------------------------------------

/* What sub_4eb230 leaves on the x87. */
Float randomFraction(x86::CPU& cpu, Guest& guest)
{
    return *fe::direct<0x4eb230, ea::randomFractionOf>(guest.app(), cpu);
}

}  // namespace

/* sub_4ca4b0: a slot for a new particle: the first free, else the one with the least life
 * left, of equals the last.  (With 600 to look at, its fallback for finding none is dead.) */
x86::reg32 frontend::sime::particleSlot(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 16);
    reg32 slot = 0;
    for (reg32 i = 0; i < kParticleCount; ++i)
    {
        const reg32 particle = kParticles + (i << 7);
        const sreg32 life = sreg32(guest.dword(particle + kParticleLife));
        if (life == 0)
            return particle;
        if (slot == 0 || life <= sreg32(guest.dword(slot + kParticleLife)))
            slot = particle;
    }
    return slot;
}

namespace
{

/* A spawn's stack, as sub_4ca850 reads it. */
struct Spawn
{
    reg32 position[3];
    reg32 scale;
    reg32 along[3];
    reg32 up[3];
    reg32 back[3];
};

/* sub_4ca850 called as C++, the spawn's words put where the game's code pushes them. */
reg32 spawnParticleOf(x86::CPU& cpu, Guest& guest, reg32 kind, const Spawn& s)
{
    const PushedWords pushed(guest, cpu,
                             { s.position[0], s.position[1], s.position[2], s.scale, s.along[0], s.along[1], s.along[2],
                               s.up[0], s.up[1], s.up[2], s.back[0], s.back[1], s.back[2] });
    return guest::direct<0x4ca850, frontend::sime::spawnParticle>(guest.app(), cpu, kind, pushed.address());
}

}  // namespace

/* sub_4ca850: a particle of a kind (eax) where the stack says, going the kind's second speed
 * along the first vector, its third along the second, its fourth along the third, and its
 * first straight up, each but the fourth with its chance, and the first two vectors' by the
 * scale.  Effects off, or low and this one minor, or not its tick: 0.  Otherwise the
 * particle, whose life is 0 if it does not move. */
x86::reg32 frontend::sime::spawnParticle(App* app, CPU& cpu, reg32 kind, reg32 on)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x2c);
    constexpr reg32 kPosition = 0, kScale = 0xc, kAlong = 0x10, kUp = 0x1c, kBack = 0x28;
    constexpr reg32 kMinusHalf = 0x5403a8;  // a double
    const reg32 particle = guest::direct<0x4ca4b0, particleSlot>(app, cpu);
    const reg32 shape = kindAt(kind);
    const float spreadX = guest.floatAt(shape + kKindSpread);
    const float spreadY = guest.floatAt(shape + kKindSpread + 4);
    const float spreadZ = guest.floatAt(shape + kKindSpread + 8);
    const float backSpeed = guest.floatAt(shape + kKindBackSpeed);

    const sreg32 halfTicks = sreg32(guest.dword(kClock)) >> 1;
    const sreg32 every = sreg32(guest.dword(shape + kKindEvery));
    const reg32 minor = (halfTicks % sreg32(reg32(every) * 2 + 2)) == 0 ? 1 : 0;
    const sreg32 level = sreg32(gameSettings(guest).effectsLevel);
    if (level == 2 || (level == 1 && minor != 0))
        return 0;
    if (halfTicks % sreg32(reg32(every) + 1) != 0)
        return 0;

    guest.setDword(particle + kParticleKind, kind);
    const reg32 life = guest.dword(shape + kKindLife);
    guest.setDword(particle + kParticleLife, life);
    guest.setDword(particle + kParticleLifeAtStart, life);
    guest.setByte(particle + kParticleMinor, std::uint8_t(minor));
    copyDwords(guest, particle + kParticlePosition, on + kPosition, 3);

    const Float minusHalf = doubleAt(guest, kMinusHalf);
    const float scale = guest.floatAt(on + kScale);
    const auto chance = [&] {
        const Float centred = fp::add(randomFraction(cpu, guest), minusHalf);
        return fp::add(fp::mul(centred, floatFrom(guest, shape + kKindJitter)), 1.0);
    };
    const float chanceX = float(fp::mul(chance(), Float(spreadX)));
    const float chanceY = float(fp::mul(chance(), Float(spreadY)));
    const Float chanceZ = chance();

    const auto vectorOf = [&](reg32 at) { return floatFrom(guest, on + at); };
    const Float y = fp::mul(Float(chanceY), Float(scale));
    const Float z = fp::mul(fp::mul(chanceZ, Float(spreadZ)), Float(scale));
    const Float back = Float(backSpeed);
    const Float x = fp::mul(Float(chanceX), Float(scale));

    const Float vx = fp::add(fp::add(fp::mul(y, vectorOf(kAlong)), fp::mul(z, vectorOf(kUp))), fp::mul(vectorOf(kBack), back));
    const Float vy = fp::add(fp::add(fp::add(fp::mul(y, vectorOf(kAlong + 4)), fp::mul(z, vectorOf(kUp + 4))),
                                     fp::mul(vectorOf(kBack + 4), back)),
                             x);
    const Float vz = fp::add(fp::add(fp::mul(y, vectorOf(kAlong + 8)), fp::mul(z, vectorOf(kUp + 8))), fp::mul(back, vectorOf(kBack + 8)));
    guest.setFloat(particle + kParticleVelocity, float(vx));
    guest.setFloat(particle + kParticleVelocity + 8, float(vz));
    guest.setFloat(particle + kParticleVelocity + 4, float(vy));

    const float sizeChance = float(fp::mul(floatFrom(guest, shape + kKindSize), floatFrom(guest, shape + kKindSizeJitter)));
    const float sizeAdded = float(fp::mul(randomFraction(cpu, guest), Float(sizeChance)));
    const Float size = fp::add(floatFrom(guest, shape + kKindSize), Float(sizeAdded));
    guest.setFloat(particle + kParticleSize, float(size));
    guest.setFloat(particle + kParticleSizeAtStart, float(size));
    guest.setFloat(particle + kParticleGrowth, float(fp::mul(floatFrom(guest, shape + kKindGrowth), Float(scale))));
    guest.setFloat(particle + kParticleAngle, float(randomFraction(cpu, guest)));
    guest.setFloat(particle + kParticleSpin, float(fp::mul(floatFrom(guest, shape + kKindSpin), Float(scale))));
    guest.setFloat(particle + kParticleCosine,
                   float(guest::direct<0x4ea950, simd::cosineValue>(app, cpu, guest.dword(particle + kParticleAngle))));
    guest.setFloat(particle + kParticleSine,
                   float(guest::direct<0x4ea930, simd::sineValue>(app, cpu, guest.dword(particle + kParticleAngle))));
    guest.setFloat(particle + kParticleChance, float(randomFraction(cpu, guest)));
    guest.setDword(particle + kParticleSwirl, guest.dword(shape + kKindSwirl));
    guest.setFloat(particle + kParticleSwirlRate, float(floatFrom(guest, shape + kKindSwirlRate)));
    guest.setByte(particle + kParticleSpawns, 0);
    guest.setDword(particle + kParticleTexture, kParticleTextures + kind * kKindTextureBytes);
    guest.setFloat(particle + kParticleSwirlFade, float(floatFrom(guest, shape + kKindSwirlFade)));
    guest.setDword(particle + kParticleColour, guest.dword(shape + kKindColour));

    const reg32 anyJitter = guest.byte(shape + kKindColourJitter + 3) | guest.byte(shape + kKindColourJitter + 2)
        | guest.byte(shape + kKindColourJitter + 1) | guest.byte(shape + kKindColourJitter);
    if (anyJitter != 0)
    {
        for (const reg32 channel : { 3u, 2u, 1u, 0u })
        {
            const float most = float(Float(sreg32(guest.byte(shape + kKindColourJitter + channel))));
            const sreg32 less = fp::toInteger<sreg32>(fp::mul(randomFraction(cpu, guest), Float(most)));
            const reg32 at = particle + kParticleColour + channel;
            guest.setByte(at, std::uint8_t(guest.byte(at) - std::uint8_t(reg32(less))));
        }
    }

    for (reg32 i = 0; i < 4; ++i)
        guest.setByte(particle + kParticleFade + i, 0);
    guest.setByte(particle + kParticleFlags, guest.byte(shape + kKindFlags));
    if (isZero(guest.dword(particle + kParticleVelocity)) && isZero(guest.dword(particle + kParticleVelocity + 4))
        && isZero(guest.dword(particle + kParticleVelocity + 8)))
        guest.setDword(particle + kParticleLife, 0);
    return particle;
}

/* sub_4cab60: a dying particle, whole on the stack, handing on its children (+0x7e of them),
 * if it moves: of its kind, each where it is, at a scale by chance, along its velocity and
 * with half a life, none of their own.  What is left in eax is the count's byte, or what
 * came in. */
x86::reg32 frontend::sime::particleChildren(App* app, CPU& cpu, reg32 parent)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x18);
    constexpr reg32 kNoVector = 0x4ca490;
    constexpr reg32 kShare = 0x5403b0, kLeast = 0x5403b8;  // doubles
    const reg32 velocity = parent + kParticleVelocity;
    const reg32 none = guest.frame(), unit = guest.frame() + 0xc;
    copyDwords(guest, none, kNoVector, 3);
    if (isZero(guest.dword(velocity)) && isZero(guest.dword(velocity + 4)) && isZero(guest.dword(velocity + 8)))
        return 0;
    guest::direct<0x497500, quickNormal>(app, cpu, velocity, unit);
    for (reg32 i = 0;; ++i)
    {
        const reg32 count = guest.byte(parent + kParticleSpawns);
        if (sreg32(i) >= sreg32(count))
            return count;
        const float scale = float(fp::add(fp::mul(randomFraction(cpu, guest), doubleAt(guest, kShare)), doubleAt(guest, kLeast)));
        const Spawn spawn = { { guest.dword(parent + kParticlePosition), guest.dword(parent + kParticlePosition + 4),
                                guest.dword(parent + kParticlePosition + 8) },
                              bitsOf(scale),
                              { guest.dword(unit), guest.dword(unit + 4), guest.dword(unit + 8) },
                              { guest.dword(none), guest.dword(none + 4), guest.dword(none + 8) },
                              { guest.dword(velocity), guest.dword(velocity + 4), guest.dword(velocity + 8) } };
        const reg32 child = spawnParticleOf(cpu, guest, guest.dword(parent + kParticleKind), spawn);
        if (child != 0)
        {
            guest.setByte(child + kParticleSpawns, 0);
            guest.setDword(child + kParticleLife, reg32(sreg32(guest.dword(child + kParticleLife)) >> 1));
        }
    }
}

/* sub_4cac20: sparks off car eax where the stack says: fewer the slower the hit under 20, away
 * from the car, each with a life cut by chance and up to two children.  What is left in eax
 * is the original's. */
x86::reg32 frontend::sime::sparks(App* app, CPU& cpu, reg32 car, reg32 on)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x54);
    constexpr reg32 kPosition = 0, kSpeed = 0xc;
    constexpr reg32 kFullAt = 0x5403c0;            // float, 20
    constexpr reg32 kSpread = 0x5403c8, kCentre = 0x5403d0;       // doubles, 6 and -3
    constexpr reg32 kScaleShare = 0x5403d8, kScaleLeast = 0x5403e0;  // doubles, 0.25 and 0.75
    const reg32 top = guest.frame() + 0x54;
    const reg32 where = top - 0x24, away = top - 0x18;

    sreg32 count = sreg32(guest.dword(kindAt(kKindSparks) + kKindCount));
    const float speed = guest.floatAt(on + kSpeed);
    if (jumpsBelow(fp::compare(Float(speed), floatFrom(guest, kFullAt))))
    {
        const float scaled = float(fp::mul(Float(count), Float(speed)));
        const sreg32 full = fp::toInteger<sreg32>(floatFrom(guest, kFullAt));
        count = fp::toInteger<sreg32>(fp::div(Float(scaled), Float(full)));
    }
    const float moving[3] = { guest.floatAt(car + kCarVelocity), guest.floatAt(car + kCarVelocity + 4), guest.floatAt(car + kCarVelocity + 8) };
    copyDwords(guest, where, on + kPosition, 3);
    copyDwords(guest, away, car + kCarVelocity, 3);
    fe::direct<0x4972f0, vertices::normaliseVector>(app, cpu, away);
    const reg32 unit[3] = { guest.dword(away), guest.dword(away + 4), guest.dword(away + 8) };
    fe::direct<0x4e0000, vertices::offsetVectorList<false>>(app, cpu, 1, where, car + kCarPosition, away);
    if (!isZero(guest.dword(away)) || !isZero(guest.dword(away + 4)) || !isZero(guest.dword(away + 8)))
        fe::direct<0x4972f0, vertices::normaliseVector>(app, cpu, away);
    const reg32 awayNow[3] = { guest.dword(away), guest.dword(away + 4), guest.dword(away + 8) };
    reg32 result = awayNow[2];
    for (sreg32 i = 0; i < count; ++i)
    {
        reg32 back[3];
        for (reg32 axis = 0; axis < 3; ++axis)
        {
            const Float spread = fp::mul(randomFraction(cpu, guest), doubleAt(guest, kSpread));
            back[axis] = bitsOf(float(fp::add(fp::add(spread, Float(moving[axis])), doubleAt(guest, kCentre))));
        }
        const float scale = float(fp::add(fp::mul(randomFraction(cpu, guest), doubleAt(guest, kScaleShare)), doubleAt(guest, kScaleLeast)));
        const Spawn spawn = { { guest.dword(where), guest.dword(where + 4), guest.dword(where + 8) },
                              bitsOf(scale),
                              { unit[0], unit[1], unit[2] },
                              { awayNow[0], awayNow[1], awayNow[2] },
                              { back[0], back[1], back[2] } };
        const reg32 spark = spawnParticleOf(cpu, guest, kKindSparks, spawn);
        result = spark;
        if (spark == 0)
            continue;
        const reg32 cut = reg32(sreg32(guest.dword(spark + kParticleLife)) >> 1);
        const reg32 byChance = fe::direct<kRandom, port::nextRandom>(guest.app(), guest.cpu()) % cut;
        guest.setDword(spark + kParticleLife, guest.dword(spark + kParticleLife) - byChance);
        const reg32 children = fe::direct<kRandom, port::nextRandom>(guest.app(), guest.cpu());
        guest.setByte(spark + kParticleSpawns, std::uint8_t(children % 3));
        result = children / 3;
    }
    return result;
}

/* sub_4cae00: a puff of a kind (eax) where the stack says, standing still, spinning one way or
 * the other by chance.  None for kind 0, nor for the drip on the software drivers: what is
 * left in eax is 1 or -1 then, else the particle. */
x86::reg32 frontend::sime::crashPuff(App* app, CPU& cpu, reg32 kind, reg32 on)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 16 + 0x34);
    const sreg32 way = (fe::direct<kRandom, port::nextRandom>(guest.app(), guest.cpu()) & 1) != 0 ? 1 : -1;
    if (kind == kKindNone || (kind == kKindDrip && (guest.byte(kDriverFlags) & kSoftwareDrivers) != 0))
        return reg32(way);
    const Spawn spawn = { { guest.dword(on), guest.dword(on + 4), guest.dword(on + 8) }, 0x3f800000, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
    const reg32 particle = spawnParticleOf(cpu, guest, kind, spawn);
    if (particle != 0)
        guest.setFloat(particle + kParticleSpin, float(fp::mul(Float(way), floatFrom(guest, particle + kParticleSpin))));
    return particle;
}

/* sub_4caea0: the sparkle round car eax as it is put back, where the stack says: kinds 9 and 7
 * every time, in eight ticks of 48 half the time the sparks, and seven times in eight kind 8,
 * at a scale by where in the 48 it is, going half the car's way.  What is left in eax is the
 * last call's. */
x86::reg32 frontend::sime::respawnSparkle(App* app, CPU& cpu, reg32 car, reg32 on)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x44);
    constexpr reg32 kKindGlow = 9, kKindRing = 7, kKindFlash = 8;
    constexpr sreg32 kPeriod = 0x30, kSparksFor = 8;
    constexpr reg32 kSparksSpeed = 0x41a00000;     // 20
    constexpr reg32 kShare = 0x5403f0;             // float, 0.5
    constexpr reg32 kPerTick = 0x5403e8;           // a double, 1/48
    const reg32 x = guest.dword(on), y = guest.dword(on + 4), z = guest.dword(on + 8);
    const float moving[3] = { guest.floatAt(car + kCarVelocity), guest.floatAt(car + kCarVelocity + 4), guest.floatAt(car + kCarVelocity + 8) };
    const Spawn still = { { x, y, z }, 0x3f800000, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
    spawnParticleOf(cpu, guest, kKindGlow, still);
    spawnParticleOf(cpu, guest, kKindRing, still);
    if (sreg32(guest.dword(kClock)) % kPeriod < kSparksFor && (fe::direct<kRandom, port::nextRandom>(guest.app(), guest.cpu()) & 1) != 0)
    {
        const PushedWords pushed(guest, cpu, { x, y, z, kSparksSpeed });
        guest::direct<0x4cac20, sparks>(app, cpu, car, pushed.address());
    }
    const reg32 chance = fe::direct<kRandom, port::nextRandom>(guest.app(), guest.cpu());
    if ((chance & 7) == 0)
        return chance;

    const sreg32 tick = sreg32(guest.dword(kClock)) % kPeriod;
    const Float share = floatFrom(guest, kShare);
    const reg32 v0 = bitsOf(float(fp::mul(Float(moving[0]), share)));
    const reg32 v1 = bitsOf(float(fp::mul(Float(moving[1]), share)));
    const reg32 v2 = bitsOf(float(fp::mul(share, Float(moving[2]))));
    const reg32 scale = bitsOf(float(fp::mul(Float(tick), doubleAt(guest, kPerTick))));
    const Spawn flash = { { x, y, z }, scale, { 0, 0, 0 }, { 0, 0, 0 }, { v0, v1, v2 } };
    const reg32 particle = spawnParticleOf(cpu, guest, kKindFlash, flash);
    if (particle != 0)
        guest.setDword(particle + kParticleChance, 0);
    return particle;
}

/* sub_4cb010: a wheel's dust of a kind off car eax, wheel edx, at the stack's point in 16.16:
 * the kind's count of them, scaled by the car's speed against the kind's where it has one,
 * going half the car's way, and for wheels 0 and 3 across the other way, turning the other
 * way.  What is left in eax is the count with no kind, or the original's; in edx the kind. */
x86::reg32 frontend::sime::wheelParticles(App* app, CPU& cpu, reg32 car, reg32 wheel, reg32 on)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 20 + 0x68);
    constexpr reg32 kPoint = 0, kKind = 0xc;
    constexpr reg32 kBackWheelsExtra = 0x5403f8;   // a double
    constexpr reg32 kFromFixed = 0x540400;         // a double
    constexpr reg32 kShare = 0x540408;             // float, 0.5
    constexpr reg32 kOne = 0x3f800000, kMinusOne = 0xbf800000;
    const reg32 kind = guest.dword(on + kKind);
    const reg32 shape = kindAt(kind);
    const reg32 count = guest.dword(shape + kKindCount);
    if (kind == kKindNone || (kind == kKindDrip && (guest.byte(kDriverFlags) & kSoftwareDrivers) != 0))
        return count;

    reg32 scale = kOne;
    if (!isZero(guest.dword(shape + kKindFullSpeed)))
    {
        const float speed = guest.floatAt(car + kCarSpeed);
        const float unsignedSpeed = jumpsBelow(fp::compare(0.0, Float(speed))) ? speed : float(-Float(speed));
        float share = float(fp::div(Float(unsignedSpeed), floatFrom(guest, shape + kKindFullSpeed)));
        if (kind != kKindNoExtra && guest.dword(car + kCarBackWheelsExtra) != 0 && sreg32(wheel) > 1)
            share = float(fp::add(Float(share), doubleAt(guest, kBackWheelsExtra)));
        const Float wide = Float(share);
        scale = bitsOf(float(jumpsAbove(fp::compare(1.0, wide)) ? wide : 1.0));
    }

    const Float fromFixed = doubleAt(guest, kFromFixed);
    const float x = float(fp::mul(Float(sreg32(guest.dword(on + kPoint))), fromFixed));
    const float y = float(fp::mul(Float(sreg32(guest.dword(on + kPoint + 4))), fromFixed));
    const float z = float(fp::mul(fromFixed, Float(sreg32(guest.dword(on + kPoint + 8)))));
    const Float half = floatFrom(guest, kShare);
    const float backX = float(fp::mul(floatFrom(guest, car + kCarVelocity), half));
    const float backY = float(fp::mul(floatFrom(guest, car + kCarVelocity + 4), half));
    const float backZ = float(fp::mul(half, floatFrom(guest, car + kCarVelocity + 8)));
    const reg32 forward = guest.frame() + 0x68 - 0x38;
    copyDwords(guest, forward, car + kCarVelocity, 3);
    fe::direct<0x4972f0, vertices::normaliseVector>(app, cpu, forward);
    const reg32 unit[3] = { guest.dword(forward), guest.dword(forward + 4), guest.dword(forward + 8) };

    reg32 across[3];
    reg32 turn;
    reg32 result;
    if (wheel == 0 || wheel == 3)
    {
        for (reg32 axis = 0; axis < 3; ++axis)
            across[axis] = bitsOf(float(-floatFrom(guest, car + kCarMatrix + 4 * axis)));
        turn = kMinusOne;
        result = kMinusOne;
    }
    else
    {
        for (reg32 axis = 0; axis < 3; ++axis)
            across[axis] = guest.dword(car + kCarMatrix + 4 * axis);
        turn = kOne;
        result = across[2];
    }

    const Spawn spawn = { { bitsOf(x), bitsOf(y), bitsOf(z) },
                          scale,
                          { unit[0], unit[1], unit[2] },
                          { across[0], across[1], across[2] },
                          { bitsOf(backX), bitsOf(backY), bitsOf(backZ) } };
    for (sreg32 i = 0; i < sreg32(count); ++i)
    {
        result = spawnParticleOf(cpu, guest, kind, spawn);
        if (result != 0)
            guest.setFloat(result + kParticleSpin, float(fp::mul(floatFrom(guest, result + kParticleSpin), Float(floatOf(turn)))));
    }
    return result;
}

namespace
{

// ---------------------------------------------------------------------------
// Drops on the view
// ---------------------------------------------------------------------------

}  // namespace

/* sub_4ca500: what lands on player eax's view, of a kind (the stack's last), at the stack's
 * first two over its third: the kind's count of drops, each somewhere by chance within the
 * kind's jitter, unless it overlaps one already there (the drip, kind 10, instead runs kind
 * 11's off).  Only with effects full.  0 if nothing can land or there is no room, else 1. */
x86::reg32 frontend::sime::dropsOnView(App* app, CPU& cpu, reg32 player, reg32 on)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 24 + 0x58);
    constexpr reg32 kX = 0, kY = 4, kDistance = 8, kKind = 0xc;
    constexpr std::uint64_t kNearNothing = 0x3ef000100010000full;  // for a distance of 0
    constexpr reg32 kTwo = 0x540398, kHalf = 0x5403a0;             // doubles
    const reg32 kind = guest.dword(on + kKind);
    const reg32 shape = kindAt(kind);
    const sreg32 count = sreg32(guest.dword(shape + kKindDrops));
    if (sreg32(gameSettings(guest).effectsLevel) >= 1)
        return 0;
    if (count == 0 || guest.dword(shape + kKindDropLife) == 0 || guest.dword(kViewCovered + player * 4) != 0)
        return 0;
    if (!isZero(guest.dword(shape + kKindDropChance))
        && jumpsAbove(fp::compare(randomFraction(cpu, guest), floatFrom(guest, shape + kKindDropChance))))
        return 1;

    const reg32 drops = kDrops + player * kDropsPerPlayer;
    for (sreg32 n = 0; n < count; ++n)
    {
        reg32 slot = 0;
        for (reg32 i = 0; i < kDropCount; ++i)
        {
            if (guest.dword(drops + (i << 6) + kDropLife) == 0)
            {
                slot = drops + (i << 6);
                break;
            }
        }
        if (slot == 0)
            return 0;

        Float distance;
        if (!isZero(guest.dword(on + kDistance)))
            distance = floatFrom(guest, on + kDistance);
        else
            std::memcpy(&distance, &kNearNothing, sizeof distance);
        const Float perDistance = fp::div(1.0, distance);
        float x = float(fp::mul(floatFrom(guest, on + kX), Float(perDistance)));
        float y = float(fp::mul(perDistance, floatFrom(guest, on + kY)));
        if (!isZero(guest.dword(shape + kKindDropJitter)))
        {
            const float jitter = guest.floatAt(shape + kKindDropJitter);
            const Float wide = Float(jitter);
            const Float aside = fp::mul(fp::mul(randomFraction(cpu, guest), Float(jitter)), doubleAt(guest, kTwo));
            x = float(fp::add(fp::sub(aside, wide), Float(x)));
            const Float along = fp::mul(fp::mul(randomFraction(cpu, guest), Float(jitter)), doubleAt(guest, kTwo));
            y = float(fp::add(fp::sub(along, wide), Float(y)));
        }

        bool overlapped = false;
        for (reg32 i = 0; i < kDropCount; ++i)
        {
            const reg32 drop = drops + (i << 6);
            if (guest.dword(drop + kDropLife) == 0)
                continue;
            const float reach = float(fp::mul(floatFrom(guest, drop + kDropSize), doubleAt(guest, kHalf)));
            const float apartX = float(fp::sub(Float(x), floatFrom(guest, drop + kDropX)));
            if (!jumpsBelow(fp::compare(Float(absolute(apartX)), Float(reach))))
                continue;
            const float apartY = float(fp::sub(Float(y), floatFrom(guest, drop + kDropY)));
            if (!jumpsBelow(fp::compare(Float(absolute(apartY)), Float(reach))))
                continue;
            if (kind != kKindDrip || guest.dword(drop + kDropKind) != kKindRunOff)
            {
                overlapped = true;
                break;
            }
            guest.setDword(drop + kDropLife, reg32(sreg32(guest.dword(drop + kDropLife)) >> 1));
        }
        if (overlapped)
            continue;

        const reg32 cut = reg32(sreg32(guest.dword(shape + kKindDropLife)) >> 1);
        const reg32 byChance = fe::direct<kRandom, port::nextRandom>(guest.app(), guest.cpu()) % cut;
        guest.setDword(slot + kDropKind, kind);
        guest.setDword(slot + kDropLife, guest.dword(shape + kKindDropLife) - byChance);

        const Float size = floatFrom(guest, shape + kKindDropSize);
        const Float shrink = fp::mul(randomFraction(cpu, guest), fp::mul(Float(size), doubleAt(guest, kHalf)));
        const float sizeNow = float(fp::sub(size, shrink));
        const float angle = float(randomFraction(cpu, guest));
        const float sine =
            float(fp::mul(guest::direct<0x4ea930, simd::sineValue>(app, cpu, bitsOf(angle)), Float(sizeNow)));
        const Float cosine =
            fp::mul(guest::direct<0x4ea950, simd::cosineValue>(app, cpu, bitsOf(angle)), Float(sizeNow));

        guest.setFloat(slot + kDropX, x);
        guest.setFloat(slot + kDropY, y);
        guest.setFloat(slot + kDropSize, sizeNow);
        guest.setFloat(slot + kDropCorners, float(fp::sub(Float(x), Float(sine))));
        guest.setFloat(slot + kDropCorners + 0x04, float(fp::add(Float(y), cosine)));
        guest.setFloat(slot + kDropCorners + 0x08, float(fp::add(Float(x), cosine)));
        guest.setFloat(slot + kDropCorners + 0x0c, float(fp::add(Float(y), Float(sine))));
        guest.setFloat(slot + kDropCorners + 0x10, float(fp::add(Float(x), Float(sine))));
        guest.setFloat(slot + kDropCorners + 0x14, float(fp::sub(Float(y), cosine)));
        guest.setFloat(slot + kDropCorners + 0x18, float(fp::sub(Float(x), cosine)));
        guest.setFloat(slot + kDropCorners + 0x1c, float(fp::sub(Float(y), Float(sine))));
        guest.setDword(slot + kDropColour, guest.dword(shape + kKindDropColour));
        guest.setDword(slot + kDropFlags, guest.dword(shape + kKindDropFlags));
        guest.setDword(slot + kDropTexture, kDropTextures + kind * kKindTextureBytes);
        if (kind == kKindWithJolt)
            fx::callAt(guest, kDropEffect, args({ player }));
    }
    return 1;
}

namespace
{

constexpr std::uint8_t kKeepsNothing = 0;

}  // namespace

// ---------------------------------------------------------------------------
// As the game's functions stood: their frames, what they restore
// ---------------------------------------------------------------------------

x86::reg32 frontend::carsRemoveNumberedFolders(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_43bcf0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEsi | fe::kRestoreEdi };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::removeNumberedFolders(app, cpu); });
}

x86::reg32 frontend::effectsFogAtCamera(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_478e10", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 view = args.eax, colourOut = args.edx, distanceOut = args.ebx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::fogAtCamera(app, cpu, view, colourOut, distanceOut); });
}

x86::reg32 frontend::mathMakeRootTable(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_496310", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::makeRootTable(app, cpu); });
}

x86::reg32 frontend::mathQuickNormal(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_497500", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 vector = args.eax, out = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::quickNormal(app, cpu, vector, out); });
}

/* sub_497500: eax's vector scaled to length 1, to edx; eax the vector. */
x86::reg32 frontend::sime::quickNormal(App* app, CPU& cpu, reg32 vector, reg32 out)
{
    Guest guest(app, cpu);
    quickNormalInto(guest, vector, out);
    return vector;
}

x86::reg32 frontend::raceEffectsReset(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bc8b0", 0, 0, nullptr, 0, kKeepsNothing };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::effectsReset(app, cpu); });
}

x86::reg32 frontend::raceEffectsFree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bc950", 0, 0, nullptr, 0, kKeepsNothing };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::effectsFree(app, cpu); });
}

x86::reg32 frontend::raceViewsResize(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4be970", 0, 0, nullptr, 0 };
    const reg32 direction = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::viewsResize(app, cpu, direction); });
}

x86::reg32 frontend::raceViewResize(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4bea40", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 view = args.eax, change = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::viewResize(app, cpu, view, sreg32(change)); });
}

x86::reg32 frontend::effectsSkidSegmentPut(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c6c70", 8, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 corners = args.eax, alphas = args.edx, from = args.ebx, to = args.ecx;
    const reg32 alphaFrom = guest::at(app, cpu.esp + 4), alphaTo = guest::at(app, cpu.esp + 8);
    return fe::runThunk(app, cpu, args, site,
                 [&](Guest&) { return sime::skidSegmentPut(app, cpu, corners, alphas, from, to, alphaFrom, alphaTo); });
}

x86::reg32 frontend::effectsSkidStripsTake(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c70f0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::skidStripsTake(app, cpu); });
}

x86::reg32 frontend::effectsSkidStripsReset(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c7130", 0, 0, nullptr, 0, kKeepsNothing };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::skidStripsReset(app, cpu); });
}

x86::reg32 frontend::effectsSkidStripsFree(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4c7150", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::skidStripsFree(app, cpu); });
}

/* sub_4c7b20: a race's end has nothing for it to do. */
x86::reg32 frontend::sime::effectsEndEmpty(App*, CPU&, reg32 eax)
{
    return eax;
}

x86::reg32 frontend::raceEffectsEndEmpty(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = sime::effectsEndEmpty(app, cpu, args.eax);
    cpu.esp += 4;
    return args.eax;
}

x86::reg32 frontend::raceStripUnderWheels(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4ca160", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = args.eax, out = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::stripUnderWheels(app, cpu, car, out); });
}

x86::reg32 frontend::effectsParticleSlot(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4ca4b0", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::particleSlot(app, cpu); });
}

x86::reg32 frontend::effectsDropsOnView(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4ca500", 16, 0, nullptr, 0 };
    const reg32 player = args.eax;
    const reg32 on = cpu.esp + 4;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::dropsOnView(app, cpu, player, on); });
}

x86::reg32 frontend::effectsParticleSpawn(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4ca850", 0x34, 0, nullptr, 0 };
    const reg32 kind = args.eax;
    const reg32 on = cpu.esp + 4;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::spawnParticle(app, cpu, kind, on); });
}

x86::reg32 frontend::effectsParticleChildren(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4cab60", 0x80, 0, nullptr, 0 };
    const reg32 on = cpu.esp + 4;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::particleChildren(app, cpu, on); });
}

x86::reg32 frontend::effectsSparks(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4cac20", 0x10, 0, nullptr, 0 };
    const reg32 car = args.eax;
    const reg32 on = cpu.esp + 4;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::sparks(app, cpu, car, on); });
}

x86::reg32 frontend::effectsCrashPuff(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4cae00", 0xc, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 kind = args.eax;
    const reg32 on = cpu.esp + 4;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::crashPuff(app, cpu, kind, on); });
}

x86::reg32 frontend::effectsRespawnSparkle(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4caea0", 0xc, 0, nullptr, 0 };
    const reg32 car = args.eax;
    const reg32 on = cpu.esp + 4;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::respawnSparkle(app, cpu, car, on); });
}

x86::reg32 frontend::effectsWheelParticles(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4cb010", 0x10, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 car = args.eax, wheel = args.edx;
    const reg32 on = cpu.esp + 4;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::wheelParticles(app, cpu, car, wheel, on); });
}

x86::reg32 frontend::raceEffectsEndNothing(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4d5730", 0, 0, nullptr, 0, kKeepsNothing };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::effectsEndNothing(app, cpu); });
}

/* sub_4d5730: a race's end (sub_4d56b0), every register handed on as it came. */
x86::reg32 frontend::sime::effectsEndNothing(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 4);
    return fe::direct<0x4d56b0, rende::nothing>(app, cpu);
}

x86::reg32 frontend::effectsWheelMarkCut(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4db860", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx };
    const reg32 car = args.eax, wheel = args.edx;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::wheelMarkCut(app, cpu, car, wheel); });
}

x86::reg32 frontend::effectsWheelMarksResetAtLoad(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dbba0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::wheelMarksResetAtLoad(app, cpu); });
}

/* sub_4dbba0: the wheels' marks cleared as the race loads, the loading kept going. */
x86::reg32 frontend::sime::wheelMarksResetAtLoad(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kWheelMarks, kWheelMarksBytes);
    return fe::direct<0x4a33d0, loada::stopCheck>(app, cpu);
}

x86::reg32 frontend::effectsWheelMarksReset(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dbbc0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::wheelMarksReset(app, cpu); });
}

/* sub_4dbbc0: the wheels' marks cleared. */
x86::reg32 frontend::sime::wheelMarksReset(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame locals(guest, cpu, 8);
    return fe::direct<kFillZero, ea::clearBytes>(guest.app(), guest.cpu(), kWheelMarks, kWheelMarksBytes);
}

/* sub_4dbbe0: a race's end has nothing for the marks either. */
x86::reg32 frontend::sime::wheelMarksEndEmpty(App*, CPU&, reg32 eax)
{
    return eax;
}

x86::reg32 frontend::effectsWheelMarksEndEmpty(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = sime::wheelMarksEndEmpty(app, cpu, args.eax);
    cpu.esp += 4;
    return args.eax;
}

x86::reg32 frontend::raceLoadTrackPieces(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args)
{
    static fe::Site site{ "sub_4dbbf0", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 track = args.eax;
    return fe::runThunk(app, cpu, args, site, [&](Guest&) { return sime::loadTrackPieces(app, cpu, track); });
}

}
