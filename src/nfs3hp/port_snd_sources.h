#ifndef NFS3HP_PORT_SND_SOURCES_H
#define NFS3HP_PORT_SND_SOURCES_H

#include "port_snd.h"

#include <initializer_list>

/* What the race's sound sources share (port_sound_cops.cpp, port_sound_crash.cpp,
 * port_sound_siren.cpp; port.h says how the port is written): the sirens that
 * the cops' sounds start, tick and stop, and the knock of a car that the
 * noises by distance play.  Each is here as C++ with arguments and a result --
 * the cores the files call each other with -- and the game's entries
 * (thunks) are in the files themselves; what they call of the effects
 * (port_sound_effects.cpp) they call as C++.
 *
 * Every core that calls the game's code takes `esp`, the one its game's code
 * had on entry (at the return address), as port.h has it. */
namespace nfs3hp
{
namespace port
{
namespace sources
{

/* A quotient as the cdq; idiv of the game's code gives it: toward 0, the
 * divisor 0 (which the x86 traps and the arm64 answers) giving 0. */
inline int32_t quotient(int32_t a, int32_t b) { return b == 0 ? 0 : idiv(a, b); }

/* The code's signed shift of `value` right by `k` toward 0 (the sar after the
 * add of the sign): a division by 2^k. */
inline int32_t shiftedToZero(int32_t value, unsigned k)
{
    return value < 0 ? int32_t(uint32_t(value) + ((1u << k) - 1)) >> k : value >> k;
}

/* Whether siren `siren` is on (the game keeps a word for each). */
int32_t sirenActive(App* app, int32_t siren);

/* Siren `siren` started if it is not on (steady, or warbling between its two
 * tones for random whiles); the siren's number as eax when it was on. */
Result sirenSwitchedOn(App* app, CPU& cpu, reg32 esp, int32_t siren, bool steady);

/* Siren `siren` ticked: its tones turned when their ticks are up, its volume,
 * pitch (16.10) and pan set. */
Result sirenTicked(App* app, CPU& cpu, reg32 esp, int32_t siren, int32_t volume, int32_t pitch, reg32 pan);

/* Siren `siren` off, faded out, its state as it starts. */
Result sirenSwitchedOff(App* app, CPU& cpu, reg32 esp, int32_t siren);

/* A car's knock in slot `slot` (below 0: a crash) of kind `kind` on material
 * `material` at force `force`, `distance` and `pan` away: the crash's effect
 * picked and played in slot 0x1f, or the scrape played in the slot as loud and
 * high as the force says, or the slot stopped. */
Result knockPlayed(App* app, CPU& cpu, reg32 esp, int32_t slot, reg32 kind, reg32 material, int32_t force,
                   int32_t distance, reg32 pan);

}  // namespace sources
}  // namespace port
}  // namespace nfs3hp

#endif
