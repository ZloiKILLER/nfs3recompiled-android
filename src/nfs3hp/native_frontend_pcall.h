#ifndef NFS3HP_NATIVE_FRONTEND_PCALL_H
#define NFS3HP_NATIVE_FRONTEND_PCALL_H

#include "native_frontend.h"
#include "native_guest.h"

#include <optional>

/* Guest::call through guest::pointerCall (plan step 7, round F): the menus' calls through a pointer the game's data
 * holds, a callback, or a function with no typed core.  `arguments` as Guest::call has them -- eax, edx, ebx and ecx
 * in turn, the rest on the stack -- and the registers it leaves out from `registers` (what the code holds in them at
 * the call; zeros when it holds none), what the callee leaves in them left there, eax the result.  Terminated
 * thrown when the game stopped in it. */
namespace nfs3hp
{
namespace fe
{

inline nfs::u32 pointerCall(Guest& guest, nfs::u32 target, const Arguments& a, nfs::Args& regs)
{
    nfs::u32* const into[4] = {&regs.eax, &regs.edx, &regs.ebx, &regs.ecx};
    for (std::size_t i = 0; i < a.registerCount; ++i)
        *into[i] = a.registers[i];
    std::optional<nfs::u32> r;
    switch (a.stackCount)
    {
    case 0: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {}); break;
    case 1: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0]}); break;
    case 2: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1]}); break;
    case 3: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1], a.stack[2]}); break;
    case 4: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1], a.stack[2], a.stack[3]}); break;
    case 5: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1], a.stack[2], a.stack[3], a.stack[4]}); break;
    case 6: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1], a.stack[2], a.stack[3], a.stack[4], a.stack[5]}); break;
    case 7: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1], a.stack[2], a.stack[3], a.stack[4], a.stack[5], a.stack[6]}); break;
    case 8: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1], a.stack[2], a.stack[3], a.stack[4], a.stack[5], a.stack[6], a.stack[7]}); break;
    case 9: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1], a.stack[2], a.stack[3], a.stack[4], a.stack[5], a.stack[6], a.stack[7], a.stack[8]}); break;
    case 10: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1], a.stack[2], a.stack[3], a.stack[4], a.stack[5], a.stack[6], a.stack[7], a.stack[8], a.stack[9]}); break;
    case 11: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1], a.stack[2], a.stack[3], a.stack[4], a.stack[5], a.stack[6], a.stack[7], a.stack[8], a.stack[9], a.stack[10]}); break;
    case 12: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1], a.stack[2], a.stack[3], a.stack[4], a.stack[5], a.stack[6], a.stack[7], a.stack[8], a.stack[9], a.stack[10], a.stack[11]}); break;
    case 13: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1], a.stack[2], a.stack[3], a.stack[4], a.stack[5], a.stack[6], a.stack[7], a.stack[8], a.stack[9], a.stack[10], a.stack[11], a.stack[12]}); break;
    case 14: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1], a.stack[2], a.stack[3], a.stack[4], a.stack[5], a.stack[6], a.stack[7], a.stack[8], a.stack[9], a.stack[10], a.stack[11], a.stack[12], a.stack[13]}); break;
    case 15: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1], a.stack[2], a.stack[3], a.stack[4], a.stack[5], a.stack[6], a.stack[7], a.stack[8], a.stack[9], a.stack[10], a.stack[11], a.stack[12], a.stack[13], a.stack[14]}); break;
    case 16: r = guest::pointerCall(guest.app(), guest.ts(), target, regs, {a.stack[0], a.stack[1], a.stack[2], a.stack[3], a.stack[4], a.stack[5], a.stack[6], a.stack[7], a.stack[8], a.stack[9], a.stack[10], a.stack[11], a.stack[12], a.stack[13], a.stack[14], a.stack[15]}); break;
    default:
        throw Terminated();
    }
    if (!r)
        throw Terminated();
    regs.eax = *r;
    return *r;
}

inline nfs::u32 pointerCall(Guest& guest, nfs::u32 target, const Arguments& a = Arguments())
{
    nfs::Args regs;
    return pointerCall(guest, target, a, regs);
}

}  // namespace fe
}  // namespace nfs3hp

#endif
