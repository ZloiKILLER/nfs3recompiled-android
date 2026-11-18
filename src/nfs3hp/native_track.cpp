#include "game_settings.h"
#include "native_aiplan.h"
#include "native_carstate.h"
#include "native_collide.h"
#include "native_ea_memory.h"
#include "native_ea_support.h"
#include "native_ea_threads.h"
#include "native_frontend_loada.h"
#include "native_frontend_rendd.h"
#include "native_frontend_rende.h"
#include "native_frontend_simc.h"
#include "native_graphics.h"
#include "native_guest.h"
#include "native_load_track.h"
#include "native_track.h"
#include "native_trackdata.h"
#include "native_vertices.h"
#include "native_windows.h"
#include "port_snd.h"
#include "port_sound_effects.h"
#include "port_thrash.h"
#include "host_system.h"
#include "native_render.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <optional>
#include <utility>

/* The track: its geometry read from the .frd file into an arena of its own,
 * its colours tinted, its blocks and objects drawn in each view with the
 * headlights' light on them, its ambient sounds, the files that go with it
 * (the .col file, the textures' .qfs) and the views' bookkeeping.
 *
 * The track's blocks: 0x5c0 bytes each from 0x571370, [0x5dd83c] the last
 * one's number -- +0x58 and +0x74 its vertices' count and colours, +0x5c its
 * polygon groups, +0x70 its vertices, +0x78 its centre, +0x84 its box of four
 * corners, +0xb4 the 300 neighbours drawn with it (a word each, -1 none),
 * +0x564/+0x56c/+0x574/+0x57c its four lists of objects (a count, then 0x34
 * bytes each), +0x5b0 its sound sources, +0x5b8 its lights.  An object of a
 * list: its kind at +0 (4 animated, 3 a car's, else fixed), its vertices'
 * count at +0x20, its vertices +0x24, colours +0x28, polygons' count +0x2c
 * and polygons +0x30 (14 bytes each).  The list of the whole track's objects
 * is at 0x5dd070.
 *
 * A view: 0x54 bytes each from 0x5dd0e0, four of them, [0x5dd830] the one
 * being drawn -- +0 -1 as they start over, +4 the block it is in, +0x30 the
 * distance the world ends at,
 * +0x34 the distance blocks are drawn to, +0x40/+0x44 the two distances the
 * track's model gets coarser at; the camera (eax of the drawing) its kind at
 * +0 (1 the rear-view mirror's pass), its place at +8, its matrix at +0x44,
 * its view box at +0x38.
 *
 * The .frd is read by moving its bytes from where the file was loaded
 * ([0x5dd078], moved on as it goes) into blocks taken from the arena: 0x310000
 * bytes at [0x552dfc], [0x552e00] of them used.
 *
 * Each is a typed function under a name of its own, with a thunk under
 * the game's name for the callers still on registers and the calls through a pointer: it
 * leaves eax, the registers it was handed no argument in, the x87's stack and
 * its control word as the game's code does (a core hands back what the code
 * leaves where a caller can see it).  The other typed files' functions are
 * called as C++ (guest::direct,
 * their headers), what the code reads in a register after one made from
 * what it gives back or from what the code hands it there, which the call
 * keeps -- by address where the code reads what it leaves that it does not
 * give back.  Every other function of the game's, and Windows' through its
 * imports, is called by address, handed its argument registers (callWith
 * keeps the others across a callee that keeps them), its frames on the
 * guest's stack as deep as the code's where a callee is
 * handed their places; the x87's operations are fp's, in the game's order, a
 * float kept where the game stores one; the safepoint at each loop's head
 * as in the game's code.  A core calls this file's cores as C++, the
 * return address's slot (and the stack's words) emulated where its frame is
 * handed on.
 *
 * The port's changes these functions carry: the alpha intensity's scaling on
 * this driver too (the port's alpha intensity change, projectedTriangle) and the
 * rear-view mirror drawn as the main view (the port's mirror detail change,
 * modelByDistance, headlightsOnPoints, drawTrackBlocks), through
 * nfs3hp::mirrorAsMain as the game's code had them. */
namespace nfs3hp
{

x86::reg32 mirrorAsMain(x86::reg32 kind);  // nfs3hp_main.cpp (the port's mirror detail change)

namespace track
{

namespace
{

using guest::at;
using guest::byte;
using guest::put;
using guest::putByte;
using x86::reg16;
using x86::reg32;
using x86::reg8;
using x86::sreg32;
using App = win32::WinApplication;
using CPU = x86::CPU;
using Float = x86::Float;
using i32 = std::int32_t;

// The track's blocks, the views, the objects' list, the view being drawn.
constexpr reg32 kBlocks = 0x571370, kBlockSize = 0x5c0, kLastBlock = 0x5dd83c;
constexpr reg32 kViews = 0x5dd0e0, kView = 0x5dd830, kTrackObjects = 0x5dd070;
constexpr reg32 kViewSerial = 0x5dd0d8, kViewSerialHigh = 0x5dd0dc;
// A view's size and its block (+4); the pass being drawn, 1 the rear-view mirror's.
constexpr reg32 kViewSize = 0x54, kViewBlock = 4, kPassKind = 0x7a3d14, kMirrorPass = 1;
// A view's +0: -1 as the views start over; an object's state still (its first byte).
constexpr reg32 kViewStart = 0;
constexpr reg8 kStateStill = 0xff;
// The .frd's reading: where the file is read from, the arena and its use.
constexpr reg32 kFrdAt = 0x5dd078, kArena = 0x552dfc, kArenaUsed = 0x552e00, kArenaSize = 0x310000;
// The lights and sound sources of the .frd, their count; the kinds' records.
constexpr reg32 kLights = 0x552df4, kLightCount = 0x552df8, kLightKinds = 0x8b41a0, kTrackKinds = 0x5dd858;
// The colours' tint (percent of red, green, blue) and the driver's bits
// (1 << the driver's type: 2 a grey track, 0x40 Direct3D).
constexpr reg32 kTint = 0x55d148, kDriverBits = 0x7a3a58;
// The shading's distances: near, far, the light's reach, 1 / (far - near).
constexpr reg32 kShadeNear = 0x552e04, kShadeFar = 0x552e08, kShadeReach = 0x552e0c, kShadeScale = 0x552e10;
// Alpha intensity (0..1) and the headlights' polygon's texture.
constexpr reg32 kHeadlightTexture = 0x8b4814;
// The race: split screen, the network's, the track's number; the loading stopped.
constexpr reg32 kLoadStopped = 0x55e88c;
// The cars (the headlights): their count and pointers.
constexpr reg32 kCarCount = 0x5efd9c, kCars = 0x5efac8;
// The objects' animation states, 3 bytes each.
constexpr reg32 kObjectStates = 0x5dd230, kObjectStateSize = 3;
// The models by distance: 3 records of 12 bytes.
constexpr reg32 kModels = 0x552e20;
// The .col file, its records, the track's textures and their handle.
constexpr reg32 kColFile = 0x552e18, kColVertices = 0x552e14, kColObjects = 0x5dd844, kColLights = 0x5dd840;
constexpr reg32 kTexturesFile = 0x5dd850, kTexturesCount = 0x5dd854, kTexturesLeft = 0x5dd85c, kTexturesAll = 0x5dd860;
constexpr reg32 kObjectPool = 0x5dd848;
// The ambient sound played last, and the pass's half-second counter.
constexpr reg32 kAmbientLast = 0x552e44, kFrameCounter = 0x5f1fa8;
// The views' buckets of polygons.
constexpr reg32 kBucketOpaque = 0x7dcfe8, kBucketA = 0x7dcff4, kBucketB = 0x7dcff0, kBucketC = 0x7dcfe4;
constexpr reg32 kBucketOrdered = 0x7dcff8, kWorldsDrawn = 0x7a3a60;
// Strings and constants of the game's.
constexpr reg32 kOpen = 0x536c30, kNoName = 0x536c60, kNoName2 = 0x536d10;
constexpr reg32 kHeadlightsBox = 0x41a880, kHeadlightsPoint = 0x41a8a4;  // data among the code
// What is called by address.
constexpr reg32 kGetModuleHandleA = 0x53454c, kShellExecuteA = 0x534480;
constexpr reg32 kTopUp = 0x4a33d0;

/* A loop's safepoint, as the game's code has one at each loop's head: the
 * guest's context handed over when another thread (the sound's timer, its
 * mixer) has waited for it.  The only thing a core without callees takes the
 * CPU for. */
void safepoint(App* app, const CPU& cpu)
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

Float f32(App* app, reg32 address)
{
    return Float(app->getMemory<float>(address));
}

Float f64(App* app, reg32 address)
{
    return Float(app->getMemory<double>(address));
}

void putF32(App* app, reg32 address, const Float& value)
{
    app->getMemory<float>(address) = float(value);
}

/* The block of number `index`: kBlocks + index * 0x5c0, as the code works it
 * out (lea, sub, shl 3, sub, shl 6). */
reg32 blockOffset(reg32 index)
{
    return index * kBlockSize;
}

/* The vector at `vector` turned by the 3 by 3 matrix at `matrix` into `to`:
 * each row's products summed in the code's order (the turn every view takes
 * of a place, inline in the code). */
void turnVector(App* app, reg32 vector, reg32 matrix, reg32 to)
{
    const reg32 v = vector, m = matrix;
    const Float a = fp::mul(f32(app, v), f32(app, m));
    const Float b = fp::mul(f32(app, v), f32(app, m + 4));
    const Float c = fp::mul(f32(app, v), f32(app, m + 8));
    const Float d = fp::mul(f32(app, v + 4), f32(app, m + 0xc));
    const Float e = fp::mul(f32(app, v + 4), f32(app, m + 0x10));
    const Float f = fp::mul(f32(app, v + 4), f32(app, m + 0x14));
    const Float x = fp::add(a, d), y = fp::add(b, e), z = fp::add(c, f);
    const Float g = fp::mul(f32(app, v + 8), f32(app, m + 0x18));
    const Float h = fp::mul(f32(app, v + 8), f32(app, m + 0x1c));
    const Float i = fp::mul(f32(app, v + 8), f32(app, m + 0x20));
    const Float rx = fp::add(x, g), ry = fp::add(y, h), rz = fp::add(z, i);
    putF32(app, to, rx);
    putF32(app, to + 4, ry);
    putF32(app, to + 8, rz);
}

/* A float's bits, and the float of bits, as the code moves them through a
 * register. */
reg32 bitsOf(float value)
{
    reg32 bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

Float floatOf(reg32 bits)
{
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return Float(value);
}

/* Room on the guest's stack for what a callee is handed the address of: the
 * code's frame (`pushed` bytes of registers under esp on entry, then
 * `bytes`), esp under it while the core runs and back after. */
class GuestFrame
{
public:
    GuestFrame(CPU& cpu, reg32 pushed, reg32 bytes) : cpu_(cpu), esp_(cpu.esp), base_(cpu.esp - pushed)
    {
        cpu.esp = base_ - bytes;
    }
    ~GuestFrame() { cpu_.esp = esp_; }
    reg32 operator-(reg32 offset) const { return base_ - offset; }
    reg32 operator+(reg32 offset) const { return base_ + offset; }

private:
    CPU& cpu_;
    const reg32 esp_, base_;
};

/* cmp al, ah; ja; mov al, ah: the larger of two bytes. */
reg8 larger(reg8 a, reg8 b)
{
    return a > b ? a : b;
}

// The registers a function's code saves (pushes and pops); ebp each of them does.
constexpr reg32 kSaveEbx = 1, kSaveEcx = 2, kSaveEdx = 4, kSaveEsi = 8, kSaveEdi = 0x10;

/* The registers a function's code saves and restores, and ebp, as they were
 * on entry whatever its callees leave; the others left as the code and its
 * callees leave them. */
class Kept
{
public:
    Kept(x86::Args& entry, reg32 saved)
        : cpu_(entry), saved_(saved), ebx_(entry.ebx), ecx_(entry.ecx), edx_(entry.edx), esi_(entry.esi),
          edi_(entry.edi), ebp_(entry.ebp)
    {
    }
    ~Kept()
    {
        if (saved_ & kSaveEbx)
            cpu_.ebx = ebx_;
        if (saved_ & kSaveEcx)
            cpu_.ecx = ecx_;
        if (saved_ & kSaveEdx)
            cpu_.edx = edx_;
        if (saved_ & kSaveEsi)
            cpu_.esi = esi_;
        if (saved_ & kSaveEdi)
            cpu_.edi = edi_;
        cpu_.ebp = ebp_;
    }

private:
    x86::Args& cpu_;
    const reg32 saved_, ebx_, ecx_, edx_, esi_, edi_, ebp_;
};

/* A call by address (a callee with no typed core) as the code makes it:
 * `registers` into eax, edx, ebx and ecx in turn, `words` pushed (the first
 * the lowest) and taken off after it; every register but eax as it was before
 * after it.  eax back; nullopt when the game stopped. */
std::optional<reg32> callWith(App* app, CPU& cpu, reg32 target, std::initializer_list<reg32> registers,
                              std::initializer_list<reg32> words = {})
{
    return guest::pointerCall(app, cpu, target, registers, words);
}

/* Render_GetTm's room (0x4bbde0): `bytes` of the drawing's arena, their place to [place]; 1, 0 when it has
 * run out.  nullopt when the game stopped. */
std::optional<reg32> arenaRoom(App* app, CPU& cpu, reg32 place, reg32 bytes)
{
    const reg32 room = guest::direct<0x4bbde0, frontend::rendd::arenaTake>(app, cpu, place, bytes);
    if (cpu.terminate)
        return std::nullopt;
    return room;
}

/* Render_GetTm's next (0x4bbe40): where the arena is now, to [place]; 1, 0 when it has run out. */
std::optional<reg32> arenaNext(App* app, CPU& cpu, reg32 place)
{
    const reg32 room = guest::direct<0x4bbe40, frontend::rendd::arenaHere>(app, cpu, place);
    if (cpu.terminate)
        return std::nullopt;
    return room;
}

// Other files' functions the track's call by address (Watcom's, unless said).
constexpr reg32 kLoadFrd = 0x419c20;          // loader::frd: the .frd read into the track's blocks
constexpr reg32 kClearCameras = 0x40f230;     // load::clearTrackCameras

/* The sound's stream topped up (0x4a33d0), as the track's long loops do it.
 * False when the game stopped. */
bool topUp(App* app, CPU& cpu)
{
    guest::direct<0x4a33d0, frontend::loada::stopCheck>(app, cpu);
    return !cpu.terminate;
}

/* `bytes` bytes of the game's heap (0x4e1620), `name` the string it is
 * known by, no flags; their address (in eax too). */
std::optional<reg32> allocate(App* app, CPU& cpu, reg32 name, reg32 bytes)
{
    return guest::direct<0x4e1620, ea::reserved>(app, cpu, name, bytes, 0);
}

/* The heap's block at `block` given back (0x4e1890); what that gives back,
 * in eax too. */
std::optional<reg32> freeBlock(App* app, CPU& cpu, reg32 block)
{
    return guest::direct<0x4e1890, ea::purgeBlock>(app, cpu, block);
}

/* The game's memmove (0x4ea4f0): `bytes` from `from` to `to`.  By address:
 * the .frd's readers leave what it leaves in eax (ea::moveMemory gives back
 * only whether the game went on).  That eax; nullopt when the game
 * stopped. */
std::optional<reg32> memmove(App* app, CPU& cpu, reg32 to, reg32 from, reg32 bytes)
{
    if (!guest::direct<0x4ea4f0, ea::moveMemory>(app, cpu, to, from, bytes))
        return std::nullopt;
    // The eax the game's memmove leaves: `from` where the copy runs backwards, else past the bytes copied to.
    return to > from && to < from + bytes ? from : to + bytes;
}

}  // namespace

x86::reg32 shellOpen(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tintColours(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tintObjectList(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 tintTrack(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 greyColours(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 greyObjectList(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 greyTrack(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 readObjectList(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 readLights(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 newArena(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 freeArena(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 arenaTake(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 frdRead(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 loadGeometry(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 projectedTriangle(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 shadeVertex(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 setShadeDistances(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 viewObjectVertices(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 viewTurnedObjectVertices(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 modelByDistance(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 viewOnBlock(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 lightObjectPolygons(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 bracketList(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ambientSounds(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ambientWanted(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 pointBehindView(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 litBoxOutside(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 flushBuckets(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 headlightsOnObjects(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 headlightsOnBlock(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 blockLights(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 drawObjectList(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 drawTrack(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 ambientSource(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 selectView(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 objectAnimation(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 turnFromAngles(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 turnFromPitchYaw(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 setViewDistance(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 nextViewDistance(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 modelsUnlimited(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 freeAt(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 unloadTrack(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 newPool(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 newObjectPool(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 freeTrackKinds(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 loadTextures(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 takeTextures(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 loadCollisions(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 freeCollisions(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 clearObjectStates(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 resetTrackViews(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 resetViewsOuter(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 resetViews(App* app, x86::CPU& cpu, x86::Args& args);
x86::reg32 blockInView(App* app, x86::CPU& cpu, x86::Args& args);

// The typed cores a core calls before they are defined.
std::optional<reg32> readFrd(App* app, CPU& cpu, reg32 to, reg32 bytes);
std::optional<reg32> takeFromArena(App* app, reg32 bytes);
std::optional<reg32> freeTheArena(App* app, CPU& cpu, reg32 eax);
bool startViewsOver(App* app, CPU& cpu);
std::optional<reg32> freeCollisionFile(App* app, CPU& cpu, reg32 eax);
reg32 chooseView(App* app, reg32 index);
reg32 scaledColour(reg32 colour, reg32 factor);
std::optional<reg32> turnOfAngles(App* app, CPU& cpu, reg32 first, reg32 second, reg32 third, reg32 into);
std::optional<reg32> turnOfPitchYaw(App* app, CPU& cpu, reg32 pitch, reg32 yaw, reg32 into);
std::optional<bool> blockDrawn(App* app, CPU& cpu, reg32 camera, reg32 index);

/* 0x418cc0's: the file or address at `file` opened -- ShellExecuteA's
 * "open", shown normally (1), for this module (GetModuleHandleA(0)).  False
 * when the game stopped. */
bool openWithShell(App* app, CPU& cpu, reg32 file)
{
    constexpr reg32 kShowNormal = 1;
    const std::optional<reg32> module =
        guest::windows<kGetModuleHandleA, &host::moduleHandle>(app, cpu, 0);
    return module
           && guest::windows<kShellExecuteA, &host::shellExecute>(app, cpu, *module, kOpen, file, 0, 0,
                                                                             kShowNormal);
}

/* 0x418cc0: openWithShell of eax; 1. */
x86::reg32 shellOpen(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEcx | kSaveEdx);
    if (!openWithShell(app, cpu, args.eax))
        return args.eax;
    args.eax = 1;
    cpu.esp += 4;
    return args.eax;
}

// A colour's alpha (ARGB); the tint's percents of red, green and blue (kTint, +4, +8).
constexpr reg32 kAlpha = 0xff000000, kTintRed = 0, kTintGreen = 4, kTintBlue = 8;

/* A colour's channel `shift` bits up taken to `percent` per cent, as the code
 * works it out: the byte times the percent, divided by 100 signed. */
reg32 tintedChannel(reg32 colour, unsigned shift, reg32 percent)
{
    return reg32(i32(((colour >> shift) & 0xff) * percent) / 100);
}

/* The colours a run of `count` does, as the code leaves the count in eax. */
reg32 coloursDone(reg32 count)
{
    return sreg32(count) > 0 ? count : 0;
}

/* 0x419660's: the `count` colours at `colours` (ARGB) tinted -- red, green
 * and blue each to its percent of the tint, alpha kept. */
void tintColourRun(App* app, const CPU& cpu, reg32 count, reg32 colours)
{
    for (reg32 done = 0;; ++done)
    {
        safepoint(app, cpu);
        if (sreg32(done) >= sreg32(count))
            break;
        const reg32 place = colours + done * 4, colour = at(app, place);
        const reg32 red = tintedChannel(colour, 16, at(app, kTint + kTintRed));
        const reg32 green = tintedChannel(colour, 8, at(app, kTint + kTintGreen)) << 8;
        const reg32 blue = tintedChannel(colour, 0, at(app, kTint + kTintBlue));
        put(app, place, (red << 16) | (colour & kAlpha) | green | blue);
    }
}

/* 0x419660: tintColourRun of eax colours at edx; eax the colours done. */
x86::reg32 tintColours(App* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 count = args.eax;
    tintColourRun(app, cpu, count, args.edx);
    args.eax = coloursDone(count);
    cpu.esp += 4;
    return args.eax;
}

// An object list: its count, then its objects, 0x34 bytes each: +0x18 its extra data's size, +0x1c the
// data, +0x20 its vertices' count, +0x24 its vertices (12 bytes each), +0x28 their colours, +0x2c its
// polygons' count, +0x30 its polygons (14 bytes each).
constexpr reg32 kListCount = 0, kListObjects = 4, kObjectSize = 0x34, kObjectExtraSize = 0x18, kObjectExtra = 0x1c,
                kObjectVertexCount = 0x20, kObjectVertices = 0x24, kObjectColours = 0x28, kObjectPolygonCount = 0x2c,
                kObjectPolygons = 0x30, kVertexSize = 12, kColourSize = 4, kPolygonSize = 14;
// A block's own vertices' count (+0x58) and colours (+0x74); the lists of objects tinted with it.
constexpr reg32 kBlockVertexCount = 0x58, kBlockColours = 0x74;
constexpr reg32 kBlockLists[] = {0x564, 0x56c, 0x574};

/* 0x419720's: the colours of each object of the list at `list` tinted.  What
 * the code leaves in eax: the last object's colours done, or the list where
 * it has none. */
reg32 tintObjects(App* app, const CPU& cpu, reg32 list)
{
    reg32 left = list, object = at(app, list + kListObjects);
    for (reg32 remaining = at(app, list + kListCount);; object += kObjectSize)
    {
        safepoint(app, cpu);
        if (remaining-- == 0)
            break;
        const reg32 count = at(app, object + kObjectVertexCount);
        tintColourRun(app, cpu, count, at(app, object + kObjectColours));
        left = coloursDone(count);
    }
    return left;
}

/* 0x419720: tintObjects of eax. */
x86::reg32 tintObjectList(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = tintObjects(app, cpu, args.eax);
    cpu.esp += 4;
    return args.eax;
}

/* 0x419750's: the whole track's colours tinted: the track's objects, then
 * each block's three lists of objects and its own vertices.  What the code
 * leaves in eax: the last block's colours done (the track objects' where
 * there is no block). */
reg32 tintTrackColours(App* app, const CPU& cpu)
{
    reg32 left = tintObjects(app, cpu, kTrackObjects);
    for (reg32 index = 0;; ++index)
    {
        safepoint(app, cpu);
        if (sreg32(index) > sreg32(at(app, kLastBlock)))
            break;
        const reg32 block = kBlocks + blockOffset(index);
        for (const reg32 list : kBlockLists)
            tintObjects(app, cpu, block + list);
        const reg32 count = at(app, block + kBlockVertexCount);
        tintColourRun(app, cpu, count, at(app, block + kBlockColours));
        left = coloursDone(count);
    }
    return left;
}

/* 0x419750: tintTrackColours. */
x86::reg32 tintTrack(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = tintTrackColours(app, cpu);
    cpu.esp += 4;
    return args.eax;
}

/* 0x4197c0's: the `count` colours at `colours` made grey -- the mean of
 * red, green and blue in each, alpha opaque.  The last one, as the code
 * leaves it in eax; nullopt where there is none. */
std::optional<reg32> greyColourRun(App* app, const CPU& cpu, reg32 count, reg32 colours)
{
    std::optional<reg32> last;
    for (reg32 done = 0;; ++done)
    {
        safepoint(app, cpu);
        if (sreg32(done) >= sreg32(count))
            break;
        const reg32 place = colours + done * 4, colour = at(app, place);
        const reg32 mean = reg32(i32(((colour >> 16) & 0xff) + ((colour >> 8) & 0xff) + (colour & 0xff)) / 3);
        last = mean | (mean << 8) | (mean << 16) | kAlpha;
        put(app, place, *last);
    }
    return last;
}

/* What greyColours leaves in eax: the last colour, or the count where none. */
reg32 greyLeft(reg32 count, const std::optional<reg32>& last)
{
    return last ? *last : count;
}

/* 0x4197c0: greyColourRun of eax colours at edx; eax greyLeft. */
x86::reg32 greyColours(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = greyLeft(args.eax, greyColourRun(app, cpu, args.eax, args.edx));
    cpu.esp += 4;
    return args.eax;
}

/* 0x419840's: the colours of each object of the list at `list` made grey.
 * What the code leaves in eax: the last object's greyLeft, or the list where
 * it has none. */
reg32 greyObjects(App* app, const CPU& cpu, reg32 list)
{
    reg32 left = list, object = at(app, list + kListObjects);
    for (reg32 remaining = at(app, list + kListCount);; object += kObjectSize)
    {
        safepoint(app, cpu);
        if (remaining-- == 0)
            break;
        const reg32 count = at(app, object + kObjectVertexCount);
        left = greyLeft(count, greyColourRun(app, cpu, count, at(app, object + kObjectColours)));
    }
    return left;
}

/* 0x419840: greyObjects of eax. */
x86::reg32 greyObjectList(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = greyObjects(app, cpu, args.eax);
    cpu.esp += 4;
    return args.eax;
}

/* 0x419870's: the whole track's colours made grey, as tintTrackColours goes.
 * What the code leaves in eax, as there. */
reg32 greyTrackColours(App* app, const CPU& cpu)
{
    reg32 left = greyObjects(app, cpu, kTrackObjects);
    for (reg32 index = 0;; ++index)
    {
        safepoint(app, cpu);
        if (sreg32(index) > sreg32(at(app, kLastBlock)))
            break;
        const reg32 block = kBlocks + blockOffset(index);
        for (const reg32 list : kBlockLists)
            greyObjects(app, cpu, block + list);
        const reg32 count = at(app, block + kBlockVertexCount);
        left = greyLeft(count, greyColourRun(app, cpu, count, at(app, block + kBlockColours)));
    }
    return left;
}

/* 0x419870: greyTrackColours. */
x86::reg32 greyTrack(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = greyTrackColours(app, cpu);
    cpu.esp += 4;
    return args.eax;
}

/* 0x4198e0's: a list of objects read from the .frd into `list` -- its count,
 * then 0x34 bytes each, from the arena, with what each points at: its extra
 * data, its vertices, their colours and its polygons.  Each field's place
 * worked out afresh from the list, as the code does.  What the code leaves
 * in eax: the last read's (the memmove's), or what the arena gave where
 * there is no object; nullopt when the game stopped. */
std::optional<reg32> readObjects(App* app, CPU& cpu, reg32 list)
{
    if (!readFrd(app, cpu, list + kListCount, 4))
        return std::nullopt;
    reg32 left = takeFromArena(app, at(app, list + kListCount) * kObjectSize).value_or(0);
    put(app, list + kListObjects, left);
    struct Field
    {
        reg32 offset, bytes;
    };
    // Its first five fields: 4, 4, 4, 12 and 4 bytes.
    static const Field kFirstFields[] = {{0, 4}, {4, 4}, {8, 4}, {0xc, 0xc}, {kObjectExtraSize, 4}};
    for (reg32 index = 0;; ++index)
    {
        safepoint(app, cpu);
        if (sreg32(index) >= sreg32(at(app, list + kListCount)))
            break;
        const auto place = [&](reg32 offset) { return at(app, list + kListObjects) + index * kObjectSize + offset; };
        // `bytes` of the arena taken, to the field at `offset`.
        const auto takeInto = [&](reg32 offset, reg32 bytes) {
            const reg32 taken = takeFromArena(app, bytes).value_or(0);
            put(app, place(offset), taken);
        };
        for (const Field& field : kFirstFields)
            if (!readFrd(app, cpu, place(field.offset), field.bytes))
                return std::nullopt;
        const reg32 object = place(0), extraSize = at(app, object + kObjectExtraSize);
        if (extraSize == 0)
            put(app, object + kObjectExtra, 0);
        else
        {
            takeInto(kObjectExtra, extraSize);
            if (!readFrd(app, cpu, at(app, place(kObjectExtra)), at(app, place(kObjectExtraSize))))
                return std::nullopt;
        }
        if (!readFrd(app, cpu, place(kObjectVertexCount), 4))
            return std::nullopt;
        takeInto(kObjectVertices, at(app, place(kObjectVertexCount)) * kVertexSize);
        const reg32 vertices = place(0);
        if (!readFrd(app, cpu, at(app, vertices + kObjectVertices), at(app, vertices + kObjectVertexCount) * kVertexSize))
            return std::nullopt;
        takeInto(kObjectColours, at(app, place(kObjectVertexCount)) * kColourSize);
        if (!readFrd(app, cpu, at(app, place(kObjectColours)), at(app, place(kObjectVertexCount)) * kColourSize))
            return std::nullopt;
        if (!readFrd(app, cpu, place(kObjectPolygonCount), 4))
            return std::nullopt;
        takeInto(kObjectPolygons, at(app, place(kObjectPolygonCount)) * kPolygonSize);
        const reg32 last = place(0);
        const std::optional<reg32> read =
            readFrd(app, cpu, at(app, last + kObjectPolygons), at(app, last + kObjectPolygonCount) * kPolygonSize);
        if (!read)
            return std::nullopt;
        left = *read;
    }
    return left;
}

/* 0x4198e0: readObjects into eax. */
x86::reg32 readObjectList(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEdi);
    const std::optional<reg32> left = readObjects(app, cpu, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    cpu.esp += 4;
    return args.eax;
}

// A light or sound source of the .frd: 0x2f bytes, +4 its kind's record, +0x2b's high word its kind,
// +0x2c set where the kind is the game's (kLightKinds: a record's address each), else the track's
// (kTrackKinds: 0x2c bytes each); a kind's record +4 what the light is given.
constexpr reg32 kLightSize = 0x2f, kLightKindRecord = 4, kLightKindWord = 0x2b, kLightGameKind = 0x2c,
                kTrackKindSize = 0x2c, kKindRecord = 4;

/* 0x419ab0's: the track's lights and sound sources read from the .frd
 * (their count, then each into the arena), each given the record of its
 * kind.  What the code leaves in eax: the last one's record, or what the
 * arena gave where there is none; nullopt when the game stopped. */
std::optional<reg32> readTrackLights(App* app, CPU& cpu)
{
    if (!readFrd(app, cpu, kLightCount, 4))
        return std::nullopt;
    reg32 left = takeFromArena(app, at(app, kLightCount) * kLightSize).value_or(0);
    put(app, kLights, left);
    for (reg32 index = 0;; ++index)
    {
        safepoint(app, cpu);
        if (sreg32(index) >= sreg32(at(app, kLightCount)))
            break;
        if (!readFrd(app, cpu, at(app, kLights) + index * kLightSize, kLightSize))
            return std::nullopt;
        const reg32 light = at(app, kLights) + index * kLightSize;
        const reg32 kind = reg32(sreg32(at(app, light + kLightKindWord)) >> 16);
        if (byte(app, light + kLightGameKind) != 0)
            left = at(app, at(app, kLightKinds + kind * 4) + kKindRecord);
        else
            left = at(app, kind * kTrackKindSize + at(app, kTrackKinds) + kKindRecord);
        put(app, light + kLightKindRecord, left);
    }
    return left;
}

/* 0x419ab0: readTrackLights. */
x86::reg32 readLights(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx | kSaveEdx);
    const std::optional<reg32> left = readTrackLights(app, cpu);
    if (!left)
        return args.eax;
    args.eax = *left;
    cpu.esp += 4;
    return args.eax;
}

/* 0x419b60's: the arena allocated (0x310000 bytes, unnamed) and none of it
 * used.  Its address, as the code leaves it in eax; nullopt when the game
 * stopped. */
std::optional<reg32> startArena(App* app, CPU& cpu)
{
    put(app, kArenaUsed, 0);
    const std::optional<reg32> arena = allocate(app, cpu, kNoName, kArenaSize);
    if (arena)
        put(app, kArena, *arena);
    return arena;
}

/* 0x419b60: startArena. */
x86::reg32 newArena(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEdx);
    const std::optional<reg32> arena = startArena(app, cpu);
    if (!arena)
        return args.eax;
    args.eax = *arena;
    cpu.esp += 4;
    return args.eax;
}

/* 0x419b90's: the arena freed, if there is one, and forgotten.  eax as the
 * free leaves it (`eax` as it was where there is no arena); nullopt when the
 * game stopped. */
std::optional<reg32> freeTheArena(App* app, CPU& cpu, reg32 eax)
{
    const reg32 arena = at(app, kArena);
    if (arena == 0)
        return eax;
    const std::optional<reg32> freed = freeBlock(app, cpu, arena);
    if (!freed)
        return std::nullopt;
    put(app, kArena, 0);
    put(app, kArenaUsed, 0);
    return freed;
}

/* 0x419b90: freeTheArena. */
x86::reg32 freeArena(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEcx | kSaveEdx);
    const std::optional<reg32> freed = freeTheArena(app, cpu, args.eax);
    if (!freed)
        return args.eax;
    args.eax = *freed;
    cpu.esp += 4;
    return args.eax;
}

/* 0x419bc0's: `bytes` of the arena, the use moved past them; nullopt where
 * they would take it past its size (the use stored as it was). */
std::optional<reg32> takeFromArena(App* app, reg32 bytes)
{
    const reg32 used = at(app, kArenaUsed), after = bytes + used;
    if (sreg32(after) > sreg32(kArenaSize))
    {
        put(app, kArenaUsed, used);
        return std::nullopt;
    }
    const reg32 taken = at(app, kArena) + used;
    put(app, kArenaUsed, after);
    return taken;
}

/* 0x419bc0: takeFromArena of eax bytes: eax what is taken, or 0; edx it too
 * where there is room, as the code leaves it. */
x86::reg32 arenaTake(App* app, x86::CPU& cpu, x86::Args& args)
{
    const std::optional<reg32> taken = takeFromArena(app, args.eax);
    args.eax = taken.value_or(0);
    if (taken)
        args.edx = *taken;
    cpu.esp += 4;
    return args.eax;
}

/* 0x419bf0's: `bytes` of the .frd moved to `to` (the game's memmove), the
 * reading moved on past them.  What the memmove leaves in eax; nullopt when
 * the game stopped. */
std::optional<reg32> readFrd(App* app, CPU& cpu, reg32 to, reg32 bytes)
{
    const std::optional<reg32> moved = memmove(app, cpu, to, at(app, kFrdAt), bytes);
    if (moved)
        put(app, kFrdAt, at(app, kFrdAt) + bytes);
    return moved;
}

/* 0x419bf0: readFrd of edx bytes to eax. */
x86::reg32 frdRead(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx | kSaveEsi);
    const std::optional<reg32> moved = readFrd(app, cpu, args.eax, args.edx);
    if (!moved)
        return args.eax;
    args.eax = *moved;
    cpu.esp += 4;
    return args.eax;
}

// The driver's bit (kDriverBits) of a grey track.
constexpr reg8 kDriverGrey = 2;

/* 0x41a100's: the track's geometry loaded (the .frd read), its colours
 * tinted and, on a driver that wants it, made grey.  What the code leaves in
 * eax (tintTrackColours' or greyTrackColours'); nullopt when the game
 * stopped. */
std::optional<reg32> loadTrackGeometry(App* app, CPU& cpu)
{
    guest::pointerCall(app, cpu, 0x419c20, {});  // loader::frd (native_loading.cpp): a thunk without a core
    if (cpu.terminate)
        return std::nullopt;
    reg32 left = tintTrackColours(app, cpu);
    if ((byte(app, kDriverBits) & kDriverGrey) != 0)
        left = greyTrackColours(app, cpu);
    return left;
}

/* 0x41a100: loadTrackGeometry. */
x86::reg32 loadGeometry(App* app, x86::CPU& cpu, x86::Args& args)
{
    const std::optional<reg32> left = loadTrackGeometry(app, cpu);
    if (!left)
        return args.eax;
    args.eax = *left;
    cpu.esp += 4;
    return args.eax;
}

/* What the code's colour scaling leaves in ebx: the even channels scaled
 * (scaledColour's first mul, shifted down). */
reg32 evenScaled(reg32 colour, reg32 factor)
{
    if (sreg32(factor) >= 0x10000)
        factor = 0xffff;
    return (((colour & 0xff00ff) * ((factor >> 8) & 0xff)) >> 8) & 0xff00ff;
}

// A polygon record of the headlights' pass: 0xa0 bytes -- +0 the next's place, +4 its kind (a word, 1),
// +6 a word (0x13), +8.. its three corners' places, +0x18 its texture, its corners of 0x20 bytes from
// +0x20 (each +0xc the stack's corner's first word, +0x10 its colour, +0x18 and +0x1c its +0xc and
// +0x10).  A corner on the stack: 0x1c bytes, +8 its vertex (+0x14 its screen's outcode).
constexpr reg32 kTriangleSize = 0xa0, kTriangleCorner = 0x20, kStackCornerSize = 0x1c, kCornerVertex = 8,
                kVertexOutcode = 0x14, kCornerColour = 0x14;
constexpr reg32 kAlphaToFactor = 0x536c6c;  // a double: alpha intensity to a 16.16 factor
constexpr reg32 kLeastAlpha = 8;

/* What projectedTriangle leaves in eax, ebx and edx. */
struct TriangleLeft
{
    reg32 drawn;     // 1 for a record, else 0
    reg32 scaled;    // the last outcode's byte, or the last colour's even channels scaled (ebx)
    reg32 pointing;  // the outcodes' and, the third corner's place, or the records' slot (edx)
};

/* 0x41a120's: a triangle of the projected headlights' pass as a polygon
 * record at [records], [records] moved past it -- the corners the three at
 * `corners` (0x1c bytes each, on the stack), each's colour scaled by alpha
 * intensity below 1 (where they lie).  Not where all three corners are off
 * the screen on one side, nor where none is bright enough (the colours'
 * alpha below 8).
 *
 * The port's change (the port's alpha intensity change): the scaling on this
 * driver too, not only where [0x7a3a58] says Direct3D (bit 0x40). */
TriangleLeft headlightTriangle(App* app, reg32 records, reg32 corners)
{
    const auto corner = [&](reg32 k) { return corners + k * kStackCornerSize; };
    const auto outcode = [&](reg32 k) { return reg32(byte(app, at(app, corner(k) + kCornerVertex) + kVertexOutcode)); };
    const reg32 record = at(app, records);
    const reg32 places[] = {record + kTriangleCorner, record + 2 * kTriangleCorner, record + 3 * kTriangleCorner};
    const reg32 both = outcode(1) & outcode(0), third = outcode(2);
    if ((both & third) != 0)
        return {0, third, both};
    putHalf(app, record + 4, 1);
    put(app, record + 0x18, kHeadlightTexture);
    putHalf(app, record + 6, 0x13);
    put(app, record, record + kTriangleSize);
    put(app, record + 8, places[0]);
    put(app, record + 0xc, places[1]);
    put(app, record + 0x10, places[2]);
    // rep movsd: each corner's vertex, 8 dwords, into the record's corner.
    for (reg32 k = 0; k < 3; ++k)
        for (reg32 i = 0; i < 0x20; i += 4)
            put(app, places[k] + i, at(app, at(app, corner(k) + kCornerVertex) + i));
    TriangleLeft left = {0, third, places[2]};
    // Port: test ah, 0x40 taken as set -- the scaling on this driver too.  An x87 compare of the
    // intensity stored as a double (a NaN not scaled).
    const Float intensity = Float(double(Float(gameSettings(app).alphaIntensity)));
    if (Float(1.0) > intensity)
    {
        const reg32 factor = reg32(fp::toInteger<sreg32>(fp::mul(intensity, f64(app, kAlphaToFactor))));
        for (reg32 k = 0; k < 3; ++k)
        {
            const reg32 colour = at(app, corner(k) + kCornerColour);
            left.scaled = evenScaled(colour, factor);
            put(app, corner(k) + kCornerColour, scaledColour(colour, factor));
        }
    }
    for (reg32 k = 0; k < 3; ++k)
    {
        put(app, places[k] + 0xc, at(app, corner(k)));
        put(app, places[k] + 0x18, at(app, corner(k) + 0xc));
        put(app, places[k] + 0x1c, at(app, corner(k) + 0x10));
        put(app, places[k] + 0x10, at(app, corner(k) + kCornerColour));
    }
    const reg32 alpha = (at(app, places[0] + 0x10) | at(app, places[1] + 0x10) | at(app, places[2] + 0x10)) >> 0x18;
    if (alpha < kLeastAlpha)
        return left;
    put(app, records, at(app, records) + kTriangleSize);
    return {1, left.scaled, records};
}

/* 0x41a120 (stdcall, 0x54 bytes): headlightTriangle into [eax] of the
 * stack's three corners. */
x86::reg32 projectedTriangle(App* app, x86::CPU& cpu, x86::Args& args)
{
    const TriangleLeft left = headlightTriangle(app, args.eax, cpu.esp + 4);
    args.eax = left.drawn;
    args.ebx = left.scaled;
    args.edx = left.pointing;
    cpu.esp += 4 + 0x54;
    return args.eax;
}

/* A float's size as the code takes it, by its bits: only a value under 0
 * negated, anything else (a NaN too) kept to the bit. */
reg32 sizeBits(reg32 bits)
{
    return Float(0.0) > floatOf(bits) ? bitsOf(float(-floatOf(bits))) : bits;
}

/* A size at most 1, as the code clamps it: 0 under 0 (an x87 compare), else
 * its bits against 1's as an int (a NaN with its sign set kept). */
reg32 clampedBits(reg32 bits)
{
    constexpr reg32 kOne = 0x3f800000;
    if (Float(0.0) > floatOf(bits))
        return 0;
    return sreg32(bits) > sreg32(kOne) ? kOne : bits;
}

/* The colour scaled by the 16.16 factor (0x10000 and up taken as 0xffff):
 * the factor's high byte times each channel, two at a time, as the code's
 * two muls do. */
reg32 scaledColour(reg32 colour, reg32 factor)
{
    if (sreg32(factor) >= 0x10000)
        factor = 0xffff;
    const reg32 by = (factor >> 8) & 0xff;
    const reg32 even = (colour & 0xff00ff) * by, odd = ((colour & 0xff00ff00) >> 8) * by;
    return (odd & 0xff00ff00) | ((even >> 8) & 0xff00ff);
}

constexpr reg32 kOffMiddleScale = 0x536c74;  // a float: how much off the view's middle darkens
constexpr reg32 kShadeToFactor = 0x536c7c;   // a double: the shade to a 16.16 factor

/* 0x41a3e0's: the colour of a track vertex darkened, to [to] -- by how far
 * off the view's middle it is (|x| and |y|, `xBits` and `yBits` as the code
 * has them, over its depth, each clamped to 1, their sum times 0x536c74 off
 * 1) and, beyond the near distance, by its depth over the shading's range
 * (shadeDistances).  The colour, as the code leaves it in eax. */
reg32 shadeTrackVertex(App* app, reg32 xBits, reg32 yBits, float depth, reg32 colour, reg32 to)
{
    const float inverse = float(fp::div(Float(1.0), Float(depth)));
    float shade = 1.0f;
    const reg32 absX = sizeBits(xBits), absY = sizeBits(yBits);
    const float across = float(fp::mul(floatOf(absX), Float(inverse)));
    const float up = float(fp::mul(Float(inverse), floatOf(absY)));
    // An x87 compare: beyond the near distance only where ordered and over.
    if (Float(depth) > f32(app, kShadeNear))
    {
        const Float beyond = fp::mul(fp::sub(Float(depth), f32(app, kShadeNear)), f32(app, kShadeScale));
        shade = float(fp::sub(Float(1.0), beyond));
    }
    const Float sum = fp::add(floatOf(clampedBits(bitsOf(across))), floatOf(clampedBits(bitsOf(up))));
    const Float off = fp::mul(sum, f32(app, kOffMiddleScale));
    const Float darkened = fp::mul(fp::sub(Float(1.0), off), Float(shade));
    const Float factor = fp::mul(darkened, f64(app, kShadeToFactor));
    const reg32 shaded = scaledColour(colour, reg32(fp::toInteger<sreg32>(factor)));
    put(app, to, shaded);
    return shaded;
}

/* 0x41a3e0 (stdcall, 5 words): shadeTrackVertex of x, y, the depth, the
 * colour and where to of the stack's words. */
x86::reg32 shadeVertex(App* app, x86::CPU& cpu, x86::Args& args)
{
    const reg32 words = cpu.esp;
    args.eax = shadeTrackVertex(app, at(app, words + 4), at(app, words + 8), app->getMemory<float>(words + 0xc),
                               at(app, words + 0x10), at(app, words + 0x14));
    cpu.esp += 4 + 0x14;
    return args.eax;
}

// A camera: its kind (+0, 1 the mirror's pass), its place (+8), its view box (+0x38), its matrix (+0x44);
// the shading's: the light's reach (+0x64), the near and far distances (+0x68, +0x6c).
constexpr reg32 kCameraKind = 0, kCameraNode = 4, kCameraPlace = 8, kCameraBox = 0x38, kCameraMatrix = 0x44,
                kCameraReach = 0x64, kCameraNear = 0x68, kCameraFar = 0x6c;

/* 0x41a800's: the shading's distances for the camera at `camera` scaled by
 * `scale`: its near and far ones, the far at least 1 past the near; the
 * light's reach; 1 / (far - near).  The far one as stored (its bits, as the
 * code leaves them in eax). */
reg32 shadeDistances(App* app, reg32 camera, float scale)
{
    const Float s = Float(scale);
    const Float nearDistance = fp::mul(f32(app, camera + kCameraNear), s);
    const Float farDistance = fp::mul(s, f32(app, camera + kCameraFar));
    put(app, kShadeReach, at(app, camera + kCameraReach));
    putF32(app, kShadeNear, nearDistance);
    putF32(app, kShadeFar, farDistance);
    const float nearPlusOne = float(fp::add(Float(1.0), f32(app, kShadeNear)));
    // An x87 compare: the far one kept only where it is over (a NaN either side takes the near's).
    const reg32 farthest = f32(app, kShadeFar) > Float(nearPlusOne) ? at(app, kShadeFar) : bitsOf(nearPlusOne);
    const Float inverse = fp::div(Float(1.0), fp::sub(floatOf(farthest), f32(app, kShadeNear)));
    put(app, kShadeFar, farthest);
    putF32(app, kShadeScale, inverse);
    return farthest;
}

/* 0x41a800 (stdcall, 2 words): shadeDistances for the camera eax by
 * [esp+4]. */
x86::reg32 setShadeDistances(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = shadeDistances(app, args.eax, app->getMemory<float>(cpu.esp + 4));
    cpu.esp += 4 + 8;
    return args.eax;
}


/* An object's vertices to the screen (vertices::objectVerticesList): turned
 * by the matrix at `matrix`, moved by the vector at `move`, `count` of them
 * at `vertices`, their colours at `colours`, the screen's records out to
 * `records`.  False when the game stopped. */
bool objectVerticesList(App* app, CPU& cpu, reg32 matrix, reg32 move, reg32 count, reg32 vertices, reg32 colours,
                        reg32 records)
{
    const GuestFrame words(cpu, 2 * 4, 0);
    guest::direct<0x4dbef0, vertices::objectVerticesListCore>(app, cpu, matrix, move, count, vertices, colours, records);
    return !cpu.terminate;
}

/* The place at `point` less the camera's, turned by its matrix: the view's
 * move into the frame's `into` (12 bytes), `apart` its 12 bytes before. */
void pointIntoView(App* app, reg32 camera, reg32 point, reg32 apart, reg32 into)
{
    putF32(app, apart, fp::sub(f32(app, point), f32(app, camera + kCameraPlace)));
    putF32(app, apart + 4, fp::sub(f32(app, point + 4), f32(app, camera + kCameraPlace + 4)));
    putF32(app, apart + 8, fp::sub(f32(app, point + 8), f32(app, camera + kCameraPlace + 8)));
    turnVector(app, apart, camera + kCameraMatrix, into);
}

/* 0x41a8d0's: the point at `point` taken into the camera's view and the
 * object's vertices into view from it (objectVerticesList), the frame as
 * deep as the code's (the callee is handed its move).  False when the game
 * stopped. */
bool viewVertices(App* app, CPU& cpu, reg32 camera, reg32 point, reg32 count, reg32 vertices, reg32 colours,
                  reg32 records)
{
    const GuestFrame frame(cpu, 0xc, 0x18);
    const reg32 apart = frame - 0x18, move = frame - 0xc;
    pointIntoView(app, camera, point, apart, move);
    return objectVerticesList(app, cpu, camera + kCameraMatrix, move, count, vertices, colours, records);
}

/* 0x41a8d0 (stdcall, 2 words): viewVertices for the camera eax, the point
 * ebx, edx vertices at ecx, the colours and records the stack's words; the
 * registers as objectVerticesList leaves them. */
x86::reg32 viewObjectVertices(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEsi | kSaveEdi);
    if (!viewVertices(app, cpu, args.eax, args.ebx, args.edx, args.ecx, at(app, cpu.esp + 4), at(app, cpu.esp + 8)))
        return args.eax;
    cpu.esp += 4 + 8;
    return args.eax;
}

/* 0x41a9b0's: viewVertices for an object turned by its own matrix at
 * `turn`: the point into the camera's view, the object's matrix times the
 * camera's, then its vertices into view.  False when the game stopped. */
bool viewTurnedVertices(App* app, CPU& cpu, reg32 camera, reg32 turn, reg32 point, reg32 count, reg32 vertices,
                        reg32 colours, reg32 records)
{
    const GuestFrame frame(cpu, 0xc, 0x3c);
    const reg32 apart = frame - 0x18, move = frame - 0xc, matrix = frame - 0x3c;
    pointIntoView(app, camera, point, apart, move);
    guest::direct<0x4e0280, vertices::multiplyMatrix>(app, cpu, turn, camera + kCameraMatrix, matrix);
    if (cpu.terminate)
        return false;
    return objectVerticesList(app, cpu, matrix, move, count, vertices, colours, records);
}

/* 0x41a9b0 (stdcall, 3 words): viewTurnedVertices for the camera eax, the
 * matrix edx, the point ecx, ebx vertices at the stack's first word, the
 * colours and records its next two. */
x86::reg32 viewTurnedObjectVertices(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEsi | kSaveEdi);
    if (!viewTurnedVertices(app, cpu, args.eax, args.edx, args.ecx, args.ebx, at(app, cpu.esp + 4), at(app, cpu.esp + 8),
                            at(app, cpu.esp + 0xc)))
        return args.eax;
    cpu.esp += 4 + 0xc;
    return args.eax;
}

// The view's distances the track's model gets coarser at: the middle one's (+0x40) and the coarsest's (+0x44).
constexpr reg32 kViewMiddleFrom = 0x40, kViewCoarsestFrom = 0x44;

/* 0x41aa60's: which of the track's models (0 the coarsest, 2 the detailed
 * one) to draw at the square `squared` of a distance for the camera at
 * `camera`: past the view's coarsest distance, 0; past its middle one, 1;
 * else 2 -- x87 compares, a NaN counting as past.  The mirror's pass always
 * 0 -- the port's change (the port's mirror detail change): asked through
 * nfs3hp::mirrorAsMain, which answers as the main view would with the mirror
 * drawn in full. */
reg32 modelAt(App* app, reg32 camera, float squared)
{
    if (mirrorAsMain(at(app, camera + kCameraKind)) == kMirrorPass)
        return 0;
    const reg32 view = at(app, kView);
    const Float coarsest = f32(app, view + kViewCoarsestFrom);
    if (!(fp::mul(coarsest, coarsest) >= Float(squared)))
        return 0;
    const Float middle = f32(app, view + kViewMiddleFrom);
    if (!(fp::mul(middle, middle) >= Float(squared)))
        return 1;
    return 2;
}

/* 0x41aa60 (stdcall, 1 word): modelAt for the camera eax at [esp+4]. */
x86::reg32 modelByDistance(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = modelAt(app, args.eax, app->getMemory<float>(cpu.esp + 4));
    cpu.esp += 4 + 4;
    return args.eax;
}

// A view's groups of 8 blocks: +0xc the one its block is in, +0x10 the same of +4 once moved there; +0x14
// cleared to -1 where the group changed, what it held kept at +0x18.
constexpr reg32 kViewGroup = 0xc, kViewGroupNow = 0x10, kViewGroupMark = 0x14, kViewGroupMarkBefore = 0x18;

/* The block of the camera's node (carstate::cameraNode); nullopt when the
 * game stopped. */
std::optional<reg32> cameraNode(App* app, CPU& cpu, reg32 node)
{
    const reg32 block = guest::direct<0x4217e0, carstate::nodeOfCamera>(app, cpu, node);
    if (cpu.terminate)
        return std::nullopt;
    return block;
}

/* 0x41aaf0's: the view's distances worked out, view `index` the one drawn,
 * and the block the camera is in its block, 8 to a group.  The group, as the
 * code leaves it in edx; nullopt when the game stopped. */
std::optional<reg32> viewOnCameraBlock(App* app, CPU& cpu, reg32 camera, reg32 index)
{
    if (!guest::direct<0x41d620, render::viewDistancesSet>(app, cpu))
        return std::nullopt;
    chooseView(app, index);
    const std::optional<reg32> block = cameraNode(app, cpu, at(app, camera + kCameraNode));
    if (!block)
        return std::nullopt;
    const reg32 view = at(app, kView), group = reg32(sreg32(*block) >> 3), before = at(app, view + kViewGroup);
    put(app, view + kViewBlock, *block);
    if (group != before)
    {
        put(app, view + kViewGroup, group);
        const reg32 mark = at(app, view + kViewGroupMark);
        put(app, view + kViewGroupMark, 0xffffffff);
        put(app, view + kViewGroupMarkBefore, mark);
    }
    const reg32 drawn = at(app, kView), now = reg32(sreg32(at(app, drawn + kViewBlock)) >> 3);
    put(app, drawn + kViewGroupNow, now);
    return now;
}

/* 0x41aaf0: viewOnCameraBlock for the camera eax and view edx; eax the view
 * drawn, edx its group. */
x86::reg32 viewOnBlock(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEcx);
    const std::optional<reg32> group = viewOnCameraBlock(app, cpu, args.eax, args.edx);
    if (!group)
        return args.eax;
    args.eax = at(app, kView);
    args.edx = *group;
    cpu.esp += 4;
    return args.eax;
}

// A block's polygon groups by list: the count at +4 * list, the polygons at +0x2c + 4 * list; a polygon
// its four corners' numbers (words at +0, +2, +4, +6).  A lit corner (0x1c bytes, as 0x41a550 leaves it):
// +4 and +6 words 0 where the first and second light reach it, +8 its vertex, +0x14.. and +0x18.. the two
// lights' colour bytes; a vertex: +8 its depth, +0x10.. its colour bytes, +0x14 its screen's outcode.
constexpr reg32 kGroupPolygons = 0x2c, kLitCornerSize = 0x1c, kViewWorldEnd = 0x30;

/* A lit corner's vertex's colour byte `to` raised to the corner's byte
 * `from` where that is larger. */
void raiseColour(App* app, reg32 corner, reg32 from, reg32 to)
{
    const reg8 own = byte(app, corner + from), before = byte(app, at(app, corner + 8) + to);
    putByte(app, at(app, corner + 8) + to, larger(own, before));
}

/* 0x41ab40's: the headlights' light on the polygons of group `list` of
 * block `block`: the lit corners at `corners` of each polygon's four that
 * the first and then the second light reach raise their vertex's colour
 * bytes to theirs -- not a polygon past the world's end (an x87 compare, a
 * NaN not past) or off the screen on one side.  The polygons gone
 * through. */
reg32 lightPolygons(App* app, const CPU& cpu, reg32 block, reg32 list, reg32 corners)
{
    const reg32 group = kBlocks + blockOffset(block) + list * 4, count = at(app, group);
    reg32 polygon = at(app, group + kGroupPolygons);
    reg32 done = 0;
    for (;; ++done, polygon += kPolygonSize)
    {
        safepoint(app, cpu);
        if (sreg32(done) >= sreg32(count))
            break;
        const auto corner = [&](reg32 word) {
            return corners + reg32(sreg32(std::int16_t(half(app, polygon + word)))) * kLitCornerSize;
        };
        const reg32 first = corner(0), second = corner(2), third = corner(4), fourth = corner(6);
        const reg32 vertex = at(app, first + 8);
        if (f32(app, vertex + 8) > f32(app, at(app, kView) + kViewWorldEnd))
            continue;
        const auto outcode = [&](reg32 c) { return reg32(byte(app, at(app, c + 8) + kVertexOutcode)); };
        if ((outcode(fourth) & (reg32(byte(app, vertex + kVertexOutcode)) & outcode(second) & outcode(third))) != 0)
            continue;
        for (const reg32 c : {first, second, third, fourth})
            if (half(app, c + 4) == 0)
                for (const reg32 k : {2u, 1u, 0u})
                    raiseColour(app, c, 0x14 + k, 0x10 + k);
        if (half(app, first + 6) == 0)
            for (const reg32 k : {2u, 1u, 0u})
                raiseColour(app, first, 0x18 + k, 0x10 + k);
        for (const reg32 c : {second, third, fourth})
            if (half(app, c + 6) == 0)
                for (const reg32 k : {2u, 1u, 0u})
                    raiseColour(app, c, 0x18 + k, 0x10 + k);
    }
    return done;
}

/* 0x41ab40 (stdcall, 1 word): lightPolygons of block ebx's group ecx, the
 * corners at [esp+4]. */
x86::reg32 lightObjectPolygons(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = lightPolygons(app, cpu, args.ebx, args.ecx, at(app, cpu.esp + 4));
    cpu.esp += 4 + 4;
    return args.eax;
}

/* The drawing's state reset (0x4bbf20).  False when the game stopped. */
bool stateReset(App* app, CPU& cpu)
{
    guest::direct<0x4bbf20, frontend::rendd::openListStart>(app, cpu);
    return !cpu.terminate;
}

/* An empty list's pair (first, last) made at `pair` (0x4bbe70).  False when the game stopped. */
bool listEmptied(App* app, CPU& cpu, reg32 pair)
{
    guest::direct<0x4bbe70, frontend::rendd::emptyList>(app, cpu, pair);
    return !cpu.terminate;
}

// A state record of the drawing's: 0x10 bytes, +0 0, +4 its kind (a word, 5), +8 0xd, +0xc the last's 1.
constexpr reg32 kStateRecordSize = 0x10;

/* A state record of the drawing's at [record], where Render_GetTm gave room
 * for one: kind 5, 0xd, `last`.  False when the game stopped. */
bool stateRecord(App* app, CPU& cpu, reg32 record, reg32 last)
{
    const std::optional<reg32> room = arenaRoom(app, cpu, record, kStateRecordSize);
    if (!room)
        return false;
    if (*room != 0)
    {
        putHalf(app, at(app, record) + 4, 5);
        put(app, at(app, record), 0);
        put(app, at(app, record) + 8, 0xd);
        put(app, at(app, record) + 0xc, last);
    }
    return true;
}

/* The state record at [record] made a list of its own into `pair`, and the
 * lists `first` then `second` (pairs of first and last) joined into the
 * list's pair at `list`.  The join's eax (the list it made); nullopt when
 * the game stopped. */
std::optional<reg32> joinRecord(App* app, CPU& cpu, reg32 record, reg32 pair, reg32 list, bool recordFirst,
                                reg32 joined)
{
    const reg32 listFirst = at(app, list), listLast = at(app, list + 4);
    guest::direct<0x4bbe90, frontend::rendd::listOfOne>(app, cpu, at(app, record), pair);
    if (cpu.terminate)
        return std::nullopt;
    const reg32 pairFirst = at(app, pair), pairLast = at(app, pair + 4);
    const reg32 made = recordFirst
                           ? guest::direct<0x4bbec0, frontend::rendd::joinLists>(app, cpu, joined, pairFirst, pairLast,
                                                                                  listFirst, listLast)
                           : guest::direct<0x4bbec0, frontend::rendd::joinLists>(app, cpu, joined, listFirst, listLast,
                                                                                  pairFirst, pairLast);
    if (cpu.terminate)
        return std::nullopt;
    put(app, list, at(app, joined));
    put(app, list + 4, at(app, joined + 4));
    return made;
}

/* 0x41b8c0's: the list at `list` (its first and last record), if not
 * empty, put between two state records of the drawing's (0x10 bytes from
 * Render_GetTm, kind 5, 0xd, the first +0xc 0 and the last 1): the drawing's
 * state reset, each record made a list of its own and the lists joined.  The
 * frame as deep as the code's (its places are handed on).  False when the
 * game stopped. */
std::optional<reg32> bracketOrdered(App* app, CPU& cpu, reg32 list, reg32 eax)
{
    if (at(app, list) == 0)
        return eax;
    const GuestFrame frame(cpu, 0x18, 0x24);
    const reg32 record = frame - 4;
    if (!stateReset(app, cpu) || !stateRecord(app, cpu, record, 0)
        || !joinRecord(app, cpu, record, frame - 0x14, list, true, frame - 0x24) || !stateRecord(app, cpu, record, 1))
        return std::nullopt;
    return joinRecord(app, cpu, record, frame - 0xc, list, false, frame - 0x1c);
}

/* The header's core (native_render's caller wants only whether the game went on). */
bool bracketOrdered(App* app, CPU& cpu, reg32 list)
{
    return bracketOrdered(app, cpu, list, list).has_value();
}

/* 0x41b8c0: bracketOrdered of eax; eax as the last join leaves it. */
x86::reg32 bracketList(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEdi);
    const std::optional<reg32> last = bracketOrdered(app, cpu, args.eax, args.eax);
    if (!last)
        return args.eax;
    args.eax = *last;
    cpu.esp += 4;
    return args.eax;
}

// A block's neighbours drawn with it (+0xb4: 300, a word read every 4 bytes, -1 none) and its sound
// sources (+0x5b0 their count, +0x5b4 them).
constexpr reg32 kBlockNeighbours = 0xb4, kNeighbourCount = 300, kBlockSourceCount = 0x5b0, kBlockSources = 0x5b4;
constexpr reg32 kAmbientSlot = 0x12, kAmbientSlotSecond = 0x20;
// The second player's camera (+4 not 0).
constexpr reg32 kCameraPlayer = 4;

std::optional<reg32> heardSources(App* app, CPU& cpu, reg32 camera, reg32 sources, reg32 heard);

/* 0x41bb50's: the ambient sounds around the camera (none in the mirror's
 * pass): the view on its block, the sound sources of each of the block's
 * neighbours heard; none heard, the ambient slot stopped (0x12, or 0x20 for
 * the second player).  What the code leaves in eax: the slot's stopping's,
 * else what the last neighbour left (its sources' heard, its block's offset,
 * or its place where there is none); the camera in the mirror's pass.
 * nullopt when the game stopped. */
std::optional<reg32> ambientAround(App* app, CPU& cpu, reg32 camera)
{
    if (at(app, camera + kCameraKind) != 0)
        return camera;
    if (!viewOnCameraBlock(app, cpu, camera, at(app, camera + kCameraNode)))
        return std::nullopt;
    const GuestFrame frame(cpu, 0x18, 4);  // as deep as the code's: hearSource hands its own frame on
    const reg32 neighbours = kBlocks + blockOffset(at(app, at(app, kView) + kViewGroupNow)) + kBlockNeighbours;
    reg32 heard = 0, left = 0;
    for (reg32 index = 0; index < kNeighbourCount; ++index)
    {
        safepoint(app, cpu);
        left = neighbours + index * 4;
        const reg16 neighbour = half(app, left);
        if (neighbour == 0xffff)
            continue;
        safepoint(app, cpu);
        left = blockOffset(reg32(sreg32(std::int16_t(neighbour))));
        if (at(app, kBlocks + left + kBlockSourceCount) == 0)
            continue;
        std::optional<reg32> some;
        {
            const GuestFrame slot(cpu, 4, 0);  // the call's return address
            some = heardSources(app, cpu, camera, at(app, kBlocks + left + kBlockSources), heard);
        }
        if (!some)
            return std::nullopt;
        left = *some;
        heard |= left;
    }
    if (heard != 0)
        return left;
    const port::Result stopped = guest::direct<0x416f60, port::slotStopped>(
        app, cpu, cpu.esp - 4, i32(at(app, camera + kCameraPlayer) == 0 ? kAmbientSlot : kAmbientSlotSecond));
    return cpu.terminate || !stopped ? std::optional<reg32>() : std::optional<reg32>(reg32(*stopped));
}

/* 0x41bb50: ambientAround the camera eax. */
x86::reg32 ambientSounds(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEdi);
    const std::optional<reg32> left = ambientAround(app, cpu, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    cpu.esp += 4;
    return args.eax;
}

/* 0x41bc20's: whether the track's ambient sounds play: not in a split-screen
 * or network race but on track 0 (the settings' fields so named). */
bool ambientPlays(App* app)
{
    const auto& settings = gameSettings(app);
    return !((settings.weather != 0 || settings.night != 0) && settings.track == 0);
}

/* 0x41bc20: ambientPlays, 1 or 0. */
x86::reg32 ambientWanted(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = ambientPlays(app) ? 1 : 0;
    cpu.esp += 4;
    return args.eax;
}

// A light of the track's: its place (+0x4c) and its reach (+0x6c).
constexpr reg32 kLightPlace = 0x4c, kLightReach = 0x6c;

/* 0x41bc50's: whether the light at `light` shines away from the camera:
 * its place taken into the camera's view (into the frame, as deep as the
 * code's) and its depth there against the light's own -(+0x6c) -- an x87
 * compare, so a NaN either side is not behind.  nullopt when the game
 * stopped. */
std::optional<bool> lightBehind(App* app, CPU& cpu, reg32 light, reg32 camera)
{
    const GuestFrame frame(cpu, 0x10, 0xc);
    const reg32 inView = frame - 0xc;
    guest::direct<0x4e0430, vertices::transformPointList>(app, cpu, 1, light + kLightPlace, camera + kCameraMatrix,
                                                          camera + kCameraBox, inView);
    if (cpu.terminate)
        return std::nullopt;
    return Float(-f32(app, light + kLightReach)) > f32(app, inView + 8);
}

/* 0x41bc50: lightBehind for the light eax and the camera edx, 1 behind. */
x86::reg32 pointBehindView(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx | kSaveEsi);
    const std::optional<bool> behind = lightBehind(app, cpu, args.eax, args.edx);
    if (!behind)
        return args.eax;
    args.eax = *behind ? 1 : 0;
    cpu.esp += 4;
    return args.eax;
}

// carstate::zoomOfView (0x4227e0): the light's spread, as the x87's stack has it.
// The screen's projection (scales x, y; centres x, y) and its bounds: the depth's (1 / z), y's top and
// bottom, x's left and right.
constexpr reg32 kScreenScaleX = 0x56009c, kScreenScaleY = 0x5600a0, kScreenMiddleX = 0x5600a4,
                kScreenMiddleY = 0x5600a8;
constexpr reg32 kScreenDepth = 0x7d34f8, kScreenTop = 0x7d3504, kScreenBottom = 0x7d3500, kScreenLeft = 0x7d34fc,
                kScreenRight = 0x7d350c;
constexpr reg32 kOutBehind = 0x10, kOutAbove = 8, kOutBelow = 4, kOutLeft = 1, kOutRight = 2;
constexpr reg32 kDepthOffZero = 0x37800080;  // the depth a corner at 0 is given, as bits

/* An int of a float's bits against a bound, as the code compares them: its
 * sign set, or signed under. */
bool underBits(reg32 bits, reg32 bound)
{
    return (bits & 0x80000000) != 0 || sreg32(bits) < sreg32(bound);
}

/* 0x41bc90's: whether the box of four corners at `box` is out of the light
 * `light`'s cone: each corner turned by the matrix at `matrix` and moved by
 * the vector at `origin`, its depth kept off 0, projected onto the screen
 * with the screen's outcode, and |x| times the light's spread (at most 1)
 * against |depth|.  Out where every corner is beside the cone (x87
 * compares, a NaN not beside) and all are off the screen on one side.
 * nullopt when the game stopped. */
std::optional<bool> boxOutOfCone(App* app, CPU& cpu, reg32 light, reg32 box, reg32 matrix, reg32 origin)
{
    const Float value = guest::direct<0x4227e0, carstate::zoomOfView>(app, cpu, light);
    if (cpu.terminate)
        return std::nullopt;
    // An x87 compare: over 1, or a NaN, is 1.
    const float spread = Float(1.0) < value || value != value ? 1.0f : float(value);
    reg32 sideX[4], sideZ[4];
    reg8 outcodes[4];
    for (reg32 index = 0; index < 4; ++index)
    {
        // The corner in view: x, y, z as the code stores them (into its frame).
        const Float a = fp::mul(f32(app, box + index * 12), f32(app, matrix));
        const Float b = fp::mul(f32(app, box + index * 12), f32(app, matrix + 4));
        const Float c = fp::mul(f32(app, box + index * 12), f32(app, matrix + 8));
        const Float d = fp::mul(f32(app, box + index * 12 + 4), f32(app, matrix + 0xc));
        const Float e = fp::mul(f32(app, box + index * 12 + 4), f32(app, matrix + 0x10));
        const Float f = fp::mul(f32(app, box + index * 12 + 4), f32(app, matrix + 0x14));
        const Float x0 = fp::add(a, d), y0 = fp::add(b, e), z0 = fp::add(c, f);
        const Float g = fp::mul(f32(app, box + index * 12 + 8), f32(app, matrix + 0x18));
        const Float h = fp::mul(f32(app, box + index * 12 + 8), f32(app, matrix + 0x1c));
        const Float i = fp::mul(f32(app, box + index * 12 + 8), f32(app, matrix + 0x20));
        const float turnedX = float(fp::add(x0, g)), turnedY = float(fp::add(y0, h)), turnedZ = float(fp::add(z0, i));
        const float x = float(fp::add(f32(app, origin), Float(turnedX)));
        const float y = float(fp::add(f32(app, origin + 4), Float(turnedY)));
        reg32 zBits = bitsOf(float(fp::add(f32(app, origin + 8), Float(turnedZ))));
        if ((zBits & 0x7fffffff) == 0)
            zBits = kDepthOffZero;
        const float inverse = float(fp::div(Float(1.0), floatOf(zBits)));
        const Float p = fp::mul(f32(app, kScreenScaleX), Float(inverse));
        const Float q = fp::mul(f32(app, kScreenScaleY), Float(inverse));
        const Float qy = fp::mul(q, Float(y));
        const Float px = fp::mul(p, Float(x));
        const reg32 screenX = bitsOf(float(fp::add(px, f32(app, kScreenMiddleX))));
        const reg32 screenY = bitsOf(float(fp::add(qy, f32(app, kScreenMiddleY))));
        // The outcode: behind the near plane; else above, below, left, right of the screen.
        reg32 outcode = 0;
        if ((bitsOf(inverse) & 0x80000000) != 0 || sreg32(bitsOf(inverse)) >= sreg32(at(app, kScreenDepth)))
            outcode = kOutBehind;
        else
        {
            if (underBits(screenY, at(app, kScreenTop)))
                outcode |= kOutAbove;
            else if (sreg32(screenY) > sreg32(at(app, kScreenBottom)))
                outcode |= kOutBelow;
            if (underBits(screenX, at(app, kScreenLeft)))
                outcode |= kOutLeft;
            else if (sreg32(screenX) > sreg32(at(app, kScreenRight)))
                outcode |= kOutRight;
        }
        outcodes[index] = reg8(outcode);
        // |z|, and |x| times the spread.
        sideZ[index] = Float(0.0) > floatOf(zBits) ? bitsOf(float(-floatOf(zBits))) : zBits;
        const reg32 xBits = bitsOf(x);
        const reg32 absX = Float(0.0) > Float(x) ? bitsOf(float(-Float(x))) : xBits;
        sideX[index] = bitsOf(float(fp::mul(floatOf(absX), Float(spread))));
    }
    for (reg32 index = 0; index < 4; ++index)
        if (!(floatOf(sideX[index]) > floatOf(sideZ[index])))
            return false;
    return (outcodes[0] & outcodes[1] & outcodes[2] & outcodes[3]) != 0;
}

/* 0x41bc90: boxOutOfCone for the light eax, the box edx, the matrix ebx and
 * the move ecx; 1 out of the cone. */
x86::reg32 litBoxOutside(App* app, x86::CPU& cpu, x86::Args& args)
{
    const std::optional<bool> outside = boxOutOfCone(app, cpu, args.eax, args.edx, args.ebx, args.ecx);
    if (!outside)
        return args.eax;
    args.eax = *outside ? 1 : 0;
    cpu.esp += 4;
    return args.eax;
}

constexpr reg32 kTrackLists = 6;
// The buckets (8 bytes each, a list's pair): the four opaque ones sorted into the frame's, the ordered
// one at +0x28 bracketed, it and +0x20 added to the ordered bucket.
constexpr reg32 kBucketsOrdered = 0x28, kBucketsSecond = 0x20;

/* The pair (8 bytes) at `from` copied to `to`, as the code's two movsd. */
void copyPair(App* app, reg32 from, reg32 to)
{
    put(app, to, at(app, from));
    put(app, to + 4, at(app, from + 4));
}

/* An empty list's pair made at `pair` (in the frame) and copied to `to`.
 * False when the game stopped. */
bool emptyPairTo(App* app, CPU& cpu, reg32 pair, reg32 to)
{
    if (!listEmptied(app, cpu, pair))
        return false;
    copyPair(app, pair, to);
    return true;
}

/* 0x41c1a0's: a view's buckets of polygons handed to the drawing: the six
 * lists of the track's (`heads`, `tails`) into the buckets at `buckets` (an
 * empty one where a list is), the four opaque buckets sorted into the
 * frame's, the ordered ones bracketed (bracketOrdered) and added, all
 * gathered, the buckets emptied and the view drawn.  The frame as deep as
 * the code's (its pairs are handed on), each callee handed the registers the
 * code hands it.  False when the game stopped. */
std::optional<reg32> drawBuckets(App* app, CPU& cpu, reg32 view, reg32 tails, reg32 heads, reg32 buckets)
{
    const GuestFrame frame(cpu, 0xc, 0x4c);
    const reg32 gathered = frame - 0x4c, empty = frame - 0x34;
    if (!listEmptied(app, cpu, gathered))
        return std::nullopt;
    for (reg32 list = 0; list < kTrackLists; ++list)
    {
        const reg32 head = at(app, heads + list * 4), to = buckets + list * 8;
        if (head == 0)
        {
            safepoint(app, cpu);
            if (!emptyPairTo(app, cpu, empty, to))
                return std::nullopt;
        }
        else
        {
            put(app, to, head);
            const reg32 tail = at(app, tails + list * 4);
            put(app, tail, 0);
            put(app, to + 4, at(app, tails + list * 4));
        }
        safepoint(app, cpu);
        put(app, heads + list * 4, 0);
        put(app, tails + list * 4, 0);
    }
    static const reg32 kOpaque[][2] = {{0, kBucketOpaque}, {8, kBucketA}, {0x18, kBucketB}, {0x10, kBucketC}};
    for (const auto& bucket : kOpaque)
    {
        {
            const GuestFrame words(cpu, 4, 0);
            guest::direct<0x4c7790, vertices::depthBucketsCore>(app, cpu, at(app, buckets + bucket[0]),
                                                               at(app, bucket[1]));
        }
        if (cpu.terminate)
            return std::nullopt;
    }
    {
        // bracketList's.
        const GuestFrame slot(cpu, 4, 0);
        if (!bracketOrdered(app, cpu, buckets + kBucketsOrdered, 0))
            return std::nullopt;
    }
    guest::direct<0x4c7940, frontend::rende::drawListAdd>(app, cpu, at(app, buckets + kBucketsOrdered),
                                                         at(app, buckets + kBucketsOrdered + 4), at(app, kBucketOrdered));
    if (cpu.terminate)
        return std::nullopt;
    guest::direct<0x4c7940, frontend::rende::drawListAdd>(app, cpu, at(app, buckets + kBucketsSecond),
                                                         at(app, buckets + kBucketsSecond + 4), at(app, kBucketOrdered));
    if (cpu.terminate)
        return std::nullopt;
    guest::direct<0x4c79a0, vertices::gatherBucketsCore>(app, cpu, 0, gathered);
    if (cpu.terminate)
        return std::nullopt;
    guest::direct<0x4c7750, frontend::rende::clearBuckets>(app, cpu, 0);
    if (cpu.terminate || !emptyPairTo(app, cpu, frame - 0x44, buckets))
        return std::nullopt;
    // The other buckets emptied: their frame's empty list copied in.
    static const reg32 kEmptied[][2] = {{0x3c, 8}, {0x14, 0x18}, {0x2c, 0x10}, {0x24, 0x20}};
    for (const auto& bucket : kEmptied)
        if (!emptyPairTo(app, cpu, frame - bucket[0], buckets + bucket[1]))
            return std::nullopt;
    if (!emptyPairTo(app, cpu, frame - 0x1c, buckets + kBucketsOrdered))
        return std::nullopt;
    // The view drawn, the gathered list's pair in edx.
    const reg32 drawn = guest::direct<0x4bc060, frontend::rendd::drawListAt>(app, cpu, view, gathered, 0);
    if (cpu.terminate)
        return std::nullopt;
    return drawn;
}

/* 0x41c1a0: drawBuckets of the view eax, the lists' tails edx and heads ebx,
 * into the buckets at ecx; the registers as the drawing leaves them. */
x86::reg32 flushBuckets(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEsi | kSaveEdi);
    const std::optional<reg32> drawn = drawBuckets(app, cpu, args.eax, args.edx, args.ebx, args.ecx);
    if (!drawn)
        return args.eax;
    args.eax = *drawn;
    cpu.esp += 4;
    return args.eax;
}

// A car's flags (+0x200: 0x40 hidden, 0x20 its first light on), wrecked (+0x88), its lights on (+0x89),
// its place (+0x98); its two lights (+0x734, +0x660), each with its turn at +0x28 and its place at +0x58.
constexpr reg32 kCarFlags = 0x200, kCarWrecked = 0x88, kCarLightsOn = 0x89, kCarPlace = 0x98;
constexpr reg8 kCarHidden = 0x40, kCarFirstLightOn = 0x20;
constexpr reg32 kFirstLight = 0x734, kSecondLight = 0x660, kLightTurn = 0x28, kLightAt = 0x58;
// The squares of the distances the headlights reach objects and blocks within (doubles), and the second
// light's dimming for each car's lit before (a float).
constexpr reg32 kObjectsLitWithin = 0x536cb4, kBlocksLitWithin = 0x536cbc, kDimmedPerCar = 0x536cc4;
// A block's vertices (+0x70), its centre (+0x78) and its box of four corners (+0x84); the view's reach of
// a block (+0x34).
constexpr reg32 kBlockVertices = 0x70, kBlockCentre = 0x78, kBlockBox = 0x84, kViewReach = 0x34;
// A block's animated objects (+0x5a4): 20 bytes each, +0xe its state's number (a word), +0x13 how it
// moves (2 when hit, 3 it falls); a state's bytes: its path (0xff still, 0xfe turning), its angle, 0.
constexpr reg32 kBlockAnimated = 0x5a4, kAnimatedSize = 20, kAnimatedState = 0xe, kAnimatedMoves = 0x13;
constexpr reg8 kMovesWhenHit = 2, kFalls = 3, kStateTurning = 0xfe;
// The lists of a view's polygons, six of them (a word each, heads and tails apart): the sixth the ordered
// one, the headlights' polygons'.
constexpr reg32 kOrderedList = 5 * 4;

/* lightBehind (pointBehindView's) from a core: its return address's slot, as
 * the code's frame is handed on.  nullopt when the game stopped. */
std::optional<bool> lightAway(App* app, CPU& cpu, reg32 light, reg32 camera)
{
    const GuestFrame slot(cpu, 4, 0);
    return lightBehind(app, cpu, light, camera);
}

/* boxOutOfCone (litBoxOutside's) of the camera, the box and the light's turn
 * and place: the box out of the light's cone.  nullopt when the game
 * stopped. */
std::optional<bool> boxUnlit(App* app, CPU& cpu, reg32 camera, reg32 box, reg32 light)
{
    return boxOutOfCone(app, cpu, camera, box, light + kLightTurn, light + kLightAt);
}

/* vertices::boxOutside (0x41bf30) of the camera, the box and the light's turn
 * and place.  nullopt when the game stopped. */
std::optional<bool> boxOutsideLight(App* app, CPU& cpu, reg32 camera, reg32 box, reg32 light)
{
    const reg32 outside = guest::direct<0x41bf30, vertices::boxOutsideCore>(app, cpu, camera, box, light + kLightTurn,
                                                                           light + kLightAt);
    if (cpu.terminate)
        return std::nullopt;
    return outside != 0;
}

/* vertices::trackVertices (0x41a550): `count` vertices at `vertices` lit by
 * the light into the records at `records` (0x1c bytes each), their screen's
 * records at `screen`.  False when the game stopped. */
bool lightVertices(App* app, CPU& cpu, reg32 light, reg32 count, reg32 vertices, reg32 records, reg32 screen)
{
    const GuestFrame words(cpu, 4, 0);
    guest::direct<0x41a550, vertices::trackVerticesCore>(app, cpu, light, count, vertices, records, screen);
    return !cpu.terminate;
}

/* An object's points headlightsOnObjects lights: `count` of them at `points`
 * (12 bytes each), taken into view by the matrix at `matrix` and the point at
 * `point`, their screen's records at `vertices` (0x20 bytes each). */
struct LitPoints
{
    reg32 count, matrix, point, points, vertices;
};

/* 0x41c310's for one light: each point taken into view (transformPoints) into
 * `inView`, lit (trackVertices) into the record at `lit`, and its screen
 * record's colour bytes raised to the record's where the light reaches it
 * (its +4, +6 words 0): the first light's both colours, the second's first.
 * False when the game stopped. */
bool lightPoints(App* app, CPU& cpu, const LitPoints& points, reg32 light, reg32 lit, reg32 inView, bool first)
{
    for (reg32 n = 0;; ++n)
    {
        safepoint(app, cpu);
        if (sreg32(n) >= sreg32(points.count))
            return true;
        guest::direct<0x4e0430, vertices::transformPointList>(app, cpu, 1, points.points + n * kVertexSize,
                                                              points.matrix, points.point, inView);
        if (cpu.terminate || !lightVertices(app, cpu, light, 1, inView, lit, points.vertices + (n << 5)))
            return false;
        if ((at(app, lit + 4) & 0xffff) == 0)
            for (const reg32 k : {2u, 1u, 0u})
                raiseColour(app, lit, 0x14 + k, 0x10 + k);
        if (first && half(app, lit + 6) == 0)
            for (const reg32 k : {2u, 1u, 0u})
                raiseColour(app, lit, 0x18 + k, 0x10 + k);
    }
}

/* 0x41c310's: the headlights' light on an object's points in the camera's
 * view, the object of block `block`: for each car near enough (its place
 * within the square root of 0x536cb4 of the point; not a car that is hidden,
 * wrecked or without lights), each of its two lights (+0x734, where +0x200
 * has bit 0x20; +0x660) that faces the points and whose cone reaches the
 * block's box, the shading's distances its own and its points lit
 * (lightPoints).  A matrix or a point of 0 is the game's own (the code's data
 * at 0x41a880 and 0x41a8a4, copied into the frame).  None in the mirror's
 * pass, nor outside a split-screen race -- the port's change
 * (the port's mirror detail change): the mirror's asked through
 * nfs3hp::mirrorAsMain, as the main view would have it.  The frame as deep as
 * the code's (its places are handed on).  The cars gone through, or the
 * camera where no headlights shine, as the code leaves it in eax; nullopt
 * when the game stopped. */
std::optional<reg32> headlightsOnPoints(App* app, CPU& cpu, LitPoints points, reg32 camera, reg32 block)
{
    const GuestFrame frame(cpu, 0xc, 0xa0);
    const reg32 ownMatrix = frame - 0xa0, firstLit = frame - 0x7c, secondLit = frame - 0x60;
    const reg32 firstInView = frame - 0x38, secondInView = frame - 0x2c, ownPoint = frame - 0x20;
    for (reg32 word = 0; word < 9; ++word)
        put(app, ownMatrix + 4 * word, at(app, kHeadlightsBox + 4 * word));
    for (reg32 word = 0; word < 3; ++word)
        put(app, ownPoint + 4 * word, at(app, kHeadlightsPoint + 4 * word));
    if (mirrorAsMain(at(app, camera + kCameraKind)) == kMirrorPass || gameSettings(app).night == 0)
        return camera;
    if (points.matrix == 0)
        points.matrix = ownMatrix;
    if (points.point == 0)
        points.point = ownPoint;
    reg32 car = 0;
    for (;; ++car)
    {
        safepoint(app, cpu);
        if (sreg32(car) >= sreg32(at(app, kCarCount)))
            break;
        const reg32 carAt = at(app, kCars + car * 4);
        if ((byte(app, carAt + kCarFlags) & kCarHidden) != 0 || byte(app, carAt + kCarWrecked) != 0
            || byte(app, carAt + kCarLightsOn) == 0)
            continue;
        const Float dz = fp::sub(f32(app, carAt + kCarPlace + 8), f32(app, points.point + 8));
        const Float dx = fp::sub(f32(app, carAt + kCarPlace), f32(app, points.point));
        const Float squared = fp::add(fp::mul(dz, dz), fp::mul(dx, dx));
        // An x87 compare: a NaN is near.
        if (squared > f64(app, kObjectsLitWithin))
            continue;
        const reg32 firstLight = carAt + kFirstLight;
        if ((byte(app, carAt + kCarFlags) & kCarFirstLightOn) != 0 && at(app, firstLight) != 0)
        {
            const std::optional<bool> behind = lightAway(app, cpu, firstLight, camera);
            if (!behind)
                return std::nullopt;
            if (!*behind)
            {
                const std::optional<bool> unlit = boxUnlit(app, cpu, camera, block + kBlockBox, firstLight);
                if (!unlit)
                    return std::nullopt;
                if (!*unlit)
                {
                    shadeDistances(app, firstLight, 1.0f);
                    if (!lightPoints(app, cpu, points, firstLight, firstLit, firstInView, true))
                        return std::nullopt;
                }
            }
        }
        const reg32 secondLight = carAt + kSecondLight;
        if (at(app, secondLight) == 0)
            continue;
        const std::optional<bool> behind = lightAway(app, cpu, secondLight, camera);
        if (!behind)
            return std::nullopt;
        if (*behind)
            continue;
        const std::optional<bool> outside = boxOutsideLight(app, cpu, camera, block + kBlockBox, secondLight);
        if (!outside)
            return std::nullopt;
        if (*outside)
            continue;
        shadeDistances(app, secondLight, 1.0f);
        if (!lightPoints(app, cpu, points, secondLight, secondLit, secondInView, false))
            return std::nullopt;
    }
    return car;
}

/* 0x41c310 (stdcall, 3 words): headlightsOnPoints of the eax points at ecx,
 * their screen's records [esp+4], by the matrix edx and the point ebx, for
 * the camera [esp+8] and the block [esp+0xc]. */
x86::reg32 headlightsOnObjects(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEsi | kSaveEdi);
    const LitPoints points = {args.eax, args.edx, args.ebx, args.ecx, at(app, cpu.esp + 4)};
    const std::optional<reg32> cars = headlightsOnPoints(app, cpu, points, at(app, cpu.esp + 8), at(app, cpu.esp + 0xc));
    if (!cars)
        return args.eax;
    args.eax = *cars;
    cpu.esp += 4 + 0xc;
    return args.eax;
}

/* A block's polygon group drawn lit (0x41ae90): group `group` of block
 * `index` for the camera by the light, its corners the lit records at `lit`,
 * into the ordered list (of `tails` and `heads`); the polygons drawn.
 * nullopt when the game stopped. */
std::optional<reg32> drawLitGroup(App* app, CPU& cpu, reg32 camera, reg32 light, reg32 index, reg32 group, reg32 lit,
                                  reg32 tails, reg32 heads)
{
    const GuestFrame words(cpu, 3 * 4, 0);
    const reg32 drawn = guest::direct<0x41ae90, vertices::trackPolygonsCore>(app, cpu, camera, index, group, lit,
                                                                            tails + kOrderedList, heads + kOrderedList);
    if (cpu.terminate)
        return std::nullopt;
    return drawn;
}

/* 0x41c670's: the headlights' light on block `index`'s road (`block` its
 * record) for the camera: room for `count` lit records taken from
 * Render_GetTm; for each car near the block's centre (within the square root
 * of 0x536cbc; not hidden, wrecked or without lights) and each of its lights
 * facing the camera whose cone reaches the block: the block's vertices lit
 * for it (setShadeDistances, trackVertices; their screen's records at
 * `screen`) -- the second light dimmer for each car's lit before -- and its
 * polygon groups `model`, +1, 7, 8, 9 (and 6 for model 4): the first light's
 * raising the corners' colours (lightPolygons), the second's drawn
 * (drawLitGroup) into the ordered list of `tails` and `heads`.  None in the
 * mirror's pass (its kind, not asked through mirrorAsMain), nor outside a
 * split-screen race.  The frame as deep as the code's (the room's place is
 * handed on).  The polygons drawn; nullopt when the game stopped. */
std::optional<reg32> headlightsOnRoad(App* app, CPU& cpu, reg32 camera, reg32 count, reg32 index, reg32 screen,
                                      reg32 tails, reg32 heads, reg32 block, reg32 model)
{
    const GuestFrame frame(cpu, 0xc, 0x40);
    const reg32 litAt = frame - 0x30;
    const reg32 kind = at(app, camera + kCameraKind);
    reg32 drawn = 0;
    if (kind == kMirrorPass || gameSettings(app).night == 0)
        return drawn;
    if (!arenaRoom(app, cpu, litAt, count * kLitCornerSize))
        return std::nullopt;
    const reg32 lit = at(app, litAt);
    if (lit == 0)
        return drawn;
    const reg32 centre = kBlocks + blockOffset(index) + kBlockCentre;
    reg32 lightsLit = 0;
    for (reg32 car = 0;; ++car)
    {
        safepoint(app, cpu);
        if (sreg32(car) >= sreg32(at(app, kCarCount)))
            break;
        const reg32 carAt = at(app, kCars + car * 4);
        const reg8 flags = byte(app, carAt + kCarFlags);
        if ((flags & kCarHidden) != 0 || byte(app, carAt + kCarWrecked) != 0 || byte(app, carAt + kCarLightsOn) == 0)
            continue;
        const Float dz = fp::sub(f32(app, carAt + kCarPlace + 8), f32(app, centre + 8));
        const Float dx = fp::sub(f32(app, carAt + kCarPlace), f32(app, centre));
        const Float squared = fp::add(fp::mul(dz, dz), fp::mul(dx, dx));
        // An x87 compare: a NaN is near.
        if (squared > f64(app, kBlocksLitWithin))
            continue;
        const reg32 firstLight = carAt + kFirstLight;
        if ((byte(app, carAt + kCarFlags) & kCarFirstLightOn) != 0 && at(app, firstLight) != 0)
        {
            const std::optional<bool> behind = lightAway(app, cpu, firstLight, camera);
            if (!behind)
                return std::nullopt;
            std::optional<bool> unlit = true;
            if (!*behind)
                unlit = boxUnlit(app, cpu, camera, block + kBlockBox, firstLight);
            if (!unlit)
                return std::nullopt;
            if (!*unlit)
            {
                shadeDistances(app, firstLight, 1.0f);
                if (!lightVertices(app, cpu, firstLight, count, at(app, block + kBlockVertices), lit, screen))
                    return std::nullopt;
                for (const reg32 group : {model, model + 1, 7u, 8u, 9u})
                    lightPolygons(app, cpu, index, group, lit);
            }
        }
        const reg32 secondLight = carAt + kSecondLight;
        if (at(app, secondLight) == 0)
            continue;
        const std::optional<bool> behind = lightAway(app, cpu, secondLight, camera);
        if (!behind)
            return std::nullopt;
        if (*behind)
            continue;
        const std::optional<bool> outside = boxOutsideLight(app, cpu, camera, block + kBlockBox, secondLight);
        if (!outside)
            return std::nullopt;
        if (*outside)
            continue;
        const Float share = fp::div(Float(sreg32(lightsLit)), Float(sreg32(at(app, kCarCount))));
        const Float scaled = fp::mul(share, f32(app, kDimmedPerCar));
        shadeDistances(app, secondLight, float(fp::sub(Float(1.0), scaled)));
        if (!lightVertices(app, cpu, secondLight, count, at(app, block + kBlockVertices), lit, screen))
            return std::nullopt;
        for (const reg32 group : {model, model + 1, 7u, 8u, 9u})
        {
            const std::optional<reg32> groupDrawn = drawLitGroup(app, cpu, camera, secondLight, index, group, lit, tails, heads);
            if (!groupDrawn)
                return std::nullopt;
            drawn += *groupDrawn;
        }
        if (model == 4)
        {
            const std::optional<reg32> groupDrawn = drawLitGroup(app, cpu, camera, secondLight, index, 6, lit, tails, heads);
            if (!groupDrawn)
                return std::nullopt;
            drawn += *groupDrawn;
        }
        ++lightsLit;
    }
    return drawn;
}

/* 0x41c670 (stdcall, 4 words): headlightsOnRoad for the camera eax, edx lit
 * records, block ebx and its screen's records ecx, the lists' tails [esp+4]
 * and heads [esp+8], the block [esp+0xc] and the model [esp+0x10]. */
x86::reg32 headlightsOnBlock(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEsi | kSaveEdi);
    const std::optional<reg32> drawn =
        headlightsOnRoad(app, cpu, args.eax, args.edx, args.ebx, args.ecx, at(app, cpu.esp + 4), at(app, cpu.esp + 8),
                         at(app, cpu.esp + 0xc), at(app, cpu.esp + 0x10));
    if (!drawn)
        return args.eax;
    args.eax = *drawn;
    cpu.esp += 4 + 0x10;
    return args.eax;
}

// A block's lights: their count (+0x5b8) and records (+0x5bc), 0x10 bytes each -- three words and a half-word.
constexpr reg32 kBlockLightCount = 0x5b8, kBlockLights = 0x5bc, kBlockLightSize = 0x10;
constexpr reg32 kGlow = 0x491190;  // render::glow (stdcall, 4 words): a light glowing for the camera eax

/* 0x41c9f0's: block `index`'s lights glowing for the camera.  What the code
 * leaves in eax: the last glow's (the block's offset where there is no
 * light); nullopt when the game stopped. */
std::optional<reg32> glowBlockLights(App* app, CPU& cpu, reg32 camera, reg32 index)
{
    const reg32 block = kBlocks + blockOffset(index);
    reg32 left = blockOffset(index);
    reg32 light = at(app, block + kBlockLights);
    for (reg32 remaining = at(app, block + kBlockLightCount);; light += kBlockLightSize)
    {
        safepoint(app, cpu);
        if (remaining-- == 0)
            break;
        const std::optional<reg32> glowed = callWith(
            app, cpu, kGlow, {camera}, {at(app, light), at(app, light + 4), at(app, light + 8), half(app, light + 0xc)});
        if (!glowed)
            return std::nullopt;
        left = *glowed;
    }
    return left;
}

/* 0x41c9f0: glowBlockLights for the camera eax and block edx. */
x86::reg32 blockLights(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx | kSaveEsi | kSaveEdi);
    const std::optional<reg32> left = glowBlockLights(app, cpu, args.eax, args.edx);
    if (!left)
        return args.eax;
    args.eax = *left;
    cpu.esp += 4;
    return args.eax;
}

// An object's kind (+0: 4 animated, 3 a car's, else fixed), its number among its block's animated ones
// (+4), the angle its turn takes (+8), its place (+0xc) and its car's (+0x1c).
constexpr reg32 kObjectKind = 0, kObjectNumber = 4, kObjectAngle = 8, kObjectPlace = 0xc, kObjectCarAt = 0x1c;
constexpr reg32 kAnimatedKind = 4, kCarKind = 3;
// A screen's record, of a vertex or a polygon: 0x20 bytes.
constexpr reg32 kRecordSize = 0x20;
constexpr reg32 kBlockListsEnd = 0x536cc8;  // a float: the world's end handed with a block's lists (-1)
constexpr reg32 kPoseTime = 0x7d3684;       // what an animated car's pose is taken at

/* A byte sign-extended, as movsx does. */
reg32 signedByte(reg8 value)
{
    return reg32(sreg32(std::int8_t(value)));
}

/* The code's cdq; idiv: `n` divided by `by`, toward 0 (-1's overflow as x86
 * leaves it). */
reg32 quotient(reg32 n, reg32 by)
{
    return by == 0xffffffff ? 0u - n : reg32(i32(n) / i32(by));
}

/* 0x41ca40's: the objects of the list at `list` (block `index`'s, or the
 * track's) drawn for the camera into the list whose last record's slot is at
 * `tail` (its first's at `head` while it is empty) -- an animated one (kind
 * 4) at its state's place and turn (the states at 0x5dd230: 0xff still, 0xfe
 * turning, else a path's step: one past the path's end stops it, turning
 * from now on, and the same object is gone through again), a car's (kind 3,
 * where `cars`) posed as the car is and drawn only where its block is in view
 * (but for a list with the world's end of a block's, -1), any other where it
 * stands: its vertices into view and lit (headlightsOnPoints), its polygons
 * as records (objectRecords) appended to the list.  The frame as deep as the
 * code's (its places are handed on).  The polygons drawn; nullopt when the
 * game stopped. */
std::optional<reg32> drawObjects(App* app, CPU& cpu, reg32 camera, reg32 tail, reg32 head, reg32 index,
                                 reg32 worldEnd, reg32 list, reg32 cars)
{
    const GuestFrame frame(cpu, 0xc, 0x68);
    const reg32 turn = frame - 0x68, place = frame - 0x44, lastAt = frame - 0x38, records = frame - 0x34;
    const reg32 vertices = frame - 0x30, steps = frame - 0x2c, length = frame - 0x28;
    const reg32 block = kBlocks + blockOffset(index);
    put(app, lastAt, at(app, tail));
    reg32 drawn = 0;
    reg32 object = at(app, list + kListObjects);
    const auto field = [&](reg32 offset) { return at(app, object + offset); };
    // Render_GetTm's room or next (`target`): `bytes` at [place].
    const auto room = [&](reg32 where, reg32 bytes) { return arenaRoom(app, cpu, where, bytes); };
    const auto roomNext = [&](reg32 where) { return arenaNext(app, cpu, where); };
    // The object's vertices into view, turned by the matrix `matrix` and moved to the point `point`.
    const auto turnedIntoView = [&](reg32 matrix, reg32 point) {
        const GuestFrame slot(cpu, 4 + 0xc, 0);
        return viewTurnedVertices(app, cpu, camera, matrix, point, field(kObjectVertexCount), field(kObjectVertices),
                                  field(kObjectColours), at(app, vertices));
    };
    // The headlights on the object's vertices, by the matrix `matrix` and the point `point`.
    const auto lit = [&](reg32 matrix, reg32 point) {
        const GuestFrame slot(cpu, 4 + 0xc, 0);
        const LitPoints points = {field(kObjectVertexCount), matrix, point, field(kObjectVertices), at(app, vertices)};
        return headlightsOnPoints(app, cpu, points, camera, block).has_value();
    };
    for (reg32 remaining = at(app, list + kListCount);;)
    {
        safepoint(app, cpu);
        if (remaining-- == 0)
            break;
        const reg32 kind = field(kObjectKind);
        if (kind == kAnimatedKind)
        {
            if (!room(vertices, field(kObjectVertexCount) << 5))
                return std::nullopt;
            const reg32 animated = at(app, block + kBlockAnimated) + field(kObjectNumber) * kAnimatedSize;
            const reg32 state = kObjectStates + half(app, animated + kAnimatedState) * kObjectStateSize;
            const reg32 objectPlace = object + kObjectPlace;
            const reg8 now = byte(app, state);
            if (now == kStateStill)
            {
                {
                    const GuestFrame slot(cpu, 4 + 8, 0);
                    if (!viewVertices(app, cpu, camera, objectPlace, field(kObjectVertexCount),
                                      field(kObjectVertices), field(kObjectColours), at(app, vertices)))
                        return std::nullopt;
                }
                if (!lit(0, objectPlace))
                    return std::nullopt;
            }
            else if (now == kStateTurning)
            {
                putByte(app, state + 2, byte(app, object + kObjectAngle));
                {
                    const GuestFrame slot(cpu, 4, 0);
                    if (!turnOfPitchYaw(app, cpu, signedByte(byte(app, state + 2)) << 8,
                                        signedByte(byte(app, state + 1)) << 8, turn))
                        return std::nullopt;
                }
                if (!turnedIntoView(turn, objectPlace) || !lit(turn, objectPlace))
                    return std::nullopt;
            }
            else
            {
                const std::optional<reg32> stepped =
                    guest::direct<0x40f940, aiplan::stepAnimation>(app, cpu, signedByte(now), place, turn);
                if (!stepped)
                    return std::nullopt;
                if (*stepped != 1)
                {
                    guest::direct<0x40f460, aiplan::endAnimation>(app, cpu, signedByte(byte(app, state)));
                    if (cpu.terminate)
                        return std::nullopt;
                    putByte(app, state, kStateTurning);
                    continue;  // the same object again: the code does not move on
                }
                putByte(app, state + 2, byte(app, object + kObjectAngle));
                guest::direct<0x4dffb0, vertices::offsetVectorList<true>>(app, cpu, 1, place, objectPlace, place);
                if (cpu.terminate)
                    return std::nullopt;
                guest::direct<0x40f800, aiplan::animationSpans>(app, cpu, signedByte(byte(app, state)), steps, length);
                if (cpu.terminate)
                    return std::nullopt;
                const reg32 height = signedByte(byte(app, state + 2)) << 8;
                const reg32 along = quotient(at(app, steps) << 16, at(app, length));
                {
                    const GuestFrame slot(cpu, 4, 0);
                    if (!turnOfAngles(app, cpu, along, signedByte(byte(app, state + 1)) << 8, height, turn))
                        return std::nullopt;
                }
                if (!turnedIntoView(turn, place) || !lit(turn, place))
                    return std::nullopt;
            }
        }
        else if (kind == kCarKind)
        {
            if (cars == 0)
            {
                object += kObjectSize;
                continue;
            }
            if (!guest::direct<0x40f4b0, aiplan::poseAt>(app, cpu, field(kObjectCarAt), 1, i32(at(app, kPoseTime)), place,
                                                         turn))
                return std::nullopt;
            if (!room(vertices, field(kObjectVertexCount) << 5) || !turnedIntoView(turn, place)
                || !lit(turn, place))
                return std::nullopt;
            // An x87 compare: ZF for equal or unordered -- the car's block looked up only for a list drawn to
            // an end other than a block's.
            const Float end = floatOf(worldEnd), blocksEnd = f32(app, kBlockListsEnd);
            if (end < blocksEnd || end > blocksEnd)
            {
                const reg32 nearest = guest::direct<0x41e0f0, vertices::nearestBlockTo>(app, cpu, place);
                if (cpu.terminate)
                    return std::nullopt;
                const std::optional<bool> inView = blockDrawn(app, cpu, camera, reg32(sreg32(nearest) >> 3));
                if (!inView)
                    return std::nullopt;
                if (!*inView)
                {
                    object += kObjectSize;
                    continue;
                }
            }
        }
        else
        {
            if (!room(vertices, field(kObjectVertexCount) << 5))
                return std::nullopt;
            guest::direct<0x41a970, vertices::colouredVerticesOf>(app, cpu, camera, field(kObjectVertexCount),
                                                                  field(kObjectVertices), field(kObjectColours),
                                                                  at(app, vertices));
            if (cpu.terminate)
                return std::nullopt;
            if (!lit(0, 0))
                return std::nullopt;
        }
        // Its polygons as records, appended to the list.
        const std::optional<reg32> roomFor = roomNext(records);
        if (!roomFor)
            return std::nullopt;
        if (*roomFor == 0)
        {
            object += kObjectSize;
            continue;
        }
        reg32 made;
        {
            const GuestFrame words(cpu, 4, 0);
            made = guest::direct<0x41b640, vertices::objectRecordsCore>(app, cpu, sreg32(field(kObjectPolygonCount)),
                                                                       field(kObjectPolygons), at(app, records),
                                                                       at(app, vertices));
        }
        if (cpu.terminate)
            return std::nullopt;
        if (made != 0)
        {
            if (!room(records, made << 5))
                return std::nullopt;
            const reg32 last = at(app, lastAt);
            put(app, last == 0 ? head : last, at(app, records));
            if (!roomNext(lastAt))
                return std::nullopt;
            put(app, lastAt, at(app, lastAt) - kRecordSize);
        }
        drawn += field(kObjectPolygonCount);
        object += kObjectSize;
    }
    put(app, tail, at(app, lastAt));
    return drawn;
}

/* 0x41ca40 (stdcall, 3 words): drawObjects for the camera eax, the list
 * [esp+8] of block ecx drawn to the world's end [esp+4] (a float, -1 for a
 * block's), into the list of the tail's slot edx and the head's ebx, cars
 * where [esp+0xc]. */
x86::reg32 drawObjectList(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEsi | kSaveEdi);
    const std::optional<reg32> drawn = drawObjects(app, cpu, args.eax, args.edx, args.ebx, args.ecx, at(app, cpu.esp + 4),
                                                   at(app, cpu.esp + 8), at(app, cpu.esp + 0xc));
    if (!drawn)
        return args.eax;
    args.eax = *drawn;
    cpu.esp += 4 + 0xc;
    return args.eax;
}

constexpr reg32 kPassLayerNothing = 0x431c20;  // collide::passLayerNothing: eax for the layer edx
// A block's models' vertex counts (+0x5c, a word each), its lists of objects (+0x564, 8 bytes each); a
// model's record (12 bytes): the block's word of it, the model, the most polygons before a flush.
constexpr reg32 kBlockModelVertices = 0x5c, kBlockObjectLists = 0x564, kModelSize = 12, kModelOf = 4,
                kModelMostPolygons = 8;
constexpr reg32 kNoBlock = 0xffffffff, kNoModel = 0xffffffff;
constexpr reg32 kMinusOne = 0xbf800000;  // -1 as a float: the world's end a block's lists are drawn to

/* 0x41cdc0's: the track drawn for the camera, its polygons into the buckets
 * at `buckets`: each block of the list at `list` (the view's, up to the last
 * block: a word of the block, -1 none, and a word of its model, -1 by its
 * distance) within the view's reach (an x87 compare, a NaN out of it) and in
 * view -- its model (the mirror's always the coarsest, the port's change
 * through nfs3hp::mirrorAsMain; the next block's asked for too and not
 * taken), its vertices into view (objectVerticesColoured), its polygon groups
 * (trackRecords, models as the model's record at 0x552e20 says, a tenth
 * outside split screen and network races), the headlights on it
 * (headlightsOnRoad), its lights (glowBlockLights) and its object lists
 * (drawObjects) into the six lists -- the buckets flushed (drawBuckets)
 * whenever the polygons pass the model's most.  Then the track's own objects,
 * where it has lights ([0x5dd840]); the six lists into the buckets; the
 * collisions' two layers handed 0 (0x431c20, the frame's two words the code
 * never sets past their 0).  The frame as deep as the code's (the lists are
 * handed on); eax as the last of those calls leaves it.  False when the
 * game stopped. */
bool drawTrackBlocks(App* app, CPU& cpu, reg32 camera, reg32 list, reg32 buckets)
{
    const GuestFrame frame(cpu, 0x10, 0x70);
    const reg32 heads = frame - 0x70, tails = frame - 0x58, empty = frame - 0x40, vertices = frame - 0x38;
    // A block's group of polygons into the lists `k` bytes in.
    const auto groupInto = [&](reg32 index, reg32 group, reg32 screen, reg32 k) {
        const GuestFrame words(cpu, 2 * 4, 0);
        const reg32 made = guest::direct<0x41b300, vertices::trackRecordsCore>(app, cpu, screen, index, group, tails + k,
                                                                              heads + k);
        return cpu.terminate ? std::optional<reg32>() : std::optional<reg32>(made);
    };
    // A list of objects into the lists `k` bytes in; the polygons drawn.
    const auto objectsInto = [&](reg32 index, reg32 worldEnd, reg32 objects, reg32 cars, reg32 k) {
        const GuestFrame slot(cpu, 4 + 0xc, 0);
        return drawObjects(app, cpu, camera, tails + k, heads + k, index, worldEnd, objects, cars);
    };
    for (reg32 k = 0; k < kTrackLists * 4; k += 4)
    {
        safepoint(app, cpu);
        put(app, heads + k, 0);
        put(app, tails + k, 0);
    }
    reg32 drawn = 0;
    for (reg32 entry = 0;; ++entry)
    {
        safepoint(app, cpu);
        if (sreg32(entry) > sreg32(at(app, kLastBlock)))
            break;
        const reg32 slotAt = list + entry * 4;
        const reg32 index = reg32(sreg32(std::int16_t(half(app, slotAt))));
        if (index == kNoBlock)
            continue;
        // Within the view's reach (the square stored as a float).
        const reg32 block = kBlocks + blockOffset(index);
        const Float dz = fp::sub(f32(app, camera + kCameraPlace + 8), f32(app, block + kBlockCentre + 8));
        const Float dx = fp::sub(f32(app, camera + kCameraPlace), f32(app, block + kBlockCentre));
        const reg32 view = at(app, kView);
        const Float squared = fp::add(fp::mul(dz, dz), fp::mul(dx, dx));
        const Float reach = fp::mul(f32(app, view + kViewReach), f32(app, view + kViewReach));
        const float distance = float(squared);
        if (!(reach >= Float(distance)))
            continue;
        // In view.
        const reg32 outside = guest::direct<0x41bf30, vertices::boxOutsideCore>(
            app, cpu, camera, block + kBlockBox, camera + kCameraMatrix, camera + kCameraBox);
        if (cpu.terminate)
            return false;
        if (outside != 0)
            continue;
        // Its model: by its own distance where the list has none.
        reg32 level = reg32(sreg32(at(app, slotAt)) >> 16);
        if (level == kNoModel)
            level = modelAt(app, camera, distance);
        if (mirrorAsMain(at(app, camera + kCameraKind)) == kMirrorPass)
            level = 0;
        const reg32 record = kModels + level * kModelSize, model = at(app, record + kModelOf);
        reg32 nextIndex = index + 1;
        if (sreg32(nextIndex) > sreg32(at(app, kLastBlock)))
            nextIndex = 0;
        const reg32 next = kBlocks + blockOffset(nextIndex) + kBlockCentre;
        const Float nz = fp::sub(f32(app, camera + kCameraPlace + 8), f32(app, next + 8));
        const Float nx = fp::sub(f32(app, camera + kCameraPlace), f32(app, next));
        modelAt(app, camera, float(fp::add(fp::mul(nz, nz), fp::mul(nx, nx))));  // asked for, not taken
        // Its vertices into view.
        const reg32 count = at(app, block + kBlockModelVertices + at(app, record) * 4);
        if (!arenaRoom(app, cpu, vertices, count << 5))
            return false;
        const reg32 screen = at(app, vertices);
        if (screen == 0)
            continue;
        guest::direct<0x41a970, vertices::colouredVerticesOf>(app, cpu, camera, count, at(app, block + kBlockVertices),
                                                              at(app, block + kBlockColours), screen);
        if (cpu.terminate)
            return false;
        // Its polygon groups: the model's, the next, 7, 8, 9, into the lists 0, 1, 1, 3, 2.
        const reg32 groups[][2] = {{model, 0}, {model + 1, 4}, {7, 4}, {8, 0xc}, {9, 8}};
        for (const auto& group : groups)
        {
            const std::optional<reg32> added = groupInto(index, group[0], screen, group[1]);
            if (!added)
                return false;
            drawn += *added;
        }
        if (gameSettings(app).night == 0 && gameSettings(app).weather == 0)
        {
            const std::optional<reg32> added = groupInto(index, 0xa, screen, 0);
            if (!added)
                return false;
            drawn += *added;
        }
        if (model == 4)
        {
            const std::optional<reg32> added = groupInto(index, 6, screen, 0x10);
            if (!added)
                return false;
            drawn += *added;
        }
        // The headlights on it, its lights.
        {
            const GuestFrame slot(cpu, 4 + 0x10, 0);
            const std::optional<reg32> lit = headlightsOnRoad(app, cpu, camera, count, index, screen, tails, heads, block, model);
            if (!lit)
                return false;
            drawn += *lit;
        }
        if (at(app, block + kBlockLightCount) != 0 && !glowBlockLights(app, cpu, camera, index))
            return false;
        // Its object lists: three into the lists 1, 3, 2; a fourth into 0 outside split screen and
        // network races.
        const reg32 objectLists[][2] = {{0, 4}, {8, 0xc}, {0x10, 8}};
        for (const auto& objects : objectLists)
        {
            const std::optional<reg32> added =
                objectsInto(index, kMinusOne, block + kBlockObjectLists + objects[0], 1, objects[1]);
            if (!added)
                return false;
            drawn += *added;
        }
        if (gameSettings(app).night == 0 && gameSettings(app).weather == 0)
        {
            const std::optional<reg32> added = objectsInto(index, kMinusOne, block + kBlockObjectLists + 0x18, 1, 0);
            if (!added)
                return false;
            drawn += *added;
        }
        // The buckets flushed past the model's most.
        if (sreg32(drawn) > sreg32(at(app, record + kModelMostPolygons)) && at(app, kWorldsDrawn) != 0)
        {
            const GuestFrame slot(cpu, 4, 0);
            if (!drawBuckets(app, cpu, camera, tails, heads, buckets))
                return false;
            drawn = 0;
        }
    }
    // The track's own objects, where it has lights.
    if (at(app, kColLights) != 0)
    {
        const reg32 cars = ambientPlays(app) ? 1 : 0;
        if (!objectsInto(0, at(app, at(app, kView) + kViewWorldEnd), kTrackObjects, cars, 8))
            return false;
    }
    // The six lists into the buckets: a list's first and last, or an empty list's.
    for (reg32 k = 0; k < kTrackLists; ++k)
    {
        safepoint(app, cpu);
        const reg32 bucket = buckets + k * 8, head = at(app, heads + k * 4);
        if (head == 0)
        {
            if (!emptyPairTo(app, cpu, empty, bucket))
                return false;
        }
        else
        {
            put(app, bucket, head);
            const reg32 tail = at(app, tails + k * 4);
            put(app, tail, 0);
            put(app, bucket + 4, at(app, tails + k * 4));
        }
    }
    guest::direct<0x431c20, collide::layerNothing>(app, cpu, 0, 1);
    if (cpu.terminate)
        return false;
    guest::direct<0x431c20, collide::layerNothing>(app, cpu, 0, 2);
    return !cpu.terminate;
}

/* 0x41cdc0: drawTrackBlocks for the camera eax, the list edx, the buckets
 * ebx. */
x86::reg32 drawTrack(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEcx | kSaveEsi | kSaveEdi);
    if (!drawTrackBlocks(app, cpu, args.eax, args.edx, args.ebx))
        return args.eax;
    args.eax = 0;  // what the last (empty) layer function leaves: the 0 it was handed
    cpu.esp += 4;
    return args.eax;
}

// A sound source: its sound (+0xc, a word) and its kind (+0xe: 1 played at a random half-second).
constexpr reg32 kSourceSound = 0xc, kSourceKind = 0xe;
constexpr reg16 kSourceRandom = 1;
constexpr reg32 kLoudest = 0x7f, kPanMiddle = 0x40, kRandomSlot = 0x13, kEffectRate = 0x10000, kPitchBelow = 0x10;

/* An int shifted right `k` towards 0, as the code's sar after the sign's
 * bias does it. */
reg32 shiftedTowardZero(reg32 value, unsigned k)
{
    const reg32 bias = i32(value) < 0 ? (1u << k) - 1 : 0;
    return reg32(i32(value + bias) >> k);
}

/* 0x41d210's: the sound source `source` heard from the camera: its distance
 * and side from the camera's place (into the frame, as deep as the code's)
 * made a volume (0x10000 over the distance's square, at most 0x7f) and a
 * pan; a source of kind 1 played in the ambient slot 0x13 at a random
 * half-second of its own (the frame counter's eighths against [0x552e44],
 * drawn anew), else, the first heard (`heard` 0), in slot 0x12 -- at
 * [the view's +0] less 0x10.  1 where this one played the ambient slot, else
 * `heard` as it came; nullopt when the game stopped. */
std::optional<reg32> hearSource(App* app, CPU& cpu, reg32 camera, reg32 source, reg32 heard)
{
    const GuestFrame frame(cpu, 0x10, 8);
    const reg32 across = frame - 8, side = frame - 4;
    const port::Result distance = guest::direct<0x412620, port::placeAcrossOf>(app, cpu, cpu.esp - 4,
                                                                                camera + kCameraPlace, source, across);
    if (cpu.terminate || !distance)
        return std::nullopt;
    const reg32 far = reg32(i32(*distance) >> 0xc), squared = reg32(i32(far * far) >> 0xa);
    reg32 volume = kLoudest;
    if (squared != 0)
    {
        const i32 loudness = i32(0x10000) / i32(squared);
        volume = loudness >= i32(kLoudest) ? kLoudest : reg32(loudness);
    }
    const reg32 pan = guest::direct<0x4185e0, port::panFromSideOf>(
        app, cpu, i32(shiftedTowardZero(at(app, across), 0xa) + kPanMiddle), i32(at(app, side)));
    const reg16 kind = half(app, source + kSourceKind);
    reg32 slot = kAmbientSlot;
    if (kind == kSourceRandom)
    {
        const reg32 eighth = (at(app, kFrameCounter) & 0x3f) >> 3, last = at(app, kAmbientLast);
        if (eighth != last)
            return heard;
        slot = kRandomSlot;
    }
    else if (heard != 0)
        return heard;
    const reg32 pitch = at(app, at(app, kView) + kViewStart) - kPitchBelow;
    {
        const GuestFrame words(cpu, 3 * 4, 0);
        guest::direct<0x416fc0, port::slotEffectPlayed>(app, cpu, cpu.esp - 4, i32(slot),
                                                       i32(half(app, source + kSourceSound)), kEffectRate, kPanMiddle,
                                                       volume, pan, pitch);
    }
    if (cpu.terminate)
        return std::nullopt;
    if (slot == kAmbientSlot)
        return 1;
    const reg32 drawn = guest::direct<0x4e0d88, port::nextRandom>(app, cpu);
    put(app, kAmbientLast, drawn & 7);
    return heard;
}

/* 0x41d210: hearSource for the camera eax, the source edx, heard ebx. */
x86::reg32 ambientSource(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEcx | kSaveEsi | kSaveEdi);
    const std::optional<reg32> heard = hearSource(app, cpu, args.eax, args.edx, args.ebx);
    if (!heard)
        return args.eax;
    args.eax = *heard;
    cpu.esp += 4;
    return args.eax;
}

/* The heard of the sources at `sources` (hearSource), for ambientAround. */
std::optional<reg32> heardSources(App* app, CPU& cpu, reg32 camera, reg32 sources, reg32 heard)
{
    return hearSource(app, cpu, camera, sources, heard);
}

/* 0x41d310's: view `index` the one drawn ([0x5dd830]) -- in the mirror's pass
 * given the first view's block.  The view's offset, as the code leaves it in
 * eax. */
reg32 chooseView(App* app, reg32 index)
{
    const reg32 offset = index * kViewSize, view = kViews + offset;
    if (at(app, kPassKind) == kMirrorPass)
        put(app, view + kViewBlock, at(app, kViews + kViewBlock));
    put(app, kView, view);
    return offset;
}

/* 0x41d310: chooseView of eax. */
x86::reg32 selectView(App* app, x86::CPU& cpu, x86::Args& args)
{
    args.eax = chooseView(app, args.eax);
    cpu.esp += 4;
    return args.eax;
}

constexpr reg32 kSpeedToAngle = 0x536ccc;      // a double: a speed to the angle's units
// A car: its block (+0xc), its speed's x and z (+0xa4, +0xac), its flags (+0x200; 2 the threshold 0).
constexpr reg32 kCarBlock = 0xc, kCarSpeedX = 0xa4, kCarSpeedZ = 0xac;
constexpr reg8 kCarAnyThreshold = 2;
constexpr reg32 kSlowThreshold = 0xb, kFastSpeed = 0x19, kPathSlow = 2, kPathFast = 3, kPathMode = 2;

/* An x87 fabs as the code takes it here: a value at most 0 negated (0 to
 * -0), a NaN kept to the bit. */
reg32 negatedUnlessOver(reg32 bits)
{
    return Float(0.0) >= floatOf(bits) ? bitsOf(float(-floatOf(bits))) : bits;
}

/* 0x41d350's: object `index` of the car's block set moving: the car's speed
 * (|x| + |z| of its speed) against its threshold (0, or 0xb for a car
 * without flag 2).  An object that moves when hit and is still: fast enough,
 * set on a path (2, or 3 at 25 and up) -- its angle from the car's move and
 * the path started -- or turning where there is none; -1.  Too slow, 1; one
 * moving already, 0; one that falls, 2; any other 1.  nullopt when the game
 * stopped. */
std::optional<reg32> setObjectMoving(App* app, CPU& cpu, reg32 car, reg32 index)
{
    const reg32 objects = at(app, at(app, car + kCarBlock) + kBlockAnimated);
    const reg32 threshold = (byte(app, car + kCarFlags) & kCarAnyThreshold) != 0 ? 0 : kSlowThreshold;
    const reg32 absX = negatedUnlessOver(at(app, car + kCarSpeedX));
    const reg32 absZ = negatedUnlessOver(at(app, car + kCarSpeedZ));
    const reg32 speed = reg32(fp::toInteger<sreg32>(fp::add(floatOf(absX), floatOf(absZ))));
    const reg32 object = objects + index * kAnimatedSize;
    const reg32 stateNumber = half(app, object + kAnimatedState), state = kObjectStates + stateNumber * 3;
    const reg8 moves = byte(app, object + kAnimatedMoves);
    if (moves != kMovesWhenHit)
        return moves == kFalls ? 2u : 1u;
    if (byte(app, state) != kStateStill)
        return 0;
    if (sreg32(speed) < sreg32(threshold))
        return 1;
    const reg32 path =
        guest::direct<0x40f3b0, aiplan::playAnimation>(app, cpu, sreg32(speed) < sreg32(kFastSpeed) ? kPathSlow : kPathFast);
    if (cpu.terminate)
        return std::nullopt;
    const reg32 movedNumber = half(app, object + kAnimatedState);
    putByte(app, movedNumber * 3 + kObjectStates, reg8(path));
    if (std::int8_t(byte(app, state)) < 0)
    {
        putByte(app, state, kStateTurning);
        return 0xffffffff;
    }
    const reg32 z = reg32(sreg32(reg32(fp::toInteger<sreg32>(fp::mul(f32(app, car + kCarSpeedZ), f64(app, kSpeedToAngle))))) >> 8);
    const reg32 x = reg32(sreg32(reg32(fp::toInteger<sreg32>(fp::mul(f32(app, car + kCarSpeedX), f64(app, kSpeedToAngle))))) >> 8);
    const reg32 angle = guest::direct<0x4ea570, frontend::simc::angle>(app, cpu, x, z);
    if (cpu.terminate)
        return std::nullopt;
    const reg8 turn = reg8(sreg32(angle) >> 8);
    putByte(app, state + 1, turn);
    const reg32 number = half(app, object + kAnimatedState);
    guest::direct<0x40f430, aiplan::setAnimation>(app, cpu, reg32(sreg32(at(app, number * 3 + kObjectStates - 3)) >> 0x18),
                                                  kPathMode, 0, reg32(sreg32(std::int8_t(turn))));
    if (cpu.terminate)
        return std::nullopt;
    putByte(app, state + 2, 0);
    return 0xffffffff;
}

/* 0x41d350: setObjectMoving of the car eax's object edx. */
x86::reg32 objectAnimation(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx | kSaveEsi | kSaveEdi);
    const std::optional<reg32> moved = setObjectMoving(app, cpu, args.eax, args.edx);
    if (!moved)
        return args.eax;
    args.eax = *moved;
    cpu.esp += 4;
    return args.eax;
}

constexpr reg32 kAnglesScale = 0x536cd4, kPitchYawScale = 0x536cdc;  // doubles: a 16-bit turn's radians

/* The turn about y by the float of those bits into the matrix at `matrix`
 * (ea::rotateAboutY, 0x4dfde0, stdcall): the float in the word the code
 * pushes, the callee handed its address.  False when the game stopped. */
bool turnAboutY(App* app, CPU& cpu, reg32 matrix, reg32 angleBits)
{
    const GuestFrame pushed(cpu, 0, 4);
    put(app, pushed - 4, angleBits);
    return guest::direct<0x4dfde0, ea::rotateAboutY>(app, cpu, matrix, pushed - 4).has_value();
}

/* 0x41d510's: the turn matrix at `into` of the angles `first`, `second`
 * and `third` (16-bit turns, scaled by 0x536cd4): a turn about y by
 * -first's * (1 - second's) - second's * third's, one about y by the second
 * itself (unscaled, as the code has it) into the frame, the two multiplied
 * and the result by the second's into `into`.  The frames as deep as the
 * code's.  False when the game stopped. */
std::optional<reg32> turnOfAngles(App* app, CPU& cpu, reg32 first, reg32 second, reg32 third, reg32 into)
{
    const GuestFrame frame(cpu, 4, 0x84);
    const reg32 e = frame - 0x82;
    const reg32 yawTurn = e + 0x22, product = e + 0x46, secondTurn = e - 2;
    const Float k = f64(app, kAnglesScale);
    const Float secondScaled = fp::mul(Float(sreg32(second)), k);
    const Float firstScaled = fp::mul(Float(sreg32(first)), k);
    const Float yawShare = fp::mul(-secondScaled, fp::sub(Float(1.0), firstScaled));
    const Float pitchShare = fp::mul(firstScaled, -fp::mul(k, Float(sreg32(third))));
    const float angle = float(fp::add(pitchShare, yawShare));
    if (!turnAboutY(app, cpu, yawTurn, bitsOf(angle)))
        return std::nullopt;
    if (!turnAboutY(app, cpu, secondTurn, bitsOf(float(Float(sreg32(second))))))
        return std::nullopt;
    guest::direct<0x4e0280, vertices::multiplyMatrix>(app, cpu, yawTurn, into, product);
    if (cpu.terminate)
        return std::nullopt;
    // The last leaves the eax turnFromAngles' leaves.
    const reg32 left = guest::direct<0x4e0280, vertices::multiplyMatrixEax>(app, cpu, product, secondTurn, into);
    if (cpu.terminate)
        return std::nullopt;
    return left;
}

/* 0x41d510: turnOfAngles of eax, ebx and edx into ecx; eax as the last
 * multiply leaves it. */
x86::reg32 turnFromAngles(App* app, x86::CPU& cpu, x86::Args& args)
{
    const std::optional<reg32> left = turnOfAngles(app, cpu, args.eax, args.ebx, args.edx, args.ecx);
    if (!left)
        return args.eax;
    args.eax = *left;
    cpu.esp += 4;
    return args.eax;
}

/* 0x41d5a0's: the turn matrix at `into` of a pitch and a yaw (16-bit turns,
 * scaled by 0x536cdc): a turn about x by a quarter into `into`, about y by
 * the pitch turned back and by the yaw (into the frame, as deep as the
 * code's), all multiplied into `into`.  False when the game stopped. */
std::optional<reg32> turnOfPitchYaw(App* app, CPU& cpu, reg32 pitch, reg32 yaw, reg32 into)
{
    constexpr reg32 kQuarter = 0x3e800000;  // 0.25 as a float
    const GuestFrame frame(cpu, 8, 0x7c);
    const reg32 pitchTurn = frame - 0x58, yawTurn = frame - 0x7c, product = frame - 0x34;
    const Float k = f64(app, kPitchYawScale);
    const float pitchAngle = float(fp::mul(Float(sreg32(pitch)), k));
    const float yawAngle = float(fp::mul(k, Float(sreg32(yaw))));
    {
        const GuestFrame pushed(cpu, 0, 4);
        put(app, pushed - 4, kQuarter);
        if (!guest::direct<0x4dfd80, ea::rotateAboutX>(app, cpu, into, pushed - 4))
            return std::nullopt;
    }
    if (!turnAboutY(app, cpu, pitchTurn, bitsOf(float(-Float(pitchAngle))))
        || !turnAboutY(app, cpu, yawTurn, bitsOf(yawAngle)))
        return std::nullopt;
    guest::direct<0x4e0280, vertices::multiplyMatrix>(app, cpu, pitchTurn, into, product);
    if (cpu.terminate)
        return std::nullopt;
    // The last leaves the eax turnFromPitchYaw's leaves.
    const reg32 left = guest::direct<0x4e0280, vertices::multiplyMatrixEax>(app, cpu, product, yawTurn, into);
    if (cpu.terminate)
        return std::nullopt;
    return left;
}

/* 0x41d5a0: turnOfPitchYaw of eax and edx into ebx. */
x86::reg32 turnFromPitchYaw(App* app, x86::CPU& cpu, x86::Args& args)
{
    const std::optional<reg32> left = turnOfPitchYaw(app, cpu, args.eax, args.edx, args.ebx);
    if (!left)
        return args.eax;
    args.eax = *left;
    cpu.esp += 4;
    return args.eax;
}

/* 0x41d610's: View Distance set (0 Full .. 3 Close). */
void chooseViewDistance(App* app, reg32 distance)
{
    gameSettings(app).viewDistance = distance;
}

/* 0x41d610: chooseViewDistance of eax. */
x86::reg32 setViewDistance(App* app, x86::CPU& cpu, x86::Args& args)
{
    chooseViewDistance(app, args.eax);
    cpu.esp += 4;
    return args.eax;
}

constexpr reg32 kViewDistanceCount = 4;

/* 0x41d870's: View Distance the next of its four (the pause menu's item) --
 * the next stored, then it wrapped round -- and the views' distances worked
 * out again, handed the division's quotient in eax as the code leaves it
 * (eax comes back as it went on some of its paths).  That eax; nullopt when
 * the game stopped. */
std::optional<reg32> viewDistanceOnward(App* app, CPU& cpu)
{
    const reg32 next = gameSettings(app).viewDistance + 1;
    gameSettings(app).viewDistance = next;
    gameSettings(app).viewDistance = reg32(i32(next) % i32(kViewDistanceCount));
    return guest::direct<0x41d620, render::viewDistancesSet>(app, cpu);
}

/* 0x41d870: viewDistanceOnward; eax as the views' distances leave it. */
x86::reg32 nextViewDistance(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEcx | kSaveEdx);
    const std::optional<reg32> left = viewDistanceOnward(app, cpu);
    if (!left)
        return args.eax;
    args.eax = *left;
    cpu.esp += 4;
    return args.eax;
}

// The models by distance (kModels): 3 records of 12 bytes, +8 the most polygons drawn of one (-1 no limit).
constexpr reg32 kNoLimit = 0xffffffff;
// The drivers of types 7 and 8 (kDriverBits).
constexpr reg16 kDriversUnlimited = 0x180;
// A block's +0x18, cleared where the models have no limit (not read in this file).
constexpr reg32 kBlockField18 = 0x18;

/* 0x41d900's: on the drivers of types 7 and 8 the models' most polygons -1
 * (the buckets flushed after every block) and each block's +0x18 cleared,
 * but the last's.  The last block cleared, as the code leaves it in eax;
 * nullopt where none was. */
std::optional<reg32> unlimitedModels(App* app, const CPU& cpu)
{
    if ((half(app, kDriverBits) & kDriversUnlimited) == 0)
        return std::nullopt;
    put(app, kModels + kModelSize + kModelMostPolygons, kNoLimit);
    put(app, kModels + 2 * kModelSize + kModelMostPolygons, kNoLimit);
    put(app, kModels + kModelMostPolygons, kNoLimit);
    std::optional<reg32> last;
    for (reg32 index = 0;; ++index)
    {
        safepoint(app, cpu);
        if (sreg32(index) >= sreg32(at(app, kLastBlock)))
            break;
        last = kBlocks + blockOffset(index);
        put(app, *last + kBlockField18, 0);
    }
    return last;
}

/* 0x41d900: unlimitedModels; eax the last block cleared, else as it was. */
x86::reg32 modelsUnlimited(App* app, x86::CPU& cpu, x86::Args& args)
{
    if (const std::optional<reg32> last = unlimitedModels(app, cpu))
        args.eax = *last;
    cpu.esp += 4;
    return args.eax;
}

/* 0x41da40's: the heap's block at [slot] freed, if there is one, and [slot]
 * 0.  eax as the code leaves it: the free's, or the slot where there was
 * none; nullopt when the game stopped. */
std::optional<reg32> freeAtSlot(App* app, CPU& cpu, reg32 slot)
{
    const reg32 block = at(app, slot);
    if (block == 0)
        return slot;
    const std::optional<reg32> left = freeBlock(app, cpu, block);
    if (!left)
        return std::nullopt;
    put(app, slot, 0);
    return left;
}

/* 0x41da40: freeAtSlot of eax. */
x86::reg32 freeAt(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEcx | kSaveEdx);
    const std::optional<reg32> left = freeAtSlot(app, cpu, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    cpu.esp += 4;
    return args.eax;
}

// Set by render (0x1234) as the track is drawn, 0 once it is let go of.
constexpr reg32 kTrackMark = 0x5dd834;

/* 0x41da60's: the track let go of at the race's end: the arena, the
 * textures' file, the track's nodes, the views, the .col file, the paths and
 * the kinds' records.  eax as the code leaves it (the kinds' freeAtSlot's);
 * nullopt when the game stopped. */
std::optional<reg32> letGoOfTrack(App* app, CPU& cpu)
{
    put(app, kTrackMark, 0);
    if (!freeTheArena(app, cpu, 0))
        return std::nullopt;
    if (at(app, kTexturesFile) != 0 && !freeAtSlot(app, cpu, kTexturesFile))
        return std::nullopt;
    if (!guest::direct<0x41e3b0, trackdata::closeTrack>(app, cpu) || !startViewsOver(app, cpu)
        || !freeCollisionFile(app, cpu, 0) || !guest::direct<0x40f330, aiplan::freePaths>(app, cpu))
        return std::nullopt;
    return freeAtSlot(app, cpu, kTrackKinds);
}

/* 0x41da60: letGoOfTrack. */
x86::reg32 unloadTrack(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEdx);
    const std::optional<reg32> left = letGoOfTrack(app, cpu);
    if (!left)
        return args.eax;
    args.eax = *left;
    cpu.esp += 4;
    return args.eax;
}

// A pool's record: +0 the record taken (-1 none), +0xc, +0x10 its records, +0x18; 36 bytes a record.
constexpr reg32 kPoolTaken = 0, kPoolC = 0xc, kPoolRecords = 0x10, kPool18 = 0x18, kPoolRecordSize = 36;
constexpr reg32 kObjectPoolRecords = 0x800;

/* 0x41daa0's: the pool at `pool` of `count` records (allocated, unnamed):
 * none taken, +0x18 and +0xc 0.  Its records' address, as the code leaves it
 * in eax; nullopt when the game stopped. */
std::optional<reg32> startPool(App* app, CPU& cpu, reg32 pool, reg32 count)
{
    put(app, pool + kPoolTaken, 0xffffffff);
    put(app, pool + kPool18, 0);
    const std::optional<reg32> records = allocate(app, cpu, kNoName2, count * kPoolRecordSize);
    if (!records)
        return std::nullopt;
    put(app, pool + kPoolC, 0);
    put(app, pool + kPoolRecords, *records);
    return records;
}

/* 0x41daa0: startPool of eax, edx records. */
x86::reg32 newPool(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx);
    const std::optional<reg32> records = startPool(app, cpu, args.eax, args.edx);
    if (!records)
        return args.eax;
    args.eax = *records;
    cpu.esp += 4;
    return args.eax;
}

/* 0x41dae0's: startPool of the objects' pool, kObjectPoolRecords records.  Its records' address,
 * as the code leaves it in eax; nullopt when the game stopped. */
std::optional<reg32> startObjectPool(App* app, CPU& cpu)
{
    return startPool(app, cpu, kObjectPool, kObjectPoolRecords);
}

/* 0x41dae0: startObjectPool. */
x86::reg32 newObjectPool(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx | kSaveEdx);
    const std::optional<reg32> records = startObjectPool(app, cpu);
    if (!records)
        return args.eax;
    args.eax = *records;
    cpu.esp += 4;
    return args.eax;
}

/* 0x41db00: freeAtSlot of the track's kinds' records. */
x86::reg32 freeTrackKinds(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEcx | kSaveEdx);
    const std::optional<reg32> left = freeAtSlot(app, cpu, kTrackKinds);
    if (!left)
        return args.eax;
    args.eax = *left;
    cpu.esp += 4;
    return args.eax;
}

constexpr reg32 kQfsTextures = 0x4ea654;      // the textures' count of the .qfs at eax
constexpr reg32 kQfsTexture = 0x4ea658;       // texture edx of the .qfs at eax, 0 none
constexpr reg32 kTakeTexture = 0x4d4080;      // texture edx into the record at ecx, ebx a side at most, [esp+4] 3
// The loading bar at the float [esp+4] per cent (stdcall; graphics::loadingProgress): by address, the
// registers it leaves being what loadTextures leaves where the loading stopped.
constexpr reg32 kLoadingBar = 0x495230;
constexpr reg32 kHandOnRecords = 0x478b70;    // the textures' records at eax handed on
constexpr reg32 kTexturesShare = 0x536d1c;    // a float: the loading bar's share of the textures, 15
constexpr reg32 kTextureRecordSize = 0x2c;
constexpr float kTexturesFrom = 43.0f, kTexturesStep = 15.0f;  // the loading bar's per cent, and a step

/* Entry `index` of the .qfs at `file`, 0 past its end; nullopt when the game stopped. */
std::optional<reg32> packEntry(App* app, CPU& cpu, reg32 file, reg32 index)
{
    const reg32 entry = guest::direct<0x4ea658, frontend::simc::packEntry>(app, cpu, file, index);
    if (cpu.terminate)
        return std::nullopt;
    return entry;
}

/* The textures' count of the .qfs at `file`; nullopt when the game
 * stopped. */
std::optional<reg32> qfsTextures(App* app, CPU& cpu, reg32 file)
{
    const reg32 count = guest::direct<0x4ea654, frontend::simc::packCount>(app, cpu, file);
    if (cpu.terminate)
        return std::nullopt;
    return count;
}

/* What loadTextures leaves in eax, ebx and ecx, which its code does not
 * save. */
struct TexturesLeft
{
    bool stopped;  // the loading stopped (else each texture gone through)
    reg32 count;   // the textures' count, as last asked
    reg32 stop;    // [0x55e88c] as last read, 15's bits before
    bool sized;    // a texture taken the ordinary way (ebx handed its side)
};

/* 0x41db10's: the textures of the .qfs at `file` taken by the driver, each
 * into its record from `records` (0x2c bytes apart) -- a grey driver's one
 * way, else the other (at 128; the port's 512) -- the loading bar moved on
 * from 43 per cent by its share of 15 a texture, until the loading stops.
 * nullopt when the game stopped. */
std::optional<TexturesLeft> takeQfsTextures(App* app, CPU& cpu, reg32 file, reg32 records)
{
    const std::optional<reg32> count = qfsTextures(app, cpu, file);
    if (!count)
        return std::nullopt;
    float step = kTexturesStep, progress = kTexturesFrom;
    if (*count != 0)
        step = float(fp::div(f32(app, kTexturesShare), Float(sreg32(*count))));
    TexturesLeft left = {false, 0, bitsOf(kTexturesStep), false};
    for (reg32 record = records, index = 0;; ++index)
    {
        safepoint(app, cpu);
        const std::optional<reg32> textures = qfsTextures(app, cpu, file);
        if (!textures)
            return std::nullopt;
        left.count = *textures;
        if (sreg32(left.count) <= sreg32(index))
            return left;
        if (!topUp(app, cpu))
            return std::nullopt;
        const std::optional<reg32> texture = packEntry(app, cpu, file, index);
        if (!texture)
            return std::nullopt;
        if (*texture == 0)
            continue;
        if ((byte(app, kDriverBits) & kDriverGrey) != 0)
        {
            if (!guest::direct<0x4d41a0, render::paletteTextured>(app, cpu, *texture, record))
                return std::nullopt;
        }
        else
        {
            constexpr reg32 kTakeKind = 3;
            // port: each texture at its own size up to 512, not 128 (port_thrash.h)
            left.sized = true;
            guest::direct<0x4d4080, frontend::rende::loadShapeTexture>(app, cpu, 0, *texture,
                                                                      port::driver::kMaxTextureSide, record, kTakeKind);
            if (cpu.terminate)
                return std::nullopt;
        }
        progress = float(fp::add(Float(progress), Float(step)));
        {
            // The per cent in the word the code pushes, the bar handed its address.
            const GuestFrame pushed(cpu, 0, 4);
            put(app, pushed - 4, bitsOf(progress));
            if (!guest::direct<0x495230, graphics::loadingProgress>(app, cpu, pushed - 4))
                return std::nullopt;
        }
        left.stop = at(app, kLoadStopped);
        record += kTextureRecordSize;
        if (left.stop != 0)
        {
            left.stopped = true;
            return left;
        }
    }
}

/* 0x41db10: takeQfsTextures of the file eax into the records at edx; eax,
 * ebx and ecx as the code leaves them (eax the loading bar's, where the
 * loading stopped). */
x86::reg32 loadTextures(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEsi | kSaveEdi);
    const std::optional<TexturesLeft> left = takeQfsTextures(app, cpu, args.eax, args.edx);
    if (!left)
        return args.eax;
    if (!left->sized)
        args.ebx = 0;
    args.ecx = left->stop;
    if (!left->stopped)
        args.eax = left->count;
    cpu.esp += 4;
    return args.eax;
}

/* 0x41dc60's: the opened textures taken by the driver (takeQfsTextures, into
 * the track's kinds' records; their count to 0x5dd854, 0x5dd85c and
 * 0x5dd860) and the file let go of; the records handed on.  eax as that
 * leaves it; nullopt when the game stopped. */
std::optional<reg32> takeTrackTextures(App* app, CPU& cpu)
{
    if (at(app, kTexturesFile) != 0)
    {
        const std::optional<reg32> count = qfsTextures(app, cpu, at(app, kTexturesFile));
        if (!count)
            return std::nullopt;
        const reg32 records = at(app, kTrackKinds);
        put(app, kTexturesCount, *count);
        if (!takeQfsTextures(app, cpu, at(app, kTexturesFile), records))
            return std::nullopt;
        const reg32 taken = at(app, kTexturesCount);
        put(app, kTexturesLeft, taken);
        put(app, kTexturesAll, taken);
        if (!freeAtSlot(app, cpu, kTexturesFile))
            return std::nullopt;
        put(app, kTexturesFile, 0);
    }
    const reg32 left = guest::direct<0x478b70, frontend::loada::horizonSetPieces>(app, cpu, at(app, kTrackKinds));
    if (cpu.terminate)
        return std::nullopt;
    return left;
}

/* 0x41dc60: takeTrackTextures. */
x86::reg32 takeTextures(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx | kSaveEdx);
    if (!takeTrackTextures(app, cpu))
        return args.eax;
    cpu.esp += 4;
    return args.eax;
}

// The .col file closed (collide::closeColl, Watcom's, the file at eax).
constexpr reg32 kColClose = 0x4313b0;
constexpr reg32 kColLeast = 0x16800;       // the least of the game's memory the .col is given
// A chunk: +4 its kind (a word: 2 the vertices, 7 the lights, 8 the objects, 0xf the track's nodes),
// its records from +8; a vertex of it 8 bytes, +6 and +7 given 1 and 0x1f where 0.
constexpr reg16 kChunkVertices = 2, kChunkLights = 7, kChunkObjects = 8, kChunkNodes = 0xf;
constexpr reg32 kChunkKind = 4, kChunkRecords = 8, kChunkCount = 6, kColVertexSize = 8;

/* 0x41dcc0's: the track's .col file named `name` loaded, if it is there --
 * at least 0x16800 bytes of the game's memory for it (0x552e18) -- and its
 * chunks gone through: the vertices (their bytes +6 and +7 given 1 and 0x1f
 * where 0), the lights (0x5dd840), the objects (0x5dd844), each chunk of the
 * track's nodes opened; the file closed.  What the code leaves in eax: the
 * closing's, 0 where there is no file; nullopt when the game stopped. */
std::optional<reg32> loadCollisionFile(App* app, CPU& cpu, reg32 name)
{
    const GuestFrame frame(cpu, 0x18, 4);  // as deep as the code's: the .col's functions' own frames log their places
    put(app, kColObjects, 0);
    put(app, kColLights, 0);
    const std::optional<bool> exists = guest::direct<0x4e0e60, ea::fileThere>(app, cpu, name);
    if (!exists)
        return std::nullopt;
    if (!*exists)
        return 0;
    const std::optional<reg32> file = guest::direct<0x431320, collide::openColl>(app, cpu, name);
    if (!file)
        return std::nullopt;
    reg32 size = guest::direct<0x4313d0, collide::collSize>(app, cpu);
    if (cpu.terminate)
        return std::nullopt;
    if (sreg32(size) > sreg32(kColLeast))
    {
        size = guest::direct<0x4313d0, collide::collSize>(app, cpu);
        if (cpu.terminate)
            return std::nullopt;
    }
    else
        size = kColLeast;
    const std::optional<reg32> memory = allocate(app, cpu, kNoName2, size);
    if (!memory)
        return std::nullopt;
    put(app, kColFile, *memory);
    // The read gives back 1, which the code tests.
    if (!guest::direct<0x4313e0, collide::readColl>(app, cpu, *file, *memory))
        return std::nullopt;
    {
        const reg32 chunks = guest::direct<0x431410, collide::collCount>(app, cpu);
        if (cpu.terminate)
            return std::nullopt;
        for (reg32 index = 0; sreg32(index) < sreg32(chunks); ++index)
        {
            safepoint(app, cpu);
            const reg32 chunk = guest::direct<0x431420, collide::collChunkAt>(app, cpu, at(app, kColFile), index);
            if (cpu.terminate)
                return std::nullopt;
            if (!topUp(app, cpu))
                return std::nullopt;
            const reg16 kind = half(app, chunk + kChunkKind);
            if (kind < kChunkLights)
            {
                if (kind != kChunkVertices)
                    continue;
                put(app, kColVertices, chunk + kChunkRecords);
                for (reg32 vertex = 0;; ++vertex)
                {
                    safepoint(app, cpu);
                    if (sreg32(vertex) >= sreg32(reg32(half(app, chunk + kChunkCount))))
                        break;
                    if (byte(app, at(app, kColVertices) + vertex * kColVertexSize + 7) == 0)
                        putByte(app, at(app, kColVertices) + vertex * kColVertexSize + 7, 0x1f);
                    if (byte(app, at(app, kColVertices) + vertex * kColVertexSize + 6) == 0)
                        putByte(app, at(app, kColVertices) + vertex * kColVertexSize + 6, 1);
                }
            }
            else if (kind == kChunkLights)
                put(app, kColLights, chunk);
            else if (kind <= kChunkObjects)
                put(app, kColObjects, chunk);
            else if (kind == kChunkNodes && !guest::direct<0x41e370, trackdata::openTrack>(app, cpu, chunk))
                return std::nullopt;
        }
    }
    // The closing's eax is what loadCollisions leaves.
    return guest::direct<0x4313b0, collide::closeColl>(app, cpu, *file);
}

/* 0x41dcc0: loadCollisionFile of eax. */
x86::reg32 loadCollisions(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx | kSaveEdx | kSaveEsi | kSaveEdi);
    const std::optional<reg32> left = loadCollisionFile(app, cpu, args.eax);
    if (!left)
        return args.eax;
    args.eax = *left;
    cpu.esp += 4;
    return args.eax;
}

/* 0x41de10's: the .col file's memory freed, if there is some, and [0x552e18]
 * 0 (freeAtSlot has done it already).  eax as the free leaves it (`eax` as it
 * was where there is none); nullopt when the game stopped. */
std::optional<reg32> freeCollisionFile(App* app, CPU& cpu, reg32 eax)
{
    if (at(app, kColFile) == 0)
        return eax;
    const std::optional<reg32> freed = freeAtSlot(app, cpu, kColFile);
    if (!freed)
        return std::nullopt;
    put(app, kColFile, 0);
    return freed;
}

/* 0x41de10: freeCollisionFile. */
x86::reg32 freeCollisions(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEcx | kSaveEdx);
    const std::optional<reg32> freed = freeCollisionFile(app, cpu, args.eax);
    if (!freed)
        return args.eax;
    args.eax = *freed;
    cpu.esp += 4;
    return args.eax;
}

// The objects' animation states (kObjectStates): 0x200 of 3 bytes, still as 0xff, 0, 0.
constexpr reg32 kObjectStateCount = 0x200;

/* 0x41de40's: the objects' states all still, the sound's stream topped up
 * before each.  False when the game stopped. */
bool stillObjects(App* app, CPU& cpu)
{
    for (reg32 index = 0; index < kObjectStateCount; ++index)
    {
        safepoint(app, cpu);
        if (!topUp(app, cpu))
            return false;
        const reg32 state = kObjectStates + index * kObjectStateSize;
        putByte(app, state, kStateStill);
        putByte(app, state + 1, 0);
        putByte(app, state + 2, 0);
    }
    return true;
}

/* 0x41de40: stillObjects; eax the last state's offset, as the code leaves it. */
x86::reg32 clearObjectStates(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEdx);
    if (!stillObjects(app, cpu))
        return args.eax;
    args.eax = (kObjectStateCount - 1) * kObjectStateSize;
    cpu.esp += 4;
    return args.eax;
}

/* 0x41de80's: the views as a race starts: the objects still, the first view
 * the one drawn and the cameras cleared (eax 0, as chooseView leaves it).
 * False when the game stopped. */
std::optional<reg32> startTrackViews(App* app, CPU& cpu)
{
    if (!stillObjects(app, cpu))
        return std::nullopt;
    chooseView(app, 0);
    guest::direct<0x40f230, load::clearCameras>(app, cpu);
    if (cpu.terminate)
        return std::nullopt;
    return 255 * 3;  // the last of the cameras' 256 views times 3, as clearTrackCameras leaves it
}

/* 0x41de80: startTrackViews; eax as the cameras' clearing leaves it. */
x86::reg32 resetTrackViews(App* app, x86::CPU& cpu, x86::Args& args)
{
    const std::optional<reg32> left = startTrackViews(app, cpu);
    if (!left)
        return args.eax;
    args.eax = *left;
    cpu.esp += 4;
    return args.eax;
}

// The views (kViews): +0 -1 as they start over (kNoBlock); the serial they start at.
constexpr reg32 kViewCount = 4, kSerialStart = 0x1234;

/* 0x41deb0's: the views' serial started over and the four views on no block,
 * the sound's stream topped up before each.  False when the game stopped. */
bool startViewsOver(App* app, CPU& cpu)
{
    put(app, kViewSerial, kSerialStart);
    put(app, kViewSerialHigh, 0);
    for (reg32 view = 0; view < kViewCount; ++view)
    {
        safepoint(app, cpu);
        if (!topUp(app, cpu))
            return false;
        put(app, kViews + view * kViewSize + kViewStart, kNoBlock);
    }
    return true;
}

/* 0x41dea0: startViewsOver (0x41deb0's own); eax the last view's offset in
 * dwords, as the code leaves it. */
x86::reg32 resetViewsOuter(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx | kSaveEdx);
    if (!startViewsOver(app, cpu))
        return args.eax;
    args.eax = (kViewCount - 1) * (kViewSize / 4);
    cpu.esp += 4;
    return args.eax;
}

/* 0x41deb0: startViewsOver; eax as resetViewsOuter leaves it. */
x86::reg32 resetViews(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx | kSaveEdx);
    if (!startViewsOver(app, cpu))
        return args.eax;
    args.eax = (kViewCount - 1) * (kViewSize / 4);
    cpu.esp += 4;
    return args.eax;
}


/* 0x41e070's: whether block `index` is drawn for the camera: in view (its
 * box) and within the view's reach of its centre, across the ground (x and
 * z) -- an x87 compare, a NaN counting as out of reach.  nullopt when the
 * game stopped. */
std::optional<bool> blockDrawn(App* app, CPU& cpu, reg32 camera, reg32 index)
{
    const reg32 block = kBlocks + blockOffset(index);
    const reg32 outside = guest::direct<0x41bf30, vertices::boxOutsideCore>(
        app, cpu, camera, block + kBlockBox, camera + kCameraMatrix, camera + kCameraBox);
    if (cpu.terminate)
        return std::nullopt;
    if (outside != 0)
        return false;
    const reg32 centre = block + kBlockCentre;
    const Float dz = fp::sub(f32(app, camera + kCameraPlace + 8), f32(app, centre + 8));
    const Float dx = fp::sub(f32(app, camera + kCameraPlace), f32(app, centre));
    const Float reach = f32(app, at(app, kView) + kViewReach);
    const Float squared = fp::add(fp::mul(dz, dz), fp::mul(dx, dx));
    return fp::mul(reach, reach) >= squared;
}

/* 0x41e070: blockDrawn for the camera eax and block edx, 1 where it is. */
x86::reg32 blockInView(App* app, x86::CPU& cpu, x86::Args& args)
{
    const Kept kept(args, kSaveEbx | kSaveEcx | kSaveEsi | kSaveEdi);
    const std::optional<bool> drawn = blockDrawn(app, cpu, args.eax, args.edx);
    if (!drawn)
        return args.eax;
    args.eax = *drawn ? 1 : 0;
    cpu.esp += 4;
    return args.eax;
}

}  // namespace track
}  // namespace nfs3hp
