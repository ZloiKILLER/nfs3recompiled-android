#include "native_frontend.h"
#include "native_frontend_dialogs.h"
#include "native_watcom.h"
#include "port_text.h"
#include <SDL3/SDL.h>
#include <string>

/* What the decompiled screens share: guest memory and calls, the items of a
 * screen, the menu engine's functions, and when a port function runs at all.  See
 * native_frontend.h. */
namespace nfs3hp
{
namespace fe
{

namespace
{

// The menu engine's functions, by address.
constexpr nfs::u32 kFindItem = 0x442a40;
constexpr nfs::u32 kText = 0x4d1850;
constexpr nfs::u32 kTextWidth = 0x452020;
constexpr nfs::u32 kMessageBox = 0x444be0;
constexpr nfs::u32 kOpenDialog = 0x444c20;
constexpr nfs::u32 kOpenQuestion = 0x4446d0;
constexpr nfs::u32 kDialogEditText = 0x444c10;
constexpr nfs::u32 kDialogFocus = 0x4446c0;
constexpr nfs::u32 kTruncate = 0x4dfd56;     // frndint towards zero

}

void Guest::copyString(nfs::u32 to, nfs::u32 from)
{
    for (;;)
    {
        const std::uint8_t c = byte(from++);
        setByte(to++, c);
        if (c == 0)
            return;
    }
}

std::uint32_t Guest::length(nfs::u32 string) const
{
    std::uint32_t n = 0;
    while (byte(string + n) != 0)
        ++n;
    return n;
}

bool Guest::holds(nfs::u32 address, const char* text) const
{
    for (;; ++address, ++text)
    {
        if (byte(address) != std::uint8_t(*text))
            return false;
        if (*text == 0)
            return true;
    }
}

nfs::u32 Guest::call(nfs::u32 function, const Arguments& arguments)
{
    nfs::Args args;
    args.ebp = guest::heldEbp();
    nfs::u32* const registers[4] = { &args.eax, &args.edx, &args.ebx, &args.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *registers[i] = arguments.registers[i];
    const nfs::u32 esp = m_ts.sp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        m_ts.sp -= 4;
        setDword(m_ts.sp, arguments.stack[i]);
    }
    // The return address's slot, left unwritten as the game's code leaves it.
    m_ts.sp -= 4;
    const nfs::u32 eax = m_app->callAddress(function, m_ts, args);
    if (m_ts.terminate)
        throw Terminated();
    m_ts.sp = esp;
    return eax;
}

void Item::setDisabled(bool disabled)
{
    const std::uint8_t state = m_guest->byte(m_address + item::kState);
    m_guest->setByte(m_address + item::kState,
                     std::uint8_t(disabled ? state | item::kDisabled : state & ~item::kDisabled));
}

void Item::setHidden(bool hidden)
{
    const std::uint8_t flags = m_guest->byte(m_address + item::kFlags);
    m_guest->setByte(m_address + item::kFlags, std::uint8_t(hidden ? flags | item::kHidden : flags & ~item::kHidden));
}

Item findItem(Guest& guest, nfs::u32 menu, const GuestString& codelink)
{
    return Item(guest, guest.call(kFindItem, args({ menu, codelink.address })));
}

/* The helpers below call their typed cores as C++ (fe::direct). */
nfs::u32 text(Guest& guest, std::uint32_t id)
{
    nfs::ThreadState& ts = guest.ts();
    // edx only reaches a name past the language's lines, which the menus never ask for.
    return nfs::u32(*direct<kText, port::text::lineText>(guest.app(), ts, ts.sp - 4, std::int32_t(id), 0u));
}

std::int32_t textWidth(Guest& guest, nfs::u32 text, std::uint32_t font)
{
    nfs::ThreadState& ts = guest.ts();
    return std::int32_t(*direct<kTextWidth, port::text::menuTextWidth>(guest.app(), ts, ts.sp - 4, text, font));
}

void messageBox(Guest& guest, std::uint32_t textId)
{
    direct<kMessageBox, frontend::dialogs::messageById>(guest.app(), guest.ts(), textId);
}

void openDialog(Guest& guest, const Dialog& dialog)
{
    direct<kOpenDialog, frontend::dialogs::openEdit>(
        guest.app(), guest.ts(), dialog.lineCount, dialog.lines, dialog.buttonCount, dialog.buttons, dialog.focus,
        dialog.edit, dialog.editLength, dialog.editMode, dialog.callback);
}

void openQuestion(Guest& guest, const Question& question)
{
    direct<kOpenQuestion, frontend::dialogs::openQuestion>(
        guest.app(), guest.ts(), question.lineCount, question.lines, question.buttonCount, question.buttons,
        question.buttonHelp, question.focus, question.column, question.callback);
}

nfs::u32 dialogEditText(Guest& guest)
{
    const nfs::u32 typed = direct<kDialogEditText, frontend::dialogs::editText>(guest.app(), guest.ts());
    return typed;
}

std::uint32_t dialogFocus(Guest& guest)
{
    const nfs::u32 focus = direct<kDialogFocus, frontend::dialogs::focusedButton>(guest.app(), guest.ts());
    return focus;
}

bool FloatMath::above(const nfs::Float& a, const nfs::Float& b)
{
    return a > b;  // c0 and c3 clear: neither below, equal nor unordered
}

bool FloatMath::below(const nfs::Float& a, const nfs::Float& b)
{
    return fp::compare(a, b) & 0x100;  // c0: below, or unordered
}

std::int32_t FloatMath::truncated(const nfs::Float& value)
{
    const double chopped = direct<kTruncate, watcom::clib::chop>(m_guest.app(), m_guest.ts(), double(value));
    return fp::toInteger<nfs::s32>(chopped);  // fistp
}

void checkStrings(Guest& guest, Site& site)
{
    for (std::size_t i = 0; i < site.stringCount; ++i)
    {
        const GuestString& string = site.strings[i];
        if (!guest.holds(string.address, string.text))
            SDL_Log("[FE] %s: 0x%x is not \"%s\"", site.name, string.address, string.text);
    }
    site.ready = 1;
}

}
}
