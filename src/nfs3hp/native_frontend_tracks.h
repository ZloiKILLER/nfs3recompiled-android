#ifndef NFS3HP_NATIVE_FRONTEND_TRACKS_H
#define NFS3HP_NATIVE_FRONTEND_TRACKS_H

#include "native_frontend.h"
#include "native_guest.h"

/* native_frontend_tracks.cpp's typed functions (the track screens: location, trkinfo), which other
 * files call as C++ (guest::direct), and how the game's code calls each by address.  Each calls the game's other functions by address on `cpu` and throws
 * fe::Terminated when the game stopped in one, as a call by address does; its result is what the game's
 * function leaves in eax. */
namespace nfs3hp
{
namespace frontend
{
namespace tracks
{

using App = win32::WinApplication;
using CPU = x86::CPU;
using reg32 = x86::reg32;

// The game's 0x4487e0: location as it opens (menu, the screen), everything greyed out in a series.
reg32 locationEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x4487d0: location's every pass, the developer's recorder.
reg32 locationFrame(App* app, CPU& cpu);
// The game's 0x456200: the track's places on its map.
reg32 placeTrackPlaces(App* app, CPU& cpu, reg32 eax);
// The game's 0x4568d0: trkinfo as it opens (menu, the screen): art, mask, places.
reg32 infoEnter(App* app, CPU& cpu, reg32 menu);
// The game's 0x456890: trkinfo's every pass, the description spoken once.
reg32 infoFrame(App* app, CPU& cpu);
// The game's 0x456990: trkinfo as it closes, the description stopped.
reg32 infoExit(App* app, CPU& cpu);
// The game's 0x4c94e0: the track's description spoken, in language.
reg32 speak(App* app, CPU& cpu, reg32 language, reg32 track);
// The game's 0x4c9590: the description stopped, if it is spoken.
reg32 stopSpeaking(App* app, CPU& cpu, reg32 eax);

}  // namespace tracks
}  // namespace frontend

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
