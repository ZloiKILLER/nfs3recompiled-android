#ifndef CPU_H_
#define CPU_H_

#include <x86.h>
#include <fpu.h>
#include <mmx.h>

#define REGISTER_A(n)       \
union                       \
{                           \
    x86::reg32      e##n##x;\
    x86::reg16      n##x;   \
    struct                  \
    {                       \
        x86::reg8   n##l;   \
        x86::reg8   n##h;   \
    };                      \
}


#define REGISTER_P(n)       \
union                       \
{                           \
    x86::reg32  e##n;       \
    x86::reg16  n;          \
}

namespace x86
{

struct CPU
{
    union Flags
    {
        reg32 eflags;
        struct
        {
            reg16 hiword;
            union
            {
                reg16 word;
                struct
                {
                    reg8 lo;
                    reg8 hi;
                };
                struct
                {
                    reg8 cf      : 1;
                    reg8 unused1 : 1;
                    reg8 pf      : 1;
                    reg8 unused2 : 1;
                    reg8 af      : 1;
                    reg8 unused3 : 1;
                    reg8 zf      : 1;
                    reg8 sf      : 1;
                    reg8 unused4 : 1;
                    reg8 unused5 : 1;
                    reg8 df      : 1;
                    reg8 of      : 1;
                    reg8 unused6 : 4;
                };
            };
        };
    };

    union
    {
        reg64 edx_eax;
        struct
        {
            REGISTER_A(a);
            REGISTER_A(d);
        };
    };
    REGISTER_A(c);
    REGISTER_A(b);
    REGISTER_P(sp);
    REGISTER_P(bp);
    REGISTER_P(si);
    REGISTER_P(di);

    REGISTER_P(cs);
    REGISTER_P(ds);
    REGISTER_P(es);
    REGISTER_P(fs);
    REGISTER_P(gs);
    REGISTER_P(ss);

    reg32 ip;

    Flags flags;

    inline void set_szp(reg8 v)
    {
        flags.zf = !v;
        flags.sf = 1 & (v >> 7);
    }
    inline void set_szp(reg16 v)
    {
        flags.zf = !v;
        flags.sf = 1 & (v >> 15);
    }
    inline void set_szp(reg32 v)
    {
        flags.zf = !v;
        flags.sf = 1 & (v >> 31);
    }
    inline void clear_co()
    {
        flags.cf = 0;
        flags.of = 0;
    }
    inline void init(reg32 fs, reg32 ip)
    {
        fpu.init();
        terminate = false;
        this->flags.eflags = 0x00200202;
        this->flags.cf = 0;
        this->flags.zf = 0;
        this->flags.of = 0;
        this->flags.sf = 0;
        this->flags.df = 0;
        this->flags.pf = 0;
        this->ecs = 0;
        this->eds = 0;
        this->ees = 0;
        this->efs = fs;
        this->egs = 0;
        this->ess = 0;
        this->ip = ip;
    }

    void cpuid();
    void rdtsc();

    /* What a generated function says around a call (disasm/codegen): here,
     * where the function works on the CPU itself, there is nothing to hand
     * over or take back.  See Local for the other kind. */
    CPU& sync() { return *this; }
    void reload() {}
    CPU& handOver() { return *this; }

    FPU fpu;
    MMX mmx;
    bool terminate;
};

#if defined(_MSC_VER) && !defined(__clang__)
#define X86_LOCAL_INLINE __forceinline
#else
#define X86_LOCAL_INLINE inline __attribute__((always_inline))
#endif

/* The general registers and the flags of a generated function, kept to itself.
 *
 * A generated function used to work on the CPU it was handed, a structure in
 * memory that every callee receives.  To the compiler, every store the guest
 * makes to its own memory might have changed that structure, so each register
 * was read back from memory after each such store and written out again after
 * each change: two to four memory accesses for almost every instruction.  A
 * Local copies the registers in as the function starts, keeps them where the
 * compiler can hold them in the host's registers, and puts them back where the
 * CPU was only where somebody else looks: around a call (sync, then reload),
 * when the function hands over to another for good (handOver, a jmp to it),
 * and as it returns.  The names are the CPU's, so the generated code and the
 * tools/apply_*.py adapters read the same with either.
 *
 * Segment registers, ip, the FPU and MMX stay where they were and are reached
 * through references: they change seldom or not at all, and the FPU is a stack
 * that needs a Local of its own.
 *
 * Everything here is forced inline.  A destructor left out of line, as the
 * cleanup path of a call can leave it, takes the Local's address, and one taken
 * address puts the whole of it back in memory. */
struct Local
{
    union
    {
        reg64 edx_eax;
        struct
        {
            REGISTER_A(a);
            REGISTER_A(d);
        };
    };
    REGISTER_A(c);
    REGISTER_A(b);
    REGISTER_P(sp);
    REGISTER_P(bp);
    REGISTER_P(si);
    REGISTER_P(di);
    CPU::Flags flags;

    CPU& real;
    reg32& ecs;
    reg16& cs;
    reg32& eds;
    reg16& ds;
    reg32& ees;
    reg16& es;
    reg32& efs;
    reg16& fs;
    reg32& egs;
    reg16& gs;
    reg32& ess;
    reg16& ss;
    reg32& ip;
    FPU& fpu;
    MMX& mmx;
    bool& terminate;
    bool handedOver = false;

    X86_LOCAL_INLINE explicit Local(CPU& cpu)
        :   real(cpu)
        ,   ecs(cpu.ecs), cs(cpu.cs), eds(cpu.eds), ds(cpu.ds), ees(cpu.ees), es(cpu.es)
        ,   efs(cpu.efs), fs(cpu.fs), egs(cpu.egs), gs(cpu.gs), ess(cpu.ess), ss(cpu.ss)
        ,   ip(cpu.ip), fpu(cpu.fpu), mmx(cpu.mmx), terminate(cpu.terminate)
    {
        reload();
    }
    X86_LOCAL_INLINE ~Local()
    {
        if (!handedOver)
            store();
    }
    Local(const Local&) = delete;
    Local& operator=(const Local&) = delete;

    /* The registers back where the CPU is, for a callee to find. */
    X86_LOCAL_INLINE CPU& sync()
    {
        store();
        return real;
    }
    /* And what the callee left there, taken up again. */
    X86_LOCAL_INLINE void reload()
    {
        edx_eax = real.edx_eax;
        ecx = real.ecx;
        ebx = real.ebx;
        esp = real.esp;
        ebp = real.ebp;
        esi = real.esi;
        edi = real.edi;
        flags.eflags = real.flags.eflags;
    }
    /* For a jmp to another function, which returns for this one too: the
     * registers go over, and nothing comes back to be written out after it. */
    X86_LOCAL_INLINE CPU& handOver()
    {
        store();
        handedOver = true;
        return real;
    }
    /* The port's own lockContext and unlockContext read nothing but fs. */
    X86_LOCAL_INLINE operator const CPU&() const { return real; }

    X86_LOCAL_INLINE void set_szp(reg8 v)
    {
        flags.zf = !v;
        flags.sf = 1 & (v >> 7);
    }
    X86_LOCAL_INLINE void set_szp(reg16 v)
    {
        flags.zf = !v;
        flags.sf = 1 & (v >> 15);
    }
    X86_LOCAL_INLINE void set_szp(reg32 v)
    {
        flags.zf = !v;
        flags.sf = 1 & (v >> 31);
    }
    X86_LOCAL_INLINE void clear_co()
    {
        flags.cf = 0;
        flags.of = 0;
    }
    void cpuid()
    {
        store();
        real.cpuid();
        reload();
    }
    void rdtsc()
    {
        store();
        real.rdtsc();
        reload();
    }

private:
    X86_LOCAL_INLINE void store()
    {
        real.edx_eax = edx_eax;
        real.ecx = ecx;
        real.ebx = ebx;
        real.esp = esp;
        real.ebp = ebp;
        real.esi = esi;
        real.edi = edi;
        real.flags.eflags = flags.eflags;
    }
};

}

#endif /* !CPU_H_ */
