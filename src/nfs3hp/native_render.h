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
void choosePolygonDrawers(nfs::App* app);

// The functions below are written as the game's code runs them, the registers and the frames line by line
// (their bodies are native_render.cpp's thunks); each is called as C++ here, its registers set from the
// arguments, `false` or nullopt when the game stopped in it.  Everything it reads is an argument or the game's
// memory but where its comment says a register is kept as it came (the caller's).

// The game's 0x41d620: the views' distances for View Distance; eax what it leaves.
std::optional<nfs::u32> viewDistancesSet(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x472d10: the detail settings for the machine; eax what it leaves.
std::optional<nfs::u32> detailSettingsSet(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4bef50: screen mode `mode` opened from the driver's list.
bool screenModeSet(nfs::App* app, nfs::ThreadState& ts, nfs::u32 mode);
// The game's 0x4d0a10: the 2D picture at [0x56727c] made a texture.
bool pictureTextured(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4d41a0: the paletted picture `picture` as a texture into the record `record`.
bool paletteTextured(nfs::App* app, nfs::ThreadState& ts, nfs::u32 picture, nfs::u32 record);
// The game's 0x4de070: view `view`'s smoke, `count` particles, the list at `list` (esi).
bool smokeDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 count, nfs::u32 list);
// The game's 0x4de700: view `view`'s spray streaks, `count` of them, `liveCount` live, the list at `list` (esi).
bool streaksDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 count, nfs::u32 liveCount,
                  nfs::u32 list);
// The game's 0x434870 as the code runs it: the polygon lists' drawers chosen.
bool polygonDrawersSet(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x4b59c0: driver `driver` started, the window's message `message`.
bool driverStarted(nfs::App* app, nfs::ThreadState& ts, nfs::u32 driver, nfs::u32 message);
// The game's 0x4b7d30: the textures of the car `car`, its 3D shape `shape`.
bool carTexturesMade(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car, nfs::u32 shape);
// The game's 0x47a880: the sky of the pass `view`, the two words it makes at `to` (esi).
bool viewSkyMade(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 to);
// The game's 0x41d960: the views for a race.
bool raceViewsSet(nfs::App* app, nfs::ThreadState& ts);
// The game's 0x494cb0: the loading screen, the picture `picture` shown.
bool loadingPictureShown(nfs::App* app, nfs::ThreadState& ts, nfs::u32 picture);
// The game's 0x4800a0: the dark band behind player `player`'s HUD in split screen; eax what it leaves.
std::optional<nfs::u32> hudBandBuilt(nfs::App* app, nfs::ThreadState& ts, nfs::u32 player);

// The game's 0x480910: the HUD rectangle of element `element` of view `view`, the view's size `width` by `height`.
bool hudRectPlaced(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 element, nfs::u32 width,
                   nfs::u32 height);
// The game's 0x480fc0, 0x4812e0, 0x481aa0 and 0x481c50: view `view`'s picture, dial, map panel and table of the HUD
// placed; eax what each leaves.
std::optional<nfs::u32> hudPicturePlaced(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view);
std::optional<nfs::u32> hudDialPlaced(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view);
std::optional<nfs::u32> hudPanelPlaced(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view);
std::optional<nfs::u32> hudTablePlaced(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view);
// The game's 0x48c0e0: a meter of segments at (`x`, `y`) `width` by `height`, the first `lit` of them lit, `corner`
// the colour of two corners; eax what it leaves (`view` is what the code has in eax, which it does not read).
std::optional<nfs::u32> meterDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 x,
                                     nfs::u32 y, nfs::u32 width, nfs::u32 height, nfs::u32 lit,
                                     nfs::u32 corner);
// The game's 0x48c7c0: the start lights' panel, `first`..`fourth` the quads of its lamps, `lit` the stack's word;
// eax what it leaves.
std::optional<nfs::u32> startLightsDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 first,
                                           nfs::u32 second, nfs::u32 third, nfs::u32 fourth, nfs::u32 lit);

// The game's 0x49d800: the glare's two colours `first` and `second` set, darkened by the sun's glare.
void glareColoursSet(nfs::App* app, nfs::u32 first, nfs::u32 second);
// The game's 0x4b56e0: the transform buffer's size set to `size` and the buffer allocated; the allocator's eax,
// nullopt when the game stopped.
std::optional<nfs::u32> transformBufferAllocated(nfs::App* app, nfs::ThreadState& ts, nfs::u32 size);
// The game's 0x4b97b0: car `car`'s colour `colour` at night; the colour.
std::optional<nfs::u32> nightColourOf(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car, nfs::u32 colour);
// The game's 0x4b9bc0 and 0x4bb200: a car's lights, a medium car's, for the view `view`.
bool carLightsDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car, nfs::u32 view);
bool mediumCarLightsDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 car, nfs::u32 view);
// The game's 0x4c0b20: a line between the vertices at `first` and `second`; eax what it leaves.
std::optional<nfs::u32> lineDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 first, nfs::u32 second);
// The game's 0x4ddb40: the sparks of view `view`'s group.
bool sparksDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view);
// The game's 0x4d3f50: a texture from the picture at `picture`, its palette `palette`, its corners `corners`, `side`
// the square's side, `format` the address of the format's word; the handle.
std::optional<nfs::u32> squareTextureMade(nfs::App* app, nfs::ThreadState& ts, nfs::u32 picture,
                                            nfs::u32 palette, nfs::u32 corners, nfs::u32 side,
                                            nfs::u32 format);
// The game's 0x47f650: the frame around a HUD element, eight quads at `quads`, around the box `left`, `top`,
// `right`, `bottom`; eax what it leaves.
std::optional<nfs::u32> hudFrameBuilt(nfs::App* app, nfs::ThreadState& ts, nfs::u32 quads, nfs::u32 top,
                                        nfs::u32 left, nfs::u32 bottom, nfs::u32 right);

// The functions of this file the code calls by address, as cores for guest::direct: the thunk's body run with the
// words in eax, edx, ebx, ecx in turn (the registers the callers set) and the rest on the stack; eax what it leaves,
// nullopt when the game stopped.
// 0x41df10: a view record.
std::optional<nfs::u32> viewRecordMade(nfs::App* app, nfs::ThreadState& ts, nfs::u32 kind, nfs::u32 size,
                                         nfs::u32 record);
// 0x432720: a quad piece at depth `depth` from the projected corners sa..sc and sd (first stack word), the vertices a..d.
std::optional<nfs::u32> quadPieceDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 depth, nfs::u32 sa,
                                         nfs::u32 sb, nfs::u32 sc, nfs::u32 sd, nfs::u32 a, nfs::u32 b,
                                         nfs::u32 c, nfs::u32 d);
// 0x433060: the quad a, b, c, d drawn whole or subdivided.
std::optional<nfs::u32> quadWholeDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 a, nfs::u32 b,
                                         nfs::u32 c, nfs::u32 d);
// 0x475b50: the text `text` at (`x`, `y`) in the solid font, `scale`, `size`, `texture` and `matrix` the words it takes
// on the stack (the first one lowest).
std::optional<nfs::u32> solidTextDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 text, nfs::u32 x,
                                         nfs::u32 y, nfs::u32 scale, nfs::u32 size, nfs::u32 texture,
                                         nfs::u32 matrix);
// 0x47a1f0: the flat sky.
std::optional<nfs::u32> flatSkyDrawn(nfs::App* app, nfs::ThreadState& ts);
// 0x47b7d0: the colours of `count` vertices at `vertices` darkened by the glare.
std::optional<nfs::u32> glareVerticesSet(nfs::App* app, nfs::ThreadState& ts, nfs::u32 count,
                                           nfs::u32 vertices);
// 0x47a190: the sky of view `view`.
std::optional<nfs::u32> skyDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view);
// 0x47b850, 0x4946e0, 0x4880b0, 0x488470: the sky dome, the lightning, the needles and the bar gauges of view/car `view`.
std::optional<nfs::u32> skyDomeDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view);
std::optional<nfs::u32> lightningDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view);
std::optional<nfs::u32> needlesDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view);
std::optional<nfs::u32> barGaugesDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view);
// 0x489240: the cabin.
std::optional<nfs::u32> cockpitDrawn(nfs::App* app, nfs::ThreadState& ts);
// 0x4bf790: the triangle `list` (three corners) clipped by z, `codes` their codes, `any` the codes or-ed.
std::optional<nfs::u32> clippedByZ(nfs::App* app, nfs::ThreadState& ts, nfs::u32 list, nfs::u32 codes,
                                     nfs::u32 any);
// 0x4c3ad0: the near triangle of record `record`, `nearBits` its near bits, `ebx` the record again, `ecx` as the code has it.
std::optional<nfs::u32> nearTriangleDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 record,
                                            nfs::u32 nearBits, nfs::u32 ebx, nfs::u32 ecx);
// 0x49d640: the sun's lighting back to the game's defaults.
std::optional<nfs::u32> sunLightingReset(nfs::App* app, nfs::ThreadState& ts);
// 0x41b9b0, 0x482b00 and 0x482620: the view pass, the HUD and the countdown of the race, `view` in eax and `esi` as the
// code has it.
std::optional<nfs::u32> viewPassDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 esi);
std::optional<nfs::u32> hudDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 esi);
std::optional<nfs::u32> countdownDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 view, nfs::u32 esi);
// 0x418990: player `player`'s chase at level `level`.
std::optional<nfs::u32> chaseLevelRaised(nfs::App* app, nfs::ThreadState& ts, nfs::u32 level, nfs::u32 player);
// 0x4c20e0: the quad a, b, c, d whose corners carry z to the screen.
std::optional<nfs::u32> quadByZDrawn(nfs::App* app, nfs::ThreadState& ts, nfs::u32 a, nfs::u32 b, nfs::u32 c,
                                       nfs::u32 d);
// 0x4d4580: a piece texture.
std::optional<nfs::u32> pieceTextureMade(nfs::App* app, nfs::ThreadState& ts, nfs::u32 pieces,
                                           nfs::u32 record, nfs::u32 format, nfs::u32 x);

}  // namespace render

}  // namespace nfs3hp

#endif
