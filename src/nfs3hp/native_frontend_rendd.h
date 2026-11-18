#ifndef NFS3HP_NATIVE_FRONTEND_RENDD_H
#define NFS3HP_NATIVE_FRONTEND_RENDD_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_rendd.cpp's typed functions (the race's cars as the renderer sees them: the frame's
 * arena and its lists, each car at its detail, its lights, shadow and colours, the replay's ghost, the
 * projection), which other files call as C++ (guest::direct), and how the game's code calls each by
 * address.  Each calls the game's other functions by address on
 * `cpu` and throws fe::Terminated when the game stopped in one, as a call by address does; its result is what
 * the game's function leaves in eax.  A list is two dwords, its first record and its last. */
namespace nfs3hp
{
namespace frontend
{
namespace rendd
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4b5700: the block freed, if there is one.
reg32 freeBlock(App* app, CPU& cpu, reg32 block);
// The game's 0x4b5720: the driver the registry and the command line name, to *chosen.
reg32 chooseDriver(App* app, CPU& cpu, reg32 chosen);
// The game's 0x4b5940: which of the driver's five features answer, a bit each to 0x7a3a5c.
reg32 readDriverFeatures(App* app, CPU& cpu, reg32 first);
// The game's 0x4b7d00: the player's model freed and the models' batch ended.
reg32 freePlayerModel(App* app, CPU& cpu);
// The game's 0x4b7eb0: the car's model freed, the replay's too when it is the player's.
reg32 freeCarModel(App* app, CPU& cpu, reg32 car);
// The game's 0x4b7f80: the four corners of the car's shadow, on the road.
reg32 shadowCorners(App* app, CPU& cpu, reg32 car, reg32 corners);
// The game's 0x4b81a0: the car's shadow in the view, a quad on its list.
reg32 carShadow(App* app, CPU& cpu, reg32 car, reg32 view);
// The game's 0x4b8740: the flares of the car's lights, its body at matrix and place.
reg32 carLights(App* app, CPU& cpu, reg32 view, reg32 car, reg32 matrix, reg32 place, reg32 flareSize, reg32 paint);
// The game's 0x4b90e0: the car's body's shake this frame, along or across.
reg32 bodyShake(App* app, CPU& cpu, reg32 car, reg32 across);
// The game's 0x4b9140: a front wheel's matrix, steered when asked.
reg32 frontWheelTurn(App* app, CPU& cpu, reg32 car, reg32 out, reg32 steered);
// The game's 0x4b9280: a rear wheel's matrix.
reg32 rearWheelTurn(App* app, CPU& cpu, reg32 car, reg32 out);
// The game's 0x4b93c0: the car's body leaning, its matrix to out.
reg32 bodyLean(App* app, CPU& cpu, reg32 car, reg32 out, reg32 before);
// The game's 0x4b9550: the colour lit by the track's lights around the car.
reg32 trackLitColour(App* app, CPU& cpu, reg32 car, reg32 colour);
// The game's 0x4b9df0: the recorded car drawn as its ghost beside the player's.
reg32 replayPose(App* app, CPU& cpu, reg32 view, reg32 car);
// The game's 0x4ba320: the state models draw in: ambient, sun, the outline's colours (outline, car's edx).
reg32 carStates(App* app, CPU& cpu, reg32 view, reg32 car, reg32 outline);
// The game's 0x4ba350: the specular colours of the car's paint; the light's flag.
reg32 carShade(App* app, CPU& cpu, reg32 view, reg32 car, reg32 paint);
// The game's 0x4ba810: the car's colour, the average of the road's at its wheels.
reg32 carAverageColour(App* app, CPU& cpu, reg32 car);
// The game's 0x4ba960: a car at its least detail: its whole and its lights.
reg32 carFarthest(App* app, CPU& cpu, reg32 car, reg32 view);
// The game's 0x4baa50: a car far away: its whole, its lights and its shadow.
reg32 carFar(App* app, CPU& cpu, reg32 car, reg32 view);
// The game's 0x4bab70: a car near: body, wheels, lights and shadow.
reg32 carNear(App* app, CPU& cpu, reg32 car, reg32 view);
// The game's 0x4bb110: traffic's car at its nearest: one part, lights, shadow.
reg32 carSimpleNear(App* app, CPU& cpu, reg32 car, reg32 view);
// The game's 0x4bb4c0: the car in the view at the detail its model has.
reg32 drawCar(App* app, CPU& cpu, reg32 car, reg32 view, reg32 detail);
// The game's 0x4bba10: how much of the budget the cars' details use.
reg32 detailInUse(App* app, CPU& cpu);
// The game's 0x4bba50: the farthest car but keep at detail brought to lowered; 0 when none.
reg32 lowerDetail(App* app, CPU& cpu, reg32 keep, reg32 detail, reg32 lowered);
// The game's 0x4bbad0: the cars' details cut to the setting's budget.
reg32 fitDetailBudget(App* app, CPU& cpu, reg32 camera, reg32 ecx);
// The game's 0x4bbba0: every car in the view, the replay's ghost and the particles.
reg32 cars(App* app, CPU& cpu, reg32 view);
// The game's 0x4bbd70: the arena back to its start.
reg32 arenaRewind(App* app, CPU& cpu);
// The game's 0x4bbd80: the arena made.
reg32 arenaCreate(App* app, CPU& cpu);
// The game's 0x4bbdc0: the arena freed.
reg32 arenaFree(App* app, CPU& cpu);
// The game's 0x4bbde0: bytes of the arena to *slot; 1, or 0 when it has run out.
reg32 arenaTake(App* app, CPU& cpu, reg32 slot, reg32 bytes);
// The game's 0x4bbe40: where the arena is now, to *slot; 1, or 0 when it has run out.
reg32 arenaHere(App* app, CPU& cpu, reg32 slot);
// The game's 0x4bbe70: *list an empty list.
reg32 emptyList(App* app, CPU& cpu, reg32 list);
// The game's 0x4bbe90: *list the list of the one record.
reg32 listOfOne(App* app, CPU& cpu, reg32 record, reg32 list);
// The game's 0x4bbec0: *list the two lists joined.
reg32 joinLists(App* app, CPU& cpu, reg32 list, reg32 firstHead, reg32 firstTail, reg32 secondHead, reg32 secondTail);
// The game's 0x4bbf20: the open list empty.
reg32 openListStart(App* app, CPU& cpu);
// The game's 0x4bbf40: the list put on the end of the open one.
reg32 openListAppend(App* app, CPU& cpu, reg32 head, reg32 tail);
reg32 openListAppendKeeping(App* app, CPU& cpu, reg32 head, reg32 tail, reg32 eax);
// The game's 0x4bbf80: bytes of the arena to *slot, on the end of the open list.
reg32 openListTake(App* app, CPU& cpu, reg32 slot, reg32 bytes);
// The game's 0x4bbfc0: the open list closed, to *list.
reg32 openListClose(App* app, CPU& cpu, reg32 list);
// The game's 0x4bc000: the mesh's vertices flushed and listed, to *out.
reg32 flushAndList(App* app, CPU& cpu, reg32 mesh, reg32 out);
// The game's 0x4bc030: the list drawn by sub_4349d0 in the view, and left empty.
reg32 drawListPlain(App* app, CPU& cpu, reg32 view, reg32 list);
// The game's 0x4bc060: the list drawn at the depth, and left empty.
reg32 drawListAt(App* app, CPU& cpu, reg32 view, reg32 list, reg32 depth);
// The game's 0x4bc0b0: the list drawn as a list of the depth, and left empty.
reg32 drawListMirrored(App* app, CPU& cpu, reg32 view, reg32 list, reg32 depth);
// The game's 0x4bf110: the table of reciprocals at 0x7d30d8.
reg32 fillReciprocals(App* app, CPU& cpu);
// The game's 0x4bf1b0: the clip's numbers, the vertex cache marked unused.
reg32 setClipBounds(App* app, CPU& cpu, reg32 left, reg32 top, reg32 right, reg32 bottom, reg32 nearest,
                    reg32 farthest);
// The game's 0x4bf260: the projection's scale and centre from the window and the fields of view.
reg32 setProjection(App* app, CPU& cpu, reg32 cornerXBits, reg32 cornerYBits, reg32 widthBits, reg32 heightBits,
                    reg32 fovX, reg32 fovY, reg32 mirrored);
// The game's 0x4bf370: count points projected to the vertices.
reg32 projectPoints(App* app, CPU& cpu, reg32 count, reg32 points, reg32 vertices);

}  // namespace rendd
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
