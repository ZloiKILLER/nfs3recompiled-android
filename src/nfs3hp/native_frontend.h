#ifndef NATIVE_FRONTEND_H_
#define NATIVE_FRONTEND_H_

#include <lib/winapp.h>
#include <cpu.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>

/* The game's menus, decompiled screen by screen, and the menu engine under
 * them a layer at a time (tools/apply_native_frontend.py).
 *
 * Each function here stands in for one of the generated ones and is entered
 * from its top, as native_vertices.cpp's are: the generated function stays
 * behind it and runs whenever this one says no -- with NFS_FE_NATIVE=0, for a
 * function named in NFS_FE_NATIVE_OFF, or when the game's strings it relies on
 * are not where it expects them.
 *
 * Unlike the vertex loops these are not about speed.  They are there to be
 * read and changed.  The menus' state stays in guest memory, where the rest of
 * the game reads it, and what the menu engine does for a screen -- finding an
 * item, a line of text, a dialog box -- is a call into the game's own function
 * at its address.  A stand-in leaves memory and registers as the generated
 * function does: eax its result, the registers it saves as they came (see
 * Restores), the return address and any stack arguments popped.  Not the
 * flags, which no caller of a C function reads (the generator computes none
 * of them at a return), and not the stack below the returned esp. */
namespace nfs3hp
{
namespace fe
{

/* The game was told to stop (cpu.terminate) inside a call into it: the
 * stand-in returns at once, as the generated code does. */
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

    std::uint8_t byte(x86::reg32 address) const { return m_app->getMemory<x86::reg8>(address); }
    std::uint16_t word(x86::reg32 address) const { return m_app->getMemory<x86::reg16>(address); }
    std::uint32_t dword(x86::reg32 address) const { return m_app->getMemory<x86::reg32>(address); }
    x86::reg32 eax() const { return m_cpu.eax; }
    x86::reg32 edx() const { return m_cpu.edx; }
    /* A register the stand-in does not restore, left as the generated
     * function leaves it, for callers that go on with it. */
    void leaveEbx(x86::reg32 value) { m_cpu.ebx = value; }
    void leaveEcx(x86::reg32 value) { m_cpu.ecx = value; }
    void leaveEdx(x86::reg32 value) { m_cpu.edx = value; }
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

    /* The lowest address of the stand-in's frame, laid out as the generated
     * function's: its locals, where they were, for what the stand-in hands
     * to the game by address. */
    x86::reg32 frame() const { return m_frame; }
    void setFrame(x86::reg32 frame) { m_frame = frame; }

    /* A float result on the x87 stack, as `fld dword` leaves it, and one
     * taken off it, as `fstp dword` takes it. */
    void pushFloat(float value) { m_cpu.fpu.push(x86::Float(value)); }
    float popFloat() { return float(m_cpu.fpu.pop()); }
    x86::FPU& fpu() { return m_cpu.fpu; }

private:
    win32::WinApplication* m_app;
    x86::CPU& m_cpu;
    x86::reg32 m_frame = 0;
};

/* The x87 as the generated code uses it, for a stand-in that computes in
 * floating point as the game does: each sum and product rounded as the
 * control word asks (a float's precision in a race's pause screens, see
 * x86::FPU), a comparison leaving the status word as fcom leaves it, and
 * sub_4dfd56 -- frndint towards zero -- called where it is, the value on the
 * x87's stack.  A float in guest memory comes in and goes out as fld and
 * fst move it: Float(guest.floatAt(...)), float(value). */
class X87
{
public:
    explicit X87(Guest& guest) : m_guest(guest), m_fpu(guest.fpu()) {}

    // fild
    static x86::Float integer(std::int32_t value) { return x86::Float(x86::sreg32(value)); }
    x86::Float load(x86::reg32 address) const { return x86::Float(m_guest.floatAt(address)); }

    x86::Float add(const x86::Float& a, const x86::Float& b) { return m_fpu.add(a, b); }
    x86::Float sub(const x86::Float& a, const x86::Float& b) { return m_fpu.sub(a, b); }
    x86::Float mul(const x86::Float& a, const x86::Float& b) { return m_fpu.mul(a, b); }

    /* fcom a, b and what a jump after it reads: ja, and jb (true unordered). */
    bool above(const x86::Float& a, const x86::Float& b);
    bool below(const x86::Float& a, const x86::Float& b);

    /* sub_4dfd56 and fistp: `value` cut to an integer towards zero. */
    std::int32_t truncated(const x86::Float& value);

private:
    Guest& m_guest;
    x86::FPU& m_fpu;
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

/* The registers a generated function hands back as it found them: the ones
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

/* A stand-in: the generated function it stands in for, what that one pops,
 * its frame, the game's strings it hands to the game and the registers it
 * restores.  Kept in a static, one per stand-in, so the switches and the
 * strings are looked at once. */
struct Site
{
    const char* name;                 // "sub_45d050", as NFS_FE_NATIVE_OFF names it
    x86::reg32 stackBytes;            // the arguments its ret pops
    /* What the generated function's prologue puts on the stack (the registers
     * it pushes, its locals) before it calls anything.  The stand-in leaves
     * the same room, so what it calls runs at the same stack addresses as it
     * did from the generated code: the C library's strtok keeps a pointer
     * into the stack between calls (the item lookup splits codelinks with it). */
    x86::reg32 frameBytes;
    const GuestString* strings;
    std::size_t stringCount;
    std::uint8_t restores = kRestoreAll;  // Restores; ebp always
    signed char ready = 0;            // 0 not looked at yet, 1 native, -1 generated
};

/* Whether `site` runs native: not with NFS_FE_NATIVE=0, nor when
 * NFS_FE_NATIVE_OFF=sub_45d050,sub_45cff0 names it, nor when one of its
 * strings is not where it expects it.  Decided and logged the first time. */
bool ready(Guest& guest, Site& site);

/* `body` run as the generated function runs: in a frame of its size, its
 * result in eax, the registers it restores restored, the return address and
 * the arguments popped.  A function with nothing to return returns
 * guest.eax(), what the calls it made left there.  False --
 * the generated code runs -- when the stand-in is not ready. */
template <typename Body>
bool run(win32::WinApplication* app, x86::CPU& cpu, Site& site, Body body)
{
    Guest guest(app, cpu);
    if (site.ready < 0 || (site.ready == 0 && !ready(guest, site)))
        return false;
    const x86::reg32 ebx = cpu.ebx, ecx = cpu.ecx, edx = cpu.edx;
    const x86::reg32 esi = cpu.esi, edi = cpu.edi, ebp = cpu.ebp, esp = cpu.esp;
    try
    {
        cpu.esp -= site.frameBytes;
        guest.setFrame(cpu.esp);
        const x86::reg32 result = body(guest);
        cpu.eax = result;
        if (site.restores & kRestoreEbx)
            cpu.ebx = ebx;
        if (site.restores & kRestoreEcx)
            cpu.ecx = ecx;
        if (site.restores & kRestoreEdx)
            cpu.edx = edx;
        if (site.restores & kRestoreEsi)
            cpu.esi = esi;
        if (site.restores & kRestoreEdi)
            cpu.edi = edi;
        cpu.ebp = ebp;
        cpu.esp = esp + 4 + site.stackBytes;
    }
    catch (const Terminated&)
    {
    }
    return true;
}

}

/* The menu files and the screens they make (native_frontend_menus.cpp). */
bool menuCodelinkMatches(win32::WinApplication* app, x86::CPU& cpu);
bool menuFindItem(win32::WinApplication* app, x86::CPU& cpu);
bool menuAssignHandlers(win32::WinApplication* app, x86::CPU& cpu);
bool menuKeywordIndex(win32::WinApplication* app, x86::CPU& cpu);
bool menuStoreString(win32::WinApplication* app, x86::CPU& cpu);
bool menuWidgetType(win32::WinApplication* app, x86::CPU& cpu);
bool menuWidgetName(win32::WinApplication* app, x86::CPU& cpu);
bool menuInitItem(win32::WinApplication* app, x86::CPU& cpu);
bool menuSectionType(win32::WinApplication* app, x86::CPU& cpu);
bool menuParseInt(win32::WinApplication* app, x86::CPU& cpu);
bool menuParseLong(win32::WinApplication* app, x86::CPU& cpu);
bool menuParseFloat(win32::WinApplication* app, x86::CPU& cpu);
bool menuSetField(win32::WinApplication* app, x86::CPU& cpu);
bool menuReadEntry(win32::WinApplication* app, x86::CPU& cpu);
bool menuLoadMenus(win32::WinApplication* app, x86::CPU& cpu);
bool menuCheckMenuFile(win32::WinApplication* app, x86::CPU& cpu);
bool menuCheckMenuFiles(win32::WinApplication* app, x86::CPU& cpu);
bool menuCallOnLoad(win32::WinApplication* app, x86::CPU& cpu);

/* The menu loop and moving through the screens (native_frontend_loop.cpp). */
bool menuItemUsable(win32::WinApplication* app, x86::CPU& cpu);
bool menuKeyAllowed(win32::WinApplication* app, x86::CPU& cpu);
bool menuDrawHelp(win32::WinApplication* app, x86::CPU& cpu);
bool menuKeysTaken(win32::WinApplication* app, x86::CPU& cpu);
bool menuSelectable(win32::WinApplication* app, x86::CPU& cpu);
bool menuPointAtSelected(win32::WinApplication* app, x86::CPU& cpu);
bool menuSelectPrevious(win32::WinApplication* app, x86::CPU& cpu);
bool menuSelectNext(win32::WinApplication* app, x86::CPU& cpu);
bool menuEscapeButton(win32::WinApplication* app, x86::CPU& cpu);
bool menuHandleKey(win32::WinApplication* app, x86::CPU& cpu);
bool menuSelectedIndex(win32::WinApplication* app, x86::CPU& cpu);
bool menuSelectItem(win32::WinApplication* app, x86::CPU& cpu);
bool menuDrawItems(win32::WinApplication* app, x86::CPU& cpu);
bool menuShownNow(win32::WinApplication* app, x86::CPU& cpu);
bool menuReleaseMouse(win32::WinApplication* app, x86::CPU& cpu);
bool menuItemUnderMouse(win32::WinApplication* app, x86::CPU& cpu);
bool menuItemsShown(win32::WinApplication* app, x86::CPU& cpu);
bool menuPassItems(win32::WinApplication* app, x86::CPU& cpu);
bool menuShowItems(win32::WinApplication* app, x86::CPU& cpu);
bool menuHideItems(win32::WinApplication* app, x86::CPU& cpu);
bool menuLeaveScreen(win32::WinApplication* app, x86::CPU& cpu);
bool menuResetItems(win32::WinApplication* app, x86::CPU& cpu);
bool menuEnterScreen(win32::WinApplication* app, x86::CPU& cpu);
bool menuCurrentName(win32::WinApplication* app, x86::CPU& cpu);
bool menuByNameStandIn(win32::WinApplication* app, x86::CPU& cpu);
bool menuCurrent(win32::WinApplication* app, x86::CPU& cpu);
bool menuPop(win32::WinApplication* app, x86::CPU& cpu);
bool menuPush(win32::WinApplication* app, x86::CPU& cpu);
bool menuSetHistory(win32::WinApplication* app, x86::CPU& cpu);
bool menuPlayer1NameAnswer(win32::WinApplication* app, x86::CPU& cpu);
bool menuPlayer2NameAnswer(win32::WinApplication* app, x86::CPU& cpu);
bool menuQuitAnswer(win32::WinApplication* app, x86::CPU& cpu);
bool menuWaitWhileSuspended(win32::WinApplication* app, x86::CPU& cpu);
bool menuLoopStandIn(win32::WinApplication* app, x86::CPU& cpu);
bool menuFitRaceSettings(win32::WinApplication* app, x86::CPU& cpu);

/* The dialogs over the screens (native_frontend_dialogs.cpp): what is up, */
bool dialogShownStandIn(win32::WinApplication* app, x86::CPU& cpu);
bool dialogKindStandIn(win32::WinApplication* app, x86::CPU& cpu);
bool dialogFocusStandIn(win32::WinApplication* app, x86::CPU& cpu);
bool dialogEditText(win32::WinApplication* app, x86::CPU& cpu);
bool dialogListFocus(win32::WinApplication* app, x86::CPU& cpu);
bool dialogFadedIn(win32::WinApplication* app, x86::CPU& cpu);
bool dialogClose(win32::WinApplication* app, x86::CPU& cpu);
/* the layout, */
bool dialogLinesBox(win32::WinApplication* app, x86::CPU& cpu);
bool dialogButtonRowBox(win32::WinApplication* app, x86::CPU& cpu);
bool dialogPlaceButtons(win32::WinApplication* app, x86::CPU& cpu);
bool dialogPlaceEntries(win32::WinApplication* app, x86::CPU& cpu);
/* drawing, */
bool dialogDrawBox(win32::WinApplication* app, x86::CPU& cpu);
bool dialogDrawLines(win32::WinApplication* app, x86::CPU& cpu);
bool dialogDrawButtons(win32::WinApplication* app, x86::CPU& cpu);
bool dialogDrawEditField(win32::WinApplication* app, x86::CPU& cpu);
bool dialogDrawList(win32::WinApplication* app, x86::CPU& cpu);
bool dialogDrawHelp(win32::WinApplication* app, x86::CPU& cpu);
bool dialogLoadArt(win32::WinApplication* app, x86::CPU& cpu);
/* the pointer, */
bool dialogPointAtButton(win32::WinApplication* app, x86::CPU& cpu);
bool dialogButtonUnderMouse(win32::WinApplication* app, x86::CPU& cpu);
bool dialogPointAtEntry(win32::WinApplication* app, x86::CPU& cpu);
bool dialogEntryUnderMouse(win32::WinApplication* app, x86::CPU& cpu);
/* each kind opened, its keys and its pass, */
bool dialogOpenQuestion(win32::WinApplication* app, x86::CPU& cpu);
bool dialogQuestionKey(win32::WinApplication* app, x86::CPU& cpu);
bool dialogQuestionPass(win32::WinApplication* app, x86::CPU& cpu);
bool dialogOpenMessage(win32::WinApplication* app, x86::CPU& cpu);
bool dialogMessageById(win32::WinApplication* app, x86::CPU& cpu);
bool dialogEndMessage(win32::WinApplication* app, x86::CPU& cpu);
bool dialogMessageKey(win32::WinApplication* app, x86::CPU& cpu);
bool dialogMessagePass(win32::WinApplication* app, x86::CPU& cpu);
bool dialogOpenEdit(win32::WinApplication* app, x86::CPU& cpu);
bool dialogEditKey(win32::WinApplication* app, x86::CPU& cpu);
bool dialogEditPass(win32::WinApplication* app, x86::CPU& cpu);
bool dialogOpenList(win32::WinApplication* app, x86::CPU& cpu);
bool dialogListKey(win32::WinApplication* app, x86::CPU& cpu);
bool dialogListPass(win32::WinApplication* app, x86::CPU& cpu);
/* and the dialog up, whatever its kind. */
bool dialogKeyStandIn(win32::WinApplication* app, x86::CPU& cpu);
bool dialogPassStandIn(win32::WinApplication* app, x86::CPU& cpu);

/* The main menu in its three forms (native_frontend_main.cpp): MAIN.MNU, */
bool mainMenuOnEnter(win32::WinApplication* app, x86::CPU& cpu);
bool mainMenuOnExit(win32::WinApplication* app, x86::CPU& cpu);
bool mainMenuOnFrame(win32::WinApplication* app, x86::CPU& cpu);
bool mainMenuOnPlayerName(win32::WinApplication* app, x86::CPU& cpu);
bool mainMenuPlayerNameAnswer(win32::WinApplication* app, x86::CPU& cpu);
/* MAINSPLT.MNU, two players on one machine, */
bool splitMenuOnEnter(win32::WinApplication* app, x86::CPU& cpu);
bool splitMenuOnExit(win32::WinApplication* app, x86::CPU& cpu);
bool splitMenuOnFrame(win32::WinApplication* app, x86::CPU& cpu);
bool splitMenuOnPlayer2Name(win32::WinApplication* app, x86::CPU& cpu);
bool splitMenuPlayer2NameAnswer(win32::WinApplication* app, x86::CPU& cpu);
/* and MAINMULT.MNU, a race over a network. */
bool multiMenuOnEnter(win32::WinApplication* app, x86::CPU& cpu);
bool multiMenuOnExit(win32::WinApplication* app, x86::CPU& cpu);
bool multiMenuOnFrame(win32::WinApplication* app, x86::CPU& cpu);
bool multiMenuApplyRaceList(win32::WinApplication* app, x86::CPU& cpu);
bool multiMenuFollowRaceType(win32::WinApplication* app, x86::CPU& cpu);
bool multiMenuOnBack(win32::WinApplication* app, x86::CPU& cpu);
bool multiMenuLeaveAnswer(win32::WinApplication* app, x86::CPU& cpu);

}

#endif
