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
 * disassemble_nfs3hp.py binds each address to the adapter of that name here
 * (PORT_TEXT_NATIVES), and the generated code calls it by name; the other
 * natives call these adapters as the game's code would call the address. */
namespace nfs3hp
{
namespace port
{
namespace text
{

/* The adapters, under the game's functions' names. */
void textWidth(App* app, CPU& cpu);              // 0x452020
void selectFont(App* app, CPU& cpu);             // 0x452100
void measureText(App* app, CPU& cpu);            // 0x4d1430
void textWidthWithOverhang(App* app, CPU& cpu);  // 0x4d1480
void drawText(App* app, CPU& cpu);               // 0x4d1000
void drawCharacter(App* app, CPU& cpu);          // 0x4d06f0
void takeQuad(App* app, CPU& cpu);               // 0x49bcd0
void viewport(App* app, CPU& cpu);               // 0x4beb80
void toScreenX(App* app, CPU& cpu);              // 0x451400
void toScreenY(App* app, CPU& cpu);              // 0x451440
void specialText(App* app, CPU& cpu);            // 0x4d17b0
void languageText(App* app, CPU& cpu);           // 0x4d1850
void flushFonts(App* app, CPU& cpu);             // 0x4d1340
void setTextScale(App* app, CPU& cpu);           // 0x4d1390
void keepTextScale(App* app, CPU& cpu);          // 0x4d1400
void restoreTextScale(App* app, CPU& cpu);       // 0x4d1410
void characterWidth(App* app, CPU& cpu);         // 0x4d1540
void characterOverhang(App* app, CPU& cpu);      // 0x4d1590
void setTextColour(App* app, CPU& cpu);          // 0x4513c0
void screenToMenuX(App* app, CPU& cpu);          // 0x451480
void screenToMenuY(App* app, CPU& cpu);          // 0x4514c0
void readMenuMouse(App* app, CPU& cpu);          // 0x451500
void dimBackground(App* app, CPU& cpu);          // 0x451530
void drawTextAt(App* app, CPU& cpu);             // 0x451590
void drawMenuText(App* app, CPU& cpu);           // 0x452190
void flushQuads(App* app, CPU& cpu);             // 0x49be40
void joystickKey(App* app, CPU& cpu);            // 0x451700
void captureKeys(App* app, CPU& cpu);            // 0x451810
void scanCodeOfKey(App* app, CPU& cpu);          // 0x451840
void keyOfScanCode(App* app, CPU& cpu);          // 0x4518c0
void menuKey(App* app, CPU& cpu);                // 0x451960
void resetMenuKey(App* app, CPU& cpu);           // 0x451b20
void nextWord(App* app, CPU& cpu);               // 0x451b50
void drawWrappedText(App* app, CPU& cpu);        // 0x451cd0
void drawTextBox(App* app, CPU& cpu);            // 0x451fc0
void measureTextBox(App* app, CPU& cpu);         // 0x451ff0
void menusEnd(App* app, CPU& cpu);               // 0x452110
void queueQuad(App* app, CPU& cpu);              // 0x49bd30
void unqueueQuad(App* app, CPU& cpu);            // 0x49bdc0
void mouseMenuKey(App* app, CPU& cpu);           // 0x49c000
void loadLanguageText(App* app, CPU& cpu);       // 0x4d1750

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address
 * (ByAddress below, for the checks' stand-ins).  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its adapter hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * adapter hands it, so the core's frame lies where the game's does; by address the call makes it. */

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
// The game's 0x451530: the screen behind the menus dimmed by the float at `slot` (its stack argument's place, where a
// race's pause screens put their shade instead): that shade, else `eax`.
reg32 dimmed(App* app, CPU& cpu, reg32 slot, reg32 eax);
// The game's 0x451960: the menus' key now, a held one repeating when `repeating`; esp the frame base.
Result menuKeyOf(App* app, CPU& cpu, reg32 esp, reg32 repeating);

/* Those whose code leaves a value on the x87's stack (Watcom's float result): the value comes back
 * as a double, and eax as the code leaves it; their adapters put it on the x87's stack, and their
 * ByAddress take it off. */
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

}  // namespace text
}  // namespace port

namespace guest
{

// The text in eax, the font in edx; the width in eax.
template <>
struct ByAddress<port::text::menuTextWidth>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 text, x86::reg32 font)
    {
        return watcomEax(app, cpu, Address, {text, font});
    }
};

// The font in eax (eax left as it came).
template <>
struct ByAddress<port::text::selectFontOf>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 font)
    {
        watcom(app, cpu, Address, {font});
    }
};

// Stdcall: the text in eax, x in edx, y in ebx, the font in ecx; the alignment and the colour pushed (ret 8).
template <>
struct ByAddress<port::text::menuTextDrawn>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 text, std::int32_t x,
                             std::int32_t y, x86::reg32 font, std::int32_t align, x86::reg32 colour)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = text;
        cpu.edx = x86::reg32(x);
        cpu.ebx = x86::reg32(y);
        cpu.ecx = font;
        if (!guest::call(app, cpu, Address, {x86::reg32(align), colour}))
            return std::nullopt;
        return std::int32_t(cpu.eax);
    }
};

// Nothing handed over; eax back.
template <>
struct ByAddress<port::text::fontsDrawn>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32)
    {
        return watcomEax(app, cpu, Address, {});
    }
};

// Nothing handed over; the scale in eax.
template <>
struct ByAddress<port::text::keepTextScaleOf>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
        return cpu.eax;
    }
};

// Nothing handed over (eax left as it came).
template <>
struct ByAddress<port::text::restoreTextScaleOf>
{
    template <x86::reg32 Address>
    static void call(win32::WinApplication* app, x86::CPU& cpu)
    {
        watcom(app, cpu, Address, {});
    }
};

// The language in eax; eax back.
template <>
struct ByAddress<port::text::loadedLanguageText>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t language)
    {
        return watcomEax(app, cpu, Address, {x86::reg32(language)});
    }
};

// The number in eax, `edx` in edx; the text in eax.
template <>
struct ByAddress<port::text::lineText>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, std::int32_t number,
                             x86::reg32 edx)
    {
        return watcomEax(app, cpu, Address, {x86::reg32(number), edx});
    }
};

// Stdcall: `eax` in eax, the float at `slot` pushed (ret 4); what the function leaves in the argument's place
// written back to `slot`, where the core writes it; eax back.
template <>
struct ByAddress<port::text::dimmed>
{
    template <x86::reg32 Address>
    static x86::reg32 call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 slot, x86::reg32 eax)
    {
        const KeptRegisters kept(cpu);
        const x86::reg32 pushed = cpu.esp - 4;
        cpu.eax = eax;
        guest::call(app, cpu, Address, {guest::at(app, slot)});
        guest::put(app, slot, guest::at(app, pushed));
        return cpu.eax;
    }
};

// Whether a held key repeats in eax; the key in eax.
template <>
struct ByAddress<port::text::menuKeyOf>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 repeating)
    {
        return watcomEax(app, cpu, Address, {repeating});
    }
};

/* What a call by address left: eax, and the value taken off the x87's stack. */
inline port::text::WidthResult textWidthLeft(x86::CPU& cpu)
{
    const x86::reg32 eax = cpu.eax;
    return port::text::Width{eax, double(cpu.fpu.take())};
}

// Stdcall: the scale pushed (ret 4); the one before off the x87's stack.
template <>
struct ByAddress<port::text::textScaleSet>
{
    template <x86::reg32 Address>
    static double call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 scale)
    {
        const KeptRegisters kept(cpu);
        if (!guest::call(app, cpu, Address, {scale}))
            return 0.0;
        return double(cpu.fpu.take());
    }
};

// Stdcall: the number in eax, the texture's square in edx and ebx; x (a float's bits), y, the width, the height,
// the colour and whether only measured pushed (ret 0x18); eax back and the advance off the x87's stack.
template <>
struct ByAddress<port::text::characterDrawn>
{
    template <x86::reg32 Address>
    static port::text::WidthResult call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32,
                                        port::text::Character ch)
    {
        const KeptRegisters kept(cpu);
        cpu.eax = ch.number;
        cpu.edx = x86::reg32(ch.textureX);
        cpu.ebx = x86::reg32(ch.textureY);
        x86::reg32 x;
        std::memcpy(&x, &ch.x, sizeof x);
        if (!guest::call(app, cpu, Address,
                         {x, x86::reg32(ch.y), x86::reg32(ch.width), x86::reg32(ch.height), ch.colour,
                          x86::reg32(ch.measured)}))
            return std::nullopt;
        return textWidthLeft(cpu);
    }
};

// The text in eax; eax back and the width off the x87's stack.
template <>
struct ByAddress<port::text::textMeasured>
{
    template <x86::reg32 Address>
    static port::text::WidthResult call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 text)
    {
        if (!watcom(app, cpu, Address, {text}))
            return std::nullopt;
        return textWidthLeft(cpu);
    }
};

// The text in eax; eax back and the width off the x87's stack.
template <>
struct ByAddress<port::text::textMeasuredWithOverhang>
{
    template <x86::reg32 Address>
    static port::text::WidthResult call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 text)
    {
        if (!watcom(app, cpu, Address, {text}))
            return std::nullopt;
        return textWidthLeft(cpu);
    }
};

// The text in eax, x in edx, y in ebx; eax back and the width off the x87's stack.
template <>
struct ByAddress<port::text::textDrawn>
{
    template <x86::reg32 Address>
    static port::text::WidthResult call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 text,
                                        std::int32_t x, std::int32_t y)
    {
        if (!watcom(app, cpu, Address, {text, x86::reg32(x), x86::reg32(y)}))
            return std::nullopt;
        return textWidthLeft(cpu);
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
