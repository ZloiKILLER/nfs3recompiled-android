#include "native_frontend.h"
#include <SDL3/SDL.h>
#include <string>

/* What the decompiled screens share: guest memory and calls, the items of a
 * screen, the menu engine's functions, and when a stand-in runs at all.  See
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

bool allOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_FE_NATIVE");
        return !value || SDL_strcmp(value, "0") != 0;
    }();
    return on;
}

bool switchedOff(const char* name)
{
    static const std::string off = []() {
        const char* value = SDL_getenv("NFS_FE_NATIVE_OFF");
        return std::string(value ? value : "");
    }();
    if (off.empty())
        return false;
    const std::size_t length = SDL_strlen(name);
    for (std::size_t at = off.find(name); at != std::string::npos; at = off.find(name, at + 1))
    {
        const std::size_t end = at + length;
        if ((at == 0 || off[at - 1] == ',') && (end == off.size() || off[end] == ','))
            return true;
    }
    return false;
}

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
    x86::reg32* const registers[4] = { &m_cpu.eax, &m_cpu.edx, &m_cpu.ebx, &m_cpu.ecx };
    for (std::size_t i = 0; i < arguments.registerCount; ++i)
        *registers[i] = arguments.registers[i];
    const x86::reg32 esp = m_cpu.esp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        m_cpu.esp -= 4;
        setDword(m_cpu.esp, arguments.stack[i]);
    }
    // The return address's slot, left unwritten as the generated code leaves it.
    m_cpu.esp -= 4;
    m_app->dynamic_call(function, m_cpu);
    if (m_cpu.terminate)
        throw Terminated();
    m_cpu.esp = esp;
    return m_cpu.eax;
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

x86::reg32 text(Guest& guest, std::uint32_t id)
{
    return guest.call(kText, args({ id }));
}

std::int32_t textWidth(Guest& guest, x86::reg32 text, std::uint32_t font)
{
    return std::int32_t(guest.call(kTextWidth, args({ text, font })));
}

void messageBox(Guest& guest, std::uint32_t textId)
{
    guest.call(kMessageBox, args({ textId }));
}

void openDialog(Guest& guest, const Dialog& dialog)
{
    guest.call(kOpenDialog, args({ dialog.lineCount, dialog.lines, dialog.buttonCount, dialog.buttons },
                                 { dialog.focus, dialog.edit, dialog.editLength, dialog.editMode, dialog.callback }));
}

void openQuestion(Guest& guest, const Question& question)
{
    guest.call(kOpenQuestion, args({ question.lineCount, question.lines, question.buttonCount, question.buttons },
                                   { question.buttonHelp, question.focus, question.column, question.callback }));
}

x86::reg32 dialogEditText(Guest& guest)
{
    return guest.call(kDialogEditText);
}

std::uint32_t dialogFocus(Guest& guest)
{
    return guest.call(kDialogFocus);
}

bool X87::above(const x86::Float& a, const x86::Float& b)
{
    m_fpu.compare(a, b);
    return !m_fpu.status.c0 && !m_fpu.status.c3;
}

bool X87::below(const x86::Float& a, const x86::Float& b)
{
    m_fpu.compare(a, b);
    return m_fpu.status.c0;
}

std::int32_t X87::truncated(const x86::Float& value)
{
    m_fpu.push(value);
    m_guest.call(kTruncate);
    return m_fpu.toInteger<x86::sreg32>(m_fpu.pop());
}

bool ready(Guest& guest, Site& site)
{
    if (!allOn() || switchedOff(site.name))
    {
        SDL_Log("[FE] %s left to the generated code (NFS_FE_NATIVE)", site.name);
        site.ready = -1;
        return false;
    }
    for (std::size_t i = 0; i < site.stringCount; ++i)
    {
        const GuestString& string = site.strings[i];
        if (!guest.holds(string.address, string.text))
        {
            SDL_Log("[FE] %s left to the generated code: 0x%x is not \"%s\"", site.name, string.address,
                    string.text);
            site.ready = -1;
            return false;
        }
    }
    SDL_Log("[FE] %s runs native", site.name);
    site.ready = 1;
    return true;
}

}
}
