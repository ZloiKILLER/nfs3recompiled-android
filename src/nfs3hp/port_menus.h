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
 * Each function here is a thunk under the game's name: the game's code calls them by name, and the
 * native whole screens (native_frontend_*.cpp) by address.  The thunk takes
 * the arguments from the registers and the stack and gives back the result
 * in eax, every other register as the caller had it. */
namespace nfs3hp
{
namespace port
{
namespace menus
{

/* The thunks, under the game's functions' names. */
x86::reg32 codelinkMatches(App* app, CPU& cpu, x86::Args& args);   // 0x4429c0
x86::reg32 assignHandlers(App* app, CPU& cpu, x86::Args& args);    // 0x442b10
x86::reg32 keywordIndex(App* app, CPU& cpu, x86::Args& args);      // 0x448f00
x86::reg32 storeString(App* app, CPU& cpu, x86::Args& args);       // 0x448f50
x86::reg32 widgetType(App* app, CPU& cpu, x86::Args& args);        // 0x448fd0
x86::reg32 widgetName(App* app, CPU& cpu, x86::Args& args);        // 0x449020
x86::reg32 initItem(App* app, CPU& cpu, x86::Args& args);          // 0x449060
x86::reg32 sectionType(App* app, CPU& cpu, x86::Args& args);       // 0x4490e0
x86::reg32 parseInt(App* app, CPU& cpu, x86::Args& args);          // 0x449130
x86::reg32 parseLong(App* app, CPU& cpu, x86::Args& args);         // 0x449170
x86::reg32 parseFloat(App* app, CPU& cpu, x86::Args& args);        // 0x4491b0
x86::reg32 setField(App* app, CPU& cpu, x86::Args& args);          // 0x449240
x86::reg32 readEntry(App* app, CPU& cpu, x86::Args& args);         // 0x4493b0
x86::reg32 loadMenus(App* app, CPU& cpu, x86::Args& args);         // 0x449510
x86::reg32 checkMenuFile(App* app, CPU& cpu, x86::Args& args);     // 0x4498a0
x86::reg32 checkMenuFiles(App* app, CPU& cpu, x86::Args& args);    // 0x449960
x86::reg32 callOnLoad(App* app, CPU& cpu, x86::Args& args);        // 0x449990
x86::reg32 appendBackslash(App* app, CPU& cpu, x86::Args& args);   // 0x4529a0
reg32 pathBackslashed(App* app, reg32 path);  // 0x4529a0's core: the path at `path` ended with a '\'; 0

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address.  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its thunk hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * thunk hands it, so the core's frame lies where the game's does; by address the call makes it. */

// The game's 0x4429c0: whether the codelink at `codelink` names the name at `name` (`ebx` handed on); esp the frame base.
Result codelinkMatched(App* app, CPU& cpu, reg32 esp, reg32 codelink, reg32 name, reg32 ebx);
// The game's 0x449510: the screen named at `name` loaded, every screen let go first with `reset`; in.ebx and in.ecx
// handed on as the caller's (in.eax and in.edx unread); esp the frame base.
Result menusLoaded(App* app, CPU& cpu, reg32 esp, reg32 name, reg32 reset, const Args& in);
// The game's 0x4529a0: the path at `path` ended with a backslash (put there when it has none); 0.
reg32 backslashEnded(App* app, reg32 path);

}  // namespace menus
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
