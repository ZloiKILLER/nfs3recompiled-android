#ifndef NFS3HP_NATIVE_FRONTEND_RENDA_H
#define NFS3HP_NATIVE_FRONTEND_RENDA_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_renda.cpp's typed functions (the race's sky and what stands with it: the fog's
 * states, the sky's rings, the sun, the sky's colour, the dome's grid and its flashes, the sky's
 * polygon list's ends, a quad's record, the sparks on the screen, a string's width in the race's
 * font), which other files call as C++ (guest::direct), and how the game's code calls each by
 * address.  Each calls the game's other functions by address
 * on `cpu` and throws fe::Terminated when the game stopped in one, as a call by address does; its result
 * is what the game's function leaves in eax.  Some also leave registers they do not save as their
 * code does (curvePoints' and bandVertices' ebx, ecx and edx, domeVertices' ecx and edx): a caller
 * that reads such a register calls by address. */
namespace nfs3hp
{
namespace frontend
{
namespace renda
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x470a60: the fog's amount, as fogDensity was last given it.
reg32 fogAmount(App* app, CPU& cpu);
// The game's 0x470aa0: render state 0xf, the fog's colour, set and kept on a driver with a depth bias.
reg32 fogColour(App* app, CPU& cpu, reg32 colour);
// The game's 0x470ad0: render state 0xe for the fog's amount, kept, on a driver with a depth bias.
reg32 fogDensity(App* app, CPU& cpu, reg32 amount);
// The game's 0x470b20: the same, times `times`, the amount not kept; edx left the state's value.
reg32 fogDensityTimes(App* app, CPU& cpu, reg32 amount, reg32 times);
// The game's 0x470b70: the fog's colour and amount both.
reg32 fogBoth(App* app, CPU& cpu, reg32 colour, reg32 amount);
// The game's 0x470b90: the fog as a race starts: the amount unknown, no colour, state 0xe 0.
reg32 fogReset(App* app, CPU& cpu);
// The game's 0x476fb0: how wide the string at `text` is drawn in the race's font.
reg32 stringWidth(App* app, CPU& cpu, reg32 text);
// The game's 0x4774b0: the current spark set's sparks put on the screen, those run out gone.
reg32 sparksOnScreen(App* app, CPU& cpu);
// The game's 0x478cb0: the polygon's record at `polygon` made a quad of the four vertices by value, their four
// colours and a flag the caller has put on the stack above the return address (0x94 bytes, ret 0x94).
reg32 quadRecord(App* app, CPU& cpu, reg32 polygon);
// The game's 0x478dc0: the view's rectangle at 0x6fdc40, its right and bottom for its sizes.
reg32 viewRectangle(App* app, CPU& cpu);
// The game's 0x479080: the sky's polygon list started, a mark first.
reg32 skyListStart(App* app, CPU& cpu);
// The game's 0x4790d0: the list ended and kept for pass `pass`, a new one started.
reg32 skyListKeep(App* app, CPU& cpu, reg32 pass);
// The game's 0x479100: the list ended with a mark, its two ends to `out` (esi).
reg32 skyListEnd(App* app, CPU& cpu, reg32 out);
// The game's 0x479160: a band's three points from view `view` to `points`, raised by the float `rise`, the upper one
// `upperLength` long, its turn to `turn`.
reg32 bandPoints(App* app, CPU& cpu, reg32 view, reg32 rise, reg32 upperLength, reg32 points, reg32 turn);
// The game's 0x4792e0: `count` points to `out`, on from the two at `in` and as far apart.
reg32 curvePoints(App* app, CPU& cpu, reg32 count, reg32 in, reg32 out);
// The game's 0x479370: `count` points at `points` as vertices at `vertices` by view `view`'s camera, their clip codes
// to `codes`, in `colour`.
reg32 bandVertices(App* app, CPU& cpu, reg32 count, reg32 view, reg32 points, reg32 vertices, reg32 codes,
                   reg32 colour);
// The game's 0x479500: a ring's two curves of nine points from view `view`, at the float `height`, the upper
// `upperLength` on, the turn to `turn`.
reg32 ringPoints(App* app, CPU& cpu, reg32 view, reg32 height, reg32 upperLength, reg32 turn, reg32 upperCurve,
                 reg32 lowerCurve);
// The game's 0x479680: the ring of the sky's gradient from view `view`.
reg32 skyGradientRing(App* app, CPU& cpu, reg32 view);
// The game's 0x479860: `colour` with the fog's share of its colour, on a driver without a depth bias.
reg32 foggedColour(App* app, CPU& cpu, reg32 colour);
// The game's 0x4799d0: the ring of the fog's haze from view `view`.
reg32 skyHazeRing(App* app, CPU& cpu, reg32 view);
// The game's 0x479b90: the ring of the clouds from view `view`.
reg32 skyCloudRing(App* app, CPU& cpu, reg32 view);
// The game's 0x479ed0: the sun's quad for view `view`, when the track has one.
reg32 sun(App* app, CPU& cpu, reg32 view);
// The game's 0x47a4c0: whether the sky goes flat for car `car`'s view, the fog's colour set either way.
reg32 skyFlat(App* app, CPU& cpu, reg32 car);
// The game's 0x47adc0: the point at `point`'s height from its distance, down the sky's curve.
reg32 pointHeight(App* app, CPU& cpu, reg32 point);
// The game's 0x47ae10: the dome's grid made as the track loads.
reg32 cloudGrid(App* app, CPU& cpu);
// The game's 0x47b350: the sky's flashes as a race starts.
reg32 flashesReset(App* app, CPU& cpu);
// The game's 0x47b390: the same, for another caller.
reg32 flashesResetAgain(App* app, CPU& cpu);
// The game's 0x47b3a0: the flash on the dome's grid, three by three about its cell.
reg32 flashOnGrid(App* app, CPU& cpu);
// The game's 0x47b4f0: `count` points at `points` as vertices at `vertex` by view `view`'s camera, their colours and
// corners from `corners` moved by the floats `moveU` and `moveV`.
reg32 domeVertices(App* app, CPU& cpu, reg32 view, reg32 count, reg32 points, reg32 corners, reg32 vertex, reg32 moveU,
                   reg32 moveV);
// The game's 0x47b5c0: the sky's weather up to the race's tick: flashes, their thunder, the clouds' drift.
reg32 skyWeather(App* app, CPU& cpu);

}  // namespace renda
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
