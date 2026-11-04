#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_51de60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051de60  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051de61  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051de62  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051de63  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051de64  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051de66  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051de68  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0051de6a  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0051de6c  7e68                   -jle 0x51ded6
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051ded6;
    }
    // 0051de6e  bd3f000000             -mov ebp, 0x3f
    cpu.ebp = 63 /*0x3f*/;
    // 0051de73  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0051de75  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
L_0x0051de77:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051de77  81feff000000           +cmp esi, 0xff
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(255 /*0xff*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051de7d  755c                   -jne 0x51dedb
    if (!cpu.flags.zf)
    {
        goto L_0x0051dedb;
    }
    // 0051de7f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0051de81:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051de81  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051de83  884103                 -mov byte ptr [ecx + 3], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(3) /* 0x3 */) = cpu.al;
    // 0051de86  8a13                   -mov dl, byte ptr [ebx]
    cpu.dl = app->getMemory<x86::reg8>(cpu.ebx);
    // 0051de88  69d2ff000000           -imul edx, edx, 0xff
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(255 /*0xff*/)));
    // 0051de8e  83c220                 -add edx, 0x20
    (cpu.edx) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 0051de91  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051de93  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0051de96  f7fd                   -idiv ebp
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebp);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0051de98  884102                 -mov byte ptr [ecx + 2], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(2) /* 0x2 */) = cpu.al;
    // 0051de9b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051de9d  8a4301                 -mov al, byte ptr [ebx + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(1) /* 0x1 */);
    // 0051dea0  69c0ff000000           -imul eax, eax, 0xff
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(255 /*0xff*/)));
    // 0051dea6  8d5020                 -lea edx, [eax + 0x20]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(32) /* 0x20 */);
    // 0051dea9  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051deab  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0051deae  f7fd                   -idiv ebp
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebp);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0051deb0  884101                 -mov byte ptr [ecx + 1], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0051deb3  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051deb5  8a4302                 -mov al, byte ptr [ebx + 2]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(2) /* 0x2 */);
    // 0051deb8  69c0ff000000           -imul eax, eax, 0xff
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(255 /*0xff*/)));
    // 0051debe  8d5020                 -lea edx, [eax + 0x20]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(32) /* 0x20 */);
    // 0051dec1  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051dec3  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0051dec6  f7fd                   -idiv ebp
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebp);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 0051dec8  83c104                 -add ecx, 4
    (cpu.ecx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051decb  83c303                 -add ebx, 3
    (cpu.ebx) += x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0051dece  46                     -inc esi
    (cpu.esi)++;
    // 0051decf  8841fc                 -mov byte ptr [ecx - 4], al
    app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(-4) /* -0x4 */) = cpu.al;
    // 0051ded2  39fe                   +cmp esi, edi
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ded4  7ca1                   -jl 0x51de77
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051de77;
    }
L_0x0051ded6:
    // 0051ded6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ded7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ded8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ded9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051deda  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051dedb:
    // 0051dedb  b8ff000000             -mov eax, 0xff
    cpu.eax = 255 /*0xff*/;
    // 0051dee0  eb9f                   -jmp 0x51de81
    goto L_0x0051de81;
}

/* sub_51def0: native, nfs3hp::dead::stub_0x51def0 */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_51df60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051df60  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051df61  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051df62  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051df63  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051df66  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051df68  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051df6a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051df6c  7e33                   -jle 0x51dfa1
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051dfa1;
    }
    // 0051df6e  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
L_0x0051df70:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051df70  81f9ff000000           +cmp ecx, 0xff
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(255 /*0xff*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051df76  7430                   -je 0x51dfa8
    if (cpu.flags.zf)
    {
        goto L_0x0051dfa8;
    }
    // 0051df78  c70424ff000000         -mov dword ptr [esp], 0xff
    app->getMemory<x86::reg32>(cpu.esp) = 255 /*0xff*/;
L_0x0051df7f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051df7f  8a1c24                 -mov bl, byte ptr [esp]
    cpu.bl = app->getMemory<x86::reg8>(cpu.esp);
    // 0051df82  885803                 -mov byte ptr [eax + 3], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(3) /* 0x3 */) = cpu.bl;
    // 0051df85  8a1a                   -mov bl, byte ptr [edx]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx);
    // 0051df87  885802                 -mov byte ptr [eax + 2], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(2) /* 0x2 */) = cpu.bl;
    // 0051df8a  8a5a01                 -mov bl, byte ptr [edx + 1]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 0051df8d  83c203                 -add edx, 3
    (cpu.edx) += x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0051df90  885801                 -mov byte ptr [eax + 1], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(1) /* 0x1 */) = cpu.bl;
    // 0051df93  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051df96  8a5aff                 -mov bl, byte ptr [edx - 1]
    cpu.bl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(-1) /* -0x1 */);
    // 0051df99  41                     -inc ecx
    (cpu.ecx)++;
    // 0051df9a  8858fc                 -mov byte ptr [eax - 4], bl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(-4) /* -0x4 */) = cpu.bl;
    // 0051df9d  39f1                   +cmp ecx, esi
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
    // 0051df9f  7ccf                   -jl 0x51df70
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051df70;
    }
L_0x0051dfa1:
    // 0051dfa1  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051dfa4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051dfa5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051dfa6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051dfa7  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051dfa8:
    // 0051dfa8  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0051dfaa  893c24                 -mov dword ptr [esp], edi
    app->getMemory<x86::reg32>(cpu.esp) = cpu.edi;
    // 0051dfad  ebd0                   -jmp 0x51df7f
    goto L_0x0051df7f;
}

/* sub_51dfb0: native, nfs3hp::dead::stub_0x51dfb0 */

/* sub_51e020: native, nfs3hp::dead::stub_0x51e020 */

/* sub_51e060: native, nfs3hp::watcom::wctomb_ */

/* sub_51e0a0: native, nfs3hp::watcom::utoa */

/* sub_51e0ec: native, nfs3hp::watcom::itoa */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_51e110(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e110  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e111  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0051e116  b868135500             -mov eax, 0x551368
    cpu.eax = 5575528 /*0x551368*/;
    // 0051e11b  e8ec3bffff             -call 0x511d0c
    cpu.esp -= 4;
    nfs3hp::watcom::fatal_runtime_error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e120  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e121  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_51e130: native, nfs3hp::watcom::ulltoa */

/* sub_51e1c8: native, nfs3hp::dead::stub_0x51e1c8 */

/* sub_51e220: native, nfs3hp::watcom::prtfUtoa */

/* sub_51e26c: native, nfs3hp::dead::stub_0x51e26c */

/* sub_51e290: native, nfs3hp::dead::stub_0x51e290 */

/* align: skip 0x8b 0xc0 */
void Application::sub_51e2a0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e2a0  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051e2a2  e9f9780000             -jmp 0x525ba0
    return sub_525ba0(app, cpu.handOver());
}

/* sub_51e2b0: native, nfs3hp::watcom::handlesFull */

/* sub_51e304: native, nfs3hp::watcom::allocPOSIXHandle */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_51e388(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e388  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e389  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e38a  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e38b  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051e38c  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051e38d  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051e390  8b3dbcac5600           -mov edi, dword ptr [0x56acbc]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5680316) /* 0x56acbc */);
    // 0051e396  890424                 -mov dword ptr [esp], eax
    app->getMemory<x86::reg32>(cpu.esp) = cpu.eax;
    // 0051e399  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 0051e39b  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051e39d  0f8caa000000           -jl 0x51e44d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051e44d;
    }
    // 0051e3a3  ff1598775600           -call dword ptr [0x567798]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666712) /* 0x567798 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e3a9  83fa01                 +cmp edx, 1
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e3ac  7209                   -jb 0x51e3b7
    if (cpu.flags.cf)
    {
        goto L_0x0051e3b7;
    }
    // 0051e3ae  7613                   -jbe 0x51e3c3
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051e3c3;
    }
    // 0051e3b0  83fa02                 +cmp edx, 2
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e3b3  7416                   -je 0x51e3cb
    if (cpu.flags.zf)
    {
        goto L_0x0051e3cb;
    }
    // 0051e3b5  eb21                   -jmp 0x51e3d8
    goto L_0x0051e3d8;
L_0x0051e3b7:
    // 0051e3b7  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051e3b9  751d                   -jne 0x51e3d8
    if (!cpu.flags.zf)
    {
        goto L_0x0051e3d8;
    }
    // 0051e3bb  8b0c24                 -mov ecx, dword ptr [esp]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051e3be  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e3bf  6af6                   -push -0xa
    app->getMemory<x86::reg32>(cpu.esp-4) = -10 /*-0xa*/;
    cpu.esp -= 4;
    // 0051e3c1  eb0e                   -jmp 0x51e3d1
    goto L_0x0051e3d1;
L_0x0051e3c3:
    // 0051e3c3  8b1c24                 -mov ebx, dword ptr [esp]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051e3c6  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e3c7  6af5                   -push -0xb
    app->getMemory<x86::reg32>(cpu.esp-4) = -11 /*-0xb*/;
    cpu.esp -= 4;
    // 0051e3c9  eb06                   -jmp 0x51e3d1
    goto L_0x0051e3d1;
L_0x0051e3cb:
    // 0051e3cb  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051e3ce  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e3cf  6af4                   -push -0xc
    app->getMemory<x86::reg32>(cpu.esp-4) = -12 /*-0xc*/;
    cpu.esp -= 4;
L_0x0051e3d1:
    // 0051e3d1  2eff15f8455300         -call dword ptr cs:[0x5345f8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457400) /* 0x5345f8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051e3d8:
    // 0051e3d8  8b2dc0ac5600           -mov ebp, dword ptr [0x56acc0]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(5680320) /* 0x56acc0 */);
    // 0051e3de  8d0cb500000000         -lea ecx, [esi*4]
    cpu.ecx = x86::reg32(cpu.esi * 4);
    // 0051e3e5  8b3dbcac5600           -mov edi, dword ptr [0x56acbc]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5680316) /* 0x56acbc */);
    // 0051e3eb  39ee                   +cmp esi, ebp
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebp));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e3ed  7d09                   -jge 0x51e3f8
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051e3f8;
    }
    // 0051e3ef  01f9                   +add ecx, edi
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051e3f1  8b0424                 -mov eax, dword ptr [esp]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    // 0051e3f4  8901                   -mov dword ptr [ecx], eax
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.eax;
    // 0051e3f6  eb49                   -jmp 0x51e441
    goto L_0x0051e441;
L_0x0051e3f8:
    // 0051e3f8  8d5104                 -lea edx, [ecx + 4]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 0051e3fb  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051e3fd  e8bea1ffff             -call 0x5185c0
    cpu.esp -= 4;
    nfs3hp::watcom::nrealloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e402  8b15c0ac5600           -mov edx, dword ptr [0x56acc0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5680320) /* 0x56acc0 */);
    // 0051e408  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0051e40a  39f2                   +cmp edx, esi
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
    // 0051e40c  7d18                   -jge 0x51e426
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051e426;
    }
    // 0051e40e  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0051e415  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
L_0x0051e417:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051e417  c7040300000000         -mov dword ptr [ebx + eax], 0
    app->getMemory<x86::reg32>(cpu.ebx + cpu.eax * 1) = 0 /*0x0*/;
    // 0051e41e  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051e421  42                     -inc edx
    (cpu.edx)++;
    // 0051e422  39c8                   +cmp eax, ecx
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
    // 0051e424  7cf1                   -jl 0x51e417
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051e417;
    }
L_0x0051e426:
    // 0051e426  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 0051e42d  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051e430  46                     -inc esi
    (cpu.esi)++;
    // 0051e431  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 0051e433  893dbcac5600           -mov dword ptr [0x56acbc], edi
    app->getMemory<x86::reg32>(x86::reg32(5680316) /* 0x56acbc */) = cpu.edi;
    // 0051e439  8935c0ac5600           -mov dword ptr [0x56acc0], esi
    app->getMemory<x86::reg32>(x86::reg32(5680320) /* 0x56acc0 */) = cpu.esi;
    // 0051e43f  8910                   -mov dword ptr [eax], edx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.edx;
L_0x0051e441:
    // 0051e441  ff159c775600           -call dword ptr [0x56779c]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5666716) /* 0x56779c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e447  8b3dbcac5600           -mov edi, dword ptr [0x56acbc]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5680316) /* 0x56acbc */);
L_0x0051e44d:
    // 0051e44d  8b3dbcac5600           -mov edi, dword ptr [0x56acbc]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5680316) /* 0x56acbc */);
    // 0051e453  83c404                 -add esp, 4
    (cpu.esp) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051e456  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e457  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e458  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e459  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e45a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e45b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_51e45c: native, nfs3hp::watcom::freePOSIXHandle */

/* sub_51e490: native, nfs3hp::watcom::initPOSIXHandles */

/* sub_51e4f0: native, nfs3hp::watcom::NTGetFakeHandle */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_51e51c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e51c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e51d  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e51e  8b15bcac5600           -mov edx, dword ptr [0x56acbc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5680316) /* 0x56acbc */);
    // 0051e524  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051e526  740f                   -je 0x51e537
    if (cpu.flags.zf)
    {
        goto L_0x0051e537;
    }
    // 0051e528  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051e52a  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051e52c  e8bf94fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e531  891dbcac5600           -mov dword ptr [0x56acbc], ebx
    app->getMemory<x86::reg32>(x86::reg32(5680316) /* 0x56acbc */) = cpu.ebx;
L_0x0051e537:
    // 0051e537  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e538  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e539  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_51e540(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e540  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_51e544(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e544  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e545  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e546  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e547  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e548  8b1580baa000           -mov edx, dword ptr [0xa0ba80]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10533504) /* 0xa0ba80 */);
    // 0051e54e  83fa40                 +cmp edx, 0x40
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(64 /*0x40*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e551  7d1e                   -jge 0x51e571
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051e571;
    }
    // 0051e553  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0051e55a  bb50b4a000             -mov ebx, 0xa0b450
    cpu.ebx = 10531920 /*0xa0b450*/;
    // 0051e55f  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0051e561  8d7201                 -lea esi, [edx + 1]
    cpu.esi = x86::reg32(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 0051e564  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0051e567  893580baa000           -mov dword ptr [0xa0ba80], esi
    app->getMemory<x86::reg32>(x86::reg32(10533504) /* 0xa0ba80 */) = cpu.esi;
    // 0051e56d  01c3                   +add ebx, eax
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051e56f  eb67                   -jmp 0x51e5d8
    goto L_0x0051e5d8;
L_0x0051e571:
    // 0051e571  ba18000000             -mov edx, 0x18
    cpu.edx = 24 /*0x18*/;
    // 0051e576  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051e57b  e870780000             -call 0x525df0
    cpu.esp -= 4;
    nfs3hp::watcom::calloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e580  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051e582  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051e584  750f                   -jne 0x51e595
    if (!cpu.flags.zf)
    {
        goto L_0x0051e595;
    }
    // 0051e586  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0051e58b  b88c135500             -mov eax, 0x55138c
    cpu.eax = 5575564 /*0x55138c*/;
    // 0051e590  e87737ffff             -call 0x511d0c
    cpu.esp -= 4;
    nfs3hp::watcom::fatal_runtime_error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051e595:
    // 0051e595  8b1584baa000           -mov edx, dword ptr [0xa0ba84]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10533508) /* 0xa0ba84 */);
    // 0051e59b  42                     -inc edx
    (cpu.edx)++;
    // 0051e59c  a188baa000             -mov eax, dword ptr [0xa0ba88]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10533512) /* 0xa0ba88 */);
    // 0051e5a1  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 0051e5a4  e817a0ffff             -call 0x5185c0
    cpu.esp -= 4;
    nfs3hp::watcom::nrealloc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e5a9  a388baa000             -mov dword ptr [0xa0ba88], eax
    app->getMemory<x86::reg32>(x86::reg32(10533512) /* 0xa0ba88 */) = cpu.eax;
    // 0051e5ae  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051e5b0  750f                   -jne 0x51e5c1
    if (!cpu.flags.zf)
    {
        goto L_0x0051e5c1;
    }
    // 0051e5b2  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0051e5b7  b8b0135500             -mov eax, 0x5513b0
    cpu.eax = 5575600 /*0x5513b0*/;
    // 0051e5bc  e84b37ffff             -call 0x511d0c
    cpu.esp -= 4;
    nfs3hp::watcom::fatal_runtime_error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051e5c1:
    // 0051e5c1  a184baa000             -mov eax, dword ptr [0xa0ba84]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10533508) /* 0xa0ba84 */);
    // 0051e5c6  8b1588baa000           -mov edx, dword ptr [0xa0ba88]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10533512) /* 0xa0ba88 */);
    // 0051e5cc  8d4801                 -lea ecx, [eax + 1]
    cpu.ecx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0051e5cf  891c82                 -mov dword ptr [edx + eax*4], ebx
    app->getMemory<x86::reg32>(cpu.edx + cpu.eax * 4) = cpu.ebx;
    // 0051e5d2  890d84baa000           -mov dword ptr [0xa0ba84], ecx
    app->getMemory<x86::reg32>(x86::reg32(10533508) /* 0xa0ba84 */) = cpu.ecx;
L_0x0051e5d8:
    // 0051e5d8  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e5d9  2eff1584455300         -call dword ptr cs:[0x534584]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457284) /* 0x534584 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e5e0  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051e5e2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e5e3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e5e4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e5e5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e5e6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_51e5e8(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e5e8  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e5e9  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e5ea  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e5eb  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e5ec  8b1580baa000           -mov edx, dword ptr [0xa0ba80]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10533504) /* 0xa0ba80 */);
    // 0051e5f2  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0051e5f4  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051e5f6  7e1b                   -jle 0x51e613
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051e613;
    }
    // 0051e5f8  bb50b4a000             -mov ebx, 0xa0b450
    cpu.ebx = 10531920 /*0xa0b450*/;
L_0x0051e5fd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051e5fd  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e5fe  46                     -inc esi
    (cpu.esi)++;
    // 0051e5ff  2eff15ac445300         -call dword ptr cs:[0x5344ac]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457068) /* 0x5344ac */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e606  8b0d80baa000           -mov ecx, dword ptr [0xa0ba80]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(10533504) /* 0xa0ba80 */);
    // 0051e60c  83c318                 -add ebx, 0x18
    (cpu.ebx) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0051e60f  39ce                   +cmp esi, ecx
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e611  7cea                   -jl 0x51e5fd
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051e5fd;
    }
L_0x0051e613:
    // 0051e613  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e614  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e615  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e616  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e617  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_51e618(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e618  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e619  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e61a  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e61b  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e61c  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051e61d  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051e61e  8b1584baa000           -mov edx, dword ptr [0xa0ba84]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10533508) /* 0xa0ba84 */);
    // 0051e624  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0051e626  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051e628  7e2d                   -jle 0x51e657
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051e657;
    }
    // 0051e62a  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
L_0x0051e62c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051e62c  a188baa000             -mov eax, dword ptr [0xa0ba88]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10533512) /* 0xa0ba88 */);
    // 0051e631  8b0c03                 -mov ecx, dword ptr [ebx + eax]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx + cpu.eax * 1);
    // 0051e634  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e635  2eff15ac445300         -call dword ptr cs:[0x5344ac]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457068) /* 0x5344ac */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e63c  a188baa000             -mov eax, dword ptr [0xa0ba88]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10533512) /* 0xa0ba88 */);
    // 0051e641  8b0403                 -mov eax, dword ptr [ebx + eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + cpu.eax * 1);
    // 0051e644  46                     -inc esi
    (cpu.esi)++;
    // 0051e645  e8a693fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e64a  8b3d84baa000           -mov edi, dword ptr [0xa0ba84]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10533508) /* 0xa0ba84 */);
    // 0051e650  83c304                 -add ebx, 4
    (cpu.ebx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0051e653  39fe                   +cmp esi, edi
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e655  7cd5                   -jl 0x51e62c
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0051e62c;
    }
L_0x0051e657:
    // 0051e657  8b2d88baa000           -mov ebp, dword ptr [0xa0ba88]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(10533512) /* 0xa0ba88 */);
    // 0051e65d  85ed                   +test ebp, ebp
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebp & cpu.ebp));
    // 0051e65f  7407                   -je 0x51e668
    if (cpu.flags.zf)
    {
        goto L_0x0051e668;
    }
    // 0051e661  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 0051e663  e88893fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051e668:
    // 0051e668  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e669  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e66a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e66b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e66c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e66d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e66e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_51e670(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e670  c7400800000000         -mov dword ptr [eax + 8], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = 0 /*0x0*/;
    // 0051e677  c7400c00000000         -mov dword ptr [eax + 0xc], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */) = 0 /*0x0*/;
    // 0051e67e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_51e680(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e680  b820b3a000             -mov eax, 0xa0b320
    cpu.eax = 10531616 /*0xa0b320*/;
    // 0051e685  e992000000             -jmp 0x51e71c
    return sub_51e71c(app, cpu.handOver());
}

/* align: skip 0x8b 0xc0 */
void Application::sub_51e68c(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e68c  b820b3a000             -mov eax, 0xa0b320
    cpu.eax = 10531616 /*0xa0b320*/;
    // 0051e691  e9ea000000             -jmp 0x51e780
    return sub_51e780(app, cpu.handOver());
}

/* align: skip 0x8b 0xc0 */
void Application::sub_51e698(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e698  83e00f                 -and eax, 0xf
    cpu.eax &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 0051e69b  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0051e69e  0540b3a000             +add eax, 0xa0b340
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(10531648 /*0xa0b340*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051e6a3  e974000000             -jmp 0x51e71c
    return sub_51e71c(app, cpu.handOver());
}

/* align: skip  */
void Application::sub_51e6a8(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e6a8  83e00f                 -and eax, 0xf
    cpu.eax &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 0051e6ab  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0051e6ae  0540b3a000             +add eax, 0xa0b340
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(10531648 /*0xa0b340*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051e6b3  e9c8000000             -jmp 0x51e780
    return sub_51e780(app, cpu.handOver());
}

/* align: skip  */
void Application::sub_51e6b8(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e6b8  e947fcffff             -jmp 0x51e304
    return nfs3hp::watcom::allocPOSIXHandle(app, cpu.handOver());
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_51e6c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e6c0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e6c1  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051e6c3  83e00f                 -and eax, 0xf
    cpu.eax &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 0051e6c6  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0051e6c9  0540b3a000             -add eax, 0xa0b340
    (cpu.eax) += x86::reg32(x86::sreg32(10531648 /*0xa0b340*/));
    // 0051e6ce  e89dffffff             -call 0x51e670
    cpu.esp -= 4;
    sub_51e670(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e6d3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051e6d5  e882fdffff             -call 0x51e45c
    cpu.esp -= 4;
    nfs3hp::watcom::freePOSIXHandle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e6da  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e6db  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_51e6dc(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e6dc  b840b4a000             -mov eax, 0xa0b440
    cpu.eax = 10531904 /*0xa0b440*/;
    // 0051e6e1  eb39                   -jmp 0x51e71c
    return sub_51e71c(app, cpu.handOver());
}

/* align: skip 0x90 */
void Application::sub_51e6e4(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e6e4  b840b4a000             -mov eax, 0xa0b440
    cpu.eax = 10531904 /*0xa0b440*/;
    // 0051e6e9  e992000000             -jmp 0x51e780
    return sub_51e780(app, cpu.handOver());
}

/* align: skip 0x8b 0xc0 */
void Application::sub_51e6f0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e6f0  b830b3a000             -mov eax, 0xa0b330
    cpu.eax = 10531632 /*0xa0b330*/;
    // 0051e6f5  eb25                   -jmp 0x51e71c
    return sub_51e71c(app, cpu.handOver());
}

/* align: skip 0x90 */
void Application::sub_51e6f8(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e6f8  b830b3a000             -mov eax, 0xa0b330
    cpu.eax = 10531632 /*0xa0b330*/;
    // 0051e6fd  e97e000000             -jmp 0x51e780
    return sub_51e780(app, cpu.handOver());
}

/* align: skip 0x8b 0xc0 */
void Application::sub_51e704(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e704  b860baa000             -mov eax, 0xa0ba60
    cpu.eax = 10533472 /*0xa0ba60*/;
    // 0051e709  eb11                   -jmp 0x51e71c
    return sub_51e71c(app, cpu.handOver());
}

/* align: skip 0x90 */
void Application::sub_51e70c(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e70c  b860baa000             -mov eax, 0xa0ba60
    cpu.eax = 10533472 /*0xa0ba60*/;
    // 0051e711  eb6d                   -jmp 0x51e780
    return sub_51e780(app, cpu.handOver());
}

/* align: skip 0x90 */
void Application::sub_51e714(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e714  b870baa000             -mov eax, 0xa0ba70
    cpu.eax = 10533488 /*0xa0ba70*/;
    // 0051e719  8d4000                 -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 0051e71c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e71d  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e71e  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e71f  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e720  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051e721  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051e723  2eff150c455300         -call dword ptr cs:[0x53450c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457164) /* 0x53450c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e72a  8b5308                 -mov edx, dword ptr [ebx + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
    // 0051e72d  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051e72f  39d0                   +cmp eax, edx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e731  743b                   -je 0x51e76e
    if (cpu.flags.zf)
    {
        goto L_0x0051e76e;
    }
    // 0051e733  837b0400               +cmp dword ptr [ebx + 4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e737  7528                   -jne 0x51e761
    if (!cpu.flags.zf)
    {
        goto L_0x0051e761;
    }
    // 0051e739  b850baa000             -mov eax, 0xa0ba50
    cpu.eax = 10533456 /*0xa0ba50*/;
    // 0051e73e  e8d9ffffff             -call 0x51e71c
    cpu.esp -= 4;
    sub_51e71c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e743  837b0400               +cmp dword ptr [ebx + 4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e747  750e                   -jne 0x51e757
    if (!cpu.flags.zf)
    {
        goto L_0x0051e757;
    }
    // 0051e749  e8f6fdffff             -call 0x51e544
    cpu.esp -= 4;
    sub_51e544(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e74e  c7430401000000         -mov dword ptr [ebx + 4], 1
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */) = 1 /*0x1*/;
    // 0051e755  8903                   -mov dword ptr [ebx], eax
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.eax;
L_0x0051e757:
    // 0051e757  b850baa000             -mov eax, 0xa0ba50
    cpu.eax = 10533456 /*0xa0ba50*/;
    // 0051e75c  e81f000000             -call 0x51e780
    cpu.esp -= 4;
    sub_51e780(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051e761:
    // 0051e761  8b2b                   -mov ebp, dword ptr [ebx]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.ebx);
    // 0051e763  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051e764  2eff15b8445300         -call dword ptr cs:[0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e76b  897308                 -mov dword ptr [ebx + 8], esi
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */) = cpu.esi;
L_0x0051e76e:
    // 0051e76e  ff430c                 -inc dword ptr [ebx + 0xc]
    (app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */))++;
    // 0051e771  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e772  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e773  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e774  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e775  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e776  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_51e71c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x0051e71c;
    // 0051e714  b870baa000             -mov eax, 0xa0ba70
    cpu.eax = 10533488 /*0xa0ba70*/;
    // 0051e719  8d4000                 -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
L_entry_0x0051e71c:
    // 0051e71c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e71d  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e71e  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e71f  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e720  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051e721  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051e723  2eff150c455300         -call dword ptr cs:[0x53450c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457164) /* 0x53450c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e72a  8b5308                 -mov edx, dword ptr [ebx + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
    // 0051e72d  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051e72f  39d0                   +cmp eax, edx
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e731  743b                   -je 0x51e76e
    if (cpu.flags.zf)
    {
        goto L_0x0051e76e;
    }
    // 0051e733  837b0400               +cmp dword ptr [ebx + 4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e737  7528                   -jne 0x51e761
    if (!cpu.flags.zf)
    {
        goto L_0x0051e761;
    }
    // 0051e739  b850baa000             -mov eax, 0xa0ba50
    cpu.eax = 10533456 /*0xa0ba50*/;
    // 0051e73e  e8d9ffffff             -call 0x51e71c
    cpu.esp -= 4;
    sub_51e71c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e743  837b0400               +cmp dword ptr [ebx + 4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e747  750e                   -jne 0x51e757
    if (!cpu.flags.zf)
    {
        goto L_0x0051e757;
    }
    // 0051e749  e8f6fdffff             -call 0x51e544
    cpu.esp -= 4;
    sub_51e544(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e74e  c7430401000000         -mov dword ptr [ebx + 4], 1
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */) = 1 /*0x1*/;
    // 0051e755  8903                   -mov dword ptr [ebx], eax
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.eax;
L_0x0051e757:
    // 0051e757  b850baa000             -mov eax, 0xa0ba50
    cpu.eax = 10533456 /*0xa0ba50*/;
    // 0051e75c  e81f000000             -call 0x51e780
    cpu.esp -= 4;
    sub_51e780(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051e761:
    // 0051e761  8b2b                   -mov ebp, dword ptr [ebx]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.ebx);
    // 0051e763  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051e764  2eff15b8445300         -call dword ptr cs:[0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e76b  897308                 -mov dword ptr [ebx + 8], esi
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */) = cpu.esi;
L_0x0051e76e:
    // 0051e76e  ff430c                 -inc dword ptr [ebx + 0xc]
    (app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */))++;
    // 0051e771  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e772  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e773  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e774  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e775  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e776  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_51e778(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e778  b870baa000             -mov eax, 0xa0ba70
    cpu.eax = 10533488 /*0xa0ba70*/;
    // 0051e77d  8d4000                 -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 0051e780  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e781  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e782  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e783  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e784  8b500c                 -mov edx, dword ptr [eax + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 0051e787  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051e789  7617                   -jbe 0x51e7a2
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051e7a2;
    }
    // 0051e78b  8d5aff                 -lea ebx, [edx - 1]
    cpu.ebx = x86::reg32(cpu.edx + x86::reg32(-1) /* -0x1 */);
    // 0051e78e  89580c                 -mov dword ptr [eax + 0xc], ebx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */) = cpu.ebx;
    // 0051e791  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051e793  750d                   -jne 0x51e7a2
    if (!cpu.flags.zf)
    {
        goto L_0x0051e7a2;
    }
    // 0051e795  8b30                   -mov esi, dword ptr [eax]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax);
    // 0051e797  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e798  895808                 -mov dword ptr [eax + 8], ebx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = cpu.ebx;
    // 0051e79b  2eff1588455300         -call dword ptr cs:[0x534588]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051e7a2:
    // 0051e7a2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e7a3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e7a4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e7a5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e7a6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_51e780(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x0051e780;
    // 0051e778  b870baa000             -mov eax, 0xa0ba70
    cpu.eax = 10533488 /*0xa0ba70*/;
    // 0051e77d  8d4000                 -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
L_entry_0x0051e780:
    // 0051e780  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e781  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e782  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e783  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e784  8b500c                 -mov edx, dword ptr [eax + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 0051e787  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0051e789  7617                   -jbe 0x51e7a2
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051e7a2;
    }
    // 0051e78b  8d5aff                 -lea ebx, [edx - 1]
    cpu.ebx = x86::reg32(cpu.edx + x86::reg32(-1) /* -0x1 */);
    // 0051e78e  89580c                 -mov dword ptr [eax + 0xc], ebx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */) = cpu.ebx;
    // 0051e791  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051e793  750d                   -jne 0x51e7a2
    if (!cpu.flags.zf)
    {
        goto L_0x0051e7a2;
    }
    // 0051e795  8b30                   -mov esi, dword ptr [eax]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax);
    // 0051e797  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e798  895808                 -mov dword ptr [eax + 8], ebx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = cpu.ebx;
    // 0051e79b  2eff1588455300         -call dword ptr cs:[0x534588]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051e7a2:
    // 0051e7a2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e7a3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e7a4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e7a5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e7a6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_51e7a8(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e7a8  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e7a9  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e7aa  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e7ab  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e7ac  2eff1534455300         -call dword ptr cs:[0x534534]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457204) /* 0x534534 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e7b3  8b1560775600           -mov edx, dword ptr [0x567760]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */);
    // 0051e7b9  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e7ba  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051e7bc  2eff1514465300         -call dword ptr cs:[0x534614]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457428) /* 0x534614 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e7c3  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051e7c5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051e7c7  7507                   -jne 0x51e7d0
    if (!cpu.flags.zf)
    {
        goto L_0x0051e7d0;
    }
    // 0051e7c9  e842760000             -call 0x525e10
    cpu.esp -= 4;
    sub_525e10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e7ce  eb0b                   -jmp 0x51e7db
    goto L_0x0051e7db;
L_0x0051e7d0:
    // 0051e7d0  80785300               +cmp byte ptr [eax + 0x53], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(83) /* 0x53 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051e7d4  7407                   -je 0x51e7dd
    if (cpu.flags.zf)
    {
        goto L_0x0051e7dd;
    }
    // 0051e7d6  e871760000             -call 0x525e4c
    cpu.esp -= 4;
    sub_525e4c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051e7db:
    // 0051e7db  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
L_0x0051e7dd:
    // 0051e7dd  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e7de  2eff15f4455300         -call dword ptr cs:[0x5345f4]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457396) /* 0x5345f4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e7e5  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051e7e7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e7e8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e7e9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e7ea  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e7eb  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_51e7ec: native, nfs3hp::watcom::initThreadData */

/* align: skip 0x8b 0xc0 */
void Application::sub_51e828(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e828  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e829  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e82a  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e82b  8b1d60775600           -mov ebx, dword ptr [0x567760]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */);
    // 0051e831  83fbff                 +cmp ebx, -1
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e834  753b                   -jne 0x51e871
    if (!cpu.flags.zf)
    {
        goto L_0x0051e871;
    }
    // 0051e836  2eff150c465300         -call dword ptr cs:[0x53460c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457420) /* 0x53460c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e83d  668b1541785600         -mov dx, word ptr [0x567841]
    cpu.dx = app->getMemory<x86::reg16>(x86::reg32(5666881) /* 0x567841 */);
    // 0051e844  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051e846  6681fa0080             +cmp dx, 0x8000
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(32768 /*0x8000*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0051e84b  7224                   -jb 0x51e871
    if (cpu.flags.cf)
    {
        goto L_0x0051e871;
    }
    // 0051e84d  803d3f78560004         +cmp byte ptr [0x56783f], 4
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5666879) /* 0x56783f */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(4 /*0x4*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051e854  731b                   -jae 0x51e871
    if (!cpu.flags.cf)
    {
        goto L_0x0051e871;
    }
L_0x0051e856:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051e856  83fbff                 +cmp ebx, -1
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e859  7416                   -je 0x51e871
    if (cpu.flags.zf)
    {
        goto L_0x0051e871;
    }
    // 0051e85b  83fb02                 +cmp ebx, 2
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e85e  7711                   -ja 0x51e871
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0051e871;
    }
    // 0051e860  891d60775600           -mov dword ptr [0x567760], ebx
    app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */) = cpu.ebx;
    // 0051e866  2eff150c465300         -call dword ptr cs:[0x53460c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457420) /* 0x53460c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e86d  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051e86f  ebe5                   -jmp 0x51e856
    goto L_0x0051e856;
L_0x0051e871:
    // 0051e871  83fbff                 +cmp ebx, -1
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e874  0f95c0                 -setne al
    cpu.al = !cpu.flags.zf;
    // 0051e877  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0051e87c  891d60775600           -mov dword ptr [0x567760], ebx
    app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */) = cpu.ebx;
    // 0051e882  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e883  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e884  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e885  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_51e888(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e888  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e889  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e88a  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e88b  833d60775600ff         +cmp dword ptr [0x567760], -1
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
    // 0051e892  7506                   -jne 0x51e89a
    if (!cpu.flags.zf)
    {
        goto L_0x0051e89a;
    }
    // 0051e894  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051e896  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e897  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e898  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e899  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051e89a:
    // 0051e89a  e84dffffff             -call 0x51e7ec
    cpu.esp -= 4;
    nfs3hp::watcom::initThreadData(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e89f  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051e8a1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051e8a3  7432                   -je 0x51e8d7
    if (cpu.flags.zf)
    {
        goto L_0x0051e8d7;
    }
    // 0051e8a5  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 0051e8a7  8b80da000000           -mov eax, dword ptr [eax + 0xda]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(218) /* 0xda */);
    // 0051e8ad  e86e760000             -call 0x525f20
    cpu.esp -= 4;
    sub_525f20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e8b2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051e8b4  750d                   -jne 0x51e8c3
    if (!cpu.flags.zf)
    {
        goto L_0x0051e8c3;
    }
    // 0051e8b6  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051e8b8  e83391fdff             -call 0x4f79f0
    cpu.esp -= 4;
    nfs3hp::watcom::nfree(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e8bd  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051e8bf  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e8c0  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e8c1  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e8c2  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051e8c3:
    // 0051e8c3  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e8c4  8b1d60775600           -mov ebx, dword ptr [0x567760]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */);
    // 0051e8ca  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e8cb  2eff1518465300         -call dword ptr cs:[0x534618]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457432) /* 0x534618 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e8d2  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x0051e8d7:
    // 0051e8d7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e8d8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e8d9  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e8da  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_51e8dc(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e8dc  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e8dd  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e8de  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e8df  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e8e0  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051e8e1  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051e8e3  8b1560775600           -mov edx, dword ptr [0x567760]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */);
    // 0051e8e9  83faff                 +cmp edx, -1
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e8ec  743d                   -je 0x51e92b
    if (cpu.flags.zf)
    {
        goto L_0x0051e92b;
    }
    // 0051e8ee  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e8ef  2eff1514465300         -call dword ptr cs:[0x534614]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457428) /* 0x534614 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e8f6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051e8f8  7431                   -je 0x51e92b
    if (cpu.flags.zf)
    {
        goto L_0x0051e92b;
    }
    // 0051e8fa  8bb0de000000           -mov esi, dword ptr [eax + 0xde]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(222) /* 0xde */);
    // 0051e900  8b80da000000           -mov eax, dword ptr [eax + 0xda]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(218) /* 0xda */);
    // 0051e906  e879760000             -call 0x525f84
    cpu.esp -= 4;
    sub_525f84(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e90b  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051e90d  8b3d60775600           -mov edi, dword ptr [0x567760]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */);
    // 0051e913  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051e914  2eff1518465300         -call dword ptr cs:[0x534618]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457432) /* 0x534618 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e91b  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 0051e91d  740c                   -je 0x51e92b
    if (cpu.flags.zf)
    {
        goto L_0x0051e92b;
    }
    // 0051e91f  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051e921  7408                   -je 0x51e92b
    if (cpu.flags.zf)
    {
        goto L_0x0051e92b;
    }
    // 0051e923  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e924  2eff1588445300         -call dword ptr cs:[0x534488]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457032) /* 0x534488 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051e92b:
    // 0051e92b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e92c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e92d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e92e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e92f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e930  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_51e934(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e934  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051e939  e89effffff             -call 0x51e8dc
    cpu.esp -= 4;
    sub_51e8dc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e93e  8bc0                   -mov eax, eax
    cpu.eax = cpu.eax;
    // 0051e940  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e941  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e942  8b1560775600           -mov edx, dword ptr [0x567760]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */);
    // 0051e948  83faff                 +cmp edx, -1
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e94b  7412                   -je 0x51e95f
    if (cpu.flags.zf)
    {
        goto L_0x0051e95f;
    }
    // 0051e94d  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e94e  2eff1510465300         -call dword ptr cs:[0x534610]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457424) /* 0x534610 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e955  c70560775600ffffffff   -mov dword ptr [0x567760], 0xffffffff
    app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */) = 4294967295 /*0xffffffff*/;
L_0x0051e95f:
    // 0051e95f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e960  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e961  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_51e940(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    goto L_entry_0x0051e940;
    // 0051e934  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051e939  e89effffff             -call 0x51e8dc
    cpu.esp -= 4;
    sub_51e8dc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e93e  8bc0                   -mov eax, eax
    cpu.eax = cpu.eax;
L_entry_0x0051e940:
    // 0051e940  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e941  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e942  8b1560775600           -mov edx, dword ptr [0x567760]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */);
    // 0051e948  83faff                 +cmp edx, -1
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051e94b  7412                   -je 0x51e95f
    if (cpu.flags.zf)
    {
        goto L_0x0051e95f;
    }
    // 0051e94d  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e94e  2eff1510465300         -call dword ptr cs:[0x534610]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457424) /* 0x534610 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051e955  c70560775600ffffffff   -mov dword ptr [0x567760], 0xffffffff
    app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */) = 4294967295 /*0xffffffff*/;
L_0x0051e95f:
    // 0051e95f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e960  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051e961  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_51e964(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051e964  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051e965  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051e966  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051e967  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051e968  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051e969  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051e96a  ba98e65100             -mov edx, 0x51e698
    cpu.edx = 5367448 /*0x51e698*/;
    // 0051e96f  bba8e65100             -mov ebx, 0x51e6a8
    cpu.ebx = 5367464 /*0x51e6a8*/;
    // 0051e974  b9b8e65100             -mov ecx, 0x51e6b8
    cpu.ecx = 5367480 /*0x51e6b8*/;
    // 0051e979  bec0e65100             -mov esi, 0x51e6c0
    cpu.esi = 5367488 /*0x51e6c0*/;
    // 0051e97e  bf80e65100             -mov edi, 0x51e680
    cpu.edi = 5367424 /*0x51e680*/;
    // 0051e983  bd8ce65100             -mov ebp, 0x51e68c
    cpu.ebp = 5367436 /*0x51e68c*/;
    // 0051e988  b804e75100             -mov eax, 0x51e704
    cpu.eax = 5367556 /*0x51e704*/;
    // 0051e98d  891568775600           -mov dword ptr [0x567768], edx
    app->getMemory<x86::reg32>(x86::reg32(5666664) /* 0x567768 */) = cpu.edx;
    // 0051e993  891d6c775600           -mov dword ptr [0x56776c], ebx
    app->getMemory<x86::reg32>(x86::reg32(5666668) /* 0x56776c */) = cpu.ebx;
    // 0051e999  890d70775600           -mov dword ptr [0x567770], ecx
    app->getMemory<x86::reg32>(x86::reg32(5666672) /* 0x567770 */) = cpu.ecx;
    // 0051e99f  893574775600           -mov dword ptr [0x567774], esi
    app->getMemory<x86::reg32>(x86::reg32(5666676) /* 0x567774 */) = cpu.esi;
    // 0051e9a5  893d78775600           -mov dword ptr [0x567778], edi
    app->getMemory<x86::reg32>(x86::reg32(5666680) /* 0x567778 */) = cpu.edi;
    // 0051e9ab  892d7c775600           -mov dword ptr [0x56777c], ebp
    app->getMemory<x86::reg32>(x86::reg32(5666684) /* 0x56777c */) = cpu.ebp;
    // 0051e9b1  a390775600             -mov dword ptr [0x567790], eax
    app->getMemory<x86::reg32>(x86::reg32(5666704) /* 0x567790 */) = cpu.eax;
    // 0051e9b6  ba0ce75100             -mov edx, 0x51e70c
    cpu.edx = 5367564 /*0x51e70c*/;
    // 0051e9bb  bb1ce75100             -mov ebx, 0x51e71c
    cpu.ebx = 5367580 /*0x51e71c*/;
    // 0051e9c0  b980e75100             -mov ecx, 0x51e780
    cpu.ecx = 5367680 /*0x51e780*/;
    // 0051e9c5  be70e65100             -mov esi, 0x51e670
    cpu.esi = 5367408 /*0x51e670*/;
    // 0051e9ca  bfdce65100             -mov edi, 0x51e6dc
    cpu.edi = 5367516 /*0x51e6dc*/;
    // 0051e9cf  bdf0e65100             -mov ebp, 0x51e6f0
    cpu.ebp = 5367536 /*0x51e6f0*/;
    // 0051e9d4  b8e4e65100             -mov eax, 0x51e6e4
    cpu.eax = 5367524 /*0x51e6e4*/;
    // 0051e9d9  891594775600           -mov dword ptr [0x567794], edx
    app->getMemory<x86::reg32>(x86::reg32(5666708) /* 0x567794 */) = cpu.edx;
    // 0051e9df  891dc8ac5600           -mov dword ptr [0x56acc8], ebx
    app->getMemory<x86::reg32>(x86::reg32(5680328) /* 0x56acc8 */) = cpu.ebx;
    // 0051e9e5  890dccac5600           -mov dword ptr [0x56accc], ecx
    app->getMemory<x86::reg32>(x86::reg32(5680332) /* 0x56accc */) = cpu.ecx;
    // 0051e9eb  8935d0ac5600           -mov dword ptr [0x56acd0], esi
    app->getMemory<x86::reg32>(x86::reg32(5680336) /* 0x56acd0 */) = cpu.esi;
    // 0051e9f1  893d80775600           -mov dword ptr [0x567780], edi
    app->getMemory<x86::reg32>(x86::reg32(5666688) /* 0x567780 */) = cpu.edi;
    // 0051e9f7  892d84775600           -mov dword ptr [0x567784], ebp
    app->getMemory<x86::reg32>(x86::reg32(5666692) /* 0x567784 */) = cpu.ebp;
    // 0051e9fd  a388775600             -mov dword ptr [0x567788], eax
    app->getMemory<x86::reg32>(x86::reg32(5666696) /* 0x567788 */) = cpu.eax;
    // 0051ea02  baf8e65100             -mov edx, 0x51e6f8
    cpu.edx = 5367544 /*0x51e6f8*/;
    // 0051ea07  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0051ea0c  b914e75100             -mov ecx, 0x51e714
    cpu.ecx = 5367572 /*0x51e714*/;
    // 0051ea11  be78e75100             -mov esi, 0x51e778
    cpu.esi = 5367672 /*0x51e778*/;
    // 0051ea16  bf34e95100             -mov edi, 0x51e934
    cpu.edi = 5368116 /*0x51e934*/;
    // 0051ea1b  89158c775600           -mov dword ptr [0x56778c], edx
    app->getMemory<x86::reg32>(x86::reg32(5666700) /* 0x56778c */) = cpu.edx;
    // 0051ea21  e81efbffff             -call 0x51e544
    cpu.esp -= 4;
    sub_51e544(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ea26  8b152c649f00           -mov edx, dword ptr [0x9f642c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10445868) /* 0x9f642c */);
    // 0051ea2c  a350baa000             -mov dword ptr [0xa0ba50], eax
    app->getMemory<x86::reg32>(x86::reg32(10533456) /* 0xa0ba50 */) = cpu.eax;
    // 0051ea31  891d54baa000           -mov dword ptr [0xa0ba54], ebx
    app->getMemory<x86::reg32>(x86::reg32(10533460) /* 0xa0ba54 */) = cpu.ebx;
    // 0051ea37  890d98775600           -mov dword ptr [0x567798], ecx
    app->getMemory<x86::reg32>(x86::reg32(5666712) /* 0x567798 */) = cpu.ecx;
    // 0051ea3d  89359c775600           -mov dword ptr [0x56779c], esi
    app->getMemory<x86::reg32>(x86::reg32(5666716) /* 0x56779c */) = cpu.esi;
    // 0051ea43  8b82da000000           -mov eax, dword ptr [edx + 0xda]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(218) /* 0xda */);
    // 0051ea49  893da0775600           -mov dword ptr [0x5677a0], edi
    app->getMemory<x86::reg32>(x86::reg32(5666720) /* 0x5677a0 */) = cpu.edi;
    // 0051ea4f  e8cc740000             -call 0x525f20
    cpu.esp -= 4;
    sub_525f20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ea54  8b2d2c649f00           -mov ebp, dword ptr [0x9f642c]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(10445868) /* 0x9f642c */);
    // 0051ea5a  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051ea5b  a160775600             -mov eax, dword ptr [0x567760]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5666656) /* 0x567760 */);
    // 0051ea60  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ea61  2eff1518465300         -call dword ptr cs:[0x534618]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457432) /* 0x534618 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ea68  c70564775600a8e75100   -mov dword ptr [0x567764], 0x51e7a8
    app->getMemory<x86::reg32>(x86::reg32(5666660) /* 0x567764 */) = 5367720 /*0x51e7a8*/;
    // 0051ea72  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ea73  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ea74  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ea75  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ea76  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ea77  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ea78  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_51ea7c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051ea7c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051ea7d  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051ea7e  b820b3a000             -mov eax, 0xa0b320
    cpu.eax = 10531616 /*0xa0b320*/;
    // 0051ea83  ba40b3a000             -mov edx, 0xa0b340
    cpu.edx = 10531648 /*0xa0b340*/;
    // 0051ea88  ff15d0ac5600           -call dword ptr [0x56acd0]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5680336) /* 0x56acd0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ea8e  8d9a00010000           -lea ebx, [edx + 0x100]
    cpu.ebx = x86::reg32(cpu.edx + x86::reg32(256) /* 0x100 */);
L_0x0051ea94:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051ea94  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0051ea96  83c210                 -add edx, 0x10
    (cpu.edx) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0051ea99  ff15d0ac5600           -call dword ptr [0x56acd0]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5680336) /* 0x56acd0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ea9f  39da                   +cmp edx, ebx
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
    // 0051eaa1  75f1                   -jne 0x51ea94
    if (!cpu.flags.zf)
    {
        goto L_0x0051ea94;
    }
    // 0051eaa3  b870baa000             -mov eax, 0xa0ba70
    cpu.eax = 10533488 /*0xa0ba70*/;
    // 0051eaa8  ff15d0ac5600           -call dword ptr [0x56acd0]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5680336) /* 0x56acd0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051eaae  e865fbffff             -call 0x51e618
    cpu.esp -= 4;
    sub_51e618(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051eab3  e8a8750000             -call 0x526060
    cpu.esp -= 4;
    sub_526060(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051eab8  e803570000             -call 0x5241c0
    cpu.esp -= 4;
    nfs3hp::watcom::nheapshrink(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051eabd  b840b4a000             -mov eax, 0xa0b440
    cpu.eax = 10531904 /*0xa0b440*/;
    // 0051eac2  ff15d0ac5600           -call dword ptr [0x56acd0]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5680336) /* 0x56acd0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051eac8  b830b3a000             -mov eax, 0xa0b330
    cpu.eax = 10531632 /*0xa0b330*/;
    // 0051eacd  ff15d0ac5600           -call dword ptr [0x56acd0]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5680336) /* 0x56acd0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ead3  b860baa000             -mov eax, 0xa0ba60
    cpu.eax = 10533472 /*0xa0ba60*/;
    // 0051ead8  ff15d0ac5600           -call dword ptr [0x56acd0]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5680336) /* 0x56acd0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051eade  b850baa000             -mov eax, 0xa0ba50
    cpu.eax = 10533456 /*0xa0ba50*/;
    // 0051eae3  ff15d0ac5600           -call dword ptr [0x56acd0]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5680336) /* 0x56acd0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051eae9  e8fafaffff             -call 0x51e5e8
    cpu.esp -= 4;
    sub_51e5e8(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051eaee  e84dfeffff             -call 0x51e940
    cpu.esp -= 4;
    sub_51e940(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051eaf3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051eaf4  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051eaf5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_51eb00: native, nfs3hp::watcom::moduleWideName */

/* sub_51eba0: native, nfs3hp::watcom::copyCommandLine */

/* sub_51ebf0: native, nfs3hp::watcom::copyWideCommandLine */

/* sub_51ec20: native, nfs3hp::watcom::stackLimits */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_51ec90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051ec90  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051ec91  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051ec92  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051ec93  68d4135500             -push 0x5513d4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5575636 /*0x5513d4*/;
    cpu.esp -= 4;
    // 0051ec98  2eff158c455300         -call dword ptr cs:[0x53458c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457292) /* 0x53458c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ec9f  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051eca1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051eca3  7417                   -je 0x51ecbc
    if (cpu.flags.zf)
    {
        goto L_0x0051ecbc;
    }
    // 0051eca5  68e0135500             -push 0x5513e0
    app->getMemory<x86::reg32>(cpu.esp-4) = 5575648 /*0x5513e0*/;
    cpu.esp -= 4;
    // 0051ecaa  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051ecab  2eff1558455300         -call dword ptr cs:[0x534558]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457240) /* 0x534558 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ecb2  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051ecb4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051ecb6  7404                   -je 0x51ecbc
    if (cpu.flags.zf)
    {
        goto L_0x0051ecbc;
    }
    // 0051ecb8  ffd2                   -call edx
    cpu.ip = cpu.edx;
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ecba  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
L_0x0051ecbc:
    // 0051ecbc  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051ecbe  0f95c0                 -setne al
    cpu.al = !cpu.flags.zf;
    // 0051ecc1  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0051ecc6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ecc7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ecc8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ecc9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_51eccc(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051eccc  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051eccd  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051ecce  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051eccf  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 0051ecd1  8a08                   -mov cl, byte ptr [eax]
    cpu.cl = app->getMemory<x86::reg8>(cpu.eax);
    // 0051ecd3  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0051ecd5  84c9                   +test cl, cl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & cpu.cl));
    // 0051ecd7  7408                   -je 0x51ece1
    if (cpu.flags.zf)
    {
        goto L_0x0051ece1;
    }
L_0x0051ecd9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051ecd9  8a6801                 -mov ch, byte ptr [eax + 1]
    cpu.ch = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0051ecdc  40                     -inc eax
    (cpu.eax)++;
    // 0051ecdd  84ed                   +test ch, ch
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ch & cpu.ch));
    // 0051ecdf  75f8                   -jne 0x51ecd9
    if (!cpu.flags.zf)
    {
        goto L_0x0051ecd9;
    }
L_0x0051ece1:
    // 0051ece1  8d7009                 -lea esi, [eax + 9]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(9) /* 0x9 */);
L_0x0051ece4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051ece4  8a0a                   -mov cl, byte ptr [edx]
    cpu.cl = app->getMemory<x86::reg8>(cpu.edx);
    // 0051ece6  8808                   -mov byte ptr [eax], cl
    app->getMemory<x86::reg8>(cpu.eax) = cpu.cl;
    // 0051ece8  84c9                   +test cl, cl
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.cl & cpu.cl));
    // 0051ecea  7412                   -je 0x51ecfe
    if (cpu.flags.zf)
    {
        goto L_0x0051ecfe;
    }
    // 0051ecec  80f930                 +cmp cl, 0x30
    {
        x86::reg8 tmp1 = cpu.cl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(48 /*0x30*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051ecef  7508                   -jne 0x51ecf9
    if (!cpu.flags.zf)
    {
        goto L_0x0051ecf9;
    }
    // 0051ecf1  807a0178               +cmp byte ptr [edx + 1], 0x78
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(1) /* 0x1 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(120 /*0x78*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051ecf5  7502                   -jne 0x51ecf9
    if (!cpu.flags.zf)
    {
        goto L_0x0051ecf9;
    }
    // 0051ecf7  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
L_0x0051ecf9:
    // 0051ecf9  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0051ecfa  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0051ecfb  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0051ecfc  ebe6                   -jmp 0x51ece4
    goto L_0x0051ece4;
L_0x0051ecfe:
    // 0051ecfe  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051ed00  741c                   -je 0x51ed1e
    if (cpu.flags.zf)
    {
        goto L_0x0051ed1e;
    }
    // 0051ed02  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0051ed04  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 0051ed06  7416                   -je 0x51ed1e
    if (cpu.flags.zf)
    {
        goto L_0x0051ed1e;
    }
L_0x0051ed08:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051ed08  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051ed0a  83e20f                 -and edx, 0xf
    cpu.edx &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 0051ed0d  4b                     -dec ebx
    (cpu.ebx)--;
    // 0051ed0e  8a92dcac5600           -mov dl, byte ptr [edx + 0x56acdc]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(5680348) /* 0x56acdc */);
    // 0051ed14  c1e804                 -shr eax, 4
    cpu.eax >>= 4 /*0x4*/ % 32;
    // 0051ed17  885301                 -mov byte ptr [ebx + 1], dl
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(1) /* 0x1 */) = cpu.dl;
    // 0051ed1a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051ed1c  75ea                   -jne 0x51ed08
    if (!cpu.flags.zf)
    {
        goto L_0x0051ed08;
    }
L_0x0051ed1e:
    // 0051ed1e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ed1f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ed20  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051ed21  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_51ed24(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051ed24  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051ed25  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051ed26  81ec04010000           -sub esp, 0x104
    (cpu.esp) -= x86::reg32(x86::sreg32(260 /*0x104*/));
    // 0051ed2c  8b9c2410010000         -mov ebx, dword ptr [esp + 0x110]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(272) /* 0x110 */);
    // 0051ed33  8b0b                   -mov ecx, dword ptr [ebx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebx);
    // 0051ed35  8b5b04                 -mov ebx, dword ptr [ebx + 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(4) /* 0x4 */);
    // 0051ed38  e853ffffff             -call 0x51ec90
    cpu.esp -= 4;
    sub_51ec90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ed3d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051ed3f  750a                   -jne 0x51ed4b
    if (!cpu.flags.zf)
    {
        goto L_0x0051ed4b;
    }
    // 0051ed41  e8ee69ffff             -call 0x515734
    cpu.esp -= 4;
    nfs3hp::watcom::NTConsoleOutput(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ed46  83f8ff                 +cmp eax, -1
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
    // 0051ed49  7507                   -jne 0x51ed52
    if (!cpu.flags.zf)
    {
        goto L_0x0051ed52;
    }
L_0x0051ed4b:
    // 0051ed4b  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051ed4d  e98c010000             -jmp 0x51eede
    goto L_0x0051eede;
L_0x0051ed52:
    // 0051ed52  30e4                   -xor ah, ah
    cpu.ah ^= x86::reg8(x86::sreg8(cpu.ah));
    // 0051ed54  882424                 -mov byte ptr [esp], ah
    app->getMemory<x86::reg8>(cpu.esp) = cpu.ah;
    // 0051ed57  8b01                   -mov eax, dword ptr [ecx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx);
    // 0051ed59  3d900000c0             +cmp eax, 0xc0000090
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225616 /*0xc0000090*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ed5e  724d                   -jb 0x51edad
    if (cpu.flags.cf)
    {
        goto L_0x0051edad;
    }
    // 0051ed60  0f86c2000000           -jbe 0x51ee28
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051ee28;
    }
    // 0051ed66  3d930000c0             +cmp eax, 0xc0000093
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225619 /*0xc0000093*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ed6b  7233                   -jb 0x51eda0
    if (cpu.flags.cf)
    {
        goto L_0x0051eda0;
    }
    // 0051ed6d  0f86ab000000           -jbe 0x51ee1e
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051ee1e;
    }
    // 0051ed73  3d960000c0             +cmp eax, 0xc0000096
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225622 /*0xc0000096*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ed78  7216                   -jb 0x51ed90
    if (cpu.flags.cf)
    {
        goto L_0x0051ed90;
    }
    // 0051ed7a  0f86ec000000           -jbe 0x51ee6c
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051ee6c;
    }
    // 0051ed80  3dfd0000c0             +cmp eax, 0xc00000fd
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225725 /*0xc00000fd*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ed85  0f84f6000000           -je 0x51ee81
    if (cpu.flags.zf)
    {
        goto L_0x0051ee81;
    }
    // 0051ed8b  e9f8000000             -jmp 0x51ee88
    goto L_0x0051ee88;
L_0x0051ed90:
    // 0051ed90  3d940000c0             +cmp eax, 0xc0000094
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225620 /*0xc0000094*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ed95  0f84df000000           -je 0x51ee7a
    if (cpu.flags.zf)
    {
        goto L_0x0051ee7a;
    }
    // 0051ed9b  e9e8000000             -jmp 0x51ee88
    goto L_0x0051ee88;
L_0x0051eda0:
    // 0051eda0  3d910000c0             +cmp eax, 0xc0000091
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225617 /*0xc0000091*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051eda5  0f8669000000           -jbe 0x51ee14
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051ee14;
    }
    // 0051edab  eb2f                   -jmp 0x51eddc
    goto L_0x0051eddc;
L_0x0051edad:
    // 0051edad  3d8d0000c0             +cmp eax, 0xc000008d
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225613 /*0xc000008d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051edb2  720b                   -jb 0x51edbf
    if (cpu.flags.cf)
    {
        goto L_0x0051edbf;
    }
    // 0051edb4  7640                   -jbe 0x51edf6
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051edf6;
    }
    // 0051edb6  3d8e0000c0             +cmp eax, 0xc000008e
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225614 /*0xc000008e*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051edbb  7643                   -jbe 0x51ee00
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051ee00;
    }
    // 0051edbd  eb4b                   -jmp 0x51ee0a
    goto L_0x0051ee0a;
L_0x0051edbf:
    // 0051edbf  3d050000c0             +cmp eax, 0xc0000005
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225477 /*0xc0000005*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051edc4  0f82be000000           -jb 0x51ee88
    if (cpu.flags.cf)
    {
        goto L_0x0051ee88;
    }
    // 0051edca  7666                   -jbe 0x51ee32
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x0051ee32;
    }
    // 0051edcc  3d1d0000c0             +cmp eax, 0xc000001d
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3221225501 /*0xc000001d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051edd1  0f849c000000           -je 0x51ee73
    if (cpu.flags.zf)
    {
        goto L_0x0051ee73;
    }
    // 0051edd7  e9ac000000             -jmp 0x51ee88
    goto L_0x0051ee88;
L_0x0051eddc:
    // 0051eddc  f6432102               +test byte ptr [ebx + 0x21], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(33) /* 0x21 */) & 2 /*0x2*/));
    // 0051ede0  740a                   -je 0x51edec
    if (cpu.flags.zf)
    {
        goto L_0x0051edec;
    }
    // 0051ede2  baf0135500             -mov edx, 0x5513f0
    cpu.edx = 5575664 /*0x5513f0*/;
    // 0051ede7  e9af000000             -jmp 0x51ee9b
    goto L_0x0051ee9b;
L_0x0051edec:
    // 0051edec  ba44145500             -mov edx, 0x551444
    cpu.edx = 5575748 /*0x551444*/;
    // 0051edf1  e9a5000000             -jmp 0x51ee9b
    goto L_0x0051ee9b;
L_0x0051edf6:
    // 0051edf6  ba98145500             -mov edx, 0x551498
    cpu.edx = 5575832 /*0x551498*/;
    // 0051edfb  e99b000000             -jmp 0x51ee9b
    goto L_0x0051ee9b;
L_0x0051ee00:
    // 0051ee00  baec145500             -mov edx, 0x5514ec
    cpu.edx = 5575916 /*0x5514ec*/;
    // 0051ee05  e991000000             -jmp 0x51ee9b
    goto L_0x0051ee9b;
L_0x0051ee0a:
    // 0051ee0a  ba40155500             -mov edx, 0x551540
    cpu.edx = 5576000 /*0x551540*/;
    // 0051ee0f  e987000000             -jmp 0x51ee9b
    goto L_0x0051ee9b;
L_0x0051ee14:
    // 0051ee14  ba94155500             -mov edx, 0x551594
    cpu.edx = 5576084 /*0x551594*/;
    // 0051ee19  e97d000000             -jmp 0x51ee9b
    goto L_0x0051ee9b;
L_0x0051ee1e:
    // 0051ee1e  bae0155500             -mov edx, 0x5515e0
    cpu.edx = 5576160 /*0x5515e0*/;
    // 0051ee23  e973000000             -jmp 0x51ee9b
    goto L_0x0051ee9b;
L_0x0051ee28:
    // 0051ee28  ba30165500             -mov edx, 0x551630
    cpu.edx = 5576240 /*0x551630*/;
    // 0051ee2d  e969000000             -jmp 0x51ee9b
    goto L_0x0051ee9b;
L_0x0051ee32:
    // 0051ee32  ba88165500             -mov edx, 0x551688
    cpu.edx = 5576328 /*0x551688*/;
    // 0051ee37  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0051ee39  8b590c                 -mov ebx, dword ptr [ecx + 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 0051ee3c  e88bfeffff             -call 0x51eccc
    cpu.esp -= 4;
    sub_51eccc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ee41  babc165500             -mov edx, 0x5516bc
    cpu.edx = 5576380 /*0x5516bc*/;
    // 0051ee46  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0051ee48  8b5918                 -mov ebx, dword ptr [ecx + 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */);
    // 0051ee4b  e87cfeffff             -call 0x51eccc
    cpu.esp -= 4;
    sub_51eccc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ee50  83791400               +cmp dword ptr [ecx + 0x14], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(20) /* 0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ee54  750b                   -jne 0x51ee61
    if (!cpu.flags.zf)
    {
        goto L_0x0051ee61;
    }
    // 0051ee56  bae4165500             -mov edx, 0x5516e4
    cpu.edx = 5576420 /*0x5516e4*/;
    // 0051ee5b  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0051ee5d  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0051ee5f  eb3f                   -jmp 0x51eea0
    goto L_0x0051eea0;
L_0x0051ee61:
    // 0051ee61  baec165500             -mov edx, 0x5516ec
    cpu.edx = 5576428 /*0x5516ec*/;
    // 0051ee66  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0051ee68  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0051ee6a  eb34                   -jmp 0x51eea0
    goto L_0x0051eea0;
L_0x0051ee6c:
    // 0051ee6c  baf8165500             -mov edx, 0x5516f8
    cpu.edx = 5576440 /*0x5516f8*/;
    // 0051ee71  eb28                   -jmp 0x51ee9b
    goto L_0x0051ee9b;
L_0x0051ee73:
    // 0051ee73  ba38175500             -mov edx, 0x551738
    cpu.edx = 5576504 /*0x551738*/;
    // 0051ee78  eb21                   -jmp 0x51ee9b
    goto L_0x0051ee9b;
L_0x0051ee7a:
    // 0051ee7a  ba74175500             -mov edx, 0x551774
    cpu.edx = 5576564 /*0x551774*/;
    // 0051ee7f  eb1a                   -jmp 0x51ee9b
    goto L_0x0051ee9b;
L_0x0051ee81:
    // 0051ee81  bab8175500             -mov edx, 0x5517b8
    cpu.edx = 5576632 /*0x5517b8*/;
    // 0051ee86  eb13                   -jmp 0x51ee9b
    goto L_0x0051ee9b;
L_0x0051ee88:
    // 0051ee88  baf4175500             -mov edx, 0x5517f4
    cpu.edx = 5576692 /*0x5517f4*/;
    // 0051ee8d  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0051ee8f  8b19                   -mov ebx, dword ptr [ecx]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx);
    // 0051ee91  e836feffff             -call 0x51eccc
    cpu.esp -= 4;
    sub_51eccc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051ee96  ba28185500             -mov edx, 0x551828
    cpu.edx = 5576744 /*0x551828*/;
L_0x0051ee9b:
    // 0051ee9b  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0051ee9d  8b590c                 -mov ebx, dword ptr [ecx + 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
L_0x0051eea0:
    // 0051eea0  e827feffff             -call 0x51eccc
    cpu.esp -= 4;
    sub_51eccc(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051eea5  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0051eea7  8d842404010000         -lea eax, [esp + 0x104]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(260) /* 0x104 */);
    // 0051eeae  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051eeaf  8d7c2408               -lea edi, [esp + 8]
    cpu.edi = x86::reg32(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051eeb3  06                     -push es
    app->getMemory<x86::reg16>(cpu.esp-4) = cpu.es;
    cpu.esp -= 4;
    // 0051eeb4  8cd8                   -mov eax, ds
    cpu.eax = cpu.ds;
    // 0051eeb6  8ec0                   -mov es, eax
    cpu.es = cpu.eax;
    // 0051eeb8  29c9                   -sub ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051eeba  49                     -dec ecx
    (cpu.ecx)--;
    // 0051eebb  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051eebd  f2ae                   -repne scasb al, byte ptr es:[edi]
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
    // 0051eebf  f7d1                   -not ecx
    cpu.ecx = ~cpu.ecx;
    // 0051eec1  49                     -dec ecx
    (cpu.ecx)--;
    // 0051eec2  07                     -pop es
    cpu.es = app->getMemory<x86::reg16>(cpu.esp);
    cpu.esp += 4;
    // 0051eec3  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051eec4  8d44240c               -lea eax, [esp + 0xc]
    cpu.eax = x86::reg32(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0051eec8  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051eec9  a1bcac5600             -mov eax, dword ptr [0x56acbc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5680316) /* 0x56acbc */);
    // 0051eece  8b5808                 -mov ebx, dword ptr [eax + 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 0051eed1  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051eed2  2eff1540465300         -call dword ptr cs:[0x534640]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457472) /* 0x534640 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051eed9  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x0051eede:
    // 0051eede  81c404010000           -add esp, 0x104
    (cpu.esp) += x86::reg32(x86::sreg32(260 /*0x104*/));
    // 0051eee4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051eee5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051eee6  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_51ef08(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0051ef08  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051ef09  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051ef0a  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051ef0b  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0051ef0e  8b742418               -mov esi, dword ptr [esp + 0x18]
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(24) /* 0x18 */);
    // 0051ef12  8b7c2420               -mov edi, dword ptr [esp + 0x20]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(32) /* 0x20 */);
    // 0051ef16  f6460406               +test byte ptr [esi + 4], 6
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.esi + x86::reg32(4) /* 0x4 */) & 6 /*0x6*/));
    // 0051ef1a  0f85a1010000           -jne 0x51f0c1
    if (!cpu.flags.zf)
    {
        goto L_0x0051f0c1;
    }
    // 0051ef20  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 0051ef22  0573ffff3f             -add eax, 0x3fffff73
    (cpu.eax) += x86::reg32(x86::sreg32(1073741683 /*0x3fffff73*/));
    // 0051ef27  83f806                 +cmp eax, 6
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(6 /*0x6*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051ef2a  0f871f010000           -ja 0x51f04f
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0051f04f;
    }
    // 0051ef30  2eff2485ecee5100       -jmp dword ptr cs:[eax*4 + 0x51eeec]
    cpu.ip = app->getMemory<x86::reg32>(5369580 + cpu.eax * 4); goto dynamic_jump;
  case 0x0051ef38:
    // 0051ef38  f6472102               +test byte ptr [edi + 0x21], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.edi + x86::reg32(33) /* 0x21 */) & 2 /*0x2*/));
    // 0051ef3c  740a                   -je 0x51ef48
    if (cpu.flags.zf)
    {
        goto L_0x0051ef48;
    }
    // 0051ef3e  bb8a000000             -mov ebx, 0x8a
    cpu.ebx = 138 /*0x8a*/;
    // 0051ef43  e9ca000000             -jmp 0x51f012
    goto L_0x0051f012;
L_0x0051ef48:
    // 0051ef48  bb8b000000             -mov ebx, 0x8b
    cpu.ebx = 139 /*0x8b*/;
    // 0051ef4d  e9c0000000             -jmp 0x51f012
    goto L_0x0051f012;
  case 0x0051ef52:
    // 0051ef52  bb82000000             -mov ebx, 0x82
    cpu.ebx = 130 /*0x82*/;
    // 0051ef57  e9b6000000             -jmp 0x51f012
    goto L_0x0051f012;
  case 0x0051ef5c:
    // 0051ef5c  bb86000000             -mov ebx, 0x86
    cpu.ebx = 134 /*0x86*/;
    // 0051ef61  e9ac000000             -jmp 0x51f012
    goto L_0x0051f012;
  case 0x0051ef66:
    // 0051ef66  bb84000000             -mov ebx, 0x84
    cpu.ebx = 132 /*0x84*/;
    // 0051ef6b  e9a2000000             -jmp 0x51f012
    goto L_0x0051f012;
  case 0x0051ef70:
    // 0051ef70  bb85000000             -mov ebx, 0x85
    cpu.ebx = 133 /*0x85*/;
    // 0051ef75  e998000000             -jmp 0x51f012
    goto L_0x0051f012;
  case 0x0051ef7a:
    // 0051ef7a  8b4728                 -mov eax, dword ptr [edi + 0x28]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(40) /* 0x28 */);
    // 0051ef7d  668b10                 -mov dx, word ptr [eax]
    cpu.dx = app->getMemory<x86::reg16>(cpu.eax);
    // 0051ef80  bb81000000             -mov ebx, 0x81
    cpu.ebx = 129 /*0x81*/;
    // 0051ef85  6681fad9fa             +cmp dx, 0xfad9
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(64217 /*0xfad9*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0051ef8a  750a                   -jne 0x51ef96
    if (!cpu.flags.zf)
    {
        goto L_0x0051ef96;
    }
    // 0051ef8c  bb88000000             -mov ebx, 0x88
    cpu.ebx = 136 /*0x88*/;
    // 0051ef91  e97c000000             -jmp 0x51f012
    goto L_0x0051f012;
L_0x0051ef96:
    // 0051ef96  6681fad9f1             +cmp dx, 0xf1d9
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(61913 /*0xf1d9*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0051ef9b  750a                   -jne 0x51efa7
    if (!cpu.flags.zf)
    {
        goto L_0x0051efa7;
    }
    // 0051ef9d  bb8e000000             -mov ebx, 0x8e
    cpu.ebx = 142 /*0x8e*/;
    // 0051efa2  e96b000000             -jmp 0x51f012
    goto L_0x0051f012;
L_0x0051efa7:
    // 0051efa7  750a                   -jne 0x51efb3
    if (!cpu.flags.zf)
    {
        goto L_0x0051efb3;
    }
    // 0051efa9  bb8f000000             -mov ebx, 0x8f
    cpu.ebx = 143 /*0x8f*/;
    // 0051efae  e95f000000             -jmp 0x51f012
    goto L_0x0051f012;
L_0x0051efb3:
    // 0051efb3  8a30                   -mov dh, byte ptr [eax]
    cpu.dh = app->getMemory<x86::reg8>(cpu.eax);
    // 0051efb5  80fedb                 +cmp dh, 0xdb
    {
        x86::reg8 tmp1 = cpu.dh;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(219 /*0xdb*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051efb8  7405                   -je 0x51efbf
    if (cpu.flags.zf)
    {
        goto L_0x0051efbf;
    }
    // 0051efba  80fedf                 +cmp dh, 0xdf
    {
        x86::reg8 tmp1 = cpu.dh;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(223 /*0xdf*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051efbd  7510                   -jne 0x51efcf
    if (!cpu.flags.zf)
    {
        goto L_0x0051efcf;
    }
L_0x0051efbf:
    // 0051efbf  8a5001                 -mov dl, byte ptr [eax + 1]
    cpu.dl = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0051efc2  80e230                 -and dl, 0x30
    cpu.dl &= x86::reg8(x86::sreg8(48 /*0x30*/));
    // 0051efc5  80fa10                 +cmp dl, 0x10
    {
        x86::reg8 tmp1 = cpu.dl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(16 /*0x10*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051efc8  7505                   -jne 0x51efcf
    if (!cpu.flags.zf)
    {
        goto L_0x0051efcf;
    }
    // 0051efca  bb8d000000             -mov ebx, 0x8d
    cpu.ebx = 141 /*0x8d*/;
L_0x0051efcf:
    // 0051efcf  f60001                 +test byte ptr [eax], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax) & 1 /*0x1*/));
    // 0051efd2  7539                   -jne 0x51f00d
    if (!cpu.flags.zf)
    {
        goto L_0x0051f00d;
    }
    // 0051efd4  8a4001                 -mov al, byte ptr [eax + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0051efd7  2430                   -and al, 0x30
    cpu.al &= x86::reg8(x86::sreg8(48 /*0x30*/));
    // 0051efd9  3c30                   +cmp al, 0x30
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
    // 0051efdb  7530                   -jne 0x51f00d
    if (!cpu.flags.zf)
    {
        goto L_0x0051f00d;
    }
    // 0051efdd  8b4720                 -mov eax, dword ptr [edi + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(32) /* 0x20 */);
    // 0051efe0  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 0051efe5  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0051efe8  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0051efea  66c1e80d               -shr ax, 0xd
    cpu.ax >>= 13 /*0xd*/ % 32;
    // 0051efee  8b5724                 -mov edx, dword ptr [edi + 0x24]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(36) /* 0x24 */);
    // 0051eff1  6689c1                 -mov cx, ax
    cpu.cx = cpu.ax;
    // 0051eff4  81e2ffff0000           -and edx, 0xffff
    cpu.edx &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 0051effa  01c9                   -add ecx, ecx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0051effc  d3ea                   -shr edx, cl
    cpu.edx >>= cpu.cl % 32;
    // 0051effe  83e201                 -and edx, 1
    cpu.edx &= x86::reg32(x86::sreg32(1 /*0x1*/));
    // 0051f001  83fa01                 +cmp edx, 1
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051f004  7507                   -jne 0x51f00d
    if (!cpu.flags.zf)
    {
        goto L_0x0051f00d;
    }
  [[fallthrough]];
  case 0x0051f006:
    // 0051f006  bb83000000             -mov ebx, 0x83
    cpu.ebx = 131 /*0x83*/;
    // 0051f00b  eb05                   -jmp 0x51f012
    goto L_0x0051f012;
L_0x0051f00d:
    // 0051f00d  83fbff                 +cmp ebx, -1
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051f010  743d                   -je 0x51f04f
    if (cpu.flags.zf)
    {
        goto L_0x0051f04f;
    }
L_0x0051f012:
    // 0051f012  c60594baa00001         -mov byte ptr [0xa0ba94], 1
    app->getMemory<x86::reg8>(x86::reg32(10533524) /* 0xa0ba94 */) = 1 /*0x1*/;
    // 0051f019  e852700000             -call 0x526070
    cpu.esp -= 4;
    sub_526070(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051f01e  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051f020  e81f720000             -call 0x526244
    cpu.esp -= 4;
    sub_526244(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051f025  83f8ff                 +cmp eax, -1
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
    // 0051f028  0f8474000000           -je 0x51f0a2
    if (cpu.flags.zf)
    {
        goto L_0x0051f0a2;
    }
    // 0051f02e  803d94baa00000         +cmp byte ptr [0xa0ba94], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(10533524) /* 0xa0ba94 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051f035  0f8467000000           -je 0x51f0a2
    if (cpu.flags.zf)
    {
        goto L_0x0051f0a2;
    }
    // 0051f03b  668b5f20               -mov bx, word ptr [edi + 0x20]
    cpu.bx = app->getMemory<x86::reg16>(cpu.edi + x86::reg32(32) /* 0x20 */);
    // 0051f03f  30db                   -xor bl, bl
    cpu.bl ^= x86::reg8(x86::sreg8(cpu.bl));
    // 0051f041  80e77f                 -and bh, 0x7f
    cpu.bh &= x86::reg8(x86::sreg8(127 /*0x7f*/));
    // 0051f044  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 0051f046  66895f20               -mov word ptr [edi + 0x20], bx
    app->getMemory<x86::reg16>(cpu.edi + x86::reg32(32) /* 0x20 */) = cpu.bx;
    // 0051f04a  e977000000             -jmp 0x51f0c6
    goto L_0x0051f0c6;
L_0x0051f04f:
    // 0051f04f  833dd8ac560000         +cmp dword ptr [0x56acd8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5680344) /* 0x56acd8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051f056  744a                   -je 0x51f0a2
    if (cpu.flags.zf)
    {
        goto L_0x0051f0a2;
    }
    // 0051f058  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0051f05d  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
L_0x0051f05f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0051f05f  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051f061  8b16                   -mov edx, dword ptr [esi]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi);
    // 0051f063  ff15d4ac5600           -call dword ptr [0x56acd4]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5680340) /* 0x56acd4 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051f069  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051f06b  742f                   -je 0x51f09c
    if (cpu.flags.zf)
    {
        goto L_0x0051f09c;
    }
    // 0051f06d  83f801                 +cmp eax, 1
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
    // 0051f070  7430                   -je 0x51f0a2
    if (cpu.flags.zf)
    {
        goto L_0x0051f0a2;
    }
    // 0051f072  83f802                 +cmp eax, 2
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
    // 0051f075  742b                   -je 0x51f0a2
    if (cpu.flags.zf)
    {
        goto L_0x0051f0a2;
    }
    // 0051f077  83f803                 +cmp eax, 3
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
    // 0051f07a  7426                   -je 0x51f0a2
    if (cpu.flags.zf)
    {
        goto L_0x0051f0a2;
    }
    // 0051f07c  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0051f07e  880d94baa000           -mov byte ptr [0xa0ba94], cl
    app->getMemory<x86::reg8>(x86::reg32(10533524) /* 0xa0ba94 */) = cpu.cl;
    // 0051f084  ff15d8ac5600           -call dword ptr [0x56acd8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5680344) /* 0x56acd8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051f08a  803d94baa00000         +cmp byte ptr [0xa0ba94], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(10533524) /* 0xa0ba94 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051f091  7409                   -je 0x51f09c
    if (cpu.flags.zf)
    {
        goto L_0x0051f09c;
    }
    // 0051f093  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051f095  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0051f098  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051f099  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051f09a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051f09b  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0051f09c:
    // 0051f09c  43                     -inc ebx
    (cpu.ebx)++;
    // 0051f09d  83fb0c                 +cmp ebx, 0xc
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(12 /*0xc*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051f0a0  7ebd                   -jle 0x51f05f
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051f05f;
    }
L_0x0051f0a2:
    // 0051f0a2  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 0051f0a4  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0051f0a5  89742404               -mov dword ptr [esp + 4], esi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.esi;
    // 0051f0a9  897c2408               -mov dword ptr [esp + 8], edi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.edi;
    // 0051f0ad  2eff151c465300         -call dword ptr cs:[0x53461c]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457436) /* 0x53461c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051f0b4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0051f0b6  7409                   -je 0x51f0c1
    if (cpu.flags.zf)
    {
        goto L_0x0051f0c1;
    }
    // 0051f0b8  6aff                   -push -1
    app->getMemory<x86::reg32>(cpu.esp-4) = -1 /*-0x1*/;
    cpu.esp -= 4;
    // 0051f0ba  2eff15c0445300         -call dword ptr cs:[0x5344c0]
    cpu.ip = app->getMemory<x86::reg32>(cpu.ecs + x86::reg32(5457088) /* 0x5344c0 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0051f0c1:
    // 0051f0c1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
L_0x0051f0c6:
    // 0051f0c6  83c408                 -add esp, 8
    (cpu.esp) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0051f0c9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051f0ca  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051f0cb  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051f0cc  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* sub_51f0d0: native, nfs3hp::watcom::InitExceptionHandling */

/* sub_51f11c: native, nfs3hp::watcom::DoneExceptionHandler */

/* align: skip  */
void Application::sub_51f140(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 0051f140  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0051f141  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0051f143  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051f144  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0051f145  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0051f146  83ec40                 -sub esp, 0x40
    (cpu.esp) -= x86::reg32(x86::sreg32(64 /*0x40*/));
    // 0051f149  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0051f14b  89d9                   -mov ecx, ebx
    cpu.ecx = cpu.ebx;
    // 0051f14d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051f14f  8a4315                 -mov al, byte ptr [ebx + 0x15]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(21) /* 0x15 */);
    // 0051f152  8945c0                 -mov dword ptr [ebp - 0x40], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = cpu.eax;
    // 0051f155  8a4115                 -mov al, byte ptr [ecx + 0x15]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(21) /* 0x15 */);
    // 0051f158  8b5b08                 -mov ebx, dword ptr [ebx + 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */);
    // 0051f15b  245f                   -and al, 0x5f
    cpu.al &= x86::reg8(x86::sreg8(95 /*0x5f*/));
    // 0051f15d  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 0051f162  83f847                 +cmp eax, 0x47
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(71 /*0x47*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051f165  7523                   -jne 0x51f18a
    if (!cpu.flags.zf)
    {
        goto L_0x0051f18a;
    }
    // 0051f167  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 0051f169  7505                   -jne 0x51f170
    if (!cpu.flags.zf)
    {
        goto L_0x0051f170;
    }
    // 0051f16b  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
L_0x0051f170:
    // 0051f170  c745bc04000000         -mov dword ptr [ebp - 0x44], 4
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */) = 4 /*0x4*/;
    // 0051f177  8b7dc0                 -mov edi, dword ptr [ebp - 0x40]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 0051f17a  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0051f17f  83ef02                 +sub edi, 2
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0051f182  8945b8                 -mov dword ptr [ebp - 0x48], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */) = cpu.eax;
    // 0051f185  897dc0                 -mov dword ptr [ebp - 0x40], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = cpu.edi;
    // 0051f188  eb1f                   -jmp 0x51f1a9
    goto L_0x0051f1a9;
L_0x0051f18a:
    // 0051f18a  83f845                 +cmp eax, 0x45
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(69 /*0x45*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051f18d  750d                   -jne 0x51f19c
    if (!cpu.flags.zf)
    {
        goto L_0x0051f19c;
    }
    // 0051f18f  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 0051f194  897dbc                 -mov dword ptr [ebp - 0x44], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */) = cpu.edi;
    // 0051f197  897db8                 -mov dword ptr [ebp - 0x48], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */) = cpu.edi;
    // 0051f19a  eb0d                   -jmp 0x51f1a9
    goto L_0x0051f1a9;
L_0x0051f19c:
    // 0051f19c  bf02000000             -mov edi, 2
    cpu.edi = 2 /*0x2*/;
    // 0051f1a1  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0051f1a3  897dbc                 -mov dword ptr [ebp - 0x44], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */) = cpu.edi;
    // 0051f1a6  8945b8                 -mov dword ptr [ebp - 0x48], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */) = cpu.eax;
L_0x0051f1a9:
    // 0051f1a9  f6411e01               +test byte ptr [ecx + 0x1e], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(30) /* 0x1e */) & 1 /*0x1*/));
    // 0051f1ad  7404                   -je 0x51f1b3
    if (cpu.flags.zf)
    {
        goto L_0x0051f1b3;
    }
    // 0051f1af  804dbc10               -or byte ptr [ebp - 0x44], 0x10
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-68) /* -0x44 */) |= x86::reg8(x86::sreg8(16 /*0x10*/));
L_0x0051f1b3:
    // 0051f1b3  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 0051f1b5  83c008                 -add eax, 8
    (cpu.eax) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0051f1b8  8902                   -mov dword ptr [edx], eax
    app->getMemory<x86::reg32>(cpu.edx) = cpu.eax;
    // 0051f1ba  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0051f1bc  8b40f8                 -mov eax, dword ptr [eax - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-8) /* -0x8 */);
    // 0051f1bf  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 0051f1c2  8b42fc                 -mov eax, dword ptr [edx - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(-4) /* -0x4 */);
    // 0051f1c5  8d55e0                 -lea edx, [ebp - 0x20]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 0051f1c8  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 0051f1cb  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0051f1ce  dd00                   -fld qword ptr [eax]
    fp0 = x86::Float(app->getMemory<double>(cpu.eax));
    // 0051f1d0  db3a                   -fstp xword ptr [edx]
    app->getMemory<x86::IEEEf80>(cpu.edx) = x86::Float(fp0);
    // 0051f1d2  83fbff                 +cmp ebx, -1
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051f1d5  7505                   -jne 0x51f1dc
    if (!cpu.flags.zf)
    {
        goto L_0x0051f1dc;
    }
    // 0051f1d7  bb06000000             -mov ebx, 6
    cpu.ebx = 6 /*0x6*/;
L_0x0051f1dc:
    // 0051f1dc  8d45e0                 -lea eax, [ebp - 0x20]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 0051f1df  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051f1e1  895db4                 -mov dword ptr [ebp - 0x4c], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */) = cpu.ebx;
    // 0051f1e4  8955c4                 -mov dword ptr [ebp - 0x3c], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.edx;
    // 0051f1e7  8d5e01                 -lea ebx, [esi + 1]
    cpu.ebx = x86::reg32(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0051f1ea  8d55b4                 -lea edx, [ebp - 0x4c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-76) /* -0x4c */);
    // 0051f1ed  e8c175ffff             -call 0x5167b3
    cpu.esp -= 4;
    sub_5167b3(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051f1f2  8b45d0                 -mov eax, dword ptr [ebp - 0x30]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0051f1f5  894128                 -mov dword ptr [ecx + 0x28], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(40) /* 0x28 */) = cpu.eax;
    // 0051f1f8  8b45d4                 -mov eax, dword ptr [ebp - 0x2c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 0051f1fb  89412c                 -mov dword ptr [ecx + 0x2c], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(44) /* 0x2c */) = cpu.eax;
    // 0051f1fe  8b45d8                 -mov eax, dword ptr [ebp - 0x28]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 0051f201  894130                 -mov dword ptr [ecx + 0x30], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(48) /* 0x30 */) = cpu.eax;
    // 0051f204  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 0051f207  894134                 -mov dword ptr [ecx + 0x34], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(52) /* 0x34 */) = cpu.eax;
    // 0051f20a  837dc800               +cmp dword ptr [ebp - 0x38], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0051f20e  7d0f                   -jge 0x51f21f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0051f21f;
    }
    // 0051f210  8b4120                 -mov eax, dword ptr [ecx + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */);
    // 0051f213  8d5001                 -lea edx, [eax + 1]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0051f216  895120                 -mov dword ptr [ecx + 0x20], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */) = cpu.edx;
    // 0051f219  c604062d               -mov byte ptr [esi + eax], 0x2d
    app->getMemory<x86::reg8>(cpu.esi + cpu.eax * 1) = 45 /*0x2d*/;
    // 0051f21d  eb29                   -jmp 0x51f248
    goto L_0x0051f248;
L_0x0051f21f:
    // 0051f21f  8a611e                 -mov ah, byte ptr [ecx + 0x1e]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(30) /* 0x1e */);
    // 0051f222  f6c404                 +test ah, 4
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 4 /*0x4*/));
    // 0051f225  740f                   -je 0x51f236
    if (cpu.flags.zf)
    {
        goto L_0x0051f236;
    }
    // 0051f227  8b4120                 -mov eax, dword ptr [ecx + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */);
    // 0051f22a  8d5001                 -lea edx, [eax + 1]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0051f22d  895120                 -mov dword ptr [ecx + 0x20], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */) = cpu.edx;
    // 0051f230  c604062b               -mov byte ptr [esi + eax], 0x2b
    app->getMemory<x86::reg8>(cpu.esi + cpu.eax * 1) = 43 /*0x2b*/;
    // 0051f234  eb12                   -jmp 0x51f248
    goto L_0x0051f248;
L_0x0051f236:
    // 0051f236  f6c402                 +test ah, 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & 2 /*0x2*/));
    // 0051f239  740d                   -je 0x51f248
    if (cpu.flags.zf)
    {
        goto L_0x0051f248;
    }
    // 0051f23b  8b4120                 -mov eax, dword ptr [ecx + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */);
    // 0051f23e  8d5001                 -lea edx, [eax + 1]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 0051f241  895120                 -mov dword ptr [ecx + 0x20], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(32) /* 0x20 */) = cpu.edx;
    // 0051f244  c6040620               -mov byte ptr [esi + eax], 0x20
    app->getMemory<x86::reg8>(cpu.esi + cpu.eax * 1) = 32 /*0x20*/;
L_0x0051f248:
    // 0051f248  8cda                   -mov edx, ds
    cpu.edx = cpu.ds;
    // 0051f24a  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0051f24c  8d65f4                 -lea esp, [ebp - 0xc]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0051f24f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051f250  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051f251  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051f252  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051f253  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_51f254(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 0051f254  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051f255  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 0051f257  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0051f259  e85890fdff             -call 0x4f82b6
    cpu.esp -= 4;
    nfs3hp::watcom::strtod(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fp0 = cpu.fpu.take();
    // 0051f25e  dd1b                   -fstp qword ptr [ebx]
    app->getMemory<double>(cpu.ebx) = double(fp0);
    // 0051f260  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051f261  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_51f270(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0051f270  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_51f272: native, nfs3hp::dead::stub_0x51f272 */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_51f2a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fpa1 = cpu.fpu.st(0);
    // 0051f2a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0051f2a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0051f2a2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0051f2a3  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0051f2a6  c7442408000000c0       -mov dword ptr [esp + 8], 0xc0000000
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = 3221225472 /*0xc0000000*/;
    // 0051f2ae  c744240c7e015041       -mov dword ptr [esp + 0xc], 0x4150017e
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = 1095762302 /*0x4150017e*/;
    // 0051f2b6  c7042400000080         -mov dword ptr [esp], 0x80000000
    app->getMemory<x86::reg32>(cpu.esp) = 2147483648 /*0x80000000*/;
    // 0051f2bd  c7442404ffff4741       -mov dword ptr [esp + 4], 0x4147ffff
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */) = 1095237631 /*0x4147ffff*/;
    // 0051f2c5  803dc144560003         +cmp byte ptr [0x5644c1], 3
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5653697) /* 0x5644c1 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(3 /*0x3*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 0051f2cc  7249                   -jb 0x51f317
    if (cpu.flags.cf)
    {
        goto L_0x0051f317;
    }
    // 0051f2ce  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051f2d2  8b54240c               -mov edx, dword ptr [esp + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0051f2d6  8b1c24                 -mov ebx, dword ptr [esp]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051f2d9  8b4c2404               -mov ecx, dword ptr [esp + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051f2dd  e85a710000             -call 0x52643c
    cpu.fpu.push(fpa1);
    cpu.esp -= 4;
    sub_52643c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    fpa1 = cpu.fpu.take();
    // 0051f2e2  8b1c24                 -mov ebx, dword ptr [esp]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    // 0051f2e5  8b4c2404               -mov ecx, dword ptr [esp + 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(4) /* 0x4 */);
    // 0051f2e9  e811750000             -call 0x5267ff
    cpu.esp -= 4;
    sub_5267ff(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051f2ee  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0051f2f0  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0051f2f2  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 0051f2f6  8b54240c               -mov edx, dword ptr [esp + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 0051f2fa  e843730000             -call 0x526642
    cpu.esp -= 4;
    sub_526642(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051f2ff  bb3a8c30e2             -mov ebx, 0xe2308c3a
    cpu.ebx = 3794832442 /*0xe2308c3a*/;
    // 0051f304  b98e79453e             -mov ecx, 0x3e45798e
    cpu.ecx = 1044740494 /*0x3e45798e*/;
    // 0051f309  e88e760000             -call 0x52699c
    cpu.esp -= 4;
    sub_52699c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0051f30e  7e07                   -jle 0x51f317
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0051f317;
    }
    // 0051f310  800d5878560001         -or byte ptr [0x567858], 1
    app->getMemory<x86::reg8>(x86::reg32(5666904) /* 0x567858 */) |= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x0051f317:
    // 0051f317  83c410                 -add esp, 0x10
    (cpu.esp) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 0051f31a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051f31b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051f31c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0051f31d  c3                     -ret 
    cpu.fpu.st(0) = fpa1;
    cpu.esp += 4;
    return;
}

/* sub_51f320: native, nfs3hp::dead::stub_0x51f320 */

/* sub_51f520: native, nfs3hp::port::panSides */

/* sub_51f580: native, nfs3hp::port::volumeMonoPair */

/* sub_51f5a4: native, nfs3hp::port::volumeMonoLevels */

/* sub_51f5dd: native, nfs3hp::mix::addMono8 */

/* sub_51f680: native, nfs3hp::port::volumeStereoPair */

/* sub_51f6a4: native, nfs3hp::port::volumeStereoLevels */

/* sub_51f6c7: native, nfs3hp::mix::addStereo8 */

/* sub_51f750: native, nfs3hp::port::volumeMonoSetup */

/* sub_51f774: native, nfs3hp::port::volumeMonoLevelsMmx */

/* sub_51f7ad: native, nfs3hp::mix::addMono16 */

/* sub_51f850: native, nfs3hp::port::volumeStereoSetup */

/* sub_51f874: native, nfs3hp::port::volumeStereoLevelsMmx */

/* sub_51f897: native, nfs3hp::mix::addStereo16 */

/* sub_51f930: native, nfs3hp::port::resample8MonoNeededDown */

/* sub_51f978: native, nfs3hp::port::resample8MonoNeededSame */

/* sub_51f980: native, nfs3hp::port::resample8MonoNeededUp */

/* sub_51f9ac: native, nfs3hp::port::resample8MonoDown */

/* sub_51fa80: native, nfs3hp::port::resample8MonoSame */

/* sub_51fa8c: native, nfs3hp::port::resample8MonoUp */

/* sub_51fb94: native, nfs3hp::port::resample8MonoStep */

/* sub_51fc38: native, nfs3hp::port::resample8MonoSetup */

/* sub_51fc80: native, nfs3hp::port::resample8StereoNeededDown */

/* sub_51fcc8: native, nfs3hp::port::resample8StereoNeededSame */

/* sub_51fcd0: native, nfs3hp::port::resample8StereoNeededUp */

/* sub_51fcfc: native, nfs3hp::port::resample8StereoDown */

/* sub_51fe24: native, nfs3hp::port::resample8StereoSame */

/* sub_51fe30: native, nfs3hp::port::resample8StereoUp */

/* sub_51ff88: native, nfs3hp::port::resample8StereoStep */

/* sub_52002c: native, nfs3hp::port::resample8StereoSetup */

/* sub_520070: native, nfs3hp::port::resample16MonoNeededDown */

/* sub_5200b8: native, nfs3hp::port::resample16MonoNeededSame */

/* sub_5200c0: native, nfs3hp::port::resample16MonoNeededUp */

}
