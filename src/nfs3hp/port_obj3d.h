#ifndef NFS3HP_PORT_OBJ3D_H
#define NFS3HP_PORT_OBJ3D_H

#include "native_guest.h"
#include "port.h"

/* The menus' 3D object, rewritten (port.h says how): the obj3d widget that
 * shows a car (or the game's logo) turning on a menu screen -- its handlers
 * (shown, hidden, drawn, a key, the mouse let go, passed over), the loader
 * that reads the car's .viv a part at a time (its header, the model, its
 * texture and shapes) while the screen runs, the fade between two cars, the
 * object turned by the clock or dragged by the mouse, its lights' flares,
 * its rectangle on the screen, the logo's quad.
 *
 * Each function is a typed core under its own name and a thunk under the
 * game's, which is bound to the game's address
 * (PORT_OBJ3D_NATIVES): the thunk takes the arguments from the registers
 * and the stack and gives back what Watcom's convention promises the caller
 * -- the result in eax (what the game's code leaves there), every other
 * register as it was, a stdcall's stack words taken off -- but where a
 * caller reads more of what the game's code leaves (port_obj3d.cpp says
 * which).  The cores call each other as C++, and what is not rewritten -- the
 * menus' loop, the renderer, the models, EA's files and memory -- by address,
 * from where the game's code had its stack.
 *
 * The x87's arithmetic is the game's to the bit: each product and sum through
 * fp::, in the order and with the operands the code has, under the control
 * word as it is; a value stored as a float where the code stores one. */
namespace nfs3hp
{
namespace port
{
namespace obj3d
{

/* ---- The typed cores ---- */

/* Each takes `esp`, the game's esp at its entry (at the return address). */

/* Car `car`'s .viv path into `buffer`: the cars' folder, the car's own, its
 * files' prefix, ".VIV". */
Result carVivPathOf(App* app, ThreadState& ts, u32 esp, int32_t car, u32 buffer);
/* The generic car's and the track's .viv paths into `buffer`; the model's
 * name of number `n` ("go%02d"), in a buffer of the game's. */
Result genericVivPathOf(App* app, ThreadState& ts, u32 esp, u32 buffer);
Result genericModelNameOf(App* app, ThreadState& ts, u32 esp, int32_t n);
Result trackVivPathOf(App* app, ThreadState& ts, u32 esp, u32 buffer);
Result trackModelNameOf(App* app, ThreadState& ts, u32 esp, int32_t n);

/* A light's flare: a quad of the flares' texture at the 3D point `x`, `y`,
 * `z` (floats' bits) in `colour`, `size` across, turned by its distance --
 * in a room of the renderer's, its record handed back; 0 when it is behind
 * the eye or off the screen.  `zSlot` where the code's z argument lies on
 * the stack (a z of 0 is written there as the code does), 0 for none. */
Result flare(App* app, ThreadState& ts, u32 esp, u32 x, u32 y, u32 z, uint32_t colour, u32 size, u32 zSlot);
/* Light `index` of model `model` turned by `matrix` and moved by the offset
 * (floats' bits) into `out`: 0 when the model has no such light. */
Result lightPlace(App* app, ThreadState& ts, u32 esp, u32 model, int32_t index, u32 out, u32 matrix,
                  const u32 offset[3]);
/* A headlight's (and a taillight's) flare, by how far the light faces the
 * eye, its alpha scaled by `alpha`: the flare's record joined to the list at
 * `pair` (the renderer's first and last). */
Result headlightFlare(App* app, ThreadState& ts, u32 esp, u32 pair, u32 model, u32 matrix, u32 position,
                      int32_t index, u32 alpha);
Result taillightFlare(App* app, ThreadState& ts, u32 esp, u32 pair, u32 model, u32 matrix, u32 position,
                      int32_t index, u32 alpha);
/* The flares of every light of `model` into the list at `out`: `out`. */
Result lightFlares(App* app, ThreadState& ts, u32 esp, u32 model, u32 matrix, u32 position, u32 alpha,
                   u32 out);

/* The object dragged round by the mouse while its button is down. */
Result dragSpin(App* app, ThreadState& ts, u32 esp, u32 item, u32 edx);
/* The object's rectangle on the screen into `rect`: its model's box turned
 * as the object is, projected, the smallest rectangle round it. */
Result screenBounds(App* app, ThreadState& ts, u32 esp, u32 item, u32 rect);

/* The handlers. */
Result drawObject(App* app, ThreadState& ts, u32 esp, u32 item, u32 edx);
Result objectKey(App* app, ThreadState& ts, u32 esp, u32 item);
Result objectPass(App* app, ThreadState& ts, u32 esp, u32 item);
Result objectUp(App* app, ThreadState& ts, u32 esp, u32 item);
Result objectShown(App* app, ThreadState& ts, u32 esp, u32 item);
Result objectHidden(App* app, ThreadState& ts, u32 esp, u32 item);

/* The loader: started afresh; the files of the object's car chosen; the
 * .viv's first bytes, its header, the model, the texture and the shapes read;
 * the model taken over by the object; its state run a step. */
Result resetLoaderOf(App* app, ThreadState& ts, u32 esp, u32 loader);
Result chooseFilesOf(App* app, ThreadState& ts, u32 esp, u32 item);
Result readPreheadOf(App* app, ThreadState& ts, u32 esp, u32 loader);
Result readHeadOf(App* app, ThreadState& ts, u32 esp, u32 loader);
Result readPartOf(App* app, ThreadState& ts, u32 esp, u32 loader, int part);
Result takeModelOf(App* app, ThreadState& ts, u32 esp, u32 item);
Result fadeInOf(App* app, ThreadState& ts, u32 esp, u32 item);
Result fadeOutOf(App* app, ThreadState& ts, u32 esp, u32 item);
Result wantedChangedOf(App* app, ThreadState& ts, u32 esp, u32 item);
Result runLoaderOf(App* app, ThreadState& ts, u32 esp, u32 item);
/* The object's car's colour into `out` (4 words); its texture made. */
Result carColourOf(App* app, ThreadState& ts, u32 esp, u32 item, u32 out);
Result makeTextureOf(App* app, ThreadState& ts, u32 esp, u32 item);

/* The logo: the shape's texture over the rectangle at `x`, `y`, `width` by
 * `height`, in `colour`, as a quad of the 2D screens. */
Result drawLogoOf(App* app, ThreadState& ts, u32 esp, u32 texture, int32_t x, int32_t y, int32_t width, int32_t height,
                  uint32_t colour);

/* ---- The thunks, under the game's names ---- */

nfs::u32 carVivPath(App* app, ThreadState& ts, nfs::Args& args);        // 0x439360
nfs::u32 flareQuad(App* app, ThreadState& ts, nfs::Args& args);         // 0x465eb0
nfs::u32 lightPlaced(App* app, ThreadState& ts, nfs::Args& args);       // 0x466330
nfs::u32 headlightGlare(App* app, ThreadState& ts, nfs::Args& args);    // 0x4663a0
nfs::u32 taillightGlare(App* app, ThreadState& ts, nfs::Args& args);    // 0x466580
nfs::u32 objectLights(App* app, ThreadState& ts, nfs::Args& args);      // 0x466740
nfs::u32 objectDragged(App* app, ThreadState& ts, nfs::Args& args);     // 0x4667d0
nfs::u32 objectBounds(App* app, ThreadState& ts, nfs::Args& args);      // 0x4668b0
nfs::u32 objectDraw(App* app, ThreadState& ts, nfs::Args& args);        // 0x466cf0
nfs::u32 objectKeyed(App* app, ThreadState& ts, nfs::Args& args);       // 0x4675b0
nfs::u32 objectPassed(App* app, ThreadState& ts, nfs::Args& args);      // 0x4675d0
nfs::u32 objectReleased(App* app, ThreadState& ts, nfs::Args& args);    // 0x467650
nfs::u32 objectOpened(App* app, ThreadState& ts, nfs::Args& args);      // 0x467700
nfs::u32 objectClosed(App* app, ThreadState& ts, nfs::Args& args);      // 0x467930
nfs::u32 resetLoader(App* app, ThreadState& ts, nfs::Args& args);       // 0x467ac0
nfs::u32 genericVivPath(App* app, ThreadState& ts, nfs::Args& args);    // 0x467bc0
nfs::u32 genericModelName(App* app, ThreadState& ts, nfs::Args& args);  // 0x467be0
nfs::u32 trackVivPath(App* app, ThreadState& ts, nfs::Args& args);      // 0x467c00
nfs::u32 trackModelName(App* app, ThreadState& ts, nfs::Args& args);    // 0x467c20
nfs::u32 chooseFiles(App* app, ThreadState& ts, nfs::Args& args);       // 0x467c40
nfs::u32 readPrehead(App* app, ThreadState& ts, nfs::Args& args);       // 0x467ea0
nfs::u32 readHead(App* app, ThreadState& ts, nfs::Args& args);          // 0x467ef0
nfs::u32 readModel(App* app, ThreadState& ts, nfs::Args& args);         // 0x468010
nfs::u32 readTexture(App* app, ThreadState& ts, nfs::Args& args);       // 0x468170
nfs::u32 readShapes(App* app, ThreadState& ts, nfs::Args& args);        // 0x4682c0
nfs::u32 takeModel(App* app, ThreadState& ts, nfs::Args& args);         // 0x468410
nfs::u32 fadeIn(App* app, ThreadState& ts, nfs::Args& args);            // 0x468580
nfs::u32 fadeOut(App* app, ThreadState& ts, nfs::Args& args);           // 0x4685e0
nfs::u32 wantedChanged(App* app, ThreadState& ts, nfs::Args& args);     // 0x468670
nfs::u32 runLoader(App* app, ThreadState& ts, nfs::Args& args);         // 0x468860
nfs::u32 carColour(App* app, ThreadState& ts, nfs::Args& args);         // 0x468da0
nfs::u32 makeTexture(App* app, ThreadState& ts, nfs::Args& args);       // 0x468f00
nfs::u32 drawLogo(App* app, ThreadState& ts, nfs::Args& args);          // 0x4d79b0

}  // namespace obj3d
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
