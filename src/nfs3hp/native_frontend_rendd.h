#ifndef NFS3HP_NATIVE_FRONTEND_RENDD_H
#define NFS3HP_NATIVE_FRONTEND_RENDD_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_rendd.cpp's typed functions (the race's cars as the renderer sees them: the frame's
 * arena and its lists, each car at its detail, its lights, shadow and colours, the replay's ghost, the
 * projection), which other files call as C++ (guest::direct), and how the game's code calls each by
 * address.  Each calls the game's other functions by address on
 * `ts` and throws fe::Terminated when the game stopped in one, as a call by address does; its result is what
 * the game's function leaves in eax.  A list is two dwords, its first record and its last. */
namespace nfs3hp
{
namespace frontend
{
namespace rendd
{

using App = nfs::App;
using ThreadState = nfs::ThreadState;
using u32 = nfs::u32;

// The game's 0x4b5700: the block freed, if there is one.
u32 freeBlock(App* app, ThreadState& ts, u32 block);
// The game's 0x4b5720: the driver the registry and the command line name, to *chosen.
u32 chooseDriver(App* app, ThreadState& ts, u32 chosen);
// The game's 0x4b5940: which of the driver's five features answer, a bit each to 0x7a3a5c.
u32 readDriverFeatures(App* app, ThreadState& ts, u32 first);
// The game's 0x4b7d00: the player's model freed and the models' batch ended.
u32 freePlayerModel(App* app, ThreadState& ts);
// The game's 0x4b7eb0: the car's model freed, the replay's too when it is the player's.
u32 freeCarModel(App* app, ThreadState& ts, u32 car);
// The game's 0x4b7f80: the four corners of the car's shadow, on the road.
u32 shadowCorners(App* app, ThreadState& ts, u32 car, u32 corners);
// The game's 0x4b81a0: the car's shadow in the view, a quad on its list.
u32 carShadow(App* app, ThreadState& ts, u32 car, u32 view);
// The game's 0x4b8740: the flares of the car's lights, its body at matrix and place.
u32 carLights(App* app, ThreadState& ts, u32 view, u32 car, u32 matrix, u32 place, u32 flareSize, u32 paint);
// The game's 0x4b90e0: the car's body's shake this frame, along or across.
u32 bodyShake(App* app, ThreadState& ts, u32 car, u32 across);
// The game's 0x4b9140: a front wheel's matrix, steered when asked.
u32 frontWheelTurn(App* app, ThreadState& ts, u32 car, u32 out, u32 steered);
// The game's 0x4b9280: a rear wheel's matrix.
u32 rearWheelTurn(App* app, ThreadState& ts, u32 car, u32 out);
// The game's 0x4b93c0: the car's body leaning, its matrix to out.
u32 bodyLean(App* app, ThreadState& ts, u32 car, u32 out, u32 before);
// The game's 0x4b9550: the colour lit by the track's lights around the car.
u32 trackLitColour(App* app, ThreadState& ts, u32 car, u32 colour);
// The game's 0x4b9df0: the recorded car drawn as its ghost beside the player's.
u32 replayPose(App* app, ThreadState& ts, u32 view, u32 car);
// The game's 0x4ba320: the state models draw in: ambient, sun, the outline's colours (outline, car's edx).
u32 carStates(App* app, ThreadState& ts, u32 view, u32 car, u32 outline);
// The game's 0x4ba350: the specular colours of the car's paint; the light's flag.
u32 carShade(App* app, ThreadState& ts, u32 view, u32 car, u32 paint);
// The game's 0x4ba810: the car's colour, the average of the road's at its wheels.
u32 carAverageColour(App* app, ThreadState& ts, u32 car);
// The game's 0x4ba960: a car at its least detail: its whole and its lights.
u32 carFarthest(App* app, ThreadState& ts, u32 car, u32 view);
// The game's 0x4baa50: a car far away: its whole, its lights and its shadow.
u32 carFar(App* app, ThreadState& ts, u32 car, u32 view);
// The game's 0x4bab70: a car near: body, wheels, lights and shadow.
u32 carNear(App* app, ThreadState& ts, u32 car, u32 view);
// The game's 0x4bb110: traffic's car at its nearest: one part, lights, shadow.
u32 carSimpleNear(App* app, ThreadState& ts, u32 car, u32 view);
// The game's 0x4bb4c0: the car in the view at the detail its model has.
u32 drawCar(App* app, ThreadState& ts, u32 car, u32 view, u32 detail);
// The game's 0x4bba10: how much of the budget the cars' details use.
u32 detailInUse(App* app, ThreadState& ts);
// The game's 0x4bba50: the farthest car but keep at detail brought to lowered; 0 when none.
u32 lowerDetail(App* app, ThreadState& ts, u32 keep, u32 detail, u32 lowered);
// The game's 0x4bbad0: the cars' details cut to the setting's budget.
u32 fitDetailBudget(App* app, ThreadState& ts, u32 camera, u32 ecx);
// The game's 0x4bbba0: every car in the view, the replay's ghost and the particles.
u32 cars(App* app, ThreadState& ts, u32 view);
// The game's 0x4bbd70: the arena back to its start.
u32 arenaRewind(App* app, ThreadState& ts);
// The game's 0x4bbd80: the arena made.
u32 arenaCreate(App* app, ThreadState& ts);
// The game's 0x4bbdc0: the arena freed.
u32 arenaFree(App* app, ThreadState& ts);
// The game's 0x4bbde0: bytes of the arena to *slot; 1, or 0 when it has run out.
u32 arenaTake(App* app, ThreadState& ts, u32 slot, u32 bytes);
// The game's 0x4bbe40: where the arena is now, to *slot; 1, or 0 when it has run out.
u32 arenaHere(App* app, ThreadState& ts, u32 slot);
// The game's 0x4bbe70: *list an empty list.
u32 emptyList(App* app, ThreadState& ts, u32 list);
// The game's 0x4bbe90: *list the list of the one record.
u32 listOfOne(App* app, ThreadState& ts, u32 record, u32 list);
// The game's 0x4bbec0: *list the two lists joined.
u32 joinLists(App* app, ThreadState& ts, u32 list, u32 firstHead, u32 firstTail, u32 secondHead, u32 secondTail);
// The game's 0x4bbf20: the open list empty.
u32 openListStart(App* app, ThreadState& ts);
// The game's 0x4bbf40: the list put on the end of the open one.
u32 openListAppend(App* app, ThreadState& ts, u32 head, u32 tail);
u32 openListAppendKeeping(App* app, ThreadState& ts, u32 head, u32 tail, u32 eax);
// The game's 0x4bbf80: bytes of the arena to *slot, on the end of the open list.
u32 openListTake(App* app, ThreadState& ts, u32 slot, u32 bytes);
// The game's 0x4bbfc0: the open list closed, to *list.
u32 openListClose(App* app, ThreadState& ts, u32 list);
// The game's 0x4bc000: the mesh's vertices flushed and listed, to *out.
u32 flushAndList(App* app, ThreadState& ts, u32 mesh, u32 out);
// The game's 0x4bc030: the list drawn by sub_4349d0 in the view, and left empty.
u32 drawListPlain(App* app, ThreadState& ts, u32 view, u32 list);
// The game's 0x4bc060: the list drawn at the depth, and left empty.
u32 drawListAt(App* app, ThreadState& ts, u32 view, u32 list, u32 depth);
// The game's 0x4bc0b0: the list drawn as a list of the depth, and left empty.
u32 drawListMirrored(App* app, ThreadState& ts, u32 view, u32 list, u32 depth);
// The game's 0x4bf110: the table of reciprocals at 0x7d30d8.
u32 fillReciprocals(App* app, ThreadState& ts);
// The game's 0x4bf1b0: the clip's numbers, the vertex cache marked unused.
u32 setClipBounds(App* app, ThreadState& ts, u32 left, u32 top, u32 right, u32 bottom, u32 nearest,
                    u32 farthest);
// The game's 0x4bf260: the projection's scale and centre from the window and the fields of view.
u32 setProjection(App* app, ThreadState& ts, u32 cornerXBits, u32 cornerYBits, u32 widthBits, u32 heightBits,
                    u32 fovX, u32 fovY, u32 mirrored);
// The game's 0x4bf370: count points projected to the vertices.
u32 projectPoints(App* app, ThreadState& ts, u32 count, u32 points, u32 vertices);

}  // namespace rendd
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
