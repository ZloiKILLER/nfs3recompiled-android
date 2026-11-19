#ifndef NFS3HP_PORT_GAMECALL_H
#define NFS3HP_PORT_GAMECALL_H

#include "native_guest.h"
#include "port.h"

/* A call into the game's code by address as Watcom makes it -- the register arguments of `a` (eax, edx,
 * ebx, ecx), `stack` pushed, the return address's slot under them, esp as given before and after --
 * through guest::pointerCall, which finds the callee's thunk.  Its eax in `result`; false when the game
 * stopped in it. */
namespace nfs3hp
{
namespace port
{

inline bool gameCall(App* app, ThreadState& ts, u32 esp, u32 target, Args a, std::initializer_list<u32> stack,
                     u32& result)
{
    ts.sp = esp;
    nfs::Args registers;
    registers.eax = a.eax;
    registers.edx = a.edx;
    registers.ebx = a.ebx;
    registers.ecx = a.ecx;
    const std::optional<u32> eax = guest::pointerCall(app, ts, target, registers, stack);
    if (!eax)
        return false;
    result = *eax;
    return true;
}

inline bool gameCall(App* app, ThreadState& ts, u32 esp, u32 target, Args a = {},
                     std::initializer_list<u32> stack = {})
{
    u32 ignored;
    return gameCall(app, ts, esp, target, a, stack, ignored);
}

}  // namespace port
}  // namespace nfs3hp

#endif
