#include "native_guest.h"

#ifdef __ANDROID__
#include <SDL3/SDL.h>
#endif

#include <cstdio>
#include <cstdlib>

/* The game's graphics code that never runs on the port (native_graphics_dead.inc):
 * each address a stub that says so and stops the game, rather than running
 * code no one has checked. */
namespace nfs3hp
{
namespace graphics
{

namespace
{

[[noreturn]] void deadCode(x86::reg32 address)
{
#ifdef __ANDROID__
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "[GFX] dead code called: sub_%x (native_graphics_dead.inc)", address);
#else
    std::fprintf(stderr, "[GFX] dead code called: sub_%x (native_graphics_dead.inc)\n", address);
#endif
    std::abort();
}

}  // namespace

#define DEAD(address) \
    x86::reg32 dead_##address(win32::WinApplication*, x86::CPU&, x86::Args&) { deadCode(address); }
#include "native_graphics_dead.inc"
#undef DEAD

}  // namespace graphics
}  // namespace nfs3hp
