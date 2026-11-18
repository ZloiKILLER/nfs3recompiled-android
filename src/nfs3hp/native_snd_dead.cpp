#include "native_snd.h"

#ifdef __ANDROID__
#include <SDL3/SDL.h>
#endif

#include <cstdio>
#include <cstdlib>

/* The game's sound code that never runs on the port (native_snd_dead.inc):
 * each address a stub that says so and stops the game, rather than running
 * code no one has checked. */
namespace nfs3hp
{
namespace snd
{

namespace
{

[[noreturn]] void deadCode(reg32 address)
{
#ifdef __ANDROID__
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "[SND] dead code called: sub_%x (native_snd_dead.inc)", address);
#else
    std::fprintf(stderr, "[SND] dead code called: sub_%x (native_snd_dead.inc)\n", address);
#endif
    std::abort();
}

}  // namespace

#define DEAD(address) \
    x86::reg32 dead_##address(App*, CPU&, x86::Args&) { deadCode(address); return 0; }
#include "native_snd_dead.inc"
#undef DEAD

}  // namespace snd
}  // namespace nfs3hp
