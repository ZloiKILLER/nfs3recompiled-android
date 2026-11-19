#ifndef NFS3HP_PORT_TEXT_H
#define NFS3HP_PORT_TEXT_H

#include "native_guest.h"
#include "port.h"

/* The menus' text and the 2D drawing under it, rewritten (port.h says how;
 * port_text.cpp): a line of the language's text by its number, the font a
 * text is measured and drawn in -- the 3D card's, a quad of the font's
 * texture a character, in a race's pause screens and the HUD; the 2D
 * screens' otherwise --, the fonts' quads queued and drawn, the text scale,
 * a text placed on the menus' 640 by 480 screen and on the screen's own, a
 * text wrapped into a box, the menus' key from the keyboard and the mouse.
 *
 * The game's code calls each by the name it has here; the other
 * natives call these thunks as the game's code would call the address. */
namespace nfs3hp
{
namespace port
{
namespace text
{

/* The thunks, under the game's functions' names. */
nfs::u32 textWidth(App* app, ThreadState& ts, nfs::Args& args);              // 0x452020
nfs::u32 selectFont(App* app, ThreadState& ts, nfs::Args& args);             // 0x452100
nfs::u32 measureText(App* app, ThreadState& ts, nfs::Args& args);            // 0x4d1430
nfs::u32 textWidthWithOverhang(App* app, ThreadState& ts, nfs::Args& args);  // 0x4d1480
nfs::u32 drawText(App* app, ThreadState& ts, nfs::Args& args);               // 0x4d1000
nfs::u32 drawCharacter(App* app, ThreadState& ts, nfs::Args& args);          // 0x4d06f0
nfs::u32 takeQuad(App* app, ThreadState& ts, nfs::Args& args);               // 0x49bcd0
nfs::u32 viewport(App* app, ThreadState& ts, nfs::Args& args);               // 0x4beb80
nfs::u32 toScreenX(App* app, ThreadState& ts, nfs::Args& args);              // 0x451400
nfs::u32 toScreenY(App* app, ThreadState& ts, nfs::Args& args);              // 0x451440
nfs::u32 specialText(App* app, ThreadState& ts, nfs::Args& args);            // 0x4d17b0
nfs::u32 languageText(App* app, ThreadState& ts, nfs::Args& args);           // 0x4d1850
nfs::u32 flushFonts(App* app, ThreadState& ts, nfs::Args& args);             // 0x4d1340
nfs::u32 setTextScale(App* app, ThreadState& ts, nfs::Args& args);           // 0x4d1390
nfs::u32 keepTextScale(App* app, ThreadState& ts, nfs::Args& args);          // 0x4d1400
nfs::u32 restoreTextScale(App* app, ThreadState& ts, nfs::Args& args);       // 0x4d1410
nfs::u32 characterWidth(App* app, ThreadState& ts, nfs::Args& args);         // 0x4d1540
nfs::u32 characterOverhang(App* app, ThreadState& ts, nfs::Args& args);      // 0x4d1590
nfs::u32 setTextColour(App* app, ThreadState& ts, nfs::Args& args);          // 0x4513c0
nfs::u32 screenToMenuX(App* app, ThreadState& ts, nfs::Args& args);          // 0x451480
nfs::u32 screenToMenuY(App* app, ThreadState& ts, nfs::Args& args);          // 0x4514c0
nfs::u32 readMenuMouse(App* app, ThreadState& ts, nfs::Args& args);          // 0x451500
nfs::u32 dimBackground(App* app, ThreadState& ts, nfs::Args& args);          // 0x451530
nfs::u32 drawTextAt(App* app, ThreadState& ts, nfs::Args& args);             // 0x451590
nfs::u32 drawMenuText(App* app, ThreadState& ts, nfs::Args& args);           // 0x452190
nfs::u32 flushQuads(App* app, ThreadState& ts, nfs::Args& args);             // 0x49be40
nfs::u32 joystickKey(App* app, ThreadState& ts, nfs::Args& args);            // 0x451700
nfs::u32 captureKeys(App* app, ThreadState& ts, nfs::Args& args);            // 0x451810
nfs::u32 scanCodeOfKey(App* app, ThreadState& ts, nfs::Args& args);          // 0x451840
nfs::u32 keyOfScanCode(App* app, ThreadState& ts, nfs::Args& args);          // 0x4518c0
nfs::u32 menuKey(App* app, ThreadState& ts, nfs::Args& args);                // 0x451960
nfs::u32 resetMenuKey(App* app, ThreadState& ts, nfs::Args& args);           // 0x451b20
nfs::u32 nextWord(App* app, ThreadState& ts, nfs::Args& args);               // 0x451b50
nfs::u32 drawWrappedText(App* app, ThreadState& ts, nfs::Args& args);        // 0x451cd0
nfs::u32 drawTextBox(App* app, ThreadState& ts, nfs::Args& args);            // 0x451fc0
nfs::u32 measureTextBox(App* app, ThreadState& ts, nfs::Args& args);         // 0x451ff0
nfs::u32 menusEnd(App* app, ThreadState& ts, nfs::Args& args);               // 0x452110
nfs::u32 queueQuad(App* app, ThreadState& ts, nfs::Args& args);              // 0x49bd30
nfs::u32 unqueueQuad(App* app, ThreadState& ts, nfs::Args& args);            // 0x49bdc0
nfs::u32 mouseMenuKey(App* app, ThreadState& ts, nfs::Args& args);           // 0x49c000
nfs::u32 loadLanguageText(App* app, ThreadState& ts, nfs::Args& args);       // 0x4d1750

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address.  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its thunk hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes ts.sp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * thunk hands it, so the core's frame lies where the game's does; by address the call makes it. */

// The game's 0x452020: how wide `text` is in font `font` (5 for none); esp the frame base.
Result menuTextWidth(App* app, ThreadState& ts, u32 esp, u32 text, u32 font);
// The game's 0x452100: the 2D screens' font set to `font`.
void selectFontOf(App* app, u32 font);
// The game's 0x452190: `text` in `font` at x, y, aligned by `align` (1..5), in `colour`; esp the frame base.
Result menuTextDrawn(App* app, ThreadState& ts, u32 esp, u32 text, int32_t x, int32_t y, u32 font, int32_t align,
                     u32 colour);
// The game's 0x4d1340: both fonts' queued quads drawn; esp the frame base.
Result fontsDrawn(App* app, ThreadState& ts, u32 esp);
// The game's 0x4d1400: the text scale kept; the scale (a float's bits).
u32 keepTextScaleOf(App* app);
// The game's 0x4d1410: the text scale kept put back.
void restoreTextScaleOf(App* app, ThreadState& ts);
// The game's 0x4d1750: language `language`'s text loaded (0..6): the block, or the language past them; esp the frame base.
Result loadedLanguageText(App* app, ThreadState& ts, u32 esp, int32_t language);
// The game's 0x4d1850: line `number` of the language's text, or a name past them (`edx` handed on to those); esp the frame base.
Result lineText(App* app, ThreadState& ts, u32 esp, int32_t number, u32 edx);
// 0x451400 and 0x451440 as plain functions: x on the 640 wide menu screen, y on the 480 high one, to the screen's own.
bool mouseRead(App* app, ThreadState& ts, u32 esp, u32 buttons, u32 x, u32 y, u32 wheel);  // 0x451500
int32_t screenX(App* app, int32_t x);
int32_t screenY(App* app, int32_t y);
// The game's 0x4513c0: the menus' text's colour set to `colour` (eax left as it came).
void textColourSet(App* app, u32 colour);
// The game's 0x451590: `text` at x, y of the menus' screen; esp the frame base.
Result textDrawnAt(App* app, ThreadState& ts, u32 esp, u32 text, int32_t x, int32_t y);
// The game's 0x451530: the screen behind the menus dimmed by the float at `slot` (its stack argument's place, where a
// race's pause screens put their shade instead): that shade, else `eax`.
u32 dimmed(App* app, ThreadState& ts, u32 slot, u32 eax);
// The game's 0x451960: the menus' key now, a held one repeating when `repeating`; esp the frame base.
Result menuKeyOf(App* app, ThreadState& ts, u32 esp, u32 repeating);

/* Those whose code leaves a value on the x87's stack (Watcom's float result): the value comes back
 * as a double, and eax as the code leaves it; their thunks hand it back in Args::floatResult. */
struct Width
{
    u32 eax;
    double value;
};
using WidthResult = std::optional<Width>;

/* What 0x4d06f0 is handed: the character's number (0xe0 a font, from the space), its square of the
 * font's texture, where it goes (x a float), its colour, whether it is only measured. */
struct Character
{
    u32 number;
    int32_t textureX, textureY;
    float x;
    int32_t y;
    int32_t width, height;
    u32 colour;
    int32_t measured;
};

// The game's 0x4d1390 (stdcall 4): the text scale set to `scale` (a float's bits); the one before (eax kept).
double textScaleSet(App* app, ThreadState& ts, u32 scale);
// The game's 0x4d06f0 (stdcall 0x18): a character of the fonts queued, unless only measured; how far the next
// goes.  Its x moves to the quad's left edge in its stack argument's place, esp + 4; esp the frame base.
WidthResult characterDrawn(App* app, ThreadState& ts, u32 esp, Character ch);
// The game's 0x4d1430: how wide `text` is, in whole pixels; esp the frame base.
WidthResult textMeasured(App* app, ThreadState& ts, u32 esp, u32 text);
// The game's 0x4d1480: `text` measured, its last character's overhang added; esp the frame base.
WidthResult textMeasuredWithOverhang(App* app, ThreadState& ts, u32 esp, u32 text);
// The game's 0x4d1000: `text` at x, y in the font's colour, or only measured; how wide; esp the frame base.
WidthResult textDrawn(App* app, ThreadState& ts, u32 esp, u32 text, int32_t x, int32_t y);

// The game's 0x49bd30 (stdcall 4): the quad a, b, c, d queued on `list` (`d` the stack's word); esp the frame base.
Result quadQueued(App* app, ThreadState& ts, u32 esp, u32 list, u32 a, u32 b, u32 c, u32 d);
// The game's 0x49bdc0 (stdcall 4): the quad taken off `list`'s queue; esp the frame base.
Result quadUnqueued(App* app, ThreadState& ts, u32 esp, u32 list, u32 a, u32 b, u32 c, u32 d);
// The game's 0x49be40: `list`'s quads drawn; esp the frame base.
Result quadsFlushed(App* app, ThreadState& ts, u32 esp, u32 list);

/* What 0x451cd0 is handed: the text, its font, colour and place (y moves a
 * line down as each is drawn, in the argument's place on the stack), how wide
 * a line may be; where to put how wide the widest line is, how high the box,
 * how many lines; whether it is only measured; which lines are drawn, from
 * the first. */
struct Box
{
    u32 text, font, colour;
    int32_t x, y;
    int32_t width;
    u32 widest, height, lines;
    int32_t measureOnly;
    int32_t count, first;
};
// The game's 0x451cd0 (stdcall 0x20): `b`'s text wrapped into its box; eax as the code leaves it; esp the frame base.
Result wrappedText(App* app, ThreadState& ts, u32 esp, Box b);
// The game's 0x451fc0 (stdcall 0x18): `text` in `font` and `colour` wrapped into a box `width` wide at x, y, every line
// drawn (`measureOnly` 0); where the widest line, the height and the lines go; esp the frame base.
Result wrappedBoxDrawn(App* app, ThreadState& ts, u32 esp, u32 text, u32 font, u32 colour, int32_t x, int32_t y,
                       int32_t width, u32 widest, u32 height, u32 lines, int32_t measureOnly);
// The game's 0x451ff0 (stdcall 8): `text` in `font` measured in a box `width` wide: the widest line, the height and
// the lines to the three places; esp the frame base.
Result boxMeasured(App* app, ThreadState& ts, u32 esp, u32 text, u32 font, int32_t width, u32 widest, u32 height,
                   u32 lines);
// The game's 0x4d1540 and 0x4d1590: character `character`'s height (less one) and its drop, in pixels of the font as drawn.
int32_t characterHeight(App* app, ThreadState& ts, u32 character);
int32_t characterDrop(App* app, ThreadState& ts, u32 character);
// The game's 0x4beb80: the screen's viewport left, top, width and height put at the four places; the height.
u32 viewportOf(App* app, u32 leftAt, u32 topAt, u32 widthAt, u32 heightAt);
}  // namespace text
}  // namespace port
}  // namespace nfs3hp

#endif
