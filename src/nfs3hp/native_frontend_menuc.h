#ifndef NFS3HP_NATIVE_FRONTEND_MENUC_H
#define NFS3HP_NATIVE_FRONTEND_MENUC_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_menuc.cpp's typed functions (the results tables after a race, the menus' art, the
 * title and the front end's ends), which other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  Each calls the game's
 * other functions by address on `cpu` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax, and a void one leaves eax
 * as it came. */
namespace nfs3hp
{
namespace frontend
{
namespace menuc
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x45cb50: the HUD editor's element, hidden: nothing to do (eax left as it came).
void hudElementHidden(App* app, CPU& cpu);
// The game's 0x460760: the buttons' pulse from its brightest again (eax left as it came).
void resetButtonPulse(App* app, CPU& cpu);
// The game's 0x4654c0: the two values kept in the slots at 0x678954; the first back.
reg32 setSliderComponents(App* app, CPU& cpu, reg32 first, reg32 second);
// The game's 0x465d60: no chrome, no glow (eax left as it came).
void clearMenuArtSlots(App* app, CPU& cpu);
// The game's 0x465d80: the chrome and the glow from the screen's art.
reg32 loadMenuArtSlots(App* app, CPU& cpu, reg32 art);
// The game's 0x46cce0: "sort by" and an arrow over a column's title (its box, then the arrow's shape).
reg32 drawColumnSortMark(App* app, CPU& cpu, reg32 topLeft, reg32 widthHeight, reg32 shape);
// The game's 0x46cdc0: the mark over the results' sorted column; edx left the column.
reg32 resultsSortMark(App* app, CPU& cpu, reg32 item);
// The game's 0x46cdf0: the column of a row under the pointer, for its popups: 1, 2 or 0.
reg32 resultsPopupColumn(App* app, CPU& cpu, reg32 item);
// The game's 0x46d180: the popup of a racer's lap times over the row under the pointer.
reg32 resultsLapTimesPopup(App* app, CPU& cpu, reg32 item);
// The game's 0x46d3c0: the results table drawn; edx left 0.
reg32 resultsTableDraw(App* app, CPU& cpu, reg32 item, reg32 pointing);
// The game's 0x46df90: the results as their screen opens: sorted by place, the player's row in view.
reg32 resultsTableShown(App* app, CPU& cpu, reg32 item);
// The game's 0x46e0b0: the results' every pass: whether they are shown.
reg32 resultsTablePass(App* app, CPU& cpu, reg32 item);
// The game's 0x46e0d0: the results' boxes as the table stands now.
reg32 resultsTableLayout(App* app, CPU& cpu, reg32 item);
// The game's 0x46e270: the results' keys: 2 when one was used (edx too, ebx where it got to).
reg32 resultsTableKey(App* app, CPU& cpu, reg32 item, reg32 menu, reg32 key);
// The game's 0x46e640: the results hidden: 0.
reg32 resultsTableHidden(App* app, CPU& cpu, reg32 item);
// The game's 0x46e660: the mark over the standings' sorted column; edx left the column.
reg32 standingsSortMark(App* app, CPU& cpu, reg32 item);
// The game's 0x46e690: the standings table drawn; edx left 0.
reg32 standingsTableDraw(App* app, CPU& cpu, reg32 item, reg32 pointing);
// The game's 0x46ef90: the standings as their screen opens.
reg32 standingsTableShown(App* app, CPU& cpu, reg32 item);
// The game's 0x46f0a0: the standings' every pass: whether they are shown.
reg32 standingsTablePass(App* app, CPU& cpu, reg32 item);
// The game's 0x46f0c0: the standings' boxes as the table stands now.
reg32 standingsTableLayout(App* app, CPU& cpu, reg32 item);
// The game's 0x46f200: the standings' keys.
reg32 standingsTableKey(App* app, CPU& cpu, reg32 item, reg32 menu, reg32 key);
// The game's 0x46f4c0: the standings hidden: 0.
reg32 standingsTableHidden(App* app, CPU& cpu, reg32 item);
// The game's 0x46f4e0: the mark over the rounds' sorted column; edx left the column.
reg32 roundsSortMark(App* app, CPU& cpu, reg32 item);
// The game's 0x46f510: the rounds table drawn; edx left 0.
reg32 roundsTableDraw(App* app, CPU& cpu, reg32 item, reg32 pointing);
// The game's 0x46fe30: the rounds as their screen opens.
reg32 roundsTableShown(App* app, CPU& cpu, reg32 item);
// The game's 0x46fee0: the rounds' every pass: whether they are shown.
reg32 roundsTablePass(App* app, CPU& cpu, reg32 item);
// The game's 0x46ff00: the rounds' boxes as the table stands now.
reg32 roundsTableLayout(App* app, CPU& cpu, reg32 item);
// The game's 0x470040: the rounds' keys.
reg32 roundsTableKey(App* app, CPU& cpu, reg32 item, reg32 menu, reg32 key);
// The game's 0x470340: the rounds hidden: 0.
reg32 roundsTableHidden(App* app, CPU& cpu, reg32 item);
// The game's 0x470360: the players side by side: where the table stands, the first time.
reg32 headToHeadShown(App* app, CPU& cpu, reg32 item);
// The game's 0x470390: the side by side table's pass: nothing to do (eax left as it came).
void headToHeadPass(App* app, CPU& cpu);
// The game's 0x4703a0: the players side by side drawn; edx left 0.
reg32 headToHeadDraw(App* app, CPU& cpu, reg32 item);
// The game's 0x472f20: the title's picture, in the language's.
reg32 titleDraw(App* app, CPU& cpu);
// The game's 0x472fc0: the title on the screen until a key, or until its time is up.
reg32 titleWait(App* app, CPU& cpu);
// The game's 0x473010: a comparison with the intro's flag, for no one (eax left as it came).
void skipIntroCompare(App* app, CPU& cpu);
// The game's 0x473020: into the menus: the input's mode kept, the sound and the screens taken up.
reg32 menusEnter(App* app, CPU& cpu);
// The game's 0x473080: out of the menus, all of it undone.
reg32 menusLeave(App* app, CPU& cpu);
// The game's 0x4732b0: the screens after a race: the menu loop on postgame, its answer.
reg32 postgameRun(App* app, CPU& cpu);
// The game's 0x477c00: the flag at 0x55d000 down: 0.
reg32 clearLoadFlag(App* app, CPU& cpu);

}  // namespace menuc
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
