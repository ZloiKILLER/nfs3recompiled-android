#ifndef NFS3HP_NATIVE_FRONTEND_PAUSE_H
#define NFS3HP_NATIVE_FRONTEND_PAUSE_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_pause.cpp's typed functions (a race's pause screens: pause, psoption, pscntl1,
 * psgraph, pshud, pshud1p, pshud2p, pscamera, quitto; the saves pause offers; the race's views'
 * rectangles), which other files call as C++ (guest::direct), and how the game's code calls each
 * by address.  Each calls the game's other functions by
 * address on `ts` and throws fe::Terminated when the game stopped in one, as a call by address does;
 * its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace pause
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x446510: a series' game by this name stands in the saves' folder.
u32 seriesSaveExists(App* app, ThreadState& ts, u32 name);
// The game's 0x446550: the series' game written under this name; 1 when it went out.
u32 writeSeriesSave(App* app, ThreadState& ts, u32 name);
// The game's 0x446740: a ghost by this name stands.
u32 ghostExists(App* app, ThreadState& ts, u32 name);
// The game's 0x4468c0: the tournament's game written under this name; 1 when it went out.
u32 writeTournamentSave(App* app, ThreadState& ts, u32 name);
// The game's 0x446950: a tournament's game by this name stands.
u32 tournamentSaveExists(App* app, ThreadState& ts, u32 name);
// The game's 0x446a90: a replay by this name stands.
u32 replayExists(App* app, ThreadState& ts, u32 name);
// The game's 0x478010: a save about to be written is one, its cars' names filled in; 1 when it held.
u32 checkSaveable(App* app, ThreadState& ts, u32 save);
// The game's 0x447800: whether to write over the file answered: Yes writes, No and Escape ask again.
u32 overwriteAnswered(App* app, ThreadState& ts, u32 answer, u32 keepOpen);
// The game's 0x4478c0: the name dialog answered: a name that stands asks first, a new one is written.
u32 saveNameAnswered(App* app, ThreadState& ts, u32 answer, u32 close);
// The game's 0x447a30: the name to save under asked for, of this kind of save.
u32 askSaveName(App* app, ThreadState& ts, u32 kind);
// The game's 0x44c2d0: the six spare items hidden; the last one found, or 0.
u32 hideSpare(App* app, ThreadState& ts, u32 menu);
// The game's 0x44c360: a cop's display: a racer's instruments hidden.
u32 hideForCop(App* app, ThreadState& ts, u32 menu);
// The game's 0x44c4e0: a racer's display: a cop's items hidden.
u32 hideForRacer(App* app, ThreadState& ts, u32 menu);
// The game's 0x44c610: "are you sure?" about leaving answered: Yes hands the menu loop where to.
u32 leaveAnswered(App* app, ThreadState& ts, u32 answer, u32 close);
// The game's 0x44c670: Restart's question answered: Yes restarts.
u32 restartAnswered(App* app, ThreadState& ts, u32 answer, u32 close);
// The game's 0x44c6d0: "are you sure?" asked for a choice of the quit box, No first.
u32 askLeave(App* app, ThreadState& ts, u32 question, u32 leaveTo);
// The game's 0x44c730: the quit box answered: its button's question, or Cancel.
u32 quitChoiceAnswered(App* app, ThreadState& ts, u32 answer, u32 close);
// The game's 0x44c7b0: Quit clicked on pause: where to, by the kind of race.
u32 onQuitClicked(App* app, ThreadState& ts);
// The game's 0x44ca10: Restart clicked.
u32 onRestartClicked(App* app, ThreadState& ts);
// The game's 0x44ca80: Save clicked: the name asked for, of the race type's kind of save.
u32 onSaveClicked(App* app, ThreadState& ts);
// The game's 0x44cab0: pause as it opens.
u32 onPauseEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x44cf70: the effects' intensity moved.
u32 intensityMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x44cfa0: pshud's map size moved, a cop's.
u32 mapCopMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x44cfe0: pshud's map size moved, a racer's.
u32 mapRacerMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x44d020: pshud1p's map size moved, a cop's.
u32 mapFirstCopMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x44d060: pshud1p's map size moved, a racer's.
u32 mapFirstRacerMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x44d0a0: pshud2p's map size moved, a cop's.
u32 mapSecondCopMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x44d0e0: pshud2p's map size moved, a racer's.
u32 mapSecondRacerMoved(App* app, ThreadState& ts, u32 item);
// The game's 0x44d120: psoption and psoptn2 as they open.
u32 onOptionsEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x44d170: Cancel's key on pscntl1: Enter on it is what cancels.
u32 cancelKey(App* app, ThreadState& ts, u32 item, u32 menu, std::uint16_t key);
// The game's 0x44d190: pscntl1 as it opens: the controls kept for Cancel.
u32 onControlsEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x44d220: pscntl1 as it closes: the old controls back on Cancel, else the new taken up.
u32 onControlsExit(App* app, ThreadState& ts);
// The game's 0x44d270: the screen's shape clicked: the views laid out again.
u32 onWidescreenClicked(App* app, ThreadState& ts);
// The game's 0x44d2a0: psgraph as it opens: the settings kept, what the card cannot do greyed out.
u32 onGraphicsEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x44d3e0: psgraph on every pass: the projected textures' button by what is on.
u32 fitProjected(App* app, ThreadState& ts, u32 menu);
// The game's 0x44d420: psgraph as it closes: Cancel puts the settings back.
u32 onGraphicsExit(App* app, ThreadState& ts, u32 menu);
// The game's 0x44d4c0: pshud as it opens.
u32 onHudEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x44d670: pshud as it closes.
u32 hudExit(App* app, ThreadState& ts, u32 menu);
// The game's 0x44d7a0: pshud1p as it opens: the first player's display.
u32 onHud1Enter(App* app, ThreadState& ts, u32 menu);
// The game's 0x44d9a0: pshud1p as it closes.
u32 hud1Exit(App* app, ThreadState& ts, u32 menu);
// The game's 0x44dad0: pshud2p as it opens: the second player's display.
u32 onHud2Enter(App* app, ThreadState& ts, u32 menu);
// The game's 0x44dcd0: pshud2p as it closes.
u32 hud2Exit(App* app, ThreadState& ts, u32 menu);
// The game's 0x45d770: the question about two keys on one camera answered.
u32 cameraAnswered(App* app, ThreadState& ts, u32 answer, u32 close);
// The game's 0x45d840: pscamera's way out: it asks if two keys hold one camera.
u32 cameraKey(App* app, ThreadState& ts);
// The game's 0x45d920: pscamera as it opens: the cameras kept for Cancel.
u32 onCameraEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x45d9a0: at least one camera to look from.
u32 ensureSomeCamera(App* app, ThreadState& ts);
// The game's 0x45d9e0: pscamera as it closes: Cancel puts them back, else the race takes them up.
u32 onCameraExit(App* app, ThreadState& ts, u32 menu);
// The game's 0x4b67b0: quitto's "are you sure?" answered: Yes hands the menu loop where to.
u32 quitToAnswered(App* app, ThreadState& ts, u32 answer, u32 close);
// The game's 0x4b6810: "are you sure?" for a button of quitto.
u32 quitToAsk(App* app, ThreadState& ts, u32 question, u32 leaveTo);
// The game's 0x4b6870: quitto's replay button clicked.
u32 quitToReplay(App* app, ThreadState& ts);
// The game's 0x4b6890: quitto's post-race button clicked.
u32 quitToPost(App* app, ThreadState& ts);
// The game's 0x4b68d0: quitto's main menu button clicked.
u32 quitToMain(App* app, ThreadState& ts);
// The game's 0x4b6910: quitto's system button clicked.
u32 quitToSystem(App* app, ThreadState& ts);
// The game's 0x4b6930: quitto as it opens: a network race has the one way back, any other the two.
u32 onQuitToEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x4be3d0: the full rectangle and the view's into the corners of the quads over the screen.
u32 fillViewQuads(App* app, ThreadState& ts);
// The game's 0x4beb00: a rectangle's first half set.
u32 setRectFirst(App* app, ThreadState& ts, u32 rect, u32 a, u32 b, u32 c, u32 d);
// The game's 0x4beb20: a rectangle read, each of its four through a pointer.
u32 readRect(App* app, ThreadState& ts, u32 rect, u32 a, u32 b, u32 c, u32 d);
// The game's 0x4beb40: a rectangle's second half set.
u32 setRectSecond(App* app, ThreadState& ts, u32 rect, u32 a, u32 b, u32 c, u32 d);

}  // namespace pause
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
