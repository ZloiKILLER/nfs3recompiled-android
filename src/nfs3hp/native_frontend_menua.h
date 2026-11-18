#ifndef NFS3HP_NATIVE_FRONTEND_MENUA_H
#define NFS3HP_NATIVE_FRONTEND_MENUA_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstdint>

/* native_frontend_menua.cpp's typed functions (the controls' list, the colour picker, the network
 * lobby's panel, what may be typed, the screen modes' list, the race's pause menu), which other
 * files call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `cpu` and throws
 * fe::Terminated when the game stopped in one, as a call by address does; its result is what the game's
 * function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace menua
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x43be40: what a joystick is doing, as a binding, for the list waiting for one; 0 for nothing.
reg32 pollJoystickBinding(App* app, CPU& cpu);
// The game's 0x43c110: a binding's two lines at a place of the list (the label in eax unused; chosen on the stack).
reg32 drawBinding(App* app, CPU& cpu, reg32 label, reg32 binding, reg32 left, reg32 y, reg32 chosen);
// The game's 0x43c900: the controls' list on every pass.
reg32 controlsList(App* app, CPU& cpu, reg32 item, reg32 pointerWanted);
// The game's 0x43cda0: the key just pressed as a binding, or what the joystick gave.
reg32 keyAsBinding(App* app, CPU& cpu, reg32 character);
// The game's 0x43ce80: in a race, a binding of the other kind (digital or not) for the steering and the
// pedals has a message.
reg32 warnDigitalBinding(App* app, CPU& cpu, reg32 slot, reg32 binding);
// The game's 0x43cf80: the controls' list's keys (eax the item, edx the screen, bx the key).
reg32 listKey(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key);
// The game's 0x43d2b0: the box the controls' list takes the pointer in.
reg32 listFitBox(App* app, CPU& cpu, reg32 item);
// The game's 0x43d310: the controls' list as its screen opens.
reg32 listPrepare(App* app, CPU& cpu, reg32 item);
// The game's 0x43d650: waiting until the joysticks are let go.
reg32 waitJoystickRelease(App* app, CPU& cpu);
// The game's 0x43d6a0: a binding taken away from the rows that had it.
reg32 clearConflictingBindings(App* app, CPU& cpu, reg32 binding);
// The game's 0x43d7f0: a key binding's name into buffer.
reg32 bindingKeyName(App* app, CPU& cpu, reg32 binding, reg32 buffer);
// The game's 0x43d920: where each axis of the devices that are on rests now.
reg32 sampleAxesAtRest(App* app, CPU& cpu, reg32 eax);
// The game's 0x43db20: the colour picker's bar's texture, the hues from top to bottom.
reg32 makeHueRamp(App* app, CPU& cpu);
// The game's 0x43dc90: the picker's colour on the player's car.
reg32 applyColourToCar(App* app, CPU& cpu, reg32 item);
// The game's 0x43dd90: the colour taken, the picker gone.
reg32 acceptColour(App* app, CPU& cpu, reg32 item);
// The game's 0x43ddd0: the colour as it was, the picker gone.
reg32 revertColour(App* app, CPU& cpu, reg32 item);
// The game's 0x43de20: a colour of hue, saturation and value as opaque ARGB into [out].
reg32 hsvToArgb(App* app, CPU& cpu, reg32 hsv, reg32 out);
// The game's 0x43de60: where a colour puts its marker in the picker's triangle (outX, outY on the stack).
reg32 triangleFromColour(App* app, CPU& cpu, std::int32_t width, std::int32_t height, std::int32_t across,
                         std::int32_t down, reg32 outX, reg32 outY);
// The game's 0x43def0: back from a place in the picker's triangle to the colour's two numbers (x, y on the stack).
reg32 colourAtPlace(App* app, CPU& cpu, std::int32_t width, std::int32_t height, reg32 outAcross, reg32 outDown,
                    std::int32_t x, std::int32_t y);
// The game's 0x43dfb0: the picker's back, frame and title.
reg32 drawPickerFrame(App* app, CPU& cpu, reg32 item);
// The game's 0x43e040: the swatch of the colour, with its shadow.
reg32 drawColourSwatch(App* app, CPU& cpu, reg32 item);
// The game's 0x43e0d0: the picker's two buttons, the one under the pointer lit.
reg32 drawPickerButtons(App* app, CPU& cpu, reg32 item);
// The game's 0x43e1f0: the markers of the hue on the bar and of the colour in the triangle.
reg32 drawColourMarkers(App* app, CPU& cpu, reg32 item);
// The game's 0x43e310: the triangle of the colour's hue.
reg32 drawColourTriangle(App* app, CPU& cpu, reg32 item);
// The game's 0x43e490: the picker's bar of hues.
reg32 drawValueBar(App* app, CPU& cpu, reg32 item);
// The game's 0x43e7a0: the picker's area pressed followed by the pointer.
reg32 pickerDragMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x43e8e0: which of the picker's areas the pointer is on.
reg32 pickerHitRegion(App* app, CPU& cpu, reg32 item);
// The game's 0x43ead0: the mouse on the picker.
reg32 pickerMouse(App* app, CPU& cpu, reg32 item);
// The game's 0x43ebc0: the picker on every pass, while it is up.
reg32 drawColourPicker(App* app, CPU& cpu, reg32 item);
// The game's 0x43ec40: the picker's keys (eax the item, edx the screen, bx the key).
reg32 pickerKey(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key);
// The game's 0x43ef80: what the picker has the pointer show over it.
reg32 pickerPointerKind(App* app, CPU& cpu, reg32 item);
// The game's 0x43eff0: the box the picker takes the pointer in.
reg32 pickerFitBox(App* app, CPU& cpu, reg32 item);
// The game's 0x43f030: the picker as the screen comes up.
reg32 pickerPrepare(App* app, CPU& cpu, reg32 item);
// The game's 0x43f160: whether the picker is shown now.
reg32 pickerShown(App* app, CPU& cpu, reg32 item);
// The game's 0x440b30: the icon of a player taking part.
reg32 drawPlayerIcon(App* app, CPU& cpu, reg32 x, reg32 y);
// The game's 0x440b50: the icon of a player of the list, in a glow that pulses.
reg32 drawPulsingIcon(App* app, CPU& cpu, reg32 chosen, reg32 index, reg32 x, reg32 y);
// The game's 0x440d60: the pointer put on the lobby panel's messages.
reg32 pointAtPanel(App* app, CPU& cpu);
// The game's 0x440d80: the lobby panel over a network race's pause.
reg32 drawRacePanel(App* app, CPU& cpu);
// The game's 0x441040: the lobby panel beside a network screen on every pass.
reg32 panelFrame(App* app, CPU& cpu);
// The game's 0x441680: whether the lobby panel is to have the keys and the pointer.
reg32 panelBusy(App* app, CPU& cpu);
// The game's 0x441700: a key for the lobby panel (ax the key).
reg32 panelKey(App* app, CPU& cpu, std::uint16_t key);
// The game's 0x4418e0: how wide a lobby message may be.
reg32 panelLineWidth(App* app, CPU& cpu);
// The game's 0x441900: whether a message fits a line of the lobby panel.
reg32 panelLineFits(App* app, CPU& cpu, reg32 text);
// The game's 0x441950: the lobby panel's messages and the line being typed.
reg32 drawPanelMessages(App* app, CPU& cpu, reg32 x, reg32 y);
// The game's 0x441b20: a key for the lobby panel's typed line (its low word the key).
reg32 typeInPanel(App* app, CPU& cpu, reg32 pressed);
// The game's 0x4434f0: whether a character may be typed at all.
reg32 characterInRange(App* app, CPU& cpu, reg32 character);
// The game's 0x443510: the same for a byte.
reg32 byteInRange(App* app, CPU& cpu, reg32 character);
// The game's 0x443520: whether every byte of a text may be typed.
reg32 textInRange(App* app, CPU& cpu, reg32 text);
// The game's 0x4435c0: whether a character is one of a number or a network address.
reg32 characterOfAddress(App* app, CPU& cpu, reg32 character);
// The game's 0x4435e0: whether a whole text is a number or a network address.
reg32 textOfAddress(App* app, CPU& cpu, reg32 text);
// The game's 0x443620: the same as the game's 0x443510.
reg32 byteInRangeAgain(App* app, CPU& cpu, reg32 character);
// The game's 0x4436c0: a character by the language's first test.
reg32 languageTestFirst(App* app, CPU& cpu, reg32 character);
// The game's 0x443740: a character by the language's second test.
reg32 languageTestSecond(App* app, CPU& cpu, reg32 character);
// The game's 0x4437c0: a character by the language's third test.
reg32 languageTestThird(App* app, CPU& cpu, reg32 character);
// The game's 0x443810: whether a character may be typed in a line of a kind.
reg32 characterAllowed(App* app, CPU& cpu, reg32 character, reg32 kind);
// The game's 0x448210: a mode for the Screen Size list (whether it has a depth buffer on the stack).
reg32 addScreenMode(App* app, CPU& cpu, reg32 width, reg32 height, reg32 bits, reg32 depth, reg32 depthBuffer);
// The game's 0x4482c0: the screen modes' list made afresh from the driver.
reg32 listModes(App* app, CPU& cpu);
// The game's 0x4482f0: the screen modes' list made, the mode chosen still in it, its width.
reg32 chosenModeWidth(App* app, CPU& cpu);
// The game's 0x448330: the mode chosen copied to [out] (out in esi).
reg32 copyChosenMode(App* app, CPU& cpu, reg32 out);
// The game's 0x4483a0: the screen modes' list made, the mode of a width chosen.
reg32 chooseModeOfWidth(App* app, CPU& cpu, reg32 width);
// The game's 0x44b950: the pause menu as a race pauses for the first time.
reg32 racePauseMenuBegin(App* app, CPU& cpu);
// The game's 0x44ba30: the pause menu as a race pauses again.
reg32 racePauseMenuAgain(App* app, CPU& cpu);
// The game's 0x44bac0: a pass of the pause menu.
reg32 racePauseMenuFrame(App* app, CPU& cpu);

}  // namespace menua
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
