#ifndef NFS3HP_DIAG_HOOKS_H
#define NFS3HP_DIAG_HOOKS_H

#include <lib/app.h>

/* What the game's rewritten functions tell the diagnostics, when something
 * listens (nfs3hp_main.cpp sets them as the traces ask).  Null, as they are for
 * a player, each costs a test and links nothing. */
namespace nfs3hp
{
namespace diag
{

// A car's physics stepped (physics::carPhysics), the car in `car`.
inline void (*carStepped)(nfs::App* app, nfs::u32 car) = nullptr;
// The race being taken down (graphics::takeDownRace).
inline void (*raceDown)(nfs::App* app) = nullptr;
// The menus ask once a frame whether to go straight to the last race's replay.
inline bool (*takeAutoReplay)() = nullptr;

}  // namespace diag
}  // namespace nfs3hp

#endif
