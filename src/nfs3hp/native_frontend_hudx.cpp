#include "native_frontend.h"
#include <nfs3hp.h>
#include <fpu.h>

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
 * with them. */
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

constexpr reg32 kClearLights = 0x474d50;

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

/* sub_474d50: the selected look's lights off, if it has any.  It does not look
 * at the number: its callers have.  It leaves eax the look's offset when there
 * was nothing to do, and the lights' count when there was. */
reg32 clearLights(Guest& guest)
{
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
reg32 setFlat(Guest& guest, reg32 first, reg32 second, reg32 third)
{
    const reg32 number = guest.dword(kSelectedLook);
    const reg32 look = lookAt(number);
    guest.leaveEdx(number);
    if (!lookUsable(guest, number, look))
        return look;
    guest.setDword(look + kLookKind, kFlat);
    guest.setDword(look + kLookColours, first);
    guest.setDword(look + kLookColours + 4, second);
    guest.setDword(look + kLookColours + 8, third);
    return guest.call(kClearLights, args({ look }));
}

/* sub_477170: the selected look lit from one direction, as the countdown has
 * it; its lights go. */
reg32 setDirectional(Guest& guest, reg32 colour, reg32 direction)
{
    const reg32 number = guest.dword(kSelectedLook);
    const reg32 look = lookAt(number);
    guest.leaveEbx(number);
    if (!lookUsable(guest, number, look))
        return look;
    guest.setDword(look + kLookKind, kDirectional);
    guest.setDword(look + kLookDirectionalColour, colour);
    copyTriple(guest, look + kLookDirection, direction);
    return guest.call(kClearLights, args({ look }));
}

/* sub_4771c0: a light added to the selected look, into the first free of its
 * eight: its number, or kNoLookForLight, or kLightsFull. */
reg32 addLight(Guest& guest, reg32 first, reg32 position, reg32 second, reg32 third)
{
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
reg32 setTexture(Guest& guest, reg32 texture)
{
    const reg32 number = guest.dword(kSelectedLook);
    const reg32 look = lookAt(number);
    if (lookUsable(guest, number, look))
        guest.setDword(look + kLookTexture, texture);
    return look;
}

/* ---------------------------------------------------------------------------
 * The HUD's records and panes
 * ------------------------------------------------------------------------- */

constexpr reg32 kSplitScreen = 0x6fd3b0;           // 1 in split screen
constexpr reg32 kNetPlayers = 0x6fd3a0;            // native_frontend_pause.cpp's name; see toggleShown
constexpr reg32 kPaneUp = 0x791b1c;
constexpr reg32 kRebuildAfterDefaults = 0x55d440;  // native_frontend_hudedit.cpp reads it

constexpr reg32 kRecords = 0x6fbc50;
constexpr reg32 kPlayerRecord = 0x348;
constexpr reg32 kCopRecord = 0x1a4;
constexpr reg32 kShown = 0x00;
constexpr reg32 kRearView = 0x0c;
constexpr reg32 kMapScale = 0x10;                  // a float
constexpr reg32 kMapScaleOne = 0x3f800000;         // 1.0f, its bits: compared as a word

constexpr reg32 kCameraTarget = 0x422bd0;          // eax a view: the car it follows
constexpr reg32 kCarFlags = 0x200;                 // byte
constexpr std::uint8_t kCarIsCop = 0x20;
constexpr reg32 kSetPane = 0x4844c0;               // eax a view
constexpr reg32 kRebuildElement = 0x4853c0;        // eax a view, edx an element
constexpr reg32 kElements = 23;
constexpr reg32 kRebuildBand = 0x4800a0;           // eax a view
constexpr reg32 kPanePlace = 0x481f90;             // eax a view
constexpr reg32 kZoomStepped = 0x48ea20;           // eax a view, edx the step

constexpr reg32 kPaneResults = 0x7925d0;           // by view: what kPanePlace gave
constexpr reg32 kPaneColours = 0x792880;           // by view: which of the box's colours

/* The record of a player, racer or cop. */
reg32 record(reg32 player, reg32 cop)
{
    return kRecords + player * kPlayerRecord + cop * kCopRecord;
}

/* In split screen view 0 has player 1's record and view 1 player 2's. */
reg32 playerOfView(const Guest& guest, reg32 view)
{
    return view + (guest.dword(kSplitScreen) == 1 ? 1 : 0);
}

/* Whether the car a view follows is a cop's: which of the player's records. */
reg32 copOfView(Guest& guest, reg32 view)
{
    const reg32 car = guest.call(kCameraTarget, args({ view }));
    return (guest.byte(car + kCarFlags) & kCarIsCop) != 0 ? 1 : 0;
}

void toggle(Guest& guest, reg32 address)
{
    guest.setDword(address, guest.dword(address) == 0 ? 1 : 0);
}

/* sub_484260: a view's HUD on or off, its pane set afresh.  Only view 0 has one
 * outside split screen.  With no pane up yet and kNetPlayers at 2 it is on, not
 * toggled, and in split screen the other view's pane is set too. */
reg32 toggleShown(Guest& guest, reg32 view)
{
    const reg32 player = playerOfView(guest, view);
    const reg32 cop = copOfView(guest, view);
    if (view != 0 && guest.dword(kSplitScreen) != 1)
        return cop;
    const reg32 shown = record(player, cop) + kShown;
    if (guest.dword(kPaneUp) == 0 && guest.dword(kNetPlayers) == 2)
    {
        guest.setDword(shown, 1);
        const bool split = guest.dword(kSplitScreen) == 1;
        guest.setDword(kPaneUp, 1);
        if (split)
            guest.call(kSetPane, args({ 1 - view }));
    }
    else
    {
        toggle(guest, shown);
    }
    return guest.call(kSetPane, args({ view }));
}

/* sub_484450: the rear-view mirror on or off, in a race of one view only. */
reg32 toggleRearView(Guest& guest)
{
    if (guest.dword(kSplitScreen) == 1 || guest.dword(kNetPlayers) != 0)
        return guest.eax();
    const reg32 cop = copOfView(guest, 0);
    toggle(guest, record(0, cop) + kRearView);
    return guest.call(kSetPane, args({ 0 }));
}

/* sub_485640: a view's pane, its elements and its band made afresh. */
reg32 rebuildView(Guest& guest, reg32 view)
{
    guest.call(kSetPane, args({ view }));
    for (reg32 element = 0; element < kElements; ++element)
        guest.call(kRebuildElement, args({ view, element }));
    return guest.call(kRebuildBand, args({ view }));
}

/* sub_48ccf0: where a view's map pane is now, and from that which colour its
 * box takes: 0 at scale 1, 1 at any other, 2 with no place at all. */
reg32 refreshPane(Guest& guest, reg32 view)
{
    const reg32 player = playerOfView(guest, view);
    const reg32 cop = copOfView(guest, view);
    const reg32 scale = guest.dword(record(player, cop) + kMapScale);
    const reg32 place = guest.call(kPanePlace, args({ view }));
    guest.setDword(kPaneResults + 4 * view, place);
    const reg32 colour = place == 0 ? 2 : scale == kMapScaleOne ? 0 : 1;
    guest.setDword(kPaneColours + 4 * view, colour);
    return place;
}

/* sub_48eb60: the zoom stepped in view 0 and, in split screen, in view 1. */
reg32 stepZoom(Guest& guest, reg32 step)
{
    reg32 result = guest.call(kZoomStepped, args({ 0, step }));
    if (guest.dword(kSplitScreen) == 1)
        result = guest.call(kZoomStepped, args({ 1, step }));
    return result;
}

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

/* sub_49bef0: a surface empty, its indices in order, its vertices at the same
 * texture coordinates.  It leaves eax the size. */
reg32 resetSurface(Guest& guest, reg32 surface)
{
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
reg32 takeVertices(Guest& guest, reg32 surface, reg32 pairs, reg32 first)
{
    guest.setDword(first, surface + kSurfaceVertices + guest.dword(surface + kSurfaceVertexCount) * kVertexBytes);
    guest.leaveEdx(pairs * 2);
    guest.setDword(surface + kSurfaceVertexCount, guest.dword(surface + kSurfaceVertexCount) + pairs * 2);
    return surface;
}

/* ---------------------------------------------------------------------------
 * The HUD's text
 * ------------------------------------------------------------------------- */

constexpr reg32 kTextWidthOverhang = 0x4d1480;     // eax the text: its width onto the x87's stack
constexpr reg32 kDrawText = 0x4d1000;              // eax the text, edx x, ebx y: its width onto the x87's stack
constexpr reg32 kSurfaceBegin = 0x49bc80;          // eax a font's surface, edx its texture
constexpr reg32 kFontSurface = 0x7f74c0, kFontTexture = 0x83f6ac;
constexpr reg32 kFont2Surface = 0x81b4d0, kFont2Texture = 0x83f6d8;
constexpr reg32 kFont = 0x840184;
constexpr reg32 kCharacterSize = 0x84018c;         // a float
constexpr reg32 kCharacterDrops = 0x83fa60;        // int8, by font and character
constexpr reg32 kFontCharacters = 0xe0;

/* sub_4d10e0: a text ending at x, at the y in ebx.  The width comes off the
 * x87's stack into the sum; what kDrawText leaves there is dropped. */
reg32 drawTextToRightEdge(Guest& guest, reg32 text, reg32 right)
{
    guest.call(kTextWidthOverhang, args({ text }));
    const Float width = guest.fpu().pop();
    fe::X87 x87(guest);
    const std::int32_t left = x87.truncated(fp::sub(Float(sreg32(right)), width));
    guest.call(kDrawText, args({ text, reg32(left) }));
    guest.fpu().pop();
    return guest.eax();
}

/* sub_4d1310: the two fonts' surfaces begun, each with its own texture. */
reg32 beginFontSurfaces(Guest& guest)
{
    guest.call(kSurfaceBegin, args({ kFontSurface, guest.dword(kFontTexture) }));
    return guest.call(kSurfaceBegin, args({ kFont2Surface, guest.dword(kFont2Texture) }));
}

/* sub_4d14f0: how far a character of the font drops, at the character size. */
reg32 characterDrop(Guest& guest, std::uint8_t character)
{
    const reg32 at = kCharacterDrops + character + guest.dword(kFont) * kFontCharacters;
    const Float drop = Float(sreg32(std::int8_t(guest.byte(at))));
    fe::X87 x87(guest);
    return reg32(x87.truncated(fp::mul(drop, Float(guest.floatAt(kCharacterSize)))));
}

}

void frontend::textLookClearLights(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_474d50", 0, 12, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return clearLights(guest); });
}

void frontend::textLookSetFlat(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477070", 0, 12, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 first = cpu.eax, second = cpu.edx, third = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return setFlat(guest, first, second, third); });
}

void frontend::textLookSetDirectional(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477170", 0, 8, nullptr, 0, fe::kRestoreEcx };
    const reg32 colour = cpu.eax, direction = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return setDirectional(guest, colour, direction); });
}

void frontend::textLookAddLight(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4771c0", 0, 16, nullptr, 0, fe::kRestoreEsi | fe::kRestoreEdi };
    const reg32 first = cpu.eax, position = cpu.edx, third = cpu.ebx, second = cpu.ecx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return addLight(guest, first, position, second, third); });
}

void frontend::textLookSetTexture(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_477470", 0, 12, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 texture = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return setTexture(guest, texture); });
}

void frontend::hudClearRebuildFlag(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_484220: as a race ends, the flag that has the HUD's defaults rebuild it taken down.
    static fe::Site site{ "sub_484220", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) {
        guest.setDword(kRebuildAfterDefaults, 0);
        return guest.eax();
    });
}

void frontend::hudToggleShown(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_484260", 0, 24, nullptr, 0 };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return toggleShown(guest, view); });
}

void frontend::hudToggleRearView(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_484450", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return toggleRearView(guest); });
}

void frontend::hudRebuildView(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_485640", 0, 16, nullptr, 0, fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return rebuildView(guest, view); });
}

void frontend::hudGroupClearCount(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_487340: a group of the HUD's rectangles empty.
    static fe::Site site{ "sub_487340", 0, 8, nullptr, 0, fe::kRestoreEdx };
    const reg32 group = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        guest.setDword(kGroupCounts + group * 4, 0);
        return group;
    });
}

void frontend::hudGroupDropLast(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_4873c0: a group's last rectangle gone, if it has one.  It leaves eax the count's offset.
    static fe::Site site{ "sub_4873c0", 0, 12, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 offset = cpu.eax * 4;
    fe::runWhole(app, cpu, site, [&](Guest& guest) {
        const reg32 count = guest.dword(kGroupCounts + offset);
        if (count != 0)
            guest.setDword(kGroupCounts + offset, count - 1);
        return offset;
    });
}

void frontend::hudRefreshPane(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_48ccf0", 0, 24, nullptr, 0,
                          fe::kRestoreEbx | fe::kRestoreEcx | fe::kRestoreEdx | fe::kRestoreEdi };
    const reg32 view = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return refreshPane(guest, view); });
}

void frontend::hudStepZoom(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_48eb60", 0, 12, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 step = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return stepZoom(guest, step); });
}

void frontend::hudVectorLength(win32::WinApplication* app, x86::CPU& cpu)
{
    // sub_497590 (stdcall 8): the length of the vector x, y on the stack, on the x87's stack.
    // eax comes back as it went: the function saves it.
    static constexpr reg32 kFrameBytes = 8;
    static fe::Site site{ "sub_497590", 8, kFrameBytes, nullptr, 0, 0 };
    fe::runWhole(app, cpu, site, [](Guest& guest) {
        const Float x(guest.floatAt(guest.frame() + kFrameBytes + 4));
        const Float y(guest.floatAt(guest.frame() + kFrameBytes + 8));
        const Float xSquared = fp::mul(x, x);
        const Float ySquared = fp::mul(y, y);
        guest.fpu().push(fp::sqrt(fp::add(xSquared, ySquared)));
        return guest.eax();
    });
}

void frontend::hudSurfaceReset(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49bef0", 0, 12, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const reg32 surface = cpu.eax;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return resetSurface(guest, surface); });
}

void frontend::hudSurfaceTakeVertices(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_49bf40", 0, 12, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEsi };
    const reg32 surface = cpu.eax, pairs = cpu.edx, first = cpu.ebx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return takeVertices(guest, surface, pairs, first); });
}

void frontend::hudTextToRightEdge(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d10e0", 0, 16, nullptr, 0, fe::kRestoreEcx };
    const reg32 text = cpu.eax, right = cpu.edx;
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return drawTextToRightEdge(guest, text, right); });
}

void frontend::hudFontSurfacesBegin(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d1310", 0, 8, nullptr, 0, fe::kRestoreEdx };
    fe::runWhole(app, cpu, site, [](Guest& guest) { return beginFontSurfaces(guest); });
}

void frontend::hudCharacterDrop(win32::WinApplication* app, x86::CPU& cpu)
{
    static fe::Site site{ "sub_4d14f0", 0, 20, nullptr, 0, fe::kRestoreEcx | fe::kRestoreEdx };
    const std::uint8_t character = std::uint8_t(cpu.eax);
    fe::runWhole(app, cpu, site, [&](Guest& guest) { return characterDrop(guest, character); });
}

}
