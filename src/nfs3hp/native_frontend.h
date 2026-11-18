#ifndef NATIVE_FRONTEND_H_
#define NATIVE_FRONTEND_H_

#include "game_settings.h"
#include "native_guest.h"

#include <lib/winapp.h>
#include <cpu.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>

/* The game's menus, decompiled screen by screen, and the menu engine under
 * them a layer at a time, each function native whole.
 *
 * These are not about speed.  They are there to be read and changed.  The menus' state stays in guest memory, where the rest of
 * the game reads it, and what the menu engine does for a screen -- finding an
 * item, a line of text, a dialog box -- is a call into the game's own function
 * at its address.  A stand-in leaves memory and registers as the game's function does: eax its result, the registers it saves as they came (see
 * Restores), the return address and any stack arguments popped.  Not the
 * flags, which no caller of a C function reads, and not the stack below the returned esp. */
namespace nfs3hp
{
namespace fe
{

/* The game was told to stop (cpu.terminate) inside a call into it: the
 * stand-in returns at once, as the game's code does. */
struct Terminated
{
};

/* The registers and stack a call into the game takes.  Watcom passes the
 * first four arguments in eax, edx, ebx and ecx, the rest on the stack with
 * the first of them nearest the return address; the result comes back in eax.
 * A register past `registerCount` keeps whatever value it has. */
struct Arguments
{
    std::array<x86::reg32, 4> registers{};  // eax, edx, ebx, ecx
    std::size_t registerCount = 0;
    std::array<x86::reg32, 16> stack{};
    std::size_t stackCount = 0;
};

inline Arguments args(std::initializer_list<x86::reg32> registers, std::initializer_list<x86::reg32> stack = {})
{
    Arguments a;
    for (const x86::reg32 value : registers)
        a.registers[a.registerCount++] = value;
    for (const x86::reg32 value : stack)
        a.stack[a.stackCount++] = value;
    return a;
}

/* Guest memory and calls into guest code, for a stand-in. */
class Guest
{
public:
    Guest(win32::WinApplication* app, x86::CPU& cpu) : m_app(app), m_cpu(cpu) {}

    win32::WinApplication* app() const { return m_app; }
    /* The guest's processor, for a direct call (fe::direct) from code that has only the Guest. */
    x86::CPU& cpu() const { return m_cpu; }

    std::uint8_t byte(x86::reg32 address) const { return m_app->getMemory<x86::reg8>(address); }
    std::uint16_t word(x86::reg32 address) const { return m_app->getMemory<x86::reg16>(address); }
    std::uint32_t dword(x86::reg32 address) const { return m_app->getMemory<x86::reg32>(address); }
    void setByte(x86::reg32 address, std::uint8_t value) { m_app->getMemory<x86::reg8>(address) = value; }
    void setWord(x86::reg32 address, std::uint16_t value) { m_app->getMemory<x86::reg16>(address) = value; }
    void setDword(x86::reg32 address, std::uint32_t value) { m_app->getMemory<x86::reg32>(address) = value; }
    float floatAt(x86::reg32 address) const { return m_app->getMemory<float>(address); }
    void setFloat(x86::reg32 address, float value) { m_app->getMemory<float>(address) = value; }

    /* A zero-terminated string copied within guest memory, terminator and all. */
    void copyString(x86::reg32 to, x86::reg32 from);
    /* Its length, as strlen gives it. */
    std::uint32_t length(x86::reg32 string) const;
    /* Whether the string at `address` is `text`. */
    bool holds(x86::reg32 address, const char* text) const;

    /* The guest function at `function`, called as a `call` instruction calls
     * it: a slot for the return address, the function, its result from eax.
     * Stack arguments are popped afterwards whether the callee popped them
     * (ret n) or left them to the caller.  Throws Terminated when the game
     * stops. */
    x86::reg32 call(x86::reg32 function, const Arguments& arguments = Arguments());

    /* The lowest address of the stand-in's frame, laid out as the game's function's: its locals, where they were, for what the stand-in hands
     * to the game by address. */
    x86::reg32 frame() const { return m_frame; }
    void setFrame(x86::reg32 frame) { m_frame = frame; }


private:
    win32::WinApplication* m_app;
    x86::CPU& m_cpu;
    x86::reg32 m_frame = 0;
};

/* A typed core's own frame on the guest's stack (plan step 5): `bytes` below esp as the core
 * starts -- the game's function's pushes and locals, its Site's frameBytes -- made guest's
 * frame(), and esp back as the core ends, however it ends.  Called as the game's code calls it
 * (a thunk, or guest::direct, which leaves the return address's slot under esp), the frame
 * lies where the game's does.  Only for a frame whose address something called is handed (a
 * buffer, a string, strtok's pointer); C++ locals otherwise. */
class Frame
{
public:
    Frame(Guest& guest, x86::CPU& cpu, x86::reg32 bytes) : m_cpu(cpu), m_esp(cpu.esp)
    {
        cpu.esp -= bytes;
        guest.setFrame(cpu.esp);
    }
    ~Frame() { m_cpu.esp = m_esp; }
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;

private:
    x86::CPU& m_cpu;
    const x86::reg32 m_esp;
};

/* A typed core called as C++ from a menu's core (guest::direct, native_guest.h): another menu
 * file's, the port's, a library's.  Terminated thrown when the game stopped in it, as Guest::call
 * throws; its result as the core gives it (an optional, a bool, a value). */
template <x86::reg32 Address, auto Core, typename... Args>
auto direct(win32::WinApplication* app, x86::CPU& cpu, Args... args)
{
    if constexpr (std::is_void_v<decltype(guest::direct<Address, Core>(app, cpu, args...))>)
    {
        guest::direct<Address, Core>(app, cpu, args...);
        if (cpu.terminate)
            throw Terminated();
    }
    else
    {
        auto result = guest::direct<Address, Core>(app, cpu, args...);
        if (cpu.terminate)
            throw Terminated();
        return result;
    }
}

}  // namespace fe

inline GameSettings& gameSettings(const fe::Guest& guest)
{
    return gameSettings(guest.app());
}

namespace fe
{

/* The x87 as the game's code uses it, for a stand-in that computes in
 * floating point as the game does: each sum and product rounded as the
 * control word asks (a float's precision in a race's pause screens, see
 * x86::FloatEnv), a comparison leaving the status word as fcom leaves it, and
 * sub_4dfd56 -- frndint towards zero -- called where it is, the value on the
 * x87's stack.  A float in guest memory comes in and goes out as fld and
 * fst move it: Float(guest.floatAt(...)), float(value). */
class FloatMath
{
public:
    explicit FloatMath(Guest& guest) : m_guest(guest) {}

    // fild
    static x86::Float integer(std::int32_t value) { return x86::Float(x86::sreg32(value)); }
    x86::Float load(x86::reg32 address) const { return x86::Float(m_guest.floatAt(address)); }

    // The game's single precision, to the bit, with nothing of the x87 behind it (fp::, fpu.h).
    x86::Float add(const x86::Float& a, const x86::Float& b) { return fp::add(a, b); }
    x86::Float sub(const x86::Float& a, const x86::Float& b) { return fp::sub(a, b); }
    x86::Float mul(const x86::Float& a, const x86::Float& b) { return fp::mul(a, b); }

    /* fcom a, b and what a jump after it reads: ja, and jb (true unordered). */
    bool above(const x86::Float& a, const x86::Float& b);
    bool below(const x86::Float& a, const x86::Float& b);

    /* sub_4dfd56 and fistp: `value` cut to an integer towards zero. */
    std::int32_t truncated(const x86::Float& value);

private:
    Guest& m_guest;
};

/* A string of the game's own data, at the address the game keeps it: a
 * codelink, a list's name.  A stand-in hands the address to the game, which
 * compares or keeps it, so it has to be that one and not a copy. */
struct GuestString
{
    x86::reg32 address;
    const char* text;
};

/* ---------------------------------------------------------------------------
 * The menu engine's data
 * ------------------------------------------------------------------------- */

/* A screen: 36 bytes in the table at 0x6040d0, one for each MENUS/<name>.MNU
 * loaded.  sub_442b10 fills the first five fields from the handler table at
 * 0x555c48, found there by the screen's name. */
namespace menu
{
constexpr x86::reg32 kOnLoad = 0x00;      // once, after the menus are loaded (sub_449990)
constexpr x86::reg32 kOnEnter = 0x08;     // as the screen opens (sub_44ab40)
constexpr x86::reg32 kOnExit = 0x0c;      // as it closes (sub_44aa30, sub_44b1a0)
constexpr x86::reg32 kOnFrame = 0x10;     // every pass of the menu loop (sub_44b230); nonzero leaves the screen
constexpr x86::reg32 kName = 0x14;        // char*, the .MNU's name
constexpr x86::reg32 kFirstItem = 0x18;   // int16, its first item in the item table
constexpr x86::reg32 kSelected = 0x1a;    // int16, the item selected, counted from the first
constexpr x86::reg32 kSize = 0x24;
}

/* An item of a screen: 188 bytes in the table at 0x604ee0.  The fields the
 * .MNU keywords fill are in the parser's table at 0x557c10 (keyword, kind,
 * field); the screens' code sets the rest. */
namespace item
{
constexpr x86::reg32 kTable = 0x604ee0;
constexpr x86::reg32 kSize = 0xbc;
constexpr x86::reg32 kType = 0x00;        // the widget: 5 a button, 17 a cascade... (registry at 0x557aa8)
constexpr x86::reg32 kState = 0x04;       // byte; kDisabled greys the item out
constexpr x86::reg32 kFlags = 0x05;       // byte; kHidden hides it, as `hidethis` does
constexpr x86::reg32 kX = 0x06;           // int16, "x", on the 640x480 screen
constexpr x86::reg32 kY = 0x08;           // int16, "y"
constexpr x86::reg32 kHelp = 0x0c;        // char*, "help": its line of help text
constexpr x86::reg32 kCodelink = 0x10;    // char*, "codelink": the name code finds it by
constexpr x86::reg32 kText = 0x3c;        // char*, "text"
constexpr x86::reg32 kNextMenu = 0x40;    // char*, a button's "nextmenu"
constexpr x86::reg32 kListName = 0x58;    // char*, a button's "listname": the list it pops up
constexpr x86::reg32 kOnClick = 0x64;     // a button's handler, set by code
constexpr std::uint8_t kDisabled = 0x01;
constexpr std::uint8_t kHidden = 0x10;
}

/* An item, found by its codelink; null when the screen has none by that name. */
class Item
{
public:
    Item(Guest& guest, x86::reg32 address) : m_guest(&guest), m_address(address) {}

    explicit operator bool() const { return m_address != 0; }
    x86::reg32 address() const { return m_address; }

    void setDisabled(bool disabled);
    void setHidden(bool hidden);
    void setX(std::int16_t x) { m_guest->setWord(m_address + item::kX, std::uint16_t(x)); }
    void setHelp(x86::reg32 text) { m_guest->setDword(m_address + item::kHelp, text); }
    void setText(x86::reg32 text) { m_guest->setDword(m_address + item::kText, text); }
    void setListName(x86::reg32 name) { m_guest->setDword(m_address + item::kListName, name); }
    void setOnClick(x86::reg32 handler) { m_guest->setDword(m_address + item::kOnClick, handler); }

private:
    Guest* m_guest;
    x86::reg32 m_address;
};

/* ---------------------------------------------------------------------------
 * The menu engine's functions, called where they are
 * ------------------------------------------------------------------------- */

/* sub_442a40: the screen's item by its codelink. */
Item findItem(Guest& guest, x86::reg32 menu, const GuestString& codelink);
/* sub_4d1850: a line of the language's text, by its number. */
x86::reg32 text(Guest& guest, std::uint32_t id);
/* sub_452020: how wide a text is drawn in a font, in pixels. */
std::int32_t textWidth(Guest& guest, x86::reg32 text, std::uint32_t font);
/* sub_444be0: a message of one line and one button, over the screen. */
void messageBox(Guest& guest, std::uint32_t textId);

/* The dialog box over a screen (sub_444c20): lines of text, a row of buttons,
 * and a line to type in.  While it is up the menu loop hands it the keys
 * (sub_445e60); a key that answers it calls `callback` with eax the answer
 * (kDialogEnter or kDialogEscape) and edx a byte, 1, that the callback clears
 * to keep the dialog up.  What the callback returns goes on to the menu loop,
 * its kDialogHandled as 0: the key is spent.  See native_frontend_dialogs.cpp. */
struct Dialog
{
    std::uint32_t lineCount;
    x86::reg32 lines;            // char*[lineCount]
    std::uint32_t buttonCount;
    x86::reg32 buttons;          // char*[buttonCount]
    std::uint32_t focus;         // the button highlighted first
    x86::reg32 edit;             // the text typed into, copied first into the dialog's own buffer
    std::uint32_t editLength;    // the most characters
    std::uint32_t editMode;      // handed with the buffer to sub_443860; 1 for a name
    x86::reg32 callback;
};
void openDialog(Guest& guest, const Dialog& dialog);
/* sub_444c10: the dialog's own buffer, what was typed. */
x86::reg32 dialogEditText(Guest& guest);
/* sub_4446c0: which button is highlighted, 0 for the first. */
std::uint32_t dialogFocus(Guest& guest);

/* The same box without a line to type in (sub_4446d0): a question and its
 * buttons.  Its keys come back the same way, through `callback`. */
struct Question
{
    std::uint32_t lineCount;
    x86::reg32 lines;            // char*[lineCount]
    std::uint32_t buttonCount;
    x86::reg32 buttons;          // char*[buttonCount]
    x86::reg32 buttonHelp;       // char*[buttonCount]: a line of help for each button (sub_445ac0), or 0
    std::uint32_t focus;         // the button highlighted first
    std::uint32_t column;        // nonzero: the buttons one above the other, not in a row
    x86::reg32 callback;
};
void openQuestion(Guest& guest, const Question& question);

constexpr std::uint32_t kDialogEnter = 1;   // Enter, or a click (sub_444f50)
constexpr std::uint32_t kDialogEscape = 3;  // Escape
constexpr std::uint32_t kDialogHandled = 2;

/* What a screen's handler hands the menu loop (sub_44b230) besides 0, nothing. */
// Back screen by screen to the first one (sub_44aec0, until the screen at 0x6040d0).
constexpr x86::reg32 kMenuBackToFirst = 4;
// The attract-mode movie has played over the screen: it closes without fading
// out (sub_44aa30), and the loop goes on.
constexpr x86::reg32 kMenuMoviePlayed = 9;

/* 1 when the menu's last input came from the mouse (sub_451960, from
 * sub_49c000's poll), 0 when from the keys. */
constexpr x86::reg32 kInputFromMouse = 0x664658;

/* ---------------------------------------------------------------------------
 * Running a stand-in
 * ------------------------------------------------------------------------- */

/* The registers the game's function hands back as it found them: the ones
 * it pushes.  Watcom knows which registers a function of its own module
 * changes, and the callers do not count on the others -- a parameter
 * register, say, or scratch the function leaves behind -- so a stand-in that
 * restores the same ones is as good to them. */
enum Restores : std::uint8_t
{
    kRestoreEbx = 0x01,
    kRestoreEcx = 0x02,
    kRestoreEdx = 0x04,
    kRestoreEsi = 0x08,
    kRestoreEdi = 0x10,
    kRestoreAll = 0x1f,
};

/* A stand-in: the game's function it stands in for, what that one pops,
 * its frame, the game's strings it hands to the game and the registers it
 * restores.  Kept in a static, one per stand-in, so the switches and the
 * strings are looked at once. */
struct Site
{
    const char* name;                 // "sub_437e00"
    x86::reg32 stackBytes;            // the arguments its ret pops
    /* What the game's function's prologue puts on the stack (the registers
     * it pushes, its locals) before it calls anything.  The stand-in leaves
     * the same room, so what it calls runs at the same stack addresses as it
     * did from the game's code: the C library's strtok keeps a pointer
     * into the stack between calls (the item lookup splits codelinks with it). */
    x86::reg32 frameBytes;
    const GuestString* strings;
    std::size_t stringCount;
    std::uint8_t restores = kRestoreAll;  // Restores; ebp always
    signed char ready = 0;            // its strings looked at
};

/* Whether the strings a screen that is native whole hands to the game are
 * where it expects them: logged the first time, not, and the screen runs all
 * the same -- there is no other code to leave it to. */
void checkStrings(Guest& guest, Site& site);

/* `body` run as the game's function runs, for a thunk (win32::ThunkPtr): in a frame of its size,
 * its result returned, the registers the function keeps left in `args` as they came (the others as
 * they came too: no caller reads them), esp moved on as its ret moves it; 0 when the game stopped
 * in it. */
template <typename Body>
x86::reg32 runThunk(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args, Site& site, Body body)
{
    Guest guest(app, cpu);
    if (site.ready == 0)
        checkStrings(guest, site);
    const x86::reg32 esp = cpu.esp;
    try
    {
        cpu.esp -= site.frameBytes;
        guest.setFrame(cpu.esp);
        const x86::reg32 result = body(guest);
        cpu.esp = esp + 4 + site.stackBytes;
        args.eax = result;
        return result;
    }
    catch (const Terminated&)
    {
        cpu.esp = esp;
        return 0;
    }
}

}

/* The main menu in its three forms (native_frontend_main.cpp) -- MAIN.MNU,
 * MAINSPLT.MNU, MAINMULT.MNU -- is native whole: its handlers are
 * in nfs3hp::frontend, declared with the game's (nfs3hp.h).  So is what it calls of the menu loop (a screen's
 * items shown, the shown screen's name: native_frontend_loop.cpp; the loop
 * itself is port::loop, port_loop.cpp) and of the dialogs (the dialogs
 * themselves are port::dialogs, port_dialogs.cpp): a question, a message and a
 * line to type in opened, their layout and the pointer on their button
 * (native_frontend_dialogs.cpp). */

/* The options screens (native_frontend_options.cpp): the options screen and
 * the audio feed's keys (audfeed), */
namespace frontend { x86::reg32 optionsOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 feedOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 feedOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 feedSeparate(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 feedTimeButtonKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 feedPositionButtonKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 feedSpeedButtonKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 feedTimeListKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 feedPositionListKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 feedSpeedListKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
/* the sound and its sliders, */
namespace frontend { x86::reg32 audioOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 audioOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 audioOnFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 audioVolumeDraw(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 audioMenuMusicMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 audioRaceMusicMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 audioEffectsMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 audioEngineMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 audioSpeechMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
/* and the graphics.  (The controls' screens -- the function keys, the fixed
 * ones, the controllers, their dead zones and force feedback -- are native
 * whole, declared with the game's.) */
namespace frontend { x86::reg32 graphicsOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 graphicsOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 graphicsOnFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 advancedOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 advancedOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 advancedOnFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 advancedIntensityMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }

/* The cars' screens (native_frontend_cars.cpp): car1 and car2, the music, the
 * developer's recorder, opponent, and cedbasic, a car's tuning, */
namespace frontend { x86::reg32 carMenuMusicBack(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 carSaveFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 trackSaveFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 carRecord(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 trackRecord(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 car1OnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 car2OnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 car1OnFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 car2OnFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 car1OnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 car2OnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 car1EditClicked(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 car2EditClicked(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 car1ColourClicked(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 car2ColourClicked(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 opponentFitCar(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 opponentFitArt(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 opponentOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 opponentOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 opponentOnFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningLoad(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningSave(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningEngineMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningBrakesMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningSteeringMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningGearsMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningSuspensionMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningAeroMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningTiresMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningSetSliders(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningRememberSliders(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningSlidersMoved(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningBindSliders(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningLoadAdvanced(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningDefaultsAnswered(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningDoneKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningCancelKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningCarListKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningDefaultsKey(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningBindDoneCancel(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningFitButtons(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningPrepare(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningAdvancedDiffers(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 tuningCopyAdvanced(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
/* and the track's (native_frontend_tracks.cpp): location and trkinfo. */
namespace frontend { x86::reg32 locationOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 locationOnFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 locationOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 infoPlaceTrackPlaces(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 infoOnEnter(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 infoOnFrame(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 infoOnExit(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 infoSpeak(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }
namespace frontend { x86::reg32 infoStopSpeaking(win32::WinApplication* app, x86::CPU& cpu, x86::Args& args); }

}

#endif
