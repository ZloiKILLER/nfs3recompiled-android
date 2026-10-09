#include "native_ai.h"
#include <nfs3hp.h>
#include <SDL3/SDL.h>

/* The opponents' driving, decompiled (native_ai.h), entered from the top of
 * the generated sub_4064f0 and sub_4070c0 (tools/apply_native_ai.py).  They
 * come out as the generated code does to the bit, in every precision the race
 * runs at (tools/native_ai_checks.py), so they change nothing the game does:
 * they are the two functions written to be read -- and to be changed, the
 * opponents' pull and steering being here as C++.  NFS_NATIVE_AI=0 leaves the
 * generated code to run instead. */
namespace nfs3hp
{

namespace
{

bool aiNativesOn()
{
    static const bool on = []() {
        const char* value = SDL_getenv("NFS_NATIVE_AI");
        return !(value && SDL_strcmp(value, "0") == 0);
    }();
    return on;
}

}  // namespace

bool aiPull(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!aiNativesOn())
        return false;
    ai::aiPull(app, cpu);
    return true;
}

bool aiHeading(win32::WinApplication* app, x86::CPU& cpu)
{
    if (!aiNativesOn())
        return false;
    ai::aiHeading(app, cpu);
    return true;
}

}  // namespace nfs3hp
