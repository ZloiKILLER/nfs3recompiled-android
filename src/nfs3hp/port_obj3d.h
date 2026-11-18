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
Result carVivPathOf(App* app, CPU& cpu, reg32 esp, int32_t car, reg32 buffer);
/* The generic car's and the track's .viv paths into `buffer`; the model's
 * name of number `n` ("go%02d"), in a buffer of the game's. */
Result genericVivPathOf(App* app, CPU& cpu, reg32 esp, reg32 buffer);
Result genericModelNameOf(App* app, CPU& cpu, reg32 esp, int32_t n);
Result trackVivPathOf(App* app, CPU& cpu, reg32 esp, reg32 buffer);
Result trackModelNameOf(App* app, CPU& cpu, reg32 esp, int32_t n);

/* A light's flare: a quad of the flares' texture at the 3D point `x`, `y`,
 * `z` (floats' bits) in `colour`, `size` across, turned by its distance --
 * in a room of the renderer's, its record handed back; 0 when it is behind
 * the eye or off the screen.  `zSlot` where the code's z argument lies on
 * the stack (a z of 0 is written there as the code does), 0 for none. */
Result flare(App* app, CPU& cpu, reg32 esp, reg32 x, reg32 y, reg32 z, uint32_t colour, reg32 size, reg32 zSlot);
/* Light `index` of model `model` turned by `matrix` and moved by the offset
 * (floats' bits) into `out`: 0 when the model has no such light. */
Result lightPlace(App* app, CPU& cpu, reg32 esp, reg32 model, int32_t index, reg32 out, reg32 matrix,
                  const reg32 offset[3]);
/* A headlight's (and a taillight's) flare, by how far the light faces the
 * eye, its alpha scaled by `alpha`: the flare's record joined to the list at
 * `pair` (the renderer's first and last). */
Result headlightFlare(App* app, CPU& cpu, reg32 esp, reg32 pair, reg32 model, reg32 matrix, reg32 position,
                      int32_t index, reg32 alpha);
Result taillightFlare(App* app, CPU& cpu, reg32 esp, reg32 pair, reg32 model, reg32 matrix, reg32 position,
                      int32_t index, reg32 alpha);
/* The flares of every light of `model` into the list at `out`: `out`. */
Result lightFlares(App* app, CPU& cpu, reg32 esp, reg32 model, reg32 matrix, reg32 position, reg32 alpha,
                   reg32 out);

/* The object dragged round by the mouse while its button is down. */
Result dragSpin(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 edx);
/* The object's rectangle on the screen into `rect`: its model's box turned
 * as the object is, projected, the smallest rectangle round it. */
Result screenBounds(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 rect);

/* The handlers. */
Result drawObject(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 edx);
Result objectKey(App* app, CPU& cpu, reg32 esp, reg32 item);
Result objectPass(App* app, CPU& cpu, reg32 esp, reg32 item);
Result objectUp(App* app, CPU& cpu, reg32 esp, reg32 item);
Result objectShown(App* app, CPU& cpu, reg32 esp, reg32 item);
Result objectHidden(App* app, CPU& cpu, reg32 esp, reg32 item);

/* The loader: started afresh; the files of the object's car chosen; the
 * .viv's first bytes, its header, the model, the texture and the shapes read;
 * the model taken over by the object; its state run a step. */
Result resetLoaderOf(App* app, CPU& cpu, reg32 esp, reg32 loader);
Result chooseFilesOf(App* app, CPU& cpu, reg32 esp, reg32 item);
Result readPreheadOf(App* app, CPU& cpu, reg32 esp, reg32 loader);
Result readHeadOf(App* app, CPU& cpu, reg32 esp, reg32 loader);
Result readPartOf(App* app, CPU& cpu, reg32 esp, reg32 loader, int part);
Result takeModelOf(App* app, CPU& cpu, reg32 esp, reg32 item);
Result fadeInOf(App* app, CPU& cpu, reg32 esp, reg32 item);
Result fadeOutOf(App* app, CPU& cpu, reg32 esp, reg32 item);
Result wantedChangedOf(App* app, CPU& cpu, reg32 esp, reg32 item);
Result runLoaderOf(App* app, CPU& cpu, reg32 esp, reg32 item);
/* The object's car's colour into `out` (4 words); its texture made. */
Result carColourOf(App* app, CPU& cpu, reg32 esp, reg32 item, reg32 out);
Result makeTextureOf(App* app, CPU& cpu, reg32 esp, reg32 item);

/* The logo: the shape's texture over the rectangle at `x`, `y`, `width` by
 * `height`, in `colour`, as a quad of the 2D screens. */
Result drawLogoOf(App* app, CPU& cpu, reg32 esp, reg32 texture, int32_t x, int32_t y, int32_t width, int32_t height,
                  uint32_t colour);

/* ---- The thunks, under the game's names ---- */

x86::reg32 carVivPath(App* app, CPU& cpu, x86::Args& args);        // 0x439360
x86::reg32 flareQuad(App* app, CPU& cpu, x86::Args& args);         // 0x465eb0
x86::reg32 lightPlaced(App* app, CPU& cpu, x86::Args& args);       // 0x466330
x86::reg32 headlightGlare(App* app, CPU& cpu, x86::Args& args);    // 0x4663a0
x86::reg32 taillightGlare(App* app, CPU& cpu, x86::Args& args);    // 0x466580
x86::reg32 objectLights(App* app, CPU& cpu, x86::Args& args);      // 0x466740
x86::reg32 objectDragged(App* app, CPU& cpu, x86::Args& args);     // 0x4667d0
x86::reg32 objectBounds(App* app, CPU& cpu, x86::Args& args);      // 0x4668b0
x86::reg32 objectDraw(App* app, CPU& cpu, x86::Args& args);        // 0x466cf0
x86::reg32 objectKeyed(App* app, CPU& cpu, x86::Args& args);       // 0x4675b0
x86::reg32 objectPassed(App* app, CPU& cpu, x86::Args& args);      // 0x4675d0
x86::reg32 objectReleased(App* app, CPU& cpu, x86::Args& args);    // 0x467650
x86::reg32 objectOpened(App* app, CPU& cpu, x86::Args& args);      // 0x467700
x86::reg32 objectClosed(App* app, CPU& cpu, x86::Args& args);      // 0x467930
x86::reg32 resetLoader(App* app, CPU& cpu, x86::Args& args);       // 0x467ac0
x86::reg32 genericVivPath(App* app, CPU& cpu, x86::Args& args);    // 0x467bc0
x86::reg32 genericModelName(App* app, CPU& cpu, x86::Args& args);  // 0x467be0
x86::reg32 trackVivPath(App* app, CPU& cpu, x86::Args& args);      // 0x467c00
x86::reg32 trackModelName(App* app, CPU& cpu, x86::Args& args);    // 0x467c20
x86::reg32 chooseFiles(App* app, CPU& cpu, x86::Args& args);       // 0x467c40
x86::reg32 readPrehead(App* app, CPU& cpu, x86::Args& args);       // 0x467ea0
x86::reg32 readHead(App* app, CPU& cpu, x86::Args& args);          // 0x467ef0
x86::reg32 readModel(App* app, CPU& cpu, x86::Args& args);         // 0x468010
x86::reg32 readTexture(App* app, CPU& cpu, x86::Args& args);       // 0x468170
x86::reg32 readShapes(App* app, CPU& cpu, x86::Args& args);        // 0x4682c0
x86::reg32 takeModel(App* app, CPU& cpu, x86::Args& args);         // 0x468410
x86::reg32 fadeIn(App* app, CPU& cpu, x86::Args& args);            // 0x468580
x86::reg32 fadeOut(App* app, CPU& cpu, x86::Args& args);           // 0x4685e0
x86::reg32 wantedChanged(App* app, CPU& cpu, x86::Args& args);     // 0x468670
x86::reg32 runLoader(App* app, CPU& cpu, x86::Args& args);         // 0x468860
x86::reg32 carColour(App* app, CPU& cpu, x86::Args& args);         // 0x468da0
x86::reg32 makeTexture(App* app, CPU& cpu, x86::Args& args);       // 0x468f00
x86::reg32 drawLogo(App* app, CPU& cpu, x86::Args& args);          // 0x4d79b0

}  // namespace obj3d
}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
