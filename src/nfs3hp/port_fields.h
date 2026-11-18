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
 * Each is a thunk under the game's function's name, which
 * the game's code calls by name; the widget table (0x557a84) and the other
 * natives call it through the address. */
namespace nfs3hp
{
namespace port
{
namespace fields
{

/* ---- The car records' colours ---- */
x86::reg32 carColourCount(App* app, CPU& cpu, x86::Args& args);          // 0x439760
x86::reg32 carCustomColour(App* app, CPU& cpu, x86::Args& args);         // 0x439780
x86::reg32 carColour(App* app, CPU& cpu, x86::Args& args);               // 0x439790
x86::reg32 carColourName(App* app, CPU& cpu, x86::Args& args);           // 0x4397c0
x86::reg32 setCarColour(App* app, CPU& cpu, x86::Args& args);            // 0x4397d0
x86::reg32 carColourRgb(App* app, CPU& cpu, x86::Args& args);            // 0x439810
x86::reg32 setCarCustomColour(App* app, CPU& cpu, x86::Args& args);      // 0x4398c0
x86::reg32 carText(App* app, CPU& cpu, x86::Args& args);                 // 0x439ab0
x86::reg32 saveCarColours(App* app, CPU& cpu, x86::Args& args);          // 0x43b7f0
x86::reg32 wheelFromCarColour(App* app, CPU& cpu, x86::Args& args);      // 0x43dbe0
x86::reg32 wheelForPlayer(App* app, CPU& cpu, x86::Args& args);          // 0x43dd50
x86::reg32 refreshCustomColours(App* app, CPU& cpu, x86::Args& args);    // 0x472360

/* ---- A field's text and cursor ---- */
x86::reg32 setCursorColour(App* app, CPU& cpu, x86::Args& args);         // 0x4513e0
x86::reg32 drawFieldText(App* app, CPU& cpu, x86::Args& args);           // 0x4515e0
x86::reg32 drawFieldLine(App* app, CPU& cpu, x86::Args& args);           // 0x452220
x86::reg32 drawSpacedText(App* app, CPU& cpu, x86::Args& args);          // 0x4522c0
x86::reg32 drawCursorText3d(App* app, CPU& cpu, x86::Args& args);        // 0x4d11f0
x86::reg32 drawCursor(App* app, CPU& cpu, x86::Args& args);              // 0x4da560
x86::reg32 drawCursorText(App* app, CPU& cpu, x86::Args& args);          // 0x4da8c0

/* ---- The car's colours (carclrselect) ---- */
x86::reg32 colourListWidth(App* app, CPU& cpu, x86::Args& args);         // 0x4569a0
x86::reg32 colourListRefresh(App* app, CPU& cpu, x86::Args& args);       // 0x456a20
x86::reg32 drawColourLine(App* app, CPU& cpu, x86::Args& args);          // 0x456b50
x86::reg32 drawColourList(App* app, CPU& cpu, x86::Args& args);          // 0x456ce0
x86::reg32 colourListShown(App* app, CPU& cpu, x86::Args& args);         // 0x456ea0
x86::reg32 pointAtColour(App* app, CPU& cpu, x86::Args& args);           // 0x456f10
x86::reg32 openColourList(App* app, CPU& cpu, x86::Args& args);          // 0x456f50
x86::reg32 closeColourList(App* app, CPU& cpu, x86::Args& args);         // 0x456f70
x86::reg32 chooseColour(App* app, CPU& cpu, x86::Args& args);            // 0x456f90
x86::reg32 colourListKey(App* app, CPU& cpu, x86::Args& args);           // 0x457030
x86::reg32 layoutColourList(App* app, CPU& cpu, x86::Args& args);        // 0x4571e0
x86::reg32 colourListHidden(App* app, CPU& cpu, x86::Args& args);        // 0x457250
x86::reg32 drawFrameAt(App* app, CPU& cpu, x86::Args& args);             // 0x4d8aa0

/* ---- The fields typed into ---- */
x86::reg32 editKey(App* app, CPU& cpu, x86::Args& args);                 // 0x457ae0
x86::reg32 drawIntField(App* app, CPU& cpu, x86::Args& args);            // 0x457c40
x86::reg32 intRangeMessage(App* app, CPU& cpu, x86::Args& args);         // 0x457df0
x86::reg32 intFieldKey(App* app, CPU& cpu, x86::Args& args);             // 0x457e30
x86::reg32 layoutIntField(App* app, CPU& cpu, x86::Args& args);          // 0x457f70
x86::reg32 intFieldShown(App* app, CPU& cpu, x86::Args& args);           // 0x458010
x86::reg32 intFieldHidden(App* app, CPU& cpu, x86::Args& args);          // 0x458050
x86::reg32 bindIntField(App* app, CPU& cpu, x86::Args& args);            // 0x458080
x86::reg32 drawStringField(App* app, CPU& cpu, x86::Args& args);         // 0x4580b0
x86::reg32 stringFieldKey(App* app, CPU& cpu, x86::Args& args);          // 0x4581f0
x86::reg32 layoutStringField(App* app, CPU& cpu, x86::Args& args);       // 0x458340
x86::reg32 stringFieldShown(App* app, CPU& cpu, x86::Args& args);        // 0x4583b0
x86::reg32 stringFieldHidden(App* app, CPU& cpu, x86::Args& args);       // 0x458410
x86::reg32 drawCharField(App* app, CPU& cpu, x86::Args& args);           // 0x458480
x86::reg32 charRangeMessage(App* app, CPU& cpu, x86::Args& args);        // 0x458640
x86::reg32 charFieldKey(App* app, CPU& cpu, x86::Args& args);            // 0x458690
x86::reg32 layoutCharField(App* app, CPU& cpu, x86::Args& args);         // 0x4587e0
x86::reg32 charFieldShown(App* app, CPU& cpu, x86::Args& args);          // 0x4588a0
x86::reg32 charFieldHidden(App* app, CPU& cpu, x86::Args& args);         // 0x4588e0
x86::reg32 bindCharField(App* app, CPU& cpu, x86::Args& args);           // 0x458910
x86::reg32 drawFloatField(App* app, CPU& cpu, x86::Args& args);          // 0x458940
x86::reg32 floatRangeMessage(App* app, CPU& cpu, x86::Args& args);       // 0x458b20
x86::reg32 floatFieldKey(App* app, CPU& cpu, x86::Args& args);           // 0x458b80
x86::reg32 layoutFloatField(App* app, CPU& cpu, x86::Args& args);        // 0x458ce0
x86::reg32 floatFieldShown(App* app, CPU& cpu, x86::Args& args);         // 0x458d90
x86::reg32 floatFieldHidden(App* app, CPU& cpu, x86::Args& args);        // 0x458dd0
x86::reg32 bindFloatField(App* app, CPU& cpu, x86::Args& args);          // 0x458e00
x86::reg32 drawFixedField(App* app, CPU& cpu, x86::Args& args);          // 0x458e30
x86::reg32 fixedRangeMessage(App* app, CPU& cpu, x86::Args& args);       // 0x459020
x86::reg32 fixedFieldKey(App* app, CPU& cpu, x86::Args& args);           // 0x459090
x86::reg32 layoutFixedField(App* app, CPU& cpu, x86::Args& args);        // 0x459210
x86::reg32 fixedFieldShown(App* app, CPU& cpu, x86::Args& args);         // 0x4592b0
x86::reg32 fixedFieldHidden(App* app, CPU& cpu, x86::Args& args);        // 0x4592f0
x86::reg32 bindFixedField(App* app, CPU& cpu, x86::Args& args);          // 0x459320

/* ---- The list of lines (List) ---- */
x86::reg32 drawListBox(App* app, CPU& cpu, x86::Args& args);             // 0x465500
x86::reg32 pointAtListBoxLine(App* app, CPU& cpu, x86::Args& args);      // 0x465880
x86::reg32 listBoxDown(App* app, CPU& cpu, x86::Args& args);             // 0x4658f0
x86::reg32 listBoxUp(App* app, CPU& cpu, x86::Args& args);               // 0x465950
x86::reg32 listBoxPartAt(App* app, CPU& cpu, x86::Args& args);           // 0x4659a0
x86::reg32 listBoxArrowAt(App* app, CPU& cpu, x86::Args& args);          // 0x465a10
x86::reg32 listBoxKey(App* app, CPU& cpu, x86::Args& args);              // 0x465a30
x86::reg32 layoutListBox(App* app, CPU& cpu, x86::Args& args);           // 0x465ba0
x86::reg32 listBoxShown(App* app, CPU& cpu, x86::Args& args);            // 0x465c10
x86::reg32 listBoxHidden(App* app, CPU& cpu, x86::Args& args);           // 0x465c60
x86::reg32 listBoxChooseAt(App* app, CPU& cpu, x86::Args& args);         // 0x465ca0
x86::reg32 drawShadedTriangle(App* app, CPU& cpu, x86::Args& args);      // 0x4d8570

/* ---- The sliders ---- */
x86::reg32 sliderPartSizes(App* app, CPU& cpu, x86::Args& args);         // 0x469080
x86::reg32 sliderPartUnderPointer(App* app, CPU& cpu, x86::Args& args);  // 0x4692c0
x86::reg32 sliderThumbPlace(App* app, CPU& cpu, x86::Args& args);        // 0x469340
x86::reg32 snapToStep(App* app, CPU& cpu, x86::Args& args);              // 0x4693c0
x86::reg32 setSliderValue(App* app, CPU& cpu, x86::Args& args);          // 0x469480
x86::reg32 setSliderStep(App* app, CPU& cpu, x86::Args& args);           // 0x469530
x86::reg32 dragSliderTo(App* app, CPU& cpu, x86::Args& args);            // 0x469590
x86::reg32 drawSlider(App* app, CPU& cpu, x86::Args& args);              // 0x469680
x86::reg32 trackSlider(App* app, CPU& cpu, x86::Args& args);             // 0x46a5f0
x86::reg32 sliderKey(App* app, CPU& cpu, x86::Args& args);               // 0x46a860
x86::reg32 placeSliderParts(App* app, CPU& cpu, x86::Args& args);        // 0x46a980
x86::reg32 setUpSlider(App* app, CPU& cpu, x86::Args& args);             // 0x46ab10
x86::reg32 sliderBox(App* app, CPU& cpu, x86::Args& args);               // 0x46abb0
x86::reg32 twoAxisThumbPlace(App* app, CPU& cpu, x86::Args& args);       // 0x46ac40
x86::reg32 setTwoAxisValue(App* app, CPU& cpu, x86::Args& args);         // 0x46add0
x86::reg32 setTwoAxisStep(App* app, CPU& cpu, x86::Args& args);          // 0x46afe0
x86::reg32 dragTwoAxisTo(App* app, CPU& cpu, x86::Args& args);           // 0x46b040
x86::reg32 drawTwoAxis(App* app, CPU& cpu, x86::Args& args);             // 0x46b3a0
x86::reg32 trackTwoAxis(App* app, CPU& cpu, x86::Args& args);            // 0x46c320
x86::reg32 twoAxisKey(App* app, CPU& cpu, x86::Args& args);              // 0x46c4a0
x86::reg32 placeTwoAxisParts(App* app, CPU& cpu, x86::Args& args);       // 0x46c640
x86::reg32 setUpTwoAxis(App* app, CPU& cpu, x86::Args& args);            // 0x46c820
x86::reg32 twoAxisBox(App* app, CPU& cpu, x86::Args& args);              // 0x46c8b0
x86::reg32 sliderSetValue(App* app, CPU& cpu, x86::Args& args);          // 0x46c950
x86::reg32 sliderDraw(App* app, CPU& cpu, x86::Args& args);              // 0x46cae0
x86::reg32 sliderWidgetKey(App* app, CPU& cpu, x86::Args& args);         // 0x46cb80
x86::reg32 sliderWidgetLayout(App* app, CPU& cpu, x86::Args& args);      // 0x46cbe0
x86::reg32 sliderWidgetHidden(App* app, CPU& cpu, x86::Args& args);      // 0x46cc10
x86::reg32 sliderWidgetShown(App* app, CPU& cpu, x86::Args& args);       // 0x46cc50
x86::reg32 sliderShape(App* app, CPU& cpu, x86::Args& args);             // 0x4d5e40

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address.  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its thunk hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * thunk hands it, so the core's frame lies where the game's does; by address the call makes it. */

// The game's 0x439760: how many colours car `car` has, its custom one counted; esp the frame base.
Result colourCount(App* app, CPU& cpu, reg32 esp, reg32 car);
// The game's 0x439780: car `car`'s custom colour's number; esp the frame base.
Result customColourOf(App* app, CPU& cpu, reg32 esp, reg32 car);
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
// The game's 0x439810 (stdcall 0xc): colour `colour` of car `car` -- its custom one, the player's, for the last -- its
// four bytes to `out0` to `out3`: 0, or -1 past them; esp the frame base.
Result carColourBytes(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t colour, int32_t player, reg32 out0, reg32 out1,
                      reg32 out2, reg32 out3);
// The game's 0x4d8570: the triangle x0, y0; x1, y1; the corner whose x is at `x2At` (its stack argument's place,
// which the function rewrites on a race's pause screens) and y2; its colours c0..c2; esp the frame base.
Result triangle(App* app, CPU& cpu, reg32 esp, int32_t x0, int32_t y0, int32_t x1, int32_t y1, reg32 x2At,
                int32_t y2, reg32 c0, reg32 c1, reg32 c2);

}  // namespace fields
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
