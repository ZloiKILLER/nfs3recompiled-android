#include "native_frontend_hudx.h"
#include "game_settings.h"
#include "native_carstate.h"
#include "native_frontend.h"
#include "native_frontend_hudr.h"
#include "native_frontend_hudz.h"
#include "port_hud.h"
#include <nfs3hp.h>
#include <fpu.h>
#include <cstring>

/* The race HUD's small functions, decompiled: what src/nfs3hp/port_hud.cpp and
 * the race's generated code call by address.
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
 * its own where the game's function has one, and an adapter under the game's name for the calls by
 * address.  A core of this file calls another, and those of the HUD's other files done so
 * (native_frontend_hudr.cpp, _hudz.cpp), as C++ (guest::direct), and the typed cores of other files
 * too (fe::direct); the rest of the game still by address. */
namespace nfs3hp
{

namespace
{

using fe::args;
using fe::Guest;
using x86::Float;
using x86::reg32;
using x86::sreg32;

/* ---------------------------------------------------------------------------
 * The 3D letters' looks
 * ------------------------------------------------------------------------- */

constexpr reg32 kSelectedLook = 0x55b088;          // which one the setters work on; -1 none
constexpr reg32 kLooks = 0x55b08c;
constexpr reg32 kLookBytes = 0x238;
constexpr reg32 kNoLook = 0xffffffff;

constexpr reg32 kLookKind = 0x00;                  // kFlat, kDirectional or kLit
constexpr reg32 kLookInUse = 0x04;                 // the setters leave a look with 0 here alone
constexpr reg32 kLookColours = 0x08;               // kFlat's three
constexpr reg32 kLookDirectionalColour = 0x14;
constexpr reg32 kLookDirection = 0x20;             // kDirectional's: three words
constexpr reg32 kLookTexture = 0x2c;
constexpr reg32 kLookLights = 0x30;                // kLightCount of kLightBytes
constexpr reg32 kLookLightCount = 0x230;
constexpr reg32 kLookLightCountAgain = 0x234;

constexpr std::uint32_t kFlat = 0, kDirectional = 3, kLit = 4;

constexpr reg32 kLightCount = 8;
constexpr reg32 kLightBytes = 0x40;
constexpr reg32 kLightCleared = 0x00;              // set to 0 as a light is added
constexpr reg32 kLightSecond = 0x04;
constexpr reg32 kLightFirst = 0x08;
constexpr reg32 kLightUsed = 0x0c;
constexpr reg32 kLightThird = 0x10;
constexpr reg32 kLightPosition = 0x28;             // three words

constexpr reg32 kNoLookForLight = 0xfffffffe;      // sub_4771c0's answers past a light's number
constexpr reg32 kLightsFull = 0xffffffff;

/* The selected look's address, -1's too: the setters work it out before they
 * look at the number. */
reg32 lookAt(reg32 number)
{
    return kLooks + number * kLookBytes;
}

bool lookUsable(const Guest& guest, reg32 number, reg32 look)
{
    return number != kNoLook && guest.dword(look + kLookInUse) != 0;
}

/* Three words copied one at a time, as the game copies them: `from` may be
 * anywhere, the look itself too. */
void copyTriple(Guest& guest, reg32 to, reg32 from)
{
    for (reg32 i = 0; i < 3; ++i)
        guest.setDword(to + 4 * i, guest.dword(from + 4 * i));
}

}  // namespace

/* sub_474d50: the selected look's lights off, if it has any.  It does not look
 * at the number: its callers have.  It leaves eax the look's offset when there
 * was nothing to do, and the lights' count when there was. */
x86::reg32 frontend::hudx::clearLights(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const reg32 number = guest.dword(kSelectedLook);
    const reg32 look = lookAt(number);
    if (guest.dword(look + kLookLightCount) == 0)
        return number * kLookBytes;
    guest.setDword(look + kLookLightCountAgain, 0);
    guest.setDword(look + kLookLightCount, 0);
    for (reg32 i = 0; i < kLightCount; ++i)
        guest.setDword(look + kLookLights + i * kLightBytes + kLightUsed, 0);
    return kLightCount;
}

/* sub_477070: the selected look flat, in three colours; its lights go. */
x86::reg32 frontend::hudx::setFlat(App* app, CPU& cpu, reg32 first, reg32 second, reg32 third)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    const reg32 number = guest.dword(kSelectedLook);
    const reg32 look = lookAt(number);
    guest.leaveEdx(number);
    if (!lookUsable(guest, number, look))
        return look;
    guest.setDword(look + kLookKind, kFlat);
    guest.setDword(look + kLookColours, first);
    guest.setDword(look + kLookColours + 4, second);
    guest.setDword(look + kLookColours + 8, third);
    // The game's code hands it the look in eax, which it does not read.
    return guest::direct<0x474d50, clearLights>(app, cpu);
}

/* sub_477170: the selected look lit from one direction, as the countdown has
 * it; its lights go. */
x86::reg32 frontend::hudx::setDirectional(App* app, CPU& cpu, reg32 colour, reg32 direction)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    const reg32 number = guest.dword(kSelectedLook);
    const reg32 look = lookAt(number);
    guest.leaveEbx(number);
    if (!lookUsable(guest, number, look))
        return look;
    guest.setDword(look + kLookKind, kDirectional);
    guest.setDword(look + kLookDirectionalColour, colour);
    copyTriple(guest, look + kLookDirection, direction);
    // The game's code hands it the look in eax, which it does not read.
    return guest::direct<0x474d50, clearLights>(app, cpu);
}

/* sub_4771c0: a light added to the selected look, into the first free of its
 * eight: its number, or kNoLookForLight, or kLightsFull. */
x86::reg32 frontend::hudx::addLight(App* app, CPU& cpu, reg32 first, reg32 position, reg32 third, reg32 second)
{
    Guest guest(app, cpu);
    const reg32 number = guest.dword(kSelectedLook);
    const reg32 look = lookAt(number);
    if (!lookUsable(guest, number, look))
    {
        guest.leaveEdx(number);
        guest.leaveEcx(kNoLookForLight);
        return kNoLookForLight;
    }
    guest.setDword(look + kLookKind, kLit);
    reg32 index = 0;
    while (index < kLightCount && guest.dword(look + kLookLights + index * kLightBytes + kLightUsed) != 0)
        ++index;
    if (index == kLightCount)
    {
        guest.leaveEdx(look + (kLightCount - 1) * kLightBytes);
        guest.leaveEcx(kLightsFull);
        return kLightsFull;
    }
    const reg32 light = look + kLookLights + index * kLightBytes;
    guest.setDword(light + kLightCleared, 0);
    guest.setDword(light + kLightUsed, 1);
    guest.setDword(light + kLightSecond, second);
    guest.setDword(light + kLightThird, third);
    guest.setDword(light + kLightFirst, first);
    copyTriple(guest, light + kLightPosition, position);
    const reg32 count = guest.dword(look + kLookLightCount) + 1;
    const reg32 countAgain = guest.dword(look + kLookLightCountAgain) + 1;
    guest.setDword(look + kLookLightCount, count);
    guest.setDword(look + kLookLightCountAgain, countAgain);
    guest.leaveEbx(count);
    guest.leaveEcx(index);
    guest.leaveEdx(index * kLightBytes);
    return index;
}

/* sub_477470: the selected look's texture. */
x86::reg32 frontend::hudx::setTexture(App* app, CPU& cpu, reg32 texture)
{
    Guest guest(app, cpu);
    const reg32 number = guest.dword(kSelectedLook);
    const reg32 look = lookAt(number);
    if (lookUsable(guest, number, look))
        guest.setDword(look + kLookTexture, texture);
    return look;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The HUD's records and panes
 * ------------------------------------------------------------------------- */

constexpr reg32 kPaneUp = 0x791b1c;
constexpr reg32 kRebuildAfterDefaults = 0x55d440;  // native_frontend_hudedit.cpp reads it

constexpr reg32 kMapScaleOne = 0x3f800000;         // 1.0f, its bits: compared as a word

constexpr reg32 kCarFlags = 0x200;                 // byte
constexpr std::uint8_t kCarIsCop = 0x20;
constexpr reg32 kSetPane = 0x4844c0;               // eax a view
constexpr reg32 kElements = 23;
constexpr reg32 kRebuildBand = 0x4800a0;           // eax a view
constexpr reg32 kZoomStepped = 0x48ea20;           // eax a view, edx the step

constexpr reg32 kPaneResults = 0x7925d0;           // by view: what sub_481f90 gave
constexpr reg32 kPaneColours = 0x792880;           // by view: which of the box's colours

/* In split screen view 0 has player 1's record and view 1 player 2's. */
reg32 playerOfView(const Guest& guest, reg32 view)
{
    return view + (gameSettings(guest).playMode == 1 ? 1 : 0);
}

/* Whether the car a view follows is a cop's: which of the player's records. */
reg32 copOfView(Guest& guest, x86::CPU& cpu, reg32 view)
{
    const reg32 car = fe::direct<0x422bd0, carstate::targetOfCamera>(guest.app(), cpu, view);
    return (guest.byte(car + kCarFlags) & kCarIsCop) != 0 ? 1 : 0;
}

void toggle(reg32& field)
{
    field = field == 0 ? 1 : 0;
}

}  // namespace

/* sub_484260: a view's HUD on or off, its pane set afresh.  Only view 0 has one
 * outside split screen.  With no pane up yet and kNetPlayers at 2 it is on, not
 * toggled, and in split screen the other view's pane is set too. */
x86::reg32 frontend::hudx::toggleShown(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    const reg32 player = playerOfView(guest, view);
    const reg32 cop = copOfView(guest, cpu, view);
    if (view != 0 && gameSettings(guest).playMode != 1)
        return cop;
    reg32& shown = gameSettings(guest).hudOf(player, cop).shown;
    if (guest.dword(kPaneUp) == 0 && gameSettings(guest).replayMode == 2)
    {
        shown = 1;
        const bool split = gameSettings(guest).playMode == 1;
        guest.setDword(kPaneUp, 1);
        if (split)
            fe::direct<0x4844c0, port::hud::hudPaneSetOf>(app, cpu, cpu.esp - 4,
                                                          port::Args{ 1 - view, cpu.edx, cpu.ebx, cpu.ecx });
    }
    else
    {
        toggle(shown);
    }
    return reg32(*fe::direct<0x4844c0, port::hud::hudPaneSetOf>(app, cpu, cpu.esp - 4,
                                                                 port::Args{ view, cpu.edx, cpu.ebx, cpu.ecx }));
}

/* sub_484450: the rear-view mirror on or off, in a race of one view only. */
x86::reg32 frontend::hudx::toggleRearView(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    if (gameSettings(guest).playMode == 1 || gameSettings(guest).replayMode != 0)
        return guest.eax();
    const reg32 cop = copOfView(guest, cpu, 0);
    toggle(gameSettings(guest).hudOf(0, cop).rearView);
    // By address: the function goes out with ebx, ecx, esi and edi as sub_4844c0 leaves them.
    return guest.call(kSetPane, args({ 0 }));
}

/* sub_485640: a view's pane, its elements and its band made afresh. */
x86::reg32 frontend::hudx::rebuildView(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    fe::direct<0x4844c0, port::hud::hudPaneSetOf>(app, cpu, cpu.esp - 4, port::Args{ view, cpu.edx, cpu.ebx, cpu.ecx });
    for (reg32 element = 0; element < kElements; ++element)
        guest::direct<0x4853c0, hudr::rebuildElement>(app, cpu, view, element);
    return guest.call(kRebuildBand, args({ view }));
}

/* sub_48ccf0: where a view's map pane is now, and from that which colour its
 * box takes: 0 at scale 1, 1 at any other, 2 with no place at all. */
x86::reg32 frontend::hudx::refreshPane(App* app, CPU& cpu, reg32 view)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 24);
    const reg32 player = playerOfView(guest, view);
    const reg32 cop = copOfView(guest, cpu, view);
    const reg32 scale = gameSettings(guest).hudOf(player, cop).mapScaleBits;
    const reg32 place = guest::direct<0x481f90, hudz::mapPaneIsSmall>(app, cpu, view);
    guest.setDword(kPaneResults + 4 * view, place);
    const reg32 colour = place == 0 ? 2 : scale == kMapScaleOne ? 0 : 1;
    guest.setDword(kPaneColours + 4 * view, colour);
    return place;
}

/* sub_48eb60: the zoom stepped in view 0 and, in split screen, in view 1. */
x86::reg32 frontend::hudx::stepZoom(App* app, CPU& cpu, reg32 step)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 12);
    reg32 result = guest.call(kZoomStepped, args({ 0, step }));
    if (gameSettings(guest).playMode == 1)
        result = guest.call(kZoomStepped, args({ 1, step }));
    return result;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The HUD's rectangle groups
 * ------------------------------------------------------------------------- */

constexpr reg32 kGroupCounts = 0x55d694;           // by group: how many rectangles (sub_487350 adds them)

/* ---------------------------------------------------------------------------
 * The HUD's surface
 * ------------------------------------------------------------------------- */

constexpr reg32 kSurfaceVertexCount = 0x00;
constexpr reg32 kSurfaceVertices = 0x04;           // kSurfaceSize of kVertexBytes
constexpr reg32 kSurfaceIndices = 0x8004;          // kSurfaceSize words
constexpr reg32 kSurfaceSize = 0x400;
constexpr reg32 kVertexBytes = 0x20;
constexpr reg32 kVertexU = 0x08;
constexpr reg32 kVertexV = 0x0c;
constexpr reg32 kVertexFlags = 0x14;
constexpr reg32 kInsideTexelLow = 0x37800080;      // a float just above 1/65536
constexpr reg32 kInsideTexelHigh = 0x3f7fff00;     // a float just below 1

}  // namespace

/* sub_49bef0: a surface empty, its indices in order, its vertices at the same
 * texture coordinates.  It leaves eax the size. */
x86::reg32 frontend::hudx::resetSurface(App* app, CPU& cpu, reg32 surface)
{
    Guest guest(app, cpu);
    guest.setDword(surface + kSurfaceVertexCount, 0);
    for (reg32 i = 0; i < kSurfaceSize; ++i)
    {
        const reg32 vertex = surface + kSurfaceVertices + i * kVertexBytes;
        guest.setDword(surface + kSurfaceIndices + i * 4, i);
        guest.setDword(vertex + kVertexU, kInsideTexelLow);
        guest.setDword(vertex + kVertexV, kInsideTexelHigh);
        guest.setDword(vertex + kVertexFlags, 0);
    }
    return kSurfaceSize;
}

/* sub_49bf40: `pairs` pairs of vertices taken from a surface, the first one's
 * address put at `first`. */
x86::reg32 frontend::hudx::takeVertices(App* app, CPU& cpu, reg32 surface, reg32 pairs, reg32 first)
{
    Guest guest(app, cpu);
    guest.setDword(first, surface + kSurfaceVertices + guest.dword(surface + kSurfaceVertexCount) * kVertexBytes);
    guest.leaveEdx(pairs * 2);
    guest.setDword(surface + kSurfaceVertexCount, guest.dword(surface + kSurfaceVertexCount) + pairs * 2);
    return surface;
}

namespace
{

/* ---------------------------------------------------------------------------
 * The HUD's text
 * ------------------------------------------------------------------------- */

constexpr reg32 kTextWidthOverhang = 0x4d1480;     // eax the text: its width onto the x87's stack
constexpr reg32 kDrawText = 0x4d1000;              // eax the text, edx x, ebx y: its width onto the x87's stack
constexpr reg32 kFontSurface = 0x7f74c0, kFontTexture = 0x83f6ac;
constexpr reg32 kFont2Surface = 0x81b4d0, kFont2Texture = 0x83f6d8;
constexpr reg32 kFont = 0x840184;
constexpr reg32 kCharacterSize = 0x84018c;         // a float
constexpr reg32 kCharacterDrops = 0x83fa60;        // int8, by font and character
constexpr reg32 kFontCharacters = 0xe0;

}  // namespace

/* sub_4d10e0: a text ending at x, at the y in ebx.  The width comes off the
 * x87's stack into the sum; what kDrawText leaves there is dropped. */
x86::reg32 frontend::hudx::drawTextToRightEdge(App* app, CPU& cpu, reg32 text, reg32 right, reg32 y)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 16);
    guest.leaveEbx(y);  // kDrawText's y, as the caller hands it
    guest.call(kTextWidthOverhang, args({ text }));
    const Float width = guest.fpu().pop();
    fe::X87 x87(guest);
    const std::int32_t left = x87.truncated(fp::sub(Float(sreg32(right)), width));
    guest.call(kDrawText, args({ text, reg32(left) }));
    guest.fpu().pop();
    return guest.eax();
}

/* sub_4d1310: the two fonts' surfaces begun, each with its own texture. */
x86::reg32 frontend::hudx::beginFontSurfaces(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    const fe::Frame frame(guest, cpu, 8);
    guest::direct<0x49bc80, hudz::queueStart>(app, cpu, kFontSurface, guest.dword(kFontTexture));
    return guest::direct<0x49bc80, hudz::queueStart>(app, cpu, kFont2Surface, guest.dword(kFont2Texture));
}

/* sub_4d14f0: how far a character of the font drops, at the character size. */
x86::reg32 frontend::hudx::characterDrop(App* app, CPU& cpu, reg32 code)
{
    Guest guest(app, cpu);
    const std::uint8_t character = std::uint8_t(code);
    const reg32 at = kCharacterDrops + character + guest.dword(kFont) * kFontCharacters;
    const Float drop = Float(sreg32(std::int8_t(guest.byte(at))));
    fe::X87 x87(guest);
    return reg32(x87.truncated(fp::mul(drop, Float(guest.floatAt(kCharacterSize)))));
}

/* sub_484220: as a race ends, the flag that has the HUD's defaults rebuild it taken down. */
void frontend::hudx::clearRebuildFlag(App* app, CPU& cpu)
{
    Guest guest(app, cpu);
    guest.setDword(kRebuildAfterDefaults, 0);
}

/* sub_487340: a group of the HUD's rectangles empty. */
x86::reg32 frontend::hudx::groupClearCount(App* app, CPU& cpu, reg32 group)
{
    Guest guest(app, cpu);
    guest.setDword(kGroupCounts + group * 4, 0);
    return group;
}

/* sub_4873c0: a group's last rectangle gone, if it has one.  It leaves eax the count's offset. */
x86::reg32 frontend::hudx::groupDropLast(App* app, CPU& cpu, reg32 group)
{
    Guest guest(app, cpu);
    const reg32 offset = group * 4;
    const reg32 count = guest.dword(kGroupCounts + offset);
    if (count != 0)
        guest.setDword(kGroupCounts + offset, count - 1);
    return offset;
}

/* sub_497590 (stdcall 8): the length of the vector x, y on the stack, on the x87's stack.
 * eax comes back as it went: the function saves it. */
void frontend::hudx::vectorLength(App* app, CPU& cpu, reg32 xBits, reg32 yBits)
{
    Guest guest(app, cpu);
    float xValue, yValue;
    std::memcpy(&xValue, &xBits, sizeof xValue);
    std::memcpy(&yValue, &yBits, sizeof yValue);
    const Float x(xValue);
    const Float y(yValue);
    const Float xSquared = fp::mul(x, x);
    const Float ySquared = fp::mul(y, y);
    guest.fpu().push(fp::sqrt(fp::add(xSquared, ySquared)));
}

/* ---------------------------------------------------------------------------
 * Under the game's addresses
 * ------------------------------------------------------------------------- */

void frontend::textLookClearLights(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_474d50", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::clearLights(app, cpu); });
}

void frontend::textLookSetFlat(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477070", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 first = cpu.eax, second = cpu.edx, third = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::setFlat(app, cpu, first, second, third); });
}

void frontend::textLookSetDirectional(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477170", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 colour = cpu.eax, direction = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::setDirectional(app, cpu, colour, direction); });
}

void frontend::textLookAddLight(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4771c0", 0, 0, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 first = cpu.eax, position = cpu.edx, third = cpu.ebx, second = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::addLight(app, cpu, first, position, third, second); });
}

void frontend::textLookSetTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477470", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 texture = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::setTexture(app, cpu, texture); });
}

void frontend::hudClearRebuildFlag(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_484220", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) {
        hudx::clearRebuildFlag(app, cpu);
        return cpu.eax;
    });
}

void frontend::hudToggleShown(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_484260", 0, 0, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::toggleShown(app, cpu, view); });
}

void frontend::hudToggleRearView(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_484450", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::toggleRearView(app, cpu); });
}

void frontend::hudRebuildView(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_485640", 0, 0, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::rebuildView(app, cpu, view); });
}

void frontend::hudGroupClearCount(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_487340", 0, 0, nullptr, 0, fe::kRestoreEdx };
    const reg32 group = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::groupClearCount(app, cpu, group); });
}

void frontend::hudGroupDropLast(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4873c0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 group = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::groupDropLast(app, cpu, group); });
}

void frontend::hudRefreshPane(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_48ccf0", 0, 0, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::refreshPane(app, cpu, view); });
}

void frontend::hudStepZoom(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_48eb60", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 step = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::stepZoom(app, cpu, step); });
}

void frontend::hudVectorLength(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_497590", 8, 0, nullptr, 0, 0 };
    const reg32 x = app->getMemory<reg32>(cpu.esp + 4), y = app->getMemory<reg32>(cpu.esp + 8);
    fe::runWhole(app, cpu, site, [&](Guest&) {
        hudx::vectorLength(app, cpu, x, y);
        return cpu.eax;
    });
}

void frontend::hudSurfaceReset(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49bef0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 surface = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::resetSurface(app, cpu, surface); });
}

void frontend::hudSurfaceTakeVertices(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49bf40", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 surface = cpu.eax, pairs = cpu.edx, first = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::takeVertices(app, cpu, surface, pairs, first); });
}

void frontend::hudTextToRightEdge(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d10e0", 0, 0, nullptr, 0, fe::kRestoreEcx };
    const reg32 text = cpu.eax, right = cpu.edx, y = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::drawTextToRightEdge(app, cpu, text, right, y); });
}

void frontend::hudFontSurfacesBegin(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d1310", 0, 0, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::beginFontSurfaces(app, cpu); });
}

void frontend::hudCharacterDrop(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d14f0", 0, 0, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 character = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest&) { return hudx::characterDrop(app, cpu, character); });
}

}
