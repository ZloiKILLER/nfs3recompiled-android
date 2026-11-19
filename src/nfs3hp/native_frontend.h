#ifndef NATIVE_FRONTEND_H_
#define NATIVE_FRONTEND_H_

#include "game_settings.h"
#include "native_guest.h"
#include "os_threads.h"

#include <lib/app.h>
#include <game_thread.h>
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

/* The game was told to stop (ts.terminate) inside a call into it: the
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
    std::array<nfs::u32, 4> registers{};  // eax, edx, ebx, ecx
    std::size_t registerCount = 0;
    std::array<nfs::u32, 16> stack{};
    std::size_t stackCount = 0;
};

inline Arguments args(std::initializer_list<nfs::u32> registers, std::initializer_list<nfs::u32> stack = {})
{
    Arguments a;
    for (const nfs::u32 value : registers)
        a.registers[a.registerCount++] = value;
    for (const nfs::u32 value : stack)
        a.stack[a.stackCount++] = value;
    return a;
}

/* Guest memory and calls into guest code, for a stand-in. */
class Guest
{
public:
    Guest(nfs::App* app, nfs::ThreadState& ts) : m_app(app), m_ts(ts) {}

    nfs::App* app() const { return m_app; }
    /* The guest's processor, for a direct call (fe::direct) from code that has only the Guest. */
    nfs::ThreadState& ts() const { return m_ts; }

    std::uint8_t byte(nfs::u32 address) const { return m_app->getMemory<nfs::u8>(address); }
    std::uint16_t word(nfs::u32 address) const { return m_app->getMemory<nfs::u16>(address); }
    std::uint32_t dword(nfs::u32 address) const { return m_app->getMemory<nfs::u32>(address); }
    void setByte(nfs::u32 address, std::uint8_t value) { m_app->getMemory<nfs::u8>(address) = value; }
    void setWord(nfs::u32 address, std::uint16_t value) { m_app->getMemory<nfs::u16>(address) = value; }
    void setDword(nfs::u32 address, std::uint32_t value) { m_app->getMemory<nfs::u32>(address) = value; }
    float floatAt(nfs::u32 address) const { return m_app->getMemory<float>(address); }
    void setFloat(nfs::u32 address, float value) { m_app->getMemory<float>(address) = value; }

    /* A zero-terminated string copied within guest memory, terminator and all. */
    void copyString(nfs::u32 to, nfs::u32 from);
    /* Its length, as strlen gives it. */
    std::uint32_t length(nfs::u32 string) const;
    /* Whether the string at `address` is `text`. */
    bool holds(nfs::u32 address, const char* text) const;

    /* The guest function at `function`, called as a `call` instruction calls
     * it: a slot for the return address, the function, its result from eax.
     * Stack arguments are popped afterwards whether the callee popped them
     * (ret n) or left them to the caller.  Throws Terminated when the game
     * stops. */
    nfs::u32 call(nfs::u32 function, const Arguments& arguments = Arguments());

    /* The lowest address of the stand-in's frame, laid out as the game's function's: its locals, where they were, for what the stand-in hands
     * to the game by address. */
    nfs::u32 frame() const { return m_frame; }
    void setFrame(nfs::u32 frame) { m_frame = frame; }


private:
    nfs::App* m_app;
    nfs::ThreadState& m_ts;
    nfs::u32 m_frame = 0;
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
    Frame(Guest& guest, nfs::ThreadState& ts, nfs::u32 bytes) : m_ts(ts), m_esp(ts.sp)
    {
        ts.sp -= bytes;
        guest.setFrame(ts.sp);
    }
    ~Frame() { m_ts.sp = m_esp; }
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;

private:
    nfs::ThreadState& m_ts;
    const nfs::u32 m_esp;
};

/* A typed core called as C++ from a menu's core (guest::direct, native_guest.h): another menu
 * file's, the port's, a library's.  Terminated thrown when the game stopped in it, as Guest::call
 * throws; its result as the core gives it (an optional, a bool, a value). */
template <nfs::u32 Address, auto Core, typename... Args>
auto direct(nfs::App* app, nfs::ThreadState& ts, Args... args)
{
    if constexpr (std::is_void_v<decltype(guest::direct<Address, Core>(app, ts, args...))>)
    {
        guest::direct<Address, Core>(app, ts, args...);
        if (ts.terminate)
            throw Terminated();
    }
    else
    {
        auto result = guest::direct<Address, Core>(app, ts, args...);
        if (ts.terminate)
            throw Terminated();
        return result;
    }
}

/* The game's lock at `section` entered or left (os::locks) from a menu's code: 0, what the code
 * has in eax after it.  Terminated thrown when the game stopped while it waited. */
inline nfs::u32 enterLock(Guest& guest, nfs::ThreadState& ts, nfs::u32 section)
{
    if (!os::locks::enter(guest.app(), ts, section))
        throw Terminated();
    return 0;
}

inline nfs::u32 leaveLock(Guest& guest, nfs::ThreadState& ts, nfs::u32 section)
{
    if (!os::locks::leave(guest.app(), ts, section))
        throw Terminated();
    return 0;
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
 * nfs::FloatEnv), a comparison leaving the status word as fcom leaves it, and
 * sub_4dfd56 -- frndint towards zero -- called where it is, the value on the
 * x87's stack.  A float in guest memory comes in and goes out as fld and
 * fst move it: Float(guest.floatAt(...)), float(value). */
class FloatMath
{
public:
    explicit FloatMath(Guest& guest) : m_guest(guest) {}

    // fild
    static nfs::Float integer(std::int32_t value) { return nfs::Float(nfs::s32(value)); }
    nfs::Float load(nfs::u32 address) const { return nfs::Float(m_guest.floatAt(address)); }

    // The game's single precision, to the bit, with nothing of the x87 behind it (fp::, fpu.h).
    nfs::Float add(const nfs::Float& a, const nfs::Float& b) { return fp::add(a, b); }
    nfs::Float sub(const nfs::Float& a, const nfs::Float& b) { return fp::sub(a, b); }
    nfs::Float mul(const nfs::Float& a, const nfs::Float& b) { return fp::mul(a, b); }

    /* fcom a, b and what a jump after it reads: ja, and jb (true unordered). */
    bool above(const nfs::Float& a, const nfs::Float& b);
    bool below(const nfs::Float& a, const nfs::Float& b);

    /* sub_4dfd56 and fistp: `value` cut to an integer towards zero. */
    std::int32_t truncated(const nfs::Float& value);

private:
    Guest& m_guest;
};

/* A string of the game's own data, at the address the game keeps it: a
 * codelink, a list's name.  A stand-in hands the address to the game, which
 * compares or keeps it, so it has to be that one and not a copy. */
struct GuestString
{
    nfs::u32 address;
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
constexpr nfs::u32 kOnLoad = 0x00;      // once, after the menus are loaded (sub_449990)
constexpr nfs::u32 kOnEnter = 0x08;     // as the screen opens (sub_44ab40)
constexpr nfs::u32 kOnExit = 0x0c;      // as it closes (sub_44aa30, sub_44b1a0)
constexpr nfs::u32 kOnFrame = 0x10;     // every pass of the menu loop (sub_44b230); nonzero leaves the screen
constexpr nfs::u32 kName = 0x14;        // char*, the .MNU's name
constexpr nfs::u32 kFirstItem = 0x18;   // int16, its first item in the item table
constexpr nfs::u32 kSelected = 0x1a;    // int16, the item selected, counted from the first
constexpr nfs::u32 kSize = 0x24;
}

/* An item of a screen: 188 bytes in the table at 0x604ee0.  The fields the
 * .MNU keywords fill are in the parser's table at 0x557c10 (keyword, kind,
 * field); the screens' code sets the rest. */
namespace item
{
constexpr nfs::u32 kTable = 0x604ee0;
constexpr nfs::u32 kSize = 0xbc;
constexpr nfs::u32 kType = 0x00;        // the widget: 5 a button, 17 a cascade... (registry at 0x557aa8)
constexpr nfs::u32 kState = 0x04;       // byte; kDisabled greys the item out
constexpr nfs::u32 kFlags = 0x05;       // byte; kHidden hides it, as `hidethis` does
constexpr nfs::u32 kX = 0x06;           // int16, "x", on the 640x480 screen
constexpr nfs::u32 kY = 0x08;           // int16, "y"
constexpr nfs::u32 kHelp = 0x0c;        // char*, "help": its line of help text
constexpr nfs::u32 kCodelink = 0x10;    // char*, "codelink": the name code finds it by
constexpr nfs::u32 kText = 0x3c;        // char*, "text"
constexpr nfs::u32 kNextMenu = 0x40;    // char*, a button's "nextmenu"
constexpr nfs::u32 kListName = 0x58;    // char*, a button's "listname": the list it pops up
constexpr nfs::u32 kOnClick = 0x64;     // a button's handler, set by code
constexpr std::uint8_t kDisabled = 0x01;
constexpr std::uint8_t kHidden = 0x10;
}

/* An item, found by its codelink; null when the screen has none by that name. */
class Item
{
public:
    Item(Guest& guest, nfs::u32 address) : m_guest(&guest), m_address(address) {}

    explicit operator bool() const { return m_address != 0; }
    nfs::u32 address() const { return m_address; }

    void setDisabled(bool disabled);
    void setHidden(bool hidden);
    void setX(std::int16_t x) { m_guest->setWord(m_address + item::kX, std::uint16_t(x)); }
    void setHelp(nfs::u32 text) { m_guest->setDword(m_address + item::kHelp, text); }
    void setText(nfs::u32 text) { m_guest->setDword(m_address + item::kText, text); }
    void setListName(nfs::u32 name) { m_guest->setDword(m_address + item::kListName, name); }
    void setOnClick(nfs::u32 handler) { m_guest->setDword(m_address + item::kOnClick, handler); }

private:
    Guest* m_guest;
    nfs::u32 m_address;
};

/* ---------------------------------------------------------------------------
 * The menu engine's functions, called where they are
 * ------------------------------------------------------------------------- */

/* sub_442a40: the screen's item by its codelink. */
Item findItem(Guest& guest, nfs::u32 menu, const GuestString& codelink);
/* sub_4d1850: a line of the language's text, by its number. */
nfs::u32 text(Guest& guest, std::uint32_t id);
/* sub_452020: how wide a text is drawn in a font, in pixels. */
std::int32_t textWidth(Guest& guest, nfs::u32 text, std::uint32_t font);
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
    nfs::u32 lines;            // char*[lineCount]
    std::uint32_t buttonCount;
    nfs::u32 buttons;          // char*[buttonCount]
    std::uint32_t focus;         // the button highlighted first
    nfs::u32 edit;             // the text typed into, copied first into the dialog's own buffer
    std::uint32_t editLength;    // the most characters
    std::uint32_t editMode;      // handed with the buffer to sub_443860; 1 for a name
    nfs::u32 callback;
};
void openDialog(Guest& guest, const Dialog& dialog);
/* sub_444c10: the dialog's own buffer, what was typed. */
nfs::u32 dialogEditText(Guest& guest);
/* sub_4446c0: which button is highlighted, 0 for the first. */
std::uint32_t dialogFocus(Guest& guest);

/* The same box without a line to type in (sub_4446d0): a question and its
 * buttons.  Its keys come back the same way, through `callback`. */
struct Question
{
    std::uint32_t lineCount;
    nfs::u32 lines;            // char*[lineCount]
    std::uint32_t buttonCount;
    nfs::u32 buttons;          // char*[buttonCount]
    nfs::u32 buttonHelp;       // char*[buttonCount]: a line of help for each button (sub_445ac0), or 0
    std::uint32_t focus;         // the button highlighted first
    std::uint32_t column;        // nonzero: the buttons one above the other, not in a row
    nfs::u32 callback;
};
void openQuestion(Guest& guest, const Question& question);

constexpr std::uint32_t kDialogEnter = 1;   // Enter, or a click (sub_444f50)
constexpr std::uint32_t kDialogEscape = 3;  // Escape
constexpr std::uint32_t kDialogHandled = 2;

/* What a screen's handler hands the menu loop (sub_44b230) besides 0, nothing. */
// Back screen by screen to the first one (sub_44aec0, until the screen at 0x6040d0).
constexpr nfs::u32 kMenuBackToFirst = 4;
// The attract-mode movie has played over the screen: it closes without fading
// out (sub_44aa30), and the loop goes on.
constexpr nfs::u32 kMenuMoviePlayed = 9;

/* 1 when the menu's last input came from the mouse (sub_451960, from
 * sub_49c000's poll), 0 when from the keys. */
constexpr nfs::u32 kInputFromMouse = 0x664658;

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
    nfs::u32 stackBytes;            // the arguments its ret pops
    /* What the game's function's prologue puts on the stack (the registers
     * it pushes, its locals) before it calls anything.  The stand-in leaves
     * the same room, so what it calls runs at the same stack addresses as it
     * did from the game's code: the C library's strtok keeps a pointer
     * into the stack between calls (the item lookup splits codelinks with it). */
    nfs::u32 frameBytes;
    const GuestString* strings;
    std::size_t stringCount;
    std::uint8_t restores = kRestoreAll;  // Restores; ebp always
    signed char ready = 0;            // its strings looked at
};

/* Whether the strings a screen that is native whole hands to the game are
 * where it expects them: logged the first time, not, and the screen runs all
 * the same -- there is no other code to leave it to. */
void checkStrings(Guest& guest, Site& site);

/* `body` run as the game's function runs, for a thunk (nfs::ThunkPtr): in a frame of its size,
 * its result returned, the registers the function keeps left in `args` as they came (the others as
 * they came too: no caller reads them), esp moved on as its ret moves it; 0 when the game stopped
 * in it. */
template <typename Body>
nfs::u32 runThunk(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args, Site& site, Body body)
{
    Guest guest(app, ts);
    if (site.ready == 0)
        checkStrings(guest, site);
    const nfs::u32 esp = ts.sp;
    try
    {
        ts.sp -= site.frameBytes;
        guest.setFrame(ts.sp);
        const nfs::u32 result = body(guest);
        ts.sp = esp + 4 + site.stackBytes;
        args.eax = result;
        return result;
    }
    catch (const Terminated&)
    {
        ts.sp = esp;
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
namespace frontend { nfs::u32 optionsOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 feedOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 feedOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 feedSeparate(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 feedTimeButtonKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 feedPositionButtonKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 feedSpeedButtonKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 feedTimeListKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 feedPositionListKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 feedSpeedListKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
/* the sound and its sliders, */
namespace frontend { nfs::u32 audioOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 audioOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 audioOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 audioVolumeDraw(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 audioMenuMusicMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 audioRaceMusicMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 audioEffectsMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 audioEngineMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 audioSpeechMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
/* and the graphics.  (The controls' screens -- the function keys, the fixed
 * ones, the controllers, their dead zones and force feedback -- are native
 * whole, declared with the game's.) */
namespace frontend { nfs::u32 graphicsOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 graphicsOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 graphicsOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 advancedOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 advancedOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 advancedOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 advancedIntensityMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }

/* The cars' screens (native_frontend_cars.cpp): car1 and car2, the music, the
 * developer's recorder, opponent, and cedbasic, a car's tuning, */
namespace frontend { nfs::u32 carMenuMusicBack(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 carSaveFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 trackSaveFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 carRecord(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 trackRecord(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 car1OnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 car2OnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 car1OnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 car2OnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 car1OnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 car2OnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 car1EditClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 car2EditClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 car1ColourClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 car2ColourClicked(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 opponentFitCar(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 opponentFitArt(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 opponentOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 opponentOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 opponentOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningLoad(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningSave(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningEngineMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningBrakesMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningSteeringMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningGearsMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningSuspensionMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningAeroMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningTiresMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningSetSliders(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningRememberSliders(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningSlidersMoved(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningBindSliders(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningLoadAdvanced(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningDefaultsAnswered(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningDoneKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningCancelKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningCarListKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningDefaultsKey(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningBindDoneCancel(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningFitButtons(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningPrepare(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningAdvancedDiffers(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 tuningCopyAdvanced(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
/* and the track's (native_frontend_tracks.cpp): location and trkinfo. */
namespace frontend { nfs::u32 locationOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 locationOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 locationOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 infoPlaceTrackPlaces(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 infoOnEnter(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 infoOnFrame(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 infoOnExit(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 infoSpeak(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }
namespace frontend { nfs::u32 infoStopSpeaking(nfs::App* app, nfs::ThreadState& ts, nfs::Args& args); }

}

#endif
