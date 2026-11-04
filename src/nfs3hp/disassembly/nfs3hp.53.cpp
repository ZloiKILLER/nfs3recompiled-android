#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_52d010: native, nfs3hp::snd::dead_0x52d010 */

/* sub_52d110: native, nfs3hp::snd::dead_0x52d110 */

/* sub_52d160: native, nfs3hp::port::packetPlayerOf */

/* sub_52d1a0: native, nfs3hp::mix::xaMono */

/* sub_52d250: native, nfs3hp::mix::xaStereo */

/* sub_52d370: native, nfs3hp::port::codedBits */

/* sub_52d3cc: native, nfs3hp::port::codedSkip */

/* sub_52d428: native, nfs3hp::port::codedPulses */

/* sub_52d5cc: native, nfs3hp::port::codedInterpolate */

/* sub_52d618: native, nfs3hp::port::codedFilter */

/* sub_52d6d8: native, nfs3hp::port::codedSynthesis */

/* sub_52de94: native, nfs3hp::port::codedStart */

/* sub_52df80: native, nfs3hp::port::codedFrame */

/* sub_52e310: native, nfs3hp::watcom::mbccpy */

/* sub_52e350: native, nfs3hp::watcom::mbsupr */

/* sub_52e3a0: native, nfs3hp::watcom::mbccmp */

/* align: skip  */
void Application::sub_52e3f8(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052e3f8  e9fd090000             -jmp 0x52edfa
    return sub_52edfa(app, cpu.handOver());
}

/* sub_52e400: native, nfs3hp::watcom::get_std_stream */

/* align: skip 0x00 */
void Application::sub_52e420(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052e420  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052e421  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052e422  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052e423  8b0dfc4f5600           -mov ecx, dword ptr [0x564ffc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5656572) /* 0x564ffc */);
    // 0052e429  8b1df84f5600           -mov ebx, dword ptr [0x564ff8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5656568) /* 0x564ff8 */);
    // 0052e42f  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0052e431  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052e433  e8c8090000             -call 0x52ee00
    cpu.esp -= 4;
    sub_52ee00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e438  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e439  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e43a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e43b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 */
void Application::sub_52e440(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052e440  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052e441  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052e442  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052e443  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052e444  81ecc8000000           -sub esp, 0xc8
    (cpu.esp) -= x86::reg32(x86::sreg32(200 /*0xc8*/));
    // 0052e44a  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0052e44c  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052e44e  e8ed62fcff             -call 0x4f4740
    cpu.esp -= 4;
    sub_4f4740(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e453  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052e455  e80624fcff             -call 0x4f0860
    cpu.esp -= 4;
    sub_4f0860(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e45a  8b1524725600           -mov edx, dword ptr [0x567224]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5665316) /* 0x567224 */);
    // 0052e460  a120725600             -mov eax, dword ptr [0x567220]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5665312) /* 0x567220 */);
    // 0052e465  8d8c24c4000000         -lea ecx, [esp + 0xc4]
    cpu.ecx = x86::reg32(cpu.esp + x86::reg32(196) /* 0xc4 */);
    // 0052e46c  e8bf13fcff             -call 0x4ef830
    cpu.esp -= 4;
    sub_4ef830(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e471  8d8424c0000000         -lea eax, [esp + 0xc0]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(192) /* 0xc0 */);
    // 0052e478  8d9c24b8000000         -lea ebx, [esp + 0xb8]
    cpu.ebx = x86::reg32(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 0052e47f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052e480  8d9424c0000000         -lea edx, [esp + 0xc0]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(192) /* 0xc0 */);
    // 0052e487  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0052e489  e80228fcff             -call 0x4f0c90
    cpu.esp -= 4;
    sub_4f0c90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e48e  8b8c24b8000000         -mov ecx, dword ptr [esp + 0xb8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(184) /* 0xb8 */);
    // 0052e495  8b9c24bc000000         -mov ebx, dword ptr [esp + 0xbc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(188) /* 0xbc */);
    // 0052e49c  8b1524725600           -mov edx, dword ptr [0x567224]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5665316) /* 0x567224 */);
    // 0052e4a2  a1f8715600             -mov eax, dword ptr [0x5671f8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5665272) /* 0x5671f8 */);
    // 0052e4a7  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052e4a8  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0052e4aa  8b15fc715600           -mov edx, dword ptr [0x5671fc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5665276) /* 0x5671fc */);
    // 0052e4b0  8b9c24c8000000         -mov ebx, dword ptr [esp + 0xc8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(200) /* 0xc8 */);
    // 0052e4b7  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0052e4b9  8b8c24c4000000         -mov ecx, dword ptr [esp + 0xc4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(196) /* 0xc4 */);
    // 0052e4c0  e87bf0fcff             -call 0x4fd540
    cpu.esp -= 4;
    sub_4fd540(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e4c5  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052e4c7  e89462fcff             -call 0x4f4760
    cpu.esp -= 4;
    sub_4f4760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e4cc  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0052e4ce  e87d16fcff             -call 0x4efb50
    cpu.esp -= 4;
    sub_4efb50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e4d3  81c4c8000000           -add esp, 0xc8
    (cpu.esp) += x86::reg32(x86::sreg32(200 /*0xc8*/));
    // 0052e4d9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e4da  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e4db  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e4dc  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e4dd  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_52e4e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052e4e0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052e4e1  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0052e4e3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0052e4e5  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0052e4e7  e86423fcff             -call 0x4f0850
    cpu.esp -= 4;
    sub_4f0850(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e4ec  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052e4ee  e84dffffff             -call 0x52e440
    cpu.esp -= 4;
    sub_52e440(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e4f3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e4f4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_52e500(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052e500  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052e501  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052e502  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052e503  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0052e506  8944240c               -mov dword ptr [esp + 0xc], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 0052e50a  89542404               -mov dword ptr [esp + 4], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.edx;
    // 0052e50e  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 0052e510  894c2408               -mov dword ptr [esp + 8], ecx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.ecx;
    // 0052e514  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0052e516  7402                   -je 0x52e51a
    if (cpu.flags.zf)
    {
        goto L_0x0052e51a;
    }
    // 0052e518  8902                   -mov dword ptr [edx], eax
    app->getMemory<x86::reg32>(cpu.edx) = cpu.eax;
L_0x0052e51a:
    // 0052e51a  8b54240c               -mov edx, dword ptr [esp + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
L_0x0052e51e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052e51e  8a02                   -mov al, byte ptr [edx]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx);
    // 0052e520  fec0                   -inc al
    (cpu.al)++;
    // 0052e522  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0052e527  f680f04e560002         +test byte ptr [eax + 0x564ef0], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5656304) /* 0x564ef0 */) & 2 /*0x2*/));
    // 0052e52e  7403                   -je 0x52e533
    if (cpu.flags.zf)
    {
        goto L_0x0052e533;
    }
    // 0052e530  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0052e531  ebeb                   -jmp 0x52e51e
    goto L_0x0052e51e;
L_0x0052e533:
    // 0052e533  8a2a                   -mov ch, byte ptr [edx]
    cpu.ch = app->getMemory<x86::reg8>(cpu.edx);
    // 0052e535  80fd2b                 +cmp ch, 0x2b
    {
        x86::reg8 tmp1 = cpu.ch;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(43 /*0x2b*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e538  7405                   -je 0x52e53f
    if (cpu.flags.zf)
    {
        goto L_0x0052e53f;
    }
    // 0052e53a  80fd2d                 +cmp ch, 0x2d
    {
        x86::reg8 tmp1 = cpu.ch;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(45 /*0x2d*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e53d  7501                   -jne 0x52e540
    if (!cpu.flags.zf)
    {
        goto L_0x0052e540;
    }
L_0x0052e53f:
    // 0052e53f  42                     -inc edx
    (cpu.edx)++;
L_0x0052e540:
    // 0052e540  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0052e542  752c                   -jne 0x52e570
    if (!cpu.flags.zf)
    {
        goto L_0x0052e570;
    }
    // 0052e544  803a30                 +cmp byte ptr [edx], 0x30
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edx);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(48 /*0x30*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e547  7514                   -jne 0x52e55d
    if (!cpu.flags.zf)
    {
        goto L_0x0052e55d;
    }
    // 0052e549  8a4a01                 -mov cl, byte ptr [edx + 1]
    cpu.cl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 0052e54c  80f978                 +cmp cl, 0x78
    {
        x86::reg8 tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(120 /*0x78*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e54f  7405                   -je 0x52e556
    if (cpu.flags.zf)
    {
        goto L_0x0052e556;
    }
    // 0052e551  80f958                 +cmp cl, 0x58
    {
        x86::reg8 tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(88 /*0x58*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e554  7507                   -jne 0x52e55d
    if (!cpu.flags.zf)
    {
        goto L_0x0052e55d;
    }
L_0x0052e556:
    // 0052e556  be10000000             -mov esi, 0x10
    cpu.esi = 16 /*0x10*/;
    // 0052e55b  eb33                   -jmp 0x52e590
    goto L_0x0052e590;
L_0x0052e55d:
    // 0052e55d  803a30                 +cmp byte ptr [edx], 0x30
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edx);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(48 /*0x30*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e560  7507                   -jne 0x52e569
    if (!cpu.flags.zf)
    {
        goto L_0x0052e569;
    }
    // 0052e562  be08000000             -mov esi, 8
    cpu.esi = 8 /*0x8*/;
    // 0052e567  eb3c                   -jmp 0x52e5a5
    goto L_0x0052e5a5;
L_0x0052e569:
    // 0052e569  be0a000000             -mov esi, 0xa
    cpu.esi = 10 /*0xa*/;
    // 0052e56e  eb35                   -jmp 0x52e5a5
    goto L_0x0052e5a5;
L_0x0052e570:
    // 0052e570  83fe02                 +cmp esi, 2
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052e573  7c05                   -jl 0x52e57a
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0052e57a;
    }
    // 0052e575  83fe24                 +cmp esi, 0x24
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(36 /*0x24*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052e578  7e11                   -jle 0x52e58b
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0052e58b;
    }
L_0x0052e57a:
    // 0052e57a  b80d000000             -mov eax, 0xd
    cpu.eax = 13 /*0xd*/;
    // 0052e57f  e8fc42fdff             -call 0x502880
    cpu.esp -= 4;
    nfs3hp::watcom::set_errno(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e584  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0052e586  e9c1000000             -jmp 0x52e64c
    goto L_0x0052e64c;
L_0x0052e58b:
    // 0052e58b  83fe10                 +cmp esi, 0x10
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052e58e  7515                   -jne 0x52e5a5
    if (!cpu.flags.zf)
    {
        goto L_0x0052e5a5;
    }
L_0x0052e590:
    // 0052e590  803a30                 +cmp byte ptr [edx], 0x30
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edx);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(48 /*0x30*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e593  7510                   -jne 0x52e5a5
    if (!cpu.flags.zf)
    {
        goto L_0x0052e5a5;
    }
    // 0052e595  8a7a01                 -mov bh, byte ptr [edx + 1]
    cpu.bh = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 0052e598  80ff78                 +cmp bh, 0x78
    {
        x86::reg8 tmp1 = cpu.bh;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(120 /*0x78*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e59b  7405                   -je 0x52e5a2
    if (cpu.flags.zf)
    {
        goto L_0x0052e5a2;
    }
    // 0052e59d  80ff58                 +cmp bh, 0x58
    {
        x86::reg8 tmp1 = cpu.bh;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(88 /*0x58*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e5a0  7503                   -jne 0x52e5a5
    if (!cpu.flags.zf)
    {
        goto L_0x0052e5a5;
    }
L_0x0052e5a2:
    // 0052e5a2  83c202                 -add edx, 2
    (cpu.edx) += x86::reg32(x86::sreg32(2 /*0x2*/));
L_0x0052e5a5:
    // 0052e5a5  891424                 -mov dword ptr [esp], edx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.edx;
    // 0052e5a8  8d2cb500000000         -lea ebp, [esi*4]
    cpu.ebp = x86::reg32(cpu.esi * 4);
    // 0052e5af  30c9                   -xor cl, cl
    cpu.cl ^= x86::reg8(x86::sreg8(cpu.cl));
    // 0052e5b1  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x0052e5b3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052e5b3  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052e5b5  8a02                   -mov al, byte ptr [edx]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx);
    // 0052e5b7  e8b4000000             -call 0x52e670
    cpu.esp -= 4;
    sub_52e670(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e5bc  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0052e5be  39f0                   +cmp eax, esi
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052e5c0  7d1a                   -jge 0x52e5dc
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0052e5dc;
    }
    // 0052e5c2  3b9d48b65600           +cmp ebx, dword ptr [ebp + 0x56b648]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(5682760) /* 0x56b648 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052e5c8  7602                   -jbe 0x52e5cc
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0052e5cc;
    }
    // 0052e5ca  b101                   -mov cl, 1
    cpu.cl = 1 /*0x1*/;
L_0x0052e5cc:
    // 0052e5cc  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0052e5ce  0fafde                 -imul ebx, esi
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(cpu.esi)));
    // 0052e5d1  01fb                   -add ebx, edi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edi));
    // 0052e5d3  39c3                   +cmp ebx, eax
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052e5d5  7302                   -jae 0x52e5d9
    if (!cpu.flags.cf)
    {
        goto L_0x0052e5d9;
    }
    // 0052e5d7  b101                   -mov cl, 1
    cpu.cl = 1 /*0x1*/;
L_0x0052e5d9:
    // 0052e5d9  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0052e5da  ebd7                   -jmp 0x52e5b3
    goto L_0x0052e5b3;
L_0x0052e5dc:
    // 0052e5dc  3b1424                 +cmp edx, dword ptr [esp]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052e5df  7504                   -jne 0x52e5e5
    if (!cpu.flags.zf)
    {
        goto L_0x0052e5e5;
    }
    // 0052e5e1  8b54240c               -mov edx, dword ptr [esp + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
L_0x0052e5e5:
    // 0052e5e5  8b7c2404               -mov edi, dword ptr [esp + 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0052e5e9  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0052e5eb  7402                   -je 0x52e5ef
    if (cpu.flags.zf)
    {
        goto L_0x0052e5ef;
    }
    // 0052e5ed  8917                   -mov dword ptr [edi], edx
    app->getMemory<x86::reg32>(cpu.edi) = cpu.edx;
L_0x0052e5ef:
    // 0052e5ef  837c240801             +cmp dword ptr [esp + 8], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052e5f4  750f                   -jne 0x52e605
    if (!cpu.flags.zf)
    {
        goto L_0x0052e605;
    }
    // 0052e5f6  81fb00000080           +cmp ebx, 0x80000000
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052e5fc  7207                   -jb 0x52e605
    if (cpu.flags.cf)
    {
        goto L_0x0052e605;
    }
    // 0052e5fe  7509                   -jne 0x52e609
    if (!cpu.flags.zf)
    {
        goto L_0x0052e609;
    }
    // 0052e600  80fd2d                 +cmp ch, 0x2d
    {
        x86::reg8 tmp1 = cpu.ch;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(45 /*0x2d*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e603  7504                   -jne 0x52e609
    if (!cpu.flags.zf)
    {
        goto L_0x0052e609;
    }
L_0x0052e605:
    // 0052e605  84c9                   +test cl, cl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & cpu.cl));
    // 0052e607  743a                   -je 0x52e643
    if (cpu.flags.zf)
    {
        goto L_0x0052e643;
    }
L_0x0052e609:
    // 0052e609  b80e000000             -mov eax, 0xe
    cpu.eax = 14 /*0xe*/;
    // 0052e60e  e86d42fdff             -call 0x502880
    cpu.esp -= 4;
    nfs3hp::watcom::set_errno(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e613  837c240800             +cmp dword ptr [esp + 8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052e618  750c                   -jne 0x52e626
    if (!cpu.flags.zf)
    {
        goto L_0x0052e626;
    }
    // 0052e61a  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052e61f  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0052e622  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e623  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e624  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e625  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e626:
    // 0052e626  80fd2d                 +cmp ch, 0x2d
    {
        x86::reg8 tmp1 = cpu.ch;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(45 /*0x2d*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e629  750c                   -jne 0x52e637
    if (!cpu.flags.zf)
    {
        goto L_0x0052e637;
    }
    // 0052e62b  b800000080             -mov eax, 0x80000000
    cpu.eax = 2147483648 /*0x80000000*/;
    // 0052e630  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0052e633  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e634  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e635  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e636  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e637:
    // 0052e637  b8ffffff7f             -mov eax, 0x7fffffff
    cpu.eax = 2147483647 /*0x7fffffff*/;
    // 0052e63c  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0052e63f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e640  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e641  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e642  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e643:
    // 0052e643  80fd2d                 +cmp ch, 0x2d
    {
        x86::reg8 tmp1 = cpu.ch;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(45 /*0x2d*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e646  7502                   -jne 0x52e64a
    if (!cpu.flags.zf)
    {
        goto L_0x0052e64a;
    }
    // 0052e648  f7db                   -neg ebx
    cpu.ebx = ~cpu.ebx + 1;
L_0x0052e64a:
    // 0052e64a  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
L_0x0052e64c:
    // 0052e64c  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0052e64f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e650  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e651  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e652  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_52e654: native, nfs3hp::dead::stub_0x52e654 */

/* align: skip 0x8b 0xc0 */
void Application::sub_52e660(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052e660  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052e661  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0052e666  e895feffff             -call 0x52e500
    cpu.esp -= 4;
    sub_52e500(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e66b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e66c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_52e670(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052e670  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052e671  3c30                   +cmp al, 0x30
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(48 /*0x30*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e673  720e                   -jb 0x52e683
    if (cpu.flags.cf)
    {
        goto L_0x0052e683;
    }
    // 0052e675  3c39                   +cmp al, 0x39
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(57 /*0x39*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e677  770a                   -ja 0x52e683
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0052e683;
    }
    // 0052e679  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0052e67e  83e830                 -sub eax, 0x30
    (cpu.eax) -= x86::reg32(x86::sreg32(48 /*0x30*/));
    // 0052e681  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e682  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e683:
    // 0052e683  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0052e688  e85324fcff             -call 0x4f0ae0
    cpu.esp -= 4;
    nfs3hp::watcom::tolower(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e68d  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052e68f  3c61                   +cmp al, 0x61
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(97 /*0x61*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e691  720d                   -jb 0x52e6a0
    if (cpu.flags.cf)
    {
        goto L_0x0052e6a0;
    }
    // 0052e693  3c69                   +cmp al, 0x69
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(105 /*0x69*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e695  7709                   -ja 0x52e6a0
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0052e6a0;
    }
    // 0052e697  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052e699  88d0                   -mov al, dl
    cpu.al = cpu.dl;
    // 0052e69b  83e857                 -sub eax, 0x57
    (cpu.eax) -= x86::reg32(x86::sreg32(87 /*0x57*/));
    // 0052e69e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e69f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e6a0:
    // 0052e6a0  3c6a                   +cmp al, 0x6a
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(106 /*0x6a*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e6a2  720e                   -jb 0x52e6b2
    if (cpu.flags.cf)
    {
        goto L_0x0052e6b2;
    }
    // 0052e6a4  3c72                   +cmp al, 0x72
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(114 /*0x72*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e6a6  770a                   -ja 0x52e6b2
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0052e6b2;
    }
    // 0052e6a8  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0052e6ad  83e857                 -sub eax, 0x57
    (cpu.eax) -= x86::reg32(x86::sreg32(87 /*0x57*/));
    // 0052e6b0  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e6b1  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e6b2:
    // 0052e6b2  3c73                   +cmp al, 0x73
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(115 /*0x73*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e6b4  720e                   -jb 0x52e6c4
    if (cpu.flags.cf)
    {
        goto L_0x0052e6c4;
    }
    // 0052e6b6  3c7a                   +cmp al, 0x7a
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(122 /*0x7a*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e6b8  770a                   -ja 0x52e6c4
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0052e6c4;
    }
    // 0052e6ba  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0052e6bf  83e857                 -sub eax, 0x57
    (cpu.eax) -= x86::reg32(x86::sreg32(87 /*0x57*/));
    // 0052e6c2  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e6c3  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e6c4:
    // 0052e6c4  b825000000             -mov eax, 0x25
    cpu.eax = 37 /*0x25*/;
    // 0052e6c9  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e6ca  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_52e6d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052e6d0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052e6d1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052e6d2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052e6d3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052e6d4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052e6d5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052e6d6  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0052e6d9  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 0052e6db  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 0052e6e0  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0052e6e5  891424                 -mov dword ptr [esp], edx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.edx;
    // 0052e6e8  ba3d000000             -mov edx, 0x3d
    cpu.edx = 61 /*0x3d*/;
    // 0052e6ed  895c2404               -mov dword ptr [esp + 4], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.ebx;
    // 0052e6f1  e86a070000             -call 0x52ee60
    cpu.esp -= 4;
    sub_52ee60(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e6f6  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0052e6f8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052e6fa  750a                   -jne 0x52e706
    if (!cpu.flags.zf)
    {
        goto L_0x0052e706;
    }
    // 0052e6fc  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052e701  e97c010000             -jmp 0x52e882
    goto L_0x0052e882;
L_0x0052e706:
    // 0052e706  39e8                   +cmp eax, ebp
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebp));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052e708  750f                   -jne 0x52e719
    if (!cpu.flags.zf)
    {
        goto L_0x0052e719;
    }
    // 0052e70a  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052e70f  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0052e712  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e713  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e714  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e715  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e716  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e717  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e718  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e719:
    // 0052e719  29e8                   -sub eax, ebp
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 0052e71b  89442410               -mov dword ptr [esp + 0x10], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */) = cpu.eax;
    // 0052e71f  40                     -inc eax
    (cpu.eax)++;
    // 0052e720  e8db91fcff             -call 0x4f7900
    cpu.esp -= 4;
    nfs3hp::watcom::nmalloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e725  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052e727  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0052e72b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052e72d  750f                   -jne 0x52e73e
    if (!cpu.flags.zf)
    {
        goto L_0x0052e73e;
    }
    // 0052e72f  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052e734  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0052e737  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e738  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e739  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e73a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e73b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e73c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e73d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e73e:
    // 0052e73e  8b4c2410               -mov ecx, dword ptr [esp + 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 0052e742  89ee                   -mov esi, ebp
    cpu.esi = cpu.ebp;
    // 0052e744  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0052e746  06                     -push es
    app->getMemory<x86::reg16>(cpu.esp-4) = cpu.es;
    cpu.esp -= 4;
    // 0052e747  8cd8                   -mov eax, ds
    cpu.eax = cpu.ds;
    // 0052e749  8ec0                   -mov es, eax
    cpu.es = cpu.eax;
    // 0052e74b  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052e74c  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052e74e  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0052e751  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 4;
            cpu.esi -= 4;
        }
        else
        {
            cpu.edi += 4;
            cpu.esi += 4;
        }
        --cpu.ecx;
    }
    // 0052e753  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0052e755  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0052e758  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg8>(cpu.ees + cpu.edi) = app->getMemory<x86::reg8>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 1;
            cpu.esi -= 1;
        }
        else
        {
            cpu.edi += 1;
            cpu.esi += 1;
        }
        --cpu.ecx;
        if (cpu.flags.zf)
            break;
    }
    // 0052e75a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e75b  07                     -pop es
    cpu.es = app->getMemory<x86::reg16>(cpu.esp);
    cpu.esp += 4;
    // 0052e75c  8b442410               -mov eax, dword ptr [esp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 0052e760  8d7301                 -lea esi, [ebx + 1]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 0052e763  c6040200               -mov byte ptr [edx + eax], 0
    app->getMemory<x86::reg8>(cpu.edx + cpu.eax * 1) = 0 /*0x0*/;
    // 0052e767  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0052e769  e8e298feff             -call 0x518050
    cpu.esp -= 4;
    nfs3hp::watcom::mbslen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e76e  89442414               -mov dword ptr [esp + 0x14], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 0052e772  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052e774  744b                   -je 0x52e7c1
    if (cpu.flags.zf)
    {
        goto L_0x0052e7c1;
    }
    // 0052e776  40                     -inc eax
    (cpu.eax)++;
    // 0052e777  e88491fcff             -call 0x4f7900
    cpu.esp -= 4;
    nfs3hp::watcom::nmalloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e77c  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0052e77e  8944240c               -mov dword ptr [esp + 0xc], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 0052e782  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052e784  7516                   -jne 0x52e79c
    if (!cpu.flags.zf)
    {
        goto L_0x0052e79c;
    }
    // 0052e786  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0052e788  e86392fcff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e78d  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052e792  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0052e795  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e796  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e797  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e798  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e799  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e79a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e79b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e79c:
    // 0052e79c  8b4c2414               -mov ecx, dword ptr [esp + 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */);
    // 0052e7a0  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0052e7a2  06                     -push es
    app->getMemory<x86::reg16>(cpu.esp-4) = cpu.es;
    cpu.esp -= 4;
    // 0052e7a3  8cd8                   -mov eax, ds
    cpu.eax = cpu.ds;
    // 0052e7a5  8ec0                   -mov es, eax
    cpu.es = cpu.eax;
    // 0052e7a7  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052e7a8  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052e7aa  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0052e7ad  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 4;
            cpu.esi -= 4;
        }
        else
        {
            cpu.edi += 4;
            cpu.esi += 4;
        }
        --cpu.ecx;
    }
    // 0052e7af  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0052e7b1  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0052e7b4  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg8>(cpu.ees + cpu.edi) = app->getMemory<x86::reg8>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 1;
            cpu.esi -= 1;
        }
        else
        {
            cpu.edi += 1;
            cpu.esi += 1;
        }
        --cpu.ecx;
        if (cpu.flags.zf)
            break;
    }
    // 0052e7b6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e7b7  07                     -pop es
    cpu.es = app->getMemory<x86::reg16>(cpu.esp);
    cpu.esp += 4;
    // 0052e7b8  035c2414               +add ebx, dword ptr [esp + 0x14]
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(20) /* 0x14 */)));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0052e7bc  c60300                 -mov byte ptr [ebx], 0
    app->getMemory<x86::reg8>(cpu.ebx) = 0 /*0x0*/;
    // 0052e7bf  eb04                   -jmp 0x52e7c5
    goto L_0x0052e7c5;
L_0x0052e7c1:
    // 0052e7c1  8944240c               -mov dword ptr [esp + 0xc], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.eax;
L_0x0052e7c5:
    // 0052e7c5  8b44240c               -mov eax, dword ptr [esp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0052e7c9  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052e7ca  8b54240c               -mov edx, dword ptr [esp + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0052e7ce  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052e7cf  2eff15dc455300         -call dword ptr cs:[0x5345dc]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457372) /* 0x5345dc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e7d6  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052e7d8  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0052e7dc  e80f92fcff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e7e1  8b44240c               -mov eax, dword ptr [esp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0052e7e5  e80692fcff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e7ea  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0052e7ec  750f                   -jne 0x52e7fd
    if (!cpu.flags.zf)
    {
        goto L_0x0052e7fd;
    }
    // 0052e7ee  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052e7f3  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0052e7f6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e7f7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e7f8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e7f9  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e7fa  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e7fb  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e7fc  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e7fd:
    // 0052e7fd  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0052e7ff  e888000000             -call 0x52e88c
    cpu.esp -= 4;
    sub_52e88c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e804  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052e806  740f                   -je 0x52e817
    if (cpu.flags.zf)
    {
        goto L_0x0052e817;
    }
    // 0052e808  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052e80d  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0052e810  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e811  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e812  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e813  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e814  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e815  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e816  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e817:
    // 0052e817  833d58b1a00000         +cmp dword ptr [0xa0b158], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10531160) /* 0xa0b158 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052e81e  7462                   -je 0x52e882
    if (cpu.flags.zf)
    {
        goto L_0x0052e882;
    }
    // 0052e820  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0052e822  e82998feff             -call 0x518050
    cpu.esp -= 4;
    nfs3hp::watcom::mbslen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e827  8d5801                 -lea ebx, [eax + 1]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0052e82a  8b0424                 -mov eax, dword ptr [esp]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    // 0052e82d  0fafc3                 -imul eax, ebx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ebx)));
    // 0052e830  e8cb90fcff             -call 0x4f7900
    cpu.esp -= 4;
    nfs3hp::watcom::nmalloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e835  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0052e837  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052e839  7519                   -jne 0x52e854
    if (!cpu.flags.zf)
    {
        goto L_0x0052e854;
    }
    // 0052e83b  b805000000             -mov eax, 5
    cpu.eax = 5 /*0x5*/;
    // 0052e840  e84727fdff             -call 0x500f8c
    cpu.esp -= 4;
    nfs3hp::watcom::set_errno_dos(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e845  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052e84a  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0052e84d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e84e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e84f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e850  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e851  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e852  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e853  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e854:
    // 0052e854  0faf5c2404             -imul ebx, dword ptr [esp + 4]
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */))));
    // 0052e859  89ea                   -mov edx, ebp
    cpu.edx = cpu.ebp;
    // 0052e85b  e82098feff             -call 0x518080
    cpu.esp -= 4;
    nfs3hp::watcom::mbstowcs(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e860  83f8ff                 +cmp eax, -1
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052e863  7516                   -jne 0x52e87b
    if (!cpu.flags.zf)
    {
        goto L_0x0052e87b;
    }
    // 0052e865  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052e867  e88491fcff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e86c  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052e871  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0052e874  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e875  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e876  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e877  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e878  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e879  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e87a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e87b:
    // 0052e87b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052e87d  e8f6d8ffff             -call 0x52c178
    cpu.esp -= 4;
    nfs3hp::watcom::wputenv(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052e882:
    // 0052e882  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0052e885  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e886  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e887  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e888  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e889  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e88a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e88b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_52e88c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052e88c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052e88d  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052e88e  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052e88f  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052e890  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052e891  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052e892  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0052e895  89442404               -mov dword ptr [esp + 4], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 0052e899  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052e89b  750a                   -jne 0x52e8a7
    if (!cpu.flags.zf)
    {
        goto L_0x0052e8a7;
    }
    // 0052e89d  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052e8a2  e989010000             -jmp 0x52ea30
    goto L_0x0052ea30;
L_0x0052e8a7:
    // 0052e8a7  803800                 +cmp byte ptr [eax], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e8aa  7415                   -je 0x52e8c1
    if (cpu.flags.zf)
    {
        goto L_0x0052e8c1;
    }
    // 0052e8ac  8d5001                 -lea edx, [eax + 1]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0052e8af  803a00                 +cmp byte ptr [edx], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edx);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e8b2  740d                   -je 0x52e8c1
    if (cpu.flags.zf)
    {
        goto L_0x0052e8c1;
    }
L_0x0052e8b4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052e8b4  803a3d                 +cmp byte ptr [edx], 0x3d
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edx);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(61 /*0x3d*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e8b7  7408                   -je 0x52e8c1
    if (cpu.flags.zf)
    {
        goto L_0x0052e8c1;
    }
    // 0052e8b9  8a4a01                 -mov cl, byte ptr [edx + 1]
    cpu.cl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 0052e8bc  42                     -inc edx
    (cpu.edx)++;
    // 0052e8bd  84c9                   +test cl, cl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & cpu.cl));
    // 0052e8bf  75f3                   -jne 0x52e8b4
    if (!cpu.flags.zf)
    {
        goto L_0x0052e8b4;
    }
L_0x0052e8c1:
    // 0052e8c1  803a00                 +cmp byte ptr [edx], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edx);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e8c4  750f                   -jne 0x52e8d5
    if (!cpu.flags.zf)
    {
        goto L_0x0052e8d5;
    }
    // 0052e8c6  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052e8cb  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0052e8ce  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e8cf  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e8d0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e8d1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e8d2  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e8d3  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e8d4  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e8d5:
    // 0052e8d5  807a0100               +cmp byte ptr [edx + 1], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(1) /* 0x1 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052e8d9  0f94c0                 -sete al
    cpu.al = cpu.flags.zf;
    // 0052e8dc  0fb6e8                 -movzx ebp, al
    cpu.ebp = x86::reg32(cpu.al);
    // 0052e8df  a154b1a000             -mov eax, dword ptr [0xa0b154]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */);
    // 0052e8e4  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0052e8e8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052e8ea  7541                   -jne 0x52e92d
    if (!cpu.flags.zf)
    {
        goto L_0x0052e92d;
    }
    // 0052e8ec  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 0052e8ee  0f853c010000           -jne 0x52ea30
    if (!cpu.flags.zf)
    {
        goto L_0x0052ea30;
    }
    // 0052e8f4  b809000000             -mov eax, 9
    cpu.eax = 9 /*0x9*/;
    // 0052e8f9  e80290fcff             -call 0x4f7900
    cpu.esp -= 4;
    nfs3hp::watcom::nmalloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e8fe  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0052e902  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052e904  750f                   -jne 0x52e915
    if (!cpu.flags.zf)
    {
        goto L_0x0052e915;
    }
    // 0052e906  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052e90b  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0052e90e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e90f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e910  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e911  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e912  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e913  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e914  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e915:
    // 0052e915  a354b1a000             -mov dword ptr [0xa0b154], eax
    app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */) = cpu.eax;
    // 0052e91a  83c008                 +add eax, 8
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0052e91d  8968f8                 -mov dword ptr [eax - 8], ebp
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-8) /* -0x8 */) = cpu.ebp;
    // 0052e920  8968fc                 -mov dword ptr [eax - 4], ebp
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.ebp;
    // 0052e923  a350b1a000             -mov dword ptr [0xa0b150], eax
    app->getMemory<x86::reg32>(x86::reg32(10531152) /* 0xa0b150 */) = cpu.eax;
    // 0052e928  e9e5000000             -jmp 0x52ea12
    goto L_0x0052ea12;
L_0x0052e92d:
    // 0052e92d  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0052e931  89ea                   -mov edx, ebp
    cpu.edx = cpu.ebp;
    // 0052e933  e804010000             -call 0x52ea3c
    cpu.esp -= 4;
    sub_52ea3c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e938  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 0052e93a  0f85ee000000           -jne 0x52ea2e
    if (!cpu.flags.zf)
    {
        goto L_0x0052ea2e;
    }
    // 0052e940  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052e942  0f8fc7000000           -jg 0x52ea0f
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0052ea0f;
    }
    // 0052e948  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 0052e94a  f7dd                   -neg ebp
    cpu.ebp = ~cpu.ebp + 1;
    // 0052e94c  8d5d01                 -lea ebx, [ebp + 1]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(1) /* 0x1 */);
    // 0052e94f  8d0cad00000000         -lea ecx, [ebp*4]
    cpu.ecx = x86::reg32(cpu.ebp * 4);
    // 0052e956  8d4108                 -lea eax, [ecx + 8]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0052e959  8b3550b1a000           -mov esi, dword ptr [0xa0b150]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(10531152) /* 0xa0b150 */);
    // 0052e95f  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0052e962  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0052e964  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0052e966  754f                   -jne 0x52e9b7
    if (!cpu.flags.zf)
    {
        goto L_0x0052e9b7;
    }
    // 0052e968  e8938ffcff             -call 0x4f7900
    cpu.esp -= 4;
    nfs3hp::watcom::nmalloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e96d  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052e96f  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0052e973  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052e975  750f                   -jne 0x52e986
    if (!cpu.flags.zf)
    {
        goto L_0x0052e986;
    }
    // 0052e977  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052e97c  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0052e97f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e980  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e981  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e982  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e983  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e984  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e985  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e986:
    // 0052e986  8b3554b1a000           -mov esi, dword ptr [0xa0b154]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */);
    // 0052e98c  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0052e98e  06                     -push es
    app->getMemory<x86::reg16>(cpu.esp-4) = cpu.es;
    cpu.esp -= 4;
    // 0052e98f  8cd8                   -mov eax, ds
    cpu.eax = cpu.ds;
    // 0052e991  8ec0                   -mov es, eax
    cpu.es = cpu.eax;
    // 0052e993  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052e994  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052e996  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 0052e999  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 4;
            cpu.esi -= 4;
        }
        else
        {
            cpu.edi += 4;
            cpu.esi += 4;
        }
        --cpu.ecx;
    }
    // 0052e99b  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 0052e99d  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 0052e9a0  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
    while (cpu.ecx)
    {
        app->getMemory<x86::reg8>(cpu.ees + cpu.edi) = app->getMemory<x86::reg8>(cpu.esi);
        if (cpu.flags.df)
        {
            cpu.edi -= 1;
            cpu.esi -= 1;
        }
        else
        {
            cpu.edi += 1;
            cpu.esi += 1;
        }
        --cpu.ecx;
        if (cpu.flags.zf)
            break;
    }
    // 0052e9a2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e9a3  07                     -pop es
    cpu.es = app->getMemory<x86::reg16>(cpu.esp);
    cpu.esp += 4;
    // 0052e9a4  8b0424                 -mov eax, dword ptr [esp]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    // 0052e9a7  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0052e9a9  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0052e9ab  a350b1a000             -mov dword ptr [0xa0b150], eax
    app->getMemory<x86::reg32>(x86::reg32(10531152) /* 0xa0b150 */) = cpu.eax;
    // 0052e9b0  e88b1cfbff             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e9b5  eb3c                   -jmp 0x52e9f3
    goto L_0x0052e9f3;
L_0x0052e9b7:
    // 0052e9b7  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052e9b9  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0052e9bd  e8fe9bfeff             -call 0x5185c0
    cpu.esp -= 4;
    nfs3hp::watcom::nrealloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e9c2  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0052e9c6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052e9c8  750f                   -jne 0x52e9d9
    if (!cpu.flags.zf)
    {
        goto L_0x0052e9d9;
    }
    // 0052e9ca  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052e9cf  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0052e9d2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e9d3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e9d4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e9d5  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e9d6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e9d7  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052e9d8  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052e9d9:
    // 0052e9d9  8b0c24                 -mov ecx, dword ptr [esp]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    // 0052e9dc  8b1550b1a000           -mov edx, dword ptr [0xa0b150]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10531152) /* 0xa0b150 */);
    // 0052e9e2  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0052e9e4  89eb                   -mov ebx, ebp
    cpu.ebx = cpu.ebp;
    // 0052e9e6  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052e9e8  e8539cfeff             -call 0x518640
    cpu.esp -= 4;
    nfs3hp::watcom::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052e9ed  890d50b1a000           -mov dword ptr [0xa0b150], ecx
    app->getMemory<x86::reg32>(x86::reg32(10531152) /* 0xa0b150 */) = cpu.ecx;
L_0x0052e9f3:
    // 0052e9f3  8b7c2408               -mov edi, dword ptr [esp + 8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0052e9f7  8d04ad00000000         -lea eax, [ebp*4]
    cpu.eax = x86::reg32(cpu.ebp * 4);
    // 0052e9fe  01f8                   +add eax, edi
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0052ea00  893d54b1a000           -mov dword ptr [0xa0b154], edi
    app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */) = cpu.edi;
    // 0052ea06  c7400400000000         -mov dword ptr [eax + 4], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = 0 /*0x0*/;
    // 0052ea0d  eb03                   -jmp 0x52ea12
    goto L_0x0052ea12;
L_0x0052ea0f:
    // 0052ea0f  8d68ff                 -lea ebp, [eax - 1]
    cpu.ebp = x86::reg32(cpu.eax + x86::reg32(-1) /* -0x1 */);
L_0x0052ea12:
    // 0052ea12  8b542408               -mov edx, dword ptr [esp + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0052ea16  8d04ad00000000         -lea eax, [ebp*4]
    cpu.eax = x86::reg32(cpu.ebp * 4);
    // 0052ea1d  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0052ea1f  8b542404               -mov edx, dword ptr [esp + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0052ea23  8910                   -mov dword ptr [eax], edx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.edx;
    // 0052ea25  a150b1a000             -mov eax, dword ptr [0xa0b150]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531152) /* 0xa0b150 */);
    // 0052ea2a  c6042800               -mov byte ptr [eax + ebp], 0
    app->getMemory<x86::reg8>(cpu.eax + cpu.ebp * 1) = 0 /*0x0*/;
L_0x0052ea2e:
    // 0052ea2e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0052ea30:
    // 0052ea30  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0052ea33  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ea34  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ea35  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ea36  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ea37  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ea38  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ea39  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_52ea3c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052ea3c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052ea3d  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052ea3e  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052ea3f  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052ea40  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052ea41  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0052ea43  89d5                   -mov ebp, edx
    cpu.ebp = cpu.edx;
    // 0052ea45  8b3554b1a000           -mov esi, dword ptr [0xa0b154]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */);
    // 0052ea4b  e9cb000000             -jmp 0x52eb1b
    goto L_0x0052eb1b;
L_0x0052ea50:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052ea50  8a27                   -mov ah, byte ptr [edi]
    cpu.ah = app->getMemory<x86::reg8>(cpu.edi);
    // 0052ea52  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0052ea54  84e4                   +test ah, ah
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & cpu.ah));
    // 0052ea56  0f84bc000000           -je 0x52eb18
    if (cpu.flags.zf)
    {
        goto L_0x0052eb18;
    }
L_0x0052ea5c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052ea5c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052ea5e  8a02                   -mov al, byte ptr [edx]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx);
    // 0052ea60  e87b03fcff             -call 0x4eede0
    cpu.esp -= 4;
    nfs3hp::watcom::toupper(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ea65  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0052ea67  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052ea69  8a03                   -mov al, byte ptr [ebx]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx);
    // 0052ea6b  e87003fcff             -call 0x4eede0
    cpu.esp -= 4;
    nfs3hp::watcom::toupper(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ea70  39c1                   +cmp ecx, eax
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052ea72  0f85a0000000           -jne 0x52eb18
    if (!cpu.flags.zf)
    {
        goto L_0x0052eb18;
    }
    // 0052ea78  803a3d                 +cmp byte ptr [edx], 0x3d
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edx);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(61 /*0x3d*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052ea7b  0f858a000000           -jne 0x52eb0b
    if (!cpu.flags.zf)
    {
        goto L_0x0052eb0b;
    }
    // 0052ea81  8b1554b1a000           -mov edx, dword ptr [0xa0b154]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */);
    // 0052ea87  89f7                   -mov edi, esi
    cpu.edi = cpu.esi;
    // 0052ea89  29d7                   -sub edi, edx
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0052ea8b  c1ff02                 -sar edi, 2
    cpu.edi = x86::reg32(x86::sreg32(cpu.edi) >> (2 /*0x2*/ % 32));
    // 0052ea8e  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 0052ea90  0f846c000000           -je 0x52eb02
    if (cpu.flags.zf)
    {
        goto L_0x0052eb02;
    }
    // 0052ea96  8b16                   -mov edx, dword ptr [esi]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi);
    // 0052ea98  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 0052ea9a  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0052ea9c  740f                   -je 0x52eaad
    if (cpu.flags.zf)
    {
        goto L_0x0052eaad;
    }
L_0x0052ea9e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052ea9e  8b4104                 -mov eax, dword ptr [ecx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 0052eaa1  8901                   -mov dword ptr [ecx], eax
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.eax;
    // 0052eaa3  8b5904                 -mov ebx, dword ptr [ecx + 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 0052eaa6  83c104                 -add ecx, 4
    (cpu.ecx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0052eaa9  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0052eaab  75f1                   -jne 0x52ea9e
    if (!cpu.flags.zf)
    {
        goto L_0x0052ea9e;
    }
L_0x0052eaad:
    // 0052eaad  8b3550b1a000           -mov esi, dword ptr [0xa0b150]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(10531152) /* 0xa0b150 */);
    // 0052eab3  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0052eab5  7443                   -je 0x52eafa
    if (cpu.flags.zf)
    {
        goto L_0x0052eafa;
    }
    // 0052eab7  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0052eab9  803c0700               +cmp byte ptr [edi + eax], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edi + cpu.eax * 1);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052eabd  7407                   -je 0x52eac6
    if (cpu.flags.zf)
    {
        goto L_0x0052eac6;
    }
    // 0052eabf  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0052eac1  e82a8ffcff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052eac6:
    // 0052eac6  8b2d54b1a000           -mov ebp, dword ptr [0xa0b154]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */);
    // 0052eacc  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 0052eace  29ee                   -sub esi, ebp
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.ebp));
    // 0052ead0  8b1550b1a000           -mov edx, dword ptr [0xa0b150]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10531152) /* 0xa0b150 */);
    // 0052ead6  c1fe02                 -sar esi, 2
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (2 /*0x2*/ % 32));
    // 0052ead9  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052eadb  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0052eadd  e85e9bfeff             -call 0x518640
    cpu.esp -= 4;
    nfs3hp::watcom::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052eae2  890d50b1a000           -mov dword ptr [0xa0b150], ecx
    app->getMemory<x86::reg32>(x86::reg32(10531152) /* 0xa0b150 */) = cpu.ecx;
    // 0052eae8  39f7                   +cmp edi, esi
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052eaea  7d0e                   -jge 0x52eafa
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0052eafa;
    }
    // 0052eaec  8d040f                 -lea eax, [edi + ecx]
    cpu.eax = x86::reg32(cpu.edi + cpu.ecx * 1);
L_0x0052eaef:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052eaef  40                     -inc eax
    (cpu.eax)++;
    // 0052eaf0  8a10                   -mov dl, byte ptr [eax]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax);
    // 0052eaf2  47                     -inc edi
    (cpu.edi)++;
    // 0052eaf3  8850ff                 -mov byte ptr [eax - 1], dl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-1) /* -0x1 */) = cpu.dl;
    // 0052eaf6  39f7                   +cmp edi, esi
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052eaf8  7cf5                   -jl 0x52eaef
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0052eaef;
    }
L_0x0052eafa:
    // 0052eafa  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052eafc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eafd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eafe  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eaff  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eb00  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eb01  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052eb02:
    // 0052eb02  8d4701                 -lea eax, [edi + 1]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 0052eb05  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eb06  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eb07  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eb08  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eb09  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eb0a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052eb0b:
    // 0052eb0b  42                     -inc edx
    (cpu.edx)++;
    // 0052eb0c  8a4b01                 -mov cl, byte ptr [ebx + 1]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 0052eb0f  43                     -inc ebx
    (cpu.ebx)++;
    // 0052eb10  84c9                   +test cl, cl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & cpu.cl));
    // 0052eb12  0f8544ffffff           -jne 0x52ea5c
    if (!cpu.flags.zf)
    {
        goto L_0x0052ea5c;
    }
L_0x0052eb18:
    // 0052eb18  83c604                 -add esi, 4
    (cpu.esi) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x0052eb1b:
    // 0052eb1b  8b16                   -mov edx, dword ptr [esi]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi);
    // 0052eb1d  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0052eb1f  0f852bffffff           -jne 0x52ea50
    if (!cpu.flags.zf)
    {
        goto L_0x0052ea50;
    }
    // 0052eb25  a154b1a000             -mov eax, dword ptr [0xa0b154]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */);
    // 0052eb2a  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 0052eb2c  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 0052eb2f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eb30  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eb31  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eb32  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eb33  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eb34  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_52eb40: native, nfs3hp::dead::stub_0x52eb40 */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_52eb70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052eb70  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052eb71  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052eb72  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052eb73  06                     -push es
    app->getMemory<x86::reg16>(cpu.esp-4) = cpu.es;
    cpu.esp -= 4;
    // 0052eb74  0fa0                   -push fs
    app->getMemory<x86::reg16>(cpu.esp-4) = cpu.fs;
    cpu.esp -= 4;
    // 0052eb76  0fa8                   -push gs
    app->getMemory<x86::reg16>(cpu.esp-4) = cpu.gs;
    cpu.esp -= 4;
    // 0052eb78  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052eb79  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0052eb7b  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0052eb7e  8b7520                 -mov esi, dword ptr [ebp + 0x20]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(32) /* 0x20 */);
    // 0052eb81  8b1520649f00           -mov edx, dword ptr [0x9f6420]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10445856) /* 0x9f6420 */);
    // 0052eb87  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 0052eb89  8b7e04                 -mov edi, dword ptr [esi + 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0052eb8c  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0052eb8f  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0052eb91  7531                   -jne 0x52ebc4
    if (!cpu.flags.zf)
    {
        goto L_0x0052ebc4;
    }
    // 0052eb93  a170af5600             -mov eax, dword ptr [0x56af70]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5681008) /* 0x56af70 */);
    // 0052eb98  83c003                 -add eax, 3
    (cpu.eax) += x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0052eb9b  24fc                   -and al, 0xfc
    cpu.al &= x86::reg8(x86::sreg8(252 /*0xfc*/));
    // 0052eb9d  29c4                   -sub esp, eax
    (cpu.esp) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0052eb9f  89e1                   -mov ecx, esp
    cpu.ecx = cpu.esp;
    // 0052eba1  8b1d70af5600           -mov ebx, dword ptr [0x56af70]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5681008) /* 0x56af70 */);
    // 0052eba7  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052eba9  e8921afbff             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ebae  a170af5600             -mov eax, dword ptr [0x56af70]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5681008) /* 0x56af70 */);
    // 0052ebb3  8981f0000000           -mov dword ptr [ecx + 0xf0], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(240) /* 0xf0 */) = cpu.eax;
    // 0052ebb9  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052ebbb  e8c8fcfeff             -call 0x51e888
    cpu.esp -= 4;
    sub_51e888(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ebc0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052ebc2  7431                   -je 0x52ebf5
    if (cpu.flags.zf)
    {
        goto L_0x0052ebf5;
    }
L_0x0052ebc4:
    // 0052ebc4  ff1564775600           -call dword ptr [0x567764]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666660) /* 0x567764 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ebca  8b5e0c                 -mov ebx, dword ptr [esi + 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 0052ebcd  05de000000             -add eax, 0xde
    (cpu.eax) += x86::reg32(x86::sreg32(222 /*0xde*/));
    // 0052ebd2  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052ebd3  894610                 -mov dword ptr [esi + 0x10], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */) = cpu.eax;
    // 0052ebd6  2eff15e8455300         -call dword ptr cs:[0x5345e8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457384) /* 0x5345e8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ebdd  8d45f4                 -lea eax, [ebp - 0xc]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0052ebe0  e8eb04ffff             -call 0x51f0d0
    cpu.esp -= 4;
    nfs3hp::watcom::InitExceptionHandling(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ebe5  ff15a4775600           -call dword ptr [0x5677a4]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666724) /* 0x5677a4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ebeb  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0052ebed  ff55fc                 -call dword ptr [ebp - 4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ebf0  e827dcffff             -call 0x52c81c
    cpu.esp -= 4;
    sub_52c81c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052ebf5:
    // 0052ebf5  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0052ebf7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ebf8  0fa9                   -pop gs
    cpu.gs = app->getMemory<x86::reg16>(cpu.esp);
    cpu.esp += 4;
    // 0052ebfa  0fa1                   -pop fs
    cpu.fs = app->getMemory<x86::reg16>(cpu.esp);
    cpu.esp += 4;
    // 0052ebfc  07                     -pop es
    cpu.es = app->getMemory<x86::reg16>(cpu.esp);
    cpu.esp += 4;
    // 0052ebfd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ebfe  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ebff  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ec00  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_52ec04(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052ec04  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052ec05  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052ec06  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052ec07  06                     -push es
    app->getMemory<x86::reg16>(cpu.esp-4) = cpu.es;
    cpu.esp -= 4;
    // 0052ec08  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052ec09  83ec38                 -sub esp, 0x38
    (cpu.esp) -= x86::reg32(x86::sreg32(56 /*0x38*/));
    // 0052ec0c  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0052ec0e  89d5                   -mov ebp, edx
    cpu.ebp = cpu.edx;
    // 0052ec10  833d60775600ff         +cmp dword ptr [0x567760], -1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052ec17  7512                   -jne 0x52ec2b
    if (!cpu.flags.zf)
    {
        goto L_0x0052ec2b;
    }
    // 0052ec19  e80afcfeff             -call 0x51e828
    cpu.esp -= 4;
    sub_51e828(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ec1e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052ec20  0f84be000000           -je 0x52ece4
    if (cpu.flags.zf)
    {
        goto L_0x0052ece4;
    }
    // 0052ec26  e839fdfeff             -call 0x51e964
    cpu.esp -= 4;
    sub_51e964(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052ec2b:
    // 0052ec2b  89e7                   -mov edi, esp
    cpu.edi = cpu.esp;
    // 0052ec2d  894c2420               -mov dword ptr [esp + 0x20], ecx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(32) /* 0x20 */) = cpu.ecx;
    // 0052ec31  895c2424               -mov dword ptr [esp + 0x24], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(36) /* 0x24 */) = cpu.ebx;
    // 0052ec35  2eff1510455300         -call dword ptr cs:[0x534510]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457168) /* 0x534510 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ec3c  89442428               -mov dword ptr [esp + 0x28], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(40) /* 0x28 */) = cpu.eax;
    // 0052ec40  8cd8                   -mov eax, ds
    cpu.eax = cpu.ds;
    // 0052ec42  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0052ec44  8ec0                   -mov es, eax
    cpu.es = cpu.eax;
    // 0052ec46  be34215500             -mov esi, 0x552134
    cpu.esi = 5579060 /*0x552134*/;
    // 0052ec4b  895c2430               -mov dword ptr [esp + 0x30], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(48) /* 0x30 */) = cpu.ebx;
    // 0052ec4f  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 0052ec54  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 0052ec55  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
    app->getMemory<x86::reg32>(cpu.ees + cpu.edi) = app->getMemory<x86::reg32>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 4;
        cpu.esi -= 4;
    }
    else
    {
        cpu.edi += 4;
        cpu.esi += 4;
    }
    // 0052ec56  a4                     -movsb byte ptr es:[edi], byte ptr [esi]
    app->getMemory<x86::reg8>(cpu.ees + cpu.edi) = app->getMemory<x86::reg8>(cpu.esi);
    if (cpu.flags.df)
    {
        cpu.edi -= 1;
        cpu.esi -= 1;
    }
    else
    {
        cpu.edi += 1;
        cpu.esi += 1;
    }
    // 0052ec57  2eff150c455300         -call dword ptr cs:[0x53450c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457164) /* 0x53450c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ec5e  8d542408               -lea edx, [esp + 8]
    cpu.edx = x86::reg32(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0052ec62  e885f4feff             -call 0x51e0ec
    cpu.esp -= 4;
    nfs3hp::watcom::itoa(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ec67  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052ec69  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052ec6a  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0052ec6c  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0052ec6e  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0052ec70  2eff1594445300         -call dword ptr cs:[0x534494]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457044) /* 0x534494 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ec77  8944242c               -mov dword ptr [esp + 0x2c], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(44) /* 0x2c */) = cpu.eax;
    // 0052ec7b  8d442434               -lea eax, [esp + 0x34]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(52) /* 0x34 */);
    // 0052ec7f  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052ec80  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0052ec82  8d442428               -lea eax, [esp + 0x28]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(40) /* 0x28 */);
    // 0052ec86  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052ec87  81c5ff0f0000           -add ebp, 0xfff
    (cpu.ebp) += x86::reg32(x86::sreg32(4095 /*0xfff*/));
    // 0052ec8d  6870eb5200             -push 0x52eb70
    app->getMemory<x86::reg32>(cpu.esp-4) = 5434224 /*0x52eb70*/;
    cpu.esp -= 4;
    // 0052ec92  81e500f0ffff           -and ebp, 0xfffff000
    cpu.ebp &= x86::reg32(x86::sreg32(4294963200 /*0xfffff000*/));
    // 0052ec98  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052ec99  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0052ec9b  2eff15a4445300         -call dword ptr cs:[0x5344a4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457060) /* 0x5344a4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052eca2  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0052eca4  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 0052eca6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052eca8  750a                   -jne 0x52ecb4
    if (!cpu.flags.zf)
    {
        goto L_0x0052ecb4;
    }
    // 0052ecaa  c7442434ffffffff       -mov dword ptr [esp + 0x34], 0xffffffff
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(52) /* 0x34 */) = 4294967295 /*0xffffffff*/;
    // 0052ecb2  eb22                   -jmp 0x52ecd6
    goto L_0x0052ecd6;
L_0x0052ecb4:
    // 0052ecb4  6aff                   -push -1
    app->getMemory<x86::reg32>(cpu.esp-4) = -1 /*-0x1*/;
    cpu.esp -= 4;
    // 0052ecb6  8b4c2430               -mov ecx, dword ptr [esp + 0x30]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(48) /* 0x30 */);
    // 0052ecba  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052ecbb  2eff1534465300         -call dword ptr cs:[0x534634]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457460) /* 0x534634 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ecc2  8b742430               -mov esi, dword ptr [esp + 0x30]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(48) /* 0x30 */);
    // 0052ecc6  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0052ecc8  7404                   -je 0x52ecce
    if (cpu.flags.zf)
    {
        goto L_0x0052ecce;
    }
    // 0052ecca  891e                   -mov dword ptr [esi], ebx
    app->getMemory<x86::reg32>(cpu.esi) = cpu.ebx;
    // 0052eccc  eb08                   -jmp 0x52ecd6
    goto L_0x0052ecd6;
L_0x0052ecce:
    // 0052ecce  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052eccf  2eff1588445300         -call dword ptr cs:[0x534488]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457032) /* 0x534488 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052ecd6:
    // 0052ecd6  8b44242c               -mov eax, dword ptr [esp + 0x2c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(44) /* 0x2c */);
    // 0052ecda  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052ecdb  2eff1588445300         -call dword ptr cs:[0x534488]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457032) /* 0x534488 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ece2  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
L_0x0052ece4:
    // 0052ece4  83c438                 -add esp, 0x38
    (cpu.esp) += x86::reg32(x86::sreg32(56 /*0x38*/));
    // 0052ece7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ece8  07                     -pop es
    cpu.es = app->getMemory<x86::reg16>(cpu.esp);
    cpu.esp += 4;
    // 0052ece9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ecea  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eceb  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ecec  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_52ecf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052ecf0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052ecf1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052ecf2  ff15a8775600           -call dword ptr [0x5677a8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666728) /* 0x5677a8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ecf8  e81f04ffff             -call 0x51f11c
    cpu.esp -= 4;
    nfs3hp::watcom::DoneExceptionHandler(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ecfd  833d20649f0000         +cmp dword ptr [0x9f6420], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10445856) /* 0x9f6420 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052ed04  750a                   -jne 0x52ed10
    if (!cpu.flags.zf)
    {
        goto L_0x0052ed10;
    }
    // 0052ed06  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0052ed0b  e8ccfbfeff             -call 0x51e8dc
    cpu.esp -= 4;
    sub_51e8dc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052ed10:
    // 0052ed10  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0052ed12  2eff15c4445300         -call dword ptr cs:[0x5344c4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457092) /* 0x5344c4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ed19  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ed1a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ed1b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 */
void Application::sub_52ed20(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052ed20  ff1553785600           -call dword ptr [0x567853]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666899) /* 0x567853 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ed26  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_52ed30: native, nfs3hp::watcom::mbsnextc */

/* sub_52ed70: native, nfs3hp::watcom::mbctoupper */

/* sub_52ede0: native, nfs3hp::watcom::mbvtop */

/* align: skip  */
void Application::sub_52edfa(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052edfa  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052edfc  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 */
void Application::sub_52ee00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052ee00  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052ee01  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052ee03  7c49                   -jl 0x52ee4e
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0052ee4e;
    }
L_0x0052ee05:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052ee05  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0052ee07  7c49                   -jl 0x52ee52
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0052ee52;
    }
L_0x0052ee09:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052ee09  8b35f84f5600           -mov esi, dword ptr [0x564ff8]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5656568) /* 0x564ff8 */);
    // 0052ee0f  39f0                   +cmp eax, esi
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052ee11  7e02                   -jle 0x52ee15
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0052ee15;
    }
    // 0052ee13  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
L_0x0052ee15:
    // 0052ee15  39f3                   +cmp ebx, esi
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052ee17  7e02                   -jle 0x52ee1b
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0052ee1b;
    }
    // 0052ee19  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
L_0x0052ee1b:
    // 0052ee1b  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0052ee1d  7c37                   -jl 0x52ee56
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0052ee56;
    }
L_0x0052ee1f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052ee1f  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0052ee21  7c37                   -jl 0x52ee5a
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0052ee5a;
    }
L_0x0052ee23:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052ee23  8b35fc4f5600           -mov esi, dword ptr [0x564ffc]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5656572) /* 0x564ffc */);
    // 0052ee29  39f2                   +cmp edx, esi
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052ee2b  7e02                   -jle 0x52ee2f
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0052ee2f;
    }
    // 0052ee2d  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
L_0x0052ee2f:
    // 0052ee2f  39f1                   +cmp ecx, esi
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.esi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052ee31  7e02                   -jle 0x52ee35
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0052ee35;
    }
    // 0052ee33  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
L_0x0052ee35:
    // 0052ee35  891d08505600           -mov dword ptr [0x565008], ebx
    app->getMemory<x86::reg32>(x86::reg32(5656584) /* 0x565008 */) = cpu.ebx;
    // 0052ee3b  891504505600           -mov dword ptr [0x565004], edx
    app->getMemory<x86::reg32>(x86::reg32(5656580) /* 0x565004 */) = cpu.edx;
    // 0052ee41  890d0c505600           -mov dword ptr [0x56500c], ecx
    app->getMemory<x86::reg32>(x86::reg32(5656588) /* 0x56500c */) = cpu.ecx;
    // 0052ee47  a300505600             -mov dword ptr [0x565000], eax
    app->getMemory<x86::reg32>(x86::reg32(5656576) /* 0x565000 */) = cpu.eax;
    // 0052ee4c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ee4d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052ee4e:
    // 0052ee4e  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0052ee50  ebb3                   -jmp 0x52ee05
    goto L_0x0052ee05;
L_0x0052ee52:
    // 0052ee52  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0052ee54  ebb3                   -jmp 0x52ee09
    goto L_0x0052ee09;
L_0x0052ee56:
    // 0052ee56  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0052ee58  ebc5                   -jmp 0x52ee1f
    goto L_0x0052ee1f;
L_0x0052ee5a:
    // 0052ee5a  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 0052ee5c  ebc5                   -jmp 0x52ee23
    goto L_0x0052ee23;
}

/* align: skip 0x00 0x00 */
void Application::sub_52ee60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052ee60  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052ee61  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052ee62  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0052ee65  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0052ee67  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0052ee69  89e2                   -mov edx, esp
    cpu.edx = cpu.esp;
    // 0052ee6b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052ee6d  e86effffff             -call 0x52ede0
    cpu.esp -= 4;
    nfs3hp::watcom::mbvtop(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ee72  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052ee74  30d2                   -xor dl, dl
    cpu.dl ^= x86::reg8(x86::sreg8(cpu.dl));
    // 0052ee76  e835c7ffff             -call 0x52b5b0
    cpu.esp -= 4;
    nfs3hp::watcom::mbclen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ee7b  881404                 -mov byte ptr [esp + eax], dl
    app->getMemory<x86::reg8>(cpu.esp + cpu.eax * 1) = cpu.dl;
L_0x0052ee7e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052ee7e  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0052ee80  e83b62ffff             -call 0x5250c0
    cpu.esp -= 4;
    nfs3hp::watcom::mbterm(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ee85  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052ee87  7518                   -jne 0x52eea1
    if (!cpu.flags.zf)
    {
        goto L_0x0052eea1;
    }
    // 0052ee89  89e2                   -mov edx, esp
    cpu.edx = cpu.esp;
    // 0052ee8b  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0052ee8d  e80ef5ffff             -call 0x52e3a0
    cpu.esp -= 4;
    nfs3hp::watcom::mbccmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ee92  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052ee94  740b                   -je 0x52eea1
    if (cpu.flags.zf)
    {
        goto L_0x0052eea1;
    }
    // 0052ee96  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0052ee98  e86362ffff             -call 0x525100
    cpu.esp -= 4;
    nfs3hp::watcom::mbsinc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ee9d  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0052ee9f  ebdd                   -jmp 0x52ee7e
    goto L_0x0052ee7e;
L_0x0052eea1:
    // 0052eea1  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0052eea3  e81862ffff             -call 0x5250c0
    cpu.esp -= 4;
    nfs3hp::watcom::mbterm(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052eea8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052eeaa  7404                   -je 0x52eeb0
    if (cpu.flags.zf)
    {
        goto L_0x0052eeb0;
    }
    // 0052eeac  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0052eeae  7504                   -jne 0x52eeb4
    if (!cpu.flags.zf)
    {
        goto L_0x0052eeb4;
    }
L_0x0052eeb0:
    // 0052eeb0  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0052eeb2  eb02                   -jmp 0x52eeb6
    goto L_0x0052eeb6;
L_0x0052eeb4:
    // 0052eeb4  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0052eeb6:
    // 0052eeb6  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0052eeb9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eeba  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052eebb  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_52eec0: native, nfs3hp::watcom::mbcupper932 */

/* sub_52eee0: native, nfs3hp::watcom::mbclower */

/* sub_52ef30: native, nfs3hp::watcom::mbclegal */

/* sub_52ef70: native, nfs3hp::watcom::mbbtrail */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
/* data blob: 68b956000000000003ffff000000000058b956000400000003ffff000000000048b956000800000003ffff8000000000000000000c0000000cffff0000000000000000000d0000000cffff0000000000000000000e0000000cffff8000000000000000000f0000000cffff80000000001800000010000000020000001000000007000000b0ef5200000000000000000068b956000000000003ffff800001000058b956000400000003ffff800001000048b956000800000003ffff8000010000c8b956000c00000003ffff8000010000b8b956001000000003ffff8000010000a8b956001400000003ffff800001000098b956001800000003ffff800001000098b956001c00000003ffff800001000088b956002000000010ffff800000000088b956002400000010ffff800000000088b956002800000010ffff800000000088b956002c00000010ffff800000000000000000300000000cffff800000000000000000310000000cffff800000000000000000320000000cffff800000000000000000330000000cffff800000000000000000340000000cffff800000000000000000350000000cffff800000000000000000360000000cffff800000000000000000370000000cffff800000000000000000380000000cffff800000000000000000390000000cffff8000000000000000003a0000000cffff8000000000000000003b0000000cffff8000000000000000003c0000000cffff8000000000000000003d0000000cffff8000000000000000003e0000000cffff8000000000000000003f0000000cffff800000000000000000400000000cffff800000000000000000410000000cffff800000000000000000420000000cffff800000000000000000430000000cffff800000000000000000440000000cffff800000000000000000450000000cffff800000000000000000460000000cffff800000000000000000470000000cffff800000000000000000480000000cffff800000000000000000490000000cffff8000000000000000004a0000000cffff8000000000000000004b0000000cffff8000000000000000004c0000000cffff8000000000000000004d0000000cffff8000000000000000004e0000000cffff8000000000000000004f0000000cffff8000000000180000001000000001000000500000002c00000040f05200 */
void Application::sub_52f318(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052f318  833d2cd1560000         +cmp dword ptr [0x56d12c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689644) /* 0x56d12c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052f31f  751e                   -jne 0x52f33f
    if (!cpu.flags.zf)
    {
        goto L_0x0052f33f;
    }
    // 0052f321  68d8b95600             -push 0x56b9d8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5683672 /*0x56b9d8*/;
    cpu.esp -= 4;
    // 0052f326  ff158c455300           -call dword ptr [0x53458c]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457292) /* 0x53458c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052f32c  a32cd15600             -mov dword ptr [0x56d12c], eax
    app->getMemory<x86::reg32>(x86::reg32(5689644) /* 0x56d12c */) = cpu.eax;
    // 0052f331  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052f333  750a                   -jne 0x52f33f
    if (!cpu.flags.zf)
    {
        goto L_0x0052f33f;
    }
    // 0052f335  c7052cd15600ffffffff   -mov dword ptr [0x56d12c], 0xffffffff
    app->getMemory<x86::reg32>(x86::reg32(5689644) /* 0x56d12c */) = 4294967295 /*0xffffffff*/;
L_0x0052f33f:
    // 0052f33f  833d2cd15600ff         +cmp dword ptr [0x56d12c], -1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689644) /* 0x56d12c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052f346  b849000080             -mov eax, 0x80000049
    cpu.eax = 2147483721 /*0x80000049*/;
    // 0052f34b  7420                   -je 0x52f36d
    if (cpu.flags.zf)
    {
        goto L_0x0052f36d;
    }
    // 0052f34d  ff742404               -push dword ptr [esp + 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    // 0052f351  ff352cd15600           -push dword ptr [0x56d12c]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(x86::reg32(5689644) /* 0x56d12c */);
    cpu.esp -= 4;
    // 0052f357  ff1558455300           -call dword ptr [0x534558]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052f35d  8b4c2408               -mov ecx, dword ptr [esp + 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0052f361  83f801                 +cmp eax, 1
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052f364  8901                   -mov dword ptr [ecx], eax
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.eax;
    // 0052f366  1bc0                   -sbb eax, eax
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.eax) + cpu.flags.cf);
    // 0052f368  2549000080             -and eax, 0x80000049
    cpu.eax &= x86::reg32(x86::sreg32(2147483721 /*0x80000049*/));
L_0x0052f36d:
    // 0052f36d  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* sub_52f370: native, nfs3hp::dead::stub_0x52f370 */

/* sub_52f3bb: native, nfs3hp::dead::stub_0x52f3bb */

/* sub_52f406: native, nfs3hp::dead::stub_0x52f406 */

/* sub_52f451: native, nfs3hp::dead::stub_0x52f451 */

/* sub_52f49c: native, nfs3hp::dead::stub_0x52f49c */

/* sub_52f4e3: native, nfs3hp::dead::stub_0x52f4e3 */

/* align: skip  */
void Application::sub_52f535(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052f535  833d4cd1560000         +cmp dword ptr [0x56d14c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689676) /* 0x56d14c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052f53c  751f                   -jne 0x52f55d
    if (!cpu.flags.zf)
    {
        goto L_0x0052f55d;
    }
    // 0052f53e  684cd15600             -push 0x56d14c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5689676 /*0x56d14c*/;
    cpu.esp -= 4;
    // 0052f543  6850ba5600             -push 0x56ba50
    app->getMemory<x86::reg32>(cpu.esp-4) = 5683792 /*0x56ba50*/;
    cpu.esp -= 4;
    // 0052f548  e8cbfdffff             -call 0x52f318
    cpu.esp -= 4;
    sub_52f318(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052f54d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052f54f  740c                   -je 0x52f55d
    if (cpu.flags.zf)
    {
        goto L_0x0052f55d;
    }
    // 0052f551  c7054cd15600ffffffff   -mov dword ptr [0x56d14c], 0xffffffff
    app->getMemory<x86::reg32>(x86::reg32(5689676) /* 0x56d14c */) = 4294967295 /*0xffffffff*/;
    // 0052f55b  eb20                   -jmp 0x52f57d
    goto L_0x0052f57d;
L_0x0052f55d:
    // 0052f55d  833d4cd15600ff         +cmp dword ptr [0x56d14c], -1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689676) /* 0x56d14c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052f564  b849000080             -mov eax, 0x80000049
    cpu.eax = 2147483721 /*0x80000049*/;
    // 0052f569  7412                   -je 0x52f57d
    if (cpu.flags.zf)
    {
        goto L_0x0052f57d;
    }
    // 0052f56b  ff74240c               -push dword ptr [esp + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 0052f56f  ff74240c               -push dword ptr [esp + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 0052f573  ff74240c               -push dword ptr [esp + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 0052f577  ff154cd15600           -call dword ptr [0x56d14c]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5689676) /* 0x56d14c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052f57d:
    // 0052f57d  c20c00                 -ret 0xc
    cpu.esp += 4+12 /*0xc*/;
    return;
}

/* sub_52f580: native, nfs3hp::dead::stub_0x52f580 */

/* sub_52f5cb: native, nfs3hp::dead::stub_0x52f5cb */

/* sub_52f616: native, nfs3hp::dead::stub_0x52f616 */

/* align: skip  */
void Application::sub_52f661(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052f661  833d5cd1560000         +cmp dword ptr [0x56d15c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689692) /* 0x56d15c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052f668  751f                   -jne 0x52f689
    if (!cpu.flags.zf)
    {
        goto L_0x0052f689;
    }
    // 0052f66a  685cd15600             -push 0x56d15c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5689692 /*0x56d15c*/;
    cpu.esp -= 4;
    // 0052f66f  6898ba5600             -push 0x56ba98
    app->getMemory<x86::reg32>(cpu.esp-4) = 5683864 /*0x56ba98*/;
    cpu.esp -= 4;
    // 0052f674  e89ffcffff             -call 0x52f318
    cpu.esp -= 4;
    sub_52f318(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052f679  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052f67b  740c                   -je 0x52f689
    if (cpu.flags.zf)
    {
        goto L_0x0052f689;
    }
    // 0052f67d  c7055cd15600ffffffff   -mov dword ptr [0x56d15c], 0xffffffff
    app->getMemory<x86::reg32>(x86::reg32(5689692) /* 0x56d15c */) = 4294967295 /*0xffffffff*/;
    // 0052f687  eb18                   -jmp 0x52f6a1
    goto L_0x0052f6a1;
L_0x0052f689:
    // 0052f689  833d5cd15600ff         +cmp dword ptr [0x56d15c], -1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689692) /* 0x56d15c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052f690  b849000080             -mov eax, 0x80000049
    cpu.eax = 2147483721 /*0x80000049*/;
    // 0052f695  740a                   -je 0x52f6a1
    if (cpu.flags.zf)
    {
        goto L_0x0052f6a1;
    }
    // 0052f697  ff742404               -push dword ptr [esp + 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    // 0052f69b  ff155cd15600           -call dword ptr [0x56d15c]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5689692) /* 0x56d15c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052f6a1:
    // 0052f6a1  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* sub_52f6a4: native, nfs3hp::dead::stub_0x52f6a4 */

/* sub_52f6f3: native, nfs3hp::dead::stub_0x52f6f3 */

/* sub_52f742: native, nfs3hp::dead::stub_0x52f742 */

/* sub_52f78d: native, nfs3hp::dead::stub_0x52f78d */

/* sub_52f7d8: native, nfs3hp::dead::stub_0x52f7d8 */

/* sub_52f823: native, nfs3hp::dead::stub_0x52f823 */

/* sub_52f878: native, nfs3hp::dead::stub_0x52f878 */

/* sub_52f8cd: native, nfs3hp::dead::stub_0x52f8cd */

/* sub_52f922: native, nfs3hp::dead::stub_0x52f922 */

/* align: skip  */
void Application::sub_52f969(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052f969  833d84d1560000         +cmp dword ptr [0x56d184], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689732) /* 0x56d184 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052f970  751f                   -jne 0x52f991
    if (!cpu.flags.zf)
    {
        goto L_0x0052f991;
    }
    // 0052f972  6884d15600             -push 0x56d184
    app->getMemory<x86::reg32>(cpu.esp-4) = 5689732 /*0x56d184*/;
    cpu.esp -= 4;
    // 0052f977  6868bb5600             -push 0x56bb68
    app->getMemory<x86::reg32>(cpu.esp-4) = 5684072 /*0x56bb68*/;
    cpu.esp -= 4;
    // 0052f97c  e897f9ffff             -call 0x52f318
    cpu.esp -= 4;
    sub_52f318(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052f981  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052f983  740c                   -je 0x52f991
    if (cpu.flags.zf)
    {
        goto L_0x0052f991;
    }
    // 0052f985  c70584d15600ffffffff   -mov dword ptr [0x56d184], 0xffffffff
    app->getMemory<x86::reg32>(x86::reg32(5689732) /* 0x56d184 */) = 4294967295 /*0xffffffff*/;
    // 0052f98f  eb18                   -jmp 0x52f9a9
    goto L_0x0052f9a9;
L_0x0052f991:
    // 0052f991  833d84d15600ff         +cmp dword ptr [0x56d184], -1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689732) /* 0x56d184 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052f998  b849000080             -mov eax, 0x80000049
    cpu.eax = 2147483721 /*0x80000049*/;
    // 0052f99d  740a                   -je 0x52f9a9
    if (cpu.flags.zf)
    {
        goto L_0x0052f9a9;
    }
    // 0052f99f  ff742404               -push dword ptr [esp + 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    cpu.esp -= 4;
    // 0052f9a3  ff1584d15600           -call dword ptr [0x56d184]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5689732) /* 0x56d184 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052f9a9:
    // 0052f9a9  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* sub_52f9ac: native, nfs3hp::dead::stub_0x52f9ac */

/* sub_52f9fe: native, nfs3hp::dead::stub_0x52f9fe */

/* sub_52fa4d: native, nfs3hp::dead::stub_0x52fa4d */

/* sub_52fa98: native, nfs3hp::dead::stub_0x52fa98 */

/* sub_52fae3: native, nfs3hp::dead::stub_0x52fae3 */

/* align: skip  */
void Application::sub_52fb2e(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052fb2e  833d9cd1560000         +cmp dword ptr [0x56d19c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689756) /* 0x56d19c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052fb35  751f                   -jne 0x52fb56
    if (!cpu.flags.zf)
    {
        goto L_0x0052fb56;
    }
    // 0052fb37  689cd15600             -push 0x56d19c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5689756 /*0x56d19c*/;
    cpu.esp -= 4;
    // 0052fb3c  68c8bb5600             -push 0x56bbc8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5684168 /*0x56bbc8*/;
    cpu.esp -= 4;
    // 0052fb41  e8d2f7ffff             -call 0x52f318
    cpu.esp -= 4;
    sub_52f318(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052fb46  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052fb48  740c                   -je 0x52fb56
    if (cpu.flags.zf)
    {
        goto L_0x0052fb56;
    }
    // 0052fb4a  c7059cd15600ffffffff   -mov dword ptr [0x56d19c], 0xffffffff
    app->getMemory<x86::reg32>(x86::reg32(5689756) /* 0x56d19c */) = 4294967295 /*0xffffffff*/;
    // 0052fb54  eb20                   -jmp 0x52fb76
    goto L_0x0052fb76;
L_0x0052fb56:
    // 0052fb56  833d9cd15600ff         +cmp dword ptr [0x56d19c], -1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689756) /* 0x56d19c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052fb5d  b849000080             -mov eax, 0x80000049
    cpu.eax = 2147483721 /*0x80000049*/;
    // 0052fb62  7412                   -je 0x52fb76
    if (cpu.flags.zf)
    {
        goto L_0x0052fb76;
    }
    // 0052fb64  ff74240c               -push dword ptr [esp + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 0052fb68  ff74240c               -push dword ptr [esp + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 0052fb6c  ff74240c               -push dword ptr [esp + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 0052fb70  ff159cd15600           -call dword ptr [0x56d19c]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5689756) /* 0x56d19c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052fb76:
    // 0052fb76  c20c00                 -ret 0xc
    cpu.esp += 4+12 /*0xc*/;
    return;
}

/* sub_52fb79: native, nfs3hp::dead::stub_0x52fb79 */

/* sub_52fbd1: native, nfs3hp::dead::stub_0x52fbd1 */

/* sub_52fc29: native, nfs3hp::dead::stub_0x52fc29 */

/* sub_52fc81: native, nfs3hp::dead::stub_0x52fc81 */

/* sub_52fcd9: native, nfs3hp::dead::stub_0x52fcd9 */

/* sub_52fd31: native, nfs3hp::dead::stub_0x52fd31 */

/* sub_52fd89: native, nfs3hp::dead::stub_0x52fd89 */

/* sub_52fdd8: native, nfs3hp::dead::stub_0x52fdd8 */

/* sub_52fe27: native, nfs3hp::dead::stub_0x52fe27 */

/* sub_52fe76: native, nfs3hp::dead::stub_0x52fe76 */

/* sub_52fec8: native, nfs3hp::dead::stub_0x52fec8 */

/* sub_52ff1d: native, nfs3hp::dead::stub_0x52ff1d */

/* sub_52ff72: native, nfs3hp::dead::stub_0x52ff72 */

/* align: skip  */
void Application::sub_52ffc7(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052ffc7  833dd4d1560000         +cmp dword ptr [0x56d1d4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689812) /* 0x56d1d4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052ffce  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052ffcf  8bec                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0052ffd1  751f                   -jne 0x52fff2
    if (!cpu.flags.zf)
    {
        goto L_0x0052fff2;
    }
    // 0052ffd3  68d4d15600             -push 0x56d1d4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5689812 /*0x56d1d4*/;
    cpu.esp -= 4;
    // 0052ffd8  68c8bc5600             -push 0x56bcc8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5684424 /*0x56bcc8*/;
    cpu.esp -= 4;
    // 0052ffdd  e836f3ffff             -call 0x52f318
    cpu.esp -= 4;
    sub_52f318(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ffe2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052ffe4  740c                   -je 0x52fff2
    if (cpu.flags.zf)
    {
        goto L_0x0052fff2;
    }
    // 0052ffe6  c705d4d15600ffffffff   -mov dword ptr [0x56d1d4], 0xffffffff
    app->getMemory<x86::reg32>(x86::reg32(5689812) /* 0x56d1d4 */) = 4294967295 /*0xffffffff*/;
    // 0052fff0  eb23                   -jmp 0x530015
    goto L_0x00530015;
L_0x0052fff2:
    // 0052fff2  833dd4d15600ff         +cmp dword ptr [0x56d1d4], -1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689812) /* 0x56d1d4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052fff9  b849000080             -mov eax, 0x80000049
    cpu.eax = 2147483721 /*0x80000049*/;
    // 0052fffe  7415                   -je 0x530015
    if (cpu.flags.zf)
    {
        goto L_0x00530015;
    }
    // 00530000  ff7518                 -push dword ptr [ebp + 0x18]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    // 00530003  ff7514                 -push dword ptr [ebp + 0x14]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    // 00530006  ff7510                 -push dword ptr [ebp + 0x10]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    cpu.esp -= 4;
    // 00530009  ff750c                 -push dword ptr [ebp + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 0053000c  ff7508                 -push dword ptr [ebp + 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    // 0053000f  ff15d4d15600           -call dword ptr [0x56d1d4]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5689812) /* 0x56d1d4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00530015:
    // 00530015  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00530016  c21400                 -ret 0x14
    cpu.esp += 4+20 /*0x14*/;
    return;
}

/* align: skip  */
void Application::sub_530019(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00530019  833dd8d1560000         +cmp dword ptr [0x56d1d8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689816) /* 0x56d1d8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00530020  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00530021  8bec                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00530023  751f                   -jne 0x530044
    if (!cpu.flags.zf)
    {
        goto L_0x00530044;
    }
    // 00530025  68d8d15600             -push 0x56d1d8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5689816 /*0x56d1d8*/;
    cpu.esp -= 4;
    // 0053002a  68dcbc5600             -push 0x56bcdc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5684444 /*0x56bcdc*/;
    cpu.esp -= 4;
    // 0053002f  e8e4f2ffff             -call 0x52f318
    cpu.esp -= 4;
    sub_52f318(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00530034  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00530036  740c                   -je 0x530044
    if (cpu.flags.zf)
    {
        goto L_0x00530044;
    }
    // 00530038  c705d8d15600ffffffff   -mov dword ptr [0x56d1d8], 0xffffffff
    app->getMemory<x86::reg32>(x86::reg32(5689816) /* 0x56d1d8 */) = 4294967295 /*0xffffffff*/;
    // 00530042  eb23                   -jmp 0x530067
    goto L_0x00530067;
L_0x00530044:
    // 00530044  833dd8d15600ff         +cmp dword ptr [0x56d1d8], -1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689816) /* 0x56d1d8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0053004b  b849000080             -mov eax, 0x80000049
    cpu.eax = 2147483721 /*0x80000049*/;
    // 00530050  7415                   -je 0x530067
    if (cpu.flags.zf)
    {
        goto L_0x00530067;
    }
    // 00530052  ff7518                 -push dword ptr [ebp + 0x18]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    // 00530055  ff7514                 -push dword ptr [ebp + 0x14]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    // 00530058  ff7510                 -push dword ptr [ebp + 0x10]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    cpu.esp -= 4;
    // 0053005b  ff750c                 -push dword ptr [ebp + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 0053005e  ff7508                 -push dword ptr [ebp + 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    // 00530061  ff15d8d15600           -call dword ptr [0x56d1d8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5689816) /* 0x56d1d8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00530067:
    // 00530067  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00530068  c21400                 -ret 0x14
    cpu.esp += 4+20 /*0x14*/;
    return;
}

/* align: skip  */
void Application::sub_53006b(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0053006b  833ddcd1560000         +cmp dword ptr [0x56d1dc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689820) /* 0x56d1dc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 00530072  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00530073  8bec                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00530075  751f                   -jne 0x530096
    if (!cpu.flags.zf)
    {
        goto L_0x00530096;
    }
    // 00530077  68dcd15600             -push 0x56d1dc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5689820 /*0x56d1dc*/;
    cpu.esp -= 4;
    // 0053007c  68f0bc5600             -push 0x56bcf0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5684464 /*0x56bcf0*/;
    cpu.esp -= 4;
    // 00530081  e892f2ffff             -call 0x52f318
    cpu.esp -= 4;
    sub_52f318(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00530086  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00530088  740c                   -je 0x530096
    if (cpu.flags.zf)
    {
        goto L_0x00530096;
    }
    // 0053008a  c705dcd15600ffffffff   -mov dword ptr [0x56d1dc], 0xffffffff
    app->getMemory<x86::reg32>(x86::reg32(5689820) /* 0x56d1dc */) = 4294967295 /*0xffffffff*/;
    // 00530094  eb23                   -jmp 0x5300b9
    goto L_0x005300b9;
L_0x00530096:
    // 00530096  833ddcd15600ff         +cmp dword ptr [0x56d1dc], -1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5689820) /* 0x56d1dc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0053009d  b849000080             -mov eax, 0x80000049
    cpu.eax = 2147483721 /*0x80000049*/;
    // 005300a2  7415                   -je 0x5300b9
    if (cpu.flags.zf)
    {
        goto L_0x005300b9;
    }
    // 005300a4  ff7518                 -push dword ptr [ebp + 0x18]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    cpu.esp -= 4;
    // 005300a7  ff7514                 -push dword ptr [ebp + 0x14]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    cpu.esp -= 4;
    // 005300aa  ff7510                 -push dword ptr [ebp + 0x10]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    cpu.esp -= 4;
    // 005300ad  ff750c                 -push dword ptr [ebp + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 005300b0  ff7508                 -push dword ptr [ebp + 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    cpu.esp -= 4;
    // 005300b3  ff15dcd15600           -call dword ptr [0x56d1dc]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5689820) /* 0x56d1dc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x005300b9:
    // 005300b9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 005300ba  c21400                 -ret 0x14
    cpu.esp += 4+20 /*0x14*/;
    return;
}

/* sub_5300bd: native, nfs3hp::dead::stub_0x5300bd */

/* sub_530108: native, nfs3hp::dead::stub_0x530108 */

/* sub_530153: native, nfs3hp::dead::stub_0x530153 */

/* sub_53019e: native, nfs3hp::dead::stub_0x53019e */

/* sub_5301e9: native, nfs3hp::dead::stub_0x5301e9 */

/* sub_530234: native, nfs3hp::dead::stub_0x530234 */

/* sub_530286: native, nfs3hp::dead::stub_0x530286 */

/* sub_5302d8: native, nfs3hp::dead::stub_0x5302d8 */

/* sub_530323: native, nfs3hp::dead::stub_0x530323 */

/* sub_53036e: native, nfs3hp::dead::stub_0x53036e */

/* sub_5303b9: native, nfs3hp::dead::stub_0x5303b9 */

/* sub_530404: native, nfs3hp::dead::stub_0x530404 */

/* sub_530459: native, nfs3hp::dead::stub_0x530459 */

}
