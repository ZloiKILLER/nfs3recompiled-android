#include "native_frontend_hudx.h"
#include "game_settings.h"
#include "native_carstate.h"
#include "native_frontend.h"
#include "native_watcom.h"
#include "native_frontend_hudr.h"
#include "native_frontend_hudz.h"
#include "port_hud.h"
#include "port_text.h"
#include "native_render.h"
#include <nfs3hp.h>
#include <game_float.h>
#include <cstring>

/* The thunks defined below under their qualified names. */
namespace nfs3hp
{
namespace frontend
{
nfs::u32 textLookClearLights(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textLookSetFlat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textLookSetDirectional(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textLookAddLight(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 textLookSetTexture(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudClearRebuildFlag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudToggleShown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudToggleRearView(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudRebuildView(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudGroupClearCount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudGroupDropLast(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudRefreshPane(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudStepZoom(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudVectorLength(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudSurfaceReset(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudSurfaceTakeVertices(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudTextToRightEdge(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudFontSurfacesBegin(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
nfs::u32 hudCharacterDrop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args);
}
}

/* The race HUD's small functions, decompiled: what src/nfs3hp/port_hud.cpp and
 * the race's the game's code call by address.
 *
 *   the 3D letters' looks   the records the race's big messages are drawn with
 *                           (native_render.cpp reads them), which the HUD sets
 *                           up as a race starts and the countdown changes;
 *   the HUD's records       what its keys toggle, per player and racer or cop
 *                           (native_frontend_hudedit.cpp edits the same ones);
 *   its panes and rects     rebuilt, refreshed, zoomed;
 *   its surface             the 32-byte vertices and the index list at 0x793108;
 *   its text                drawn to a right edge, the fonts' surfaces begun,
 *                           a character's drop.
 *
 * All of the game's state stays in guest memory.  What each function leaves in
 * eax and in the registers it does not save is kept: Watcom's callers may go on
 * with them.
 *
 * Plan step 5: each function a typed core in frontend::hudx (native_frontend_hudx.h), in a frame of
 * its own where the game's function has one, and a thunk under the game's name for the calls by
 * address.  A core of this file calls another, and those of the HUD's other files done so
 * (native_frontend_hudr.cpp, _hudz.cpp), as C++ (guest::direct), and the typed cores of other files
 * too (fe::direct); the rest of the game still by address. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using nfs::Float;
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

/* fild, fld and sub_4dfd56 with fistp (fe::FloatMath's) as plain values: no x87 behind them. */
class FloatMath
{
public:
    explicit FloatMath(Guest& guest) : m_guest(guest) {}
    static Float integer(std::int32_t value) { return Float(s32(value)); }
    Float load(u32 address) const { return Float(m_guest.floatAt(address)); }
    std::int32_t truncated(const Float& value)
    {
        const double chopped = guest::direct<0x4dfd56, watcom::clib::chop>(m_guest.app(), m_guest.ts(), double(value));
        return fp::toInteger<s32>(chopped);  // fistp
    }

private:
    Guest& m_guest;
};

/* ---------------------------------------------------------------------------
 * The 3D letters' looks
 * ------------------------------------------------------------------------- */

constexpr u32 kSelectedLook = 0x55b088;          // which one the setters work on; -1 none
constexpr u32 kLooks = 0x55b08c;
constexpr u32 kLookBytes = 0x238;
constexpr u32 kNoLook = 0xffffffff;

constexpr u32 kLookKind = 0x00;                  // kFlat, kDirectional or kLit
constexpr u32 kLookInUse = 0x04;                 // the setters leave a look with 0 here alone
constexpr u32 kLookColours = 0x08;               // kFlat's three
constexpr u32 kLookDirectionalColour = 0x14;
constexpr u32 kLookDirection = 0x20;             // kDirectional's: three words
constexpr u32 kLookTexture = 0x2c;
constexpr u32 kLookLights = 0x30;                // kLightCount of kLightBytes
constexpr u32 kLookLightCount = 0x230;
constexpr u32 kLookLightCountAgain = 0x234;

constexpr std::uint32_t kFlat = 0, kDirectional = 3, kLit = 4;

constexpr u32 kLightCount = 8;
constexpr u32 kLightBytes = 0x40;
constexpr u32 kLightCleared = 0x00;              // set to 0 as a light is added
constexpr u32 kLightSecond = 0x04;
constexpr u32 kLightFirst = 0x08;
constexpr u32 kLightUsed = 0x0c;
constexpr u32 kLightThird = 0x10;
constexpr u32 kLightPosition = 0x28;             // three words

constexpr u32 kNoLookForLight = 0xfffffffe;      // sub_4771c0's answers past a light's number
constexpr u32 kLightsFull = 0xffffffff;

/* The selected look's address, -1's too: the setters work it out before they
 * look at the number. */
u32 lookAt(u32 number)
{
    return kLooks + number * kLookBytes;
}

bool lookUsable(const Guest& guest, u32 number, u32 look)
{
    return number != kNoLook && guest.dword(look + kLookInUse) != 0;
}

/* Three words copied one at a time, as the game copies them: `from` may be
 * anywhere, the look itself too. */
void copyTriple(Guest& guest, u32 to, u32 from)
{
    for (u32 i = 0; i < 3; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

}  // namespace

/* sub_474d50: the selected look's lights off, if it has any.  It does not look
 * at the number: its callers have.  It leaves eax the look's offset when there
 * was nothing to do, and the lights' count when there was. */
nfs::u32 frontend::hudx::clearLights(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const u32 number = guest.dword(kSelectedLook);
    const u32 look = lookAt(number);
    if (guest.dword(look + kLookLightCount) == 0)
        return number * kLookBytes;
    guest.setDword(look + kLookLightCountAgain, 0);
    guest.setDword(look + kLookLightCount, 0);
    for (u32 i = 0; i < kLightCount; ++i)
        guest.setDword(look + kLookLights + i * kLightBytes + kLightUsed, 0);
    return kLightCount;
}

/* sub_477070: the selected look flat, in three colours; its lights go. */
nfs::u32 frontend::hudx::setFlat(App* app, ThreadState& ts, u32 first, u32 second, u32 third)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    const u32 number = guest.dword(kSelectedLook);
    const u32 look = lookAt(number);
    if (!lookUsable(guest, number, look))
        return look;
    guest.setDword(look + kLookKind, kFlat);
    guest.setDword(look + kLookColours, first);
    guest.setDword(look + kLookColours + 4, second);
    guest.setDword(look + kLookColours + 8, third);
    // The game's code hands it the look in eax, which it does not read.
    return guest::direct<0x474d50, clearLights>(app, ts);
}

/* sub_477170: the selected look lit from one direction, as the countdown has
 * it; its lights go. */
nfs::u32 frontend::hudx::setDirectional(App* app, ThreadState& ts, u32 colour, u32 direction)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    const u32 number = guest.dword(kSelectedLook);
    const u32 look = lookAt(number);
    if (!lookUsable(guest, number, look))
        return look;
    guest.setDword(look + kLookKind, kDirectional);
    guest.setDword(look + kLookDirectionalColour, colour);
    copyTriple(guest, look + kLookDirection, direction);
    // The game's code hands it the look in eax, which it does not read.
    return guest::direct<0x474d50, clearLights>(app, ts);
}

/* sub_4771c0: a light added to the selected look, into the first free of its
 * eight: its number, or kNoLookForLight, or kLightsFull. */
nfs::u32 frontend::hudx::addLight(App* app, ThreadState& ts, u32 first, u32 position, u32 third, u32 second)
{
    Guest guest(app, ts);
    const u32 number = guest.dword(kSelectedLook);
    const u32 look = lookAt(number);
    if (!lookUsable(guest, number, look))
    {
        return kNoLookForLight;
    }
    guest.setDword(look + kLookKind, kLit);
    u32 index = 0;
    while (index < kLightCount && guest.dword(look + kLookLights + index * kLightBytes + kLightUsed) != 0)
        ++index;
    if (index == kLightCount)
    {
        return kLightsFull;
    }
    const u32 light = look + kLookLights + index * kLightBytes;
    guest.setDword(light + kLightCleared, 0);
    guest.setDword(light + kLightUsed, 1);
    guest.setDword(light + kLightSecond, second);
    guest.setDword(light + kLightThird, third);
    guest.setDword(light + kLightFirst, first);
    copyTriple(guest, light + kLightPosition, position);
    const u32 count = guest.dword(look + kLookLightCount) + 1;
    const u32 countAgain = guest.dword(look + kLookLightCountAgain) + 1;
    guest.setDword(look + kLookLightCount, count);
    guest.setDword(look + kLookLightCountAgain, countAgain);
    return index;
}

/* sub_477470: the selected look's texture. */
nfs::u32 frontend::hudx::setTexture(App* app, ThreadState& ts, u32 texture)
{
    Guest guest(app, ts);
    const u32 number = guest.dword(kSelectedLook);
    const u32 look = lookAt(number);
    if (lookUsable(guest, number, look))
        guest.setDword(look + kLookTexture, texture);
    return look;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The HUD's records and panes
 * ------------------------------------------------------------------------- */

constexpr u32 kPaneUp = 0x791b1c;
constexpr u32 kRebuildAfterDefaults = 0x55d440;  // native_frontend_hudedit.cpp reads it

constexpr u32 kMapScaleOne = 0x3f800000;         // 1.0f, its bits: compared as a word

constexpr u32 kCarFlags = 0x200;                 // byte
constexpr std::uint8_t kCarIsCop = 0x20;
constexpr u32 kSetPane = 0x4844c0;               // eax a view
constexpr u32 kElements = 23;
constexpr u32 kZoomStepped = 0x48ea20;           // eax a view, edx the step

constexpr u32 kPaneResults = 0x7925d0;           // by view: what sub_481f90 gave
constexpr u32 kPaneColours = 0x792880;           // by view: which of the box's colours

/* In split screen view 0 has player 1's record and view 1 player 2's. */
u32 playerOfView(const Guest& guest, u32 view)
{
    return view + (gameSettings(guest).playMode == 1 ? 1 : 0);
}

/* Whether the car a view follows is a cop's: which of the player's records. */
u32 copOfView(Guest& guest, nfs::ThreadState& ts, u32 view)
{
    const u32 car = fe::direct<0x422bd0, carstate::targetOfCamera>(guest.app(), ts, view);
    return (guest.byte(car + kCarFlags) & kCarIsCop) != 0 ? 1 : 0;
}

void toggle(u32& field)
{
    field = field == 0 ? 1 : 0;
}

}  // namespace

/* sub_484260: a view's HUD on or off, its pane set afresh.  Only view 0 has one
 * outside split screen.  With no pane up yet and kNetPlayers at 2 it is on, not
 * toggled, and in split screen the other view's pane is set too. */
nfs::u32 frontend::hudx::toggleShown(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    const u32 player = playerOfView(guest, view);
    const u32 cop = copOfView(guest, ts, view);
    if (view != 0 && gameSettings(guest).playMode != 1)
        return cop;
    u32& shown = gameSettings(guest).hudOf(player, cop).shown;
    if (guest.dword(kPaneUp) == 0 && gameSettings(guest).replayMode == 2)
    {
        shown = 1;
        const bool split = gameSettings(guest).playMode == 1;
        guest.setDword(kPaneUp, 1);
        if (split)
            fe::direct<0x4844c0, port::hud::hudPaneSetOf>(app, ts, ts.sp - 4, port::Args{ 1 - view, 0, 0, 0 });
    }
    else
    {
        toggle(shown);
    }
    return u32(*fe::direct<0x4844c0, port::hud::hudPaneSetOf>(app, ts, ts.sp - 4, port::Args{ view, 0, 0, 0 }));
}

/* sub_484450: the rear-view mirror on or off, in a race of one view only. */
nfs::u32 frontend::hudx::toggleRearView(App* app, ThreadState& ts)
{
    return toggleRearViewKeeping(app, ts, 0);
}

nfs::u32 frontend::hudx::toggleRearViewKeeping(App* app, ThreadState& ts, u32 eax)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    if (gameSettings(guest).playMode == 1 || gameSettings(guest).replayMode != 0)
        return eax;
    const u32 cop = copOfView(guest, ts, 0);
    toggle(gameSettings(guest).hudOf(0, cop).rearView);
    return u32(*fe::direct<0x4844c0, port::hud::hudPaneSetOf>(app, ts, ts.sp - 4, port::Args{ 0, 0, 0, 0 }));
}

/* sub_485640: a view's pane, its elements and its band made afresh. */
nfs::u32 frontend::hudx::rebuildView(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    fe::direct<0x4844c0, port::hud::hudPaneSetOf>(app, ts, ts.sp - 4, port::Args{ view, 0, 0, 0 });
    for (u32 element = 0; element < kElements; ++element)
        guest::direct<0x4853c0, hudr::rebuildElement>(app, ts, view, element);
    return *fe::direct<0x4800a0, render::hudBandBuilt>(app, ts, view);
}

/* sub_48ccf0: where a view's map pane is now, and from that which colour its
 * box takes: 0 at scale 1, 1 at any other, 2 with no place at all. */
nfs::u32 frontend::hudx::refreshPane(App* app, ThreadState& ts, u32 view)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 24);
    const u32 player = playerOfView(guest, view);
    const u32 cop = copOfView(guest, ts, view);
    const u32 scale = gameSettings(guest).hudOf(player, cop).mapScaleBits;
    const u32 place = guest::direct<0x481f90, hudz::mapPaneIsSmall>(app, ts, view);
    guest.setDword(kPaneResults + 4 * view, place);
    const u32 colour = place == 0 ? 2 : scale == kMapScaleOne ? 0 : 1;
    guest.setDword(kPaneColours + 4 * view, colour);
    return place;
}

/* sub_48eb60: the zoom stepped in view 0 and, in split screen, in view 1. */
nfs::u32 frontend::hudx::stepZoom(App* app, ThreadState& ts, u32 step)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 12);
    // By address: the function goes out with the ebx, esi and edi sub_48ea20 leaves.
    u32 result = callAt(guest, kZoomStepped, args({ 0, step }));
    if (gameSettings(guest).playMode == 1)
        result = callAt(guest, kZoomStepped, args({ 1, step }));
    return result;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The HUD's rectangle groups
 * ------------------------------------------------------------------------- */

constexpr u32 kGroupCounts = 0x55d694;           // by group: how many rectangles (sub_487350 adds them)

/* ---------------------------------------------------------------------------
 * The HUD's surface
 * ------------------------------------------------------------------------- */

constexpr u32 kSurfaceVertexCount = 0x00;
constexpr u32 kSurfaceVertices = 0x04;           // kSurfaceSize of kVertexBytes
constexpr u32 kSurfaceIndices = 0x8004;          // kSurfaceSize words
constexpr u32 kSurfaceSize = 0x400;
constexpr u32 kVertexBytes = 0x20;
constexpr u32 kVertexU = 0x08;
constexpr u32 kVertexV = 0x0c;
constexpr u32 kVertexFlags = 0x14;
constexpr u32 kInsideTexelLow = 0x37800080;      // a float just above 1/65536
constexpr u32 kInsideTexelHigh = 0x3f7fff00;     // a float just below 1

}  // namespace

/* sub_49bef0: a surface empty, its indices in order, its vertices at the same
 * texture coordinates.  It leaves eax the size. */
nfs::u32 frontend::hudx::resetSurface(App* app, ThreadState& ts, u32 surface)
{
    Guest guest(app, ts);
    guest.setDword(surface + kSurfaceVertexCount, 0);
    for (u32 i = 0; i < kSurfaceSize; ++i)
    {
        const u32 vertex = surface + kSurfaceVertices + i * kVertexBytes;
        guest.setDword(surface + kSurfaceIndices + i * 4, i);
        guest.setDword(vertex + kVertexU, kInsideTexelLow);
        guest.setDword(vertex + kVertexV, kInsideTexelHigh);
        guest.setDword(vertex + kVertexFlags, 0);
    }
    return kSurfaceSize;
}

/* sub_49bf40: `pairs` pairs of vertices taken from a surface, the first one's
 * address put at `first`. */
nfs::u32 frontend::hudx::takeVertices(App* app, ThreadState& ts, u32 surface, u32 pairs, u32 first)
{
    Guest guest(app, ts);
    guest.setDword(first, surface + kSurfaceVertices + guest.dword(surface + kSurfaceVertexCount) * kVertexBytes);
    guest.setDword(surface + kSurfaceVertexCount, guest.dword(surface + kSurfaceVertexCount) + pairs * 2);
    return surface;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The HUD's text
 * ------------------------------------------------------------------------- */

constexpr u32 kTextWidthOverhang = 0x4d1480;     // eax the text: its width onto the x87's stack
constexpr u32 kDrawText = 0x4d1000;              // eax the text, edx x, ebx y: its width onto the x87's stack
constexpr u32 kFontSurface = 0x7f74c0, kFontTexture = 0x83f6ac;
constexpr u32 kFont2Surface = 0x81b4d0, kFont2Texture = 0x83f6d8;
constexpr u32 kFont = 0x840184;
constexpr u32 kCharacterSize = 0x84018c;         // a float
constexpr u32 kCharacterDrops = 0x83fa60;        // int8, by font and character
constexpr u32 kFontCharacters = 0xe0;

}  // namespace

/* sub_4d10e0: a text ending at x, at the y in ebx.  The width comes off the
 * x87's stack into the sum; what kDrawText leaves there is dropped.  Both by
 * address: the drawing's y is the ebx the width's call leaves, and this
 * function leaves the registers as the drawing does. */
nfs::u32 frontend::hudx::drawTextToRightEdge(App* app, ThreadState& ts, u32 text, u32 right, u32 y)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 16);
    const auto measured = fe::direct<0x4d1480, port::text::textMeasuredWithOverhang>(app, ts, ts.sp - 4, text);
    const Float width = measured->value;
    FloatMath math(guest);
    const std::int32_t left = math.truncated(fp::sub(Float(s32(right)), width));
    // The drawing's y is the ebx the caller hands.
    const auto drawn = fe::direct<0x4d1000, port::text::textDrawn>(app, ts, ts.sp - 4, text, left, std::int32_t(y));
    return drawn->eax;
}

/* sub_4d1310: the two fonts' surfaces begun, each with its own texture. */
nfs::u32 frontend::hudx::beginFontSurfaces(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    const fe::Frame frame(guest, ts, 8);
    guest::direct<0x49bc80, hudz::queueStart>(app, ts, kFontSurface, guest.dword(kFontTexture));
    return guest::direct<0x49bc80, hudz::queueStart>(app, ts, kFont2Surface, guest.dword(kFont2Texture));
}

/* sub_4d14f0: how far a character of the font drops, at the character size. */
nfs::u32 frontend::hudx::characterDrop(App* app, ThreadState& ts, u32 code)
{
    Guest guest(app, ts);
    const std::uint8_t character = std::uint8_t(code);
    const u32 at = kCharacterDrops + character + guest.dword(kFont) * kFontCharacters;
    const Float drop = Float(s32(std::int8_t(guest.byte(at))));
    FloatMath math(guest);
    return u32(math.truncated(fp::mul(drop, Float(guest.floatAt(kCharacterSize)))));
}

/* sub_484220: as a race ends, the flag that has the HUD's defaults rebuild it taken down. */
void frontend::hudx::clearRebuildFlag(App* app, ThreadState& ts)
{
    Guest guest(app, ts);
    guest.setDword(kRebuildAfterDefaults, 0);
}

/* sub_487340: a group of the HUD's rectangles empty. */
nfs::u32 frontend::hudx::groupClearCount(App* app, ThreadState& ts, u32 group)
{
    Guest guest(app, ts);
    guest.setDword(kGroupCounts + group * 4, 0);
    return group;
}

/* sub_4873c0: a group's last rectangle gone, if it has one.  It leaves eax the count's offset. */
nfs::u32 frontend::hudx::groupDropLast(App* app, ThreadState& ts, u32 group)
{
    Guest guest(app, ts);
    const u32 offset = group * 4;
    const u32 count = guest.dword(kGroupCounts + offset);
    if (count != 0)
        guest.setDword(kGroupCounts + offset, count - 1);
    return offset;
}

/* sub_497590 (stdcall 8): the length of the vector x, y on the stack (its thunk leaves it on the
 * x87's stack).  eax comes back as it went: the function saves it. */
double frontend::hudx::vectorLength(App* app, ThreadState& ts, u32 xBits, u32 yBits)
{
    NFS2_USE(app);
    NFS2_USE(ts);
    float xValue, yValue;
    std::memcpy(&xValue, &xBits, sizeof xValue);
    std::memcpy(&yValue, &yBits, sizeof yValue);
    const Float x(xValue);
    const Float y(yValue);
    const Float xSquared = fp::mul(x, x);
    const Float ySquared = fp::mul(y, y);
    return fp::sqrt(fp::add(xSquared, ySquared));
}

/* ---------------------------------------------------------------------------
 * Under the game's addresses
 * ------------------------------------------------------------------------- */

nfs::u32 frontend::textLookClearLights(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_474d50", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::clearLights(app, ts); });
}

nfs::u32 frontend::textLookSetFlat(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_477070", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 first = args.eax, second = args.edx, third = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::setFlat(app, ts, first, second, third); });
}

nfs::u32 frontend::textLookSetDirectional(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_477170", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 colour = args.eax, direction = args.edx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::setDirectional(app, ts, colour, direction); });
}

nfs::u32 frontend::textLookAddLight(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4771c0", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const u32 first = args.eax, position = args.edx, third = args.ebx, second = args.ecx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::addLight(app, ts, first, position, third, second); });
}

nfs::u32 frontend::textLookSetTexture(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_477470", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 texture = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::setTexture(app, ts, texture); });
}

nfs::u32 frontend::hudClearRebuildFlag(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_484220", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        hudx::clearRebuildFlag(app, ts);
        return args.eax;
    });
}

nfs::u32 frontend::hudToggleShown(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_484260", 0, 0, nullptr, 0 };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::toggleShown(app, ts, view); });
}

nfs::u32 frontend::hudToggleRearView(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_484450", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::toggleRearViewKeeping(app, ts, args.eax); });
}

nfs::u32 frontend::hudRebuildView(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_485640", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::rebuildView(app, ts, view); });
}

nfs::u32 frontend::hudGroupClearCount(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_487340", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const u32 group = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::groupClearCount(app, ts, group); });
}

nfs::u32 frontend::hudGroupDropLast(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4873c0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 group = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::groupDropLast(app, ts, group); });
}

nfs::u32 frontend::hudRefreshPane(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_48ccf0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const u32 view = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::refreshPane(app, ts, view); });
}

nfs::u32 frontend::hudStepZoom(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_48eb60", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 step = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::stepZoom(app, ts, step); });
}

nfs::u32 frontend::hudVectorLength(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_497590", 8, 0, nullptr, 0, 0 };
    const u32 x = app->getMemory<u32>(ts.sp + 4), y = app->getMemory<u32>(ts.sp + 8);
    return fe::runThunk(app, ts, args, site, [&](Guest&) {
        args.floatSet = true;
        args.floatResult = double(hudx::vectorLength(app, ts, x, y));
        return args.eax;
    });
}

nfs::u32 frontend::hudSurfaceReset(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49bef0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 surface = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::resetSurface(app, ts, surface); });
}

nfs::u32 frontend::hudSurfaceTakeVertices(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_49bf40", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    const u32 surface = args.eax, pairs = args.edx, first = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::takeVertices(app, ts, surface, pairs, first); });
}

nfs::u32 frontend::hudTextToRightEdge(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d10e0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const u32 text = args.eax, right = args.edx, y = args.ebx;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::drawTextToRightEdge(app, ts, text, right, y); });
}

nfs::u32 frontend::hudFontSurfacesBegin(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d1310", 0, 0, nullptr, 0, fe::kRestoreEdx };
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::beginFontSurfaces(app, ts); });
}

nfs::u32 frontend::hudCharacterDrop(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args)
{
    static fe::Site site{ "sub_4d14f0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const u32 character = args.eax;
    return fe::runThunk(app, ts, args, site, [&](Guest&) { return hudx::characterDrop(app, ts, character); });
}

}
