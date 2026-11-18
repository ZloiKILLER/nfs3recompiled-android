#ifndef NFS3HP_NATIVE_FRONTEND_PAUSE_H
#define NFS3HP_NATIVE_FRONTEND_PAUSE_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_pause.cpp's typed functions (a race's pause screens: pause, psoption, pscntl1,
 * psgraph, pshud, pshud1p, pshud2p, pscamera, quitto; the saves pause offers; the race's views'
 * rectangles), which other files call as C++ (guest::direct), and how the game's code calls each
 * by address.  Each calls the game's other functions by
 * address on `cpu` and throws fe::Terminated when the game stopped in one, as a call by address does;
 * its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace pause
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x446510: a series' game by this name stands in the saves' folder.
reg32 seriesSaveExists(App* app, CPU& cpu, reg32 name);
// The game's 0x446550: the series' game written under this name; 1 when it went out.
reg32 writeSeriesSave(App* app, CPU& cpu, reg32 name);
// The game's 0x446740: a ghost by this name stands.
reg32 ghostExists(App* app, CPU& cpu, reg32 name);
// The game's 0x4468c0: the tournament's game written under this name; 1 when it went out.
reg32 writeTournamentSave(App* app, CPU& cpu, reg32 name);
// The game's 0x446950: a tournament's game by this name stands.
reg32 tournamentSaveExists(App* app, CPU& cpu, reg32 name);
// The game's 0x446a90: a replay by this name stands.
reg32 replayExists(App* app, CPU& cpu, reg32 name);
// The game's 0x478010: a save about to be written is one, its cars' names filled in; 1 when it held.
reg32 checkSaveable(App* app, CPU& cpu, reg32 save);
// The game's 0x447800: whether to write over the file answered: Yes writes, No and Escape ask again.
reg32 overwriteAnswered(App* app, CPU& cpu, reg32 answer, reg32 keepOpen);
// The game's 0x4478c0: the name dialog answered: a name that stands asks first, a new one is written.
reg32 saveNameAnswered(App* app, CPU& cpu, reg32 answer, reg32 close);
// The game's 0x447a30: the name to save under asked for, of this kind of save.
reg32 askSaveName(App* app, CPU& cpu, reg32 kind);
// The game's 0x44c2d0: the six spare items hidden; the last one found, or 0.
reg32 hideSpare(App* app, CPU& cpu, reg32 menu);
// The game's 0x44c360: a cop's display: a racer's instruments hidden.
reg32 hideForCop(App* app, CPU& cpu, reg32 menu);
// The game's 0x44c4e0: a racer's display: a cop's items hidden.
reg32 hideForRacer(App* app, CPU& cpu, reg32 menu);
// The game's 0x44c610: "are you sure?" about leaving answered: Yes hands the menu loop where to.
reg32 leaveAnswered(App* app, CPU& cpu, reg32 answer, reg32 close);
// The game's 0x44c670: Restart's question answered: Yes restarts.
reg32 restartAnswered(App* app, CPU& cpu, reg32 answer, reg32 close);
// The game's 0x44c6d0: "are you sure?" asked for a choice of the quit box, No first.
reg32 askLeave(App* app, CPU& cpu, reg32 question, reg32 leaveTo);
// The game's 0x44c730: the quit box answered: its button's question, or Cancel.
reg32 quitChoiceAnswered(App* app, CPU& cpu, reg32 answer, reg32 close);
// The game's 0x44c7b0: Quit clicked on pause: where to, by the kind of race.
reg32 onQuitClicked(App* app, CPU& cpu);
// The game's 0x44ca10: Restart clicked.
reg32 onRestartClicked(App* app, CPU& cpu);
// The game's 0x44ca80: Save clicked: the name asked for, of the race type's kind of save.
reg32 onSaveClicked(App* app, CPU& cpu);
// The game's 0x44cab0: pause as it opens.
reg32 onPauseEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x44cf70: the effects' intensity moved.
reg32 intensityMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x44cfa0: pshud's map size moved, a cop's.
reg32 mapCopMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x44cfe0: pshud's map size moved, a racer's.
reg32 mapRacerMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x44d020: pshud1p's map size moved, a cop's.
reg32 mapFirstCopMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x44d060: pshud1p's map size moved, a racer's.
reg32 mapFirstRacerMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x44d0a0: pshud2p's map size moved, a cop's.
reg32 mapSecondCopMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x44d0e0: pshud2p's map size moved, a racer's.
reg32 mapSecondRacerMoved(App* app, CPU& cpu, reg32 item);
// The game's 0x44d120: psoption and psoptn2 as they open.
reg32 onOptionsEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x44d170: Cancel's key on pscntl1: Enter on it is what cancels.
reg32 cancelKey(App* app, CPU& cpu, reg32 item, reg32 menu, std::uint16_t key);
// The game's 0x44d190: pscntl1 as it opens: the controls kept for Cancel.
reg32 onControlsEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x44d220: pscntl1 as it closes: the old controls back on Cancel, else the new taken up.
reg32 onControlsExit(App* app, CPU& cpu);
// The game's 0x44d270: the screen's shape clicked: the views laid out again.
reg32 onWidescreenClicked(App* app, CPU& cpu);
// The game's 0x44d2a0: psgraph as it opens: the settings kept, what the card cannot do greyed out.
reg32 onGraphicsEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x44d3e0: psgraph on every pass: the projected textures' button by what is on.
reg32 fitProjected(App* app, CPU& cpu, reg32 menu);
// The game's 0x44d420: psgraph as it closes: Cancel puts the settings back.
reg32 onGraphicsExit(App* app, CPU& cpu, reg32 menu);
// The game's 0x44d4c0: pshud as it opens.
reg32 onHudEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x44d670: pshud as it closes.
reg32 hudExit(App* app, CPU& cpu, reg32 menu);
// The game's 0x44d7a0: pshud1p as it opens: the first player's display.
reg32 onHud1Enter(App* app, CPU& cpu, reg32 menu);
// The game's 0x44d9a0: pshud1p as it closes.
reg32 hud1Exit(App* app, CPU& cpu, reg32 menu);
// The game's 0x44dad0: pshud2p as it opens: the second player's display.
reg32 onHud2Enter(App* app, CPU& cpu, reg32 menu);
// The game's 0x44dcd0: pshud2p as it closes.
reg32 hud2Exit(App* app, CPU& cpu, reg32 menu);
// The game's 0x45d770: the question about two keys on one camera answered.
reg32 cameraAnswered(App* app, CPU& cpu, reg32 answer, reg32 close);
// The game's 0x45d840: pscamera's way out: it asks if two keys hold one camera.
reg32 cameraKey(App* app, CPU& cpu);
// The game's 0x45d920: pscamera as it opens: the cameras kept for Cancel.
reg32 onCameraEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x45d9a0: at least one camera to look from.
reg32 ensureSomeCamera(App* app, CPU& cpu);
// The game's 0x45d9e0: pscamera as it closes: Cancel puts them back, else the race takes them up.
reg32 onCameraExit(App* app, CPU& cpu, reg32 menu);
// The game's 0x4b67b0: quitto's "are you sure?" answered: Yes hands the menu loop where to.
reg32 quitToAnswered(App* app, CPU& cpu, reg32 answer, reg32 close);
// The game's 0x4b6810: "are you sure?" for a button of quitto.
reg32 quitToAsk(App* app, CPU& cpu, reg32 question, reg32 leaveTo);
// The game's 0x4b6870: quitto's replay button clicked.
reg32 quitToReplay(App* app, CPU& cpu);
// The game's 0x4b6890: quitto's post-race button clicked.
reg32 quitToPost(App* app, CPU& cpu);
// The game's 0x4b68d0: quitto's main menu button clicked.
reg32 quitToMain(App* app, CPU& cpu);
// The game's 0x4b6910: quitto's system button clicked.
reg32 quitToSystem(App* app, CPU& cpu);
// The game's 0x4b6930: quitto as it opens: a network race has the one way back, any other the two.
reg32 onQuitToEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x4be3d0: the full rectangle and the view's into the corners of the quads over the screen.
reg32 fillViewQuads(App* app, CPU& cpu);
// The game's 0x4beb00: a rectangle's first half set.
reg32 setRectFirst(App* app, CPU& cpu, reg32 rect, reg32 a, reg32 b, reg32 c, reg32 d);
// The game's 0x4beb20: a rectangle read, each of its four through a pointer.
reg32 readRect(App* app, CPU& cpu, reg32 rect, reg32 a, reg32 b, reg32 c, reg32 d);
// The game's 0x4beb40: a rectangle's second half set.
reg32 setRectSecond(App* app, CPU& cpu, reg32 rect, reg32 a, reg32 b, reg32 c, reg32 d);

}  // namespace pause
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
