#ifndef NFS3HP_NATIVE_FRONTEND_FX_H
#define NFS3HP_NATIVE_FRONTEND_FX_H

#include "native_frontend.h"
#include "native_guest.h"
#include "native_watcom.h"

/* What the native functions of native_frontend_{dialogs,loada,loopa,loopb,sima,simb,simc,simd,sime,inpa}.cpp share
 * now that none of them touches the registers or the x87 of nfs::ThreadState: the game's single-precision arithmetic
 * (fp::, fpu.h) in the shape fe::X87 had it, and a call by address made through guest::pointerCall with
 * the arguments of Guest::call (eax, edx, ebx and ecx in turn, then the stack words; every other
 * register 0 -- the callees take no others). */
namespace nfs3hp
{
namespace fx
{

class Arith
{
public:
    explicit Arith(fe::Guest& guest) : m_guest(guest) {}

    // fild
    static nfs::Float integer(std::int32_t value) { return nfs::Float(nfs::s32(value)); }
    // fld dword
    nfs::Float load(nfs::u32 address) const { return nfs::Float(m_guest.floatAt(address)); }

    nfs::Float add(const nfs::Float& a, const nfs::Float& b) { return fp::add(a, b); }
    nfs::Float sub(const nfs::Float& a, const nfs::Float& b) { return fp::sub(a, b); }
    nfs::Float mul(const nfs::Float& a, const nfs::Float& b) { return fp::mul(a, b); }

    /* fcom a, b and what a jump after it reads: ja, and jb (true unordered). */
    bool above(const nfs::Float& a, const nfs::Float& b) { return (fp::compare(a, b) & 0x4100) == 0; }
    bool below(const nfs::Float& a, const nfs::Float& b) { return (fp::compare(a, b) & 0x0100) != 0; }

    /* sub_4dfd56 and fistp: `value` cut to an integer towards zero. */
    std::int32_t truncated(const nfs::Float& value)
    {
        const double chopped =
            fe::direct<0x4dfd56, watcom::clib::chop>(m_guest.app(), m_guest.ts(), double(value));
        return fp::toInteger<nfs::s32>(chopped);
    }

private:
    fe::Guest& m_guest;
};

/* The game's function at `address`, called as Guest::call called it; its eax.  `io`, when given, is the
 * rest of its Args: what goes in (esi, the float arguments floatIn/floatsIn) and what comes back (edx,
 * ebx, ecx, esi, edi, floatSet/floatResult, floatsTaken); its eax, edx, ebx and ecx are set from
 * `arguments`.  Throws fe::Terminated when the game stopped in it. */
inline nfs::u32 callAt(fe::Guest& guest, nfs::u32 address, const fe::Arguments& arguments = fe::Arguments(),
                         nfs::Args* io = nullptr)
{
    nfs::Args own;
    nfs::Args& a = io ? *io : own;
    a.eax = arguments.registers[0];
    a.edx = arguments.registers[1];
    a.ebx = arguments.registers[2];
    a.ecx = arguments.registers[3];
    nfs::ThreadState& ts = guest.ts();
    const nfs::u32 esp = ts.sp;
    for (std::size_t i = arguments.stackCount; i-- > 0;)
    {
        ts.sp -= 4;
        guest.setDword(ts.sp, arguments.stack[i]);
    }
    const std::optional<nfs::u32> eax = guest::pointerCall(guest.app(), ts, address, a);
    if (!eax)
        throw fe::Terminated();
    ts.sp = esp;
    return *eax;
}

}  // namespace fx
}  // namespace nfs3hp

#endif
