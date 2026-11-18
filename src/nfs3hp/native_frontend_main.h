#ifndef NFS3HP_NATIVE_FRONTEND_MAIN_H
#define NFS3HP_NATIVE_FRONTEND_MAIN_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_main.cpp's typed functions (the main menu in its three forms: main, mainsplt,
 * mainmult), which other files call as C++ (guest::direct), and how the game's code calls each by
 * address.  Each calls the game's other functions by address
 * on `cpu` and throws fe::Terminated when the game stopped in one, as a call by address does; its result is
 * what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace mainmenu
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x45d050: main as it opens: each item fitted to the kind of race.
reg32 mainEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x45d2f0: main as it closes: nothing to undo.
reg32 mainExit(App* app, CPU& cpu);
// The game's 0x45cff0: main's every pass: the attract-mode movie, the opponents' list.
reg32 mainFrame(App* app, CPU& cpu, reg32 menu);
// The game's 0x45cef0: player one's name button: a dialog to type it in.
reg32 playerName(App* app, CPU& cpu);
// The game's 0x45cdd0: player one's name dialog answered.
reg32 playerNameAnswer(App* app, CPU& cpu, reg32 answer, reg32 close);
// The game's 0x45d360: mainsplt as it opens: two players, the second one's name too.
reg32 splitEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x45d620: mainsplt as it closes: the port's rows put back.
reg32 splitExit(App* app, CPU& cpu);
// The game's 0x45d300: mainsplt's every pass, main's instruction for instruction.
reg32 splitFrame(App* app, CPU& cpu, reg32 menu);
// The game's 0x45cf70: player two's name button: a dialog to type it in.
reg32 player2Name(App* app, CPU& cpu);
// The game's 0x45ce60: player two's name dialog answered.
reg32 player2NameAnswer(App* app, CPU& cpu, reg32 answer, reg32 close);
// The game's 0x440470: mainmult as it opens: the race list's lines and the screen.
reg32 multiEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x4405b0: mainmult as it closes: nothing to undo.
reg32 multiExit(App* app, CPU& cpu);
// The game's 0x4403f0: mainmult's every pass: the race type followed or applied.
reg32 multiFrame(App* app, CPU& cpu, reg32 menu);
// The game's 0x440160: the race list's line applied: race type, variant, items.
reg32 applyRaceList(App* app, CPU& cpu, reg32 menu);
// The game's 0x440390: the race list's line for the race type the game has.
reg32 followRaceType(App* app, CPU& cpu);
// The game's 0x4400d0: mainmult's Back: a question first when connected.
reg32 multiBack(App* app, CPU& cpu);
// The game's 0x43fcb0: the question before leaving answered.
reg32 multiLeaveAnswer(App* app, CPU& cpu, reg32 answer, reg32 close);

}  // namespace mainmenu
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
