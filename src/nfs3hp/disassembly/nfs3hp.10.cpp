#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_43a4f0: native, nfs3hp::load::carSelectable */

/* sub_43a580: native, nfs3hp::load::buildCarClassLists */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_43a6b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0043a6b0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0043a6b1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0043a6b2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0043a6b4  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0043a6b6  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0043a6bd  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0043a6bf  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0043a6c2  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0043a6c4  c1e203                 -shl edx, 3
    cpu.edx <<= 3 /*0x3*/ % 32;
    // 0043a6c7  8b848ad8525500         -mov eax, dword ptr [edx + ecx*4 + 0x5552d8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(5591768) /* 0x5552d8 */ + cpu.ecx * 4);
    // 0043a6ce  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a6d0  740a                   -je 0x43a6dc
    if (cpu.flags.zf)
    {
        goto L_0x0043a6dc;
    }
    // 0043a6d2  051c030000             -add eax, 0x31c
    (cpu.eax) += x86::reg32(x86::sreg32(796 /*0x31c*/));
    // 0043a6d7  e8b4f2ffff             -call 0x439990
    cpu.esp -= 4;
    nfs3hp::load::findCarById(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0043a6dc:
    // 0043a6dc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043a6dd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043a6de  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_43a6e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0043a6e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0043a6e1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0043a6e2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0043a6e3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0043a6e4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0043a6e5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0043a6e7  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0043a6e9  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 0043a6eb  e8e0e4ffff             -call 0x438bd0
    cpu.esp -= 4;
    nfs3hp::load::carCount(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a6f0  8b15504f5500           -mov edx, dword ptr [0x554f50]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5590864) /* 0x554f50 */);
    // 0043a6f6  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0043a6f8  29d6                   -sub esi, edx
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0043a6fa  8d0436                 -lea eax, [esi + esi]
    cpu.eax = x86::reg32(cpu.esi + cpu.esi * 1);
    // 0043a6fd  83c008                 -add eax, 8
    (cpu.eax) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0043a700  f6c102                 +test cl, 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 2 /*0x2*/));
    // 0043a703  7436                   -je 0x43a73b
    if (cpu.flags.zf)
    {
        goto L_0x0043a73b;
    }
    // 0043a705  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043a708  7411                   -je 0x43a71b
    if (cpu.flags.zf)
    {
        goto L_0x0043a71b;
    }
    // 0043a70a  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0043a70f  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0043a711  b89a515500             -mov eax, 0x55519a
    cpu.eax = 5591450 /*0x55519a*/;
    // 0043a716  e8255f0a00             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0043a71b:
    // 0043a71b  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043a71e  0f849f050000           -je 0x43acc3
    if (cpu.flags.zf)
    {
        goto L_0x0043acc3;
    }
    // 0043a724  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0043a729  b80a505500             -mov eax, 0x55500a
    cpu.eax = 5591050 /*0x55500a*/;
    // 0043a72e  8d1c36                 -lea ebx, [esi + esi]
    cpu.ebx = x86::reg32(cpu.esi + cpu.esi * 1);
    // 0043a731  e80a5f0a00             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a736  e988050000             -jmp 0x43acc3
    goto L_0x0043acc3;
L_0x0043a73b:
    // 0043a73b  f6c101                 +test cl, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 1 /*0x1*/));
    // 0043a73e  0f846f000000           -je 0x43a7b3
    if (cpu.flags.zf)
    {
        goto L_0x0043a7b3;
    }
    // 0043a744  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043a747  7411                   -je 0x43a75a
    if (cpu.flags.zf)
    {
        goto L_0x0043a75a;
    }
    // 0043a749  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0043a74e  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0043a750  b89a515500             -mov eax, 0x55519a
    cpu.eax = 5591450 /*0x55519a*/;
    // 0043a755  e8e65e0a00             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0043a75a:
    // 0043a75a  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043a75d  7412                   -je 0x43a771
    if (cpu.flags.zf)
    {
        goto L_0x0043a771;
    }
    // 0043a75f  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0043a764  b80a505500             -mov eax, 0x55500a
    cpu.eax = 5591050 /*0x55500a*/;
    // 0043a769  8d1c36                 -lea ebx, [esi + esi]
    cpu.ebx = x86::reg32(cpu.esi + cpu.esi * 1);
    // 0043a76c  e8cf5e0a00             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0043a771:
    // 0043a771  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a773:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a773  39f2                   +cmp edx, esi
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
    // 0043a775  0f8d48050000           -jge 0x43acc3
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043acc3;
    }
    // 0043a77b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a77d  e8cef2ffff             -call 0x439a50
    cpu.esp -= 4;
    nfs3hp::load::carRecord(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a782  e889f4ffff             -call 0x439c10
    cpu.esp -= 4;
    nfs3hp::load::bonusCarLocked(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a787  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a789  7425                   -je 0x43a7b0
    if (cpu.flags.zf)
    {
        goto L_0x0043a7b0;
    }
    // 0043a78b  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0043a78d  b980809000             -mov ecx, 0x908080
    cpu.ecx = 9470080 /*0x908080*/;
    // 0043a792  66891c550a505500       -mov word ptr [edx*2 + 0x55500a], bx
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.bx;
    // 0043a79a  890c956c505500         -mov dword ptr [edx*4 + 0x55506c], ecx
    app->getMemory<x86::reg32>(x86::reg32(5591148) /* 0x55506c */ + cpu.edx * 4) = cpu.ecx;
    // 0043a7a1  66891c55a2515500       -mov word ptr [edx*2 + 0x5551a2], bx
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.bx;
    // 0043a7a9  890c9514525500         -mov dword ptr [edx*4 + 0x555214], ecx
    app->getMemory<x86::reg32>(x86::reg32(5591572) /* 0x555214 */ + cpu.edx * 4) = cpu.ecx;
L_0x0043a7b0:
    // 0043a7b0  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043a7b1  ebc0                   -jmp 0x43a773
    goto L_0x0043a773;
L_0x0043a7b3:
    // 0043a7b3  f6c110                 +test cl, 0x10
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 16 /*0x10*/));
    // 0043a7b6  7451                   -je 0x43a809
    if (cpu.flags.zf)
    {
        goto L_0x0043a809;
    }
    // 0043a7b8  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043a7bb  741e                   -je 0x43a7db
    if (cpu.flags.zf)
    {
        goto L_0x0043a7db;
    }
    // 0043a7bd  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a7bf:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a7bf  39f2                   +cmp edx, esi
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
    // 0043a7c1  7d18                   -jge 0x43a7db
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043a7db;
    }
    // 0043a7c3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a7c5  e866efffff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a7ca  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a7cc  750a                   -jne 0x43a7d8
    if (!cpu.flags.zf)
    {
        goto L_0x0043a7d8;
    }
    // 0043a7ce  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0043a7d0  66893c55a2515500       -mov word ptr [edx*2 + 0x5551a2], di
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.di;
L_0x0043a7d8:
    // 0043a7d8  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043a7d9  ebe4                   -jmp 0x43a7bf
    goto L_0x0043a7bf;
L_0x0043a7db:
    // 0043a7db  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043a7de  7429                   -je 0x43a809
    if (cpu.flags.zf)
    {
        goto L_0x0043a809;
    }
    // 0043a7e0  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a7e2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a7e2  39f2                   +cmp edx, esi
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
    // 0043a7e4  7d23                   -jge 0x43a809
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043a809;
    }
    // 0043a7e6  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a7e8  e843efffff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a7ed  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a7ef  7515                   -jne 0x43a806
    if (!cpu.flags.zf)
    {
        goto L_0x0043a806;
    }
    // 0043a7f1  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a7f3  e8b8eeffff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a7f8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a7fa  750a                   -jne 0x43a806
    if (!cpu.flags.zf)
    {
        goto L_0x0043a806;
    }
    // 0043a7fc  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0043a7fe  668904550a505500       -mov word ptr [edx*2 + 0x55500a], ax
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.ax;
L_0x0043a806:
    // 0043a806  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043a807  ebd9                   -jmp 0x43a7e2
    goto L_0x0043a7e2;
L_0x0043a809:
    // 0043a809  f6c120                 +test cl, 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 32 /*0x20*/));
    // 0043a80c  7453                   -je 0x43a861
    if (cpu.flags.zf)
    {
        goto L_0x0043a861;
    }
    // 0043a80e  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043a811  741f                   -je 0x43a832
    if (cpu.flags.zf)
    {
        goto L_0x0043a832;
    }
    // 0043a813  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a815:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a815  39f2                   +cmp edx, esi
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
    // 0043a817  7d19                   -jge 0x43a832
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043a832;
    }
    // 0043a819  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a81b  e810efffff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a820  83f801                 +cmp eax, 1
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
    // 0043a823  750a                   -jne 0x43a82f
    if (!cpu.flags.zf)
    {
        goto L_0x0043a82f;
    }
    // 0043a825  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0043a827  66893c55a2515500       -mov word ptr [edx*2 + 0x5551a2], di
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.di;
L_0x0043a82f:
    // 0043a82f  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043a830  ebe3                   -jmp 0x43a815
    goto L_0x0043a815;
L_0x0043a832:
    // 0043a832  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043a835  742a                   -je 0x43a861
    if (cpu.flags.zf)
    {
        goto L_0x0043a861;
    }
    // 0043a837  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a839:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a839  39f2                   +cmp edx, esi
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
    // 0043a83b  7d24                   -jge 0x43a861
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043a861;
    }
    // 0043a83d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a83f  e8eceeffff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a844  83f801                 +cmp eax, 1
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
    // 0043a847  7515                   -jne 0x43a85e
    if (!cpu.flags.zf)
    {
        goto L_0x0043a85e;
    }
    // 0043a849  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a84b  e860eeffff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a850  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a852  750a                   -jne 0x43a85e
    if (!cpu.flags.zf)
    {
        goto L_0x0043a85e;
    }
    // 0043a854  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0043a856  668904550a505500       -mov word ptr [edx*2 + 0x55500a], ax
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.ax;
L_0x0043a85e:
    // 0043a85e  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043a85f  ebd8                   -jmp 0x43a839
    goto L_0x0043a839;
L_0x0043a861:
    // 0043a861  f6c140                 +test cl, 0x40
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 64 /*0x40*/));
    // 0043a864  7453                   -je 0x43a8b9
    if (cpu.flags.zf)
    {
        goto L_0x0043a8b9;
    }
    // 0043a866  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043a869  741f                   -je 0x43a88a
    if (cpu.flags.zf)
    {
        goto L_0x0043a88a;
    }
    // 0043a86b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a86d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a86d  39f2                   +cmp edx, esi
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
    // 0043a86f  7d19                   -jge 0x43a88a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043a88a;
    }
    // 0043a871  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a873  e8b8eeffff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a878  83f802                 +cmp eax, 2
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0043a87b  750a                   -jne 0x43a887
    if (!cpu.flags.zf)
    {
        goto L_0x0043a887;
    }
    // 0043a87d  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0043a87f  66893c55a2515500       -mov word ptr [edx*2 + 0x5551a2], di
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.di;
L_0x0043a887:
    // 0043a887  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043a888  ebe3                   -jmp 0x43a86d
    goto L_0x0043a86d;
L_0x0043a88a:
    // 0043a88a  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043a88d  742a                   -je 0x43a8b9
    if (cpu.flags.zf)
    {
        goto L_0x0043a8b9;
    }
    // 0043a88f  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a891:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a891  39f2                   +cmp edx, esi
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
    // 0043a893  7d24                   -jge 0x43a8b9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043a8b9;
    }
    // 0043a895  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a897  e894eeffff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a89c  83f802                 +cmp eax, 2
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0043a89f  7515                   -jne 0x43a8b6
    if (!cpu.flags.zf)
    {
        goto L_0x0043a8b6;
    }
    // 0043a8a1  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a8a3  e808eeffff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a8a8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a8aa  750a                   -jne 0x43a8b6
    if (!cpu.flags.zf)
    {
        goto L_0x0043a8b6;
    }
    // 0043a8ac  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0043a8ae  668904550a505500       -mov word ptr [edx*2 + 0x55500a], ax
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.ax;
L_0x0043a8b6:
    // 0043a8b6  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043a8b7  ebd8                   -jmp 0x43a891
    goto L_0x0043a891;
L_0x0043a8b9:
    // 0043a8b9  f7c100000400           +test ecx, 0x40000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & 262144 /*0x40000*/));
    // 0043a8bf  0f845c000000           -je 0x43a921
    if (cpu.flags.zf)
    {
        goto L_0x0043a921;
    }
    // 0043a8c5  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043a8c8  7429                   -je 0x43a8f3
    if (cpu.flags.zf)
    {
        goto L_0x0043a8f3;
    }
    // 0043a8ca  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a8cc:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a8cc  39f2                   +cmp edx, esi
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
    // 0043a8ce  7d23                   -jge 0x43a8f3
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043a8f3;
    }
    // 0043a8d0  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a8d2  e8e9ecffff             -call 0x4395c0
    cpu.esp -= 4;
    nfs3hp::load::carFlag4(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a8d7  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a8d9  750b                   -jne 0x43a8e6
    if (!cpu.flags.zf)
    {
        goto L_0x0043a8e6;
    }
    // 0043a8db  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a8dd  e82eedffff             -call 0x439610
    cpu.esp -= 4;
    nfs3hp::load::carHasNumericId(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a8e2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a8e4  750a                   -jne 0x43a8f0
    if (!cpu.flags.zf)
    {
        goto L_0x0043a8f0;
    }
L_0x0043a8e6:
    // 0043a8e6  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0043a8e8  66893c55a2515500       -mov word ptr [edx*2 + 0x5551a2], di
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.di;
L_0x0043a8f0:
    // 0043a8f0  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043a8f1  ebd9                   -jmp 0x43a8cc
    goto L_0x0043a8cc;
L_0x0043a8f3:
    // 0043a8f3  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043a8f6  7429                   -je 0x43a921
    if (cpu.flags.zf)
    {
        goto L_0x0043a921;
    }
    // 0043a8f8  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a8fa:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a8fa  39f2                   +cmp edx, esi
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
    // 0043a8fc  7d23                   -jge 0x43a921
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043a921;
    }
    // 0043a8fe  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a900  e8bbecffff             -call 0x4395c0
    cpu.esp -= 4;
    nfs3hp::load::carFlag4(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a905  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a907  750b                   -jne 0x43a914
    if (!cpu.flags.zf)
    {
        goto L_0x0043a914;
    }
    // 0043a909  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a90b  e800edffff             -call 0x439610
    cpu.esp -= 4;
    nfs3hp::load::carHasNumericId(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a910  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a912  750a                   -jne 0x43a91e
    if (!cpu.flags.zf)
    {
        goto L_0x0043a91e;
    }
L_0x0043a914:
    // 0043a914  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0043a916  668904550a505500       -mov word ptr [edx*2 + 0x55500a], ax
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.ax;
L_0x0043a91e:
    // 0043a91e  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043a91f  ebd9                   -jmp 0x43a8fa
    goto L_0x0043a8fa;
L_0x0043a921:
    // 0043a921  f7c100000800           +test ecx, 0x80000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & 524288 /*0x80000*/));
    // 0043a927  7446                   -je 0x43a96f
    if (cpu.flags.zf)
    {
        goto L_0x0043a96f;
    }
    // 0043a929  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043a92c  741e                   -je 0x43a94c
    if (cpu.flags.zf)
    {
        goto L_0x0043a94c;
    }
    // 0043a92e  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a930:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a930  39f2                   +cmp edx, esi
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
    // 0043a932  7d18                   -jge 0x43a94c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043a94c;
    }
    // 0043a934  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a936  e885870100             -call 0x4530c0
    cpu.esp -= 4;
    nfs3hp::frontend::carArtExists(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a93b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a93d  750a                   -jne 0x43a949
    if (!cpu.flags.zf)
    {
        goto L_0x0043a949;
    }
    // 0043a93f  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0043a941  66893c55a2515500       -mov word ptr [edx*2 + 0x5551a2], di
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.di;
L_0x0043a949:
    // 0043a949  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043a94a  ebe4                   -jmp 0x43a930
    goto L_0x0043a930;
L_0x0043a94c:
    // 0043a94c  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043a94f  741e                   -je 0x43a96f
    if (cpu.flags.zf)
    {
        goto L_0x0043a96f;
    }
    // 0043a951  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a953:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a953  39f2                   +cmp edx, esi
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
    // 0043a955  7d18                   -jge 0x43a96f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043a96f;
    }
    // 0043a957  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a959  e862870100             -call 0x4530c0
    cpu.esp -= 4;
    nfs3hp::frontend::carArtExists(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a95e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a960  750a                   -jne 0x43a96c
    if (!cpu.flags.zf)
    {
        goto L_0x0043a96c;
    }
    // 0043a962  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0043a964  668904550a505500       -mov word ptr [edx*2 + 0x55500a], ax
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.ax;
L_0x0043a96c:
    // 0043a96c  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043a96d  ebe4                   -jmp 0x43a953
    goto L_0x0043a953;
L_0x0043a96f:
    // 0043a96f  f7c100001000           +test ecx, 0x100000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & 1048576 /*0x100000*/));
    // 0043a975  7446                   -je 0x43a9bd
    if (cpu.flags.zf)
    {
        goto L_0x0043a9bd;
    }
    // 0043a977  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043a97a  741e                   -je 0x43a99a
    if (cpu.flags.zf)
    {
        goto L_0x0043a99a;
    }
    // 0043a97c  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a97e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a97e  39f2                   +cmp edx, esi
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
    // 0043a980  7d18                   -jge 0x43a99a
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043a99a;
    }
    // 0043a982  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a984  e877880100             -call 0x453200
    cpu.esp -= 4;
    nfs3hp::frontend::showcaseHasSlides(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a989  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a98b  750a                   -jne 0x43a997
    if (!cpu.flags.zf)
    {
        goto L_0x0043a997;
    }
    // 0043a98d  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0043a98f  66893c55a2515500       -mov word ptr [edx*2 + 0x5551a2], di
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.di;
L_0x0043a997:
    // 0043a997  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043a998  ebe4                   -jmp 0x43a97e
    goto L_0x0043a97e;
L_0x0043a99a:
    // 0043a99a  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043a99d  741e                   -je 0x43a9bd
    if (cpu.flags.zf)
    {
        goto L_0x0043a9bd;
    }
    // 0043a99f  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a9a1:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a9a1  39f2                   +cmp edx, esi
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
    // 0043a9a3  7d18                   -jge 0x43a9bd
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043a9bd;
    }
    // 0043a9a5  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a9a7  e854880100             -call 0x453200
    cpu.esp -= 4;
    nfs3hp::frontend::showcaseHasSlides(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a9ac  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a9ae  750a                   -jne 0x43a9ba
    if (!cpu.flags.zf)
    {
        goto L_0x0043a9ba;
    }
    // 0043a9b0  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0043a9b2  668904550a505500       -mov word ptr [edx*2 + 0x55500a], ax
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.ax;
L_0x0043a9ba:
    // 0043a9ba  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043a9bb  ebe4                   -jmp 0x43a9a1
    goto L_0x0043a9a1;
L_0x0043a9bd:
    // 0043a9bd  f7c100002000           +test ecx, 0x200000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & 2097152 /*0x200000*/));
    // 0043a9c3  7446                   -je 0x43aa0b
    if (cpu.flags.zf)
    {
        goto L_0x0043aa0b;
    }
    // 0043a9c5  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043a9c8  741e                   -je 0x43a9e8
    if (cpu.flags.zf)
    {
        goto L_0x0043a9e8;
    }
    // 0043a9ca  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a9cc:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a9cc  39f2                   +cmp edx, esi
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
    // 0043a9ce  7d18                   -jge 0x43a9e8
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043a9e8;
    }
    // 0043a9d0  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a9d2  e8b9880100             -call 0x453290
    cpu.esp -= 4;
    nfs3hp::frontend::showcaseHasHistory(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a9d7  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a9d9  750a                   -jne 0x43a9e5
    if (!cpu.flags.zf)
    {
        goto L_0x0043a9e5;
    }
    // 0043a9db  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0043a9dd  66893c55a2515500       -mov word ptr [edx*2 + 0x5551a2], di
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.di;
L_0x0043a9e5:
    // 0043a9e5  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043a9e6  ebe4                   -jmp 0x43a9cc
    goto L_0x0043a9cc;
L_0x0043a9e8:
    // 0043a9e8  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043a9eb  741e                   -je 0x43aa0b
    if (cpu.flags.zf)
    {
        goto L_0x0043aa0b;
    }
    // 0043a9ed  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043a9ef:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043a9ef  39f2                   +cmp edx, esi
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
    // 0043a9f1  7d18                   -jge 0x43aa0b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043aa0b;
    }
    // 0043a9f3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043a9f5  e896880100             -call 0x453290
    cpu.esp -= 4;
    nfs3hp::frontend::showcaseHasHistory(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043a9fa  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043a9fc  750a                   -jne 0x43aa08
    if (!cpu.flags.zf)
    {
        goto L_0x0043aa08;
    }
    // 0043a9fe  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0043aa00  668904550a505500       -mov word ptr [edx*2 + 0x55500a], ax
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.ax;
L_0x0043aa08:
    // 0043aa08  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043aa09  ebe4                   -jmp 0x43a9ef
    goto L_0x0043a9ef;
L_0x0043aa0b:
    // 0043aa0b  f6c510                 +test ch, 0x10
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ch & 16 /*0x10*/));
    // 0043aa0e  7446                   -je 0x43aa56
    if (cpu.flags.zf)
    {
        goto L_0x0043aa56;
    }
    // 0043aa10  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043aa13  741e                   -je 0x43aa33
    if (cpu.flags.zf)
    {
        goto L_0x0043aa33;
    }
    // 0043aa15  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043aa17:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043aa17  39f2                   +cmp edx, esi
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
    // 0043aa19  7d18                   -jge 0x43aa33
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043aa33;
    }
    // 0043aa1b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043aa1d  e85eecffff             -call 0x439680
    cpu.esp -= 4;
    nfs3hp::load::carIsBonus(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043aa22  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043aa24  740a                   -je 0x43aa30
    if (cpu.flags.zf)
    {
        goto L_0x0043aa30;
    }
    // 0043aa26  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0043aa28  66893c55a2515500       -mov word ptr [edx*2 + 0x5551a2], di
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.di;
L_0x0043aa30:
    // 0043aa30  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043aa31  ebe4                   -jmp 0x43aa17
    goto L_0x0043aa17;
L_0x0043aa33:
    // 0043aa33  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043aa36  741e                   -je 0x43aa56
    if (cpu.flags.zf)
    {
        goto L_0x0043aa56;
    }
    // 0043aa38  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043aa3a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043aa3a  39f2                   +cmp edx, esi
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
    // 0043aa3c  7d18                   -jge 0x43aa56
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043aa56;
    }
    // 0043aa3e  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043aa40  e83becffff             -call 0x439680
    cpu.esp -= 4;
    nfs3hp::load::carIsBonus(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043aa45  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043aa47  740a                   -je 0x43aa53
    if (cpu.flags.zf)
    {
        goto L_0x0043aa53;
    }
    // 0043aa49  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0043aa4b  668904550a505500       -mov word ptr [edx*2 + 0x55500a], ax
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.ax;
L_0x0043aa53:
    // 0043aa53  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043aa54  ebe4                   -jmp 0x43aa3a
    goto L_0x0043aa3a;
L_0x0043aa56:
    // 0043aa56  f6c520                 +test ch, 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ch & 32 /*0x20*/));
    // 0043aa59  7446                   -je 0x43aaa1
    if (cpu.flags.zf)
    {
        goto L_0x0043aaa1;
    }
    // 0043aa5b  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043aa5e  741e                   -je 0x43aa7e
    if (cpu.flags.zf)
    {
        goto L_0x0043aa7e;
    }
    // 0043aa60  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043aa62:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043aa62  39f2                   +cmp edx, esi
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
    // 0043aa64  7d18                   -jge 0x43aa7e
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043aa7e;
    }
    // 0043aa66  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043aa68  e873ecffff             -call 0x4396e0
    cpu.esp -= 4;
    nfs3hp::load::carIsFerrariOrMercedes(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043aa6d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043aa6f  740a                   -je 0x43aa7b
    if (cpu.flags.zf)
    {
        goto L_0x0043aa7b;
    }
    // 0043aa71  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0043aa73  66893c55a2515500       -mov word ptr [edx*2 + 0x5551a2], di
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.di;
L_0x0043aa7b:
    // 0043aa7b  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043aa7c  ebe4                   -jmp 0x43aa62
    goto L_0x0043aa62;
L_0x0043aa7e:
    // 0043aa7e  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043aa81  741e                   -je 0x43aaa1
    if (cpu.flags.zf)
    {
        goto L_0x0043aaa1;
    }
    // 0043aa83  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043aa85:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043aa85  39f2                   +cmp edx, esi
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
    // 0043aa87  7d18                   -jge 0x43aaa1
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043aaa1;
    }
    // 0043aa89  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043aa8b  e850ecffff             -call 0x4396e0
    cpu.esp -= 4;
    nfs3hp::load::carIsFerrariOrMercedes(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043aa90  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043aa92  740a                   -je 0x43aa9e
    if (cpu.flags.zf)
    {
        goto L_0x0043aa9e;
    }
    // 0043aa94  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0043aa96  668904550a505500       -mov word ptr [edx*2 + 0x55500a], ax
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.ax;
L_0x0043aa9e:
    // 0043aa9e  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043aa9f  ebe4                   -jmp 0x43aa85
    goto L_0x0043aa85;
L_0x0043aaa1:
    // 0043aaa1  f6c540                 +test ch, 0x40
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ch & 64 /*0x40*/));
    // 0043aaa4  7446                   -je 0x43aaec
    if (cpu.flags.zf)
    {
        goto L_0x0043aaec;
    }
    // 0043aaa6  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043aaa9  741e                   -je 0x43aac9
    if (cpu.flags.zf)
    {
        goto L_0x0043aac9;
    }
    // 0043aaab  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043aaad:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043aaad  39f2                   +cmp edx, esi
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
    // 0043aaaf  7d18                   -jge 0x43aac9
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043aac9;
    }
    // 0043aab1  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043aab3  e8f8ebffff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043aab8  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043aaba  740a                   -je 0x43aac6
    if (cpu.flags.zf)
    {
        goto L_0x0043aac6;
    }
    // 0043aabc  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0043aabe  66893c55a2515500       -mov word ptr [edx*2 + 0x5551a2], di
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.di;
L_0x0043aac6:
    // 0043aac6  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043aac7  ebe4                   -jmp 0x43aaad
    goto L_0x0043aaad;
L_0x0043aac9:
    // 0043aac9  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043aacc  741e                   -je 0x43aaec
    if (cpu.flags.zf)
    {
        goto L_0x0043aaec;
    }
    // 0043aace  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043aad0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043aad0  39f2                   +cmp edx, esi
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
    // 0043aad2  7d18                   -jge 0x43aaec
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043aaec;
    }
    // 0043aad4  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043aad6  e8d5ebffff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043aadb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043aadd  740a                   -je 0x43aae9
    if (cpu.flags.zf)
    {
        goto L_0x0043aae9;
    }
    // 0043aadf  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0043aae1  668904550a505500       -mov word ptr [edx*2 + 0x55500a], ax
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.ax;
L_0x0043aae9:
    // 0043aae9  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043aaea  ebe4                   -jmp 0x43aad0
    goto L_0x0043aad0;
L_0x0043aaec:
    // 0043aaec  f6c580                 +test ch, 0x80
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ch & 128 /*0x80*/));
    // 0043aaef  7446                   -je 0x43ab37
    if (cpu.flags.zf)
    {
        goto L_0x0043ab37;
    }
    // 0043aaf1  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043aaf4  741e                   -je 0x43ab14
    if (cpu.flags.zf)
    {
        goto L_0x0043ab14;
    }
    // 0043aaf6  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043aaf8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043aaf8  39f2                   +cmp edx, esi
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
    // 0043aafa  7d18                   -jge 0x43ab14
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043ab14;
    }
    // 0043aafc  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043aafe  e8adebffff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ab03  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043ab05  750a                   -jne 0x43ab11
    if (!cpu.flags.zf)
    {
        goto L_0x0043ab11;
    }
    // 0043ab07  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0043ab09  66893c55a2515500       -mov word ptr [edx*2 + 0x5551a2], di
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.di;
L_0x0043ab11:
    // 0043ab11  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043ab12  ebe4                   -jmp 0x43aaf8
    goto L_0x0043aaf8;
L_0x0043ab14:
    // 0043ab14  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043ab17  741e                   -je 0x43ab37
    if (cpu.flags.zf)
    {
        goto L_0x0043ab37;
    }
    // 0043ab19  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043ab1b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043ab1b  39f2                   +cmp edx, esi
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
    // 0043ab1d  7d18                   -jge 0x43ab37
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043ab37;
    }
    // 0043ab1f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043ab21  e88aebffff             -call 0x4396b0
    cpu.esp -= 4;
    nfs3hp::load::carIsPolice(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ab26  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043ab28  750a                   -jne 0x43ab34
    if (!cpu.flags.zf)
    {
        goto L_0x0043ab34;
    }
    // 0043ab2a  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0043ab2c  668904550a505500       -mov word ptr [edx*2 + 0x55500a], ax
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.ax;
L_0x0043ab34:
    // 0043ab34  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043ab35  ebe4                   -jmp 0x43ab1b
    goto L_0x0043ab1b;
L_0x0043ab37:
    // 0043ab37  f7c100000100           +test ecx, 0x10000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & 65536 /*0x10000*/));
    // 0043ab3d  7446                   -je 0x43ab85
    if (cpu.flags.zf)
    {
        goto L_0x0043ab85;
    }
    // 0043ab3f  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043ab42  741e                   -je 0x43ab62
    if (cpu.flags.zf)
    {
        goto L_0x0043ab62;
    }
    // 0043ab44  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043ab46:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043ab46  39f2                   +cmp edx, esi
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
    // 0043ab48  7d18                   -jge 0x43ab62
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043ab62;
    }
    // 0043ab4a  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043ab4c  e8bfeaffff             -call 0x439610
    cpu.esp -= 4;
    nfs3hp::load::carHasNumericId(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ab51  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043ab53  750a                   -jne 0x43ab5f
    if (!cpu.flags.zf)
    {
        goto L_0x0043ab5f;
    }
    // 0043ab55  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0043ab57  66893c55a2515500       -mov word ptr [edx*2 + 0x5551a2], di
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.di;
L_0x0043ab5f:
    // 0043ab5f  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043ab60  ebe4                   -jmp 0x43ab46
    goto L_0x0043ab46;
L_0x0043ab62:
    // 0043ab62  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043ab65  741e                   -je 0x43ab85
    if (cpu.flags.zf)
    {
        goto L_0x0043ab85;
    }
    // 0043ab67  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043ab69:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043ab69  39f2                   +cmp edx, esi
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
    // 0043ab6b  7d18                   -jge 0x43ab85
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043ab85;
    }
    // 0043ab6d  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043ab6f  e89ceaffff             -call 0x439610
    cpu.esp -= 4;
    nfs3hp::load::carHasNumericId(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ab74  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043ab76  750a                   -jne 0x43ab82
    if (!cpu.flags.zf)
    {
        goto L_0x0043ab82;
    }
    // 0043ab78  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0043ab7a  668904550a505500       -mov word ptr [edx*2 + 0x55500a], ax
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.ax;
L_0x0043ab82:
    // 0043ab82  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043ab83  ebe4                   -jmp 0x43ab69
    goto L_0x0043ab69;
L_0x0043ab85:
    // 0043ab85  f7c100000200           +test ecx, 0x20000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & 131072 /*0x20000*/));
    // 0043ab8b  7446                   -je 0x43abd3
    if (cpu.flags.zf)
    {
        goto L_0x0043abd3;
    }
    // 0043ab8d  f6c104                 +test cl, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 4 /*0x4*/));
    // 0043ab90  741e                   -je 0x43abb0
    if (cpu.flags.zf)
    {
        goto L_0x0043abb0;
    }
    // 0043ab92  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043ab94:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043ab94  39f2                   +cmp edx, esi
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
    // 0043ab96  7d18                   -jge 0x43abb0
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043abb0;
    }
    // 0043ab98  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043ab9a  e841eaffff             -call 0x4395e0
    cpu.esp -= 4;
    sub_4395e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ab9f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043aba1  750a                   -jne 0x43abad
    if (!cpu.flags.zf)
    {
        goto L_0x0043abad;
    }
    // 0043aba3  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0043aba5  66893c55a2515500       -mov word ptr [edx*2 + 0x5551a2], di
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.di;
L_0x0043abad:
    // 0043abad  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043abae  ebe4                   -jmp 0x43ab94
    goto L_0x0043ab94;
L_0x0043abb0:
    // 0043abb0  f6c108                 +test cl, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 8 /*0x8*/));
    // 0043abb3  741e                   -je 0x43abd3
    if (cpu.flags.zf)
    {
        goto L_0x0043abd3;
    }
    // 0043abb5  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043abb7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043abb7  39f2                   +cmp edx, esi
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
    // 0043abb9  7d18                   -jge 0x43abd3
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043abd3;
    }
    // 0043abbb  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043abbd  e81eeaffff             -call 0x4395e0
    cpu.esp -= 4;
    sub_4395e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043abc2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043abc4  750a                   -jne 0x43abd0
    if (!cpu.flags.zf)
    {
        goto L_0x0043abd0;
    }
    // 0043abc6  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0043abc8  668904550a505500       -mov word ptr [edx*2 + 0x55500a], ax
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.ax;
L_0x0043abd0:
    // 0043abd0  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043abd1  ebe4                   -jmp 0x43abb7
    goto L_0x0043abb7;
L_0x0043abd3:
    // 0043abd3  f6c180                 +test cl, 0x80
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & 128 /*0x80*/));
    // 0043abd6  7409                   -je 0x43abe1
    if (cpu.flags.zf)
    {
        goto L_0x0043abe1;
    }
    // 0043abd8  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0043abda  6689159a515500         -mov word ptr [0x55519a], dx
    app->getMemory<x86::reg16>(x86::reg32(5591450) /* 0x55519a */) = cpu.dx;
L_0x0043abe1:
    // 0043abe1  f6c501                 +test ch, 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ch & 1 /*0x1*/));
    // 0043abe4  7409                   -je 0x43abef
    if (cpu.flags.zf)
    {
        goto L_0x0043abef;
    }
    // 0043abe6  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 0043abe8  66893d9c515500         -mov word ptr [0x55519c], di
    app->getMemory<x86::reg16>(x86::reg32(5591452) /* 0x55519c */) = cpu.di;
L_0x0043abef:
    // 0043abef  f6c502                 +test ch, 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ch & 2 /*0x2*/));
    // 0043abf2  7408                   -je 0x43abfc
    if (cpu.flags.zf)
    {
        goto L_0x0043abfc;
    }
    // 0043abf4  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0043abf6  66a39e515500           -mov word ptr [0x55519e], ax
    app->getMemory<x86::reg16>(x86::reg32(5591454) /* 0x55519e */) = cpu.ax;
L_0x0043abfc:
    // 0043abfc  f6c504                 +test ch, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ch & 4 /*0x4*/));
    // 0043abff  7409                   -je 0x43ac0a
    if (cpu.flags.zf)
    {
        goto L_0x0043ac0a;
    }
    // 0043ac01  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0043ac03  668915a0515500         -mov word ptr [0x5551a0], dx
    app->getMemory<x86::reg16>(x86::reg32(5591456) /* 0x5551a0 */) = cpu.dx;
L_0x0043ac0a:
    // 0043ac0a  f6c508                 +test ch, 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ch & 8 /*0x8*/));
    // 0043ac0d  7413                   -je 0x43ac22
    if (cpu.flags.zf)
    {
        goto L_0x0043ac22;
    }
    // 0043ac0f  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043ac11:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043ac11  39f2                   +cmp edx, esi
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
    // 0043ac13  7d0d                   -jge 0x43ac22
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043ac22;
    }
    // 0043ac15  42                     -inc edx
    (cpu.edx)++;
    // 0043ac16  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 0043ac18  66890c55a0515500       -mov word ptr [edx*2 + 0x5551a0], cx
    app->getMemory<x86::reg16>(x86::reg32(5591456) /* 0x5551a0 */ + cpu.edx * 2) = cpu.cx;
    // 0043ac20  ebef                   -jmp 0x43ac11
    goto L_0x0043ac11;
L_0x0043ac22:
    // 0043ac22  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0043ac24  7506                   -jne 0x43ac2c
    if (!cpu.flags.zf)
    {
        goto L_0x0043ac2c;
    }
    // 0043ac26  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ac27  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ac28  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ac29  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ac2a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ac2b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0043ac2c:
    // 0043ac2c  8b0dd4d46f00           -mov ecx, dword ptr [0x6fd4d4]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
L_0x0043ac32:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043ac32  a1d4d46f00             -mov eax, dword ptr [0x6fd4d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 0043ac37  66833c459a51550000     +cmp word ptr [eax*2 + 0x55519a], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(5591450) /* 0x55519a */ + cpu.eax * 2);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0043ac40  751f                   -jne 0x43ac61
    if (!cpu.flags.zf)
    {
        goto L_0x0043ac61;
    }
    // 0043ac42  8d7801                 -lea edi, [eax + 1]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0043ac45  8d5e04                 -lea ebx, [esi + 4]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 0043ac48  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0043ac4a  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0043ac4c  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0043ac4f  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0043ac51  893dd4d46f00           -mov dword ptr [0x6fd4d4], edi
    app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */) = cpu.edi;
    // 0043ac57  8915d4d46f00           -mov dword ptr [0x6fd4d4], edx
    app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */) = cpu.edx;
    // 0043ac5d  39d1                   +cmp ecx, edx
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0043ac5f  75d1                   -jne 0x43ac32
    if (!cpu.flags.zf)
    {
        goto L_0x0043ac32;
    }
L_0x0043ac61:
    // 0043ac61  8b0dbcd26f00           -mov ecx, dword ptr [0x6fd2bc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
L_0x0043ac67:
    app->unlockContext(cpu);
    win32::Thread::sleep(0);
    app->lockContext(cpu);
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043ac67  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 0043ac6c  66833c450a50550000     +cmp word ptr [eax*2 + 0x55500a], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.eax * 2);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0043ac75  751a                   -jne 0x43ac91
    if (!cpu.flags.zf)
    {
        goto L_0x0043ac91;
    }
    // 0043ac77  8d5001                 -lea edx, [eax + 1]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0043ac7a  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043ac7c  8915bcd26f00           -mov dword ptr [0x6fd2bc], edx
    app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */) = cpu.edx;
    // 0043ac82  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0043ac85  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0043ac87  8915bcd26f00           -mov dword ptr [0x6fd2bc], edx
    app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */) = cpu.edx;
    // 0043ac8d  39d1                   +cmp ecx, edx
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0043ac8f  75d6                   -jne 0x43ac67
    if (!cpu.flags.zf)
    {
        goto L_0x0043ac67;
    }
L_0x0043ac91:
    // 0043ac91  8b0d28d36f00           -mov ecx, dword ptr [0x6fd328]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328552) /* 0x6fd328 */);
L_0x0043ac97:
    app->unlockContext(cpu);
    win32::Thread::sleep(0);
    app->lockContext(cpu);
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043ac97  a128d36f00             -mov eax, dword ptr [0x6fd328]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328552) /* 0x6fd328 */);
    // 0043ac9c  66833c450a50550000     +cmp word ptr [eax*2 + 0x55500a], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.eax * 2);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0043aca5  751c                   -jne 0x43acc3
    if (!cpu.flags.zf)
    {
        goto L_0x0043acc3;
    }
    // 0043aca7  8d7801                 -lea edi, [eax + 1]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0043acaa  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0043acac  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0043acae  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0043acb1  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0043acb3  893d28d36f00           -mov dword ptr [0x6fd328], edi
    app->getMemory<x86::reg32>(x86::reg32(7328552) /* 0x6fd328 */) = cpu.edi;
    // 0043acb9  891528d36f00           -mov dword ptr [0x6fd328], edx
    app->getMemory<x86::reg32>(x86::reg32(7328552) /* 0x6fd328 */) = cpu.edx;
    // 0043acbf  39d1                   +cmp ecx, edx
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0043acc1  75d4                   -jne 0x43ac97
    if (!cpu.flags.zf)
    {
        goto L_0x0043ac97;
    }
L_0x0043acc3:
    // 0043acc3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043acc4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043acc5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043acc6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043acc7  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043acc8  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_43ace0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0043ace0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0043ace1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0043ace2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0043ace3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0043ace4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0043ace5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0043ace7  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0043ace9  83f802                 +cmp eax, 2
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0043acec  7e2d                   -jle 0x43ad1b
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0043ad1b;
    }
    // 0043acee  bb62000000             -mov ebx, 0x62
    cpu.ebx = 98 /*0x62*/;
    // 0043acf3  baff000000             -mov edx, 0xff
    cpu.edx = 255 /*0xff*/;
    // 0043acf8  b80a505500             -mov eax, 0x55500a
    cpu.eax = 5591050 /*0x55500a*/;
    // 0043acfd  e83e590a00             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ad02  bb6a000000             -mov ebx, 0x6a
    cpu.ebx = 106 /*0x6a*/;
    // 0043ad07  baff000000             -mov edx, 0xff
    cpu.edx = 255 /*0xff*/;
    // 0043ad0c  b89a515500             -mov eax, 0x55519a
    cpu.eax = 5591450 /*0x55519a*/;
    // 0043ad11  e82a590a00             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ad16  e938010000             -jmp 0x43ae53
    goto L_0x0043ae53;
L_0x0043ad1b:
    // 0043ad1b  bb62000000             -mov ebx, 0x62
    cpu.ebx = 98 /*0x62*/;
    // 0043ad20  b80a505500             -mov eax, 0x55500a
    cpu.eax = 5591050 /*0x55500a*/;
    // 0043ad25  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0043ad27  e814590a00             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ad2c  bb6a000000             -mov ebx, 0x6a
    cpu.ebx = 106 /*0x6a*/;
    // 0043ad31  b89a515500             -mov eax, 0x55519a
    cpu.eax = 5591450 /*0x55519a*/;
    // 0043ad36  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0043ad38  e803590a00             -call 0x4e0640
    cpu.esp -= 4;
    nfs3hp::watcom::memset(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ad3d  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0043ad42  6689159a515500         -mov word ptr [0x55519a], dx
    app->getMemory<x86::reg16>(x86::reg32(5591450) /* 0x55519a */) = cpu.dx;
    // 0043ad49  6689144d9c515500       -mov word ptr [ecx*2 + 0x55519c], dx
    app->getMemory<x86::reg16>(x86::reg32(5591452) /* 0x55519c */ + cpu.ecx * 2) = cpu.dx;
    // 0043ad51  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0043ad53:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043ad53  e878deffff             -call 0x438bd0
    cpu.esp -= 4;
    nfs3hp::load::carCount(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ad58  39c2                   +cmp edx, eax
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0043ad5a  7d23                   -jge 0x43ad7f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0043ad7f;
    }
    // 0043ad5c  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043ad5e  e8cde9ffff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ad63  39c8                   +cmp eax, ecx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0043ad65  7515                   -jne 0x43ad7c
    if (!cpu.flags.zf)
    {
        goto L_0x0043ad7c;
    }
    // 0043ad67  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 0043ad6c  668934550a505500       -mov word ptr [edx*2 + 0x55500a], si
    app->getMemory<x86::reg16>(x86::reg32(5591050) /* 0x55500a */ + cpu.edx * 2) = cpu.si;
    // 0043ad74  66893455a2515500       -mov word ptr [edx*2 + 0x5551a2], si
    app->getMemory<x86::reg16>(x86::reg32(5591458) /* 0x5551a2 */ + cpu.edx * 2) = cpu.si;
L_0x0043ad7c:
    // 0043ad7c  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0043ad7d  ebd4                   -jmp 0x43ad53
    goto L_0x0043ad53;
L_0x0043ad7f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043ad7f  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 0043ad84  e8a7e9ffff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ad89  39c8                   +cmp eax, ecx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0043ad8b  7421                   -je 0x43adae
    if (cpu.flags.zf)
    {
        goto L_0x0043adae;
    }
    // 0043ad8d  ff05bcd26f00           -inc dword ptr [0x6fd2bc]
    (app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */))++;
    // 0043ad93  e838deffff             -call 0x438bd0
    cpu.esp -= 4;
    nfs3hp::load::carCount(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ad98  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0043ad9a  a1bcd26f00             -mov eax, dword ptr [0x6fd2bc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */);
    // 0043ad9f  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0043ada1  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0043ada4  f7fb                   +idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0043ada6  8915bcd26f00           -mov dword ptr [0x6fd2bc], edx
    app->getMemory<x86::reg32>(x86::reg32(7328444) /* 0x6fd2bc */) = cpu.edx;
    // 0043adac  ebd1                   -jmp 0x43ad7f
    goto L_0x0043ad7f;
L_0x0043adae:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043adae  a128d36f00             -mov eax, dword ptr [0x6fd328]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328552) /* 0x6fd328 */);
    // 0043adb3  e878e9ffff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043adb8  39c8                   +cmp eax, ecx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0043adba  7421                   -je 0x43addd
    if (cpu.flags.zf)
    {
        goto L_0x0043addd;
    }
    // 0043adbc  ff0528d36f00           -inc dword ptr [0x6fd328]
    (app->getMemory<x86::reg32>(x86::reg32(7328552) /* 0x6fd328 */))++;
    // 0043adc2  e809deffff             -call 0x438bd0
    cpu.esp -= 4;
    nfs3hp::load::carCount(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043adc7  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0043adc9  a128d36f00             -mov eax, dword ptr [0x6fd328]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328552) /* 0x6fd328 */);
    // 0043adce  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0043add0  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0043add3  f7fb                   +idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0043add5  891528d36f00           -mov dword ptr [0x6fd328], edx
    app->getMemory<x86::reg32>(x86::reg32(7328552) /* 0x6fd328 */) = cpu.edx;
    // 0043addb  ebd1                   -jmp 0x43adae
    goto L_0x0043adae;
L_0x0043addd:
    // 0043addd  a1d4d46f00             -mov eax, dword ptr [0x6fd4d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 0043ade2  83f803                 +cmp eax, 3
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0043ade5  771e                   -ja 0x43ae05
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0043ae05;
    }
    // 0043ade7  ff2485ccac4300         -jmp dword ptr [eax*4 + 0x43accc]
    cpu.ip = app->getMemory<x86::reg32>(4435148 + cpu.eax * 4); goto dynamic_jump;
  case 0x0043adee:
    // 0043adee  a1d4d46f00             -mov eax, dword ptr [0x6fd4d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 0043adf3  48                     -dec eax
    (cpu.eax)--;
    // 0043adf4  39c1                   +cmp ecx, eax
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
    // 0043adf6  745b                   -je 0x43ae53
    if (cpu.flags.zf)
    {
        goto L_0x0043ae53;
    }
    // 0043adf8  41                     -inc ecx
    (cpu.ecx)++;
    // 0043adf9  890dd4d46f00           -mov dword ptr [0x6fd4d4], ecx
    app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */) = cpu.ecx;
    // 0043adff  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ae00  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ae01  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ae02  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ae03  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ae04  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0043ae05:
    // 0043ae05  a1d4d46f00             -mov eax, dword ptr [0x6fd4d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 0043ae0a  83e804                 -sub eax, 4
    (cpu.eax) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0043ae0d  e81ee9ffff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ae12  39c1                   +cmp ecx, eax
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
    // 0043ae14  743d                   -je 0x43ae53
    if (cpu.flags.zf)
    {
        goto L_0x0043ae53;
    }
    // 0043ae16  c705d4d46f0004000000   -mov dword ptr [0x6fd4d4], 4
    app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */) = 4 /*0x4*/;
L_0x0043ae20:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043ae20  a1d4d46f00             -mov eax, dword ptr [0x6fd4d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 0043ae25  83e804                 -sub eax, 4
    (cpu.eax) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0043ae28  e803e9ffff             -call 0x439730
    cpu.esp -= 4;
    nfs3hp::load::carClass(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ae2d  39c8                   +cmp eax, ecx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0043ae2f  7422                   -je 0x43ae53
    if (cpu.flags.zf)
    {
        goto L_0x0043ae53;
    }
    // 0043ae31  ff05d4d46f00           -inc dword ptr [0x6fd4d4]
    (app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */))++;
    // 0043ae37  e894ddffff             -call 0x438bd0
    cpu.esp -= 4;
    nfs3hp::load::carCount(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043ae3c  8d5804                 -lea ebx, [eax + 4]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 0043ae3f  a1d4d46f00             -mov eax, dword ptr [0x6fd4d4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */);
    // 0043ae44  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0043ae46  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0043ae49  f7fb                   +idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0043ae4b  8915d4d46f00           -mov dword ptr [0x6fd4d4], edx
    app->getMemory<x86::reg32>(x86::reg32(7328980) /* 0x6fd4d4 */) = cpu.edx;
    // 0043ae51  ebcd                   -jmp 0x43ae20
    goto L_0x0043ae20;
  case 0x0043ae53:
L_0x0043ae53:
    // 0043ae53  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ae54  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ae55  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ae56  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ae57  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043ae58  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* sub_43ae60: native, nfs3hp::load::copyCarAsNew */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_43b2a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0043b2a0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0043b2a1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0043b2a2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0043b2a3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0043b2a4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0043b2a5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0043b2a7  81ec2c010000           -sub esp, 0x12c
    (cpu.esp) -= x86::reg32(x86::sreg32(300 /*0x12c*/));
    // 0043b2ad  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0043b2af  beb4287a00             -mov esi, 0x7a28b4
    cpu.esi = 8005812 /*0x7a28b4*/;
    // 0043b2b4  8dbdd4feffff           -lea edi, [ebp - 0x12c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-300) /* -0x12c */);
    // 0043b2ba  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0043b2bb:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043b2bb  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0043b2bd  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0043b2bf  3c00                   +cmp al, 0
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
    // 0043b2c1  7410                   -je 0x43b2d3
    if (cpu.flags.zf)
    {
        goto L_0x0043b2d3;
    }
    // 0043b2c3  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0043b2c6  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0043b2c9  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0043b2cc  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0043b2cf  3c00                   +cmp al, 0
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
    // 0043b2d1  75e8                   -jne 0x43b2bb
    if (!cpu.flags.zf)
    {
        goto L_0x0043b2bb;
    }
L_0x0043b2d3:
    // 0043b2d3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043b2d4  8d85d4feffff           -lea eax, [ebp - 0x12c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-300) /* -0x12c */);
    // 0043b2da  8dbdd4feffff           -lea edi, [ebp - 0x12c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-300) /* -0x12c */);
    // 0043b2e0  8db210010000           -lea esi, [edx + 0x110]
    cpu.esi = x86::reg32(cpu.edx + x86::reg32(272) /* 0x110 */);
    // 0043b2e6  e855d0ffff             -call 0x438340
    cpu.esp -= 4;
    nfs3hp::load::endWithSlash(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043b2eb  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0043b2ec  2bc9                   +sub ecx, ecx
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0043b2ee  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0043b2ef  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0043b2f1  f2ae                   -repne scasb al, byte ptr es:[edi]
    while (cpu.ecx)
    {
        {
            x86::reg8 tmp1 = cpu.al;
            x86::reg8 tmp2 = x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ees + cpu.edi)));
            x86::reg8 result = tmp1 - tmp2;
            cpu.flags.cf = tmp1 < tmp2;
            cpu.flags.of = 1 & (tmp1 >> 7);
            cpu.flags.of ^= 1 & (result >> 7);
            cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
            cpu.set_szp(result);
        }
        if (cpu.flags.df)
        {
            cpu.edi -= 1;
        }
        else
        {
            cpu.edi += 1;
        }
        --cpu.ecx;
        if (cpu.flags.zf)
            break;
    }
    // 0043b2f3  4f                     -dec edi
    (cpu.edi)--;
L_0x0043b2f4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043b2f4  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0043b2f6  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0043b2f8  3c00                   +cmp al, 0
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
    // 0043b2fa  7410                   -je 0x43b30c
    if (cpu.flags.zf)
    {
        goto L_0x0043b30c;
    }
    // 0043b2fc  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0043b2ff  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0043b302  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0043b305  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0043b308  3c00                   +cmp al, 0
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
    // 0043b30a  75e8                   -jne 0x43b2f4
    if (!cpu.flags.zf)
    {
        goto L_0x0043b2f4;
    }
L_0x0043b30c:
    // 0043b30c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043b30d  8d85d4feffff           -lea eax, [ebp - 0x12c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-300) /* -0x12c */);
    // 0043b313  e8b8d6ffff             -call 0x4389d0
    cpu.esp -= 4;
    nfs3hp::load::removeFolder(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043b318  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0043b31a  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0043b31c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043b31d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043b31e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043b31f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043b320  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043b321  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_43b330: native, nfs3hp::dead::stub_0x43b330 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_43b390(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0043b390  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0043b391  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0043b392  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0043b393  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0043b395  e856d7ffff             -call 0x438af0
    cpu.esp -= 4;
    nfs3hp::load::firstCarGlobal(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0043b39a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043b39a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0043b39c  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0043b39e  743d                   -je 0x43b3dd
    if (cpu.flags.zf)
    {
        goto L_0x0043b3dd;
    }
    // 0043b3a0  f6828003000008         +test byte ptr [edx + 0x380], 8
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.edx + x86::reg32(896) /* 0x380 */) & 8 /*0x8*/));
    // 0043b3a7  742d                   -je 0x43b3d6
    if (cpu.flags.zf)
    {
        goto L_0x0043b3d6;
    }
    // 0043b3a9  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043b3ab  e8c0d7ffff             -call 0x438b70
    cpu.esp -= 4;
    sub_438b70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043b3b0  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0043b3b2  7406                   -je 0x43b3ba
    if (cpu.flags.zf)
    {
        goto L_0x0043b3ba;
    }
    // 0043b3b4  8b0a                   -mov ecx, dword ptr [edx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx);
    // 0043b3b6  8908                   -mov dword ptr [eax], ecx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.ecx;
    // 0043b3b8  eb07                   -jmp 0x43b3c1
    goto L_0x0043b3c1;
L_0x0043b3ba:
    // 0043b3ba  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 0043b3bc  a3704f5500             -mov dword ptr [0x554f70], eax
    app->getMemory<x86::reg32>(x86::reg32(5590896) /* 0x554f70 */) = cpu.eax;
L_0x0043b3c1:
    // 0043b3c1  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043b3c3  e8e8e7ffff             -call 0x439bb0
    cpu.esp -= 4;
    sub_439bb0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043b3c8  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0043b3ca  e8c1640a00             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043b3cf  e81cd7ffff             -call 0x438af0
    cpu.esp -= 4;
    nfs3hp::load::firstCarGlobal(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043b3d4  ebc4                   -jmp 0x43b39a
    goto L_0x0043b39a;
L_0x0043b3d6:
    // 0043b3d6  e835d7ffff             -call 0x438b10
    cpu.esp -= 4;
    nfs3hp::load::nextCarGlobal(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043b3db  ebbd                   -jmp 0x43b39a
    goto L_0x0043b39a;
L_0x0043b3dd:
    // 0043b3dd  e80e090000             -call 0x43bcf0
    cpu.esp -= 4;
    sub_43bcf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043b3e2  e8a9e9ffff             -call 0x439d90
    cpu.esp -= 4;
    nfs3hp::load::loadCarList(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043b3e7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0043b3e9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043b3ea  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043b3eb  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043b3ec  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_43b3f0: native, nfs3hp::load::loadCarSpecs */

/* sub_43b4f0: native, nfs3hp::load::writeCarFec */

/* sub_43b570: native, nfs3hp::load::readCarFecBody */

/* sub_43b630: native, nfs3hp::load::readCarFec */

/* sub_43b680: native, nfs3hp::load::carFecPath */

/* sub_43b740: native, nfs3hp::load::loadCarFec */

/* sub_43b7f0: native, nfs3hp::port::fields::saveCarColours */

/* sub_43b830: native, nfs3hp::ea::openViv */

/* sub_43b9b0: native, nfs3hp::ea::closeViv */

/* sub_43b9d0: native, nfs3hp::ea::findInViv */

/* sub_43ba20: native, nfs3hp::load::placeCarVivPath */

/* sub_43ba30: native, nfs3hp::load::swapBytes */

/* sub_43ba70: native, nfs3hp::load::writeBigEndian */

/* sub_43baa0: native, nfs3hp::load::appendCarpToViv */

/* sub_43bbf0: native, nfs3hp::load::lastCarLoaded */

/* sub_43bc00: native, nfs3hp::load::setLastCarId */

/* sub_43bc40: native, nfs3hp::load::setLastCarName */

/* sub_43bc70: native, nfs3hp::load::setLastCarFlags */

/* sub_43bc90: native, nfs3hp::load::markLastCarFlag8 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_43bcb0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0043bcb0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0043bcb1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0043bcb3  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0043bcb8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043bcb9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_43bcc0: native, nfs3hp::frontend::viewInteriorMapX */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_43bcf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0043bcf0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0043bcf1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0043bcf2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0043bcf3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0043bcf4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0043bcf5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0043bcf7  81ec2c010000           -sub esp, 0x12c
    (cpu.esp) -= x86::reg32(x86::sreg32(300 /*0x12c*/));
    // 0043bcfd  b8b4287a00             -mov eax, 0x7a28b4
    cpu.eax = 8005812 /*0x7a28b4*/;
    // 0043bd02  e8a9c9ffff             -call 0x4386b0
    cpu.esp -= 4;
    nfs3hp::load::firstSubfolder(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0043bd07:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043bd07  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0043bd09  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0043bd0b  0f84b9000000           -je 0x43bdca
    if (cpu.flags.zf)
    {
        goto L_0x0043bdca;
    }
    // 0043bd11  803a50                 +cmp byte ptr [edx], 0x50
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edx);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(80 /*0x50*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0043bd14  0f85a6000000           -jne 0x43bdc0
    if (!cpu.flags.zf)
    {
        goto L_0x0043bdc0;
    }
    // 0043bd1a  8a4201                 -mov al, byte ptr [edx + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 0043bd1d  fec0                   -inc al
    (cpu.al)++;
    // 0043bd1f  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0043bd24  f680f04e560020         +test byte ptr [eax + 0x564ef0], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5656304) /* 0x564ef0 */) & 32 /*0x20*/));
    // 0043bd2b  0f848f000000           -je 0x43bdc0
    if (cpu.flags.zf)
    {
        goto L_0x0043bdc0;
    }
    // 0043bd31  8a4202                 -mov al, byte ptr [edx + 2]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(2) /* 0x2 */);
    // 0043bd34  fec0                   -inc al
    (cpu.al)++;
    // 0043bd36  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0043bd3b  f680f04e560020         +test byte ptr [eax + 0x564ef0], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5656304) /* 0x564ef0 */) & 32 /*0x20*/));
    // 0043bd42  0f8478000000           -je 0x43bdc0
    if (cpu.flags.zf)
    {
        goto L_0x0043bdc0;
    }
    // 0043bd48  8a4203                 -mov al, byte ptr [edx + 3]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(3) /* 0x3 */);
    // 0043bd4b  fec0                   -inc al
    (cpu.al)++;
    // 0043bd4d  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0043bd52  f680f04e560020         +test byte ptr [eax + 0x564ef0], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5656304) /* 0x564ef0 */) & 32 /*0x20*/));
    // 0043bd59  7465                   -je 0x43bdc0
    if (cpu.flags.zf)
    {
        goto L_0x0043bdc0;
    }
    // 0043bd5b  beb4287a00             -mov esi, 0x7a28b4
    cpu.esi = 8005812 /*0x7a28b4*/;
    // 0043bd60  8dbdd4feffff           -lea edi, [ebp - 0x12c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-300) /* -0x12c */);
    // 0043bd66  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0043bd67:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043bd67  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0043bd69  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0043bd6b  3c00                   +cmp al, 0
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
    // 0043bd6d  7410                   -je 0x43bd7f
    if (cpu.flags.zf)
    {
        goto L_0x0043bd7f;
    }
    // 0043bd6f  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0043bd72  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0043bd75  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0043bd78  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0043bd7b  3c00                   +cmp al, 0
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
    // 0043bd7d  75e8                   -jne 0x43bd67
    if (!cpu.flags.zf)
    {
        goto L_0x0043bd67;
    }
L_0x0043bd7f:
    // 0043bd7f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043bd80  8d85d4feffff           -lea eax, [ebp - 0x12c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-300) /* -0x12c */);
    // 0043bd86  8dbdd4feffff           -lea edi, [ebp - 0x12c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-300) /* -0x12c */);
    // 0043bd8c  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0043bd8e  e8adc5ffff             -call 0x438340
    cpu.esp -= 4;
    nfs3hp::load::endWithSlash(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043bd93  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0043bd94  2bc9                   +sub ecx, ecx
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0043bd96  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 0043bd97  b000                   -mov al, 0
    cpu.al = 0 /*0x0*/;
    // 0043bd99  f2ae                   -repne scasb al, byte ptr es:[edi]
    while (cpu.ecx)
    {
        {
            x86::reg8 tmp1 = cpu.al;
            x86::reg8 tmp2 = x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ees + cpu.edi)));
            x86::reg8 result = tmp1 - tmp2;
            cpu.flags.cf = tmp1 < tmp2;
            cpu.flags.of = 1 & (tmp1 >> 7);
            cpu.flags.of ^= 1 & (result >> 7);
            cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
            cpu.set_szp(result);
        }
        if (cpu.flags.df)
        {
            cpu.edi -= 1;
        }
        else
        {
            cpu.edi += 1;
        }
        --cpu.ecx;
        if (cpu.flags.zf)
            break;
    }
    // 0043bd9b  4f                     -dec edi
    (cpu.edi)--;
L_0x0043bd9c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0043bd9c  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0043bd9e  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0043bda0  3c00                   +cmp al, 0
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
    // 0043bda2  7410                   -je 0x43bdb4
    if (cpu.flags.zf)
    {
        goto L_0x0043bdb4;
    }
    // 0043bda4  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0043bda7  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0043bdaa  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0043bdad  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0043bdb0  3c00                   +cmp al, 0
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
    // 0043bdb2  75e8                   -jne 0x43bd9c
    if (!cpu.flags.zf)
    {
        goto L_0x0043bd9c;
    }
L_0x0043bdb4:
    // 0043bdb4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043bdb5  8d85d4feffff           -lea eax, [ebp - 0x12c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-300) /* -0x12c */);
    // 0043bdbb  e810ccffff             -call 0x4389d0
    cpu.esp -= 4;
    nfs3hp::load::removeFolder(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0043bdc0:
    // 0043bdc0  e81bcaffff             -call 0x4387e0
    cpu.esp -= 4;
    nfs3hp::load::nextSubfolder(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043bdc5  e93dffffff             -jmp 0x43bd07
    goto L_0x0043bd07;
L_0x0043bdca:
    // 0043bdca  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0043bdcc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043bdcd  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043bdce  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043bdcf  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043bdd0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043bdd1  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_43bde0: native, nfs3hp::dead::stub_0x43bde0 */

/* sub_43be40: native, nfs3hp::frontend::controlsPollJoystickBinding */

/* sub_43c000: native, nfs3hp::frontend::bindingIsDigital */

/* sub_43c040: native, nfs3hp::frontend::noteDigitalControls */

/* sub_43c110: native, nfs3hp::frontend::controlsDrawBinding */

/* sub_43c900: native, nfs3hp::frontend::controlsDrawList */

/* sub_43cda0: native, nfs3hp::frontend::controlsKeyAsBinding */

/* sub_43ce80: native, nfs3hp::frontend::controlsWarnDigitalBinding */

/* sub_43cf80: native, nfs3hp::frontend::controlsListKey */

/* sub_43d2b0: native, nfs3hp::frontend::controlsListFitBox */

/* sub_43d310: native, nfs3hp::frontend::controlsListPrepare */

/* sub_43d3f0: native, nfs3hp::frontend::controlsCancelKey */

/* sub_43d410: native, nfs3hp::frontend::controlsResetAnswered */

/* sub_43d460: native, nfs3hp::frontend::controlsResetClicked */

/* sub_43d4d0: native, nfs3hp::frontend::controlsOnEnter */

/* sub_43d5f0: native, nfs3hp::frontend::controlsOnExit */

/* sub_43d650: native, nfs3hp::frontend::controlsWaitJoystickRelease */

/* sub_43d6a0: native, nfs3hp::frontend::controlsClearConflicts */

/* sub_43d7f0: native, nfs3hp::frontend::controlsBindingKeyName */

/* sub_43d920: native, nfs3hp::frontend::controlsSampleAxes */

/* sub_43d9a0: native, nfs3hp::frontend::deadZoneFillList */

/* sub_43d9f0: native, nfs3hp::frontend::controllerName */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_43da60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0043da60  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0043da61  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0043da62  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0043da63  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0043da65  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0043da68  c645f009               -mov byte ptr [ebp - 0x10], 9
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = 9 /*0x9*/;
    // 0043da6c  a0f4d46f00             -mov al, byte ptr [0x6fd4f4]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 0043da71  8845f1                 -mov byte ptr [ebp - 0xf], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-15) /* -0xf */) = cpu.al;
    // 0043da74  a1dcd26f00             -mov eax, dword ptr [0x6fd2dc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328476) /* 0x6fd2dc */);
    // 0043da79  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 0043da7c  a1e0d26f00             -mov eax, dword ptr [0x6fd2e0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328480) /* 0x6fd2e0 */);
    // 0043da81  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 0043da86  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0043da89  a1e4d26f00             -mov eax, dword ptr [0x6fd2e4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328484) /* 0x6fd2e4 */);
    // 0043da8e  8d55f0                 -lea edx, [ebp - 0x10]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0043da91  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0043da94  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0043da96  e845860600             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0043da9b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0043da9d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043da9e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043da9f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0043daa0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_43dab0: native, nfs3hp::frontend::connectApplyPlayerVector */

/* sub_43db20: native, nfs3hp::frontend::colourMakeHueRamp */

/* sub_43dbe0: native, nfs3hp::port::fields::wheelFromCarColour */

/* sub_43dc90: native, nfs3hp::frontend::colourApplyToCar */

/* sub_43dd50: native, nfs3hp::port::fields::wheelForPlayer */

/* sub_43dd90: native, nfs3hp::frontend::colourAccept */

/* sub_43ddd0: native, nfs3hp::frontend::colourRevert */

/* sub_43de20: native, nfs3hp::frontend::colourHsvToArgb */

/* sub_43de60: native, nfs3hp::frontend::colourTriangleFromColour */

/* sub_43def0: native, nfs3hp::frontend::colourFromTriangle */

/* sub_43dfb0: native, nfs3hp::frontend::colourDrawFrame */

/* sub_43e040: native, nfs3hp::frontend::colourDrawSwatch */

/* sub_43e0d0: native, nfs3hp::frontend::colourDrawButtons */

/* sub_43e1f0: native, nfs3hp::frontend::colourDrawMarkers */

/* sub_43e310: native, nfs3hp::frontend::colourDrawTriangle */

/* sub_43e490: native, nfs3hp::frontend::colourDrawValueBar */

/* sub_43e7a0: native, nfs3hp::frontend::colourDragMoved */

/* sub_43e8e0: native, nfs3hp::frontend::colourHitRegion */

/* sub_43ead0: native, nfs3hp::frontend::colourMouse */

/* sub_43ebc0: native, nfs3hp::frontend::colourDraw */

/* sub_43ec40: native, nfs3hp::frontend::colourKey */

/* sub_43ef80: native, nfs3hp::frontend::colourPointerKind */

/* sub_43eff0: native, nfs3hp::frontend::colourFitBox */

/* sub_43f030: native, nfs3hp::frontend::colourPrepare */

/* sub_43f160: native, nfs3hp::frontend::colourShown */

/* sub_43f1f0: native, nfs3hp::frontend::connectListingRedraw */

/* sub_43f220: native, nfs3hp::frontend::ipxRefreshGames */

/* sub_43f320: native, nfs3hp::frontend::connectOpenProtocolList */

/* sub_43f4e0: native, nfs3hp::frontend::connectOpenAddressDialog */

/* sub_43f550: native, nfs3hp::frontend::connectModemLabel */

/* sub_43f610: native, nfs3hp::frontend::connectComPortLabel */

/* sub_43f640: native, nfs3hp::frontend::ipxGameNameAnswer */

/* sub_43f680: native, nfs3hp::frontend::connectAddressAnswer */

/* sub_43f760: native, nfs3hp::frontend::connectTcpAnswer */

/* sub_43f7e0: native, nfs3hp::frontend::connectProtocolListAnswer */

/* sub_43f960: native, nfs3hp::frontend::connectSerialAnswer */

/* sub_43fa00: native, nfs3hp::frontend::connectPhoneAnswer */

/* sub_43faf0: native, nfs3hp::frontend::connectModemAnswer */

/* sub_43fc20: native, nfs3hp::frontend::connectRaceNetMissingAnswer */

/* sub_43fc50: native, nfs3hp::frontend::connectRaceNetAnswer */

/* sub_43fcb0: native, nfs3hp::frontend::multiMenuLeaveAnswer */

/* sub_43fd10: native, nfs3hp::frontend::connectRaceNetClicked */

/* sub_43fdd0: native, nfs3hp::port::buttons::joinListedGame */

/* sub_43fe50: native, nfs3hp::frontend::connectTcpClicked */

/* sub_43fee0: native, nfs3hp::frontend::connectSerialClicked */

/* sub_43ff50: native, nfs3hp::frontend::connectModemClicked */

}
