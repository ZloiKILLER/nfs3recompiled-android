#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_52a144: native, nfs3hp::port::xaStereoIntroSetup */

/* sub_52a1a0: native, nfs3hp::port::xaStereoStreamDecode */

/* sub_52a354: native, nfs3hp::port::xaStereoStreamSetup */

/* sub_52a3a0: native, nfs3hp::port::xaStereoPacketDecode */

/* sub_52a5b4: native, nfs3hp::port::xaStereoFileSetup */

/* sub_52a610: native, nfs3hp::port::framesPlayedCoded */

/* sub_52a618: native, nfs3hp::port::codedToWords */

/* sub_52a668: native, nfs3hp::port::codedDecode */

/* sub_52a790: native, nfs3hp::port::codedSetup */

/* sub_52a7e0: native, nfs3hp::port::codedStreamDecode */

/* sub_52a970: native, nfs3hp::port::codedStreamSetup */

/* sub_52a9c0: native, nfs3hp::port::codedPacketDecode */

/* sub_52ab70: native, nfs3hp::port::codedPacketSetup */

/* sub_52abd0: native, nfs3hp::snd::dead_0x52abd0 */

/* sub_52abd8: native, nfs3hp::snd::dead_0x52abd8 */

/* sub_52abf8: native, nfs3hp::snd::dead_0x52abf8 */

/* sub_52ad28: native, nfs3hp::snd::dead_0x52ad28 */

/* sub_52ad70: native, nfs3hp::snd::dead_0x52ad70 */

/* sub_52af30: native, nfs3hp::snd::dead_0x52af30 */

/* sub_52af80: native, nfs3hp::snd::dead_0x52af80 */

/* sub_52b160: native, nfs3hp::snd::dead_0x52b160 */

/* sub_52b1c0: native, nfs3hp::port::panLaw */

/* sub_52b230: native, nfs3hp::port::readBigEndian */

/* sub_52b290: native, nfs3hp::watcom::putch */

/* sub_52b300: native, nfs3hp::snd::dead_0x52b300 */

/* sub_52b4d0: native, nfs3hp::snd::dead_0x52b4d0 */

/* sub_52b500: native, nfs3hp::port::setSendLevel */

/* align: skip  */
void Application::sub_52b523(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
  x86::reg16 fsw = 0;
    // 0052b523  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052b524  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0052b526  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052b527  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052b528  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052b529  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052b52a  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0052b52d  8a6518                 -mov ah, byte ptr [ebp + 0x18]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 0052b530  80fc01                 +cmp ah, 1
    {
        x86::reg8 tmp1 = cpu.ah;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(1 /*0x1*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052b533  772f                   -ja 0x52b564
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0052b564;
    }
    // 0052b535  84e4                   +test ah, ah
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & cpu.ah));
    // 0052b537  7524                   -jne 0x52b55d
    if (!cpu.flags.zf)
    {
        goto L_0x0052b55d;
    }
    // 0052b539  d9ee                   +fldz 
    fp0 = 0.0;
    // 0052b53b  dc5d10                 +fcomp qword ptr [ebp + 0x10]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(16) /* 0x10 */)));
    // 0052b53e  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 0052b540  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0052b541  730a                   -jae 0x52b54d
    if (!cpu.flags.cf)
    {
        goto L_0x0052b54d;
    }
    // 0052b543  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 0052b545  894de8                 -mov dword ptr [ebp - 0x18], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.ecx;
    // 0052b548  894dec                 -mov dword ptr [ebp - 0x14], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.ecx;
    // 0052b54b  eb4f                   -jmp 0x52b59c
    goto L_0x0052b59c;
L_0x0052b54d:
    // 0052b54d  7607                   -jbe 0x52b556
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0052b556;
    }
    // 0052b54f  b847800000             -mov eax, 0x8047
    cpu.eax = 32839 /*0x8047*/;
    // 0052b554  eb38                   -jmp 0x52b58e
    goto L_0x0052b58e;
L_0x0052b556:
    // 0052b556  b847400000             -mov eax, 0x4047
    cpu.eax = 16455 /*0x4047*/;
    // 0052b55b  eb31                   -jmp 0x52b58e
    goto L_0x0052b58e;
L_0x0052b55d:
    // 0052b55d  b847200000             -mov eax, 0x2047
    cpu.eax = 8263 /*0x2047*/;
    // 0052b562  eb2a                   -jmp 0x52b58e
    goto L_0x0052b58e;
L_0x0052b564:
    // 0052b564  d9ee                   +fldz 
    fp0 = 0.0;
    // 0052b566  dc5d10                 +fcomp qword ptr [ebp + 0x10]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(16) /* 0x10 */)));
    // 0052b569  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 0052b56b  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0052b56c  720a                   -jb 0x52b578
    if (cpu.flags.cf)
    {
        goto L_0x0052b578;
    }
    // 0052b56e  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0052b570  8955e8                 -mov dword ptr [ebp - 0x18], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.edx;
    // 0052b573  8955ec                 -mov dword ptr [ebp - 0x14], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.edx;
    // 0052b576  eb24                   -jmp 0x52b59c
    goto L_0x0052b59c;
L_0x0052b578:
    // 0052b578  d9ee                   +fldz 
    fp0 = 0.0;
    // 0052b57a  dc5d08                 +fcomp qword ptr [ebp + 8]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(8) /* 0x8 */)));
    // 0052b57d  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 0052b57f  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 0052b580  7307                   -jae 0x52b589
    if (!cpu.flags.cf)
    {
        goto L_0x0052b589;
    }
    // 0052b582  b807810000             -mov eax, 0x8107
    cpu.eax = 33031 /*0x8107*/;
    // 0052b587  eb05                   -jmp 0x52b58e
    goto L_0x0052b58e;
L_0x0052b589:
    // 0052b589  b807110000             -mov eax, 0x1107
    cpu.eax = 4359 /*0x1107*/;
L_0x0052b58e:
    // 0052b58e  8d5d10                 -lea ebx, [ebp + 0x10]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 0052b591  8d5508                 -lea edx, [ebp + 8]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(8) /* 0x8 */);
    // 0052b594  e8188effff             -call 0x5243b1
    cpu.esp -= 4;
    nfs3hp::watcom::reportError(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0052b599  dd5de8                 -fstp qword ptr [ebp - 0x18]
    app->getMemory<double>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = double(fp0);
L_0x0052b59c:
    // 0052b59c  dd45e8                 -fld qword ptr [ebp - 0x18]
    fp0 = x86::Float(app->getMemory<double>(cpu.ebp + x86::reg32(-24) /* -0x18 */));
    // 0052b59f  8d65f0                 -lea esp, [ebp - 0x10]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0052b5a2  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b5a3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b5a4  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b5a5  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b5a6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b5a7  c21400                 -ret 0x14
    cpu.fpu.count += 1;
    cpu.fpu.st(0) = fp0;
    cpu.esp += 4+20 /*0x14*/;
    return;
}

/* sub_52b5b0: native, nfs3hp::watcom::mbclen */

/* sub_52b5e0: native, nfs3hp::watcom::mbcicmp */

/* sub_52b640: native, nfs3hp::watcom::startHook */

/* sub_52b641: native, nfs3hp::dead::stub_0x52b641 */

/* sub_52b647: native, nfs3hp::watcom::printMathError */

/* sub_52b68f: native, nfs3hp::watcom::callMatherr */

/* align: skip 0x00 0x00 */
void Application::sub_52b6c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052b6c0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052b6c1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052b6c2  8b7c240c               -mov edi, dword ptr [esp + 0xc]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0052b6c6  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0052b6c8  7c3b                   -jl 0x52b705
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0052b705;
    }
L_0x0052b6ca:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052b6ca  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 0052b6cc  7c3b                   -jl 0x52b709
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0052b709;
    }
L_0x0052b6ce:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052b6ce  8b7004                 -mov esi, dword ptr [eax + 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0052b6d1  39f2                   +cmp edx, esi
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
    // 0052b6d3  7e02                   -jle 0x52b6d7
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0052b6d7;
    }
    // 0052b6d5  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
L_0x0052b6d7:
    // 0052b6d7  39f1                   +cmp ecx, esi
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
    // 0052b6d9  7e02                   -jle 0x52b6dd
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0052b6dd;
    }
    // 0052b6db  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
L_0x0052b6dd:
    // 0052b6dd  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0052b6df  7c2c                   -jl 0x52b70d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0052b70d;
    }
L_0x0052b6e1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052b6e1  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0052b6e3  7c2c                   -jl 0x52b711
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0052b711;
    }
L_0x0052b6e5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052b6e5  8b7008                 -mov esi, dword ptr [eax + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 0052b6e8  39f3                   +cmp ebx, esi
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
    // 0052b6ea  7e02                   -jle 0x52b6ee
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0052b6ee;
    }
    // 0052b6ec  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
L_0x0052b6ee:
    // 0052b6ee  39f7                   +cmp edi, esi
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
    // 0052b6f0  7e02                   -jle 0x52b6f4
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0052b6f4;
    }
    // 0052b6f2  89f7                   -mov edi, esi
    cpu.edi = cpu.esi;
L_0x0052b6f4:
    // 0052b6f4  894814                 -mov dword ptr [eax + 0x14], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */) = cpu.ecx;
    // 0052b6f7  895810                 -mov dword ptr [eax + 0x10], ebx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */) = cpu.ebx;
    // 0052b6fa  897818                 -mov dword ptr [eax + 0x18], edi
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */) = cpu.edi;
    // 0052b6fd  89500c                 -mov dword ptr [eax + 0xc], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */) = cpu.edx;
    // 0052b700  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b701  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b702  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x0052b705:
    // 0052b705  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0052b707  ebc1                   -jmp 0x52b6ca
    goto L_0x0052b6ca;
L_0x0052b709:
    // 0052b709  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 0052b70b  ebc1                   -jmp 0x52b6ce
    goto L_0x0052b6ce;
L_0x0052b70d:
    // 0052b70d  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0052b70f  ebd0                   -jmp 0x52b6e1
    goto L_0x0052b6e1;
L_0x0052b711:
    // 0052b711  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0052b713  ebd0                   -jmp 0x52b6e5
    goto L_0x0052b6e5;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_52b720(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052b720  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052b721  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052b722  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052b723  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0052b726  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0052b728  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 0052b72a  895c2408               -mov dword ptr [esp + 8], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.ebx;
    // 0052b72e  89cd                   -mov ebp, ecx
    cpu.ebp = cpu.ecx;
    // 0052b730  39d8                   +cmp eax, ebx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052b732  0f8f84000000           -jg 0x52b7bc
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0052b7bc;
    }
L_0x0052b738:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052b738  39fd                   +cmp ebp, edi
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052b73a  7d06                   -jge 0x52b742
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0052b742;
    }
    // 0052b73c  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0052b73e  89ef                   -mov edi, ebp
    cpu.edi = cpu.ebp;
    // 0052b740  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
L_0x0052b742:
    // 0052b742  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0052b746  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 0052b748  8b4c241c               -mov ecx, dword ptr [esp + 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(28) /* 0x1c */);
    // 0052b74c  40                     -inc eax
    (cpu.eax)++;
    // 0052b74d  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0052b74f  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0052b752  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0052b754  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052b755  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 0052b757  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0052b75c  48                     -dec eax
    (cpu.eax)--;
    // 0052b75d  8b5c2404               -mov ebx, dword ptr [esp + 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0052b761  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0052b765  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0052b767  47                     -inc edi
    (cpu.edi)++;
    // 0052b768  e8d31dfdff             -call 0x4fd540
    cpu.esp -= 4;
    sub_4fd540(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b76d  8b5c241c               -mov ebx, dword ptr [esp + 0x1c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(28) /* 0x1c */);
    // 0052b771  8b4c2404               -mov ecx, dword ptr [esp + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0052b775  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0052b779  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052b77a  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0052b77c  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0052b781  e8ba1dfdff             -call 0x4fd540
    cpu.esp -= 4;
    sub_4fd540(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b786  8b44241c               -mov eax, dword ptr [esp + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(28) /* 0x1c */);
    // 0052b78a  8b4c2404               -mov ecx, dword ptr [esp + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0052b78e  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0052b793  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052b794  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0052b796  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0052b798  e8a31dfdff             -call 0x4fd540
    cpu.esp -= 4;
    sub_4fd540(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b79d  8b54241c               -mov edx, dword ptr [esp + 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(28) /* 0x1c */);
    // 0052b7a1  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0052b7a6  8b1c24                 -mov ebx, dword ptr [esp]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    // 0052b7a9  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052b7aa  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0052b7ac  89ea                   -mov edx, ebp
    cpu.edx = cpu.ebp;
    // 0052b7ae  e88d1dfdff             -call 0x4fd540
    cpu.esp -= 4;
    sub_4fd540(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b7b3  83c40c                 +add esp, 0xc
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(12 /*0xc*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0052b7b6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b7b7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b7b8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b7b9  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x0052b7bc:
    // 0052b7bc  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 0052b7be  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 0052b7c2  e971ffffff             -jmp 0x52b738
    goto L_0x0052b738;
}

/* sub_52b7d0: native, nfs3hp::dead::stub_0x52b7d0 */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_52b860(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052b860  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052b861  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052b862  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052b863  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052b864  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0052b866  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 0052b868  e88346fcff             -call 0x4efef0
    cpu.esp -= 4;
    sub_4efef0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b86d  8b1584435600           -mov edx, dword ptr [0x564384]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653380) /* 0x564384 */);
    // 0052b873  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0052b875  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0052b877  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0052b87a  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0052b87c  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0052b87e  8b3df8715600           -mov edi, dword ptr [0x5671f8]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5665272) /* 0x5671f8 */);
    // 0052b884  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0052b886  8b35fc715600           -mov esi, dword ptr [0x5671fc]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5665276) /* 0x5671fc */);
    // 0052b88c  e8bf4ffcff             -call 0x4f0850
    cpu.esp -= 4;
    sub_4f0850(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b891  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052b893  e8b842fcff             -call 0x4efb50
    cpu.esp -= 4;
    sub_4efb50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b898  893df8715600           -mov dword ptr [0x5671f8], edi
    app->getMemory<x86::reg32>(x86::reg32(5665272) /* 0x5671f8 */) = cpu.edi;
    // 0052b89e  8935fc715600           -mov dword ptr [0x5671fc], esi
    app->getMemory<x86::reg32>(x86::reg32(5665276) /* 0x5671fc */) = cpu.esi;
    // 0052b8a4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b8a5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b8a6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b8a7  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b8a8  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_52b8b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052b8b0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052b8b1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052b8b2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052b8b3  8b5008                 -mov edx, dword ptr [eax + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 0052b8b6  8b4804                 -mov ecx, dword ptr [eax + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0052b8b9  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052b8ba  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0052b8bc  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0052b8be  e8fdfdffff             -call 0x52b6c0
    cpu.esp -= 4;
    sub_52b6c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b8c3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b8c4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b8c5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b8c6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_52b8d0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052b8d0  b870b15600             -mov eax, 0x56b170
    cpu.eax = 5681520 /*0x56b170*/;
    // 0052b8d5  e946bdf6ff             -jmp 0x497620
    return sub_497620(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_52b8e0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052b8e0  e83bf4fbff             -call 0x4ead20
    cpu.esp -= 4;
    sub_4ead20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b8e5  e9362b0000             -jmp 0x52e420
    return sub_52e420(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_52b8f0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052b8f0  e99bf4fbff             -jmp 0x4ead90
    return sub_4ead90(app, cpu.handOver());
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_52b910(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0052b910  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052b911  83f804                 +cmp eax, 4
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052b914  7707                   -ja 0x52b91d
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0052b91d;
    }
    // 0052b916  ff2485f8b85200         -jmp dword ptr [eax*4 + 0x52b8f8]
    cpu.ip = app->getMemory<x86::reg32>(5421304 + cpu.eax * 4); goto dynamic_jump;
  case 0x0052b91d:
L_0x0052b91d:
    // 0052b91d  a1f84f5600             -mov eax, dword ptr [0x564ff8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5656568) /* 0x564ff8 */);
    // 0052b922  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052b924  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0052b927  c1e203                 +shl edx, 3
    {
        x86::reg8 tmp = 3 /*0x3*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 0052b92a  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0052b92c  c1f803                 -sar eax, 3
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (3 /*0x3*/ % 32));
    // 0052b92f  99                     -cdq 
    cpu.edx_eax = x86::reg64(static_cast<x86::sreg32>(cpu.eax));
    // 0052b930  31d0                   -xor eax, edx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0052b932  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0052b934  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b935  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0052b936:
    // 0052b936  a1fc4f5600             -mov eax, dword ptr [0x564ffc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5656572) /* 0x564ffc */);
    // 0052b93b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052b93d  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0052b940  c1e203                 +shl edx, 3
    {
        x86::reg8 tmp = 3 /*0x3*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 0052b943  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 0052b945  c1f803                 -sar eax, 3
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (3 /*0x3*/ % 32));
    // 0052b948  99                     -cdq 
    cpu.edx_eax = x86::reg64(static_cast<x86::sreg32>(cpu.eax));
    // 0052b949  31d0                   -xor eax, edx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0052b94b  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0052b94d  48                     -dec eax
    (cpu.eax)--;
    // 0052b94e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b94f  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0052b950:
    // 0052b950  a1f84f5600             -mov eax, dword ptr [0x564ff8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5656568) /* 0x564ff8 */);
    // 0052b955  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b956  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0052b957:
    // 0052b957  a1fc4f5600             -mov eax, dword ptr [0x564ffc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5656572) /* 0x564ffc */);
    // 0052b95c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b95d  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0052b95e:
    // 0052b95e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0052b963  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b964  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_52b970(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052b970  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052b971  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052b972  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052b973  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052b974  81ec04010000           -sub esp, 0x104
    (cpu.esp) -= x86::reg32(x86::sreg32(260 /*0x104*/));
    // 0052b97a  8bb4241c010000         -mov esi, dword ptr [esp + 0x11c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(284) /* 0x11c */);
    // 0052b981  8b8c2418010000         -mov ecx, dword ptr [esp + 0x118]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(280) /* 0x118 */);
    // 0052b988  8d842424010000         -lea eax, [esp + 0x124]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(292) /* 0x124 */);
    // 0052b98f  8d9c2400010000         -lea ebx, [esp + 0x100]
    cpu.ebx = x86::reg32(cpu.esp + x86::reg32(256) /* 0x100 */);
    // 0052b996  8b942420010000         -mov edx, dword ptr [esp + 0x120]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(288) /* 0x120 */);
    // 0052b99d  c1e603                 -shl esi, 3
    cpu.esi <<= 3 /*0x3*/ % 32;
    // 0052b9a0  89842400010000         -mov dword ptr [esp + 0x100], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */) = cpu.eax;
    // 0052b9a7  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052b9a9  c1e103                 -shl ecx, 3
    cpu.ecx <<= 3 /*0x3*/ % 32;
    // 0052b9ac  e8c33cfbff             -call 0x4df674
    cpu.esp -= 4;
    nfs3hp::watcom::vsprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b9b1  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052b9b3  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0052b9b5  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0052b9b7  e8242b0000             -call 0x52e4e0
    cpu.esp -= 4;
    sub_52e4e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b9bc  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0052b9be  89942400010000         -mov dword ptr [esp + 0x100], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */) = cpu.edx;
    // 0052b9c5  81c404010000           -add esp, 0x104
    (cpu.esp) += x86::reg32(x86::sreg32(260 /*0x104*/));
    // 0052b9cb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b9cc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b9cd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b9ce  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b9cf  90                     -nop 
    ;
    // 0052b9d0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_52b9d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x0052b9d0;
    // 0052b970  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052b971  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052b972  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052b973  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052b974  81ec04010000           -sub esp, 0x104
    (cpu.esp) -= x86::reg32(x86::sreg32(260 /*0x104*/));
    // 0052b97a  8bb4241c010000         -mov esi, dword ptr [esp + 0x11c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(284) /* 0x11c */);
    // 0052b981  8b8c2418010000         -mov ecx, dword ptr [esp + 0x118]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(280) /* 0x118 */);
    // 0052b988  8d842424010000         -lea eax, [esp + 0x124]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(292) /* 0x124 */);
    // 0052b98f  8d9c2400010000         -lea ebx, [esp + 0x100]
    cpu.ebx = x86::reg32(cpu.esp + x86::reg32(256) /* 0x100 */);
    // 0052b996  8b942420010000         -mov edx, dword ptr [esp + 0x120]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(288) /* 0x120 */);
    // 0052b99d  c1e603                 -shl esi, 3
    cpu.esi <<= 3 /*0x3*/ % 32;
    // 0052b9a0  89842400010000         -mov dword ptr [esp + 0x100], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */) = cpu.eax;
    // 0052b9a7  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052b9a9  c1e103                 -shl ecx, 3
    cpu.ecx <<= 3 /*0x3*/ % 32;
    // 0052b9ac  e8c33cfbff             -call 0x4df674
    cpu.esp -= 4;
    nfs3hp::watcom::vsprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b9b1  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052b9b3  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0052b9b5  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0052b9b7  e8242b0000             -call 0x52e4e0
    cpu.esp -= 4;
    sub_52e4e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b9bc  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0052b9be  89942400010000         -mov dword ptr [esp + 0x100], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */) = cpu.edx;
    // 0052b9c5  81c404010000           -add esp, 0x104
    (cpu.esp) += x86::reg32(x86::sreg32(260 /*0x104*/));
    // 0052b9cb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b9cc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b9cd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b9ce  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b9cf  90                     -nop 
    ;
L_entry_0x0052b9d0:
    // 0052b9d0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_52b9e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052b9e0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052b9e1  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0052b9e3  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0052b9e5  b8000000ff             -mov eax, 0xff000000
    cpu.eax = 4278190080 /*0xff000000*/;
    // 0052b9ea  e8713dfcff             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b9ef  e8bc37fcff             -call 0x4ef1b0
    cpu.esp -= 4;
    sub_4ef1b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b9f4  89cb                   -mov ebx, ecx
    cpu.ebx = cpu.ecx;
    // 0052b9f6  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0052b9f8  e8c34ffcff             -call 0x4f09c0
    cpu.esp -= 4;
    sub_4f09c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052b9fd  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052b9fe  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_52ba00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052ba00  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052ba01  e86a000000             -call 0x52ba70
    cpu.esp -= 4;
    sub_52ba70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ba06  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0052ba08  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0052ba0a  e861000000             -call 0x52ba70
    cpu.esp -= 4;
    sub_52ba70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ba0f  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052ba11  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0052ba13  e8183efcff             -call 0x4ef830
    cpu.esp -= 4;
    sub_4ef830(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052ba18  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052ba19  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_52ba70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0052ba70  83f812                 +cmp eax, 0x12
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(18 /*0x12*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052ba73  7707                   -ja 0x52ba7c
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0052ba7c;
    }
    // 0052ba75  ff24851cba5200         -jmp dword ptr [eax*4 + 0x52ba1c]
    cpu.ip = app->getMemory<x86::reg32>(5421596 + cpu.eax * 4); goto dynamic_jump;
  case 0x0052ba7c:
L_0x0052ba7c:
    // 0052ba7c  b8aaaa00ff             -mov eax, 0xff00aaaa
    cpu.eax = 4278233770 /*0xff00aaaa*/;
    // 0052ba81  e9da3cfcff             -jmp 0x4ef760
    return sub_4ef760(app, cpu.handOver());
  case 0x0052ba86:
    // 0052ba86  b8000000ff             -mov eax, 0xff000000
    cpu.eax = 4278190080 /*0xff000000*/;
    // 0052ba8b  e9d03cfcff             -jmp 0x4ef760
    return sub_4ef760(app, cpu.handOver());
  case 0x0052ba90:
    // 0052ba90  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052ba95  e9c63cfcff             -jmp 0x4ef760
    return sub_4ef760(app, cpu.handOver());
  case 0x0052ba9a:
    // 0052ba9a  b80000aaff             -mov eax, 0xffaa0000
    cpu.eax = 4289331200 /*0xffaa0000*/;
    // 0052ba9f  e9bc3cfcff             -jmp 0x4ef760
    return sub_4ef760(app, cpu.handOver());
  case 0x0052baa4:
    // 0052baa4  b850ffffff             -mov eax, 0xffffff50
    cpu.eax = 4294967120 /*0xffffff50*/;
    // 0052baa9  e9b23cfcff             -jmp 0x4ef760
    return sub_4ef760(app, cpu.handOver());
  case 0x0052baae:
    // 0052baae  b8aa0000ff             -mov eax, 0xff0000aa
    cpu.eax = 4278190250 /*0xff0000aa*/;
    // 0052bab3  e9a83cfcff             -jmp 0x4ef760
    return sub_4ef760(app, cpu.handOver());
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_52bac0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052bac0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052bac1  b8e0004f00             -mov eax, 0x4f00e0
    cpu.eax = 5177568 /*0x4f00e0*/;
    // 0052bac6  e82546fcff             -call 0x4f00f0
    cpu.esp -= 4;
    sub_4f00f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052bacb:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052bacb  e85046fcff             -call 0x4f0120
    cpu.esp -= 4;
    nfs3hp::ea::inkey(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bad0  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052bad2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052bad4  74f5                   -je 0x52bacb
    if (cpu.flags.zf)
    {
        goto L_0x0052bacb;
    }
    // 0052bad6  83f861                 +cmp eax, 0x61
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(97 /*0x61*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052bad9  7c08                   -jl 0x52bae3
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0052bae3;
    }
    // 0052badb  83f87a                 +cmp eax, 0x7a
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(122 /*0x7a*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052bade  7f03                   -jg 0x52bae3
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0052bae3;
    }
    // 0052bae0  8d50e0                 -lea edx, [eax - 0x20]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(-32) /* -0x20 */);
L_0x0052bae3:
    // 0052bae3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0052bae5  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bae6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_52baf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052baf0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052baf1  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0052baf3  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0052baf7  e874ffffff             -call 0x52ba70
    cpu.esp -= 4;
    sub_52ba70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bafc  c1e603                 -shl esi, 3
    cpu.esi <<= 3 /*0x3*/ % 32;
    // 0052baff  c1e203                 -shl edx, 3
    cpu.edx <<= 3 /*0x3*/ % 32;
    // 0052bb02  c1e303                 -shl ebx, 3
    cpu.ebx <<= 3 /*0x3*/ % 32;
    // 0052bb05  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052bb06  c1e103                 -shl ecx, 3
    cpu.ecx <<= 3 /*0x3*/ % 32;
    // 0052bb09  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0052bb0b  e8301afdff             -call 0x4fd540
    cpu.esp -= 4;
    sub_4fd540(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bb10  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bb11  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_52bb20(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052bb20  b898b15600             -mov eax, 0x56b198
    cpu.eax = 5681560 /*0x56b198*/;
    // 0052bb25  e9f6baf6ff             -jmp 0x497620
    return sub_497620(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_52bb50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0052bb50  83f804                 +cmp eax, 4
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052bb53  7707                   -ja 0x52bb5c
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0052bb5c;
    }
    // 0052bb55  ff248534bb5200         -jmp dword ptr [eax*4 + 0x52bb34]
    cpu.ip = app->getMemory<x86::reg32>(5421876 + cpu.eax * 4); goto dynamic_jump;
  case 0x0052bb5c:
L_0x0052bb5c:
    // 0052bb5c  b850000000             -mov eax, 0x50
    cpu.eax = 80 /*0x50*/;
    // 0052bb61  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0052bb62:
    // 0052bb62  b818000000             -mov eax, 0x18
    cpu.eax = 24 /*0x18*/;
    // 0052bb67  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0052bb68:
    // 0052bb68  a1f84f5600             -mov eax, dword ptr [0x564ff8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5656568) /* 0x564ff8 */);
    // 0052bb6d  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0052bb6e:
    // 0052bb6e  a1fc4f5600             -mov eax, dword ptr [0x564ffc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5656572) /* 0x564ffc */);
    // 0052bb73  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0052bb74:
    // 0052bb74  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052bb76  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_52bb80(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052bb80  e91b51fbff             -jmp 0x4e0ca0
    return nfs3hp::ea::monoClear(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_52bb90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052bb90  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052bb91  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0052bb93  e888f1fbff             -call 0x4ead20
    cpu.esp -= 4;
    sub_4ead20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bb98  e883280000             -call 0x52e420
    cpu.esp -= 4;
    sub_52e420(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bb9d  8b5602                 -mov edx, dword ptr [esi + 2]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 0052bba0  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0052bba5  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0052bba8  e8a3ffffff             -call 0x52bb50
    cpu.esp -= 4;
    sub_52bb50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bbad  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0052bbaf  8b5e04                 -mov ebx, dword ptr [esi + 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0052bbb2  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052bbb4  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 0052bbb9  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0052bbbc  e88fffffff             -call 0x52bb50
    cpu.esp -= 4;
    sub_52bb50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bbc1  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 0052bbc3  d1fa                   -sar edx, 1
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (1 /*0x1*/ % 32));
    // 0052bbc5  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0052bbc7  b8000000ff             -mov eax, 0xff000000
    cpu.eax = 4278190080 /*0xff000000*/;
    // 0052bbcc  d1fb                   -sar ebx, 1
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (1 /*0x1*/ % 32));
    // 0052bbce  e88d3bfcff             -call 0x4ef760
    cpu.esp -= 4;
    sub_4ef760(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bbd3  e8d835fcff             -call 0x4ef1b0
    cpu.esp -= 4;
    sub_4ef1b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bbd8  e8c350fbff             -call 0x4e0ca0
    cpu.esp -= 4;
    nfs3hp::ea::monoClear(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bbdd  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0052bbdf  e8dc4dfcff             -call 0x4f09c0
    cpu.esp -= 4;
    sub_4f09c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bbe4  e8a7f1fbff             -call 0x4ead90
    cpu.esp -= 4;
    sub_4ead90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bbe9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bbea  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_52bbf0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052bbf0  e80b010000             -call 0x52bd00
    cpu.esp -= 4;
    sub_52bd00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bbf5  e95651fbff             -jmp 0x4e0d50
    return nfs3hp::ea::monoSetAttribute(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_52bc00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052bc00  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052bc01  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052bc02  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052bc03  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052bc04  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052bc05  81ec04010000           -sub esp, 0x104
    (cpu.esp) -= x86::reg32(x86::sreg32(260 /*0x104*/));
    // 0052bc0b  8b8c241c010000         -mov ecx, dword ptr [esp + 0x11c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(284) /* 0x11c */);
    // 0052bc12  8bb42420010000         -mov esi, dword ptr [esp + 0x120]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(288) /* 0x120 */);
    // 0052bc19  8b1594725600           -mov edx, dword ptr [0x567294]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5665428) /* 0x567294 */);
    // 0052bc1f  c1e103                 -shl ecx, 3
    cpu.ecx <<= 3 /*0x3*/ % 32;
    // 0052bc22  c1e603                 -shl esi, 3
    cpu.esi <<= 3 /*0x3*/ % 32;
    // 0052bc25  81fa80b55000           +cmp edx, 0x50b580
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5289344 /*0x50b580*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052bc2b  7452                   -je 0x52bc7f
    if (cpu.flags.zf)
    {
        goto L_0x0052bc7f;
    }
L_0x0052bc2d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052bc2d  8d842428010000         -lea eax, [esp + 0x128]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(296) /* 0x128 */);
    // 0052bc34  8d9c2400010000         -lea ebx, [esp + 0x100]
    cpu.ebx = x86::reg32(cpu.esp + x86::reg32(256) /* 0x100 */);
    // 0052bc3b  8b942424010000         -mov edx, dword ptr [esp + 0x124]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(292) /* 0x124 */);
    // 0052bc42  89842400010000         -mov dword ptr [esp + 0x100], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */) = cpu.eax;
    // 0052bc49  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052bc4b  e8243afbff             -call 0x4df674
    cpu.esp -= 4;
    nfs3hp::watcom::vsprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bc50  e8cbf0fbff             -call 0x4ead20
    cpu.esp -= 4;
    sub_4ead20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bc55  e8c6270000             -call 0x52e420
    cpu.esp -= 4;
    sub_52e420(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bc5a  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052bc5c  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0052bc5e  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0052bc60  e87b280000             -call 0x52e4e0
    cpu.esp -= 4;
    sub_52e4e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bc65  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052bc67  89842400010000         -mov dword ptr [esp + 0x100], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(256) /* 0x100 */) = cpu.eax;
    // 0052bc6e  e81df1fbff             -call 0x4ead90
    cpu.esp -= 4;
    sub_4ead90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bc73  81c404010000           -add esp, 0x104
    (cpu.esp) += x86::reg32(x86::sreg32(260 /*0x104*/));
    // 0052bc79  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bc7a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bc7b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bc7c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bc7d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bc7e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052bc7f:
    // 0052bc7f  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052bc80  bb50205500             -mov ebx, 0x552050
    cpu.ebx = 5578832 /*0x552050*/;
    // 0052bc85  bf60205500             -mov edi, 0x552060
    cpu.edi = 5578848 /*0x552060*/;
    // 0052bc8a  bd72000000             -mov ebp, 0x72
    cpu.ebp = 114 /*0x72*/;
    // 0052bc8f  6874205500             -push 0x552074
    app->getMemory<x86::reg32>(cpu.esp-4) = 5578868 /*0x552074*/;
    cpu.esp -= 4;
    // 0052bc94  891d90215500           -mov dword ptr [0x552190], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ebx;
    // 0052bc9a  893d94215500           -mov dword ptr [0x552194], edi
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.edi;
    // 0052bca0  892d98215500           -mov dword ptr [0x552198], ebp
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.ebp;
    // 0052bca6  e86553edff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bcab  83c404                 +add esp, 4
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0052bcae  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bcaf  e979ffffff             -jmp 0x52bc2d
    goto L_0x0052bc2d;
}

/* align: skip  */
void Application::sub_52bd00(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0052bd00  83f812                 +cmp eax, 0x12
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(18 /*0x12*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052bd03  7707                   -ja 0x52bd0c
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0052bd0c;
    }
    // 0052bd05  ff2485b4bc5200         -jmp dword ptr [eax*4 + 0x52bcb4]
    cpu.ip = app->getMemory<x86::reg32>(5422260 + cpu.eax * 4); goto dynamic_jump;
  case 0x0052bd0c:
L_0x0052bd0c:
    // 0052bd0c  b807000000             -mov eax, 7
    cpu.eax = 7 /*0x7*/;
    // 0052bd11  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x0052bd12:
    // 0052bd12  b870000000             -mov eax, 0x70
    cpu.eax = 112 /*0x70*/;
    // 0052bd17  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_52bd20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052bd20  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052bd21  b8e0004f00             -mov eax, 0x4f00e0
    cpu.eax = 5177568 /*0x4f00e0*/;
    // 0052bd26  e8c543fcff             -call 0x4f00f0
    cpu.esp -= 4;
    sub_4f00f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052bd2b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052bd2b  e8f043fcff             -call 0x4f0120
    cpu.esp -= 4;
    nfs3hp::ea::inkey(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bd30  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052bd32  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052bd34  74f5                   -je 0x52bd2b
    if (cpu.flags.zf)
    {
        goto L_0x0052bd2b;
    }
    // 0052bd36  83f861                 +cmp eax, 0x61
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(97 /*0x61*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052bd39  7c08                   -jl 0x52bd43
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0052bd43;
    }
    // 0052bd3b  83f87a                 +cmp eax, 0x7a
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(122 /*0x7a*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052bd3e  7f03                   -jg 0x52bd43
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0052bd43;
    }
    // 0052bd40  8d50e0                 -lea edx, [eax - 0x20]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(-32) /* -0x20 */);
L_0x0052bd43:
    // 0052bd43  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0052bd45  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bd46  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_52bd50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052bd50  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052bd51  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052bd52  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052bd53  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0052bd56  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0052bd58  891424                 -mov dword ptr [esp], edx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.edx;
    // 0052bd5b  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 0052bd5d  89cd                   -mov ebp, ecx
    cpu.ebp = cpu.ecx;
    // 0052bd5f  8b44241c               -mov eax, dword ptr [esp + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(28) /* 0x1c */);
    // 0052bd63  8b0c24                 -mov ecx, dword ptr [esp]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    // 0052bd66  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052bd68  01cd                   -add ebp, ecx
    (cpu.ebp) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0052bd6a  e881feffff             -call 0x52bbf0
    cpu.esp -= 4;
    sub_52bbf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bd6f  39cd                   +cmp ebp, ecx
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052bd71  7e31                   -jle 0x52bda4
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0052bda4;
    }
    // 0052bd73  89742408               -mov dword ptr [esp + 8], esi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.esi;
    // 0052bd77  895c2404               -mov dword ptr [esp + 4], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.ebx;
L_0x0052bd7b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052bd7b  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0052bd7d  7e20                   -jle 0x52bd9f
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0052bd9f;
    }
    // 0052bd7f  8b5c2408               -mov ebx, dword ptr [esp + 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0052bd83  8b442404               -mov eax, dword ptr [esp + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0052bd87  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0052bd89  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
L_0x0052bd8b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052bd8b  68b4205500             -push 0x5520b4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5578932 /*0x5520b4*/;
    cpu.esp -= 4;
    // 0052bd90  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052bd91  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052bd92  e8e94bfbff             -call 0x4e0980
    cpu.esp -= 4;
    nfs3hp::ea::monoPrintfAt(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bd97  42                     -inc edx
    (cpu.edx)++;
    // 0052bd98  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0052bd9b  39da                   +cmp edx, ebx
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052bd9d  7cec                   -jl 0x52bd8b
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0052bd8b;
    }
L_0x0052bd9f:
    // 0052bd9f  41                     -inc ecx
    (cpu.ecx)++;
    // 0052bda0  39e9                   +cmp ecx, ebp
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebp));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052bda2  7cd7                   -jl 0x52bd7b
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0052bd7b;
    }
L_0x0052bda4:
    // 0052bda4  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0052bda7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bda8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bda9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bdaa  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x00 0x00 0x00 */
void Application::sub_52bdb0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052bdb0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052bdb1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052bdb2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052bdb3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052bdb4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0052bdb5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0052bdb6  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0052bdb9  b8b8205500             -mov eax, 0x5520b8
    cpu.eax = 5578936 /*0x5520b8*/;
    // 0052bdbe  e80d2dfeff             -call 0x50ead0
    cpu.esp -= 4;
    nfs3hp::watcom::getenv(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bdc3  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0052bdc5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052bdc7  0f84f9000000           -je 0x52bec6
    if (cpu.flags.zf)
    {
        goto L_0x0052bec6;
    }
L_0x0052bdcd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052bdcd  803900                 +cmp byte ptr [ecx], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.ecx);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052bdd0  0f84e6000000           -je 0x52bebc
    if (cpu.flags.zf)
    {
        goto L_0x0052bebc;
    }
    // 0052bdd6  b23a                   -mov dl, 0x3a
    cpu.dl = 58 /*0x3a*/;
    // 0052bdd8  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
L_0x0052bdda:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052bdda  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0052bddc  3ac2                   +cmp al, dl
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052bdde  7412                   -je 0x52bdf2
    if (cpu.flags.zf)
    {
        goto L_0x0052bdf2;
    }
    // 0052bde0  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052bde2  740c                   -je 0x52bdf0
    if (cpu.flags.zf)
    {
        goto L_0x0052bdf0;
    }
    // 0052bde4  46                     -inc esi
    (cpu.esi)++;
    // 0052bde5  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0052bde7  3ac2                   +cmp al, dl
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052bde9  7407                   -je 0x52bdf2
    if (cpu.flags.zf)
    {
        goto L_0x0052bdf2;
    }
    // 0052bdeb  46                     -inc esi
    (cpu.esi)++;
    // 0052bdec  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052bdee  75ea                   -jne 0x52bdda
    if (!cpu.flags.zf)
    {
        goto L_0x0052bdda;
    }
L_0x0052bdf0:
    // 0052bdf0  2bf6                   -sub esi, esi
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.esi));
L_0x0052bdf2:
    // 0052bdf2  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052bdf4  89f5                   -mov ebp, esi
    cpu.ebp = cpu.esi;
    // 0052bdf6  29ce                   -sub esi, ecx
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0052bdf8  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0052bdfa  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0052bdfc  e82f50fbff             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052be01  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 0052be06  30d2                   -xor dl, dl
    cpu.dl ^= x86::reg8(x86::sreg8(cpu.dl));
    // 0052be08  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052be0a  881434                 -mov byte ptr [esp + esi], dl
    app->getMemory<x86::reg8>(cpu.esp + cpu.esi * 1) = cpu.dl;
    // 0052be0d  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0052be0f  e84c280000             -call 0x52e660
    cpu.esp -= 4;
    sub_52e660(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052be14  8d7501                 -lea esi, [ebp + 1]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(1) /* 0x1 */);
    // 0052be17  b23a                   -mov dl, 0x3a
    cpu.dl = 58 /*0x3a*/;
    // 0052be19  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0052be1b  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
L_0x0052be1d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052be1d  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0052be1f  3ac2                   +cmp al, dl
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052be21  7412                   -je 0x52be35
    if (cpu.flags.zf)
    {
        goto L_0x0052be35;
    }
    // 0052be23  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052be25  740c                   -je 0x52be33
    if (cpu.flags.zf)
    {
        goto L_0x0052be33;
    }
    // 0052be27  46                     -inc esi
    (cpu.esi)++;
    // 0052be28  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0052be2a  3ac2                   +cmp al, dl
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052be2c  7407                   -je 0x52be35
    if (cpu.flags.zf)
    {
        goto L_0x0052be35;
    }
    // 0052be2e  46                     -inc esi
    (cpu.esi)++;
    // 0052be2f  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052be31  75ea                   -jne 0x52be1d
    if (!cpu.flags.zf)
    {
        goto L_0x0052be1d;
    }
L_0x0052be33:
    // 0052be33  2bf6                   -sub esi, esi
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.esi));
L_0x0052be35:
    // 0052be35  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052be37  89f5                   -mov ebp, esi
    cpu.ebp = cpu.esi;
    // 0052be39  29ce                   -sub esi, ecx
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0052be3b  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0052be3d  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0052be3f  e8ec4ffbff             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052be44  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 0052be49  30f6                   -xor dh, dh
    cpu.dh ^= x86::reg8(x86::sreg8(cpu.dh));
    // 0052be4b  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052be4d  883434                 -mov byte ptr [esp + esi], dh
    app->getMemory<x86::reg8>(cpu.esp + cpu.esi * 1) = cpu.dh;
    // 0052be50  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0052be52  e809280000             -call 0x52e660
    cpu.esp -= 4;
    sub_52e660(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052be57  8d7501                 -lea esi, [ebp + 1]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(1) /* 0x1 */);
    // 0052be5a  b22a                   -mov dl, 0x2a
    cpu.dl = 42 /*0x2a*/;
    // 0052be5c  8944240c               -mov dword ptr [esp + 0xc], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 0052be60  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
L_0x0052be62:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052be62  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0052be64  3ac2                   +cmp al, dl
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052be66  7412                   -je 0x52be7a
    if (cpu.flags.zf)
    {
        goto L_0x0052be7a;
    }
    // 0052be68  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052be6a  740c                   -je 0x52be78
    if (cpu.flags.zf)
    {
        goto L_0x0052be78;
    }
    // 0052be6c  46                     -inc esi
    (cpu.esi)++;
    // 0052be6d  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0052be6f  3ac2                   +cmp al, dl
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052be71  7407                   -je 0x52be7a
    if (cpu.flags.zf)
    {
        goto L_0x0052be7a;
    }
    // 0052be73  46                     -inc esi
    (cpu.esi)++;
    // 0052be74  3c00                   +cmp al, 0
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052be76  75ea                   -jne 0x52be62
    if (!cpu.flags.zf)
    {
        goto L_0x0052be62;
    }
L_0x0052be78:
    // 0052be78  2bf6                   -sub esi, esi
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.esi));
L_0x0052be7a:
    // 0052be7a  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052be7c  89f5                   -mov ebp, esi
    cpu.ebp = cpu.esi;
    // 0052be7e  29ce                   -sub esi, ecx
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0052be80  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0052be82  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0052be84  e8a74ffbff             -call 0x4e0e30
    cpu.esp -= 4;
    nfs3hp::watcom::strncpy(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052be89  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0052be8b  30db                   -xor bl, bl
    cpu.bl ^= x86::reg8(x86::sreg8(cpu.bl));
    // 0052be8d  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 0052be8f  881c34                 -mov byte ptr [esp + esi], bl
    app->getMemory<x86::reg8>(cpu.esp + cpu.esi * 1) = cpu.bl;
    // 0052be92  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 0052be97  e8c4270000             -call 0x52e660
    cpu.esp -= 4;
    sub_52e660(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052be9c  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0052be9e  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0052bea0  8b44240c               -mov eax, dword ptr [esp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0052bea4  e8df24ffff             -call 0x51e388
    cpu.esp -= 4;
    sub_51e388(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bea9  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0052beab  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0052bead  8d7501                 -lea esi, [ebp + 1]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(1) /* 0x1 */);
    // 0052beb0  e8836fffff             -call 0x522e38
    cpu.esp -= 4;
    nfs3hp::watcom::SetIOMode(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052beb5  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 0052beb7  e911ffffff             -jmp 0x52bdcd
    goto L_0x0052bdcd;
L_0x0052bebc:
    // 0052bebc  b8c4205500             -mov eax, 0x5520c4
    cpu.eax = 5578948 /*0x5520c4*/;
    // 0052bec1  e80a280000             -call 0x52e6d0
    cpu.esp -= 4;
    sub_52e6d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052bec6:
    // 0052bec6  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0052bec9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052beca  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052becb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052becc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052becd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bece  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052becf  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_52bed0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052bed0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0052bed1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0052bed2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052bed3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0052bed4  8b1554b1a000           -mov edx, dword ptr [0xa0b154]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */);
    // 0052beda  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0052bedc  0f8480000000           -je 0x52bf62
    if (cpu.flags.zf)
    {
        goto L_0x0052bf62;
    }
    // 0052bee2  eb30                   -jmp 0x52bf14
    goto L_0x0052bf14;
L_0x0052bee4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0052bee4  833d50b1a00000         +cmp dword ptr [0xa0b150], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10531152) /* 0xa0b150 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052beeb  7424                   -je 0x52bf11
    if (cpu.flags.zf)
    {
        goto L_0x0052bf11;
    }
    // 0052beed  8b3554b1a000           -mov esi, dword ptr [0xa0b154]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */);
    // 0052bef3  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0052bef5  29f1                   -sub ecx, esi
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.esi));
    // 0052bef7  8b1d50b1a000           -mov ebx, dword ptr [0xa0b150]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10531152) /* 0xa0b150 */);
    // 0052befd  c1f902                 -sar ecx, 2
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (2 /*0x2*/ % 32));
    // 0052bf00  803c1900               +cmp byte ptr [ecx + ebx], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.ecx + cpu.ebx * 1);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0052bf04  7405                   -je 0x52bf0b
    if (cpu.flags.zf)
    {
        goto L_0x0052bf0b;
    }
    // 0052bf06  e8e5bafcff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052bf0b:
    // 0052bf0b  c70200000000           -mov dword ptr [edx], 0
    app->getMemory<x86::reg32>(cpu.edx) = 0 /*0x0*/;
L_0x0052bf11:
    // 0052bf11  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
L_0x0052bf14:
    // 0052bf14  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 0052bf16  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052bf18  75ca                   -jne 0x52bee4
    if (!cpu.flags.zf)
    {
        goto L_0x0052bee4;
    }
    // 0052bf1a  833d50b1a00000         +cmp dword ptr [0xa0b150], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(10531152) /* 0xa0b150 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0052bf21  750c                   -jne 0x52bf2f
    if (!cpu.flags.zf)
    {
        goto L_0x0052bf2f;
    }
    // 0052bf23  b805000000             -mov eax, 5
    cpu.eax = 5 /*0x5*/;
    // 0052bf28  e8d3b9fcff             -call 0x4f7900
    cpu.esp -= 4;
    nfs3hp::watcom::nmalloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052bf2d  eb0f                   -jmp 0x52bf3e
    goto L_0x0052bf3e;
L_0x0052bf2f:
    // 0052bf2f  ba05000000             -mov edx, 5
    cpu.edx = 5 /*0x5*/;
    // 0052bf34  a154b1a000             -mov eax, dword ptr [0xa0b154]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */);
    // 0052bf39  e882c6feff             -call 0x5185c0
    cpu.esp -= 4;
    nfs3hp::watcom::nrealloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0052bf3e:
    // 0052bf3e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0052bf40  750a                   -jne 0x52bf4c
    if (!cpu.flags.zf)
    {
        goto L_0x0052bf4c;
    }
    // 0052bf42  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 0052bf47  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bf48  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bf49  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bf4a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bf4b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0052bf4c:
    // 0052bf4c  a354b1a000             -mov dword ptr [0xa0b154], eax
    app->getMemory<x86::reg32>(x86::reg32(10531156) /* 0xa0b154 */) = cpu.eax;
    // 0052bf51  c70000000000           -mov dword ptr [eax], 0
    app->getMemory<x86::reg32>(cpu.eax) = 0 /*0x0*/;
    // 0052bf57  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0052bf5a  a350b1a000             -mov dword ptr [0xa0b150], eax
    app->getMemory<x86::reg32>(x86::reg32(10531152) /* 0xa0b150 */) = cpu.eax;
    // 0052bf5f  c60000                 -mov byte ptr [eax], 0
    app->getMemory<x86::reg8>(cpu.eax) = 0 /*0x0*/;
L_0x0052bf62:
    // 0052bf62  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052bf64  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bf65  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bf66  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bf67  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052bf68  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_52bf70: native, nfs3hp::dead::stub_0x52bf70 */

/* sub_52bfe0: native, nfs3hp::dead::stub_0x52bfe0 */

/* sub_52c178: native, nfs3hp::watcom::wputenv */

/* sub_52c300: native, nfs3hp::watcom::wputenvFind */

/* sub_52c410: native, nfs3hp::dead::stub_0x52c410 */

/* sub_52c420: native, nfs3hp::dead::stub_0x52c420 */

/* sub_52c430: native, nfs3hp::dead::stub_0x52c430 */

/* sub_52c580: native, nfs3hp::dead::stub_0x52c580 */

/* sub_52c5d0: native, nfs3hp::dead::stub_0x52c5d0 */

/* sub_52c720: native, nfs3hp::dead::stub_0x52c720 */

/* sub_52c7b0: native, nfs3hp::dead::stub_0x52c7b0 */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_52c810(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052c810  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0052c812  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_52c814: native, nfs3hp::dead::stub_0x52c814 */

/* align: skip 0x90 */
void Application::sub_52c81c(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052c81c  ff15c4b15600           -call dword ptr [0x56b1c4]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5681604) /* 0x56b1c4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c822  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_52c824(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052c824  ff15c8b15600           -call dword ptr [0x56b1c8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5681608) /* 0x56b1c8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c82a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_52c82c: native, nfs3hp::dead::stub_0x52c82c */

/* align: skip 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_52c870(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052c870  ff15ccb15600           -call dword ptr [0x56b1cc]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5681612) /* 0x56b1cc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c876  8bc0                   -mov eax, eax
    cpu.eax = cpu.eax;
    // 0052c878  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052c879  baff000000             -mov edx, 0xff
    cpu.edx = 255 /*0xff*/;
    // 0052c87e  b8d4205500             -mov eax, 0x5520d4
    cpu.eax = 5578964 /*0x5520d4*/;
    // 0052c883  e88454feff             -call 0x511d0c
    cpu.esp -= 4;
    nfs3hp::watcom::fatal_runtime_error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c888  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052c889  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_52c878(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x0052c878;
    // 0052c870  ff15ccb15600           -call dword ptr [0x56b1cc]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5681612) /* 0x56b1cc */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c876  8bc0                   -mov eax, eax
    cpu.eax = cpu.eax;
L_entry_0x0052c878:
    // 0052c878  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0052c879  baff000000             -mov edx, 0xff
    cpu.edx = 255 /*0xff*/;
    // 0052c87e  b8d4205500             -mov eax, 0x5520d4
    cpu.eax = 5578964 /*0x5520d4*/;
    // 0052c883  e88454feff             -call 0x511d0c
    cpu.esp -= 4;
    nfs3hp::watcom::fatal_runtime_error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c888  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0052c889  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_52c88a(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052c88a  29d2                   -sub edx, edx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0052c88c  29c0                   -sub eax, eax
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0052c88e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_52c88f(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0052c88f  b883000000             -mov eax, 0x83
    cpu.eax = 131 /*0x83*/;
    // 0052c894  e887240000             -call 0x52ed20
    cpu.esp -= 4;
    sub_52ed20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c899  eb1d                   -jmp 0x52c8b8
    goto L_0x0052c8b8;
    // 0052c89b  b883000000             -mov eax, 0x83
    cpu.eax = 131 /*0x83*/;
    // 0052c8a0  e87b240000             -call 0x52ed20
    cpu.esp -= 4;
    sub_52ed20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c8a5  eb11                   -jmp 0x52c8b8
    goto L_0x0052c8b8;
    // 0052c8a7  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052c8a8  e8f75ffdff             -call 0x5028a4
    cpu.esp -= 4;
    nfs3hp::watcom::set_ERANGE(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c8ad  b884000000             -mov eax, 0x84
    cpu.eax = 132 /*0x84*/;
    // 0052c8b2  e869240000             -call 0x52ed20
    cpu.esp -= 4;
    sub_52ed20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c8b7  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x0052c8b8:
    // 0052c8b8  2500000080             -and eax, 0x80000000
    cpu.eax &= x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
    // 0052c8bd  0d0000f07f             -or eax, 0x7ff00000
    cpu.eax |= x86::reg32(x86::sreg32(2146435072 /*0x7ff00000*/));
    // 0052c8c2  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052c8c4  29c0                   -sub eax, eax
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0052c8c6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_52c8a7(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x0052c8a7;
    // 0052c88f  b883000000             -mov eax, 0x83
    cpu.eax = 131 /*0x83*/;
    // 0052c894  e887240000             -call 0x52ed20
    cpu.esp -= 4;
    sub_52ed20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c899  eb1d                   -jmp 0x52c8b8
    goto L_0x0052c8b8;
    // 0052c89b  b883000000             -mov eax, 0x83
    cpu.eax = 131 /*0x83*/;
    // 0052c8a0  e87b240000             -call 0x52ed20
    cpu.esp -= 4;
    sub_52ed20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c8a5  eb11                   -jmp 0x52c8b8
    goto L_0x0052c8b8;
L_entry_0x0052c8a7:
    // 0052c8a7  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052c8a8  e8f75ffdff             -call 0x5028a4
    cpu.esp -= 4;
    nfs3hp::watcom::set_ERANGE(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c8ad  b884000000             -mov eax, 0x84
    cpu.eax = 132 /*0x84*/;
    // 0052c8b2  e869240000             -call 0x52ed20
    cpu.esp -= 4;
    sub_52ed20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c8b7  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x0052c8b8:
    // 0052c8b8  2500000080             -and eax, 0x80000000
    cpu.eax &= x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
    // 0052c8bd  0d0000f07f             -or eax, 0x7ff00000
    cpu.eax |= x86::reg32(x86::sreg32(2146435072 /*0x7ff00000*/));
    // 0052c8c2  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052c8c4  29c0                   -sub eax, eax
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0052c8c6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_52c89b(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x0052c89b;
    // 0052c88f  b883000000             -mov eax, 0x83
    cpu.eax = 131 /*0x83*/;
    // 0052c894  e887240000             -call 0x52ed20
    cpu.esp -= 4;
    sub_52ed20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c899  eb1d                   -jmp 0x52c8b8
    goto L_0x0052c8b8;
L_entry_0x0052c89b:
    // 0052c89b  b883000000             -mov eax, 0x83
    cpu.eax = 131 /*0x83*/;
    // 0052c8a0  e87b240000             -call 0x52ed20
    cpu.esp -= 4;
    sub_52ed20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c8a5  eb11                   -jmp 0x52c8b8
    goto L_0x0052c8b8;
    // 0052c8a7  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0052c8a8  e8f75ffdff             -call 0x5028a4
    cpu.esp -= 4;
    nfs3hp::watcom::set_ERANGE(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c8ad  b884000000             -mov eax, 0x84
    cpu.eax = 132 /*0x84*/;
    // 0052c8b2  e869240000             -call 0x52ed20
    cpu.esp -= 4;
    sub_52ed20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0052c8b7  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x0052c8b8:
    // 0052c8b8  2500000080             -and eax, 0x80000000
    cpu.eax &= x86::reg32(x86::sreg32(2147483648 /*0x80000000*/));
    // 0052c8bd  0d0000f07f             -or eax, 0x7ff00000
    cpu.eax |= x86::reg32(x86::sreg32(2146435072 /*0x7ff00000*/));
    // 0052c8c2  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0052c8c4  29c0                   -sub eax, eax
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0052c8c6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_52c8c7: native, nfs3hp::dead::stub_0x52c8c7 */

/* sub_52c8f0: native, nfs3hp::port::pcmPlayed */

/* sub_52c8f4: native, nfs3hp::port::pcmOnceDecode */

/* sub_52c9a0: native, nfs3hp::port::pcmOnceSetup */

/* sub_52c9d0: native, nfs3hp::port::pcmLoopedDecode */

/* sub_52ca48: native, nfs3hp::port::pcmLoopedSetup */

/* sub_52ca80: native, nfs3hp::port::pcmStreamedDecode */

/* sub_52cb74: native, nfs3hp::port::pcmStreamedSetup */

/* sub_52cba0: native, nfs3hp::port::pcmFileDecode */

/* sub_52ccb8: native, nfs3hp::port::pcmFileSetup */

/* sub_52ccf0: native, nfs3hp::snd::dead_0x52ccf0 */

/* sub_52ccf4: native, nfs3hp::snd::dead_0x52ccf4 */

/* sub_52cdd4: native, nfs3hp::snd::dead_0x52cdd4 */

/* sub_52ce20: native, nfs3hp::snd::dead_0x52ce20 */

/* sub_52ceb0: native, nfs3hp::snd::dead_0x52ceb0 */

/* sub_52cef0: native, nfs3hp::snd::dead_0x52cef0 */

/* sub_52cfd8: native, nfs3hp::snd::dead_0x52cfd8 */

}
