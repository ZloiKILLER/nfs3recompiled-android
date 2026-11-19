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
nfs::u32 codelinkMatches(App* app, ThreadState& ts, nfs::Args& args);   // 0x4429c0
nfs::u32 assignHandlers(App* app, ThreadState& ts, nfs::Args& args);    // 0x442b10
nfs::u32 keywordIndex(App* app, ThreadState& ts, nfs::Args& args);      // 0x448f00
nfs::u32 storeString(App* app, ThreadState& ts, nfs::Args& args);       // 0x448f50
nfs::u32 widgetType(App* app, ThreadState& ts, nfs::Args& args);        // 0x448fd0
nfs::u32 widgetName(App* app, ThreadState& ts, nfs::Args& args);        // 0x449020
nfs::u32 initItem(App* app, ThreadState& ts, nfs::Args& args);          // 0x449060
nfs::u32 sectionType(App* app, ThreadState& ts, nfs::Args& args);       // 0x4490e0
nfs::u32 parseInt(App* app, ThreadState& ts, nfs::Args& args);          // 0x449130
nfs::u32 parseLong(App* app, ThreadState& ts, nfs::Args& args);         // 0x449170
nfs::u32 parseFloat(App* app, ThreadState& ts, nfs::Args& args);        // 0x4491b0
nfs::u32 setField(App* app, ThreadState& ts, nfs::Args& args);          // 0x449240
nfs::u32 readEntry(App* app, ThreadState& ts, nfs::Args& args);         // 0x4493b0
nfs::u32 loadMenus(App* app, ThreadState& ts, nfs::Args& args);         // 0x449510
nfs::u32 checkMenuFile(App* app, ThreadState& ts, nfs::Args& args);     // 0x4498a0
nfs::u32 checkMenuFiles(App* app, ThreadState& ts, nfs::Args& args);    // 0x449960
nfs::u32 callOnLoad(App* app, ThreadState& ts, nfs::Args& args);        // 0x449990
nfs::u32 appendBackslash(App* app, ThreadState& ts, nfs::Args& args);   // 0x4529a0
u32 pathBackslashed(App* app, u32 path);  // 0x4529a0's core: the path at `path` ended with a '\'; 0

/* The typed cores the menus call as C++ (guest::direct), and how the game's code calls each by address.  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its thunk hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes ts.sp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * thunk hands it, so the core's frame lies where the game's does; by address the call makes it. */

// The game's 0x4429c0: whether the codelink at `codelink` names the name at `name` (`ebx` handed on); esp the frame base.
Result codelinkMatched(App* app, ThreadState& ts, u32 esp, u32 codelink, u32 name, u32 ebx);
// The game's 0x449510: the screen named at `name` loaded, every screen let go first with `reset`; in.ebx and in.ecx
// handed on as the caller's (in.eax and in.edx unread); esp the frame base.
Result menusLoaded(App* app, ThreadState& ts, u32 esp, u32 name, u32 reset, const Args& in);
// The game's 0x4529a0: the path at `path` ended with a backslash (put there when it has none); 0.
u32 backslashEnded(App* app, u32 path);

}  // namespace menus
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
