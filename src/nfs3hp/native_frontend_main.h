#ifndef NFS3HP_NATIVE_FRONTEND_MAIN_H
#define NFS3HP_NATIVE_FRONTEND_MAIN_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_main.cpp's typed functions (the main menu in its three forms: main, mainsplt,
 * mainmult), which other files call as C++ (guest::direct), and how the game's code calls each by
 * address.  Each calls the game's other functions by address
 * on `ts` and throws fe::Terminated when the game stopped in one, as a call by address does; its result is
 * what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace mainmenu
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x45d050: main as it opens: each item fitted to the kind of race.
u32 mainEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x45d2f0: main as it closes: nothing to undo.
u32 mainExit(App* app, ThreadState& ts);
// The game's 0x45cff0: main's every pass: the attract-mode movie, the opponents' list.
u32 mainFrame(App* app, ThreadState& ts, u32 menu);
// The game's 0x45cef0: player one's name button: a dialog to type it in.
u32 playerName(App* app, ThreadState& ts);
// The game's 0x45cdd0: player one's name dialog answered.
u32 playerNameAnswer(App* app, ThreadState& ts, u32 answer, u32 close);
// The game's 0x45d360: mainsplt as it opens: two players, the second one's name too.
u32 splitEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x45d620: mainsplt as it closes: the port's rows put back.
u32 splitExit(App* app, ThreadState& ts);
// The game's 0x45d300: mainsplt's every pass, main's instruction for instruction.
u32 splitFrame(App* app, ThreadState& ts, u32 menu);
// The game's 0x45cf70: player two's name button: a dialog to type it in.
u32 player2Name(App* app, ThreadState& ts);
// The game's 0x45ce60: player two's name dialog answered.
u32 player2NameAnswer(App* app, ThreadState& ts, u32 answer, u32 close);
// The game's 0x440470: mainmult as it opens: the race list's lines and the screen.
u32 multiEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x4405b0: mainmult as it closes: nothing to undo.
u32 multiExit(App* app, ThreadState& ts);
// The game's 0x4403f0: mainmult's every pass: the race type followed or applied.
u32 multiFrame(App* app, ThreadState& ts, u32 menu);
// The game's 0x440160: the race list's line applied: race type, variant, items.
u32 applyRaceList(App* app, ThreadState& ts, u32 menu);
// The game's 0x440390: the race list's line for the race type the game has.
u32 followRaceType(App* app, ThreadState& ts);
// The game's 0x4400d0: mainmult's Back: a question first when connected.
u32 multiBack(App* app, ThreadState& ts);
// The game's 0x43fcb0: the question before leaving answered.
u32 multiLeaveAnswer(App* app, ThreadState& ts, u32 answer, u32 close);

}  // namespace mainmenu
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
