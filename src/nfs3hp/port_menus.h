#ifndef NFS3HP_PORT_MENUS_H
#define NFS3HP_PORT_MENUS_H

#include "native_guest.h"
#include "port.h"

/* The menu files and the screens they make, rewritten (port.h says how;
 * port_menus.cpp): FeData\Menus\<name>.mnu read into the table of screens and
 * the table of items, the keys of its sections looked up in the parser's
 * tables and stored into the items' fields by their kinds, a screen's
 * handlers found by its name, and the item lookup's codelink match; and the
 * path function they build the file's name with.
 *
 * disassemble_nfs3hp.py binds each address to the adapter of that name here
 * (PORT_MENUS_NATIVES): the generated code calls them by name, and the
 * native whole screens (native_frontend_*.cpp) by address.  The adapter takes
 * the arguments from the registers and the stack and gives back the result
 * in eax, every other register as the caller had it. */
namespace nfs3hp
{
namespace port
{
namespace menus
{

/* The adapters, under the game's functions' names. */
void codelinkMatches(App* app, CPU& cpu);   // 0x4429c0
void assignHandlers(App* app, CPU& cpu);    // 0x442b10
void keywordIndex(App* app, CPU& cpu);      // 0x448f00
void storeString(App* app, CPU& cpu);       // 0x448f50
void widgetType(App* app, CPU& cpu);        // 0x448fd0
void widgetName(App* app, CPU& cpu);        // 0x449020
void initItem(App* app, CPU& cpu);          // 0x449060
void sectionType(App* app, CPU& cpu);       // 0x4490e0
void parseInt(App* app, CPU& cpu);          // 0x449130
void parseLong(App* app, CPU& cpu);         // 0x449170
void parseFloat(App* app, CPU& cpu);        // 0x4491b0
void setField(App* app, CPU& cpu);          // 0x449240
void readEntry(App* app, CPU& cpu);         // 0x4493b0
void loadMenus(App* app, CPU& cpu);         // 0x449510
void checkMenuFile(App* app, CPU& cpu);     // 0x4498a0
void checkMenuFiles(App* app, CPU& cpu);    // 0x449960
void callOnLoad(App* app, CPU& cpu);        // 0x449990
void appendBackslash(App* app, CPU& cpu);   // 0x4529a0

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address
 * (ByAddress below, for the checks' stand-ins).  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its adapter hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * adapter hands it, so the core's frame lies where the game's does; by address the call makes it. */

// The game's 0x4429c0: whether the codelink at `codelink` names the name at `name` (`ebx` handed on); esp the frame base.
Result codelinkMatched(App* app, CPU& cpu, reg32 esp, reg32 codelink, reg32 name, reg32 ebx);
// The game's 0x449510: the screen named at `name` loaded, every screen let go first with `reset`; in.ebx and in.ecx
// handed on as the caller's (in.eax and in.edx unread); esp the frame base.
Result menusLoaded(App* app, CPU& cpu, reg32 esp, reg32 name, reg32 reset, const Args& in);

}  // namespace menus
}  // namespace port

namespace guest
{

// The codelink in eax, the name in edx, `ebx` in ebx; the answer in eax.
template <>
struct ByAddress<port::menus::codelinkMatched>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 codelink,
                             x86::reg32 name, x86::reg32 ebx)
    {
        return watcomEax(app, cpu, Address, {codelink, name, ebx});
    }
};

// The name in eax, `reset` in edx, in.ebx and in.ecx in ebx and ecx; eax back.
template <>
struct ByAddress<port::menus::menusLoaded>
{
    template <x86::reg32 Address>
    static port::Result call(win32::WinApplication* app, x86::CPU& cpu, x86::reg32, x86::reg32 name,
                             x86::reg32 reset, const port::Args& in)
    {
        return watcomEax(app, cpu, Address, {name, reset, in.ebx, in.ecx});
    }
};

}  // namespace guest
}  // namespace nfs3hp

#endif
