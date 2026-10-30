#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4652f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004652f0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004652f1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004652f3  833d4c89670000         +cmp dword ptr [0x67894c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6785356) /* 0x67894c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004652fa  7407                   -je 0x465303
    if (cpu.flags.zf)
    {
        goto L_0x00465303;
    }
    // 004652fc  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 00465301  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465302  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00465303:
    // 00465303  833d5089670000         +cmp dword ptr [0x678950], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(6785360) /* 0x678950 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046530a  7407                   -je 0x465313
    if (cpu.flags.zf)
    {
        goto L_0x00465313;
    }
    // 0046530c  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 00465311  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465312  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x00465313:
    // 00465313  b8fdffffff             -mov eax, 0xfffffffd
    cpu.eax = 4294967293 /*0xfffffffd*/;
    // 00465318  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465319  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_465320(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00465320  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00465321  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00465322  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00465323  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00465325  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 00465327  ba21000000             -mov edx, 0x21
    cpu.edx = 33 /*0x21*/;
    // 0046532c  e82f53feff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00465331  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00465333  741e                   -je 0x465353
    if (cpu.flags.zf)
    {
        goto L_0x00465353;
    }
    // 00465335  66c7411e4001           -mov word ptr [ecx + 0x1e], 0x140
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(30) /* 0x1e */) = 320 /*0x140*/;
    // 0046533b  668b4106               -mov ax, word ptr [ecx + 6]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(6) /* 0x6 */);
    // 0046533f  66c74120c000           -mov word ptr [ecx + 0x20], 0xc0
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(32) /* 0x20 */) = 192 /*0xc0*/;
    // 00465345  6689411c               -mov word ptr [ecx + 0x1c], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(28) /* 0x1c */) = cpu.ax;
    // 00465349  668b4108               -mov ax, word ptr [ecx + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046534d  6689411a               -mov word ptr [ecx + 0x1a], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(26) /* 0x1a */) = cpu.ax;
    // 00465351  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x00465353:
    // 00465353  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465354  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465355  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465356  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_465360(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00465360  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00465361  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00465363  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465364  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_465370(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00465370  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00465371  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00465372  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00465373  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00465374  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00465375  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00465376  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00465378  81ec58020000           -sub esp, 0x258
    (cpu.esp) -= x86::reg32(x86::sreg32(600 /*0x258*/));
    // 0046537e  a198476600             -mov eax, dword ptr [0x664798]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6703000) /* 0x664798 */);
    // 00465383  e8283ffdff             -call 0x4392b0
    cpu.esp -= 4;
    nfs3hp::load::carFolder(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00465388  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0046538a  a198476600             -mov eax, dword ptr [0x664798]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6703000) /* 0x664798 */);
    // 0046538f  a340896700             -mov dword ptr [0x678940], eax
    app->getMemory<x86::reg32>(x86::reg32(6785344) /* 0x678940 */) = cpu.eax;
    // 00465394  e8f73efdff             -call 0x439290
    cpu.esp -= 4;
    sub_439290(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00465399  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046539a  683cab5300             -push 0x53ab3c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5483324 /*0x53ab3c*/;
    cpu.esp -= 4;
    // 0046539f  8d85a8fdffff           -lea eax, [ebp - 0x258]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-600) /* -0x258 */);
    // 004653a5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004653a6  e8e5a20700             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004653ab  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004653ae  ba48ab5300             -mov edx, 0x53ab48
    cpu.edx = 5483336 /*0x53ab48*/;
    // 004653b3  e8485afeff             -call 0x44ae00
    cpu.esp -= 4;
    nfs3hp::frontend::menuCurrentName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004653b8  e8538f0800             -call 0x4ee310
    cpu.esp -= 4;
    nfs3hp::watcom::strcmp(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004653bd  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004653bf  7540                   -jne 0x465401
    if (!cpu.flags.zf)
    {
        goto L_0x00465401;
    }
    // 004653c1  8b1598476600           -mov edx, dword ptr [0x664798]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6703000) /* 0x664798 */);
    // 004653c7  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004653c8  6854ab5300             -push 0x53ab54
    app->getMemory<x86::reg32>(cpu.esp-4) = 5483348 /*0x53ab54*/;
    cpu.esp -= 4;
    // 004653cd  8d85a8fdffff           -lea eax, [ebp - 0x258]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-600) /* -0x258 */);
    // 004653d3  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004653d4  e8b7a20700             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004653d9  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004653dc  be34327a00             -mov esi, 0x7a3234
    cpu.esi = 8008244 /*0x7a3234*/;
    // 004653e1  8dbdd4feffff           -lea edi, [ebp - 0x12c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-300) /* -0x12c */);
    // 004653e7  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x004653e8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004653e8  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004653ea  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 004653ec  3c00                   +cmp al, 0
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
    // 004653ee  7410                   -je 0x465400
    if (cpu.flags.zf)
    {
        goto L_0x00465400;
    }
    // 004653f0  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004653f3  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004653f6  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004653f9  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004653fc  3c00                   +cmp al, 0
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
    // 004653fe  75e8                   -jne 0x4653e8
    if (!cpu.flags.zf)
    {
        goto L_0x004653e8;
    }
L_0x00465400:
    // 00465400  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x00465401:
    // 00465401  a198476600             -mov eax, dword ptr [0x664798]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6703000) /* 0x664798 */);
    // 00465406  e8b541fdff             -call 0x4395c0
    cpu.esp -= 4;
    nfs3hp::load::carFlag4(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046540b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046540d  741c                   -je 0x46542b
    if (cpu.flags.zf)
    {
        goto L_0x0046542b;
    }
    // 0046540f  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00465410  68b4287a00             -push 0x7a28b4
    app->getMemory<x86::reg32>(cpu.esp-4) = 8005812 /*0x7a28b4*/;
    cpu.esp -= 4;
    // 00465415  6860ab5300             -push 0x53ab60
    app->getMemory<x86::reg32>(cpu.esp-4) = 5483360 /*0x53ab60*/;
    cpu.esp -= 4;
    // 0046541a  8d85d4feffff           -lea eax, [ebp - 0x12c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-300) /* -0x12c */);
    // 00465420  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 00465421  e86aa20700             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00465426  83c410                 +add esp, 0x10
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 00465429  eb25                   -jmp 0x465450
    goto L_0x00465450;
L_0x0046542b:
    // 0046542b  be34327a00             -mov esi, 0x7a3234
    cpu.esi = 8008244 /*0x7a3234*/;
    // 00465430  8dbdd4feffff           -lea edi, [ebp - 0x12c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-300) /* -0x12c */);
    // 00465436  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x00465437:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 00465437  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 00465439  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0046543b  3c00                   +cmp al, 0
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
    // 0046543d  7410                   -je 0x46544f
    if (cpu.flags.zf)
    {
        goto L_0x0046544f;
    }
    // 0046543f  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 00465442  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 00465445  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 00465448  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046544b  3c00                   +cmp al, 0
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
    // 0046544d  75e8                   -jne 0x465437
    if (!cpu.flags.zf)
    {
        goto L_0x00465437;
    }
L_0x0046544f:
    // 0046544f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
L_0x00465450:
    // 00465450  8d95d4feffff           -lea edx, [ebp - 0x12c]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-300) /* -0x12c */);
    // 00465456  8d85a8fdffff           -lea eax, [ebp - 0x258]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-600) /* -0x258 */);
    // 0046545c  b90000803f             -mov ecx, 0x3f800000
    cpu.ecx = 1065353216 /*0x3f800000*/;
    // 00465461  e89af2ffff             -call 0x464700
    cpu.esp -= 4;
    sub_464700(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00465466  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00465468  a198476600             -mov eax, dword ptr [0x664798]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6703000) /* 0x664798 */);
    // 0046546d  890d38a45500           -mov dword ptr [0x55a438], ecx
    app->getMemory<x86::reg32>(x86::reg32(5612600) /* 0x55a438 */) = cpu.ecx;
    // 00465473  891d40a45500           -mov dword ptr [0x55a440], ebx
    app->getMemory<x86::reg32>(x86::reg32(5612608) /* 0x55a440 */) = cpu.ebx;
    // 00465479  e84268fdff             -call 0x43bcc0
    cpu.esp -= 4;
    sub_43bcc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046547e  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046547f  6814020000             -push 0x214
    app->getMemory<x86::reg32>(cpu.esp-4) = 532 /*0x214*/;
    cpu.esp -= 4;
    // 00465484  6878ab5300             -push 0x53ab78
    app->getMemory<x86::reg32>(cpu.esp-4) = 5483384 /*0x53ab78*/;
    cpu.esp -= 4;
    // 00465489  689cab5300             -push 0x53ab9c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5483420 /*0x53ab9c*/;
    cpu.esp -= 4;
    // 0046548e  6a02                   -push 2
    app->getMemory<x86::reg32>(cpu.esp-4) = 2 /*0x2*/;
    cpu.esp -= 4;
    // 00465490  6a5a                   -push 0x5a
    app->getMemory<x86::reg32>(cpu.esp-4) = 90 /*0x5a*/;
    cpu.esp -= 4;
    // 00465492  a33ca45500             -mov dword ptr [0x55a43c], eax
    app->getMemory<x86::reg32>(x86::reg32(5612604) /* 0x55a43c */) = cpu.eax;
    // 00465497  e87480fcff             -call 0x42d510
    cpu.esp -= 4;
    nfs3hp::ea::gameTrace(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046549c  83c418                 -add esp, 0x18
    (cpu.esp) += x86::reg32(x86::sreg32(24 /*0x18*/));
    // 0046549f  891d58896700           -mov dword ptr [0x678958], ebx
    app->getMemory<x86::reg32>(x86::reg32(6785368) /* 0x678958 */) = cpu.ebx;
    // 004654a5  890d54896700           -mov dword ptr [0x678954], ecx
    app->getMemory<x86::reg32>(x86::reg32(6785364) /* 0x678954 */) = cpu.ecx;
    // 004654ab  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004654ad  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004654ae  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004654af  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004654b0  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004654b1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004654b2  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004654b3  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4654c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004654c0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004654c1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004654c3  8b450c                 -mov eax, dword ptr [ebp + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    // 004654c6  a358896700             -mov dword ptr [0x678958], eax
    app->getMemory<x86::reg32>(x86::reg32(6785368) /* 0x678958 */) = cpu.eax;
    // 004654cb  8b4508                 -mov eax, dword ptr [ebp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(8) /* 0x8 */);
    // 004654ce  a354896700             -mov dword ptr [0x678954], eax
    app->getMemory<x86::reg32>(x86::reg32(6785364) /* 0x678954 */) = cpu.eax;
    // 004654d3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004654d4  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* sub_4654e0: native, nfs3hp::dead::stub_0x4654e0 */

/* sub_4654f0: native, nfs3hp::dead::stub_0x4654f0 */

/* sub_465500: native, nfs3hp::port::fields::drawListBox */

/* sub_465880: native, nfs3hp::port::fields::pointAtListBoxLine */

/* sub_4658f0: native, nfs3hp::port::fields::listBoxDown */

/* sub_465950: native, nfs3hp::port::fields::listBoxUp */

/* sub_4659a0: native, nfs3hp::port::fields::listBoxPartAt */

/* sub_465a10: native, nfs3hp::port::fields::listBoxArrowAt */

/* sub_465a30: native, nfs3hp::port::fields::listBoxKey */

/* sub_465ba0: native, nfs3hp::port::fields::layoutListBox */

/* sub_465c10: native, nfs3hp::port::fields::listBoxShown */

/* sub_465c60: native, nfs3hp::port::fields::listBoxHidden */

/* sub_465ca0: native, nfs3hp::port::fields::listBoxChooseAt */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
/* data blob: 0000803f00000000000080bf0000803f0000000000000000000000000000803f0000000000000000000000000000803f0000000000000000000000000000000000000000000000000000000000000000 */
void Application::sub_465d60(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00465d60  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00465d61  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00465d62  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00465d64  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00465d66  8915548a6700           -mov dword ptr [0x678a54], edx
    app->getMemory<x86::reg32>(x86::reg32(6785620) /* 0x678a54 */) = cpu.edx;
    // 00465d6c  8915d4a45500           -mov dword ptr [0x55a4d4], edx
    app->getMemory<x86::reg32>(x86::reg32(5612756) /* 0x55a4d4 */) = cpu.edx;
    // 00465d72  8915d0a45500           -mov dword ptr [0x55a4d0], edx
    app->getMemory<x86::reg32>(x86::reg32(5612752) /* 0x55a4d0 */) = cpu.edx;
    // 00465d78  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465d79  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465d7a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_465d80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00465d80  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00465d81  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00465d82  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00465d83  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00465d84  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00465d85  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00465d86  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00465d88  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00465d8b  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 00465d8d  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 00465d8f  8915548a6700           -mov dword ptr [0x678a54], edx
    app->getMemory<x86::reg32>(x86::reg32(6785620) /* 0x678a54 */) = cpu.edx;
    // 00465d95  8915d4a45500           -mov dword ptr [0x55a4d4], edx
    app->getMemory<x86::reg32>(x86::reg32(5612756) /* 0x55a4d4 */) = cpu.edx;
    // 00465d9b  8915d0a45500           -mov dword ptr [0x55a4d0], edx
    app->getMemory<x86::reg32>(x86::reg32(5612752) /* 0x55a4d0 */) = cpu.edx;
    // 00465da1  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00465da3  0f84f8000000           -je 0x465ea1
    if (cpu.flags.zf)
    {
        goto L_0x00465ea1;
    }
    // 00465da9  bf04000000             -mov edi, 4
    cpu.edi = 4 /*0x4*/;
    // 00465dae  bad8a45500             -mov edx, 0x55a4d8
    cpu.edx = 5612760 /*0x55a4d8*/;
    // 00465db3  b800010000             -mov eax, 0x100
    cpu.eax = 256 /*0x100*/;
    // 00465db8  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00465dba  893dd8a45500           -mov dword ptr [0x55a4d8], edi
    app->getMemory<x86::reg32>(x86::reg32(5612760) /* 0x55a4d8 */) = cpu.edi;
    // 00465dc0  e87b010700             -call 0x4d5f40
    cpu.esp -= 4;
    nfs3hp::graphics::makeArtTexture(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00465dc5  bab8ab5300             -mov edx, 0x53abb8
    cpu.edx = 5483448 /*0x53abb8*/;
    // 00465dca  a3d4a45500             -mov dword ptr [0x55a4d4], eax
    app->getMemory<x86::reg32>(x86::reg32(5612756) /* 0x55a4d4 */) = cpu.eax;
    // 00465dcf  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00465dd1  e80a940800             -call 0x4ef1e0
    cpu.esp -= 4;
    sub_4ef1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00465dd6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00465dd8  743a                   -je 0x465e14
    if (cpu.flags.zf)
    {
        goto L_0x00465e14;
    }
    // 00465dda  668b5004               -mov dx, word ptr [eax + 4]
    cpu.dx = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00465dde  6681fa0001             +cmp dx, 0x100
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(256 /*0x100*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00465de3  752f                   -jne 0x465e14
    if (!cpu.flags.zf)
    {
        goto L_0x00465e14;
    }
    // 00465de5  663b5006               +cmp dx, word ptr [eax + 6]
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */)));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00465de9  7529                   -jne 0x465e14
    if (!cpu.flags.zf)
    {
        goto L_0x00465e14;
    }
    // 00465deb  f605583a7a0002         +test byte ptr [0x7a3a58], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */) & 2 /*0x2*/));
    // 00465df2  740b                   -je 0x465dff
    if (cpu.flags.zf)
    {
        goto L_0x00465dff;
    }
    // 00465df4  bb00010000             -mov ebx, 0x100
    cpu.ebx = 256 /*0x100*/;
    // 00465df9  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 00465dfb  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00465dfd  eb09                   -jmp 0x465e08
    goto L_0x00465e08;
L_0x00465dff:
    // 00465dff  bb00010000             -mov ebx, 0x100
    cpu.ebx = 256 /*0x100*/;
    // 00465e04  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00465e06  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
L_0x00465e08:
    // 00465e08  b8d0a45500             -mov eax, 0x55a4d0
    cpu.eax = 5612752 /*0x55a4d0*/;
    // 00465e0d  e85e020700             -call 0x4d6070
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeTexture(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00465e12  eb07                   -jmp 0x465e1b
    goto L_0x00465e1b;
L_0x00465e14:
    // 00465e14  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 00465e16  a3d0a45500             -mov dword ptr [0x55a4d0], eax
    app->getMemory<x86::reg32>(x86::reg32(5612752) /* 0x55a4d0 */) = cpu.eax;
L_0x00465e1b:
    // 00465e1b  bac0ab5300             -mov edx, 0x53abc0
    cpu.edx = 5483456 /*0x53abc0*/;
    // 00465e20  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 00465e22  e8b9930800             -call 0x4ef1e0
    cpu.esp -= 4;
    sub_4ef1e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00465e27  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00465e29  0f846a000000           -je 0x465e99
    if (cpu.flags.zf)
    {
        goto L_0x00465e99;
    }
    // 00465e2f  668b4804               -mov cx, word ptr [eax + 4]
    cpu.cx = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 00465e33  6683f920               +cmp cx, 0x20
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(32 /*0x20*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00465e37  7560                   -jne 0x465e99
    if (!cpu.flags.zf)
    {
        goto L_0x00465e99;
    }
    // 00465e39  663b4806               +cmp cx, word ptr [eax + 6]
    {
        x86::reg16 tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */)));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00465e3d  755a                   -jne 0x465e99
    if (!cpu.flags.zf)
    {
        goto L_0x00465e99;
    }
    // 00465e3f  bb20000000             -mov ebx, 0x20
    cpu.ebx = 32 /*0x20*/;
    // 00465e44  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 00465e46  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 00465e48  8d45fc                 -lea eax, [ebp - 4]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00465e4b  e820020700             -call 0x4d6070
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeTexture(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 00465e50  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00465e53  bb0000803f             -mov ebx, 0x3f800000
    cpu.ebx = 1065353216 /*0x3f800000*/;
    // 00465e58  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 00465e5a  a3548a6700             -mov dword ptr [0x678a54], eax
    app->getMemory<x86::reg32>(x86::reg32(6785620) /* 0x678a54 */) = cpu.eax;
    // 00465e5f  8915588a6700           -mov dword ptr [0x678a58], edx
    app->getMemory<x86::reg32>(x86::reg32(6785624) /* 0x678a58 */) = cpu.edx;
    // 00465e65  89155c8a6700           -mov dword ptr [0x678a5c], edx
    app->getMemory<x86::reg32>(x86::reg32(6785628) /* 0x678a5c */) = cpu.edx;
    // 00465e6b  891d608a6700           -mov dword ptr [0x678a60], ebx
    app->getMemory<x86::reg32>(x86::reg32(6785632) /* 0x678a60 */) = cpu.ebx;
    // 00465e71  8915648a6700           -mov dword ptr [0x678a64], edx
    app->getMemory<x86::reg32>(x86::reg32(6785636) /* 0x678a64 */) = cpu.edx;
    // 00465e77  891d688a6700           -mov dword ptr [0x678a68], ebx
    app->getMemory<x86::reg32>(x86::reg32(6785640) /* 0x678a68 */) = cpu.ebx;
    // 00465e7d  891d6c8a6700           -mov dword ptr [0x678a6c], ebx
    app->getMemory<x86::reg32>(x86::reg32(6785644) /* 0x678a6c */) = cpu.ebx;
    // 00465e83  8915708a6700           -mov dword ptr [0x678a70], edx
    app->getMemory<x86::reg32>(x86::reg32(6785648) /* 0x678a70 */) = cpu.edx;
    // 00465e89  b60c                   -mov dh, 0xc
    cpu.dh = 12 /*0xc*/;
    // 00465e8b  891d748a6700           -mov dword ptr [0x678a74], ebx
    app->getMemory<x86::reg32>(x86::reg32(6785652) /* 0x678a74 */) = cpu.ebx;
    // 00465e91  8835788a6700           -mov byte ptr [0x678a78], dh
    app->getMemory<x86::reg8>(x86::reg32(6785656) /* 0x678a78 */) = cpu.dh;
    // 00465e97  eb08                   -jmp 0x465ea1
    goto L_0x00465ea1;
L_0x00465e99:
    // 00465e99  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 00465e9b  891d548a6700           -mov dword ptr [0x678a54], ebx
    app->getMemory<x86::reg32>(x86::reg32(6785620) /* 0x678a54 */) = cpu.ebx;
L_0x00465ea1:
    // 00465ea1  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 00465ea3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465ea4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465ea5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465ea6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465ea7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465ea8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 00465ea9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_465eb0: native, nfs3hp::port::obj3d::flareQuad */

/* sub_466330: native, nfs3hp::port::obj3d::lightPlaced */

/* sub_4663a0: native, nfs3hp::port::obj3d::headlightGlare */

/* sub_466580: native, nfs3hp::port::obj3d::taillightGlare */

/* sub_466740: native, nfs3hp::port::obj3d::objectLights */

/* sub_4667d0: native, nfs3hp::port::obj3d::objectDragged */

/* sub_4668b0: native, nfs3hp::port::obj3d::objectBounds */

/* sub_466cf0: native, nfs3hp::port::obj3d::objectDraw */

/* sub_4675b0: native, nfs3hp::port::obj3d::objectKeyed */

/* sub_4675d0: native, nfs3hp::port::obj3d::objectPassed */

/* sub_467650: native, nfs3hp::port::obj3d::objectReleased */

/* sub_467700: native, nfs3hp::port::obj3d::objectOpened */

/* sub_467930: native, nfs3hp::port::obj3d::objectClosed */

/* sub_4679f0: native, nfs3hp::ea::findVivEntry */

/* sub_467ac0: native, nfs3hp::port::obj3d::resetLoader */

/* sub_467bc0: native, nfs3hp::port::obj3d::genericVivPath */

/* sub_467be0: native, nfs3hp::port::obj3d::genericModelName */

/* sub_467c00: native, nfs3hp::port::obj3d::trackVivPath */

/* sub_467c20: native, nfs3hp::port::obj3d::trackModelName */

/* sub_467c40: native, nfs3hp::port::obj3d::chooseFiles */

/* sub_467ea0: native, nfs3hp::port::obj3d::readPrehead */

/* sub_467ef0: native, nfs3hp::port::obj3d::readHead */

/* sub_468010: native, nfs3hp::port::obj3d::readModel */

/* sub_468170: native, nfs3hp::port::obj3d::readTexture */

/* sub_4682c0: native, nfs3hp::port::obj3d::readShapes */

/* sub_468410: native, nfs3hp::port::obj3d::takeModel */

/* sub_468580: native, nfs3hp::port::obj3d::fadeIn */

/* sub_4685e0: native, nfs3hp::port::obj3d::fadeOut */

/* sub_468670: native, nfs3hp::port::obj3d::wantedChanged */

/* sub_468860: native, nfs3hp::port::obj3d::runLoader */

/* sub_468da0: native, nfs3hp::port::obj3d::carColour */

/* sub_468f00: native, nfs3hp::port::obj3d::makeTexture */

/* align: skip 0x00 0x00 */
void Application::sub_469020(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 00469020  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469022  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469024  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469026  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469028  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046902a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046902c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046902e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469030  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469032  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469034  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469036  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469038  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046903a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046903c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046903e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469040  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469042  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469044  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469046  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469048  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046904a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046904c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046904e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469050  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469052  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469054  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469056  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469058  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046905a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046905c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046905e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469060  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469062  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469064  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469066  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469068  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046906a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046906c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046906e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469070  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469072  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469074  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469076  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469078  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046907a  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046907c  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 0046907e  0000                   -add byte ptr [eax], al
    (app->getMemory<x86::reg8>(cpu.eax)) += x86::reg8(x86::sreg8(cpu.al));
    // 00469080  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 00469081  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 00469082  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 00469083  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 00469084  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 00469085  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 00469086  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 00469088  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0046908b  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0046908d  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x0046908f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046908f  8b4140                 -mov eax, dword ptr [ecx + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 00469092  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00469095  8b1445dea45500         -mov edx, dword ptr [eax*2 + 0x55a4de]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5612766) /* 0x55a4de */ + cpu.eax * 2);
    // 0046909c  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046909f  39d6                   +cmp esi, edx
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004690a1  0f8d10020000           -jge 0x4692b7
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004692b7;
    }
    // 004690a7  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004690a9  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004690ac  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004690ae  8d14b500000000         -lea edx, [esi*4]
    cpu.edx = x86::reg32(cpu.esi * 4);
    // 004690b5  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004690b8  8b1d34925500           -mov ebx, dword ptr [0x559234]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5607988) /* 0x559234 */);
    // 004690be  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004690c0  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004690c2  7475                   -je 0x469139
    if (cpu.flags.zf)
    {
        goto L_0x00469139;
    }
    // 004690c4  66c780b08a6700ffff     -mov word ptr [eax + 0x678ab0], 0xffff
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6785712) /* 0x678ab0 */) = 65535 /*0xffff*/;
    // 004690cd  8b5940                 -mov ebx, dword ptr [ecx + 0x40]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 004690d0  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 004690d3  8d049d00000000         -lea eax, [ebx*4]
    cpu.eax = x86::reg32(cpu.ebx * 4);
    // 004690da  29d8                   -sub eax, ebx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004690dc  66c784c2b28a6700ffff   -mov word ptr [edx + eax*8 + 0x678ab2], 0xffff
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(6785714) /* 0x678ab2 */ + cpu.eax * 8) = 65535 /*0xffff*/;
    // 004690e6  8b5140                 -mov edx, dword ptr [ecx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 004690e9  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004690ec  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004690f3  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004690f5  8d14f500000000         -lea edx, [esi*8]
    cpu.edx = x86::reg32(cpu.esi * 8);
    // 004690fc  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 004690ff  8d1c11                 -lea ebx, [ecx + edx]
    cpu.ebx = x86::reg32(cpu.ecx + cpu.edx * 1);
    // 00469102  668b8402f0ad5500       -mov ax, word ptr [edx + eax + 0x55adf0]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(5615088) /* 0x55adf0 */ + cpu.eax * 1);
    // 0046910a  66898380000000         -mov word ptr [ebx + 0x80], ax
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(128) /* 0x80 */) = cpu.ax;
    // 00469111  8b4140                 -mov eax, dword ptr [ecx + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 00469114  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00469117  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0046911a  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046911d  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 00469120  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 00469122  c1e004                 +shl eax, 4
    {
        x86::reg8 tmp = 4 /*0x4*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 00469125  668b8402f2ad5500       -mov ax, word ptr [edx + eax + 0x55adf2]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(5615090) /* 0x55adf2 */ + cpu.eax * 1);
    // 0046912d  66898382000000         -mov word ptr [ebx + 0x82], ax
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(130) /* 0x82 */) = cpu.ax;
    // 00469134  e986000000             -jmp 0x4691bf
    goto L_0x004691bf;
L_0x00469139:
    // 00469139  8b980caf5500           -mov ebx, dword ptr [eax + 0x55af0c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(5615372) /* 0x55af0c */);
    // 0046913f  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 00469141  7424                   -je 0x469167
    if (cpu.flags.zf)
    {
        goto L_0x00469167;
    }
    // 00469143  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 00469145  e8f6cc0600             -call 0x4d5e40
    cpu.esp -= 4;
    nfs3hp::port::fields::sliderShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046914a  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0046914c  8b4140                 -mov eax, dword ptr [ecx + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 0046914f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00469152  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00469155  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 00469158  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0046915b  29f8                   +sub eax, edi
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046915d  66899cc2b28a6700       -mov word ptr [edx + eax*8 + 0x678ab2], bx
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(6785714) /* 0x678ab2 */ + cpu.eax * 8) = cpu.bx;
    // 00469165  eb09                   -jmp 0x469170
    goto L_0x00469170;
L_0x00469167:
    // 00469167  66c780b28a6700ffff     -mov word ptr [eax + 0x678ab2], 0xffff
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6785714) /* 0x678ab2 */) = 65535 /*0xffff*/;
L_0x00469170:
    // 00469170  8b5140                 -mov edx, dword ptr [ecx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 00469173  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00469176  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 0046917d  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046917f  8d3cb500000000         -lea edi, [esi*4]
    cpu.edi = x86::reg32(cpu.esi * 4);
    // 00469186  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 00469189  8d1438                 -lea edx, [eax + edi]
    cpu.edx = x86::reg32(cpu.eax + cpu.edi * 1);
    // 0046918c  8b829caf5500           -mov eax, dword ptr [edx + 0x55af9c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(5615516) /* 0x55af9c */);
    // 00469192  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 00469194  7420                   -je 0x4691b6
    if (cpu.flags.zf)
    {
        goto L_0x004691b6;
    }
    // 00469196  e8a5cc0600             -call 0x4d5e40
    cpu.esp -= 4;
    nfs3hp::port::fields::sliderShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046919b  8b5140                 -mov edx, dword ptr [ecx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 0046919e  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004691a1  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004691a3  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004691aa  29d0                   +sub eax, edx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004691ac  66899cc7b08a6700       -mov word ptr [edi + eax*8 + 0x678ab0], bx
    app->getMemory<x86::reg16>(cpu.edi + x86::reg32(6785712) /* 0x678ab0 */ + cpu.eax * 8) = cpu.bx;
    // 004691b4  eb09                   -jmp 0x4691bf
    goto L_0x004691bf;
L_0x004691b6:
    // 004691b6  66c782b08a6700ffff     -mov word ptr [edx + 0x678ab0], 0xffff
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(6785712) /* 0x678ab0 */) = 65535 /*0xffff*/;
L_0x004691bf:
    // 004691bf  8b5140                 -mov edx, dword ptr [ecx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 004691c2  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 004691c5  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004691cc  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004691ce  8d14c500000000         -lea edx, [eax*8]
    cpu.edx = x86::reg32(cpu.eax * 8);
    // 004691d5  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 004691dc  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004691de  6683b8b28a6700ff       +cmp word ptr [eax + 0x678ab2], -1
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6785714) /* 0x678ab2 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(-1 /*-0x1*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004691e6  7420                   -je 0x469208
    if (cpu.flags.zf)
    {
        goto L_0x00469208;
    }
    // 004691e8  8d1cf500000000         -lea ebx, [esi*8]
    cpu.ebx = x86::reg32(cpu.esi * 8);
    // 004691ef  8d517c                 -lea edx, [ecx + 0x7c]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(124) /* 0x7c */);
    // 004691f2  01da                   -add edx, ebx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004691f4  8b80b08a6700           -mov eax, dword ptr [eax + 0x678ab0]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6785712) /* 0x678ab0 */);
    // 004691fa  8d5a06                 -lea ebx, [edx + 6]
    cpu.ebx = x86::reg32(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 004691fd  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00469200  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 00469203  e8e8d40600             -call 0x4d66f0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeSize(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x00469208:
    // 00469208  8d04f500000000         -lea eax, [esi*8]
    cpu.eax = x86::reg32(cpu.esi * 8);
    // 0046920f  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 00469211  6683b88200000000       +cmp word ptr [eax + 0x82], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(130) /* 0x82 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00469219  7e0e                   -jle 0x469229
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x00469229;
    }
    // 0046921b  6683b88000000000       +cmp word ptr [eax + 0x80], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(128) /* 0x80 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 00469223  0f8f88000000           -jg 0x4692b1
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004692b1;
    }
L_0x00469229:
    // 00469229  8b5140                 -mov edx, dword ptr [ecx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 0046922c  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046922f  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00469236  8d1cb500000000         -lea ebx, [esi*4]
    cpu.ebx = x86::reg32(cpu.esi * 4);
    // 0046923d  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046923f  bfffffffff             -mov edi, 0xffffffff
    cpu.edi = 4294967295 /*0xffffffff*/;
    // 00469244  6689bcc3b28a6700       -mov word ptr [ebx + eax*8 + 0x678ab2], di
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(6785714) /* 0x678ab2 */ + cpu.eax * 8) = cpu.di;
    // 0046924c  8b5140                 -mov edx, dword ptr [ecx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 0046924f  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00469252  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00469259  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046925b  6689bcc3b08a6700       -mov word ptr [ebx + eax*8 + 0x678ab0], di
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(6785712) /* 0x678ab0 */ + cpu.eax * 8) = cpu.di;
    // 00469263  8b5140                 -mov edx, dword ptr [ecx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 00469266  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 00469269  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 00469270  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 00469272  8d14f500000000         -lea edx, [esi*8]
    cpu.edx = x86::reg32(cpu.esi * 8);
    // 00469279  c1e004                 -shl eax, 4
    cpu.eax <<= 4 /*0x4*/ % 32;
    // 0046927c  8d1c11                 -lea ebx, [ecx + edx]
    cpu.ebx = x86::reg32(cpu.ecx + cpu.edx * 1);
    // 0046927f  668b8402f0ad5500       -mov ax, word ptr [edx + eax + 0x55adf0]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(5615088) /* 0x55adf0 */ + cpu.eax * 1);
    // 00469287  66898380000000         -mov word ptr [ebx + 0x80], ax
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(128) /* 0x80 */) = cpu.ax;
    // 0046928e  8b4140                 -mov eax, dword ptr [ecx + 0x40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 00469291  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 00469294  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 00469297  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046929a  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 0046929d  29f8                   -sub eax, edi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edi));
    // 0046929f  c1e004                 +shl eax, 4
    {
        x86::reg8 tmp = 4 /*0x4*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004692a2  668b8402f2ad5500       -mov ax, word ptr [edx + eax + 0x55adf2]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(5615090) /* 0x55adf2 */ + cpu.eax * 1);
    // 004692aa  66898382000000         -mov word ptr [ebx + 0x82], ax
    app->getMemory<x86::reg16>(cpu.ebx + x86::reg32(130) /* 0x82 */) = cpu.ax;
L_0x004692b1:
    // 004692b1  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004692b2  e9d8fdffff             -jmp 0x46908f
    goto L_0x0046908f;
L_0x004692b7:
    // 004692b7  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004692b9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004692ba  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004692bb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004692bc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004692bd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004692be  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004692bf  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_469080: native, nfs3hp::port::fields::sliderPartSizes */

/* sub_4692c0: native, nfs3hp::port::fields::sliderPartUnderPointer */

/* sub_469340: native, nfs3hp::port::fields::sliderThumbPlace */

/* sub_4693c0: native, nfs3hp::port::fields::snapToStep */

/* sub_469480: native, nfs3hp::port::fields::setSliderValue */

/* sub_469520: native, nfs3hp::frontend::sliderStoredValue */

/* sub_469530: native, nfs3hp::port::fields::setSliderStep */

/* sub_469580: native, nfs3hp::port::view::recordFloat */

/* sub_469590: native, nfs3hp::port::fields::dragSliderTo */

/* sub_469680: native, nfs3hp::port::fields::drawSlider */

/* sub_46a5f0: native, nfs3hp::port::fields::trackSlider */

/* sub_46a860: native, nfs3hp::port::fields::sliderKey */

/* sub_46a980: native, nfs3hp::port::fields::placeSliderParts */

/* sub_46ab10: native, nfs3hp::port::fields::setUpSlider */

/* sub_46abb0: native, nfs3hp::port::fields::sliderBox */

/* sub_46ac40: native, nfs3hp::port::fields::twoAxisThumbPlace */

/* sub_46add0: native, nfs3hp::port::fields::setTwoAxisValue */

/* sub_46afd0: native, nfs3hp::frontend::twoAxisValue */

/* sub_46afe0: native, nfs3hp::port::fields::setTwoAxisStep */

/* sub_46b030: native, nfs3hp::port::view::recordFloatAt */

/* sub_46b040: native, nfs3hp::port::fields::dragTwoAxisTo */

/* sub_46b3a0: native, nfs3hp::port::fields::drawTwoAxis */

/* sub_46c320: native, nfs3hp::port::fields::trackTwoAxis */

/* sub_46c4a0: native, nfs3hp::port::fields::twoAxisKey */

/* sub_46c640: native, nfs3hp::port::fields::placeTwoAxisParts */

/* sub_46c820: native, nfs3hp::port::fields::setUpTwoAxis */

/* sub_46c8b0: native, nfs3hp::port::fields::twoAxisBox */

/* sub_46c950: native, nfs3hp::port::fields::sliderSetValue */

/* sub_46c9b0: native, nfs3hp::frontend::sliderValue */

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_46ca10(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0046ca10  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046ca11  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046ca12  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046ca14  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0046ca16  668b5042               -mov dx, word ptr [eax + 0x42]
    cpu.dx = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(66) /* 0x42 */);
    // 0046ca1a  6683fa05               +cmp dx, 5
    {
        x86::reg16 tmp1 = cpu.dx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(5 /*0x5*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046ca1e  771e                   -ja 0x46ca3e
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0046ca3e;
    }
    // 0046ca20  81e2ffff0000           +and edx, 0xffff
    cpu.clear_co();
    cpu.set_szp((cpu.edx &= x86::reg32(x86::sreg32(65535 /*0xffff*/))));
    // 0046ca26  ff2495f4c94600         -jmp dword ptr [edx*4 + 0x46c9f4]
    cpu.ip = app->getMemory<x86::reg32>(4639220 + cpu.edx * 4); goto dynamic_jump;
  case 0x0046ca2d:
    // 0046ca2d  8b5510                 -mov edx, dword ptr [ebp + 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 0046ca30  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046ca31  ff750c                 -push dword ptr [ebp + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 0046ca34  e8a7e5ffff             -call 0x46afe0
    cpu.esp -= 4;
    nfs3hp::port::fields::setTwoAxisStep(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ca39  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046ca3a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046ca3b  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
  case 0x0046ca3e:
L_0x0046ca3e:
    // 0046ca3e  6a00                   -push 0
    app->getMemory<x86::reg32>(cpu.esp-4) = 0 /*0x0*/;
    cpu.esp -= 4;
    // 0046ca40  ff750c                 -push dword ptr [ebp + 0xc]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(12) /* 0xc */);
    cpu.esp -= 4;
    // 0046ca43  e8e8caffff             -call 0x469530
    cpu.esp -= 4;
    nfs3hp::port::fields::setSliderStep(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
  [[fallthrough]];
  case 0x0046ca48:
    // 0046ca48  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046ca49  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046ca4a  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* sub_46ca70: native, nfs3hp::port::view::recordFloatByKind */

/* sub_46cac0: native, nfs3hp::frontend::sliderSetOnMove */

/* sub_46cae0: native, nfs3hp::port::fields::sliderDraw */

/* sub_46cb80: native, nfs3hp::port::fields::sliderWidgetKey */

/* sub_46cbe0: native, nfs3hp::port::fields::sliderWidgetLayout */

/* sub_46cc10: native, nfs3hp::port::fields::sliderWidgetHidden */

/* sub_46cc50: native, nfs3hp::port::fields::sliderWidgetShown */

/* sub_46cca0: native, nfs3hp::frontend::sliderSetColour */

/* align: skip 0x00 0x00 0x00 0x00 0x00 */
/* data blob: e300c800280029000500030005000500c800ef00f000030005000500c800f70095010300050005008d80000000008bd2 */
void Application::sub_46cce0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046cce0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046cce1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046cce2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046cce3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046cce4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046cce5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046cce6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046cce8  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0046cceb  b899010000             -mov eax, 0x199
    cpu.eax = 409 /*0x199*/;
    // 0046ccf0  ba12000000             -mov edx, 0x12
    cpu.edx = 18 /*0x12*/;
    // 0046ccf5  e8564b0600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ccfa  e82153feff             -call 0x452020
    cpu.esp -= 4;
    nfs3hp::port::text::textWidth(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ccff  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0046cd01  8b451a                 -mov eax, dword ptr [ebp + 0x1a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(26) /* 0x1a */);
    // 0046cd04  8b551e                 -mov edx, dword ptr [ebp + 0x1e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(30) /* 0x1e */);
    // 0046cd07  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046cd0a  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046cd0d  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 0046cd10  8d58e2                 -lea ebx, [eax - 0x1e]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(-30) /* -0x1e */);
    // 0046cd13  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046cd15  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046cd18  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046cd1a  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046cd1c  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046cd21  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0046cd24  8b451c                 -mov eax, dword ptr [ebp + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 0046cd27  68fd9d64ff             -push 0xff649dfd
    app->getMemory<x86::reg32>(cpu.esp-4) = 4284784125 /*0xff649dfd*/;
    cpu.esp -= 4;
    // 0046cd2c  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046cd2f  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046cd32  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0046cd35  8d3c10                 -lea edi, [eax + edx]
    cpu.edi = x86::reg32(cpu.eax + cpu.edx * 1);
    // 0046cd38  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046cd3a  b899010000             -mov eax, 0x199
    cpu.eax = 409 /*0x199*/;
    // 0046cd3f  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 0046cd41  e80a4b0600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046cd46  e84554feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046cd4b  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046cd4e  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046cd51  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046cd54  83e806                 -sub eax, 6
    (cpu.eax) -= x86::reg32(x86::sreg32(6 /*0x6*/));
    // 0046cd57  83eb0e                 -sub ebx, 0xe
    (cpu.ebx) -= x86::reg32(x86::sreg32(14 /*0xe*/));
    // 0046cd5a  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046cd5c  8b4524                 -mov eax, dword ptr [ebp + 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(36) /* 0x24 */);
    // 0046cd5f  e82cac0600             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046cd64  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046cd67  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0046cd69  83e80d                 -sub eax, 0xd
    (cpu.eax) -= x86::reg32(x86::sreg32(13 /*0xd*/));
    // 0046cd6c  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046cd6f  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0046cd72  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0046cd74  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046cd76  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046cd78  68a9785fff             -push 0xff5f78a9
    app->getMemory<x86::reg32>(cpu.esp-4) = 4284446889 /*0xff5f78a9*/;
    cpu.esp -= 4;
    // 0046cd7d  68a9785fff             -push 0xff5f78a9
    app->getMemory<x86::reg32>(cpu.esp-4) = 4284446889 /*0xff5f78a9*/;
    cpu.esp -= 4;
    // 0046cd82  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046cd85  8d3407                 -lea esi, [edi + eax]
    cpu.esi = x86::reg32(cpu.edi + cpu.eax * 1);
    // 0046cd88  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 0046cd8a  29c7                   -sub edi, eax
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.eax));
    // 0046cd8c  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0046cd8e  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0046cd90  e83bc10600             -call 0x4d8ed0
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawLine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046cd95  68bd8c73ff             -push 0xff738cbd
    app->getMemory<x86::reg32>(cpu.esp-4) = 4285762749 /*0xff738cbd*/;
    cpu.esp -= 4;
    // 0046cd9a  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046cd9d  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0046cd9f  68bd8c73ff             -push 0xff738cbd
    app->getMemory<x86::reg32>(cpu.esp-4) = 4285762749 /*0xff738cbd*/;
    cpu.esp -= 4;
    // 0046cda4  83ea0c                 -sub edx, 0xc
    (cpu.edx) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0046cda7  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0046cda9  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0046cdab  e820c10600             -call 0x4d8ed0
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawLine(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046cdb0  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0046cdb2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cdb3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cdb4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cdb5  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cdb6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cdb7  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cdb8  c20c00                 -ret 0xc
    cpu.esp += 4+12 /*0xc*/;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_46cdc0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046cdc0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046cdc1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046cdc2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046cdc3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046cdc5  8b507e                 -mov edx, dword ptr [eax + 0x7e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046cdc8  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046cdcb  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046cdcc  8b9084000000           -mov edx, dword ptr [eax + 0x84]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(132) /* 0x84 */);
    // 0046cdd2  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046cdd5  8b4cd04c               -mov ecx, dword ptr [eax + edx*8 + 0x4c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(76) /* 0x4c */ + cpu.edx * 8);
    // 0046cdd9  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046cdda  8b5cd048               -mov ebx, dword ptr [eax + edx*8 + 0x48]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(72) /* 0x48 */ + cpu.edx * 8);
    // 0046cdde  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046cddf  e8fcfeffff             -call 0x46cce0
    cpu.esp -= 4;
    sub_46cce0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046cde4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cde5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cde6  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cde7  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_46cdf0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046cdf0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046cdf1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046cdf2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046cdf3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046cdf4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046cdf5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046cdf6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046cdf8  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 0046cdfb  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0046cdfd  668b563c               -mov dx, word ptr [esi + 0x3c]
    cpu.dx = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(60) /* 0x3c */);
    // 0046ce01  668b808a000000         -mov ax, word ptr [eax + 0x8a]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(138) /* 0x8a */);
    // 0046ce08  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046ce0a  668b563e               -mov dx, word ptr [esi + 0x3e]
    cpu.dx = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(62) /* 0x3e */);
    // 0046ce0e  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046ce10  668955fa               -mov word ptr [ebp - 6], dx
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-6) /* -0x6 */) = cpu.dx;
    // 0046ce14  668b4608               -mov ax, word ptr [esi + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 0046ce18  668b5e44               -mov bx, word ptr [esi + 0x44]
    cpu.bx = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(68) /* 0x44 */);
    // 0046ce1c  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0046ce1e  668945f8               -mov word ptr [ebp - 8], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ax;
    // 0046ce22  668b4640               -mov ax, word ptr [esi + 0x40]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 0046ce26  40                     -inc eax
    (cpu.eax)++;
    // 0046ce27  668945fc               -mov word ptr [ebp - 4], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ax;
    // 0046ce2b  668b4644               -mov ax, word ptr [esi + 0x44]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(68) /* 0x44 */);
    // 0046ce2f  40                     -inc eax
    (cpu.eax)++;
    // 0046ce30  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0046ce32  668945fe               -mov word ptr [ebp - 2], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-2) /* -0x2 */) = cpu.ax;
L_0x0046ce36:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046ce36  8b4644                 -mov eax, dword ptr [esi + 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(68) /* 0x44 */);
    // 0046ce39  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ce3c  39c2                   +cmp edx, eax
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
    // 0046ce3e  7d47                   -jge 0x46ce87
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046ce87;
    }
    // 0046ce40  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046ce43  e8a8f30200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ce48  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046ce4a  742d                   -je 0x46ce79
    if (cpu.flags.zf)
    {
        goto L_0x0046ce79;
    }
    // 0046ce4c  8b8682000000           -mov eax, dword ptr [esi + 0x82]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(130) /* 0x82 */);
    // 0046ce52  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0046ce54  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ce57  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0046ce59  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046ce5b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0046ce5d  e86e8efeff             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ce62  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046ce64  7421                   -je 0x46ce87
    if (cpu.flags.zf)
    {
        goto L_0x0046ce87;
    }
    // 0046ce66  83b8c402000004         +cmp dword ptr [eax + 0x2c4], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(708) /* 0x2c4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ce6d  7418                   -je 0x46ce87
    if (cpu.flags.zf)
    {
        goto L_0x0046ce87;
    }
    // 0046ce6f  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046ce74  e9a8000000             -jmp 0x46cf21
    goto L_0x0046cf21;
L_0x0046ce79:
    // 0046ce79  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046ce7c  66034e44               +add cx, word ptr [esi + 0x44]
    {
        x86::reg16& tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(cpu.esi + x86::reg32(68) /* 0x44 */)));
        x86::reg16 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) == (1 & (tmp2 >> 15));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046ce80  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0046ce81  66894df8               -mov word ptr [ebp - 8], cx
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.cx;
    // 0046ce85  ebaf                   -jmp 0x46ce36
    goto L_0x0046ce36;
L_0x0046ce87:
    // 0046ce87  833db8d36f0003         +cmp dword ptr [0x6fd3b8], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ce8e  0f848b000000           -je 0x46cf1f
    if (cpu.flags.zf)
    {
        goto L_0x0046cf1f;
    }
    // 0046ce94  668b7e3c               -mov di, word ptr [esi + 0x3c]
    cpu.di = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(60) /* 0x3c */);
    // 0046ce98  668b868a000000         -mov ax, word ptr [esi + 0x8a]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(138) /* 0x8a */);
    // 0046ce9f  668b563e               -mov dx, word ptr [esi + 0x3e]
    cpu.dx = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(62) /* 0x3e */);
    // 0046cea3  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 0046cea5  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046cea7  668b5640               -mov dx, word ptr [esi + 0x40]
    cpu.dx = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(64) /* 0x40 */);
    // 0046ceab  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046cead  668955fa               -mov word ptr [ebp - 6], dx
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-6) /* -0x6 */) = cpu.dx;
    // 0046ceb1  668b4608               -mov ax, word ptr [esi + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 0046ceb5  668b5e44               -mov bx, word ptr [esi + 0x44]
    cpu.bx = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(68) /* 0x44 */);
    // 0046ceb9  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0046cebb  668945f8               -mov word ptr [ebp - 8], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ax;
    // 0046cebf  668b4642               -mov ax, word ptr [esi + 0x42]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(66) /* 0x42 */);
    // 0046cec3  40                     -inc eax
    (cpu.eax)++;
    // 0046cec4  668945fc               -mov word ptr [ebp - 4], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ax;
    // 0046cec8  668b4644               -mov ax, word ptr [esi + 0x44]
    cpu.ax = app->getMemory<x86::reg16>(cpu.esi + x86::reg32(68) /* 0x44 */);
    // 0046cecc  40                     -inc eax
    (cpu.eax)++;
    // 0046cecd  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0046cecf  668945fe               -mov word ptr [ebp - 2], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-2) /* -0x2 */) = cpu.ax;
L_0x0046ced3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046ced3  8b4644                 -mov eax, dword ptr [esi + 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(68) /* 0x44 */);
    // 0046ced6  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ced9  39c2                   +cmp edx, eax
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
    // 0046cedb  7d42                   -jge 0x46cf1f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046cf1f;
    }
    // 0046cedd  8d45f8                 -lea eax, [ebp - 8]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046cee0  e80bf30200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046cee5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046cee7  7428                   -je 0x46cf11
    if (cpu.flags.zf)
    {
        goto L_0x0046cf11;
    }
    // 0046cee9  8b8682000000           -mov eax, dword ptr [esi + 0x82]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(130) /* 0x82 */);
    // 0046ceef  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0046cef1  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046cef4  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0046cef6  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046cef8  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0046cefa  e8d18dfeff             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ceff  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046cf01  741c                   -je 0x46cf1f
    if (cpu.flags.zf)
    {
        goto L_0x0046cf1f;
    }
    // 0046cf03  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 0046cf08  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0046cf0a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cf0b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cf0c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cf0d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cf0e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cf0f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cf10  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x0046cf11:
    // 0046cf11  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046cf14  66034e44               +add cx, word ptr [esi + 0x44]
    {
        x86::reg16& tmp1 = cpu.cx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(cpu.esi + x86::reg32(68) /* 0x44 */)));
        x86::reg16 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) == (1 & (tmp2 >> 15));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046cf18  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0046cf19  66894df8               -mov word ptr [ebp - 8], cx
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.cx;
    // 0046cf1d  ebb4                   -jmp 0x46ced3
    goto L_0x0046ced3;
L_0x0046cf1f:
    // 0046cf1f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0046cf21:
    // 0046cf21  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0046cf23  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cf24  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cf25  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cf26  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cf27  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cf28  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046cf29  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_46cf30: native, nfs3hp::port::view::metricPopup */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_46d180(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046d180  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046d181  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046d182  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046d183  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046d184  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046d185  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046d186  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046d188  81ec14010000           -sub esp, 0x114
    (cpu.esp) -= x86::reg32(x86::sreg32(276 /*0x114*/));
    // 0046d18e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0046d190  668b5a3c               -mov bx, word ptr [edx + 0x3c]
    cpu.bx = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(60) /* 0x3c */);
    // 0046d194  668b808a000000         -mov ax, word ptr [eax + 0x8a]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(138) /* 0x8a */);
    // 0046d19b  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0046d19d  668b5a3e               -mov bx, word ptr [edx + 0x3e]
    cpu.bx = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(62) /* 0x3e */);
    // 0046d1a1  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046d1a3  66895dee               -mov word ptr [ebp - 0x12], bx
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-18) /* -0x12 */) = cpu.bx;
    // 0046d1a7  668b4208               -mov ax, word ptr [edx + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 0046d1ab  668b4a44               -mov cx, word ptr [edx + 0x44]
    cpu.cx = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(68) /* 0x44 */);
    // 0046d1af  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0046d1b1  668945ec               -mov word ptr [ebp - 0x14], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.ax;
    // 0046d1b5  668b4240               -mov ax, word ptr [edx + 0x40]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(64) /* 0x40 */);
    // 0046d1b9  40                     -inc eax
    (cpu.eax)++;
    // 0046d1ba  668945f0               -mov word ptr [ebp - 0x10], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ax;
    // 0046d1be  668b4244               -mov ax, word ptr [edx + 0x44]
    cpu.ax = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(68) /* 0x44 */);
    // 0046d1c2  40                     -inc eax
    (cpu.eax)++;
    // 0046d1c3  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0046d1c5  668945f2               -mov word ptr [ebp - 0xe], ax
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-14) /* -0xe */) = cpu.ax;
L_0x0046d1c9:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046d1c9  8b4244                 -mov eax, dword ptr [edx + 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(68) /* 0x44 */);
    // 0046d1cc  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d1cf  39c1                   +cmp ecx, eax
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
    // 0046d1d1  0f8dc5010000           -jge 0x46d39c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046d39c;
    }
    // 0046d1d7  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046d1da  e811f00200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d1df  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046d1e1  0f84a4010000           -je 0x46d38b
    if (cpu.flags.zf)
    {
        goto L_0x0046d38b;
    }
    // 0046d1e7  8b8282000000           -mov eax, dword ptr [edx + 0x82]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(130) /* 0x82 */);
    // 0046d1ed  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d1f0  8d1401                 -lea edx, [ecx + eax]
    cpu.edx = x86::reg32(cpu.ecx + cpu.eax * 1);
    // 0046d1f3  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0046d1f5  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0046d1f7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0046d1f9  e8d28afeff             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d1fe  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 0046d201  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046d203  0f8493010000           -je 0x46d39c
    if (cpu.flags.zf)
    {
        goto L_0x0046d39c;
    }
    // 0046d209  83b8c402000004         +cmp dword ptr [eax + 0x2c4], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(708) /* 0x2c4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d210  0f8486010000           -je 0x46d39c
    if (cpu.flags.zf)
    {
        goto L_0x0046d39c;
    }
    // 0046d216  8b15b4d46f00           -mov edx, dword ptr [0x6fd4b4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328948) /* 0x6fd4b4 */);
    // 0046d21c  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 0046d21f  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0046d221  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 0046d224  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0046d226  89d1                   -mov ecx, edx
    cpu.ecx = cpu.edx;
    // 0046d228  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 0046d22b  8b8040020000           -mov eax, dword ptr [eax + 0x240]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(576) /* 0x240 */);
    // 0046d231  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 0046d233  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0046d235  8d4a20                 -lea ecx, [edx + 0x20]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(32) /* 0x20 */);
    // 0046d238  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
    // 0046d23b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0046d23d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046d23d  3b55fc                 +cmp edx, dword ptr [ebp - 4]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d240  7d1e                   -jge 0x46d260
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046d260;
    }
    // 0046d242  8b7df4                 -mov edi, dword ptr [ebp - 0xc]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046d245  8d1c9500000000         -lea ebx, [edx*4]
    cpu.ebx = x86::reg32(cpu.edx * 4);
    // 0046d24c  01fb                   -add ebx, edi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edi));
    // 0046d24e  8bb340020000           -mov esi, dword ptr [ebx + 0x240]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(576) /* 0x240 */);
    // 0046d254  39f0                   +cmp eax, esi
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
    // 0046d256  7e05                   -jle 0x46d25d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0046d25d;
    }
    // 0046d258  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0046d25a  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
L_0x0046d25d:
    // 0046d25d  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0046d25e  ebdd                   -jmp 0x46d23d
    goto L_0x0046d23d;
L_0x0046d260:
    // 0046d260  8b45ee                 -mov eax, dword ptr [ebp - 0x12]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-18) /* -0x12 */);
    // 0046d263  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d266  8d58fd                 -lea ebx, [eax - 3]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(-3) /* -0x3 */);
    // 0046d269  8b45ea                 -mov eax, dword ptr [ebp - 0x16]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-22) /* -0x16 */);
    // 0046d26c  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d26f  68222222d0             -push 0xd0222222
    app->getMemory<x86::reg32>(cpu.esp-4) = 3491897890 /*0xd0222222*/;
    cpu.esp -= 4;
    // 0046d274  8d5002                 -lea edx, [eax + 2]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(2) /* 0x2 */);
    // 0046d277  68222222d0             -push 0xd0222222
    app->getMemory<x86::reg32>(cpu.esp-4) = 3491897890 /*0xd0222222*/;
    cpu.esp -= 4;
    // 0046d27c  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046d27f  68400000a0             -push 0xa0000040
    app->getMemory<x86::reg32>(cpu.esp-4) = 2684354624 /*0xa0000040*/;
    cpu.esp -= 4;
    // 0046d284  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d287  68400000a0             -push 0xa0000040
    app->getMemory<x86::reg32>(cpu.esp-4) = 2684354624 /*0xa0000040*/;
    cpu.esp -= 4;
    // 0046d28c  83c002                 -add eax, 2
    (cpu.eax) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046d28f  e82cb80600             -call 0x4d8ac0
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawFieldBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d294  8b45ea                 -mov eax, dword ptr [ebp - 0x16]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-22) /* -0x16 */);
    // 0046d297  8b55ee                 -mov edx, dword ptr [ebp - 0x12]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-18) /* -0x12 */);
    // 0046d29a  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d29d  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d2a0  8d5808                 -lea ebx, [eax + 8]
    cpu.ebx = x86::reg32(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 0046d2a3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046d2a5  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046d2a8  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046d2aa  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046d2ac  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046d2af  6840e4ff00             -push 0xffe440
    app->getMemory<x86::reg32>(cpu.esp-4) = 16770112 /*0xffe440*/;
    cpu.esp -= 4;
    // 0046d2b4  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d2b7  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046d2b9  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046d2bb  b805000000             -mov eax, 5
    cpu.eax = 5 /*0x5*/;
    // 0046d2c0  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046d2c5  e886450600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d2ca  e8c14efeff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d2cf  8b45ea                 -mov eax, dword ptr [ebp - 0x16]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-22) /* -0x16 */);
    // 0046d2d2  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d2d5  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 0046d2d7  8d7818                 -lea edi, [eax + 0x18]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(24) /* 0x18 */);
L_0x0046d2da:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046d2da  3b75fc                 +cmp esi, dword ptr [ebp - 4]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d2dd  0f8db9000000           -jge 0x46d39c
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046d39c;
    }
    // 0046d2e3  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046d2e6  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 0046d2ed  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046d2ef  8d95ecfeffff           -lea edx, [ebp - 0x114]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-276) /* -0x114 */);
    // 0046d2f5  8b8040020000           -mov eax, dword ptr [eax + 0x240]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(576) /* 0x240 */);
    // 0046d2fb  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046d2fe  e83d1b0600             -call 0x4cee40
    cpu.esp -= 4;
    sub_4cee40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d303  39ce                   +cmp esi, ecx
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
    // 0046d305  7507                   -jne 0x46d30e
    if (!cpu.flags.zf)
    {
        goto L_0x0046d30e;
    }
    // 0046d307  b80000ffff             -mov eax, 0xffff0000
    cpu.eax = 4294901760 /*0xffff0000*/;
    // 0046d30c  eb05                   -jmp 0x46d313
    goto L_0x0046d313;
L_0x0046d30e:
    // 0046d30e  b8ff7f00ff             -mov eax, 0xff007fff
    cpu.eax = 4278222847 /*0xff007fff*/;
L_0x0046d313:
    // 0046d313  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046d316  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046d317  8b45ee                 -mov eax, dword ptr [ebp - 0x12]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-18) /* -0x12 */);
    // 0046d31a  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d31d  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d320  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046d325  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046d327  6a04                   -push 4
    app->getMemory<x86::reg32>(cpu.esp-4) = 4 /*0x4*/;
    cpu.esp -= 4;
    // 0046d329  8d50f8                 -lea edx, [eax - 8]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(-8) /* -0x8 */);
    // 0046d32c  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0046d32e  8d85ecfeffff           -lea eax, [ebp - 0x114]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-276) /* -0x114 */);
    // 0046d334  e8574efeff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d339  8d4601                 -lea eax, [esi + 1]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0046d33c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046d33d  68b4ae5300             -push 0x53aeb4
    app->getMemory<x86::reg32>(cpu.esp-4) = 5484212 /*0x53aeb4*/;
    cpu.esp -= 4;
    // 0046d342  8d85ecfeffff           -lea eax, [ebp - 0x114]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-276) /* -0x114 */);
    // 0046d348  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046d349  8b5df8                 -mov ebx, dword ptr [ebp - 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046d34c  e83f230700             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d351  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0046d354  39de                   +cmp esi, ebx
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d356  7507                   -jne 0x46d35f
    if (!cpu.flags.zf)
    {
        goto L_0x0046d35f;
    }
    // 0046d358  b80000ffff             -mov eax, 0xffff0000
    cpu.eax = 4294901760 /*0xffff0000*/;
    // 0046d35d  eb05                   -jmp 0x46d364
    goto L_0x0046d364;
L_0x0046d35f:
    // 0046d35f  b8ff7f00ff             -mov eax, 0xff007fff
    cpu.eax = 4278222847 /*0xff007fff*/;
L_0x0046d364:
    // 0046d364  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046d369  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046d36a  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046d36d  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0046d36f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d372  46                     -inc esi
    (cpu.esi)++;
    // 0046d373  8d5008                 -lea edx, [eax + 8]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 0046d376  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0046d378  8d85ecfeffff           -lea eax, [ebp - 0x114]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-276) /* -0x114 */);
    // 0046d37e  83c70f                 +add edi, 0xf
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(15 /*0xf*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046d381  e80a4efeff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d386  e94fffffff             -jmp 0x46d2da
    goto L_0x0046d2da;
L_0x0046d38b:
    // 0046d38b  8b75ec                 -mov esi, dword ptr [ebp - 0x14]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046d38e  66037244               +add si, word ptr [edx + 0x44]
    {
        x86::reg16& tmp1 = cpu.si;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(cpu.edx + x86::reg32(68) /* 0x44 */)));
        x86::reg16 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) == (1 & (tmp2 >> 15));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046d392  41                     +inc ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0046d393  668975ec               -mov word ptr [ebp - 0x14], si
    app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.si;
    // 0046d397  e92dfeffff             -jmp 0x46d1c9
    goto L_0x0046d1c9;
L_0x0046d39c:
    // 0046d39c  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0046d39e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046d39f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046d3a0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046d3a1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046d3a2  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046d3a3  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046d3a4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_46d3c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 0046d3c0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046d3c1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046d3c2  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046d3c3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046d3c4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046d3c5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046d3c7  81ec74010000           -sub esp, 0x174
    (cpu.esp) -= x86::reg32(x86::sreg32(372 /*0x174*/));
    // 0046d3cd  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0046d3cf  8955bc                 -mov dword ptr [ebp - 0x44], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */) = cpu.edx;
    // 0046d3d2  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0046d3d4  8955c0                 -mov dword ptr [ebp - 0x40], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = cpu.edx;
    // 0046d3d7  ba17000000             -mov edx, 0x17
    cpu.edx = 23 /*0x17*/;
    // 0046d3dc  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0046d3df  e87cd2fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d3e4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046d3e6  0f84950b0000           -je 0x46df81
    if (cpu.flags.zf)
    {
        goto L_0x0046df81;
    }
    // 0046d3ec  6683b98800000002       +cmp word ptr [ecx + 0x88], 2
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(136) /* 0x88 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2 /*0x2*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046d3f4  752a                   -jne 0x46d420
    if (!cpu.flags.zf)
    {
        goto L_0x0046d420;
    }
    // 0046d3f6  668b998a000000         -mov bx, word ptr [ecx + 0x8a]
    cpu.bx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(138) /* 0x8a */);
    // 0046d3fd  83c314                 -add ebx, 0x14
    (cpu.ebx) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0046d400  6689998a000000         -mov word ptr [ecx + 0x8a], bx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(138) /* 0x8a */) = cpu.bx;
    // 0046d407  6681fba802             +cmp bx, 0x2a8
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(680 /*0x2a8*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046d40c  7c12                   -jl 0x46d420
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0046d420;
    }
    // 0046d40e  66c7818a000000a802     -mov word ptr [ecx + 0x8a], 0x2a8
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(138) /* 0x8a */) = 680 /*0x2a8*/;
    // 0046d417  66c781880000000300     -mov word ptr [ecx + 0x88], 3
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(136) /* 0x88 */) = 3 /*0x3*/;
L_0x0046d420:
    // 0046d420  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d423  6683b88800000001       +cmp word ptr [eax + 0x88], 1
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(136) /* 0x88 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(1 /*0x1*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046d42b  7538                   -jne 0x46d465
    if (!cpu.flags.zf)
    {
        goto L_0x0046d465;
    }
    // 0046d42d  6683a88a00000014       -sub word ptr [eax + 0x8a], 0x14
    (app->getMemory<x86::reg16>(cpu.eax + x86::reg32(138) /* 0x8a */)) -= x86::reg16(x86::sreg16(20 /*0x14*/));
    // 0046d435  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d438  8b8088000000           -mov eax, dword ptr [eax + 0x88]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(136) /* 0x88 */);
    // 0046d43e  8b5204                 -mov edx, dword ptr [edx + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 0046d441  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d444  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d447  39d0                   +cmp eax, edx
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
    // 0046d449  7f1a                   -jg 0x46d465
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x0046d465;
    }
    // 0046d44b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d44e  66c780880000000000     -mov word ptr [eax + 0x88], 0
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(136) /* 0x88 */) = 0 /*0x0*/;
    // 0046d457  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d45a  668b4006               -mov ax, word ptr [eax + 6]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */);
    // 0046d45e  6689828a000000         -mov word ptr [edx + 0x8a], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(138) /* 0x8a */) = cpu.ax;
L_0x0046d465:
    // 0046d465  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d468  6683b88800000003       +cmp word ptr [eax + 0x88], 3
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(136) /* 0x88 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(3 /*0x3*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046d470  0f840b0b0000           -je 0x46df81
    if (cpu.flags.zf)
    {
        goto L_0x0046df81;
    }
    // 0046d476  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 0046d478  e8530c0000             -call 0x46e0d0
    cpu.esp -= 4;
    sub_46e0d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d47d  e86ec9fdff             -call 0x449df0
    cpu.esp -= 4;
    nfs3hp::port::loop::keysTaken(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d482  8945dc                 -mov dword ptr [ebp - 0x24], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.eax;
    // 0046d485  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d488  668b8086000000         -mov ax, word ptr [eax + 0x86]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(134) /* 0x86 */);
    // 0046d48f  663d0300               +cmp ax, 3
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(3 /*0x3*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046d493  7744                   -ja 0x46d4d9
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x0046d4d9;
    }
    // 0046d495  25ffff0000             +and eax, 0xffff
    cpu.clear_co();
    cpu.set_szp((cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/))));
    // 0046d49a  ff2485a8d34600         -jmp dword ptr [eax*4 + 0x46d3a8]
    cpu.ip = app->getMemory<x86::reg32>(4641704 + cpu.eax * 4); goto dynamic_jump;
  case 0x0046d4a1:
    // 0046d4a1  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0046d4a6  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0046d4a8  eb28                   -jmp 0x46d4d2
    goto L_0x0046d4d2;
  case 0x0046d4aa:
    // 0046d4aa  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 0046d4af  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 0046d4b1  eb1f                   -jmp 0x46d4d2
    goto L_0x0046d4d2;
  case 0x0046d4b3:
    // 0046d4b3  833db8d36f0003         +cmp dword ptr [0x6fd3b8], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d4ba  750c                   -jne 0x46d4c8
    if (!cpu.flags.zf)
    {
        goto L_0x0046d4c8;
    }
    // 0046d4bc  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0046d4c1  ba06000000             -mov edx, 6
    cpu.edx = 6 /*0x6*/;
    // 0046d4c6  eb0a                   -jmp 0x46d4d2
    goto L_0x0046d4d2;
L_0x0046d4c8:
    // 0046d4c8  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0046d4cd  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
L_0x0046d4d2:
    // 0046d4d2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0046d4d4  e83787feff             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046d4d9:
    // 0046d4d9  e8a21a0600             -call 0x4cef80
    cpu.esp -= 4;
    sub_4cef80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d4de  8b15307d6700           -mov edx, dword ptr [0x677d30]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */);
    // 0046d4e4  8945e0                 -mov dword ptr [ebp - 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.eax;
    // 0046d4e7  8955e4                 -mov dword ptr [ebp - 0x1c], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.edx;
    // 0046d4ea  83fa01                 +cmp edx, 1
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
    // 0046d4ed  7e0b                   -jle 0x46d4fa
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x0046d4fa;
    }
    // 0046d4ef  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046d4f1  7507                   -jne 0x46d4fa
    if (!cpu.flags.zf)
    {
        goto L_0x0046d4fa;
    }
    // 0046d4f3  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0046d4f8  eb02                   -jmp 0x46d4fc
    goto L_0x0046d4fc;
L_0x0046d4fa:
    // 0046d4fa  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
L_0x0046d4fc:
    // 0046d4fc  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d4ff  8b4044                 -mov eax, dword ptr [eax + 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(68) /* 0x44 */);
    // 0046d502  8b75e4                 -mov esi, dword ptr [ebp - 0x1c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 0046d505  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d508  894da0                 -mov dword ptr [ebp - 0x60], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-96) /* -0x60 */) = cpu.ecx;
    // 0046d50b  39f0                   +cmp eax, esi
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
    // 0046d50d  7c02                   -jl 0x46d511
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0046d511;
    }
    // 0046d50f  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
L_0x0046d511:
    // 0046d511  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d514  8b4a3a                 -mov ecx, dword ptr [edx + 0x3a]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(58) /* 0x3a */);
    // 0046d517  8b523c                 -mov edx, dword ptr [edx + 0x3c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(60) /* 0x3c */);
    // 0046d51a  c1f910                 -sar ecx, 0x10
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (16 /*0x10*/ % 32));
    // 0046d51d  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d520  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046d522  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d525  8b523e                 -mov edx, dword ptr [edx + 0x3e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(62) /* 0x3e */);
    // 0046d528  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d52b  01d1                   -add ecx, edx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046d52d  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d530  8b5240                 -mov edx, dword ptr [edx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(64) /* 0x40 */);
    // 0046d533  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d536  8d3c11                 -lea edi, [ecx + edx]
    cpu.edi = x86::reg32(cpu.ecx + cpu.edx * 1);
    // 0046d539  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d53c  8945b0                 -mov dword ptr [ebp - 0x50], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-80) /* -0x50 */) = cpu.eax;
    // 0046d53f  8b5242                 -mov edx, dword ptr [edx + 0x42]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(66) /* 0x42 */);
    // 0046d542  40                     -inc eax
    (cpu.eax)++;
    // 0046d543  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d546  0fafd0                 -imul edx, eax
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.eax)));
    // 0046d549  8b45bc                 -mov eax, dword ptr [ebp - 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
    // 0046d54c  8955d8                 -mov dword ptr [ebp - 0x28], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.edx;
    // 0046d54f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046d551  7411                   -je 0x46d564
    if (cpu.flags.zf)
    {
        goto L_0x0046d564;
    }
    // 0046d553  837de000               +cmp dword ptr [ebp - 0x20], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d557  750b                   -jne 0x46d564
    if (!cpu.flags.zf)
    {
        goto L_0x0046d564;
    }
    // 0046d559  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d55c  e88ff8ffff             -call 0x46cdf0
    cpu.esp -= 4;
    sub_46cdf0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d561  8945c0                 -mov dword ptr [ebp - 0x40], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = cpu.eax;
L_0x0046d564:
    // 0046d564  b9aa0000ff             -mov ecx, 0xff0000aa
    cpu.ecx = 4278190250 /*0xff0000aa*/;
    // 0046d569  bb2894d5ff             -mov ebx, 0xffd59428
    cpu.ebx = 4292187176 /*0xffd59428*/;
    // 0046d56e  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d571  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d574  8d7704                 -lea esi, [edi + 4]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(4) /* 0x4 */);
    // 0046d577  894dd0                 -mov dword ptr [ebp - 0x30], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = cpu.ecx;
    // 0046d57a  895dcc                 -mov dword ptr [ebp - 0x34], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.ebx;
    // 0046d57d  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046d57e  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046d581  8b8088000000           -mov eax, dword ptr [eax + 0x88]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(136) /* 0x88 */);
    // 0046d587  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046d588  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d58b  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d58e  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046d58f  83ea02                 -sub edx, 2
    (cpu.edx) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046d592  83e802                 -sub eax, 2
    (cpu.eax) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046d595  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046d596  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0046d598  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0046d59d  e8beb20600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d5a2  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d5a5  8b4006                 -mov eax, dword ptr [eax + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6) /* 0x6 */);
    // 0046d5a8  8b55d8                 -mov edx, dword ptr [ebp - 0x28]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 0046d5ab  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d5ae  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046d5b3  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046d5b5  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0046d5ba  8d5002                 -lea edx, [eax + 2]
    cpu.edx = x86::reg32(cpu.eax + x86::reg32(2) /* 0x2 */);
    // 0046d5bd  89f3                   -mov ebx, esi
    cpu.ebx = cpu.esi;
    // 0046d5bf  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d5c2  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046d5c7  8b75d8                 -mov esi, dword ptr [ebp - 0x28]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 0046d5ca  8b8088000000           -mov eax, dword ptr [eax + 0x88]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(136) /* 0x88 */);
    // 0046d5d0  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046d5d5  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d5d8  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046d5dd  83e802                 -sub eax, 2
    (cpu.eax) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046d5e0  83c604                 -add esi, 4
    (cpu.esi) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0046d5e3  e878b20600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d5e8  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046d5ed  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d5f0  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d5f3  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 0046d5f8  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 0046d5fa  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046d5ff  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046d602  8b8088000000           -mov eax, dword ptr [eax + 0x88]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(136) /* 0x88 */);
    // 0046d608  68aa0000ff             -push 0xff0000aa
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190250 /*0xff0000aa*/;
    cpu.esp -= 4;
    // 0046d60d  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d610  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d613  68aa0000ff             -push 0xff0000aa
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190250 /*0xff0000aa*/;
    cpu.esp -= 4;
    // 0046d618  83ea02                 -sub edx, 2
    (cpu.edx) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046d61b  83e802                 -sub eax, 2
    (cpu.eax) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046d61e  e83db20600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d623  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046d628  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d62b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d62e  bb02000000             -mov ebx, 2
    cpu.ebx = 2 /*0x2*/;
    // 0046d633  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 0046d635  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046d63a  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046d63d  8b8088000000           -mov eax, dword ptr [eax + 0x88]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(136) /* 0x88 */);
    // 0046d643  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046d648  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d64b  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d64e  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046d653  83ea02                 -sub edx, 2
    (cpu.edx) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046d656  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 0046d658  e803b20600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d65d  68aa0000ff             -push 0xff0000aa
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190250 /*0xff0000aa*/;
    cpu.esp -= 4;
    // 0046d662  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d665  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d668  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 0046d66d  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046d672  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046d675  8b4042                 -mov eax, dword ptr [eax + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(66) /* 0x42 */);
    // 0046d678  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d67b  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d67e  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046d683  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046d685  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d688  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0046d68a  68aa0000ff             -push 0xff0000aa
    app->getMemory<x86::reg32>(cpu.esp-4) = 4278190250 /*0xff0000aa*/;
    cpu.esp -= 4;
    // 0046d68f  8b8088000000           -mov eax, dword ptr [eax + 0x88]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(136) /* 0x88 */);
    // 0046d695  4a                     -dec edx
    (cpu.edx)--;
    // 0046d696  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d699  e8c2b10600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d69e  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d6a1  8b4044                 -mov eax, dword ptr [eax + 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(68) /* 0x44 */);
    // 0046d6a4  8b4de4                 -mov ecx, dword ptr [ebp - 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 0046d6a7  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d6aa  39c8                   +cmp eax, ecx
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
    // 0046d6ac  7d27                   -jge 0x46d6d5
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046d6d5;
    }
    // 0046d6ae  682894d5ff             -push 0xffd59428
    app->getMemory<x86::reg32>(cpu.esp-4) = 4292187176 /*0xffd59428*/;
    cpu.esp -= 4;
    // 0046d6b3  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d6b6  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d6b9  bb12000000             -mov ebx, 0x12
    cpu.ebx = 18 /*0x12*/;
    // 0046d6be  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 0046d6c0  8b5266                 -mov edx, dword ptr [edx + 0x66]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(102) /* 0x66 */);
    // 0046d6c3  8b4068                 -mov eax, dword ptr [eax + 0x68]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(104) /* 0x68 */);
    // 0046d6c6  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d6c9  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d6cc  4a                     -dec edx
    (cpu.edx)--;
    // 0046d6cd  83e803                 -sub eax, 3
    (cpu.eax) -= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 0046d6d0  e85bb20600             -call 0x4d8930
    cpu.esp -= 4;
    nfs3hp::port::draw2d::frame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046d6d5:
    // 0046d6d5  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d6d8  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d6db  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0046d6dd  8b4006                 -mov eax, dword ptr [eax + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6) /* 0x6 */);
    // 0046d6e0  8b5242                 -mov edx, dword ptr [edx + 0x42]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(66) /* 0x42 */);
    // 0046d6e3  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d6e6  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d6e9  895dc4                 -mov dword ptr [ebp - 0x3c], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.ebx;
    // 0046d6ec  8d3410                 -lea esi, [eax + edx]
    cpu.esi = x86::reg32(cpu.eax + cpu.edx * 1);
L_0x0046d6ef:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046d6ef  8b45c4                 -mov eax, dword ptr [ebp - 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 0046d6f2  3b45b0                 +cmp eax, dword ptr [ebp - 0x50]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-80) /* -0x50 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d6f5  7d38                   -jge 0x46d72f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046d72f;
    }
    // 0046d6f7  8b4dd0                 -mov ecx, dword ptr [ebp - 0x30]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0046d6fa  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046d6fb  8b5dcc                 -mov ebx, dword ptr [ebp - 0x34]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 0046d6fe  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d701  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046d702  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0046d704  8b8088000000           -mov eax, dword ptr [eax + 0x88]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(136) /* 0x88 */);
    // 0046d70a  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046d70b  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d70e  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046d70f  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 0046d711  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 0046d716  e845b10600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d71b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d71e  8b4dc4                 -mov ecx, dword ptr [ebp - 0x3c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */);
    // 0046d721  8b4042                 -mov eax, dword ptr [eax + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(66) /* 0x42 */);
    // 0046d724  41                     -inc ecx
    (cpu.ecx)++;
    // 0046d725  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d728  894dc4                 -mov dword ptr [ebp - 0x3c], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = cpu.ecx;
    // 0046d72b  01c6                   +add esi, eax
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046d72d  ebc0                   -jmp 0x46d6ef
    goto L_0x0046d6ef;
L_0x0046d72f:
    // 0046d72f  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d732  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d735  8b9288000000           -mov edx, dword ptr [edx + 0x88]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(136) /* 0x88 */);
    // 0046d73b  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046d73e  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d741  c1f810                 +sar eax, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 0046d744  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 0046d749  8d3c02                 -lea edi, [edx + eax]
    cpu.edi = x86::reg32(cpu.edx + cpu.eax * 1);
    // 0046d74c  eb05                   -jmp 0x46d753
    goto L_0x0046d753;
L_0x0046d74e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046d74e  83fe04                 +cmp esi, 4
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d751  7d38                   -jge 0x46d78b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046d78b;
    }
L_0x0046d753:
    // 0046d753  8b5dcc                 -mov ebx, dword ptr [ebp - 0x34]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 0046d756  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046d757  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046d758  8b55d0                 -mov edx, dword ptr [ebp - 0x30]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-48) /* -0x30 */);
    // 0046d75b  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046d75c  8b4dd8                 -mov ecx, dword ptr [ebp - 0x28]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 0046d75f  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046d760  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d763  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0046d765  83c102                 -add ecx, 2
    (cpu.ecx) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046d768  8b5206                 -mov edx, dword ptr [edx + 6]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(6) /* 0x6 */);
    // 0046d76b  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 0046d770  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d773  e8e8b00600             -call 0x4d8860
    cpu.esp -= 4;
    nfs3hp::port::draw2d::drawShadedBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d778  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d77b  8d0436                 -lea eax, [esi + esi]
    cpu.eax = x86::reg32(cpu.esi + cpu.esi * 1);
    // 0046d77e  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0046d780  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046d783  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d786  46                     -inc esi
    (cpu.esi)++;
    // 0046d787  01c7                   +add edi, eax
    {
        x86::reg32& tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046d789  ebc3                   -jmp 0x46d74e
    goto L_0x0046d74e;
L_0x0046d78b:
    // 0046d78b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d78e  6683b88800000000       +cmp word ptr [eax + 0x88], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(136) /* 0x88 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046d796  750e                   -jne 0x46d7a6
    if (!cpu.flags.zf)
    {
        goto L_0x0046d7a6;
    }
    // 0046d798  837da000               +cmp dword ptr [ebp - 0x60], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-96) /* -0x60 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d79c  7408                   -je 0x46d7a6
    if (cpu.flags.zf)
    {
        goto L_0x0046d7a6;
    }
    // 0046d79e  8b55d8                 -mov edx, dword ptr [ebp - 0x28]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */);
    // 0046d7a1  e81af6ffff             -call 0x46cdc0
    cpu.esp -= 4;
    sub_46cdc0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046d7a6:
    // 0046d7a6  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 0046d7a8  897dec                 -mov dword ptr [ebp - 0x14], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.edi;
    // 0046d7ab  e908010000             -jmp 0x46d8b8
    goto L_0x0046d8b8;
L_0x0046d7b0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046d7b0  83c60a                 -add esi, 0xa
    (cpu.esi) += x86::reg32(x86::sreg32(10 /*0xa*/));
L_0x0046d7b3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046d7b3  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046d7b6  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d7b9  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0046d7bc  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046d7be  8b514c                 -mov edx, dword ptr [ecx + 0x4c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(76) /* 0x4c */);
    // 0046d7c1  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d7c4  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046d7c6  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046d7c9  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046d7cb  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046d7cd  8b5146                 -mov edx, dword ptr [ecx + 0x46]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(70) /* 0x46 */);
    // 0046d7d0  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d7d3  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046d7d5  83e805                 -sub eax, 5
    (cpu.eax) -= x86::reg32(x86::sreg32(5 /*0x5*/));
    // 0046d7d8  8945a4                 -mov dword ptr [ebp - 0x5c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */) = cpu.eax;
    // 0046d7db  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046d7de  8b44458a               -mov eax, dword ptr [ebp + eax*2 - 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-118) /* -0x76 */ + cpu.eax * 2);
    // 0046d7e2  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d7e5  e866400600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d7ea  8b1db8d36f00           -mov ebx, dword ptr [0x6fd3b8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0046d7f0  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0046d7f2  83fb03                 +cmp ebx, 3
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d7f5  7511                   -jne 0x46d808
    if (!cpu.flags.zf)
    {
        goto L_0x0046d808;
    }
    // 0046d7f7  3b5dec                 +cmp ebx, dword ptr [ebp - 0x14]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d7fa  750c                   -jne 0x46d808
    if (!cpu.flags.zf)
    {
        goto L_0x0046d808;
    }
    // 0046d7fc  b893010000             -mov eax, 0x193
    cpu.eax = 403 /*0x193*/;
    // 0046d801  e84a400600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d806  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
L_0x0046d808:
    // 0046d808  837dbc00               +cmp dword ptr [ebp - 0x44], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d80c  7424                   -je 0x46d832
    if (cpu.flags.zf)
    {
        goto L_0x0046d832;
    }
    // 0046d80e  837de000               +cmp dword ptr [ebp - 0x20], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d812  751e                   -jne 0x46d832
    if (!cpu.flags.zf)
    {
        goto L_0x0046d832;
    }
    // 0046d814  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046d817  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d81a  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0046d81d  83c248                 -add edx, 0x48
    (cpu.edx) += x86::reg32(x86::sreg32(72 /*0x48*/));
    // 0046d820  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046d822  e8c9e90200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d827  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046d829  7407                   -je 0x46d832
    if (cpu.flags.zf)
    {
        goto L_0x0046d832;
    }
    // 0046d82b  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 0046d830  eb02                   -jmp 0x46d834
    goto L_0x0046d834;
L_0x0046d832:
    // 0046d832  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x0046d834:
    // 0046d834  8945a8                 -mov dword ptr [ebp - 0x58], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */) = cpu.eax;
    // 0046d837  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046d839  7438                   -je 0x46d873
    if (cpu.flags.zf)
    {
        goto L_0x0046d873;
    }
    // 0046d83b  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046d83e  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d841  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 0046d844  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0046d846  8b504c                 -mov edx, dword ptr [eax + 0x4c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(76) /* 0x4c */);
    // 0046d849  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d84c  8d4aff                 -lea ecx, [edx - 1]
    cpu.ecx = x86::reg32(cpu.edx + x86::reg32(-1) /* -0x1 */);
    // 0046d84f  8b504a                 -mov edx, dword ptr [eax + 0x4a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(74) /* 0x4a */);
    // 0046d852  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d855  4a                     -dec edx
    (cpu.edx)--;
    // 0046d856  6840e4ffff             -push 0xffffe440
    app->getMemory<x86::reg32>(cpu.esp-4) = 4294960192 /*0xffffe440*/;
    cpu.esp -= 4;
    // 0046d85b  89559c                 -mov dword ptr [ebp - 0x64], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-100) /* -0x64 */) = cpu.edx;
    // 0046d85e  8b5046                 -mov edx, dword ptr [eax + 0x46]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(70) /* 0x46 */);
    // 0046d861  8b4048                 -mov eax, dword ptr [eax + 0x48]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(72) /* 0x48 */);
    // 0046d864  8b5d9c                 -mov ebx, dword ptr [ebp - 0x64]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-100) /* -0x64 */);
    // 0046d867  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d86a  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d86d  40                     -inc eax
    (cpu.eax)++;
    // 0046d86e  e8bdb00600             -call 0x4d8930
    cpu.esp -= 4;
    nfs3hp::port::draw2d::frame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046d873:
    // 0046d873  837ddc00               +cmp dword ptr [ebp - 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d877  7407                   -je 0x46d880
    if (cpu.flags.zf)
    {
        goto L_0x0046d880;
    }
    // 0046d879  b8964a0000             -mov eax, 0x4a96
    cpu.eax = 19094 /*0x4a96*/;
    // 0046d87e  eb12                   -jmp 0x46d892
    goto L_0x0046d892;
L_0x0046d880:
    // 0046d880  837da800               +cmp dword ptr [ebp - 0x58], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-88) /* -0x58 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d884  7507                   -jne 0x46d88d
    if (!cpu.flags.zf)
    {
        goto L_0x0046d88d;
    }
    // 0046d886  b82894d5ff             -mov eax, 0xffd59428
    cpu.eax = 4292187176 /*0xffd59428*/;
    // 0046d88b  eb05                   -jmp 0x46d892
    goto L_0x0046d892;
L_0x0046d88d:
    // 0046d88d  b840e4ff00             -mov eax, 0xffe440
    cpu.eax = 16770112 /*0xffe440*/;
L_0x0046d892:
    // 0046d892  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046d893  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046d896  8b444592               -mov eax, dword ptr [ebp + eax*2 - 0x6e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-110) /* -0x6e */ + cpu.eax * 2);
    // 0046d89a  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046d89f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d8a2  8b5da4                 -mov ebx, dword ptr [ebp - 0x5c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-92) /* -0x5c */);
    // 0046d8a5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046d8a6  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0046d8a8  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 0046d8aa  e8e148feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d8af  ff45ec                 -inc dword ptr [ebp - 0x14]
    (app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */))++;
    // 0046d8b2  837dec04               +cmp dword ptr [ebp - 0x14], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d8b6  7d4d                   -jge 0x46d905
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046d905;
    }
L_0x0046d8b8:
    // 0046d8b8  8d7d8c                 -lea edi, [ebp - 0x74]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-116) /* -0x74 */);
    // 0046d8bb  beb0cc4600             -mov esi, 0x46ccb0
    cpu.esi = 4639920 /*0x46ccb0*/;
    // 0046d8c0  8b55ec                 -mov edx, dword ptr [ebp - 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046d8c3  8b4dfc                 -mov ecx, dword ptr [ebp - 4]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d8c6  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0046d8c7  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0046d8c8  c1e203                 -shl edx, 3
    cpu.edx <<= 3 /*0x3*/ % 32;
    // 0046d8cb  8d7d94                 -lea edi, [ebp - 0x6c]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-108) /* -0x6c */);
    // 0046d8ce  beb8cc4600             -mov esi, 0x46ccb8
    cpu.esi = 4639928 /*0x46ccb8*/;
    // 0046d8d3  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0046d8d5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0046d8d6  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 0046d8d7  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 0046d8da  8b7248                 -mov esi, dword ptr [edx + 0x48]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(72) /* 0x48 */);
    // 0046d8dd  668b7c4594             -mov di, word ptr [ebp + eax*2 - 0x6c]
    cpu.di = app->getMemory<x86::reg16>(cpu.ebp + x86::reg32(-108) /* -0x6c */ + cpu.eax * 2);
    // 0046d8e2  c1fe10                 -sar esi, 0x10
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (16 /*0x10*/ % 32));
    // 0046d8e5  6683ff05               +cmp di, 5
    {
        x86::reg16 tmp1 = cpu.di;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(5 /*0x5*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046d8e9  0f85c1feffff           -jne 0x46d7b0
    if (!cpu.flags.zf)
    {
        goto L_0x0046d7b0;
    }
    // 0046d8ef  8b524a                 -mov edx, dword ptr [edx + 0x4a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(74) /* 0x4a */);
    // 0046d8f2  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d8f5  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046d8f7  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046d8fa  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046d8fc  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046d8fe  01c6                   +add esi, eax
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046d900  e9aefeffff             -jmp 0x46d7b3
    goto L_0x0046d7b3;
L_0x0046d905:
    // 0046d905  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d908  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d90b  8b5242                 -mov edx, dword ptr [edx + 0x42]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(66) /* 0x42 */);
    // 0046d90e  8b4006                 -mov eax, dword ptr [eax + 6]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(6) /* 0x6 */);
    // 0046d911  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d914  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d917  8d0c10                 -lea ecx, [eax + edx]
    cpu.ecx = x86::reg32(cpu.eax + cpu.edx * 1);
    // 0046d91a  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046d91c  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046d91f  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046d921  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046d923  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 0046d925  83e805                 -sub eax, 5
    (cpu.eax) -= x86::reg32(x86::sreg32(5 /*0x5*/));
    // 0046d928  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 0046d92b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d92e  8b8082000000           -mov eax, dword ptr [eax + 0x82]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(130) /* 0x82 */);
    // 0046d934  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0046d936  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046d939  895dc8                 -mov dword ptr [ebp - 0x38], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.ebx;
    // 0046d93c  8945d4                 -mov dword ptr [ebp - 0x2c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.eax;
L_0x0046d93f:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046d93f  8b45c8                 -mov eax, dword ptr [ebp - 0x38]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 0046d942  3b45b0                 +cmp eax, dword ptr [ebp - 0x50]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-80) /* -0x50 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d945  0f8d7d050000           -jge 0x46dec8
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046dec8;
    }
    // 0046d94b  8b45d4                 -mov eax, dword ptr [ebp - 0x2c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 0046d94e  3b45e4                 +cmp eax, dword ptr [ebp - 0x1c]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d951  0f8d71050000           -jge 0x46dec8
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046dec8;
    }
    // 0046d957  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0046d959  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0046d95b  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0046d95d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0046d95f  e86c83feff             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d964  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 0046d967  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046d969  0f8459050000           -je 0x46dec8
    if (cpu.flags.zf)
    {
        goto L_0x0046dec8;
    }
    // 0046d96f  8b90c4020000           -mov edx, dword ptr [eax + 0x2c4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(708) /* 0x2c4 */);
    // 0046d975  83fa02                 +cmp edx, 2
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
    // 0046d978  7405                   -je 0x46d97f
    if (cpu.flags.zf)
    {
        goto L_0x0046d97f;
    }
    // 0046d97a  83fa04                 +cmp edx, 4
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d97d  7507                   -jne 0x46d986
    if (!cpu.flags.zf)
    {
        goto L_0x0046d986;
    }
L_0x0046d97f:
    // 0046d97f  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0046d984  eb02                   -jmp 0x46d988
    goto L_0x0046d988;
L_0x0046d986:
    // 0046d986  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x0046d988:
    // 0046d988  837de000               +cmp dword ptr [ebp - 0x20], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d98c  0f84d0010000           -je 0x46db62
    if (cpu.flags.zf)
    {
        goto L_0x0046db62;
    }
    // 0046d992  837ddc00               +cmp dword ptr [ebp - 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d996  7407                   -je 0x46d99f
    if (cpu.flags.zf)
    {
        goto L_0x0046d99f;
    }
    // 0046d998  b8964a0000             -mov eax, 0x4a96
    cpu.eax = 19094 /*0x4a96*/;
    // 0046d99d  eb05                   -jmp 0x46d9a4
    goto L_0x0046d9a4;
L_0x0046d99f:
    // 0046d99f  b8fd9d64ff             -mov eax, 0xff649dfd
    cpu.eax = 4284784125 /*0xff649dfd*/;
L_0x0046d9a4:
    // 0046d9a4  8945b4                 -mov dword ptr [ebp - 0x4c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */) = cpu.eax;
    // 0046d9a7  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0046d9a9  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046d9ac  8b15f4d46f00           -mov edx, dword ptr [0x6fd4f4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 0046d9b2  3b90f0010000           +cmp edx, dword ptr [eax + 0x1f0]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(496) /* 0x1f0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d9b8  7416                   -je 0x46d9d0
    if (cpu.flags.zf)
    {
        goto L_0x0046d9d0;
    }
    // 0046d9ba  8b0db0d36f00           -mov ecx, dword ptr [0x6fd3b0]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0046d9c0  83f901                 +cmp ecx, 1
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d9c3  7512                   -jne 0x46d9d7
    if (!cpu.flags.zf)
    {
        goto L_0x0046d9d7;
    }
    // 0046d9c5  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046d9c8  3b88f0010000           +cmp ecx, dword ptr [eax + 0x1f0]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(496) /* 0x1f0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046d9ce  7507                   -jne 0x46d9d7
    if (!cpu.flags.zf)
    {
        goto L_0x0046d9d7;
    }
L_0x0046d9d0:
    // 0046d9d0  c745b440e4ff00         -mov dword ptr [ebp - 0x4c], 0xffe440
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */) = 16770112 /*0xffe440*/;
L_0x0046d9d7:
    // 0046d9d7  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d9da  8bb088000000           -mov esi, dword ptr [eax + 0x88]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(136) /* 0x88 */);
    // 0046d9e0  b8ed000000             -mov eax, 0xed
    cpu.eax = 237 /*0xed*/;
    // 0046d9e5  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046d9e8  e8633e0600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046d9ed  8b523a                 -mov edx, dword ptr [edx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(58) /* 0x3a */);
    // 0046d9f0  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046d9f3  89459c                 -mov dword ptr [ebp - 0x64], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-100) /* -0x64 */) = cpu.eax;
    // 0046d9f6  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046d9f8  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046d9fb  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046d9fd  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046d9ff  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046da04  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046da07  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046da08  c1fe10                 -sar esi, 0x10
    cpu.esi = x86::reg32(x86::sreg32(cpu.esi) >> (16 /*0x10*/ % 32));
    // 0046da0b  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046da0d  8d1406                 -lea edx, [esi + eax]
    cpu.edx = x86::reg32(cpu.esi + cpu.eax * 1);
    // 0046da10  8b459c                 -mov eax, dword ptr [ebp - 0x64]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-100) /* -0x64 */);
    // 0046da13  e87847feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046da18  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046da1b  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046da1e  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046da21  8d958cfeffff           -lea edx, [ebp - 0x174]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046da27  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046da29  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046da2c  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046da2f  e83c7bfeff             -call 0x455570
    cpu.esp -= 4;
    sub_455570(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046da34  8b45b4                 -mov eax, dword ptr [ebp - 0x4c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */);
    // 0046da37  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046da3c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046da3d  83eb0a                 -sub ebx, 0xa
    (cpu.ebx) -= x86::reg32(x86::sreg32(10 /*0xa*/));
    // 0046da40  8d460a                 -lea eax, [esi + 0xa]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(10) /* 0xa */);
    // 0046da43  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0046da45  89459c                 -mov dword ptr [ebp - 0x64], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-100) /* -0x64 */) = cpu.eax;
    // 0046da48  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 0046da4a  8d858cfeffff           -lea eax, [ebp - 0x174]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046da50  e83b47feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046da55  8d958cfeffff           -lea edx, [ebp - 0x174]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046da5b  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046da5e  e89d7bfeff             -call 0x455600
    cpu.esp -= 4;
    sub_455600(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046da63  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046da66  8b403c                 -mov eax, dword ptr [eax + 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
    // 0046da69  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046da6c  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046da6d  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046da70  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046da71  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046da76  8b559c                 -mov edx, dword ptr [ebp - 0x64]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-100) /* -0x64 */);
    // 0046da79  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0046da7b  83c305                 -add ebx, 5
    (cpu.ebx) += x86::reg32(x86::sreg32(5 /*0x5*/));
    // 0046da7e  8d858cfeffff           -lea eax, [ebp - 0x174]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046da84  e89746feff             -call 0x452120
    cpu.esp -= 4;
    sub_452120(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046da89  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046da8c  8b403c                 -mov eax, dword ptr [eax + 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
    // 0046da8f  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046da92  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046da94  b8eb000000             -mov eax, 0xeb
    cpu.eax = 235 /*0xeb*/;
    // 0046da99  8b55dc                 -mov edx, dword ptr [ebp - 0x24]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 0046da9c  e8af3d0600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046daa1  8945ac                 -mov dword ptr [ebp - 0x54], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */) = cpu.eax;
    // 0046daa4  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0046daa6  7407                   -je 0x46daaf
    if (cpu.flags.zf)
    {
        goto L_0x0046daaf;
    }
    // 0046daa8  b8964a0000             -mov eax, 0x4a96
    cpu.eax = 19094 /*0x4a96*/;
    // 0046daad  eb05                   -jmp 0x46dab4
    goto L_0x0046dab4;
L_0x0046daaf:
    // 0046daaf  b800008000             -mov eax, 0x800000
    cpu.eax = 8388608 /*0x800000*/;
L_0x0046dab4:
    // 0046dab4  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dab7  8b523e                 -mov edx, dword ptr [edx + 0x3e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(62) /* 0x3e */);
    // 0046daba  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046dabd  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046dabe  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046dac0  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046dac3  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046dac5  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046dac7  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046dacc  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046dacf  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046dad1  8d1406                 -lea edx, [esi + eax]
    cpu.edx = x86::reg32(cpu.esi + cpu.eax * 1);
    // 0046dad4  8b45ac                 -mov eax, dword ptr [ebp - 0x54]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-84) /* -0x54 */);
    // 0046dad7  e8b446feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dadc  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dadf  8b403e                 -mov eax, dword ptr [eax + 0x3e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(62) /* 0x3e */);
    // 0046dae2  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046dae5  8b0db8d36f00           -mov ecx, dword ptr [0x6fd3b8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0046daeb  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046daed  83f903                 +cmp ecx, 3
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046daf0  7532                   -jne 0x46db24
    if (!cpu.flags.zf)
    {
        goto L_0x0046db24;
    }
    // 0046daf2  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046daf5  8b98c8020000           -mov ebx, dword ptr [eax + 0x2c8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(712) /* 0x2c8 */);
    // 0046dafb  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046dafc  68b8ae5300             -push 0x53aeb8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5484216 /*0x53aeb8*/;
    cpu.esp -= 4;
    // 0046db01  8d858cfeffff           -lea eax, [ebp - 0x174]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046db07  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046db08  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046db0b  e8801b0700             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046db10  8b5240                 -mov edx, dword ptr [edx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(64) /* 0x40 */);
    // 0046db13  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046db16  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046db18  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046db1b  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046db1d  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046db1f  83c40c                 +add esp, 0xc
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
    // 0046db22  eb20                   -jmp 0x46db44
    goto L_0x0046db44;
L_0x0046db24:
    // 0046db24  8d958cfeffff           -lea edx, [ebp - 0x174]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046db2a  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046db2d  e8ee7bfeff             -call 0x455720
    cpu.esp -= 4;
    sub_455720(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046db32  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046db35  8b5240                 -mov edx, dword ptr [edx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(64) /* 0x40 */);
    // 0046db38  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046db3b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046db3d  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046db40  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046db42  d1f8                   +sar eax, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
L_0x0046db44:
    // 0046db44  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046db45  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046db4a  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046db4d  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046db4f  8d1406                 -lea edx, [esi + eax]
    cpu.edx = x86::reg32(cpu.esi + cpu.eax * 1);
    // 0046db52  8d858cfeffff           -lea eax, [ebp - 0x174]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046db58  e83346feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046db5d  e942030000             -jmp 0x46dea4
    goto L_0x0046dea4;
L_0x0046db62:
    // 0046db62  837ddc00               +cmp dword ptr [ebp - 0x24], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046db66  7407                   -je 0x46db6f
    if (cpu.flags.zf)
    {
        goto L_0x0046db6f;
    }
    // 0046db68  b8964a0000             -mov eax, 0x4a96
    cpu.eax = 19094 /*0x4a96*/;
    // 0046db6d  eb10                   -jmp 0x46db7f
    goto L_0x0046db7f;
L_0x0046db6f:
    // 0046db6f  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 0046db71  7407                   -je 0x46db7a
    if (cpu.flags.zf)
    {
        goto L_0x0046db7a;
    }
    // 0046db73  b800008000             -mov eax, 0x800000
    cpu.eax = 8388608 /*0x800000*/;
    // 0046db78  eb05                   -jmp 0x46db7f
    goto L_0x0046db7f;
L_0x0046db7a:
    // 0046db7a  b8fd9d64ff             -mov eax, 0xff649dfd
    cpu.eax = 4284784125 /*0xff649dfd*/;
L_0x0046db7f:
    // 0046db7f  8945b8                 -mov dword ptr [ebp - 0x48], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */) = cpu.eax;
    // 0046db82  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 0046db85  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046db88  8b3df4d46f00           -mov edi, dword ptr [0x6fd4f4]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 0046db8e  3bb8f0010000           +cmp edi, dword ptr [eax + 0x1f0]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(496) /* 0x1f0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046db94  7415                   -je 0x46dbab
    if (cpu.flags.zf)
    {
        goto L_0x0046dbab;
    }
    // 0046db96  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046db9d  7513                   -jne 0x46dbb2
    if (!cpu.flags.zf)
    {
        goto L_0x0046dbb2;
    }
    // 0046db9f  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046dba2  83b8f001000001         +cmp dword ptr [eax + 0x1f0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(496) /* 0x1f0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046dba9  7507                   -jne 0x46dbb2
    if (!cpu.flags.zf)
    {
        goto L_0x0046dbb2;
    }
L_0x0046dbab:
    // 0046dbab  c745b840e4ff00         -mov dword ptr [ebp - 0x48], 0xffe440
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */) = 16770112 /*0xffe440*/;
L_0x0046dbb2:
    // 0046dbb2  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dbb5  8b8088000000           -mov eax, dword ptr [eax + 0x88]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(136) /* 0x88 */);
    // 0046dbbb  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046dbbe  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 0046dbc1  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046dbc4  83b8c402000004         +cmp dword ptr [eax + 0x2c4], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(708) /* 0x2c4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046dbcb  752e                   -jne 0x46dbfb
    if (!cpu.flags.zf)
    {
        goto L_0x0046dbfb;
    }
    // 0046dbcd  b8ed000000             -mov eax, 0xed
    cpu.eax = 237 /*0xed*/;
    // 0046dbd2  8dbd8cfeffff           -lea edi, [ebp - 0x174]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046dbd8  e8733c0600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dbdd  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0046dbdf  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
L_0x0046dbe0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046dbe0  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 0046dbe2  8807                   -mov byte ptr [edi], al
    app->getMemory<x86::reg8>(cpu.edi) = cpu.al;
    // 0046dbe4  3c00                   +cmp al, 0
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
    // 0046dbe6  7410                   -je 0x46dbf8
    if (cpu.flags.zf)
    {
        goto L_0x0046dbf8;
    }
    // 0046dbe8  8a4601                 -mov al, byte ptr [esi + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 0046dbeb  83c602                 -add esi, 2
    (cpu.esi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046dbee  884701                 -mov byte ptr [edi + 1], al
    app->getMemory<x86::reg8>(cpu.edi + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 0046dbf1  83c702                 -add edi, 2
    (cpu.edi) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 0046dbf4  3c00                   +cmp al, 0
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
    // 0046dbf6  75e8                   -jne 0x46dbe0
    if (!cpu.flags.zf)
    {
        goto L_0x0046dbe0;
    }
L_0x0046dbf8:
    // 0046dbf8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046dbf9  eb0b                   -jmp 0x46dc06
    goto L_0x0046dc06;
L_0x0046dbfb:
    // 0046dbfb  8d958cfeffff           -lea edx, [ebp - 0x174]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046dc01  e8ba79feff             -call 0x4555c0
    cpu.esp -= 4;
    sub_4555c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046dc06:
    // 0046dc06  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dc09  8b523a                 -mov edx, dword ptr [edx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(58) /* 0x3a */);
    // 0046dc0c  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046dc0f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046dc11  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046dc14  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046dc16  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046dc18  8b75e8                 -mov esi, dword ptr [ebp - 0x18]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0046dc1b  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046dc20  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046dc21  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046dc24  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046dc27  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046dc29  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046dc2b  8d858cfeffff           -lea eax, [ebp - 0x174]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046dc31  e85a45feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dc36  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dc39  8b7df8                 -mov edi, dword ptr [ebp - 8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046dc3c  8b403a                 -mov eax, dword ptr [eax + 0x3a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(58) /* 0x3a */);
    // 0046dc3f  8d958cfeffff           -lea edx, [ebp - 0x174]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046dc45  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046dc48  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046dc4b  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046dc4d  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046dc50  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046dc55  e81679feff             -call 0x455570
    cpu.esp -= 4;
    sub_455570(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dc5a  8b45b8                 -mov eax, dword ptr [ebp - 0x48]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-72) /* -0x48 */);
    // 0046dc5d  83eb0a                 -sub ebx, 0xa
    (cpu.ebx) -= x86::reg32(x86::sreg32(10 /*0xa*/));
    // 0046dc60  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046dc61  897df8                 -mov dword ptr [ebp - 8], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edi;
    // 0046dc64  8d770a                 -lea esi, [edi + 0xa]
    cpu.esi = x86::reg32(cpu.edi + x86::reg32(10) /* 0xa */);
    // 0046dc67  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0046dc69  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0046dc6b  8d858cfeffff           -lea eax, [ebp - 0x174]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046dc71  e81a45feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dc76  8d958cfeffff           -lea edx, [ebp - 0x174]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046dc7c  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046dc7f  e87c79feff             -call 0x455600
    cpu.esp -= 4;
    sub_455600(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dc84  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dc87  8b403c                 -mov eax, dword ptr [eax + 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
    // 0046dc8a  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046dc8d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046dc8e  8b55e8                 -mov edx, dword ptr [ebp - 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0046dc91  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046dc94  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046dc95  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046dc9a  83c305                 -add ebx, 5
    (cpu.ebx) += x86::reg32(x86::sreg32(5 /*0x5*/));
    // 0046dc9d  6a03                   -push 3
    app->getMemory<x86::reg32>(cpu.esp-4) = 3 /*0x3*/;
    cpu.esp -= 4;
    // 0046dc9f  8d858cfeffff           -lea eax, [ebp - 0x174]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046dca5  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0046dca7  e87444feff             -call 0x452120
    cpu.esp -= 4;
    sub_452120(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dcac  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046dcaf  83b8c402000004         +cmp dword ptr [eax + 0x2c4], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(708) /* 0x2c4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046dcb6  0f85d3000000           -jne 0x46dd8f
    if (!cpu.flags.zf)
    {
        goto L_0x0046dd8f;
    }
    // 0046dcbc  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dcbf  8b403c                 -mov eax, dword ptr [eax + 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
    // 0046dcc2  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046dcc5  8d1c07                 -lea ebx, [edi + eax]
    cpu.ebx = x86::reg32(cpu.edi + cpu.eax * 1);
    // 0046dcc8  b8f5000000             -mov eax, 0xf5
    cpu.eax = 245 /*0xf5*/;
    // 0046dccd  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dcd0  e87b3b0600             -call 0x4d1850
    cpu.esp -= 4;
    nfs3hp::port::text::languageText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dcd5  8b523e                 -mov edx, dword ptr [edx + 0x3e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(62) /* 0x3e */);
    // 0046dcd8  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046dcdb  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 0046dcdd  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046dcdf  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046dce2  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046dce4  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046dce6  8b7de8                 -mov edi, dword ptr [ebp - 0x18]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0046dce9  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046dcee  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046dcef  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
    // 0046dcf2  8d1403                 -lea edx, [ebx + eax]
    cpu.edx = x86::reg32(cpu.ebx + cpu.eax * 1);
    // 0046dcf5  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046dcf7  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046dcfa  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0046dcfc  e88f44feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dd01  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dd04  8b403e                 -mov eax, dword ptr [eax + 0x3e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(62) /* 0x3e */);
    // 0046dd07  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046dd0a  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046dd0d  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046dd0f  8b0db8d36f00           -mov ecx, dword ptr [0x6fd3b8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0046dd15  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 0046dd18  83f903                 +cmp ecx, 3
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046dd1b  7532                   -jne 0x46dd4f
    if (!cpu.flags.zf)
    {
        goto L_0x0046dd4f;
    }
    // 0046dd1d  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046dd20  8b98c8020000           -mov ebx, dword ptr [eax + 0x2c8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(712) /* 0x2c8 */);
    // 0046dd26  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046dd27  68b8ae5300             -push 0x53aeb8
    app->getMemory<x86::reg32>(cpu.esp-4) = 5484216 /*0x53aeb8*/;
    cpu.esp -= 4;
    // 0046dd2c  8d858cfeffff           -lea eax, [ebp - 0x174]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046dd32  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046dd33  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dd36  e855190700             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dd3b  8b5240                 -mov edx, dword ptr [edx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(64) /* 0x40 */);
    // 0046dd3e  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046dd41  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046dd43  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046dd46  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046dd48  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046dd4a  83c40c                 +add esp, 0xc
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
    // 0046dd4d  eb20                   -jmp 0x46dd6f
    goto L_0x0046dd6f;
L_0x0046dd4f:
    // 0046dd4f  8d958cfeffff           -lea edx, [ebp - 0x174]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046dd55  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046dd58  e8c379feff             -call 0x455720
    cpu.esp -= 4;
    sub_455720(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dd5d  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dd60  8b5240                 -mov edx, dword ptr [edx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(64) /* 0x40 */);
    // 0046dd63  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046dd66  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046dd68  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046dd6b  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046dd6d  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
L_0x0046dd6f:
    // 0046dd6f  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046dd74  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046dd75  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046dd78  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046dd7b  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046dd7d  01c2                   +add edx, eax
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046dd7f  8d858cfeffff           -lea eax, [ebp - 0x174]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046dd85  e80644feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dd8a  e915010000             -jmp 0x46dea4
    goto L_0x0046dea4;
L_0x0046dd8f:
    // 0046dd8f  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dd92  8b403c                 -mov eax, dword ptr [eax + 0x3c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(60) /* 0x3c */);
    // 0046dd95  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046dd98  8d1407                 -lea edx, [edi + eax]
    cpu.edx = x86::reg32(cpu.edi + cpu.eax * 1);
    // 0046dd9b  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046dd9e  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 0046dda1  8d958cfeffff           -lea edx, [ebp - 0x174]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046dda7  8b8038020000           -mov eax, dword ptr [eax + 0x238]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(568) /* 0x238 */);
    // 0046ddad  8b4dc0                 -mov ecx, dword ptr [ebp - 0x40]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 0046ddb0  e88b100600             -call 0x4cee40
    cpu.esp -= 4;
    sub_4cee40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ddb5  83f901                 +cmp ecx, 1
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ddb8  7507                   -jne 0x46ddc1
    if (!cpu.flags.zf)
    {
        goto L_0x0046ddc1;
    }
    // 0046ddba  b8964a0000             -mov eax, 0x4a96
    cpu.eax = 19094 /*0x4a96*/;
    // 0046ddbf  eb03                   -jmp 0x46ddc4
    goto L_0x0046ddc4;
L_0x0046ddc1:
    // 0046ddc1  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
L_0x0046ddc4:
    // 0046ddc4  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ddc7  8b523e                 -mov edx, dword ptr [edx + 0x3e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(62) /* 0x3e */);
    // 0046ddca  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046ddcd  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046ddce  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046ddd0  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046ddd3  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046ddd5  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046ddd7  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046dddc  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046dddf  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046dde2  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046dde4  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046dde6  8d858cfeffff           -lea eax, [ebp - 0x174]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046ddec  e89f43feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046ddf1  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ddf4  8b403e                 -mov eax, dword ptr [eax + 0x3e]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(62) /* 0x3e */);
    // 0046ddf7  8b5df8                 -mov ebx, dword ptr [ebp - 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046ddfa  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046ddfd  01c3                   -add ebx, eax
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046ddff  8b35b8d36f00           -mov esi, dword ptr [0x6fd3b8]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(7328696) /* 0x6fd3b8 */);
    // 0046de05  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
    // 0046de08  83fe03                 +cmp esi, 3
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046de0b  754a                   -jne 0x46de57
    if (!cpu.flags.zf)
    {
        goto L_0x0046de57;
    }
    // 0046de0d  8b3dbcd46f00           -mov edi, dword ptr [0x6fd4bc]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7328956) /* 0x6fd4bc */);
    // 0046de13  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046de16  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046de17  8b90c8020000           -mov edx, dword ptr [eax + 0x2c8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(712) /* 0x2c8 */);
    // 0046de1d  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046de1e  68bcae5300             -push 0x53aebc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5484220 /*0x53aebc*/;
    cpu.esp -= 4;
    // 0046de23  8d858cfeffff           -lea eax, [ebp - 0x174]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046de29  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046de2a  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046de2d  e85e180700             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046de32  8b5240                 -mov edx, dword ptr [edx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(64) /* 0x40 */);
    // 0046de35  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046de38  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046de3a  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046de3d  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046de3f  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046de41  83c410                 +add esp, 0x10
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046de44  8b4de8                 -mov ecx, dword ptr [ebp - 0x18]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 0046de47  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046de48  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046de4a  8d1403                 -lea edx, [ebx + eax]
    cpu.edx = x86::reg32(cpu.ebx + cpu.eax * 1);
    // 0046de4d  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046de52  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046de55  eb42                   -jmp 0x46de99
    goto L_0x0046de99;
L_0x0046de57:
    // 0046de57  8d958cfeffff           -lea edx, [ebp - 0x174]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046de5d  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 0046de60  8b5dc0                 -mov ebx, dword ptr [ebp - 0x40]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */);
    // 0046de63  e8b878feff             -call 0x455720
    cpu.esp -= 4;
    sub_455720(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046de68  83fb02                 +cmp ebx, 2
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
    // 0046de6b  7507                   -jne 0x46de74
    if (!cpu.flags.zf)
    {
        goto L_0x0046de74;
    }
    // 0046de6d  b8964a0000             -mov eax, 0x4a96
    cpu.eax = 19094 /*0x4a96*/;
    // 0046de72  eb03                   -jmp 0x46de77
    goto L_0x0046de77;
L_0x0046de74:
    // 0046de74  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
L_0x0046de77:
    // 0046de77  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046de7a  8b5240                 -mov edx, dword ptr [edx + 0x40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(64) /* 0x40 */);
    // 0046de7d  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046de80  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 0046de81  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046de83  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 0046de86  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 0046de88  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 0046de8a  b912000000             -mov ecx, 0x12
    cpu.ecx = 18 /*0x12*/;
    // 0046de8f  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046de92  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 0046de95  6a05                   -push 5
    app->getMemory<x86::reg32>(cpu.esp-4) = 5 /*0x5*/;
    cpu.esp -= 4;
    // 0046de97  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
L_0x0046de99:
    // 0046de99  8d858cfeffff           -lea eax, [ebp - 0x174]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-372) /* -0x174 */);
    // 0046de9f  e8ec42feff             -call 0x452190
    cpu.esp -= 4;
    nfs3hp::port::text::drawMenuText(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046dea4:
    // 0046dea4  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dea7  8b75f0                 -mov esi, dword ptr [ebp - 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 0046deaa  8b4042                 -mov eax, dword ptr [eax + 0x42]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(66) /* 0x42 */);
    // 0046dead  8b7dd4                 -mov edi, dword ptr [ebp - 0x2c]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */);
    // 0046deb0  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046deb3  47                     -inc edi
    (cpu.edi)++;
    // 0046deb4  01c6                   +add esi, eax
    {
        x86::reg32& tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046deb6  8b45c8                 -mov eax, dword ptr [ebp - 0x38]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */);
    // 0046deb9  897dd4                 -mov dword ptr [ebp - 0x2c], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.edi;
    // 0046debc  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0046debd  8975f0                 -mov dword ptr [ebp - 0x10], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.esi;
    // 0046dec0  8945c8                 -mov dword ptr [ebp - 0x38], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.eax;
    // 0046dec3  e977faffff             -jmp 0x46d93f
    goto L_0x0046d93f;
L_0x0046dec8:
    // 0046dec8  837dbc00               +cmp dword ptr [ebp - 0x44], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046decc  7416                   -je 0x46dee4
    if (cpu.flags.zf)
    {
        goto L_0x0046dee4;
    }
    // 0046dece  837de000               +cmp dword ptr [ebp - 0x20], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046ded2  7510                   -jne 0x46dee4
    if (!cpu.flags.zf)
    {
        goto L_0x0046dee4;
    }
    // 0046ded4  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046ded7  e8a4f2ffff             -call 0x46d180
    cpu.esp -= 4;
    sub_46d180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dedc  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dedf  e84cf0ffff             -call 0x46cf30
    cpu.esp -= 4;
    nfs3hp::port::view::metricPopup(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046dee4:
    // 0046dee4  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046dee7  8b4044                 -mov eax, dword ptr [eax + 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(68) /* 0x44 */);
    // 0046deea  8b5de4                 -mov ebx, dword ptr [ebp - 0x1c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 0046deed  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046def0  39d8                   +cmp eax, ebx
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
    // 0046def2  0f8d89000000           -jge 0x46df81
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046df81;
    }
    // 0046def8  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046defb  6683b88400000000       +cmp word ptr [eax + 0x84], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(132) /* 0x84 */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 0046df03  742e                   -je 0x46df33
    if (cpu.flags.zf)
    {
        goto L_0x0046df33;
    }
    // 0046df05  83c068                 -add eax, 0x68
    (cpu.eax) += x86::reg32(x86::sreg32(104 /*0x68*/));
    // 0046df08  8b58fe                 -mov ebx, dword ptr [eax - 2]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-2) /* -0x2 */);
    // 0046df0b  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 0046df0d  e8dee20200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046df12  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0046df15  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046df18  43                     -inc ebx
    (cpu.ebx)++;
    // 0046df19  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046df1b  7409                   -je 0x46df26
    if (cpu.flags.zf)
    {
        goto L_0x0046df26;
    }
    // 0046df1d  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046df20  668b407c               -mov ax, word ptr [eax + 0x7c]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(124) /* 0x7c */);
    // 0046df24  eb07                   -jmp 0x46df2d
    goto L_0x0046df2d;
L_0x0046df26:
    // 0046df26  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046df29  668b4078               -mov ax, word ptr [eax + 0x78]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(120) /* 0x78 */);
L_0x0046df2d:
    // 0046df2d  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 0046df2e  e85d9a0600             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046df33:
    // 0046df33  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046df36  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046df39  8b4044                 -mov eax, dword ptr [eax + 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(68) /* 0x44 */);
    // 0046df3c  8b9282000000           -mov edx, dword ptr [edx + 0x82]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(130) /* 0x82 */);
    // 0046df42  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046df45  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046df48  8b75e4                 -mov esi, dword ptr [ebp - 0x1c]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 0046df4b  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046df4d  39f0                   +cmp eax, esi
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
    // 0046df4f  7d30                   -jge 0x46df81
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046df81;
    }
    // 0046df51  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046df54  83c070                 -add eax, 0x70
    (cpu.eax) += x86::reg32(x86::sreg32(112 /*0x70*/));
    // 0046df57  8b58fe                 -mov ebx, dword ptr [eax - 2]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(-2) /* -0x2 */);
    // 0046df5a  8b10                   -mov edx, dword ptr [eax]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax);
    // 0046df5c  e88fe20200             -call 0x49c1f0
    cpu.esp -= 4;
    nfs3hp::frontend::pointerInBox(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046df61  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046df64  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0046df67  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046df69  7409                   -je 0x46df74
    if (cpu.flags.zf)
    {
        goto L_0x0046df74;
    }
    // 0046df6b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046df6e  668b407e               -mov ax, word ptr [eax + 0x7e]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(126) /* 0x7e */);
    // 0046df72  eb07                   -jmp 0x46df7b
    goto L_0x0046df7b;
L_0x0046df74:
    // 0046df74  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 0046df77  668b407a               -mov ax, word ptr [eax + 0x7a]
    cpu.ax = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(122) /* 0x7a */);
L_0x0046df7b:
    // 0046df7b  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 0046df7c  e80f9a0600             -call 0x4d7990
    cpu.esp -= 4;
    nfs3hp::port::shapes::drawShape(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x0046df81:
    // 0046df81  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 0046df83  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 0046df85  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0046df87  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046df88  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046df89  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046df8a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046df8b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046df8c  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_46df90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046df90  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046df91  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046df92  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046df93  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046df94  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046df95  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046df96  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046df98  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 0046df9b  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0046df9d  ba17000000             -mov edx, 0x17
    cpu.edx = 23 /*0x17*/;
    // 0046dfa2  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 0046dfa4  e8b7c6fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dfa9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046dfab  0f84f4000000           -je 0x46e0a5
    if (cpu.flags.zf)
    {
        goto L_0x0046e0a5;
    }
    // 0046dfb1  a1307d6700             -mov eax, dword ptr [0x677d30]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */);
    // 0046dfb6  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 0046dfb9  b8c4ae5300             -mov eax, 0x53aec4
    cpu.eax = 5484228 /*0x53aec4*/;
    // 0046dfbe  e8ed7d0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dfc3  66894178               -mov word ptr [ecx + 0x78], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(120) /* 0x78 */) = cpu.ax;
    // 0046dfc7  b8ccae5300             -mov eax, 0x53aecc
    cpu.eax = 5484236 /*0x53aecc*/;
    // 0046dfcc  e8df7d0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dfd1  6689417a               -mov word ptr [ecx + 0x7a], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(122) /* 0x7a */) = cpu.ax;
    // 0046dfd5  b8d4ae5300             -mov eax, 0x53aed4
    cpu.eax = 5484244 /*0x53aed4*/;
    // 0046dfda  e8d17d0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dfdf  6689417c               -mov word ptr [ecx + 0x7c], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(124) /* 0x7c */) = cpu.ax;
    // 0046dfe3  b8dcae5300             -mov eax, 0x53aedc
    cpu.eax = 5484252 /*0x53aedc*/;
    // 0046dfe8  e8c37d0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dfed  6689417e               -mov word ptr [ecx + 0x7e], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(126) /* 0x7e */) = cpu.ax;
    // 0046dff1  b8ccae5300             -mov eax, 0x53aecc
    cpu.eax = 5484236 /*0x53aecc*/;
    // 0046dff6  e8b57d0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046dffb  66898180000000         -mov word ptr [ecx + 0x80], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(128) /* 0x80 */) = cpu.ax;
    // 0046e002  b8c4ae5300             -mov eax, 0x53aec4
    cpu.eax = 5484228 /*0x53aec4*/;
    // 0046e007  e8a47d0600             -call 0x4d5db0
    cpu.esp -= 4;
    nfs3hp::port::shapes::shapeByName(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e00c  668b918a000000         -mov dx, word ptr [ecx + 0x8a]
    cpu.dx = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(138) /* 0x8a */);
    // 0046e013  66898182000000         -mov word ptr [ecx + 0x82], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(130) /* 0x82 */) = cpu.ax;
    // 0046e01a  6685d2                 +test dx, dx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.dx & cpu.dx));
    // 0046e01d  7512                   -jne 0x46e031
    if (!cpu.flags.zf)
    {
        goto L_0x0046e031;
    }
    // 0046e01f  668b4106               -mov ax, word ptr [ecx + 6]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(6) /* 0x6 */);
    // 0046e023  66899188000000         -mov word ptr [ecx + 0x88], dx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(136) /* 0x88 */) = cpu.dx;
    // 0046e02a  6689818a000000         -mov word ptr [ecx + 0x8a], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(138) /* 0x8a */) = cpu.ax;
L_0x0046e031:
    // 0046e031  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 0046e036  66c787860000000000     -mov word ptr [edi + 0x86], 0
    app->getMemory<x86::reg16>(cpu.edi + x86::reg32(134) /* 0x86 */) = 0 /*0x0*/;
    // 0046e03f  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0046e041  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0046e043  66c787840000000000     -mov word ptr [edi + 0x84], 0
    app->getMemory<x86::reg16>(cpu.edi + x86::reg32(132) /* 0x84 */) = 0 /*0x0*/;
    // 0046e04c  e8bf7bfeff             -call 0x455c10
    cpu.esp -= 4;
    sub_455c10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e051  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
L_0x0046e053:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 0046e053  3b75fc                 +cmp esi, dword ptr [ebp - 4]
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e056  7d4d                   -jge 0x46e0a5
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x0046e0a5;
    }
    // 0046e058  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 0046e05a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 0046e05c  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 0046e05e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 0046e060  e86b7cfeff             -call 0x455cd0
    cpu.esp -= 4;
    sub_455cd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e065  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046e067  743c                   -je 0x46e0a5
    if (cpu.flags.zf)
    {
        goto L_0x0046e0a5;
    }
    // 0046e069  8b1df4d46f00           -mov ebx, dword ptr [0x6fd4f4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
    // 0046e06f  8b88f0010000           -mov ecx, dword ptr [eax + 0x1f0]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(496) /* 0x1f0 */);
    // 0046e075  39d9                   +cmp ecx, ebx
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e077  740f                   -je 0x46e088
    if (cpu.flags.zf)
    {
        goto L_0x0046e088;
    }
    // 0046e079  8b15b0d36f00           -mov edx, dword ptr [0x6fd3b0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 0046e07f  83fa01                 +cmp edx, 1
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
    // 0046e082  751e                   -jne 0x46e0a2
    if (!cpu.flags.zf)
    {
        goto L_0x0046e0a2;
    }
    // 0046e084  39d1                   +cmp ecx, edx
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
    // 0046e086  751a                   -jne 0x46e0a2
    if (!cpu.flags.zf)
    {
        goto L_0x0046e0a2;
    }
L_0x0046e088:
    // 0046e088  8b4744                 -mov eax, dword ptr [edi + 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(68) /* 0x44 */);
    // 0046e08b  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e08e  39c6                   +cmp esi, eax
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 0046e090  7c10                   -jl 0x46e0a2
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0046e0a2;
    }
    // 0046e092  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 0046e094  662b4746               +sub ax, word ptr [edi + 0x46]
    {
        x86::reg16& tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(app->getMemory<x86::reg16>(cpu.edi + x86::reg32(70) /* 0x46 */)));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 0046e098  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0046e099  66898784000000         -mov word ptr [edi + 0x84], ax
    app->getMemory<x86::reg16>(cpu.edi + x86::reg32(132) /* 0x84 */) = cpu.ax;
    // 0046e0a0  eb03                   -jmp 0x46e0a5
    goto L_0x0046e0a5;
L_0x0046e0a2:
    // 0046e0a2  46                     +inc esi
    {
        x86::reg32& tmp = cpu.esi;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 0046e0a3  ebae                   -jmp 0x46e053
    goto L_0x0046e053;
L_0x0046e0a5:
    // 0046e0a5  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 0046e0a7  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e0a8  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e0a9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e0aa  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e0ab  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e0ac  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e0ad  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_46e0b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046e0b0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046e0b1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046e0b2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046e0b4  ba17000000             -mov edx, 0x17
    cpu.edx = 23 /*0x17*/;
    // 0046e0b9  e8a2c5fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e0be  85c0                   -test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046e0c0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e0c1  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e0c2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_46e0d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 0046e0d0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 0046e0d1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 0046e0d2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 0046e0d3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 0046e0d4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 0046e0d5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 0046e0d6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 0046e0d8  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 0046e0da  ba17000000             -mov edx, 0x17
    cpu.edx = 23 /*0x17*/;
    // 0046e0df  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 0046e0e1  e87ac5fdff             -call 0x44a660
    cpu.esp -= 4;
    nfs3hp::frontend::menuShownNow(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 0046e0e6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 0046e0e8  0f845d010000           -je 0x46e24b
    if (cpu.flags.zf)
    {
        goto L_0x0046e24b;
    }
    // 0046e0ee  8b4344                 -mov eax, dword ptr [ebx + 0x44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(68) /* 0x44 */);
    // 0046e0f1  8b15307d6700           -mov edx, dword ptr [0x677d30]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6782256) /* 0x677d30 */);
    // 0046e0f7  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 0046e0fa  39d0                   +cmp eax, edx
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
    // 0046e0fc  7c02                   -jl 0x46e100
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x0046e100;
    }
    // 0046e0fe  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
L_0x0046e100:
    // 0046e100  8b593c                 -mov ebx, dword ptr [ecx + 0x3c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(60) /* 0x3c */);
    // 0046e103  8b513a                 -mov edx, dword ptr [ecx + 0x3a]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(58) /* 0x3a */);
    // 0046e106  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0046e109  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e10c  01d3                   -add ebx, edx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046e10e  8b513e                 -mov edx, dword ptr [ecx + 0x3e]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(62) /* 0x3e */);
    // 0046e111  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e114  01da                   -add edx, ebx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0046e116  8b5940                 -mov ebx, dword ptr [ecx + 0x40]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 0046e119  c1fb10                 -sar ebx, 0x10
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (16 /*0x10*/ % 32));
    // 0046e11c  01d3                   -add ebx, edx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046e11e  8b5142                 -mov edx, dword ptr [ecx + 0x42]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(66) /* 0x42 */);
    // 0046e121  40                     -inc eax
    (cpu.eax)++;
    // 0046e122  c1fa10                 -sar edx, 0x10
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (16 /*0x10*/ % 32));
    // 0046e125  0fafd0                 -imul edx, eax
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.eax)));
    // 0046e128  668b818a000000         -mov ax, word ptr [ecx + 0x8a]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(138) /* 0x8a */);
    // 0046e12f  6689414a               -mov word ptr [ecx + 0x4a], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(74) /* 0x4a */) = cpu.ax;
    // 0046e133  668b4108               -mov ax, word ptr [ecx + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046e137  66894148               -mov word ptr [ecx + 0x48], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(72) /* 0x48 */) = cpu.ax;
    // 0046e13b  668b413c               -mov ax, word ptr [ecx + 0x3c]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(60) /* 0x3c */);
    // 0046e13f  6689414c               -mov word ptr [ecx + 0x4c], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(76) /* 0x4c */) = cpu.ax;
    // 0046e143  668b4144               -mov ax, word ptr [ecx + 0x44]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(68) /* 0x44 */);
    // 0046e147  6689414e               -mov word ptr [ecx + 0x4e], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(78) /* 0x4e */) = cpu.ax;
    // 0046e14b  668b713c               -mov si, word ptr [ecx + 0x3c]
    cpu.si = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(60) /* 0x3c */);
    // 0046e14f  668b818a000000         -mov ax, word ptr [ecx + 0x8a]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(138) /* 0x8a */);
    // 0046e156  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 0046e158  66894152               -mov word ptr [ecx + 0x52], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(82) /* 0x52 */) = cpu.ax;
    // 0046e15c  668b4108               -mov ax, word ptr [ecx + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046e160  66894150               -mov word ptr [ecx + 0x50], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(80) /* 0x50 */) = cpu.ax;
    // 0046e164  668b413e               -mov ax, word ptr [ecx + 0x3e]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(62) /* 0x3e */);
    // 0046e168  66894154               -mov word ptr [ecx + 0x54], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(84) /* 0x54 */) = cpu.ax;
    // 0046e16c  668b793c               -mov di, word ptr [ecx + 0x3c]
    cpu.di = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(60) /* 0x3c */);
    // 0046e170  668b4144               -mov ax, word ptr [ecx + 0x44]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(68) /* 0x44 */);
    // 0046e174  668bb18a000000         -mov si, word ptr [ecx + 0x8a]
    cpu.si = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(138) /* 0x8a */);
    // 0046e17b  66894156               -mov word ptr [ecx + 0x56], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(86) /* 0x56 */) = cpu.ax;
    // 0046e17f  01fe                   -add esi, edi
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.edi));
    // 0046e181  668b413e               -mov ax, word ptr [ecx + 0x3e]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(62) /* 0x3e */);
    // 0046e185  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046e187  668b4108               -mov ax, word ptr [ecx + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046e18b  6689715a               -mov word ptr [ecx + 0x5a], si
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(90) /* 0x5a */) = cpu.si;
    // 0046e18f  66894158               -mov word ptr [ecx + 0x58], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(88) /* 0x58 */) = cpu.ax;
    // 0046e193  668b4140               -mov ax, word ptr [ecx + 0x40]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 0046e197  6689415c               -mov word ptr [ecx + 0x5c], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(92) /* 0x5c */) = cpu.ax;
    // 0046e19b  668b4144               -mov ax, word ptr [ecx + 0x44]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(68) /* 0x44 */);
    // 0046e19f  6689415e               -mov word ptr [ecx + 0x5e], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(94) /* 0x5e */) = cpu.ax;
    // 0046e1a3  668b713c               -mov si, word ptr [ecx + 0x3c]
    cpu.si = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(60) /* 0x3c */);
    // 0046e1a7  668b818a000000         -mov ax, word ptr [ecx + 0x8a]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(138) /* 0x8a */);
    // 0046e1ae  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 0046e1b0  668b713e               -mov si, word ptr [ecx + 0x3e]
    cpu.si = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(62) /* 0x3e */);
    // 0046e1b4  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046e1b6  668b4140               -mov ax, word ptr [ecx + 0x40]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(64) /* 0x40 */);
    // 0046e1ba  01c6                   -add esi, eax
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.eax));
    // 0046e1bc  668b4108               -mov ax, word ptr [ecx + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046e1c0  66894160               -mov word ptr [ecx + 0x60], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(96) /* 0x60 */) = cpu.ax;
    // 0046e1c4  668b4142               -mov ax, word ptr [ecx + 0x42]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(66) /* 0x42 */);
    // 0046e1c8  66894164               -mov word ptr [ecx + 0x64], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(100) /* 0x64 */) = cpu.ax;
    // 0046e1cc  668b4144               -mov ax, word ptr [ecx + 0x44]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(68) /* 0x44 */);
    // 0046e1d0  66894166               -mov word ptr [ecx + 0x66], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(102) /* 0x66 */) = cpu.ax;
    // 0046e1d4  668b818a000000         -mov ax, word ptr [ecx + 0x8a]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(138) /* 0x8a */);
    // 0046e1db  6689411c               -mov word ptr [ecx + 0x1c], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(28) /* 0x1c */) = cpu.ax;
    // 0046e1df  668b4108               -mov ax, word ptr [ecx + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046e1e3  6689411a               -mov word ptr [ecx + 0x1a], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(26) /* 0x1a */) = cpu.ax;
    // 0046e1e7  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 0046e1e9  0520000000             -add eax, 0x20
    (cpu.eax) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 0046e1ee  6689411e               -mov word ptr [ecx + 0x1e], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(30) /* 0x1e */) = cpu.ax;
    // 0046e1f2  668b818a000000         -mov ax, word ptr [ecx + 0x8a]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(138) /* 0x8a */);
    // 0046e1f9  66c7416c0c00           -mov word ptr [ecx + 0x6c], 0xc
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(108) /* 0x6c */) = 12 /*0xc*/;
    // 0046e1ff  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0046e201  66c7416e0c00           -mov word ptr [ecx + 0x6e], 0xc
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(110) /* 0x6e */) = 12 /*0xc*/;
    // 0046e207  0514000000             -add eax, 0x14
    (cpu.eax) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0046e20c  6689416a               -mov word ptr [ecx + 0x6a], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(106) /* 0x6a */) = cpu.ax;
    // 0046e210  668b4108               -mov ax, word ptr [ecx + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046e214  66894168               -mov word ptr [ecx + 0x68], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(104) /* 0x68 */) = cpu.ax;
    // 0046e218  668b818a000000         -mov ax, word ptr [ecx + 0x8a]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(138) /* 0x8a */);
    // 0046e21f  66c741740c00           -mov word ptr [ecx + 0x74], 0xc
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(116) /* 0x74 */) = 12 /*0xc*/;
    // 0046e225  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 0046e227  66c741760c00           -mov word ptr [ecx + 0x76], 0xc
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(118) /* 0x76 */) = 12 /*0xc*/;
    // 0046e22d  0514000000             -add eax, 0x14
    (cpu.eax) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 0046e232  66894172               -mov word ptr [ecx + 0x72], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(114) /* 0x72 */) = cpu.ax;
    // 0046e236  668b4108               -mov ax, word ptr [ecx + 8]
    cpu.ax = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 0046e23a  66895120               -mov word ptr [ecx + 0x20], dx
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(32) /* 0x20 */) = cpu.dx;
    // 0046e23e  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 0046e240  66897162               -mov word ptr [ecx + 0x62], si
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(98) /* 0x62 */) = cpu.si;
    // 0046e244  83e80c                 -sub eax, 0xc
    (cpu.eax) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 0046e247  66894170               -mov word ptr [ecx + 0x70], ax
    app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(112) /* 0x70 */) = cpu.ax;
L_0x0046e24b:
    // 0046e24b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e24c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e24d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e24e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e24f  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e250  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 0046e251  c3                     -ret 
    cpu.esp += 4;
    return;
}

}
