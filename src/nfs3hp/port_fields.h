#ifndef NFS3HP_PORT_FIELDS_H
#define NFS3HP_PORT_FIELDS_H

#include "native_guest.h"
#include "port.h"

/* The menus' widgets that hold a value, rewritten (port.h says how;
 * port_fields.cpp): the fields typed into -- a text (edstring), a whole number
 * (edint), a byte (edchar), a float (edfloat), a 16.16 fixed number (edfixed)
 * --, the list of lines (List), the sliders single and two-axis (slider), the
 * car's colours (carclrselect) and the car records' colours they show and
 * set, with what they draw a field's text and its cursor with.
 *
 * Each is an adapter under the game's function's name, which
 * disassemble_nfs3hp.py binds to its address (PORT_FIELDS_NATIVES); the
 * generated code calls it by name, the widget table (0x557a84) and the other
 * natives through the address. */
namespace nfs3hp
{
namespace port
{
namespace fields
{

/* ---- The car records' colours ---- */
void carColourCount(App* app, CPU& cpu);          // 0x439760
void carCustomColour(App* app, CPU& cpu);         // 0x439780
void carColour(App* app, CPU& cpu);               // 0x439790
void carColourName(App* app, CPU& cpu);           // 0x4397c0
void setCarColour(App* app, CPU& cpu);            // 0x4397d0
void carColourRgb(App* app, CPU& cpu);            // 0x439810
void setCarCustomColour(App* app, CPU& cpu);      // 0x4398c0
void carText(App* app, CPU& cpu);                 // 0x439ab0
void saveCarColours(App* app, CPU& cpu);          // 0x43b7f0
void wheelFromCarColour(App* app, CPU& cpu);      // 0x43dbe0
void wheelForPlayer(App* app, CPU& cpu);          // 0x43dd50
void refreshCustomColours(App* app, CPU& cpu);    // 0x472360

/* ---- A field's text and cursor ---- */
void setCursorColour(App* app, CPU& cpu);         // 0x4513e0
void drawFieldText(App* app, CPU& cpu);           // 0x4515e0
void drawFieldLine(App* app, CPU& cpu);           // 0x452220
void drawSpacedText(App* app, CPU& cpu);          // 0x4522c0
void drawCursorText3d(App* app, CPU& cpu);        // 0x4d11f0
void drawCursor(App* app, CPU& cpu);              // 0x4da560
void drawCursorText(App* app, CPU& cpu);          // 0x4da8c0

/* ---- The car's colours (carclrselect) ---- */
void colourListWidth(App* app, CPU& cpu);         // 0x4569a0
void colourListRefresh(App* app, CPU& cpu);       // 0x456a20
void drawColourLine(App* app, CPU& cpu);          // 0x456b50
void drawColourList(App* app, CPU& cpu);          // 0x456ce0
void colourListShown(App* app, CPU& cpu);         // 0x456ea0
void pointAtColour(App* app, CPU& cpu);           // 0x456f10
void openColourList(App* app, CPU& cpu);          // 0x456f50
void closeColourList(App* app, CPU& cpu);         // 0x456f70
void chooseColour(App* app, CPU& cpu);            // 0x456f90
void colourListKey(App* app, CPU& cpu);           // 0x457030
void layoutColourList(App* app, CPU& cpu);        // 0x4571e0
void colourListHidden(App* app, CPU& cpu);        // 0x457250
void drawFrameAt(App* app, CPU& cpu);             // 0x4d8aa0

/* ---- The fields typed into ---- */
void editKey(App* app, CPU& cpu);                 // 0x457ae0
void drawIntField(App* app, CPU& cpu);            // 0x457c40
void intRangeMessage(App* app, CPU& cpu);         // 0x457df0
void intFieldKey(App* app, CPU& cpu);             // 0x457e30
void layoutIntField(App* app, CPU& cpu);          // 0x457f70
void intFieldShown(App* app, CPU& cpu);           // 0x458010
void intFieldHidden(App* app, CPU& cpu);          // 0x458050
void bindIntField(App* app, CPU& cpu);            // 0x458080
void drawStringField(App* app, CPU& cpu);         // 0x4580b0
void stringFieldKey(App* app, CPU& cpu);          // 0x4581f0
void layoutStringField(App* app, CPU& cpu);       // 0x458340
void stringFieldShown(App* app, CPU& cpu);        // 0x4583b0
void stringFieldHidden(App* app, CPU& cpu);       // 0x458410
void drawCharField(App* app, CPU& cpu);           // 0x458480
void charRangeMessage(App* app, CPU& cpu);        // 0x458640
void charFieldKey(App* app, CPU& cpu);            // 0x458690
void layoutCharField(App* app, CPU& cpu);         // 0x4587e0
void charFieldShown(App* app, CPU& cpu);          // 0x4588a0
void charFieldHidden(App* app, CPU& cpu);         // 0x4588e0
void bindCharField(App* app, CPU& cpu);           // 0x458910
void drawFloatField(App* app, CPU& cpu);          // 0x458940
void floatRangeMessage(App* app, CPU& cpu);       // 0x458b20
void floatFieldKey(App* app, CPU& cpu);           // 0x458b80
void layoutFloatField(App* app, CPU& cpu);        // 0x458ce0
void floatFieldShown(App* app, CPU& cpu);         // 0x458d90
void floatFieldHidden(App* app, CPU& cpu);        // 0x458dd0
void bindFloatField(App* app, CPU& cpu);          // 0x458e00
void drawFixedField(App* app, CPU& cpu);          // 0x458e30
void fixedRangeMessage(App* app, CPU& cpu);       // 0x459020
void fixedFieldKey(App* app, CPU& cpu);           // 0x459090
void layoutFixedField(App* app, CPU& cpu);        // 0x459210
void fixedFieldShown(App* app, CPU& cpu);         // 0x4592b0
void fixedFieldHidden(App* app, CPU& cpu);        // 0x4592f0
void bindFixedField(App* app, CPU& cpu);          // 0x459320

/* ---- The list of lines (List) ---- */
void drawListBox(App* app, CPU& cpu);             // 0x465500
void pointAtListBoxLine(App* app, CPU& cpu);      // 0x465880
void listBoxDown(App* app, CPU& cpu);             // 0x4658f0
void listBoxUp(App* app, CPU& cpu);               // 0x465950
void listBoxPartAt(App* app, CPU& cpu);           // 0x4659a0
void listBoxArrowAt(App* app, CPU& cpu);          // 0x465a10
void listBoxKey(App* app, CPU& cpu);              // 0x465a30
void layoutListBox(App* app, CPU& cpu);           // 0x465ba0
void listBoxShown(App* app, CPU& cpu);            // 0x465c10
void listBoxHidden(App* app, CPU& cpu);           // 0x465c60
void listBoxChooseAt(App* app, CPU& cpu);         // 0x465ca0
void drawShadedTriangle(App* app, CPU& cpu);      // 0x4d8570

/* ---- The sliders ---- */
void sliderPartSizes(App* app, CPU& cpu);         // 0x469080
void sliderPartUnderPointer(App* app, CPU& cpu);  // 0x4692c0
void sliderThumbPlace(App* app, CPU& cpu);        // 0x469340
void snapToStep(App* app, CPU& cpu);              // 0x4693c0
void setSliderValue(App* app, CPU& cpu);          // 0x469480
void setSliderStep(App* app, CPU& cpu);           // 0x469530
void dragSliderTo(App* app, CPU& cpu);            // 0x469590
void drawSlider(App* app, CPU& cpu);              // 0x469680
void trackSlider(App* app, CPU& cpu);             // 0x46a5f0
void sliderKey(App* app, CPU& cpu);               // 0x46a860
void placeSliderParts(App* app, CPU& cpu);        // 0x46a980
void setUpSlider(App* app, CPU& cpu);             // 0x46ab10
void sliderBox(App* app, CPU& cpu);               // 0x46abb0
void twoAxisThumbPlace(App* app, CPU& cpu);       // 0x46ac40
void setTwoAxisValue(App* app, CPU& cpu);         // 0x46add0
void setTwoAxisStep(App* app, CPU& cpu);          // 0x46afe0
void dragTwoAxisTo(App* app, CPU& cpu);           // 0x46b040
void drawTwoAxis(App* app, CPU& cpu);             // 0x46b3a0
void trackTwoAxis(App* app, CPU& cpu);            // 0x46c320
void twoAxisKey(App* app, CPU& cpu);              // 0x46c4a0
void placeTwoAxisParts(App* app, CPU& cpu);       // 0x46c640
void setUpTwoAxis(App* app, CPU& cpu);            // 0x46c820
void twoAxisBox(App* app, CPU& cpu);              // 0x46c8b0
void sliderSetValue(App* app, CPU& cpu);          // 0x46c950
void sliderDraw(App* app, CPU& cpu);              // 0x46cae0
void sliderWidgetKey(App* app, CPU& cpu);         // 0x46cb80
void sliderWidgetLayout(App* app, CPU& cpu);      // 0x46cbe0
void sliderWidgetHidden(App* app, CPU& cpu);      // 0x46cc10
void sliderWidgetShown(App* app, CPU& cpu);       // 0x46cc50
void sliderShape(App* app, CPU& cpu);             // 0x4d5e40

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address
 * (ByAddress below, for the checks' stand-ins).  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its adapter hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * adapter hands it, so the core's frame lies where the game's does; by address the call makes it. */

// The game's 0x439760: how many colours car `car` has, its custom one counted; esp the frame base.
Result colourCount(App* app, CPU& cpu, reg32 esp, reg32 car);
// The game's 0x439790: the colour player `player` chose for car `car`, -1 for no player; esp the frame base.
Result chosenColour(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t player);
// The game's 0x4397d0: car `car`'s colour for player `player` set to `colour` and saved; esp the frame base.
Result colourSet(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t player, int32_t colour);
// The game's 0x4398c0: player `player`'s custom colour of car `car` set to the bytes b0..b3 and saved; esp the frame base.
Result customColourSet(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t player, uint8_t b0, uint8_t b1, uint8_t b2,
                       uint8_t b3);
// The game's 0x439ab0: car `car`'s text number `n`; esp the frame base.
Result carTextOf(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t n);
// The game's 0x456a20: the colours' list item `item`'s colours taken from its car; esp the frame base.
Result listRefreshed(App* app, CPU& cpu, reg32 esp, reg32 item);
// The game's 0x46c950: the slider `item` set to `value` (a two-axis one's axis `axis`); esp the frame base.
Result sliderSetValueOf(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 value, reg32 axis);
// The game's 0x46cae0: the slider `item` drawn, worked by the mouse first where `hot`; esp the frame base.
Result sliderDrawOf(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 hot);
// The game's 0x472360: the custom colours taken into the settings (`eax` back where they are kept); esp the frame base.
Result customColoursRefreshed(App* app, CPU& cpu, reg32 esp, reg32 eax);
// The game's 0x4d8570: the triangle x0, y0; x1, y1; the corner whose x is at `x2At` (its stack argument's place,
// which the function rewrites on a race's pause screens) and y2; its colours c0..c2; esp the frame base.
Result triangle(App* app, CPU& cpu, reg32 esp, int32_t x0, int32_t y0, int32_t x1, int32_t y1, reg32 x2At,
                int32_t y2, reg32 c0, reg32 c1, reg32 c2);

}  // namespace fields
}  // namespace port

namespace guest
{

// The car in eax; the count in eax.
template <>
struct ByAddress<port::fields::colourCount>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 car)
    {
        return watcomEax(app, cpu, Address, {car});
    }
};

// The car in eax, the player in edx; the colour in eax.
template <>
struct ByAddress<port::fields::chosenColour>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 car,
                             std::int32_t player)
    {
        return watcomEax(app, cpu, Address, {car, x86::reg32(player)});
    }
};

// The car in eax, the player in edx, the colour in ebx; eax back.
template <>
struct ByAddress<port::fields::colourSet>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 car,
                             std::int32_t player, std::int32_t colour)
    {
        return watcomEax(app, cpu, Address, {car, x86::reg32(player), x86::reg32(colour)});
    }
};

// Stdcall: the car in eax, the player in edx, b0 in ebx, b1 in ecx; b2 and b3 pushed (ret 8); eax back.
template <>
struct ByAddress<port::fields::customColourSet>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 car,
                             std::int32_t player, std::uint8_t b0, std::uint8_t b1, std::uint8_t b2, std::uint8_t b3)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = car;
        cpu.edx = x86::reg32(player);
        cpu.ebx = b0;
        cpu.ecx = b1;
        if (!guest::call(app, cpu, Address, {b2, b3}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

// The car in eax, the number in edx; the text in eax.
template <>
struct ByAddress<port::fields::carTextOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 car, std::int32_t n)
    {
        return watcomEax(app, cpu, Address, {car, x86::reg32(n)});
    }
};

// The item in eax; eax back.
template <>
struct ByAddress<port::fields::listRefreshed>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 item)
    {
        return watcomEax(app, cpu, Address, {item});
    }
};

// Stdcall: the item in eax; the value and the axis pushed (ret 8); eax back.
template <>
struct ByAddress<port::fields::sliderSetValueOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 item, x86::reg32 value,
                             x86::reg32 axis)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = item;
        if (!guest::call(app, cpu, Address, {value, axis}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

// The item in eax, `hot` in edx; eax back.
template <>
struct ByAddress<port::fields::sliderDrawOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 item, x86::reg32 hot)
    {
        return watcomEax(app, cpu, Address, {item, hot});
    }
};

// `eax` in eax; eax back.
template <>
struct ByAddress<port::fields::customColoursRefreshed>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 eax)
    {
        return watcomEax(app, cpu, Address, {eax});
    }
};

// Stdcall: x0 in eax, y0 in edx, x1 in ebx, y1 in ecx; the word at `x2At`, y2 and the three colours pushed
// (ret 0x14); what the function leaves in the first one's place written back to `x2At`, where the core writes it;
// eax back.
template <>
struct ByAddress<port::fields::triangle>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t x0, std::int32_t y0,
                             std::int32_t x1, std::int32_t y1, x86::reg32 x2At, std::int32_t y2, x86::reg32 c0,
                             x86::reg32 c1, x86::reg32 c2)
    {
        const KeptRegisters kept(cpu);
        const x86::reg32 pushed = cpu.esp - 4 * 5;
        cpu.eax = x86::reg32(x0);
        cpu.edx = x86::reg32(y0);
        cpu.ebx = x86::reg32(x1);
        cpu.ecx = x86::reg32(y1);
        const bool went = guest::call(app, cpu, Address, {guest::at(app, x2At), x86::reg32(y2), c0, c1, c2});
        guest::put(app, x2At, guest::at(app, pushed));
        if (!went)
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
