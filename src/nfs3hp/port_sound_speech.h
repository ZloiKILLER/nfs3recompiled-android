#ifndef NFS3HP_PORT_SOUND_SPEECH_H
#define NFS3HP_PORT_SOUND_SPEECH_H

#include "native_guest.h"
#include "port.h"

/* port_sound_speech.cpp's typed cores that the menus call as C++ (guest::direct), and how the game's code calls
 * each by address.  A Result is nullopt when the game stopped in a
 * call.  A core's `esp` is the frame base its thunk hands it: the game's esp on entry, at the return address.
 * A caller through guest::direct passes cpu.esp - 4 - 4 * N, N the stack words the game's call pushes: the esp
 * the thunk hands it, so the core's frame lies where the game's does; by address the call makes it. */
namespace nfs3hp
{
namespace port
{

// The game's 0x4c9490: the description, if it is spoken, stopped (`eax` back with nothing to stop); esp the frame
// base.
Result descriptionStopped(App* app, CPU& cpu, reg32 esp, reg32 eax);
// The game's 0x4c95f0: a description spoken -- track `track` of kind `kind` (0x32 the menus', 0x34 the cars', 0x33 a
// part of the one loaded) in language `language`; esp the frame base.
Result descriptionStarted(App* app, CPU& cpu, reg32 esp, reg32 track, reg32 kind, reg32 language);
// The game's 0x4c9860: the system tasks run, then the description's tick; esp the frame base.
Result descriptionServed(App* app, CPU& cpu, reg32 esp);
// The game's 0x4c9890: the race voice bank `index` loaded; esp the frame base.
Result raceVoiceBankLoaded(App* app, CPU& cpu, reg32 esp, reg32 index);
// The game's 0x4c9990: effect `group` * 16 + `index` of the car or the scenery bank spoken: its handle; esp the frame
// base.
Result carLineSpoken(App* app, CPU& cpu, reg32 esp, reg32 index, reg32 group);
// The game's 0x4c99d0: the spoken line faded out; esp the frame base.
Result spokenStopOf(App* app, CPU& cpu, reg32 esp);
// The game's 0x4c99f0 (and 0x4c95e0): the spoken line's status; esp the frame base.
Result spokenStatusOf(App* app, CPU& cpu, reg32 esp);
// The game's 0x4c9a00: the race voice banks removed and their header copies freed; esp the frame base.
Result raceVoiceBanksFreed(App* app, CPU& cpu, reg32 esp);

}  // namespace port

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
