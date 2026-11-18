#include "native_guest.h"

#ifdef __ANDROID__
#include <SDL3/SDL.h>
#endif

#include <cstdio>
#include <cstdlib>

/* The game's code that nothing can reach (native_dead.inc, proved by
 * tools/audit_dead.py): each address a stub that says so and stops the game,
 * rather than running code no one has checked. */
namespace nfs3hp
{
namespace dead
{

namespace
{

[[noreturn]] void deadCode(x86::reg32 address)
{
#ifdef __ANDROID__
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "[DEAD] dead code called: sub_%x (native_dead.inc)", address);
#else
    std::fprintf(stderr, "[DEAD] dead code called: sub_%x (native_dead.inc)\n", address);
#endif
    std::abort();
}

}  // namespace

#define DEAD(address) \
    x86::reg32 stub_##address(win32::WinApplication*, x86::CPU&, x86::Args&) { deadCode(address); return 0; }
#include "native_dead.inc"
#undef DEAD

}  // namespace dead
}  // namespace nfs3hp
