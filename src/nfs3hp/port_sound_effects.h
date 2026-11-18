#ifndef NFS3HP_PORT_SOUND_EFFECTS_H
#define NFS3HP_PORT_SOUND_EFFECTS_H

#include "native_guest.h"
#include "port.h"
#include "port_sound_calls.h"

/* port_sound_effects.cpp's typed cores that the menus call as C++ (guest::direct), and how the game's code
 * calls each by address; bankEffect is declared in port_snd.h as
 * well.  A Result is nullopt when the game stopped in a call.  A core's
 * `esp` is the frame base its thunk hands it: the game's esp on entry, at the return address.  A caller
 * through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp the
 * thunk hands it, so the core's frame lies where the game's does; by address the call makes it. */
namespace nfs3hp
{
namespace port
{

// The game's 0x4181d0: menu effect `effect` of the first bank at `volume`: SNDplay's handle (`effect` with the
// sounds off); esp the frame base.
Result playMenuEffectOf(App* app, CPU& cpu, reg32 esp, int32_t effect, int32_t volume);
// The game's 0x4182b0: effect `effect` of bank `bank` at `volume`, pan `pan`, pitch `pitch`: the sound's handle, -1
// with the effects off; esp the frame base.
Result bankEffect(App* app, CPU& cpu, reg32 esp, int32_t bank, int32_t effect, int32_t volume, uint32_t pan,
                  uint8_t pitch);
// The game's 0x4184f0: the same at `volume` scaled by the speech's; esp the frame base.
Result speechEffect(App* app, CPU& cpu, reg32 esp, int32_t bank, int32_t effect, int32_t volume, uint32_t pan,
                    uint8_t pitch);
// The game's 0x418280: effect 5 of the car's bank at full volume, in the middle; esp the frame base.
Result carEffectFive(App* app, CPU& cpu, reg32 esp);
// The game's 0x418770 and 0x418790: the effect bus's level up (127) and down (0); esp the frame base.
Result busLevelUp(App* app, CPU& cpu, reg32 esp);
Result busLevelDown(App* app, CPU& cpu, reg32 esp);
// The game's 0x416f60: slot `slot`'s sound faded out and the slot empty: the effects playing (else `slot`); esp the
// frame base.
Result slotStopped(App* app, CPU& cpu, reg32 esp, int32_t slot);
// The game's 0x417080: slot `slot` plays `effect` at `volume`, `pitch`, `pan`: the last call's answer; esp the frame
// base.
Result slotPlayed(App* app, CPU& cpu, reg32 esp, int32_t slot, int32_t effect, int32_t volume, int32_t pitch,
                  reg32 pan);
// The game's 0x416fc0: slot `slot` plays `effect`, pitch `ecx` times `ebx` (16.16), `volume`, `pan`, for `player`;
// esp the frame base.
Result slotEffectPlayed(App* app, CPU& cpu, reg32 esp, int32_t slot, int32_t effect, reg32 ecx, reg32 ebx,
                        reg32 volume, reg32 pan, reg32 player);
// The game's 0x412620 (port_sound_3d.cpp): the place at `place` (ints) from the listener at `listener`: its distance;
// across each axis over that (16.16, -1..1) written to `across` and `across` + 4; esp the frame base.
Result placeAcrossOf(App* app, CPU& cpu, reg32 esp, reg32 listener, reg32 place, reg32 across);
// The game's 0x4185e0: side angle `angle` (0..127; `side` above 0 for the right) as a pan (0..0xffff).
reg32 panFromSideOf(App* app, int32_t angle, int32_t side);
// The game's 0x418740: the effect bus set to type 0x14, its level 0; esp the frame base.
Result busInitOf(App* app, CPU& cpu, reg32 esp);
// The game's 0x4e8f28 (SNDangle): the angle of the vector (`along`, `across`), 0x400 a turn.
reg32 vectorAngle(App* app, int32_t along, int32_t across);

}  // namespace port

}  // namespace nfs3hp

#endif
