#ifndef NFS3HP_NATIVE_RENDER_H
#define NFS3HP_NATIVE_RENDER_H

#include "native_guest.h"

/* native_render.cpp's typed functions that other files call as C++ (guest::direct), and how the
 * game's code calls each by address.  The file's other
 * functions other files call are still the code's registers line by line, with no typed core:
 * those stay calls by address. */
namespace nfs3hp
{
namespace render
{

// The game's 0x434870: the polygon lists' drawers chosen for the driver's abilities.
void choosePolygonDrawers(win32::WinApplication* app);

// The functions below are written as the game's code runs them, the registers and the frames line by line
// (their bodies are native_render.cpp's thunks); each is called as C++ here, its registers set from the
// arguments, `false` or nullopt when the game stopped in it.  Everything it reads is an argument or the game's
// memory but where its comment says a register is kept as it came (the caller's).

// The game's 0x41d620: the views' distances for View Distance; eax what it leaves.
std::optional<x86::reg32> viewDistancesSet(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x472d10: the detail settings for the machine; eax what it leaves.
std::optional<x86::reg32> detailSettingsSet(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4bef50: screen mode `mode` opened from the driver's list.
bool screenModeSet(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 mode);
// The game's 0x4d0a10: the 2D picture at [0x56727c] made a texture.
bool pictureTextured(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4d41a0: the paletted picture `picture` as a texture into the record `record`.
bool paletteTextured(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 picture, x86::reg32 record);
// The game's 0x4de070: view `view`'s smoke, `count` particles, the list at `list` (esi).
bool smokeDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 count, x86::reg32 list);
// The game's 0x4de700: view `view`'s spray streaks, `count` of them, `liveCount` live, the list at `list` (esi).
bool streaksDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 count, x86::reg32 liveCount,
                  x86::reg32 list);
// The game's 0x434870 as the code runs it: the polygon lists' drawers chosen.
bool polygonDrawersSet(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x4b59c0: driver `driver` started, the window's message `message`.
bool driverStarted(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 driver, x86::reg32 message);
// The game's 0x4b7d30: the textures of the car `car`, its 3D shape `shape`.
bool carTexturesMade(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 shape);
// The game's 0x47a880: the sky of the pass `view`, the two words it makes at `to` (esi).
bool viewSkyMade(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 to);
// The game's 0x41d960: the views for a race.
bool raceViewsSet(win32::WinApplication* app, x86::CPU& cpu);
// The game's 0x494cb0: the loading screen, the picture `picture` shown.
bool loadingPictureShown(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 picture);
// The game's 0x4800a0: the dark band behind player `player`'s HUD in split screen; eax what it leaves.
std::optional<x86::reg32> hudBandBuilt(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 player);

// The game's 0x480910: the HUD rectangle of element `element` of view `view`, the view's size `width` by `height`.
bool hudRectPlaced(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 element, x86::reg32 width,
                   x86::reg32 height);
// The game's 0x480fc0, 0x4812e0, 0x481aa0 and 0x481c50: view `view`'s picture, dial, map panel and table of the HUD
// placed; eax what each leaves.
std::optional<x86::reg32> hudPicturePlaced(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view);
std::optional<x86::reg32> hudDialPlaced(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view);
std::optional<x86::reg32> hudPanelPlaced(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view);
std::optional<x86::reg32> hudTablePlaced(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view);
// The game's 0x48c0e0: a meter of segments at (`x`, `y`) `width` by `height`, the first `lit` of them lit, `corner`
// the colour of two corners; eax what it leaves (`view` is what the code has in eax, which it does not read).
std::optional<x86::reg32> meterDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 x,
                                     x86::reg32 y, x86::reg32 width, x86::reg32 height, x86::reg32 lit,
                                     x86::reg32 corner);
// The game's 0x48c7c0: the start lights' panel, `first`..`fourth` the quads of its lamps, `lit` the stack's word;
// eax what it leaves.
std::optional<x86::reg32> startLightsDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 first,
                                           x86::reg32 second, x86::reg32 third, x86::reg32 fourth, x86::reg32 lit);

// The game's 0x49d800: the glare's two colours `first` and `second` set, darkened by the sun's glare.
void glareColoursSet(win32::WinApplication* app, x86::reg32 first, x86::reg32 second);
// The game's 0x4b56e0: the transform buffer's size set to `size` and the buffer allocated; the allocator's eax,
// nullopt when the game stopped.
std::optional<x86::reg32> transformBufferAllocated(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 size);
// The game's 0x4b97b0: car `car`'s colour `colour` at night; the colour.
std::optional<x86::reg32> nightColourOf(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 colour);
// The game's 0x4b9bc0 and 0x4bb200: a car's lights, a medium car's, for the view `view`.
bool carLightsDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 view);
bool mediumCarLightsDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 car, x86::reg32 view);
// The game's 0x4c0b20: a line between the vertices at `first` and `second`; eax what it leaves.
std::optional<x86::reg32> lineDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 first, x86::reg32 second);
// The game's 0x4ddb40: the sparks of view `view`'s group.
bool sparksDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view);
// The game's 0x4d3f50: a texture from the picture at `picture`, its palette `palette`, its corners `corners`, `side`
// the square's side, `format` the address of the format's word; the handle.
std::optional<x86::reg32> squareTextureMade(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 picture,
                                            x86::reg32 palette, x86::reg32 corners, x86::reg32 side,
                                            x86::reg32 format);
// The game's 0x47f650: the frame around a HUD element, eight quads at `quads`, around the box `left`, `top`,
// `right`, `bottom`; eax what it leaves.
std::optional<x86::reg32> hudFrameBuilt(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 quads, x86::reg32 top,
                                        x86::reg32 left, x86::reg32 bottom, x86::reg32 right);

// The functions of this file the code calls by address, as cores for guest::direct: the thunk's body run with the
// words in eax, edx, ebx, ecx in turn (the registers the callers set) and the rest on the stack; eax what it leaves,
// nullopt when the game stopped.
// 0x41df10: a view record.
std::optional<x86::reg32> viewRecordMade(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 kind, x86::reg32 size,
                                         x86::reg32 record);
// 0x432720: a quad piece at depth `depth` from the projected corners sa..sc and sd (first stack word), the vertices a..d.
std::optional<x86::reg32> quadPieceDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 depth, x86::reg32 sa,
                                         x86::reg32 sb, x86::reg32 sc, x86::reg32 sd, x86::reg32 a, x86::reg32 b,
                                         x86::reg32 c, x86::reg32 d);
// 0x433060: the quad a, b, c, d drawn whole or subdivided.
std::optional<x86::reg32> quadWholeDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b,
                                         x86::reg32 c, x86::reg32 d);
// 0x475b50: the text `text` at (`x`, `y`) in the solid font, `scale`, `size`, `texture` and `matrix` the words it takes
// on the stack (the first one lowest).
std::optional<x86::reg32> solidTextDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 text, x86::reg32 x,
                                         x86::reg32 y, x86::reg32 scale, x86::reg32 size, x86::reg32 texture,
                                         x86::reg32 matrix);
// 0x47a1f0: the flat sky.
std::optional<x86::reg32> flatSkyDrawn(win32::WinApplication* app, x86::CPU& cpu);
// 0x47b7d0: the colours of `count` vertices at `vertices` darkened by the glare.
std::optional<x86::reg32> glareVerticesSet(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 count,
                                           x86::reg32 vertices);
// 0x47a190: the sky of view `view`.
std::optional<x86::reg32> skyDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view);
// 0x47b850, 0x4946e0, 0x4880b0, 0x488470: the sky dome, the lightning, the needles and the bar gauges of view/car `view`.
std::optional<x86::reg32> skyDomeDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view);
std::optional<x86::reg32> lightningDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view);
std::optional<x86::reg32> needlesDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view);
std::optional<x86::reg32> barGaugesDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view);
// 0x489240: the cabin.
std::optional<x86::reg32> cockpitDrawn(win32::WinApplication* app, x86::CPU& cpu);
// 0x4bf790: the triangle `list` (three corners) clipped by z, `codes` their codes, `any` the codes or-ed.
std::optional<x86::reg32> clippedByZ(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 list, x86::reg32 codes,
                                     x86::reg32 any);
// 0x4c3ad0: the near triangle of record `record`, `nearBits` its near bits, `ebx` the record again, `ecx` as the code has it.
std::optional<x86::reg32> nearTriangleDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 record,
                                            x86::reg32 nearBits, x86::reg32 ebx, x86::reg32 ecx);
// 0x49d640: the sun's lighting back to the game's defaults.
std::optional<x86::reg32> sunLightingReset(win32::WinApplication* app, x86::CPU& cpu);
// 0x41b9b0, 0x482b00 and 0x482620: the view pass, the HUD and the countdown of the race, `view` in eax and `esi` as the
// code has it.
std::optional<x86::reg32> viewPassDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 esi);
std::optional<x86::reg32> hudDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 esi);
std::optional<x86::reg32> countdownDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 view, x86::reg32 esi);
// 0x418990: player `player`'s chase at level `level`.
std::optional<x86::reg32> chaseLevelRaised(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 level, x86::reg32 player);
// 0x4c20e0: the quad a, b, c, d whose corners carry z to the screen.
std::optional<x86::reg32> quadByZDrawn(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 a, x86::reg32 b, x86::reg32 c,
                                       x86::reg32 d);
// 0x4d4580: a piece texture.
std::optional<x86::reg32> pieceTextureMade(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 pieces,
                                           x86::reg32 record, x86::reg32 format, x86::reg32 x);

}  // namespace render

}  // namespace nfs3hp

#endif
