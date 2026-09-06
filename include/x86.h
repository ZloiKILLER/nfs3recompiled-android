#ifndef X86_H_
#define X86_H_

#include    <cstdint>
#include    <cstdlib>

#ifdef _MSC_VER
# define __restrict__ __restrict
#endif

#ifdef NFS2_ASSERT_TRAP
/* Opt-in hard trap for local debugging (-DNFS2_ASSERT_TRAP=ON); see the
 * #else branch for why this is not the default. */
# ifdef _MSC_VER
#  define NFS2_ASSERT(x)         \
    if (! (x)) __debugbreak()
# elif defined(__i386__) || defined(__x86_64__)
#  define NFS2_ASSERT(x)         \
    if (! (x)) asm("int3")
# else
/* int3 is an x86 instruction and will not assemble for ARM.  __builtin_trap()
 * is the portable equivalent: it aborts the process at the faulting site, which
 * on Android surfaces as a SIGILL/SIGTRAP with a usable backtrace in logcat. */
#  define NFS2_ASSERT(x)         \
    if (! (x)) __builtin_trap()
# endif
#else
/* Many of these guard functionality that is genuinely stubbed out (SetCursorPos
 * before mouse support existed, a GetSystemMetrics index nothing implements, an
 * unexpected format in a texture upload, ...) and the original game reaches
 * them as a matter of course, not as a sign of a corrupted CPU/memory state.
 * A hard trap there is fine under a desktop debugger; on a user's phone it
 * just kills the app for a path that was only ever going to be exercised by
 * this one caller.  Log once -- so a genuine logic bug is still visible, just
 * not fatal -- and fall through to whatever the surrounding function does
 * next, usually returning a default/zeroed value: the same thing the trap
 * used to prevent from ever running. */
# define NFS2_ASSERT(x)         \
    if (! (x)) ::x86::assertLog(__FILE__, __LINE__, #x)
#endif
#define NFS2_USE(x)             \
    (void)x


namespace x86
{

/* Implemented in src/lib/x86.cpp: logs through SDL_Log so it lands wherever
 * the rest of the port's logging goes (logcat on Android, console on
 * desktop) without pulling <SDL3/SDL.h> into every one of the ~85 MB of
 * generated disassembly translation units that include this header. */
void assertLog(const char* file, int line, const char* expr);

typedef uint8_t reg8;
typedef uint16_t reg16;
typedef uint32_t reg32;
typedef uint64_t reg64;

typedef int8_t sreg8;
typedef int16_t sreg16;
typedef int32_t sreg32;
typedef int64_t sreg64;

struct CPU;
struct FPU;

}

#if defined(_MSC_VER) && !defined(__clang__)
# include <intrin.h>
static inline x86::reg32 __builtin_clz(x86::reg32 value)
{
    unsigned long result;
    _BitScanReverse(&result, value);
    return 31 - result;
}

static inline x86::reg32 __builtin_ctz(x86::reg32 value)
{
    unsigned long result;
    _BitScanForward(&result, value);
    return result;
}
#endif

#endif /* !X86_H_ */
