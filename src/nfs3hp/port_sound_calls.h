#ifndef NFS3HP_PORT_SOUND_CALLS_H
#define NFS3HP_PORT_SOUND_CALLS_H

#include "native_guest.h"
#include "port.h"

#include <cstdint>
#include <initializer_list>

/* The game's sound rewrites (port_sound_*.cpp) that the others call as C++ (guest::direct), and how the game's
 * code calls each by address.  A Result is
 * nullopt when the game stopped in a call.  A core's `esp` is the frame base its thunk hands it: the game's esp
 * on entry, at the return address; a caller through guest::direct sets cpu.esp where the game's code has it at
 * the call and passes that less 4 and 4 for each word the call pushes. */
namespace nfs3hp
{
namespace port
{

/* A call into the game's code (what has no typed function) through its address, as Watcom makes it: esp as
 * the code has it, the register arguments `a`, `stack` pushed (guest::pointerCall: the callee's thunk).  Its eax
 * in `result`; false when the game stops. */
inline bool pointerGame(App* app, CPU& cpu, reg32 esp, reg32 target, Args a, std::initializer_list<reg32> stack,
                        reg32& result)
{
    cpu.esp = esp;
    x86::Args registers;
    registers.eax = a.eax;
    registers.edx = a.edx;
    registers.ebx = a.ebx;
    registers.ecx = a.ecx;
    registers.ebp = heldEbp();  // what HeldEbp holds for the code the call reaches (it reads it, uninitialised)
    const std::optional<reg32> eax = guest::pointerCall(app, cpu, target, registers, stack);
    if (!eax)
        return false;
    result = *eax;
    return true;
}

inline bool pointerGame(App* app, CPU& cpu, reg32 esp, reg32 target, Args a = {},
                        std::initializer_list<reg32> stack = {})
{
    reg32 ignored;
    return pointerGame(app, cpu, esp, target, a, stack, ignored);
}


namespace world
{

/* The arithmetic the race's sound does its floats in: the x87's precision
 * control as the game's code finds it (24 bits in a race), else double.  An
 * thunk takes it from the x87's rounding and hands it on. */
enum class Arithmetic
{
    Double,
    Single
};

inline Arithmetic arithmeticOf(x86::reg8 rounding)
{
    return rounding == x86::FloatEnv::kRoundNone ? Arithmetic::Double : Arithmetic::Single;
}

// The game's 0x410cb0: the distance of the place at `to` from the one at `from`, in arithmetic `a`.
Result distanceAt(App* app, Arithmetic a, reg32 from, reg32 to);
// The game's 0x410d30: the angle of the place at `place` from the listener at `listener`; esp the frame base.
Result angleAt(App* app, CPU& cpu, reg32 esp, Arithmetic a, reg32 listener, reg32 place);
// The game's 0x410dc0: the angle of car `car` (its direction at +0x13c) from the listener at `listener`; esp the
// frame base.
Result carAngleAt(App* app, CPU& cpu, reg32 esp, Arithmetic a, reg32 listener, reg32 car);
// The game's 0x4124d0: how far the place at `place` is ahead along the track (`direction` its direction); esp the
// frame base.
Result aheadAt(App* app, CPU& cpu, reg32 esp, Arithmetic a, reg32 place, reg32 direction);

}  // namespace world

namespace sources
{

// The game's 0x416d90: a car's knock in slot `slot` of kind `kind` on material `material` at force `force`,
// `distance` and `pan` away; esp the frame base.
Result knockPlayed(App* app, CPU& cpu, reg32 esp, int32_t slot, reg32 kind, reg32 material, int32_t force,
                   int32_t distance, reg32 pan);

}  // namespace sources

// The game's 0x410b50: bank file `file` loaded into slot `slot` (`flags` for the load) and added to SND: its
// handle; `eax` what eax holds at the call (the file); esp the frame base.
Result bankLoaded(App* app, CPU& cpu, reg32 esp, reg32 file, int32_t slot, reg32 flags, reg32 eax);
// The game's 0x418930: player `player`'s beeps started (`count` of them, the first high): the beep's handle; esp
// the frame base.
Result beepsStarted(App* app, CPU& cpu, reg32 esp, int32_t count, int32_t player);
// The game's 0x4127c0: car `car`'s radio, heard from the car's place; the radio's answer; esp the frame base.
Result carRadioCalled(App* app, CPU& cpu, reg32 esp, reg32 car);
// The game's 0x412880: the cop radio of `kind` for car `car`'s player; esp the frame base.
Result playerCopRadioCalled(App* app, CPU& cpu, reg32 esp, reg32 car, int32_t kind);
// The game's 0x418c30: effect `effect` of the cops' bank at full volume: the sound's handle; esp the frame base.
Result copEffectPlayed(App* app, CPU& cpu, reg32 esp, int32_t effect);
// The game's 0x418c60: the music's event back after an interruption (`edx` handed on to the interruption's end):
// eax as the code leaves it; esp the frame base.
Result musicEventResumed(App* app, CPU& cpu, reg32 esp, reg32 eax, reg32 edx);
// The game's 0x4c8370: the speech set up for the race: the script, as loaded; esp the frame base.
Result speechSetUp(App* app, CPU& cpu, reg32 esp);
// The game's 0x4c8440: the co-driver's script freed (`eax` back with none); esp the frame base.
Result speechScriptFreed(App* app, CPU& cpu, reg32 esp, reg32 eax);
// The game's 0x4c8d10: player `player`'s co-driver; esp the frame base.
Result coDriverTicked(App* app, CPU& cpu, reg32 esp, int32_t player);
// The game's 0x4c8810: the radio's sound of `player` ticked, `speaker` at `distance`, `pitch`, `pan`; esp the
// frame base.
Result radioSoundTicked(App* app, CPU& cpu, reg32 esp, reg32 speaker, reg32 pan, int32_t pitch, int32_t distance,
                        int32_t player);
// The game's 0x414830: the engines' sounds as the race starts (`eax` as the code has it); esp the frame base.
Result engineSoundsStarted(App* app, CPU& cpu, reg32 esp, reg32 eax);
// The game's 0x414f60: the far-off effect's tick (`eax` as the code has it); esp the frame base.
Result farEffectTicked(App* app, CPU& cpu, reg32 esp, reg32 eax);
// The game's 0x4129c0: the race's sound set up (`eax` as the code has it); esp the frame base.
Result raceSoundsSetUp(App* app, CPU& cpu, reg32 esp, reg32 eax);
// The game's 0x4121b0: cop car `car`'s sounds stopped (its slot 4 on too, unless `keep`), its source reset for car
// `keep`; esp the frame base.
Result copRemoved(App* app, CPU& cpu, reg32 esp, reg32 car, reg32 keep);
// The game's 0x411620: the source at `source` reset, for car `car` (0: none): the source.
reg32 sourceResetTo(App* app, reg32 source, reg32 car);
// The game's 0x411c50: cop car `siren`'s siren at distance `distance`, pan `pan`, pitch `pitch`; esp the frame base.
Result copSirenPlayed(App* app, CPU& cpu, reg32 esp, int32_t siren, int32_t distance, reg32 pan, int32_t pitch);
// The game's 0x411ca0: siren `siren` stopped; esp the frame base.
Result copSirenStopped(App* app, CPU& cpu, reg32 esp, int32_t siren);
// The game's 0x411cb0: cop car `cop`'s engine at distance `distance`, pitch `pitch`, pan `pan`; esp the frame base.
Result copEnginePlayed(App* app, CPU& cpu, reg32 esp, int32_t cop, int32_t distance, int32_t pitch, reg32 pan);
// The game's 0x411da0: cop car `cop`'s engine stopped; esp the frame base.
Result copEngineStopped(App* app, CPU& cpu, reg32 esp, int32_t cop);
// The game's 0x411db0: cop car `cop`'s distance from the listener, squared, in arithmetic `a`; esp the frame base.
Result copDistanceSquared(App* app, CPU& cpu, reg32 esp, world::Arithmetic a, int32_t cop);
// The game's 0x411e20: how close cop cars `from` on (`count` of them) are, all told, in arithmetic `a`; esp the
// frame base.
Result copsClosenessTold(App* app, CPU& cpu, reg32 esp, world::Arithmetic a, int32_t count, int32_t from);
// The game's 0x415050: car `car`'s sound of the track's zones `zones` in slot `slot`, offset `offset`; esp the
// frame base.
Result zoneSoundPlayed(App* app, CPU& cpu, reg32 esp, int32_t slot, reg32 car, reg32 zones, reg32 offset);
// The game's 0x417c00: the player's noise, at `distance`, pitch `pitch`, pan `pan`; esp the frame base.
Result playerNoisePlayed(App* app, CPU& cpu, reg32 esp, reg32 player, int32_t distance, reg32 pitch, reg32 pan);
// The game's 0x417c70: the player's noise stopped; esp the frame base.
Result playerNoiseStopped(App* app, CPU& cpu, reg32 esp, reg32 player);
// The game's 0x417b80: the noise of slot `slot`, effect `effect`, material `material`, force `force`, as loud as
// `distance` leaves it, pan `pan`; esp the frame base.
Result noiseByDistancePlayed(App* app, CPU& cpu, reg32 esp, int32_t slot, reg32 effect, reg32 material, reg32 force,
                             int32_t distance, reg32 pan);
// The game's 0x4138e0: player `player`'s car bank of the archive named at `archiveName`; esp the frame base.
Result carBankLoaded(App* app, CPU& cpu, reg32 esp, reg32 archiveName, int32_t player);
// The game's 0x4188a0: player `player`'s cop beeps ticked; esp the frame base.
Result copBeepsTicked(App* app, CPU& cpu, reg32 esp, reg32 player);
// The game's 0x412d60: car `car`'s announcer and music tick; esp the frame base.
Result raceAnnouncerTicked(App* app, CPU& cpu, reg32 esp, reg32 car);
// The game's 0x411680: the sources reset as a race starts (the players', each with its car, and the cop cars'); 0.
Result sourcesResetOf(App* app);
// The game's 0x411750: the cop car of `car`, not chasing yet, set chasing (`chase`, its `target`, `distance` kept
// minus): the end of the cop cars' table.
reg32 copChaseSetOf(App* app, reg32 car, reg32 target, int32_t chase, int32_t distance);
// The game's 0x414f90: effect `effect` of the crowd's bank (below 0: one of 4 at random, at a random volume), at a
// random pan -- the player's side in the split screen, `side` (0 left); esp the frame base.
Result crowdEffectOf(App* app, CPU& cpu, reg32 esp, reg32 side, int32_t effect);
// The game's 0x4129b0: nothing: `eax` as it came.
reg32 eaxKept(App* app, reg32 eax);
// The game's 0x415210: car `car`'s scenery sounds, `heading` its heading, in arithmetic `a`; esp the frame base.
Result scenerySoundsOf(App* app, CPU& cpu, reg32 esp, world::Arithmetic a, reg32 car, reg32 heading);
// The game's 0x4117a0: player `player`'s car's sounds, in arithmetic `a`; esp the frame base.
Result playerCarSounded(App* app, CPU& cpu, reg32 esp, world::Arithmetic a, reg32 player);
// The game's 0x4179d0: slot `slot`'s noise of `kind` at pitch `ecx` times `ebx`, `distance`, `pan`, `speed`; esp
// the frame base.
Result noiseBySpeedPlayed(App* app, CPU& cpu, reg32 esp, int32_t slot, reg32 kind, reg32 ecx, reg32 ebx,
                          int32_t distance, reg32 pan, int32_t speed);

}  // namespace port

}  // namespace nfs3hp

#endif
