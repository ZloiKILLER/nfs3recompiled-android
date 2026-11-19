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
nfs::u32 carColourCount(App* app, ThreadState& ts, nfs::Args& args);          // 0x439760
nfs::u32 carCustomColour(App* app, ThreadState& ts, nfs::Args& args);         // 0x439780
nfs::u32 carColour(App* app, ThreadState& ts, nfs::Args& args);               // 0x439790
nfs::u32 carColourName(App* app, ThreadState& ts, nfs::Args& args);           // 0x4397c0
nfs::u32 setCarColour(App* app, ThreadState& ts, nfs::Args& args);            // 0x4397d0
nfs::u32 carColourRgb(App* app, ThreadState& ts, nfs::Args& args);            // 0x439810
nfs::u32 setCarCustomColour(App* app, ThreadState& ts, nfs::Args& args);      // 0x4398c0
nfs::u32 carText(App* app, ThreadState& ts, nfs::Args& args);                 // 0x439ab0
nfs::u32 saveCarColours(App* app, ThreadState& ts, nfs::Args& args);          // 0x43b7f0
nfs::u32 wheelFromCarColour(App* app, ThreadState& ts, nfs::Args& args);      // 0x43dbe0
nfs::u32 wheelForPlayer(App* app, ThreadState& ts, nfs::Args& args);          // 0x43dd50
nfs::u32 refreshCustomColours(App* app, ThreadState& ts, nfs::Args& args);    // 0x472360

/* ---- A field's text and cursor ---- */
nfs::u32 setCursorColour(App* app, ThreadState& ts, nfs::Args& args);         // 0x4513e0
nfs::u32 drawFieldText(App* app, ThreadState& ts, nfs::Args& args);           // 0x4515e0
nfs::u32 drawFieldLine(App* app, ThreadState& ts, nfs::Args& args);           // 0x452220
nfs::u32 drawSpacedText(App* app, ThreadState& ts, nfs::Args& args);          // 0x4522c0
nfs::u32 drawCursorText3d(App* app, ThreadState& ts, nfs::Args& args);        // 0x4d11f0
nfs::u32 drawCursor(App* app, ThreadState& ts, nfs::Args& args);              // 0x4da560
nfs::u32 drawCursorText(App* app, ThreadState& ts, nfs::Args& args);          // 0x4da8c0

/* ---- The car's colours (carclrselect) ---- */
nfs::u32 colourListWidth(App* app, ThreadState& ts, nfs::Args& args);         // 0x4569a0
nfs::u32 colourListRefresh(App* app, ThreadState& ts, nfs::Args& args);       // 0x456a20
nfs::u32 drawColourLine(App* app, ThreadState& ts, nfs::Args& args);          // 0x456b50
nfs::u32 drawColourList(App* app, ThreadState& ts, nfs::Args& args);          // 0x456ce0
nfs::u32 colourListShown(App* app, ThreadState& ts, nfs::Args& args);         // 0x456ea0
nfs::u32 pointAtColour(App* app, ThreadState& ts, nfs::Args& args);           // 0x456f10
nfs::u32 openColourList(App* app, ThreadState& ts, nfs::Args& args);          // 0x456f50
nfs::u32 closeColourList(App* app, ThreadState& ts, nfs::Args& args);         // 0x456f70
nfs::u32 chooseColour(App* app, ThreadState& ts, nfs::Args& args);            // 0x456f90
nfs::u32 colourListKey(App* app, ThreadState& ts, nfs::Args& args);           // 0x457030
nfs::u32 layoutColourList(App* app, ThreadState& ts, nfs::Args& args);        // 0x4571e0
nfs::u32 colourListHidden(App* app, ThreadState& ts, nfs::Args& args);        // 0x457250
nfs::u32 drawFrameAt(App* app, ThreadState& ts, nfs::Args& args);             // 0x4d8aa0

/* ---- The fields typed into ---- */
nfs::u32 editKey(App* app, ThreadState& ts, nfs::Args& args);                 // 0x457ae0
nfs::u32 drawIntField(App* app, ThreadState& ts, nfs::Args& args);            // 0x457c40
nfs::u32 intRangeMessage(App* app, ThreadState& ts, nfs::Args& args);         // 0x457df0
nfs::u32 intFieldKey(App* app, ThreadState& ts, nfs::Args& args);             // 0x457e30
nfs::u32 layoutIntField(App* app, ThreadState& ts, nfs::Args& args);          // 0x457f70
nfs::u32 intFieldShown(App* app, ThreadState& ts, nfs::Args& args);           // 0x458010
nfs::u32 intFieldHidden(App* app, ThreadState& ts, nfs::Args& args);          // 0x458050
nfs::u32 bindIntField(App* app, ThreadState& ts, nfs::Args& args);            // 0x458080
nfs::u32 drawStringField(App* app, ThreadState& ts, nfs::Args& args);         // 0x4580b0
nfs::u32 stringFieldKey(App* app, ThreadState& ts, nfs::Args& args);          // 0x4581f0
nfs::u32 layoutStringField(App* app, ThreadState& ts, nfs::Args& args);       // 0x458340
nfs::u32 stringFieldShown(App* app, ThreadState& ts, nfs::Args& args);        // 0x4583b0
nfs::u32 stringFieldHidden(App* app, ThreadState& ts, nfs::Args& args);       // 0x458410
nfs::u32 drawCharField(App* app, ThreadState& ts, nfs::Args& args);           // 0x458480
nfs::u32 charRangeMessage(App* app, ThreadState& ts, nfs::Args& args);        // 0x458640
nfs::u32 charFieldKey(App* app, ThreadState& ts, nfs::Args& args);            // 0x458690
nfs::u32 layoutCharField(App* app, ThreadState& ts, nfs::Args& args);         // 0x4587e0
nfs::u32 charFieldShown(App* app, ThreadState& ts, nfs::Args& args);          // 0x4588a0
nfs::u32 charFieldHidden(App* app, ThreadState& ts, nfs::Args& args);         // 0x4588e0
nfs::u32 bindCharField(App* app, ThreadState& ts, nfs::Args& args);           // 0x458910
nfs::u32 drawFloatField(App* app, ThreadState& ts, nfs::Args& args);          // 0x458940
nfs::u32 floatRangeMessage(App* app, ThreadState& ts, nfs::Args& args);       // 0x458b20
nfs::u32 floatFieldKey(App* app, ThreadState& ts, nfs::Args& args);           // 0x458b80
nfs::u32 layoutFloatField(App* app, ThreadState& ts, nfs::Args& args);        // 0x458ce0
nfs::u32 floatFieldShown(App* app, ThreadState& ts, nfs::Args& args);         // 0x458d90
nfs::u32 floatFieldHidden(App* app, ThreadState& ts, nfs::Args& args);        // 0x458dd0
nfs::u32 bindFloatField(App* app, ThreadState& ts, nfs::Args& args);          // 0x458e00
nfs::u32 drawFixedField(App* app, ThreadState& ts, nfs::Args& args);          // 0x458e30
nfs::u32 fixedRangeMessage(App* app, ThreadState& ts, nfs::Args& args);       // 0x459020
nfs::u32 fixedFieldKey(App* app, ThreadState& ts, nfs::Args& args);           // 0x459090
nfs::u32 layoutFixedField(App* app, ThreadState& ts, nfs::Args& args);        // 0x459210
nfs::u32 fixedFieldShown(App* app, ThreadState& ts, nfs::Args& args);         // 0x4592b0
nfs::u32 fixedFieldHidden(App* app, ThreadState& ts, nfs::Args& args);        // 0x4592f0
nfs::u32 bindFixedField(App* app, ThreadState& ts, nfs::Args& args);          // 0x459320

/* ---- The list of lines (List) ---- */
nfs::u32 drawListBox(App* app, ThreadState& ts, nfs::Args& args);             // 0x465500
nfs::u32 pointAtListBoxLine(App* app, ThreadState& ts, nfs::Args& args);      // 0x465880
nfs::u32 listBoxDown(App* app, ThreadState& ts, nfs::Args& args);             // 0x4658f0
nfs::u32 listBoxUp(App* app, ThreadState& ts, nfs::Args& args);               // 0x465950
nfs::u32 listBoxPartAt(App* app, ThreadState& ts, nfs::Args& args);           // 0x4659a0
nfs::u32 listBoxArrowAt(App* app, ThreadState& ts, nfs::Args& args);          // 0x465a10
nfs::u32 listBoxKey(App* app, ThreadState& ts, nfs::Args& args);              // 0x465a30
nfs::u32 layoutListBox(App* app, ThreadState& ts, nfs::Args& args);           // 0x465ba0
nfs::u32 listBoxShown(App* app, ThreadState& ts, nfs::Args& args);            // 0x465c10
nfs::u32 listBoxHidden(App* app, ThreadState& ts, nfs::Args& args);           // 0x465c60
nfs::u32 listBoxChooseAt(App* app, ThreadState& ts, nfs::Args& args);         // 0x465ca0
nfs::u32 drawShadedTriangle(App* app, ThreadState& ts, nfs::Args& args);      // 0x4d8570

/* ---- The sliders ---- */
nfs::u32 sliderPartSizes(App* app, ThreadState& ts, nfs::Args& args);         // 0x469080
nfs::u32 sliderPartUnderPointer(App* app, ThreadState& ts, nfs::Args& args);  // 0x4692c0
nfs::u32 sliderThumbPlace(App* app, ThreadState& ts, nfs::Args& args);        // 0x469340
nfs::u32 snapToStep(App* app, ThreadState& ts, nfs::Args& args);              // 0x4693c0
nfs::u32 setSliderValue(App* app, ThreadState& ts, nfs::Args& args);          // 0x469480
nfs::u32 setSliderStep(App* app, ThreadState& ts, nfs::Args& args);           // 0x469530
nfs::u32 dragSliderTo(App* app, ThreadState& ts, nfs::Args& args);            // 0x469590
nfs::u32 drawSlider(App* app, ThreadState& ts, nfs::Args& args);              // 0x469680
nfs::u32 trackSlider(App* app, ThreadState& ts, nfs::Args& args);             // 0x46a5f0
nfs::u32 sliderKey(App* app, ThreadState& ts, nfs::Args& args);               // 0x46a860
nfs::u32 placeSliderParts(App* app, ThreadState& ts, nfs::Args& args);        // 0x46a980
nfs::u32 setUpSlider(App* app, ThreadState& ts, nfs::Args& args);             // 0x46ab10
nfs::u32 sliderBox(App* app, ThreadState& ts, nfs::Args& args);               // 0x46abb0
nfs::u32 twoAxisThumbPlace(App* app, ThreadState& ts, nfs::Args& args);       // 0x46ac40
nfs::u32 setTwoAxisValue(App* app, ThreadState& ts, nfs::Args& args);         // 0x46add0
nfs::u32 setTwoAxisStep(App* app, ThreadState& ts, nfs::Args& args);          // 0x46afe0
nfs::u32 dragTwoAxisTo(App* app, ThreadState& ts, nfs::Args& args);           // 0x46b040
nfs::u32 drawTwoAxis(App* app, ThreadState& ts, nfs::Args& args);             // 0x46b3a0
nfs::u32 trackTwoAxis(App* app, ThreadState& ts, nfs::Args& args);            // 0x46c320
nfs::u32 twoAxisKey(App* app, ThreadState& ts, nfs::Args& args);              // 0x46c4a0
nfs::u32 placeTwoAxisParts(App* app, ThreadState& ts, nfs::Args& args);       // 0x46c640
nfs::u32 setUpTwoAxis(App* app, ThreadState& ts, nfs::Args& args);            // 0x46c820
nfs::u32 twoAxisBox(App* app, ThreadState& ts, nfs::Args& args);              // 0x46c8b0
nfs::u32 sliderSetValue(App* app, ThreadState& ts, nfs::Args& args);          // 0x46c950
nfs::u32 sliderDraw(App* app, ThreadState& ts, nfs::Args& args);              // 0x46cae0
nfs::u32 sliderWidgetKey(App* app, ThreadState& ts, nfs::Args& args);         // 0x46cb80
nfs::u32 sliderWidgetLayout(App* app, ThreadState& ts, nfs::Args& args);      // 0x46cbe0
nfs::u32 sliderWidgetHidden(App* app, ThreadState& ts, nfs::Args& args);      // 0x46cc10
nfs::u32 sliderWidgetShown(App* app, ThreadState& ts, nfs::Args& args);       // 0x46cc50
nfs::u32 sliderShape(App* app, ThreadState& ts, nfs::Args& args);             // 0x4d5e40

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address.  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its thunk hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes ts.sp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * thunk hands it, so the core's frame lies where the game's does; by address the call makes it. */

// The game's 0x439760: how many colours car `car` has, its custom one counted; esp the frame base.
Result colourCount(App* app, ThreadState& ts, u32 esp, u32 car);
// The game's 0x439780: car `car`'s custom colour's number; esp the frame base.
Result customColourOf(App* app, ThreadState& ts, u32 esp, u32 car);
// The game's 0x439790: the colour player `player` chose for car `car`, -1 for no player; esp the frame base.
Result chosenColour(App* app, ThreadState& ts, u32 esp, u32 car, int32_t player);
// The game's 0x4397d0: car `car`'s colour for player `player` set to `colour` and saved; esp the frame base.
Result colourSet(App* app, ThreadState& ts, u32 esp, u32 car, int32_t player, int32_t colour);
// The game's 0x4398c0: player `player`'s custom colour of car `car` set to the bytes b0..b3 and saved; esp the frame base.
Result customColourSet(App* app, ThreadState& ts, u32 esp, u32 car, int32_t player, uint8_t b0, uint8_t b1, uint8_t b2,
                       uint8_t b3);
// The game's 0x439ab0: car `car`'s text number `n`; esp the frame base.
Result carTextOf(App* app, ThreadState& ts, u32 esp, u32 car, int32_t n);
// The game's 0x456a20: the colours' list item `item`'s colours taken from its car; esp the frame base.
Result listRefreshed(App* app, ThreadState& ts, u32 esp, u32 item);
// The game's 0x46c950: the slider `item` set to `value` (a two-axis one's axis `axis`); esp the frame base.
Result sliderSetValueOf(App* app, ThreadState& ts, u32 esp, u32 item, u32 value, u32 axis);
// The game's 0x46cae0: the slider `item` drawn, worked by the mouse first where `hot`; esp the frame base.
Result sliderDrawOf(App* app, ThreadState& ts, u32 esp, u32 item, u32 hot);
// The game's 0x472360: the custom colours taken into the settings (`eax` back where they are kept); esp the frame base.
Result customColoursRefreshed(App* app, ThreadState& ts, u32 esp, u32 eax);
// The game's 0x439810 (stdcall 0xc): colour `colour` of car `car` -- its custom one, the player's, for the last -- its
// four bytes to `out0` to `out3`: 0, or -1 past them; esp the frame base.
Result carColourBytes(App* app, ThreadState& ts, u32 esp, u32 car, int32_t colour, int32_t player, u32 out0, u32 out1,
                      u32 out2, u32 out3);
// The game's 0x4d8570: the triangle x0, y0; x1, y1; the corner whose x is at `x2At` (its stack argument's place,
// which the function rewrites on a race's pause screens) and y2; its colours c0..c2; esp the frame base.
Result triangle(App* app, ThreadState& ts, u32 esp, int32_t x0, int32_t y0, int32_t x1, int32_t y1, u32 x2At,
                int32_t y2, u32 c0, u32 c1, u32 c2);

}  // namespace fields
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
