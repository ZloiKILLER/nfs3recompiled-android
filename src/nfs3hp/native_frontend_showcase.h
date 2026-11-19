#ifndef NFS3HP_NATIVE_FRONTEND_SHOWCASE_H
#define NFS3HP_NATIVE_FRONTEND_SHOWCASE_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_showcase.cpp's typed functions (the car showcase: showcase, slides, history,
 * compare, compare2, interior), which other files call as C++ (guest::direct), and how the game's
 * code calls each by address.  Each calls the game's other
 * functions by address on `ts` and throws fe::Terminated when the game stopped in one, as
 * Guest::call does; its result is what the game's function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace showcase
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x439290: a car's file stem, in its record: 0 when it has no record.
u32 carFileStem(App* app, ThreadState& ts, u32 car);
// The game's 0x453160: whether the car has an interior to show: none without sound.
u32 hasInterior(App* app, ThreadState& ts, u32 car);
// The game's 0x453200: whether the car has photographs.
u32 hasSlides(App* app, ThreadState& ts, u32 car);
// The game's 0x453290: whether the car has a history.
u32 hasHistory(App* app, ThreadState& ts, u32 car);
// The game's 0x453320: showcase's backdrop again, its number changed.
u32 loadArt(App* app, ThreadState& ts, u32 menu);
// The game's 0x453500: the car the screen before this one was about; -1 for one of the showcase's own.
u32 resolveCar(App* app, ThreadState& ts);
// The game's 0x4535e0: showcase as it opens (menu, the screen): the car, its buttons, backdrop,
// description.
u32 showcaseEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x4537d0: showcase's every pass: opened anew for another car, the backdrop again.
u32 showcaseFrame(App* app, ThreadState& ts, u32 menu);
// The game's 0x453830: showcase as it closes, the description stopped.
u32 showcaseExit(App* app, ThreadState& ts);
// The game's 0x453840: the button: the next photograph.
u32 nextSlide(App* app, ThreadState& ts);
// The game's 0x453860: the button: the photograph before, from the first the last there is.
u32 previousSlide(App* app, ThreadState& ts);
// The game's 0x453c00: slides as it opens: its buttons, the photograph on its bitmap.
u32 slidesEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x453ef0: slides' every pass: make and model take turns.
u32 slidesFrame(App* app, ThreadState& ts, u32 menu);
// The game's 0x454000: the history's page for this era, its picture and mask at the two addresses: the
// file, or 0.
u32 historyPage(App* app, ThreadState& ts, u32 back, u32 mask);
// The game's 0x454210: the page of this era or of the first after it that has one; 0 when none has.
u32 historyPageFrom(App* app, ThreadState& ts, u32 back, u32 mask);
// The game's 0x454260: history as it opens: from the first era, its description spoken.
u32 historyEnter(App* app, ThreadState& ts, u32 menu);
// The game's 0x454310: history's every pass: opened anew for another car, the next era's page.
u32 historyFrame(App* app, ThreadState& ts, u32 menu);
// The game's 0x4543e0: history as it closes, the description stopped.
u32 historyExit(App* app, ThreadState& ts);
// The game's 0x4543f0: a car's picture for compare, as the texture it replaces or a new one.
u32 comparePicture(App* app, ThreadState& ts, u32 car, u32 replaced);
// The game's 0x454500: compare as it opens: the cars and their pictures.
u32 compareEnter(App* app, ThreadState& ts);
// The game's 0x454690: compare's every pass: on to compare2 when asked, a picture again for a car
// changed.
u32 compareFrame(App* app, ThreadState& ts);
// The game's 0x454580: compare2 as it opens: the cars, the bar and the four arrows.
u32 compare2Enter(App* app, ThreadState& ts);
// The game's 0x454660: compare2's every pass: back to compare when asked.
u32 compare2Frame(App* app, ThreadState& ts);
// The game's 0x454cf0: interior's zoom slider moved: its value to the view.
u32 zoomMoved(App* app, ThreadState& ts, u32 slider);
// The game's 0x454d10: interior as it opens: the slider's handler, the slider at 0.
u32 interiorEnter(App* app, ThreadState& ts, u32 menu);

}  // namespace showcase
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
