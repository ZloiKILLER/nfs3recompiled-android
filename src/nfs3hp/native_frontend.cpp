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
constexpr x86::reg32 kFindItem = 0x442a40;
constexpr x86::reg32 kText = 0x4d1850;
constexpr x86::reg32 kTextWidth = 0x452020;
constexpr x86::reg32 kMessageBox = 0x444be0;
constexpr x86::reg32 kOpenDialog = 0x444c20;
constexpr x86::reg32 kOpenQuestion = 0x4446d0;
constexpr x86::reg32 kDialogEditText = 0x444c10;
constexpr x86::reg32 kDialogFocus = 0x4446c0;
constexpr x86::reg32 kTruncate = 0x4dfd56;     // frndint towards zero

}

void Guest::copyString(x86::reg32 to, x86::reg32 from)
{
    for (;;)
    {
        const std::uint8_t c = byte(from++);
        setByte(to++, c);
        if (c == 0)
            return;
    }
}

std::uint32_t Guest::length(x86::reg32 string) const
{
    std::uint32_t n = 0;
    while (byte(string + n) != 0)
        ++n;
    return n;
}

bool Guest::holds(x86::reg32 address, const char* text) const
{
    for (;; ++address, ++text)
    {
        if (byte(address) != std::uint8_t(*text))
            return false;
        if (*text == 0)
            return true;
    }
}

x86::reg32 Guest::call(x86::reg32 function, const Arguments& arguments)
{
    x86::Args args;
    args.ebp = guest::heldEbp();
    x86::reg32* const registers[4] = { &args.eax, &args.edx, &args.ebx, &args.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *registers[i] = arguments.registers[i];
    const x86::reg32 esp = m_cpu.esp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        m_cpu.esp -= 4;
        setDword(m_cpu.esp, arguments.stack[i]);
    }
    // The return address's slot, left unwritten as the game's code leaves it.
    m_cpu.esp -= 4;
    const x86::reg32 eax = m_app->callAddress(function, m_cpu, args);
    if (m_cpu.terminate)
        throw Terminated();
    m_cpu.esp = esp;
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

Item findItem(Guest& guest, x86::reg32 menu, const GuestString& codelink)
{
    return Item(guest, guest.call(kFindItem, args({ menu, codelink.address })));
}

/* The helpers below call their typed cores as C++ (fe::direct). */
x86::reg32 text(Guest& guest, std::uint32_t id)
{
    x86::CPU& cpu = guest.cpu();
    // edx only reaches a name past the language's lines, which the menus never ask for.
    return x86::reg32(*direct<kText, port::text::lineText>(guest.app(), cpu, cpu.esp - 4, std::int32_t(id), 0u));
}

std::int32_t textWidth(Guest& guest, x86::reg32 text, std::uint32_t font)
{
    x86::CPU& cpu = guest.cpu();
    return std::int32_t(*direct<kTextWidth, port::text::menuTextWidth>(guest.app(), cpu, cpu.esp - 4, text, font));
}

void messageBox(Guest& guest, std::uint32_t textId)
{
    direct<kMessageBox, frontend::dialogs::messageById>(guest.app(), guest.cpu(), textId);
}

void openDialog(Guest& guest, const Dialog& dialog)
{
    direct<kOpenDialog, frontend::dialogs::openEdit>(
        guest.app(), guest.cpu(), dialog.lineCount, dialog.lines, dialog.buttonCount, dialog.buttons, dialog.focus,
        dialog.edit, dialog.editLength, dialog.editMode, dialog.callback);
}

void openQuestion(Guest& guest, const Question& question)
{
    direct<kOpenQuestion, frontend::dialogs::openQuestion>(
        guest.app(), guest.cpu(), question.lineCount, question.lines, question.buttonCount, question.buttons,
        question.buttonHelp, question.focus, question.column, question.callback);
}

x86::reg32 dialogEditText(Guest& guest)
{
    const x86::reg32 typed = direct<kDialogEditText, frontend::dialogs::editText>(guest.app(), guest.cpu());
    return typed;
}

std::uint32_t dialogFocus(Guest& guest)
{
    const x86::reg32 focus = direct<kDialogFocus, frontend::dialogs::focusedButton>(guest.app(), guest.cpu());
    return focus;
}

bool FloatMath::above(const x86::Float& a, const x86::Float& b)
{
    return a > b;  // c0 and c3 clear: neither below, equal nor unordered
}

bool FloatMath::below(const x86::Float& a, const x86::Float& b)
{
    return fp::compare(a, b) & 0x100;  // c0: below, or unordered
}

std::int32_t FloatMath::truncated(const x86::Float& value)
{
    const double chopped = direct<kTruncate, watcom::clib::chop>(m_guest.app(), m_guest.cpu(), double(value));
    return fp::toInteger<x86::sreg32>(chopped);  // fistp
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
