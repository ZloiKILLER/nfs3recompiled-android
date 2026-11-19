#ifndef NFS3HP_NATIVE_FRONTEND_RENDA_H
#define NFS3HP_NATIVE_FRONTEND_RENDA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_renda.cpp's typed functions (the race's sky and what stands with it: the fog's
 * states, the sky's rings, the sun, the sky's colour, the dome's grid and its flashes, the sky's
 * polygon list's ends, a quad's record, the sparks on the screen, a string's width in the race's
 * font), which other files call as C++ (guest::direct), and how the game's code calls each by
 * address.  Each calls the game's other functions by address
 * on `ts` and throws fe::Terminated when the game stopped in one, as a call by address does; its result
 * is what the game's function leaves in eax.  Some also leave registers they do not save as their
 * code does (curvePoints' and bandVertices' ebx, ecx and edx, domeVertices' ecx and edx): a caller
 * that reads such a register calls by address. */
namespace nfs3hp
{
namespace frontend
{
namespace renda
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x470a60: the fog's amount, as fogDensity was last given it.
u32 fogAmount(App* app, ThreadState& ts);
// The game's 0x470aa0: render state 0xf, the fog's colour, set and kept on a driver with a depth bias.
u32 fogColour(App* app, ThreadState& ts, u32 colour);
// The game's 0x470ad0: render state 0xe for the fog's amount, kept, on a driver with a depth bias.
u32 fogDensity(App* app, ThreadState& ts, u32 amount);
// The game's 0x470b20: the same, times `times`, the amount not kept; edx left the state's value.
u32 fogDensityTimes(App* app, ThreadState& ts, u32 amount, u32 times);
// The game's 0x470b70: the fog's colour and amount both.
u32 fogBoth(App* app, ThreadState& ts, u32 colour, u32 amount);
// The game's 0x470b90: the fog as a race starts: the amount unknown, no colour, state 0xe 0.
u32 fogReset(App* app, ThreadState& ts);
// The game's 0x476fb0: how wide the string at `text` is drawn in the race's font.
u32 stringWidth(App* app, ThreadState& ts, u32 text);
// The game's 0x4774b0: the current spark set's sparks put on the screen, those run out gone.
u32 sparksOnScreen(App* app, ThreadState& ts);
// The game's 0x478cb0: the polygon's record at `polygon` made a quad of the four vertices by value, their four
// colours and a flag the caller has put on the stack above the return address (0x94 bytes, ret 0x94).
u32 quadRecord(App* app, ThreadState& ts, u32 polygon);
// The game's 0x478dc0: the view's rectangle at 0x6fdc40, its right and bottom for its sizes.
u32 viewRectangle(App* app, ThreadState& ts);
// The game's 0x479080: the sky's polygon list started, a mark first.
u32 skyListStart(App* app, ThreadState& ts);
// The game's 0x4790d0: the list ended and kept for pass `pass`, a new one started.
u32 skyListKeep(App* app, ThreadState& ts, u32 pass);
// The game's 0x479100: the list ended with a mark, its two ends to `out` (esi).
u32 skyListEnd(App* app, ThreadState& ts, u32 out);
// The game's 0x479160: a band's three points from view `view` to `points`, raised by the float `rise`, the upper one
// `upperLength` long, its turn to `turn`.
u32 bandPoints(App* app, ThreadState& ts, u32 view, u32 rise, u32 upperLength, u32 points, u32 turn);
// The game's 0x4792e0: `count` points to `out`, on from the two at `in` and as far apart.
u32 curvePoints(App* app, ThreadState& ts, u32 count, u32 in, u32 out);
// The game's 0x479370: `count` points at `points` as vertices at `vertices` by view `view`'s camera, their clip codes
// to `codes`, in `colour`.
u32 bandVertices(App* app, ThreadState& ts, u32 count, u32 view, u32 points, u32 vertices, u32 codes,
                   u32 colour);
// The game's 0x479500: a ring's two curves of nine points from view `view`, at the float `height`, the upper
// `upperLength` on, the turn to `turn`.
u32 ringPoints(App* app, ThreadState& ts, u32 view, u32 height, u32 upperLength, u32 turn, u32 upperCurve,
                 u32 lowerCurve);
// The game's 0x479680: the ring of the sky's gradient from view `view`.
u32 skyGradientRing(App* app, ThreadState& ts, u32 view);
// The game's 0x479860: `colour` with the fog's share of its colour, on a driver without a depth bias.
u32 foggedColour(App* app, ThreadState& ts, u32 colour);
// The game's 0x4799d0: the ring of the fog's haze from view `view`.
u32 skyHazeRing(App* app, ThreadState& ts, u32 view);
// The game's 0x479b90: the ring of the clouds from view `view`.
u32 skyCloudRing(App* app, ThreadState& ts, u32 view);
// The game's 0x479ed0: the sun's quad for view `view`, when the track has one.
u32 sun(App* app, ThreadState& ts, u32 view);
// The game's 0x47a4c0: whether the sky goes flat for car `car`'s view, the fog's colour set either way.
u32 skyFlat(App* app, ThreadState& ts, u32 car);
// The game's 0x47adc0: the point at `point`'s height from its distance, down the sky's curve.
u32 pointHeight(App* app, ThreadState& ts, u32 point);
// The game's 0x47ae10: the dome's grid made as the track loads.
u32 cloudGrid(App* app, ThreadState& ts);
// The game's 0x47b350: the sky's flashes as a race starts.
u32 flashesReset(App* app, ThreadState& ts);
// The game's 0x47b390: the same, for another caller.
u32 flashesResetAgain(App* app, ThreadState& ts);
// The game's 0x47b3a0: the flash on the dome's grid, three by three about its cell.
u32 flashOnGrid(App* app, ThreadState& ts);
// The game's 0x47b4f0: `count` points at `points` as vertices at `vertex` by view `view`'s camera, their colours and
// corners from `corners` moved by the floats `moveU` and `moveV`.
u32 domeVertices(App* app, ThreadState& ts, u32 view, u32 count, u32 points, u32 corners, u32 vertex, u32 moveU,
                   u32 moveV);
// The game's 0x47b5c0: the sky's weather up to the race's tick: flashes, their thunder, the clouds' drift.
u32 skyWeather(App* app, ThreadState& ts);

}  // namespace renda
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
