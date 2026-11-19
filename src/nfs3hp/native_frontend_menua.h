#ifndef NFS3HP_NATIVE_FRONTEND_MENUA_H
#define NFS3HP_NATIVE_FRONTEND_MENUA_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstdint>

/* native_frontend_menua.cpp's typed functions (the controls' list, the colour picker, the network
 * lobby's panel, what may be typed, the screen modes' list, the race's pause menu), which other
 * files call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `ts` and throws
 * fe::Terminated when the game stopped in one, as a call by address does; its result is what the game's
 * function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace menua
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x43be40: what a joystick is doing, as a binding, for the list waiting for one; 0 for nothing.
u32 pollJoystickBinding(App* app, ThreadState& ts);
// The game's 0x43c110: a binding's two lines at a place of the list (the label in eax unused; chosen on the stack).
u32 drawBinding(App* app, ThreadState& ts, u32 label, u32 binding, u32 left, u32 y, u32 chosen);
// The game's 0x43c900: the controls' list on every pass.
u32 controlsList(App* app, ThreadState& ts, u32 item, u32 pointerWanted);
// The game's 0x43cda0: the key just pressed as a binding, or what the joystick gave.
u32 keyAsBinding(App* app, ThreadState& ts, u32 character);
// The game's 0x43ce80: in a race, a binding of the other kind (digital or not) for the steering and the
// pedals has a message.
u32 warnDigitalBinding(App* app, ThreadState& ts, u32 slot, u32 binding);
// The game's 0x43cf80: the controls' list's keys (eax the item, edx the screen, bx the key).
u32 listKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key);
// The game's 0x43d2b0: the box the controls' list takes the pointer in.
u32 listFitBox(App* app, ThreadState& ts, u32 item);
// The game's 0x43d310: the controls' list as its screen opens.
u32 listPrepare(App* app, ThreadState& ts, u32 item);
// The game's 0x43d650: waiting until the joysticks are let go.
u32 waitJoystickRelease(App* app, ThreadState& ts);
// The game's 0x43d6a0: a binding taken away from the rows that had it.
u32 clearConflictingBindings(App* app, ThreadState& ts, u32 binding);
// The game's 0x43d7f0: a key binding's name into buffer.
u32 bindingKeyName(App* app, ThreadState& ts, u32 binding, u32 buffer);
// The game's 0x43d920: where each axis of the devices that are on rests now.
u32 sampleAxesAtRest(App* app, ThreadState& ts, u32 eax);
// The game's 0x43db20: the colour picker's bar's texture, the hues from top to bottom.
u32 makeHueRamp(App* app, ThreadState& ts);
// The game's 0x43dc90: the picker's colour on the player's car.
u32 applyColourToCar(App* app, ThreadState& ts, u32 item);
// The game's 0x43dd90: the colour taken, the picker gone.
u32 acceptColour(App* app, ThreadState& ts, u32 item);
// The game's 0x43ddd0: the colour as it was, the picker gone.
u32 revertColour(App* app, ThreadState& ts, u32 item);
// The game's 0x43de20: a colour of hue, saturation and value as opaque ARGB into [out].
u32 hsvToArgb(App* app, ThreadState& ts, u32 hsv, u32 out);
// The game's 0x43de60: where a colour puts its marker in the picker's triangle (outX, outY on the stack).
u32 triangleFromColour(App* app, ThreadState& ts, std::int32_t width, std::int32_t height, std::int32_t across,
                         std::int32_t down, u32 outX, u32 outY);
// The game's 0x43def0: back from a place in the picker's triangle to the colour's two numbers (x, y on the stack).
u32 colourAtPlace(App* app, ThreadState& ts, std::int32_t width, std::int32_t height, u32 outAcross, u32 outDown,
                    std::int32_t x, std::int32_t y);
// The game's 0x43dfb0: the picker's back, frame and title.
u32 drawPickerFrame(App* app, ThreadState& ts, u32 item);
// The game's 0x43e040: the swatch of the colour, with its shadow.
u32 drawColourSwatch(App* app, ThreadState& ts, u32 item);
// The game's 0x43e0d0: the picker's two buttons, the one under the pointer lit.
u32 drawPickerButtons(App* app, ThreadState& ts, u32 item);
// The game's 0x43e1f0: the markers of the hue on the bar and of the colour in the triangle.
u32 drawColourMarkers(App* app, ThreadState& ts, u32 item);
// The game's 0x43e310: the triangle of the colour's hue.
u32 drawColourTriangle(App* app, ThreadState& ts, u32 item);
// The game's 0x43e490: the picker's bar of hues.
u32 drawValueBar(App* app, ThreadState& ts, u32 item);
// The game's 0x43e7a0: the picker's area pressed followed by the pointer.
u32 pickerDragMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x43e8e0: which of the picker's areas the pointer is on.
u32 pickerHitRegion(App* app, ThreadState& ts, u32 item);
// The game's 0x43ead0: the mouse on the picker.
u32 pickerMouse(App* app, ThreadState& ts, u32 item);
// The game's 0x43ebc0: the picker on every pass, while it is up.
u32 drawColourPicker(App* app, ThreadState& ts, u32 item);
// The game's 0x43ec40: the picker's keys (eax the item, edx the screen, bx the key).
u32 pickerKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key);
// The game's 0x43ef80: what the picker has the pointer show over it.
u32 pickerPointerKind(App* app, ThreadState& ts, u32 item);
// The game's 0x43eff0: the box the picker takes the pointer in.
u32 pickerFitBox(App* app, ThreadState& ts, u32 item);
// The game's 0x43f030: the picker as the screen comes up.
u32 pickerPrepare(App* app, ThreadState& ts, u32 item);
// The game's 0x43f160: whether the picker is shown now.
u32 pickerShown(App* app, ThreadState& ts, u32 item);
// The game's 0x440b30: the icon of a player taking part.
u32 drawPlayerIcon(App* app, ThreadState& ts, u32 x, u32 y);
// The game's 0x440b50: the icon of a player of the list, in a glow that pulses.
u32 drawPulsingIcon(App* app, ThreadState& ts, u32 chosen, u32 index, u32 x, u32 y);
// The game's 0x440d60: the pointer put on the lobby panel's messages.
u32 pointAtPanel(App* app, ThreadState& ts);
// The game's 0x440d80: the lobby panel over a network race's pause.
u32 drawRacePanel(App* app, ThreadState& ts);
// The game's 0x441040: the lobby panel beside a network screen on every pass.
u32 panelFrame(App* app, ThreadState& ts);
// The game's 0x441680: whether the lobby panel is to have the keys and the pointer.
u32 panelBusy(App* app, ThreadState& ts);
// The game's 0x441700: a key for the lobby panel (ax the key).
u32 panelKey(App* app, ThreadState& ts, std::uint16_t key);
// The game's 0x4418e0: how wide a lobby message may be.
u32 panelLineWidth(App* app, ThreadState& ts);
// The game's 0x441900: whether a message fits a line of the lobby panel.
u32 panelLineFits(App* app, ThreadState& ts, u32 text);
// The game's 0x441950: the lobby panel's messages and the line being typed.
u32 drawPanelMessages(App* app, ThreadState& ts, u32 x, u32 y);
// The game's 0x441b20: a key for the lobby panel's typed line (its low word the key).
u32 typeInPanel(App* app, ThreadState& ts, u32 pressed);
// The game's 0x4434f0: whether a character may be typed at all.
u32 characterInRange(App* app, ThreadState& ts, u32 character);
// The game's 0x443510: the same for a byte.
u32 byteInRange(App* app, ThreadState& ts, u32 character);
// The game's 0x443520: whether every byte of a text may be typed.
u32 textInRange(App* app, ThreadState& ts, u32 text);
// The game's 0x4435c0: whether a character is one of a number or a network address.
u32 characterOfAddress(App* app, ThreadState& ts, u32 character);
// The game's 0x4435e0: whether a whole text is a number or a network address.
u32 textOfAddress(App* app, ThreadState& ts, u32 text);
// The game's 0x443620: the same as the game's 0x443510.
u32 byteInRangeAgain(App* app, ThreadState& ts, u32 character);
// The game's 0x4436c0: a character by the language's first test.
u32 languageTestFirst(App* app, ThreadState& ts, u32 character);
// The game's 0x443740: a character by the language's second test.
u32 languageTestSecond(App* app, ThreadState& ts, u32 character);
// The game's 0x4437c0: a character by the language's third test.
u32 languageTestThird(App* app, ThreadState& ts, u32 character);
// The game's 0x443810: whether a character may be typed in a line of a kind.
u32 characterAllowed(App* app, ThreadState& ts, u32 character, u32 kind);
// The game's 0x448210: a mode for the Screen Size list (whether it has a depth buffer on the stack).
u32 addScreenMode(App* app, ThreadState& ts, u32 width, u32 height, u32 bits, u32 depth, u32 depthBuffer);
// The game's 0x4482c0: the screen modes' list made afresh from the driver.
u32 listModes(App* app, ThreadState& ts);
// The game's 0x4482f0: the screen modes' list made, the mode chosen still in it, its width.
u32 chosenModeWidth(App* app, ThreadState& ts);
// The game's 0x448330: the mode chosen copied to [out] (out in esi).
u32 copyChosenMode(App* app, ThreadState& ts, u32 out);
// The game's 0x4483a0: the screen modes' list made, the mode of a width chosen.
u32 chooseModeOfWidth(App* app, ThreadState& ts, u32 width);
// The game's 0x44b950: the pause menu as a race pauses for the first time.
u32 racePauseMenuBegin(App* app, ThreadState& ts);
// The game's 0x44ba30: the pause menu as a race pauses again.
u32 racePauseMenuAgain(App* app, ThreadState& ts);
// The game's 0x44bac0: a pass of the pause menu.
u32 racePauseMenuFrame(App* app, ThreadState& ts);

}  // namespace menua
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
