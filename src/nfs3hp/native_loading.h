#ifndef NFS3HP_NATIVE_LOADING_H
#define NFS3HP_NATIVE_LOADING_H

#include "native_guest.h"

/* native_loading.cpp's typed function that other files call as C++ (guest::direct), and how the
 * game's code calls it by address. */
namespace nfs3hp
{
namespace loader
{

// The game's 0x49cfa0: the FCE model in memory at `fce` turned into the game's polygon records; the FCE
// (nullopt when the game stopped).  Called with esp at its return address's slot.
std::optional<x86::reg32> fceModelReady(win32::WinApplication* app, x86::CPU& cpu, x86::reg32 fce);

}  // namespace loader

namespace guest
{

}  // namespace guest
}  // namespace nfs3hp

#endif
