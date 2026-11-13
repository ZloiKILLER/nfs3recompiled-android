#ifndef NFS3HP_NATIVE_FRONTEND_MENUA_H
#define NFS3HP_NATIVE_FRONTEND_MENUA_H

#include "native_frontend.h"
#include "native_guest.h"
#include <cstdint>

/* native_frontend_menua.cpp's typed functions (the controls' list, the colour picker, the network
 * lobby's panel, what may be typed, the screen modes' list, the race's pause menu), which other
 * files call as C++ (guest::direct), and how the game's code calls each by address (ByAddress, for
 * the checks' stand-ins).  Each calls the game's other functions by address on `cpu` and throws
 * fe::Terminated when the game stopped in one, as Guest::call does; its result is what the game's
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
reg32 sampleAxesAtRest(App* app, CPU& cpu);
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

template <>
struct ByAddress<frontend::menua::pollJoystickBinding>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menua::drawBinding>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 label, x86::reg32 binding,
                           x86::reg32 left, x86::reg32 y, x86::reg32 chosen)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ label, binding, left, y }, { chosen }));
    }
};

template <>
struct ByAddress<frontend::menua::controlsList>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item, x86::reg32 pointerWanted)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, pointerWanted }));
    }
};

template <>
struct ByAddress<frontend::menua::keyAsBinding>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 character)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ character }));
    }
};

template <>
struct ByAddress<frontend::menua::warnDigitalBinding>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slot, x86::reg32 binding)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ slot, binding }));
    }
};

template <>
struct ByAddress<frontend::menua::listKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item, x86::reg32 menu,
                           std::uint16_t key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, x86::reg32(key) }));
    }
};

template <>
struct ByAddress<frontend::menua::listFitBox>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::listPrepare>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::waitJoystickRelease>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menua::clearConflictingBindings>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 binding)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ binding }));
    }
};

template <>
struct ByAddress<frontend::menua::bindingKeyName>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 binding, x86::reg32 buffer)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ binding, buffer }));
    }
};

template <>
struct ByAddress<frontend::menua::sampleAxesAtRest>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menua::makeHueRamp>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menua::applyColourToCar>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::acceptColour>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::revertColour>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::hsvToArgb>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 hsv, x86::reg32 out)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ hsv, out }));
    }
};

template <>
struct ByAddress<frontend::menua::triangleFromColour>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, std::int32_t width, std::int32_t height,
                           std::int32_t across, std::int32_t down, x86::reg32 outX, x86::reg32 outY)
    {
        return fe::callByAddress(app, cpu, Address,
                                 fe::args({ x86::reg32(width), x86::reg32(height), x86::reg32(across), x86::reg32(down) },
                                          { outX, outY }));
    }
};

template <>
struct ByAddress<frontend::menua::colourAtPlace>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, std::int32_t width, std::int32_t height,
                           x86::reg32 outAcross, x86::reg32 outDown, std::int32_t x, std::int32_t y)
    {
        return fe::callByAddress(app, cpu, Address,
                                 fe::args({ x86::reg32(width), x86::reg32(height), outAcross, outDown },
                                          { x86::reg32(x), x86::reg32(y) }));
    }
};

template <>
struct ByAddress<frontend::menua::drawPickerFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::drawColourSwatch>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::drawPickerButtons>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::drawColourMarkers>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::drawColourTriangle>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::drawValueBar>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::pickerDragMoved>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::pickerHitRegion>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::pickerMouse>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::drawColourPicker>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::pickerKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item, x86::reg32 menu,
                           std::uint16_t key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item, menu, x86::reg32(key) }));
    }
};

template <>
struct ByAddress<frontend::menua::pickerPointerKind>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::pickerFitBox>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::pickerPrepare>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::pickerShown>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 item)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ item }));
    }
};

template <>
struct ByAddress<frontend::menua::drawPlayerIcon>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 x, x86::reg32 y)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ x, y }));
    }
};

template <>
struct ByAddress<frontend::menua::drawPulsingIcon>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 chosen, x86::reg32 index,
                           x86::reg32 x, x86::reg32 y)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ chosen, index, x, y }));
    }
};

template <>
struct ByAddress<frontend::menua::pointAtPanel>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menua::drawRacePanel>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menua::panelFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menua::panelBusy>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menua::panelKey>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, std::uint16_t key)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ x86::reg32(key) }));
    }
};

template <>
struct ByAddress<frontend::menua::panelLineWidth>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menua::panelLineFits>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 text)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ text }));
    }
};

template <>
struct ByAddress<frontend::menua::drawPanelMessages>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 x, x86::reg32 y)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ x, y }));
    }
};

template <>
struct ByAddress<frontend::menua::typeInPanel>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 pressed)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ pressed }));
    }
};

template <>
struct ByAddress<frontend::menua::characterInRange>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 character)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ character }));
    }
};

template <>
struct ByAddress<frontend::menua::byteInRange>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 character)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ character }));
    }
};

template <>
struct ByAddress<frontend::menua::textInRange>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 text)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ text }));
    }
};

template <>
struct ByAddress<frontend::menua::characterOfAddress>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 character)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ character }));
    }
};

template <>
struct ByAddress<frontend::menua::textOfAddress>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 text)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ text }));
    }
};

template <>
struct ByAddress<frontend::menua::byteInRangeAgain>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 character)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ character }));
    }
};

template <>
struct ByAddress<frontend::menua::languageTestFirst>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 character)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ character }));
    }
};

template <>
struct ByAddress<frontend::menua::languageTestSecond>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 character)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ character }));
    }
};

template <>
struct ByAddress<frontend::menua::languageTestThird>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 character)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ character }));
    }
};

template <>
struct ByAddress<frontend::menua::characterAllowed>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 character, x86::reg32 kind)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ character, kind }));
    }
};

template <>
struct ByAddress<frontend::menua::addScreenMode>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width, x86::reg32 height,
                           x86::reg32 bits, x86::reg32 depth, x86::reg32 depthBuffer)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ width, height, bits, depth }, { depthBuffer }));
    }
};

template <>
struct ByAddress<frontend::menua::listModes>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menua::chosenModeWidth>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menua::copyChosenMode>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 out)
    {
        cpu.esi = out;
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menua::chooseModeOfWidth>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 width)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({ width }));
    }
};

template <>
struct ByAddress<frontend::menua::racePauseMenuBegin>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menua::racePauseMenuAgain>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

template <>
struct ByAddress<frontend::menua::racePauseMenuFrame>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        return fe::callByAddress(app, cpu, Address, fe::args({}));
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
