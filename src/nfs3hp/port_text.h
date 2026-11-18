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
x86::reg32 textWidth(App* app, CPU& cpu, x86::Args& args);              // 0x452020
x86::reg32 selectFont(App* app, CPU& cpu, x86::Args& args);             // 0x452100
x86::reg32 measureText(App* app, CPU& cpu, x86::Args& args);            // 0x4d1430
x86::reg32 textWidthWithOverhang(App* app, CPU& cpu, x86::Args& args);  // 0x4d1480
x86::reg32 drawText(App* app, CPU& cpu, x86::Args& args);               // 0x4d1000
x86::reg32 drawCharacter(App* app, CPU& cpu, x86::Args& args);          // 0x4d06f0
x86::reg32 takeQuad(App* app, CPU& cpu, x86::Args& args);               // 0x49bcd0
x86::reg32 viewport(App* app, CPU& cpu, x86::Args& args);               // 0x4beb80
x86::reg32 toScreenX(App* app, CPU& cpu, x86::Args& args);              // 0x451400
x86::reg32 toScreenY(App* app, CPU& cpu, x86::Args& args);              // 0x451440
x86::reg32 specialText(App* app, CPU& cpu, x86::Args& args);            // 0x4d17b0
x86::reg32 languageText(App* app, CPU& cpu, x86::Args& args);           // 0x4d1850
x86::reg32 flushFonts(App* app, CPU& cpu, x86::Args& args);             // 0x4d1340
x86::reg32 setTextScale(App* app, CPU& cpu, x86::Args& args);           // 0x4d1390
x86::reg32 keepTextScale(App* app, CPU& cpu, x86::Args& args);          // 0x4d1400
x86::reg32 restoreTextScale(App* app, CPU& cpu, x86::Args& args);       // 0x4d1410
x86::reg32 characterWidth(App* app, CPU& cpu, x86::Args& args);         // 0x4d1540
x86::reg32 characterOverhang(App* app, CPU& cpu, x86::Args& args);      // 0x4d1590
x86::reg32 setTextColour(App* app, CPU& cpu, x86::Args& args);          // 0x4513c0
x86::reg32 screenToMenuX(App* app, CPU& cpu, x86::Args& args);          // 0x451480
x86::reg32 screenToMenuY(App* app, CPU& cpu, x86::Args& args);          // 0x4514c0
x86::reg32 readMenuMouse(App* app, CPU& cpu, x86::Args& args);          // 0x451500
x86::reg32 dimBackground(App* app, CPU& cpu, x86::Args& args);          // 0x451530
x86::reg32 drawTextAt(App* app, CPU& cpu, x86::Args& args);             // 0x451590
x86::reg32 drawMenuText(App* app, CPU& cpu, x86::Args& args);           // 0x452190
x86::reg32 flushQuads(App* app, CPU& cpu, x86::Args& args);             // 0x49be40
x86::reg32 joystickKey(App* app, CPU& cpu, x86::Args& args);            // 0x451700
x86::reg32 captureKeys(App* app, CPU& cpu, x86::Args& args);            // 0x451810
x86::reg32 scanCodeOfKey(App* app, CPU& cpu, x86::Args& args);          // 0x451840
x86::reg32 keyOfScanCode(App* app, CPU& cpu, x86::Args& args);          // 0x4518c0
x86::reg32 menuKey(App* app, CPU& cpu, x86::Args& args);                // 0x451960
x86::reg32 resetMenuKey(App* app, CPU& cpu, x86::Args& args);           // 0x451b20
x86::reg32 nextWord(App* app, CPU& cpu, x86::Args& args);               // 0x451b50
x86::reg32 drawWrappedText(App* app, CPU& cpu, x86::Args& args);        // 0x451cd0
x86::reg32 drawTextBox(App* app, CPU& cpu, x86::Args& args);            // 0x451fc0
x86::reg32 measureTextBox(App* app, CPU& cpu, x86::Args& args);         // 0x451ff0
x86::reg32 menusEnd(App* app, CPU& cpu, x86::Args& args);               // 0x452110
x86::reg32 queueQuad(App* app, CPU& cpu, x86::Args& args);              // 0x49bd30
x86::reg32 unqueueQuad(App* app, CPU& cpu, x86::Args& args);            // 0x49bdc0
x86::reg32 mouseMenuKey(App* app, CPU& cpu, x86::Args& args);           // 0x49c000
x86::reg32 loadLanguageText(App* app, CPU& cpu, x86::Args& args);       // 0x4d1750

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address.  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its thunk hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * thunk hands it, so the core's frame lies where the game's does; by address the call makes it. */

// The game's 0x452020: how wide `text` is in font `font` (5 for none); esp the frame base.
Result menuTextWidth(App* app, CPU& cpu, reg32 esp, reg32 text, reg32 font);
// The game's 0x452100: the 2D screens' font set to `font`.
void selectFontOf(App* app, reg32 font);
// The game's 0x452190: `text` in `font` at x, y, aligned by `align` (1..5), in `colour`; esp the frame base.
Result menuTextDrawn(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y, reg32 font, int32_t align,
                     reg32 colour);
// The game's 0x4d1340: both fonts' queued quads drawn; esp the frame base.
Result fontsDrawn(App* app, CPU& cpu, reg32 esp);
// The game's 0x4d1400: the text scale kept; the scale (a float's bits).
reg32 keepTextScaleOf(App* app);
// The game's 0x4d1410: the text scale kept put back.
void restoreTextScaleOf(App* app, CPU& cpu);
// The game's 0x4d1750: language `language`'s text loaded (0..6): the block, or the language past them; esp the frame base.
Result loadedLanguageText(App* app, CPU& cpu, reg32 esp, int32_t language);
// The game's 0x4d1850: line `number` of the language's text, or a name past them (`edx` handed on to those); esp the frame base.
Result lineText(App* app, CPU& cpu, reg32 esp, int32_t number, reg32 edx);
// 0x451400 and 0x451440 as plain functions: x on the 640 wide menu screen, y on the 480 high one, to the screen's own.
bool mouseRead(App* app, CPU& cpu, reg32 esp, reg32 buttons, reg32 x, reg32 y, reg32 wheel);  // 0x451500
int32_t screenX(App* app, int32_t x);
int32_t screenY(App* app, int32_t y);
// The game's 0x4513c0: the menus' text's colour set to `colour` (eax left as it came).
void textColourSet(App* app, reg32 colour);
// The game's 0x451590: `text` at x, y of the menus' screen; esp the frame base.
Result textDrawnAt(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y);
// The game's 0x451530: the screen behind the menus dimmed by the float at `slot` (its stack argument's place, where a
// race's pause screens put their shade instead): that shade, else `eax`.
reg32 dimmed(App* app, CPU& cpu, reg32 slot, reg32 eax);
// The game's 0x451960: the menus' key now, a held one repeating when `repeating`; esp the frame base.
Result menuKeyOf(App* app, CPU& cpu, reg32 esp, reg32 repeating);

/* Those whose code leaves a value on the x87's stack (Watcom's float result): the value comes back
 * as a double, and eax as the code leaves it; their thunks hand it back in Args::floatResult. */
struct Width
{
    reg32 eax;
    double value;
};
using WidthResult = std::optional<Width>;

/* What 0x4d06f0 is handed: the character's number (0xe0 a font, from the space), its square of the
 * font's texture, where it goes (x a float), its colour, whether it is only measured. */
struct Character
{
    reg32 number;
    int32_t textureX, textureY;
    float x;
    int32_t y;
    int32_t width, height;
    reg32 colour;
    int32_t measured;
};

// The game's 0x4d1390 (stdcall 4): the text scale set to `scale` (a float's bits); the one before (eax kept).
double textScaleSet(App* app, CPU& cpu, reg32 scale);
// The game's 0x4d06f0 (stdcall 0x18): a character of the fonts queued, unless only measured; how far the next
// goes.  Its x moves to the quad's left edge in its stack argument's place, esp + 4; esp the frame base.
WidthResult characterDrawn(App* app, CPU& cpu, reg32 esp, Character ch);
// The game's 0x4d1430: how wide `text` is, in whole pixels; esp the frame base.
WidthResult textMeasured(App* app, CPU& cpu, reg32 esp, reg32 text);
// The game's 0x4d1480: `text` measured, its last character's overhang added; esp the frame base.
WidthResult textMeasuredWithOverhang(App* app, CPU& cpu, reg32 esp, reg32 text);
// The game's 0x4d1000: `text` at x, y in the font's colour, or only measured; how wide; esp the frame base.
WidthResult textDrawn(App* app, CPU& cpu, reg32 esp, reg32 text, int32_t x, int32_t y);

// The game's 0x49bd30 (stdcall 4): the quad a, b, c, d queued on `list` (`d` the stack's word); esp the frame base.
Result quadQueued(App* app, CPU& cpu, reg32 esp, reg32 list, reg32 a, reg32 b, reg32 c, reg32 d);
// The game's 0x49bdc0 (stdcall 4): the quad taken off `list`'s queue; esp the frame base.
Result quadUnqueued(App* app, CPU& cpu, reg32 esp, reg32 list, reg32 a, reg32 b, reg32 c, reg32 d);
// The game's 0x49be40: `list`'s quads drawn; esp the frame base.
Result quadsFlushed(App* app, CPU& cpu, reg32 esp, reg32 list);

/* What 0x451cd0 is handed: the text, its font, colour and place (y moves a
 * line down as each is drawn, in the argument's place on the stack), how wide
 * a line may be; where to put how wide the widest line is, how high the box,
 * how many lines; whether it is only measured; which lines are drawn, from
 * the first. */
struct Box
{
    reg32 text, font, colour;
    int32_t x, y;
    int32_t width;
    reg32 widest, height, lines;
    int32_t measureOnly;
    int32_t count, first;
};
// The game's 0x451cd0 (stdcall 0x20): `b`'s text wrapped into its box; eax as the code leaves it; esp the frame base.
Result wrappedText(App* app, CPU& cpu, reg32 esp, Box b);
// The game's 0x451fc0 (stdcall 0x18): `text` in `font` and `colour` wrapped into a box `width` wide at x, y, every line
// drawn (`measureOnly` 0); where the widest line, the height and the lines go; esp the frame base.
Result wrappedBoxDrawn(App* app, CPU& cpu, reg32 esp, reg32 text, reg32 font, reg32 colour, int32_t x, int32_t y,
                       int32_t width, reg32 widest, reg32 height, reg32 lines, int32_t measureOnly);
// The game's 0x451ff0 (stdcall 8): `text` in `font` measured in a box `width` wide: the widest line, the height and
// the lines to the three places; esp the frame base.
Result boxMeasured(App* app, CPU& cpu, reg32 esp, reg32 text, reg32 font, int32_t width, reg32 widest, reg32 height,
                   reg32 lines);
// The game's 0x4d1540 and 0x4d1590: character `character`'s height (less one) and its drop, in pixels of the font as drawn.
int32_t characterHeight(App* app, CPU& cpu, reg32 character);
int32_t characterDrop(App* app, CPU& cpu, reg32 character);
// The game's 0x4beb80: the screen's viewport left, top, width and height put at the four places; the height.
reg32 viewportOf(App* app, reg32 leftAt, reg32 topAt, reg32 widthAt, reg32 heightAt);
}  // namespace text
}  // namespace port
}  // namespace nfs3hp

#endif
