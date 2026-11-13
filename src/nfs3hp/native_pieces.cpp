#include "native_guest.h"

#ifdef __ANDROID__
#include <SDL3/SDL.h>
#endif

#include <cstdio>
#include <cstdlib>

/* Addresses the disassembly takes for functions and are none (native_pieces.inc):
 * data in the code, and pieces of functions native whole.  Each a stub that says
 * so and stops the game. */
namespace nfs3hp
{
namespace pieces
{

namespace
{

[[noreturn]] void notAFunction(x86::reg32 address)
{
#ifdef __ANDROID__
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "[DEAD] no function at sub_%x (native_pieces.inc)", address);
#else
    std::fprintf(stderr, "[DEAD] no function at sub_%x (native_pieces.inc)\n", address);
#endif
    std::abort();
}

}  // namespace

#define DEAD(address) \
    void stub_##address(win32::WinApplication*, x86::CPU&) { notAFunction(address); }
#include "native_pieces.inc"
#undef DEAD

}  // namespace pieces
}  // namespace nfs3hp
