#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4fcb00: native, nfs3hp::dead::stub_0x4fcb00 */

/* sub_4fcb50: native, nfs3hp::dead::stub_0x4fcb50 */

/* sub_4fcb70: native, nfs3hp::dead::stub_0x4fcb70 */

/* sub_4fcba0: native, nfs3hp::dead::stub_0x4fcba0 */

/* sub_4fcbc0: native, nfs3hp::dead::stub_0x4fcbc0 */

/* sub_4fcc90: native, nfs3hp::dead::stub_0x4fcc90 */

/* sub_4fcce0: native, nfs3hp::dead::stub_0x4fcce0 */

/* sub_4fce20: native, nfs3hp::dead::stub_0x4fce20 */

/* sub_4fce30: native, nfs3hp::dead::stub_0x4fce30 */

/* sub_4fce40: native, nfs3hp::dead::stub_0x4fce40 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_4fce50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fce50  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004fce51  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004fce52  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004fce53  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004fce55  b27c                   -mov dl, 0x7c
    cpu.dl = 124 /*0x7c*/;
    // 004fce57  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
L_0x004fce59:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fce59  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004fce5b  3ac2                   +cmp al, dl
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
    // 004fce5d  7412                   -je 0x4fce71
    if (cpu.flags.zf)
    {
        goto L_0x004fce71;
    }
    // 004fce5f  3c00                   +cmp al, 0
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
    // 004fce61  740c                   -je 0x4fce6f
    if (cpu.flags.zf)
    {
        goto L_0x004fce6f;
    }
    // 004fce63  46                     -inc esi
    (cpu.esi)++;
    // 004fce64  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004fce66  3ac2                   +cmp al, dl
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
    // 004fce68  7407                   -je 0x4fce71
    if (cpu.flags.zf)
    {
        goto L_0x004fce71;
    }
    // 004fce6a  46                     -inc esi
    (cpu.esi)++;
    // 004fce6b  3c00                   +cmp al, 0
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
    // 004fce6d  75ea                   -jne 0x4fce59
    if (!cpu.flags.zf)
    {
        goto L_0x004fce59;
    }
L_0x004fce6f:
    // 004fce6f  2bf6                   -sub esi, esi
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.esi));
L_0x004fce71:
    // 004fce71  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004fce73  7406                   -je 0x4fce7b
    if (cpu.flags.zf)
    {
        goto L_0x004fce7b;
    }
    // 004fce75  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004fce77  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fce78  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fce79  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fce7a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004fce7b:
    // 004fce7b  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004fce7d  e8ae64feff             -call 0x4e3330
    cpu.esp -= 4;
    nfs3hp::ea::driveNumber(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fce82  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fce83  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fce84  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fce85  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4fce90: native, nfs3hp::dead::stub_0x4fce90 */

/* sub_4fcea0: native, nfs3hp::dead::stub_0x4fcea0 */

/* sub_4fceb0: native, nfs3hp::dead::stub_0x4fceb0 */

/* sub_4fcf40: native, nfs3hp::dead::stub_0x4fcf40 */

/* sub_4fcfe0: native, nfs3hp::dead::stub_0x4fcfe0 */

/* sub_4fd020: native, nfs3hp::dead::stub_0x4fd020 */

/* sub_4fd040: native, nfs3hp::dead::stub_0x4fd040 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4fd060(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd060  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004fd061  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004fd062  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004fd064  e827faffff             -call 0x4fca90
    cpu.esp -= 4;
    sub_4fca90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fd069  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd06b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004fd06d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004fd06f  7505                   -jne 0x4fd076
    if (!cpu.flags.zf)
    {
        goto L_0x004fd076;
    }
    // 004fd071  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004fd073  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd074  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd075  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004fd076:
    // 004fd076  e805f7ffff             -call 0x4fc780
    cpu.esp -= 4;
    sub_4fc780(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fd07b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004fd07d  e87ef5ffff             -call 0x4fc600
    cpu.esp -= 4;
    sub_4fc600(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fd082  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004fd084  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004fd086  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd087  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd088  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4fd090: native, nfs3hp::dead::stub_0x4fd090 */

/* sub_4fd0c0: native, nfs3hp::dead::stub_0x4fd0c0 */

/* sub_4fd0f0: native, nfs3hp::dead::stub_0x4fd0f0 */

/* sub_4fd130: native, nfs3hp::dead::stub_0x4fd130 */

/* sub_4fd160: native, nfs3hp::dead::stub_0x4fd160 */

/* sub_4fd190: native, nfs3hp::dead::stub_0x4fd190 */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4fd1c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd1c0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004fd1c1  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004fd1c3  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004fd1c5  7504                   -jne 0x4fd1cb
    if (!cpu.flags.zf)
    {
        goto L_0x004fd1cb;
    }
L_0x004fd1c7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fd1c7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004fd1c9  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd1ca  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004fd1cb:
    // 004fd1cb  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 004fd1cd  83e07f                 -and eax, 0x7f
    cpu.eax &= x86::reg32(x86::sreg32(127 /*0x7f*/));
    // 004fd1d0  8a80546d5600           -mov al, byte ptr [eax + 0x566d54]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5664084) /* 0x566d54 */);
    // 004fd1d6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004fd1d8  744c                   -je 0x4fd226
    if (cpu.flags.zf)
    {
        goto L_0x004fd226;
    }
L_0x004fd1da:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fd1da  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004fd1dc  74e9                   -je 0x4fd1c7
    if (cpu.flags.zf)
    {
        goto L_0x004fd1c7;
    }
    // 004fd1de  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004fd1e0  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004fd1e5  83f822                 +cmp eax, 0x22
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(34 /*0x22*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fd1e8  0f8479000000           -je 0x4fd267
    if (cpu.flags.zf)
    {
        goto L_0x004fd267;
    }
    // 004fd1ee  83f824                 +cmp eax, 0x24
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(36 /*0x24*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fd1f1  0f8470000000           -je 0x4fd267
    if (cpu.flags.zf)
    {
        goto L_0x004fd267;
    }
    // 004fd1f7  83f823                 +cmp eax, 0x23
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(35 /*0x23*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fd1fa  0f8467000000           -je 0x4fd267
    if (cpu.flags.zf)
    {
        goto L_0x004fd267;
    }
    // 004fd200  83f829                 +cmp eax, 0x29
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(41 /*0x29*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fd203  7462                   -je 0x4fd267
    if (cpu.flags.zf)
    {
        goto L_0x004fd267;
    }
    // 004fd205  83f82a                 +cmp eax, 0x2a
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(42 /*0x2a*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fd208  745d                   -je 0x4fd267
    if (cpu.flags.zf)
    {
        goto L_0x004fd267;
    }
    // 004fd20a  83f82c                 +cmp eax, 0x2c
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(44 /*0x2c*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fd20d  7458                   -je 0x4fd267
    if (cpu.flags.zf)
    {
        goto L_0x004fd267;
    }
    // 004fd20f  83f82d                 +cmp eax, 0x2d
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(45 /*0x2d*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fd212  7453                   -je 0x4fd267
    if (cpu.flags.zf)
    {
        goto L_0x004fd267;
    }
    // 004fd214  83f82e                 +cmp eax, 0x2e
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(46 /*0x2e*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fd217  744e                   -je 0x4fd267
    if (cpu.flags.zf)
    {
        goto L_0x004fd267;
    }
    // 004fd219  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004fd21b  c1f808                 -sar eax, 8
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (8 /*0x8*/ % 32));
    // 004fd21e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004fd220  7449                   -je 0x4fd26b
    if (cpu.flags.zf)
    {
        goto L_0x004fd26b;
    }
    // 004fd222  01c2                   +add edx, eax
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
    // 004fd224  ebb4                   -jmp 0x4fd1da
    goto L_0x004fd1da;
L_0x004fd226:
    // 004fd226  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004fd227  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004fd228  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004fd229  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004fd22b  b94ce05400             -mov ecx, 0x54e04c
    cpu.ecx = 5562444 /*0x54e04c*/;
    // 004fd230  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004fd235  bb5ce05400             -mov ebx, 0x54e05c
    cpu.ebx = 5562460 /*0x54e05c*/;
    // 004fd23a  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004fd23b  be46000000             -mov esi, 0x46
    cpu.esi = 70 /*0x46*/;
    // 004fd240  890d90215500           -mov dword ptr [0x552190], ecx
    app->getMemory<x86::reg32>(x86::reg32(5579152) /* 0x552190 */) = cpu.ecx;
    // 004fd246  686ce05400             -push 0x54e06c
    app->getMemory<x86::reg32>(cpu.esp-4) = 5562476 /*0x54e06c*/;
    cpu.esp -= 4;
    // 004fd24b  891d94215500           -mov dword ptr [0x552194], ebx
    app->getMemory<x86::reg32>(x86::reg32(5579156) /* 0x552194 */) = cpu.ebx;
    // 004fd251  893598215500           -mov dword ptr [0x552198], esi
    app->getMemory<x86::reg32>(x86::reg32(5579160) /* 0x552198 */) = cpu.esi;
    // 004fd257  e8b43df0ff             -call 0x401010
    cpu.esp -= 4;
    nfs3hp::Error(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fd25c  83c408                 +add esp, 8
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004fd25f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd260  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd261  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd262  e973ffffff             -jmp 0x4fd1da
    goto L_0x004fd1da;
L_0x004fd267:
    // 004fd267  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004fd269  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd26a  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004fd26b:
    // 004fd26b  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004fd26d  e968ffffff             -jmp 0x4fd1da
    goto L_0x004fd1da;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4fd280(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd280  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004fd281  8b0d24445600           -mov ecx, dword ptr [0x564424]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5653540) /* 0x564424 */);
    // 004fd287  a32c445600             -mov dword ptr [0x56442c], eax
    app->getMemory<x86::reg32>(x86::reg32(5653548) /* 0x56442c */) = cpu.eax;
    // 004fd28c  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd28e  a334445600             -mov dword ptr [0x564434], eax
    app->getMemory<x86::reg32>(x86::reg32(5653556) /* 0x564434 */) = cpu.eax;
    // 004fd293  a128445600             -mov eax, dword ptr [0x564428]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653544) /* 0x564428 */);
    // 004fd298  891530445600           -mov dword ptr [0x564430], edx
    app->getMemory<x86::reg32>(x86::reg32(5653552) /* 0x564430 */) = cpu.edx;
    // 004fd29e  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004fd2a0  891538445600           -mov dword ptr [0x564438], edx
    app->getMemory<x86::reg32>(x86::reg32(5653560) /* 0x564438 */) = cpu.edx;
    // 004fd2a6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd2a7  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4fd2b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd2b0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004fd2b1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004fd2b2  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004fd2b4  89154c445600           -mov dword ptr [0x56444c], edx
    app->getMemory<x86::reg32>(x86::reg32(5653580) /* 0x56444c */) = cpu.edx;
    // 004fd2ba  d1fb                   -sar ebx, 1
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (1 /*0x1*/ % 32));
    // 004fd2bc  8b152c445600           -mov edx, dword ptr [0x56442c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653548) /* 0x56442c */);
    // 004fd2c2  a348445600             -mov dword ptr [0x564448], eax
    app->getMemory<x86::reg32>(x86::reg32(5653576) /* 0x564448 */) = cpu.eax;
    // 004fd2c7  01da                   -add edx, ebx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004fd2c9  d1f9                   -sar ecx, 1
    cpu.ecx = x86::reg32(x86::sreg32(cpu.ecx) >> (1 /*0x1*/ % 32));
    // 004fd2cb  891534445600           -mov dword ptr [0x564434], edx
    app->getMemory<x86::reg32>(x86::reg32(5653556) /* 0x564434 */) = cpu.edx;
    // 004fd2d1  8b1530445600           -mov edx, dword ptr [0x564430]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653552) /* 0x564430 */);
    // 004fd2d7  891d24445600           -mov dword ptr [0x564424], ebx
    app->getMemory<x86::reg32>(x86::reg32(5653540) /* 0x564424 */) = cpu.ebx;
    // 004fd2dd  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd2df  890d28445600           -mov dword ptr [0x564428], ecx
    app->getMemory<x86::reg32>(x86::reg32(5653544) /* 0x564428 */) = cpu.ecx;
    // 004fd2e5  891538445600           -mov dword ptr [0x564438], edx
    app->getMemory<x86::reg32>(x86::reg32(5653560) /* 0x564438 */) = cpu.edx;
    // 004fd2eb  8d8800010000           -lea ecx, [eax + 0x100]
    cpu.ecx = x86::reg32(cpu.eax + x86::reg32(256) /* 0x100 */);
    // 004fd2f1  c0e507                 +shl ch, 7
    {
        x86::reg8 tmp = 7 /*0x7*/ % 32;
        x86::reg8& op = cpu.ch;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (8 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (8 - 1))));
        }
    }
    // 004fd2f4  19d2                   -sbb edx, edx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004fd2f6  00ed                   +add ch, ch
    {
        x86::reg8& tmp1 = cpu.ch;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.ch));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004fd2f8  19c9                   -sbb ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx) + cpu.flags.cf);
    // 004fd2fa  31c8                   -xor eax, ecx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd2fc  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004fd301  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd303  8b0485ac725600         -mov eax, dword ptr [eax*4 + 0x5672ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5665452) /* 0x5672ac */ + cpu.eax * 4);
    // 004fd30a  31d0                   -xor eax, edx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004fd30c  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004fd30e  8b1d24445600           -mov ebx, dword ptr [0x564424]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(5653540) /* 0x564424 */);
    // 004fd314  0fafd8                 -imul ebx, eax
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(cpu.eax)));
    // 004fd317  a148445600             -mov eax, dword ptr [0x564448]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653576) /* 0x564448 */);
    // 004fd31c  c0e407                 +shl ah, 7
    {
        x86::reg8 tmp = 7 /*0x7*/ % 32;
        x86::reg8& op = cpu.ah;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (8 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (8 - 1))));
        }
    }
    // 004fd31f  19d2                   -sbb edx, edx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004fd321  00e4                   +add ah, ah
    {
        x86::reg8& tmp1 = cpu.ah;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.ah));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004fd323  19c9                   -sbb ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx) + cpu.flags.cf);
    // 004fd325  31c8                   -xor eax, ecx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd327  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004fd32c  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd32e  8b0485ac725600         -mov eax, dword ptr [eax*4 + 0x5672ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5665452) /* 0x5672ac */ + cpu.eax * 4);
    // 004fd335  31d0                   -xor eax, edx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004fd337  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004fd339  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004fd33b  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004fd33d  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004fd33f  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004fd342  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004fd344  a33c445600             -mov dword ptr [0x56443c], eax
    app->getMemory<x86::reg32>(x86::reg32(5653564) /* 0x56443c */) = cpu.eax;
    // 004fd349  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004fd34b  746a                   -je 0x4fd3b7
    if (cpu.flags.zf)
    {
        goto L_0x004fd3b7;
    }
    // 004fd34d  a14c445600             -mov eax, dword ptr [0x56444c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653580) /* 0x56444c */);
    // 004fd352  8d8800010000           -lea ecx, [eax + 0x100]
    cpu.ecx = x86::reg32(cpu.eax + x86::reg32(256) /* 0x100 */);
    // 004fd358  c0e507                 +shl ch, 7
    {
        x86::reg8 tmp = 7 /*0x7*/ % 32;
        x86::reg8& op = cpu.ch;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (8 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (8 - 1))));
        }
    }
    // 004fd35b  19d2                   -sbb edx, edx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004fd35d  00ed                   +add ch, ch
    {
        x86::reg8& tmp1 = cpu.ch;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.ch));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004fd35f  19c9                   -sbb ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx) + cpu.flags.cf);
    // 004fd361  31c8                   -xor eax, ecx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd363  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004fd368  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd36a  8b0485ac725600         -mov eax, dword ptr [eax*4 + 0x5672ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5665452) /* 0x5672ac */ + cpu.eax * 4);
    // 004fd371  31d0                   -xor eax, edx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004fd373  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004fd375  8b1528445600           -mov edx, dword ptr [0x564428]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653544) /* 0x564428 */);
    // 004fd37b  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004fd37d  0fafda                 -imul ebx, edx
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004fd380  a14c445600             -mov eax, dword ptr [0x56444c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5653580) /* 0x56444c */);
    // 004fd385  c0e407                 +shl ah, 7
    {
        x86::reg8 tmp = 7 /*0x7*/ % 32;
        x86::reg8& op = cpu.ah;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (8 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (8 - 1))));
        }
    }
    // 004fd388  19d2                   -sbb edx, edx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004fd38a  00e4                   +add ah, ah
    {
        x86::reg8& tmp1 = cpu.ah;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.ah));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004fd38c  19c9                   -sbb ecx, ecx
    (cpu.ecx) -= x86::reg32(x86::sreg32(cpu.ecx) + cpu.flags.cf);
    // 004fd38e  31c8                   -xor eax, ecx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd390  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004fd395  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd397  8b0485ac725600         -mov eax, dword ptr [eax*4 + 0x5672ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5665452) /* 0x5672ac */ + cpu.eax * 4);
    // 004fd39e  31d0                   -xor eax, edx
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004fd3a0  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004fd3a2  f7db                   -neg ebx
    cpu.ebx = ~cpu.ebx + 1;
    // 004fd3a4  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004fd3a6  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004fd3a8  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004fd3aa  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004fd3ad  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004fd3af  a340445600             -mov dword ptr [0x564440], eax
    app->getMemory<x86::reg32>(x86::reg32(5653568) /* 0x564440 */) = cpu.eax;
    // 004fd3b4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd3b5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd3b6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004fd3b7:
    // 004fd3b7  8b1550445600           -mov edx, dword ptr [0x564450]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653584) /* 0x564450 */);
    // 004fd3bd  f7ea                   -imul edx
    cpu.edx_eax = x86::reg64(x86::sreg64(static_cast<x86::sreg32>(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004fd3bf  c1e210                 -shl edx, 0x10
    cpu.edx <<= 16 /*0x10*/ % 32;
    // 004fd3c2  c1e810                 +shr eax, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 004fd3c5  11d0                   -adc eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004fd3c7  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 004fd3c9  8b1528445600           -mov edx, dword ptr [0x564428]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5653544) /* 0x564428 */);
    // 004fd3cf  f7dd                   -neg ebp
    cpu.ebp = ~cpu.ebp + 1;
    // 004fd3d1  a340445600             -mov dword ptr [0x564440], eax
    app->getMemory<x86::reg32>(x86::reg32(5653568) /* 0x564440 */) = cpu.eax;
    // 004fd3d6  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 004fd3d8  892d40445600           -mov dword ptr [0x564440], ebp
    app->getMemory<x86::reg32>(x86::reg32(5653568) /* 0x564440 */) = cpu.ebp;
    // 004fd3de  e845bbfeff             -call 0x4e8f28
    cpu.esp -= 4;
    nfs3hp::port::SNDangle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fd3e3  a34c445600             -mov dword ptr [0x56444c], eax
    app->getMemory<x86::reg32>(x86::reg32(5653580) /* 0x56444c */) = cpu.eax;
    // 004fd3e8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd3e9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd3ea  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4fd3f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd3f0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004fd3f1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004fd3f2  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004fd3f4  c1e20a                 -shl edx, 0xa
    cpu.edx <<= 10 /*0xa*/ % 32;
    // 004fd3f7  be68010000             -mov esi, 0x168
    cpu.esi = 360 /*0x168*/;
    // 004fd3fc  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004fd3fe  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004fd401  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004fd403  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004fd405  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004fd407  c1e20a                 -shl edx, 0xa
    cpu.edx <<= 10 /*0xa*/ % 32;
    // 004fd40a  bf68010000             -mov edi, 0x168
    cpu.edi = 360 /*0x168*/;
    // 004fd40f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004fd411  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004fd414  f7ff                   -idiv edi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.edi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004fd416  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004fd418  e893feffff             -call 0x4fd2b0
    cpu.esp -= 4;
    sub_4fd2b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fd41d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd41e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd41f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4fd420: native, nfs3hp::dead::stub_0x4fd420 */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4fd490(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd490  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004fd492  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004fd494  8b1dac4f9f00           -mov ebx, dword ptr [0x9f4fac]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10440620) /* 0x9f4fac */);
    // 004fd49a  d1fa                   +sar edx, 1
    {
        x86::reg32 tmp = 1 /*0x1*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (x86::sreg32(op) >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = 0;
            cpu.set_szp((op = x86::reg32(x86::sreg32(op) >> tmp)));
        }
    }
    // 004fd49c  e98032feff             -jmp 0x4e0721
    return nfs3hp::fillWord(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4fd4b0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd4b0  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004fd4b2  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004fd4b4  8b1dac4f9f00           -mov ebx, dword ptr [0x9f4fac]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10440620) /* 0x9f4fac */);
    // 004fd4ba  e96232feff             -jmp 0x4e0721
    return nfs3hp::fillWord(app, cpu.handOver());
}

/* align: skip 0x90 */
void Application::sub_4fd4c0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd4c0  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004fd4c2  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004fd4c4  8b1dac4f9f00           -mov ebx, dword ptr [0x9f4fac]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10440620) /* 0x9f4fac */);
    // 004fd4ca  01d2                   +add edx, edx
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004fd4cc  e95032feff             -jmp 0x4e0721
    return nfs3hp::fillWord(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4fd4e0(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
L_0x004fd4e0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fd4e0  83c202                 +add edx, 2
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004fd4e3  66a1ac4f9f00           -mov ax, word ptr [0x9f4fac]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(10440620) /* 0x9f4fac */);
    // 004fd4e9  668942fe               -mov word ptr [edx - 2], ax
    app->getMemory<x86::reg16>(cpu.edx + x86::reg32(-2) /* -0x2 */) = cpu.ax;
    // 004fd4ed  4b                     +dec ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004fd4ee  75f0                   -jne 0x4fd4e0
    if (!cpu.flags.zf)
    {
        goto L_0x004fd4e0;
    }
    // 004fd4f0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4fd500(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
L_0x004fd500:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fd500  66a1ac4f9f00           -mov ax, word ptr [0x9f4fac]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(10440620) /* 0x9f4fac */);
    // 004fd506  668902                 -mov word ptr [edx], ax
    app->getMemory<x86::reg16>(cpu.edx) = cpu.ax;
    // 004fd509  a1ac4f9f00             -mov eax, dword ptr [0x9f4fac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10440620) /* 0x9f4fac */);
    // 004fd50e  83c203                 -add edx, 3
    (cpu.edx) += x86::reg32(x86::sreg32(3 /*0x3*/));
    // 004fd511  c1e810                 +shr eax, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 004fd514  8842ff                 -mov byte ptr [edx - 1], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(-1) /* -0x1 */) = cpu.al;
    // 004fd517  4b                     +dec ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004fd518  75e6                   -jne 0x4fd500
    if (!cpu.flags.zf)
    {
        goto L_0x004fd500;
    }
    // 004fd51a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4fd520(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd520  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004fd522  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004fd524  8b1dac4f9f00           -mov ebx, dword ptr [0x9f4fac]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10440620) /* 0x9f4fac */);
    // 004fd52a  c1e202                 +shl edx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004fd52d  e9ef31feff             -jmp 0x4e0721
    return nfs3hp::fillWord(app, cpu.handOver());
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_4fd540(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd540  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004fd541  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004fd542  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004fd543  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 004fd546  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004fd548  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004fd54a  8b442424               -mov eax, dword ptr [esp + 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(36) /* 0x24 */);
    // 004fd54e  8a1510505600           -mov dl, byte ptr [0x565010]
    cpu.dl = app->getMemory<x86::reg8>(x86::reg32(5656592) /* 0x565010 */);
    // 004fd554  80fa08                 +cmp dl, 8
    {
        x86::reg8 tmp1 = cpu.dl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(8 /*0x8*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004fd557  0f8381000000           -jae 0x4fd5de
    if (!cpu.flags.cf)
    {
        goto L_0x004fd5de;
    }
    // 004fd55d  80fa04                 +cmp dl, 4
    {
        x86::reg8 tmp1 = cpu.dl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(4 /*0x4*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004fd560  750a                   -jne 0x4fd56c
    if (!cpu.flags.zf)
    {
        goto L_0x004fd56c;
    }
    // 004fd562  83e00f                 -and eax, 0xf
    cpu.eax &= x86::reg32(x86::sreg32(15 /*0xf*/));
    // 004fd565  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004fd567  c1e204                 -shl edx, 4
    cpu.edx <<= 4 /*0x4*/ % 32;
L_0x004fd56a:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fd56a  09d0                   -or eax, edx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.edx));
L_0x004fd56c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fd56c  a3ac4f9f00             -mov dword ptr [0x9f4fac], eax
    app->getMemory<x86::reg32>(x86::reg32(10440620) /* 0x9f4fac */) = cpu.eax;
    // 004fd571  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004fd573  8a2510505600           -mov ah, byte ptr [0x565010]
    cpu.ah = app->getMemory<x86::reg8>(x86::reg32(5656592) /* 0x565010 */);
    // 004fd579  8915cc765600           -mov dword ptr [0x5676cc], edx
    app->getMemory<x86::reg32>(x86::reg32(5666508) /* 0x5676cc */) = cpu.edx;
    // 004fd57f  80fc10                 +cmp ah, 0x10
    {
        x86::reg8 tmp1 = cpu.ah;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(16 /*0x10*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004fd582  7513                   -jne 0x4fd597
    if (!cpu.flags.zf)
    {
        goto L_0x004fd597;
    }
    // 004fd584  833db076560000         +cmp dword ptr [0x5676b0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5666480) /* 0x5676b0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fd58b  740a                   -je 0x4fd597
    if (cpu.flags.zf)
    {
        goto L_0x004fd597;
    }
    // 004fd58d  c705cc765600e0d44f00   -mov dword ptr [0x5676cc], 0x4fd4e0
    app->getMemory<x86::reg32>(x86::reg32(5666508) /* 0x5676cc */) = 5231840 /*0x4fd4e0*/;
L_0x004fd597:
    // 004fd597  66895c2404             -mov word ptr [esp + 4], bx
    app->getMemory<x86::reg16>(cpu.esp + x86::reg32(4) /* 0x4 */) = cpu.bx;
    // 004fd59c  66894c2406             -mov word ptr [esp + 6], cx
    app->getMemory<x86::reg16>(cpu.esp + x86::reg32(6) /* 0x6 */) = cpu.cx;
    // 004fd5a1  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004fd5a3  30d2                   -xor dl, dl
    cpu.dl ^= x86::reg8(x86::sreg8(cpu.dl));
    // 004fd5a5  b9b4765600             -mov ecx, 0x5676b4
    cpu.ecx = 5666484 /*0x5676b4*/;
    // 004fd5aa  881424                 -mov byte ptr [esp], dl
    app->getMemory<x86::reg8>(cpu.esp) = cpu.dl;
    // 004fd5ad  a011505600             -mov al, byte ptr [0x565011]
    cpu.al = app->getMemory<x86::reg8>(x86::reg32(5656593) /* 0x565011 */);
    // 004fd5b2  8b1424                 -mov edx, dword ptr [esp]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    // 004fd5b5  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004fd5ba  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004fd5bc  09c2                   -or edx, eax
    cpu.edx |= x86::reg32(x86::sreg32(cpu.eax));
    // 004fd5be  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 004fd5c0  89d5                   -mov ebp, edx
    cpu.ebp = cpu.edx;
    // 004fd5c2  891424                 -mov dword ptr [esp], edx
    app->getMemory<x86::reg32>(cpu.esp) = cpu.edx;
    // 004fd5c5  81e5ff000000           -and ebp, 0xff
    cpu.ebp &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004fd5cb  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004fd5cd  892c24                 -mov dword ptr [esp], ebp
    app->getMemory<x86::reg32>(cpu.esp) = cpu.ebp;
    // 004fd5d0  e8dbda0000             -call 0x50b0b0
    cpu.esp -= 4;
    sub_50b0b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fd5d5  83c414                 +add esp, 0x14
    {
        x86::reg32& tmp1 = cpu.esp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(20 /*0x14*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004fd5d8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd5d9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd5da  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd5db  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
L_0x004fd5de:
    // 004fd5de  7716                   -ja 0x4fd5f6
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004fd5f6;
    }
    // 004fd5e0  25ff000000             -and eax, 0xff
    cpu.eax &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004fd5e5  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004fd5e7  c1e208                 -shl edx, 8
    cpu.edx <<= 8 /*0x8*/ % 32;
    // 004fd5ea  09d0                   -or eax, edx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.edx));
    // 004fd5ec  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004fd5ee  c1e210                 +shl edx, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004fd5f1  e974ffffff             -jmp 0x4fd56a
    goto L_0x004fd56a;
L_0x004fd5f6:
    // 004fd5f6  80fa0f                 +cmp dl, 0xf
    {
        x86::reg8 tmp1 = cpu.dl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(15 /*0xf*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004fd5f9  0f826dffffff           -jb 0x4fd56c
    if (cpu.flags.cf)
    {
        goto L_0x004fd56c;
    }
    // 004fd5ff  770f                   -ja 0x4fd610
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004fd610;
    }
L_0x004fd601:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fd601  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 004fd606  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004fd608  c1e210                 +shl edx, 0x10
    {
        x86::reg8 tmp = 16 /*0x10*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004fd60b  e95affffff             -jmp 0x4fd56a
    goto L_0x004fd56a;
L_0x004fd610:
    // 004fd610  80fa10                 +cmp dl, 0x10
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
    // 004fd613  74ec                   -je 0x4fd601
    if (cpu.flags.zf)
    {
        goto L_0x004fd601;
    }
    // 004fd615  e952ffffff             -jmp 0x4fd56c
    goto L_0x004fd56c;
}

/* align: skip 0x00 0x00 */
void Application::sub_4fd61c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd61c  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004fd61d  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004fd61e  891dd8765600           -mov dword ptr [0x5676d8], ebx
    app->getMemory<x86::reg32>(x86::reg32(5666520) /* 0x5676d8 */) = cpu.ebx;
    // 004fd624  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004fd626  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004fd627  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
L_0x004fd628:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fd628  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004fd62a  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004fd62c  88c3                   -mov bl, al
    cpu.bl = cpu.al;
    // 004fd62e  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004fd630  c1ef18                 -shr edi, 0x18
    cpu.edi >>= 24 /*0x18*/ % 32;
    // 004fd633  2500ff0000             -and eax, 0xff00
    cpu.eax &= x86::reg32(x86::sreg32(65280 /*0xff00*/));
    // 004fd638  c1e808                 -shr eax, 8
    cpu.eax >>= 8 /*0x8*/ % 32;
    // 004fd63b  8b2c9d18629f00         -mov ebp, dword ptr [ebx*4 + 0x9f6218]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(10445336) /* 0x9f6218 */ + cpu.ebx * 4);
    // 004fd642  8b3cbd185e9f00         -mov edi, dword ptr [edi*4 + 0x9f5e18]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10444312) /* 0x9f5e18 */ + cpu.edi * 4);
    // 004fd649  83c604                 -add esi, 4
    (cpu.esi) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fd64c  8b048518609f00         -mov eax, dword ptr [eax*4 + 0x9f6018]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10444824) /* 0x9f6018 */ + cpu.eax * 4);
    // 004fd653  01ef                   -add edi, ebp
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.ebp));
    // 004fd655  01c5                   -add ebp, eax
    (cpu.ebp) += x86::reg32(x86::sreg32(cpu.eax));
    // 004fd657  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004fd659  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004fd65b  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004fd65d  c1e817                 -shr eax, 0x17
    cpu.eax >>= 23 /*0x17*/ % 32;
    // 004fd660  81e100f80f00           -and ecx, 0xff800
    cpu.ecx &= x86::reg32(x86::sreg32(1046528 /*0xff800*/));
    // 004fd666  c1e90b                 -shr ecx, 0xb
    cpu.ecx >>= 11 /*0xb*/ % 32;
    // 004fd669  81e7ff010000           -and edi, 0x1ff
    cpu.edi &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 004fd66f  8b048550599f00         -mov eax, dword ptr [eax*4 + 0x9f5950]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10443088) /* 0x9f5950 */ + cpu.eax * 4);
    // 004fd676  8a5efe                 -mov bl, byte ptr [esi - 2]
    cpu.bl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(-2) /* -0x2 */);
    // 004fd679  8b0c8d38559f00         -mov ecx, dword ptr [ecx*4 + 0x9f5538]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(10442040) /* 0x9f5538 */ + cpu.ecx * 4);
    // 004fd680  8b3cbdb04f9f00         -mov edi, dword ptr [edi*4 + 0x9f4fb0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10440624) /* 0x9f4fb0 */ + cpu.edi * 4);
    // 004fd687  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd689  8b1c9d185e9f00         -mov ebx, dword ptr [ebx*4 + 0x9f5e18]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10444312) /* 0x9f5e18 */ + cpu.ebx * 4);
    // 004fd690  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004fd692  01eb                   -add ebx, ebp
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.ebp));
    // 004fd694  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 004fd696  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004fd698  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 004fd69a  81e300f80f00           -and ebx, 0xff800
    cpu.ebx &= x86::reg32(x86::sreg32(1046528 /*0xff800*/));
    // 004fd6a0  c1e817                 -shr eax, 0x17
    cpu.eax >>= 23 /*0x17*/ % 32;
    // 004fd6a3  81e7ff010000           -and edi, 0x1ff
    cpu.edi &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 004fd6a9  c1eb0b                 -shr ebx, 0xb
    cpu.ebx >>= 11 /*0xb*/ % 32;
    // 004fd6ac  8b0dd8765600           -mov ecx, dword ptr [0x5676d8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5666520) /* 0x5676d8 */);
    // 004fd6b2  8b048550599f00         -mov eax, dword ptr [eax*4 + 0x9f5950]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10443088) /* 0x9f5950 */ + cpu.eax * 4);
    // 004fd6b9  8b3cbdb04f9f00         -mov edi, dword ptr [edi*4 + 0x9f4fb0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10440624) /* 0x9f4fb0 */ + cpu.edi * 4);
    // 004fd6c0  8b1c9d38559f00         -mov ebx, dword ptr [ebx*4 + 0x9f5538]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10442040) /* 0x9f5538 */ + cpu.ebx * 4);
    // 004fd6c7  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004fd6c9  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004fd6cb  81e5ffff0000           -and ebp, 0xffff
    cpu.ebp &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 004fd6d1  250000ffff             -and eax, 0xffff0000
    cpu.eax &= x86::reg32(x86::sreg32(4294901760 /*0xffff0000*/));
    // 004fd6d6  09e8                   +or eax, ebp
    cpu.clear_co();
    cpu.set_szp((cpu.eax |= x86::reg32(x86::sreg32(cpu.ebp))));
    // 004fd6d8  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004fd6d9  890dd8765600           -mov dword ptr [0x5676d8], ecx
    app->getMemory<x86::reg32>(x86::reg32(5666520) /* 0x5676d8 */) = cpu.ecx;
    // 004fd6df  8902                   -mov dword ptr [edx], eax
    app->getMemory<x86::reg32>(cpu.edx) = cpu.eax;
    // 004fd6e1  8d5204                 -lea edx, [edx + 4]
    cpu.edx = x86::reg32(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 004fd6e4  0f853effffff           -jne 0x4fd628
    if (!cpu.flags.zf)
    {
        goto L_0x004fd628;
    }
    // 004fd6ea  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd6eb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd6ec  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd6ed  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd6ee  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4fd6ef(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd6ef  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004fd6f0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004fd6f1  891dd8765600           -mov dword ptr [0x5676d8], ebx
    app->getMemory<x86::reg32>(x86::reg32(5666520) /* 0x5676d8 */) = cpu.ebx;
    // 004fd6f7  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004fd6f9  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004fd6fa  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
L_0x004fd6fb:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fd6fb  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004fd6fd  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004fd6ff  88c3                   -mov bl, al
    cpu.bl = cpu.al;
    // 004fd701  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004fd703  c1ef18                 -shr edi, 0x18
    cpu.edi >>= 24 /*0x18*/ % 32;
    // 004fd706  2500ff0000             -and eax, 0xff00
    cpu.eax &= x86::reg32(x86::sreg32(65280 /*0xff00*/));
    // 004fd70b  c1e808                 -shr eax, 8
    cpu.eax >>= 8 /*0x8*/ % 32;
    // 004fd70e  8b2c9d18629f00         -mov ebp, dword ptr [ebx*4 + 0x9f6218]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(10445336) /* 0x9f6218 */ + cpu.ebx * 4);
    // 004fd715  8b3cbd185e9f00         -mov edi, dword ptr [edi*4 + 0x9f5e18]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10444312) /* 0x9f5e18 */ + cpu.edi * 4);
    // 004fd71c  83c604                 -add esi, 4
    (cpu.esi) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fd71f  8b048518609f00         -mov eax, dword ptr [eax*4 + 0x9f6018]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10444824) /* 0x9f6018 */ + cpu.eax * 4);
    // 004fd726  01ef                   -add edi, ebp
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.ebp));
    // 004fd728  01c5                   -add ebp, eax
    (cpu.ebp) += x86::reg32(x86::sreg32(cpu.eax));
    // 004fd72a  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004fd72c  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004fd72e  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004fd730  c1e817                 -shr eax, 0x17
    cpu.eax >>= 23 /*0x17*/ % 32;
    // 004fd733  81e100f80f00           -and ecx, 0xff800
    cpu.ecx &= x86::reg32(x86::sreg32(1046528 /*0xff800*/));
    // 004fd739  c1e90b                 -shr ecx, 0xb
    cpu.ecx >>= 11 /*0xb*/ % 32;
    // 004fd73c  81e7ff010000           -and edi, 0x1ff
    cpu.edi &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 004fd742  8b048550599f00         -mov eax, dword ptr [eax*4 + 0x9f5950]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10443088) /* 0x9f5950 */ + cpu.eax * 4);
    // 004fd749  8a5efe                 -mov bl, byte ptr [esi - 2]
    cpu.bl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(-2) /* -0x2 */);
    // 004fd74c  8b0c8d38559f00         -mov ecx, dword ptr [ecx*4 + 0x9f5538]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(10442040) /* 0x9f5538 */ + cpu.ecx * 4);
    // 004fd753  8b3cbdb04f9f00         -mov edi, dword ptr [edi*4 + 0x9f4fb0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10440624) /* 0x9f4fb0 */ + cpu.edi * 4);
    // 004fd75a  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd75c  8b1c9d185e9f00         -mov ebx, dword ptr [ebx*4 + 0x9f5e18]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10444312) /* 0x9f5e18 */ + cpu.ebx * 4);
    // 004fd763  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004fd765  01eb                   -add ebx, ebp
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.ebp));
    // 004fd767  8902                   -mov dword ptr [edx], eax
    app->getMemory<x86::reg32>(cpu.edx) = cpu.eax;
    // 004fd769  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004fd76b  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 004fd76d  81e300f80f00           -and ebx, 0xff800
    cpu.ebx &= x86::reg32(x86::sreg32(1046528 /*0xff800*/));
    // 004fd773  c1e817                 -shr eax, 0x17
    cpu.eax >>= 23 /*0x17*/ % 32;
    // 004fd776  81e7ff010000           -and edi, 0x1ff
    cpu.edi &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 004fd77c  c1eb0b                 -shr ebx, 0xb
    cpu.ebx >>= 11 /*0xb*/ % 32;
    // 004fd77f  8b0dd8765600           -mov ecx, dword ptr [0x5676d8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5666520) /* 0x5676d8 */);
    // 004fd785  8b048550599f00         -mov eax, dword ptr [eax*4 + 0x9f5950]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10443088) /* 0x9f5950 */ + cpu.eax * 4);
    // 004fd78c  8b3cbdb04f9f00         -mov edi, dword ptr [edi*4 + 0x9f4fb0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(10440624) /* 0x9f4fb0 */ + cpu.edi * 4);
    // 004fd793  8b1c9d38559f00         -mov ebx, dword ptr [ebx*4 + 0x9f5538]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(10442040) /* 0x9f5538 */ + cpu.ebx * 4);
    // 004fd79a  01f8                   -add eax, edi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edi));
    // 004fd79c  01d8                   +add eax, ebx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004fd79e  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004fd79f  890dd8765600           -mov dword ptr [0x5676d8], ecx
    app->getMemory<x86::reg32>(x86::reg32(5666520) /* 0x5676d8 */) = cpu.ecx;
    // 004fd7a5  894204                 -mov dword ptr [edx + 4], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */) = cpu.eax;
    // 004fd7a8  8d5208                 -lea edx, [edx + 8]
    cpu.edx = x86::reg32(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 004fd7ab  0f854affffff           -jne 0x4fd6fb
    if (!cpu.flags.zf)
    {
        goto L_0x004fd6fb;
    }
    // 004fd7b1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd7b2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd7b3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd7b4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd7b5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4fd7b6(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd7b6  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004fd7b7  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004fd7b8  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004fd7ba  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004fd7bc  890dd8765600           -mov dword ptr [0x5676d8], ecx
    app->getMemory<x86::reg32>(x86::reg32(5666520) /* 0x5676d8 */) = cpu.ecx;
    // 004fd7c2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
L_0x004fd7c3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fd7c3  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004fd7c5  8b17                   -mov edx, dword ptr [edi]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edi);
    // 004fd7c7  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004fd7c9  83c604                 -add esi, 4
    (cpu.esi) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fd7cc  d1e8                   -shr eax, 1
    cpu.eax >>= 1 /*0x1*/ % 32;
    // 004fd7ce  83c704                 -add edi, 4
    (cpu.edi) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fd7d1  257f7f7f7f             -and eax, 0x7f7f7f7f
    cpu.eax &= x86::reg32(x86::sreg32(2139062143 /*0x7f7f7f7f*/));
    // 004fd7d6  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004fd7d8  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004fd7da  89c5                   -mov ebp, eax
    cpu.ebp = cpu.eax;
    // 004fd7dc  c1e908                 -shr ecx, 8
    cpu.ecx >>= 8 /*0x8*/ % 32;
    // 004fd7df  88c2                   -mov dl, al
    cpu.dl = cpu.al;
    // 004fd7e1  c1ed10                 -shr ebp, 0x10
    cpu.ebp >>= 16 /*0x10*/ % 32;
    // 004fd7e4  81e1ff000000           -and ecx, 0xff
    cpu.ecx &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004fd7ea  c1e818                 -shr eax, 0x18
    cpu.eax >>= 24 /*0x18*/ % 32;
    // 004fd7ed  8b149518629f00         -mov edx, dword ptr [edx*4 + 0x9f6218]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10445336) /* 0x9f6218 */ + cpu.edx * 4);
    // 004fd7f4  81e5ff000000           -and ebp, 0xff
    cpu.ebp &= x86::reg32(x86::sreg32(255 /*0xff*/));
    // 004fd7fa  8b0c8d18609f00         -mov ecx, dword ptr [ecx*4 + 0x9f6018]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(10444824) /* 0x9f6018 */ + cpu.ecx * 4);
    // 004fd801  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd803  8b0485185e9f00         -mov eax, dword ptr [eax*4 + 0x9f5e18]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10444312) /* 0x9f5e18 */ + cpu.eax * 4);
    // 004fd80a  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004fd80c  8b2cad185e9f00         -mov ebp, dword ptr [ebp*4 + 0x9f5e18]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(10444312) /* 0x9f5e18 */ + cpu.ebp * 4);
    // 004fd813  01d5                   -add ebp, edx
    (cpu.ebp) += x86::reg32(x86::sreg32(cpu.edx));
    // 004fd815  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004fd817  c1ea17                 -shr edx, 0x17
    cpu.edx >>= 23 /*0x17*/ % 32;
    // 004fd81a  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004fd81c  c1e90b                 -shr ecx, 0xb
    cpu.ecx >>= 11 /*0xb*/ % 32;
    // 004fd81f  25ff010000             -and eax, 0x1ff
    cpu.eax &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 004fd824  8b149550599f00         -mov edx, dword ptr [edx*4 + 0x9f5950]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10443088) /* 0x9f5950 */ + cpu.edx * 4);
    // 004fd82b  81e1ff010000           -and ecx, 0x1ff
    cpu.ecx &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 004fd831  8b0485b04f9f00         -mov eax, dword ptr [eax*4 + 0x9f4fb0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10440624) /* 0x9f4fb0 */ + cpu.eax * 4);
    // 004fd838  83c308                 -add ebx, 8
    (cpu.ebx) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004fd83b  8b0c8d38559f00         -mov ecx, dword ptr [ecx*4 + 0x9f5538]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(10442040) /* 0x9f5538 */ + cpu.ecx * 4);
    // 004fd842  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004fd844  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd846  89ea                   -mov edx, ebp
    cpu.edx = cpu.ebp;
    // 004fd848  c1ea17                 -shr edx, 0x17
    cpu.edx >>= 23 /*0x17*/ % 32;
    // 004fd84b  8943f8                 -mov dword ptr [ebx - 8], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004fd84e  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 004fd850  81e5ff010000           -and ebp, 0x1ff
    cpu.ebp &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 004fd856  c1e80b                 -shr eax, 0xb
    cpu.eax >>= 11 /*0xb*/ % 32;
    // 004fd859  8b149550599f00         -mov edx, dword ptr [edx*4 + 0x9f5950]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(10443088) /* 0x9f5950 */ + cpu.edx * 4);
    // 004fd860  25ff010000             -and eax, 0x1ff
    cpu.eax &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 004fd865  8b2cadb04f9f00         -mov ebp, dword ptr [ebp*4 + 0x9f4fb0]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(10440624) /* 0x9f4fb0 */ + cpu.ebp * 4);
    // 004fd86c  01ea                   -add edx, ebp
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ebp));
    // 004fd86e  8b0dd8765600           -mov ecx, dword ptr [0x5676d8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5666520) /* 0x5676d8 */);
    // 004fd874  8b048538559f00         -mov eax, dword ptr [eax*4 + 0x9f5538]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(10442040) /* 0x9f5538 */ + cpu.eax * 4);
    // 004fd87b  01d0                   +add eax, edx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004fd87d  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004fd87e  890dd8765600           -mov dword ptr [0x5676d8], ecx
    app->getMemory<x86::reg32>(x86::reg32(5666520) /* 0x5676d8 */) = cpu.ecx;
    // 004fd884  90                     -nop 
    ;
    // 004fd885  8943fc                 -mov dword ptr [ebx - 4], eax
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004fd888  0f8535ffffff           -jne 0x4fd7c3
    if (!cpu.flags.zf)
    {
        goto L_0x004fd7c3;
    }
    // 004fd88e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd88f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd890  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd891  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4fd892(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd892  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004fd893  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004fd894  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004fd895  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
L_0x004fd896:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fd896  8b30                   -mov esi, dword ptr [eax]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax);
    // 004fd898  8b7804                 -mov edi, dword ptr [eax + 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004fd89b  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 004fd89d  89fd                   -mov ebp, edi
    cpu.ebp = cpu.edi;
    // 004fd89f  c1e108                 -shl ecx, 8
    cpu.ecx <<= 8 /*0x8*/ % 32;
    // 004fd8a2  81e77f7f0000           -and edi, 0x7f7f
    cpu.edi &= x86::reg32(x86::sreg32(32639 /*0x7f7f*/));
    // 004fd8a8  c1ed08                 -shr ebp, 8
    cpu.ebp >>= 8 /*0x8*/ % 32;
    // 004fd8ab  81e10000007f           -and ecx, 0x7f000000
    cpu.ecx &= x86::reg32(x86::sreg32(2130706432 /*0x7f000000*/));
    // 004fd8b1  81e500007f00           -and ebp, 0x7f0000
    cpu.ebp &= x86::reg32(x86::sreg32(8323072 /*0x7f0000*/));
    // 004fd8b7  09cf                   -or edi, ecx
    cpu.edi |= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd8b9  09ef                   -or edi, ebp
    cpu.edi |= x86::reg32(x86::sreg32(cpu.ebp));
    // 004fd8bb  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fd8be  01f7                   -add edi, esi
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.esi));
    // 004fd8c0  83c208                 -add edx, 8
    (cpu.edx) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004fd8c3  d1ef                   -shr edi, 1
    cpu.edi >>= 1 /*0x1*/ % 32;
    // 004fd8c5  81e67f7f007f           -and esi, 0x7f007f7f
    cpu.esi &= x86::reg32(x86::sreg32(2130739071 /*0x7f007f7f*/));
    // 004fd8cb  89fd                   -mov ebp, edi
    cpu.ebp = cpu.edi;
    // 004fd8cd  81e77f7f7f00           -and edi, 0x7f7f7f
    cpu.edi &= x86::reg32(x86::sreg32(8355711 /*0x7f7f7f*/));
    // 004fd8d3  c1ed08                 -shr ebp, 8
    cpu.ebp >>= 8 /*0x8*/ % 32;
    // 004fd8d6  09cf                   -or edi, ecx
    cpu.edi |= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd8d8  81e500007f00           -and ebp, 0x7f0000
    cpu.ebp &= x86::reg32(x86::sreg32(8323072 /*0x7f0000*/));
    // 004fd8de  897afc                 -mov dword ptr [edx - 4], edi
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 004fd8e1  09ee                   +or esi, ebp
    cpu.clear_co();
    cpu.set_szp((cpu.esi |= x86::reg32(x86::sreg32(cpu.ebp))));
    // 004fd8e3  4b                     +dec ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004fd8e4  8972f8                 -mov dword ptr [edx - 8], esi
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(-8) /* -0x8 */) = cpu.esi;
    // 004fd8e7  75ad                   -jne 0x4fd896
    if (!cpu.flags.zf)
    {
        goto L_0x004fd896;
    }
    // 004fd8e9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd8ea  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd8eb  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd8ec  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd8ed  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4fd8ee(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd8ee  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004fd8ef  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004fd8f0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004fd8f1  890dd8765600           -mov dword ptr [0x5676d8], ecx
    app->getMemory<x86::reg32>(x86::reg32(5666520) /* 0x5676d8 */) = cpu.ecx;
    // 004fd8f7  8b30                   -mov esi, dword ptr [eax]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax);
    // 004fd8f9  8b3a                   -mov edi, dword ptr [edx]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edx);
    // 004fd8fb  01f7                   -add edi, esi
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.esi));
    // 004fd8fd  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fd900  d1ef                   -shr edi, 1
    cpu.edi >>= 1 /*0x1*/ % 32;
    // 004fd902  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fd905  81e77f7f7f7f           -and edi, 0x7f7f7f7f
    cpu.edi &= x86::reg32(x86::sreg32(2139062143 /*0x7f7f7f7f*/));
    // 004fd90b  893ddc765600           -mov dword ptr [0x5676dc], edi
    app->getMemory<x86::reg32>(x86::reg32(5666524) /* 0x5676dc */) = cpu.edi;
L_0x004fd911:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fd911  8b30                   -mov esi, dword ptr [eax]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax);
    // 004fd913  8b3a                   -mov edi, dword ptr [edx]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edx);
    // 004fd915  01f7                   -add edi, esi
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.esi));
    // 004fd917  8b35dc765600           -mov esi, dword ptr [0x5676dc]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(5666524) /* 0x5676dc */);
    // 004fd91d  d1ef                   -shr edi, 1
    cpu.edi >>= 1 /*0x1*/ % 32;
    // 004fd91f  83c308                 -add ebx, 8
    (cpu.ebx) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004fd922  81e77f7f7f7f           -and edi, 0x7f7f7f7f
    cpu.edi &= x86::reg32(x86::sreg32(2139062143 /*0x7f7f7f7f*/));
    // 004fd928  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 004fd92a  893ddc765600           -mov dword ptr [0x5676dc], edi
    app->getMemory<x86::reg32>(x86::reg32(5666524) /* 0x5676dc */) = cpu.edi;
    // 004fd930  89fd                   -mov ebp, edi
    cpu.ebp = cpu.edi;
    // 004fd932  c1e108                 -shl ecx, 8
    cpu.ecx <<= 8 /*0x8*/ % 32;
    // 004fd935  81e77f7f0000           -and edi, 0x7f7f
    cpu.edi &= x86::reg32(x86::sreg32(32639 /*0x7f7f*/));
    // 004fd93b  c1ed08                 -shr ebp, 8
    cpu.ebp >>= 8 /*0x8*/ % 32;
    // 004fd93e  81e10000007f           -and ecx, 0x7f000000
    cpu.ecx &= x86::reg32(x86::sreg32(2130706432 /*0x7f000000*/));
    // 004fd944  81e500007f00           -and ebp, 0x7f0000
    cpu.ebp &= x86::reg32(x86::sreg32(8323072 /*0x7f0000*/));
    // 004fd94a  09cf                   -or edi, ecx
    cpu.edi |= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd94c  09ef                   -or edi, ebp
    cpu.edi |= x86::reg32(x86::sreg32(cpu.ebp));
    // 004fd94e  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fd951  01f7                   -add edi, esi
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.esi));
    // 004fd953  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fd956  d1ef                   -shr edi, 1
    cpu.edi >>= 1 /*0x1*/ % 32;
    // 004fd958  81e67f7f007f           -and esi, 0x7f007f7f
    cpu.esi &= x86::reg32(x86::sreg32(2130739071 /*0x7f007f7f*/));
    // 004fd95e  89fd                   -mov ebp, edi
    cpu.ebp = cpu.edi;
    // 004fd960  81e77f7f7f00           -and edi, 0x7f7f7f
    cpu.edi &= x86::reg32(x86::sreg32(8355711 /*0x7f7f7f*/));
    // 004fd966  c1ed08                 -shr ebp, 8
    cpu.ebp >>= 8 /*0x8*/ % 32;
    // 004fd969  09cf                   -or edi, ecx
    cpu.edi |= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd96b  81e500007f00           -and ebp, 0x7f0000
    cpu.ebp &= x86::reg32(x86::sreg32(8323072 /*0x7f0000*/));
    // 004fd971  897bfc                 -mov dword ptr [ebx - 4], edi
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(-4) /* -0x4 */) = cpu.edi;
    // 004fd974  09ee                   +or esi, ebp
    cpu.clear_co();
    cpu.set_szp((cpu.esi |= x86::reg32(x86::sreg32(cpu.ebp))));
    // 004fd976  8b0dd8765600           -mov ecx, dword ptr [0x5676d8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5666520) /* 0x5676d8 */);
    // 004fd97c  8973f8                 -mov dword ptr [ebx - 8], esi
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(-8) /* -0x8 */) = cpu.esi;
    // 004fd97f  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004fd980  890dd8765600           -mov dword ptr [0x5676d8], ecx
    app->getMemory<x86::reg32>(x86::reg32(5666520) /* 0x5676d8 */) = cpu.ecx;
    // 004fd986  7589                   -jne 0x4fd911
    if (!cpu.flags.zf)
    {
        goto L_0x004fd911;
    }
    // 004fd988  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd989  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd98a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fd98b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4fd98c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fd98c  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004fd98d  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004fd98e  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004fd98f  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004fd990  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004fd992  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004fd994  89dd                   -mov ebp, ebx
    cpu.ebp = cpu.ebx;
    // 004fd996  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004fd998  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004fd99a  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd99c  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x004fd99e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fd99e  8a06                   -mov al, byte ptr [esi]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi);
    // 004fd9a0  8a5e01                 -mov bl, byte ptr [esi + 1]
    cpu.bl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(1) /* 0x1 */);
    // 004fd9a3  8a4e02                 -mov cl, byte ptr [esi + 2]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(2) /* 0x2 */);
    // 004fd9a6  8a5603                 -mov dl, byte ptr [esi + 3]
    cpu.dl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(3) /* 0x3 */);
    // 004fd9a9  8a80e0765600           -mov al, byte ptr [eax + 0x5676e0]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5666528) /* 0x5676e0 */);
    // 004fd9af  8a9be0765600           -mov bl, byte ptr [ebx + 0x5676e0]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(5666528) /* 0x5676e0 */);
    // 004fd9b5  c1e008                 -shl eax, 8
    cpu.eax <<= 8 /*0x8*/ % 32;
    // 004fd9b8  8a89e0765600           -mov cl, byte ptr [ecx + 0x5676e0]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(5666528) /* 0x5676e0 */);
    // 004fd9be  c1e308                 -shl ebx, 8
    cpu.ebx <<= 8 /*0x8*/ % 32;
    // 004fd9c1  8a92e0765600           -mov dl, byte ptr [edx + 0x5676e0]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(5666528) /* 0x5676e0 */);
    // 004fd9c7  09c8                   -or eax, ecx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fd9c9  09d3                   -or ebx, edx
    cpu.ebx |= x86::reg32(x86::sreg32(cpu.edx));
    // 004fd9cb  c1e010                 -shl eax, 0x10
    cpu.eax <<= 16 /*0x10*/ % 32;
    // 004fd9ce  83c608                 -add esi, 8
    (cpu.esi) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004fd9d1  09d8                   -or eax, ebx
    cpu.eax |= x86::reg32(x86::sreg32(cpu.ebx));
    // 004fd9d3  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004fd9d5  8907                   -mov dword ptr [edi], eax
    app->getMemory<x86::reg32>(cpu.edi) = cpu.eax;
    // 004fd9d7  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004fd9d9  8a4efc                 -mov cl, byte ptr [esi - 4]
    cpu.cl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(-4) /* -0x4 */);
    // 004fd9dc  8a56fd                 -mov dl, byte ptr [esi - 3]
    cpu.dl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(-3) /* -0x3 */);
    // 004fd9df  8a46fe                 -mov al, byte ptr [esi - 2]
    cpu.al = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(-2) /* -0x2 */);
    // 004fd9e2  8a5eff                 -mov bl, byte ptr [esi - 1]
    cpu.bl = app->getMemory<x86::reg8>(cpu.esi + x86::reg32(-1) /* -0x1 */);
    // 004fd9e5  8a89e0765600           -mov cl, byte ptr [ecx + 0x5676e0]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(5666528) /* 0x5676e0 */);
    // 004fd9eb  8a92e0765600           -mov dl, byte ptr [edx + 0x5676e0]
    cpu.dl = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(5666528) /* 0x5676e0 */);
    // 004fd9f1  c1e108                 -shl ecx, 8
    cpu.ecx <<= 8 /*0x8*/ % 32;
    // 004fd9f4  8a80e0765600           -mov al, byte ptr [eax + 0x5676e0]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(5666528) /* 0x5676e0 */);
    // 004fd9fa  c1e208                 -shl edx, 8
    cpu.edx <<= 8 /*0x8*/ % 32;
    // 004fd9fd  8a9be0765600           -mov bl, byte ptr [ebx + 0x5676e0]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(5666528) /* 0x5676e0 */);
    // 004fda03  09c1                   -or ecx, eax
    cpu.ecx |= x86::reg32(x86::sreg32(cpu.eax));
    // 004fda05  09da                   -or edx, ebx
    cpu.edx |= x86::reg32(x86::sreg32(cpu.ebx));
    // 004fda07  c1e110                 -shl ecx, 0x10
    cpu.ecx <<= 16 /*0x10*/ % 32;
    // 004fda0a  83c708                 -add edi, 8
    (cpu.edi) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004fda0d  09d1                   -or ecx, edx
    cpu.ecx |= x86::reg32(x86::sreg32(cpu.edx));
    // 004fda0f  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004fda11  894ffc                 -mov dword ptr [edi - 4], ecx
    app->getMemory<x86::reg32>(cpu.edi + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 004fda14  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fda16  83ed02                 +sub ebp, 2
    {
        x86::reg32& tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004fda19  7f83                   -jg 0x4fd99e
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004fd99e;
    }
    // 004fda1b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fda1c  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fda1d  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fda1e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fda1f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4fda20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 004fda20  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004fda21  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004fda22  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004fda23  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004fda24  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004fda25  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004fda26  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004fda28  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 004fda2b  83e4f8                 -and esp, 0xfffffff8
    cpu.esp &= x86::reg32(x86::sreg32(4294967288 /*0xfffffff8*/));
    // 004fda2e  8944240c               -mov dword ptr [esp + 0xc], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 004fda32  b9c0ffffff             -mov ecx, 0xffffffc0
    cpu.ecx = 4294967232 /*0xffffffc0*/;
    // 004fda37  bb71000000             -mov ebx, 0x71
    cpu.ebx = 113 /*0x71*/;
    // 004fda3c  bf00180200             -mov edi, 0x21800
    cpu.edi = 137216 /*0x21800*/;
    // 004fda41  be0000802c             -mov esi, 0x2c800000
    cpu.esi = 746586112 /*0x2c800000*/;
    // 004fda46  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x004fda48:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fda48  894c2410               -mov dword ptr [esp + 0x10], ecx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */) = cpu.ecx;
    // 004fda4c  8d043e                 -lea eax, [esi + edi]
    cpu.eax = x86::reg32(cpu.esi + cpu.edi * 1);
    // 004fda4f  db442410               -fild dword ptr [esp + 0x10]
    fp0 = x86::Float(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */)));
    // 004fda53  dc05b8e05400           -fadd qword ptr [0x54e0b8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<double>(x86::reg32(5562552) /* 0x54e0b8 */)));
    // 004fda59  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004fda5b  dd1424                 -fst qword ptr [esp]
    app->getMemory<double>(cpu.esp) = double(fp0);
    // 004fda5e  dc0d98e05400           -fmul qword ptr [0x54e098]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5562520) /* 0x54e098 */)));
    // 004fda64  8982185e9f00           -mov dword ptr [edx + 0x9f5e18], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(10444312) /* 0x9f5e18 */) = cpu.eax;
    // 004fda6a  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fda6d  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004fda70  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fda71  dd0424                 -fld qword ptr [esp]
    fp0 = x86::Float(app->getMemory<double>(cpu.esp));
    // 004fda74  dc0da0e05400           -fmul qword ptr [0x54e0a0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5562528) /* 0x54e0a0 */)));
    // 004fda7a  89442410               -mov dword ptr [esp + 0x10], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */) = cpu.eax;
    // 004fda7e  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fda81  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004fda84  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fda85  dd0424                 -fld qword ptr [esp]
    fp0 = x86::Float(app->getMemory<double>(cpu.esp));
    // 004fda88  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004fda8c  8b442410               -mov eax, dword ptr [esp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 004fda90  dc0da8e05400           -fmul qword ptr [0x54e0a8]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5562536) /* 0x54e0a8 */)));
    // 004fda96  25ff010000             -and eax, 0x1ff
    cpu.eax &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 004fda9b  c1e00b                 -shl eax, 0xb
    cpu.eax <<= 11 /*0xb*/ % 32;
    // 004fda9e  89442410               -mov dword ptr [esp + 0x10], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */) = cpu.eax;
    // 004fdaa2  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 004fdaa6  25ff010000             -and eax, 0x1ff
    cpu.eax &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 004fdaab  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004fdaaf  8b442410               -mov eax, dword ptr [esp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 004fdab3  03442408               -add eax, dword ptr [esp + 8]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */)));
    // 004fdab7  898218609f00           -mov dword ptr [edx + 0x9f6018], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(10444824) /* 0x9f6018 */) = cpu.eax;
    // 004fdabd  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fdac0  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004fdac3  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fdac4  dd0424                 -fld qword ptr [esp]
    fp0 = x86::Float(app->getMemory<double>(cpu.esp));
    // 004fdac7  dc0db0e05400           -fmul qword ptr [0x54e0b0]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5562544) /* 0x54e0b0 */)));
    // 004fdacd  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004fdad1  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fdad4  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004fdad7  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fdad8  89442410               -mov dword ptr [esp + 0x10], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */) = cpu.eax;
    // 004fdadc  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 004fdae0  25ff010000             -and eax, 0x1ff
    cpu.eax &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 004fdae5  c1e017                 -shl eax, 0x17
    cpu.eax <<= 23 /*0x17*/ % 32;
    // 004fdae8  89442408               -mov dword ptr [esp + 8], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004fdaec  8b442410               -mov eax, dword ptr [esp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */);
    // 004fdaf0  83c204                 -add edx, 4
    (cpu.edx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fdaf3  25ff010000             -and eax, 0x1ff
    cpu.eax &= x86::reg32(x86::sreg32(511 /*0x1ff*/));
    // 004fdaf8  81c700080000           -add edi, 0x800
    (cpu.edi) += x86::reg32(x86::sreg32(2048 /*0x800*/));
    // 004fdafe  c1e00b                 -shl eax, 0xb
    cpu.eax <<= 11 /*0xb*/ % 32;
    // 004fdb01  81c600008000           -add esi, 0x800000
    (cpu.esi) += x86::reg32(x86::sreg32(8388608 /*0x800000*/));
    // 004fdb07  89442410               -mov dword ptr [esp + 0x10], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */) = cpu.eax;
    // 004fdb0b  8b442408               -mov eax, dword ptr [esp + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(8) /* 0x8 */);
    // 004fdb0f  41                     -inc ecx
    (cpu.ecx)++;
    // 004fdb10  03442410               -add eax, dword ptr [esp + 0x10]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(16) /* 0x10 */)));
    // 004fdb14  43                     -inc ebx
    (cpu.ebx)++;
    // 004fdb15  898214629f00           -mov dword ptr [edx + 0x9f6214], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(10445332) /* 0x9f6214 */) = cpu.eax;
    // 004fdb1b  83f940                 +cmp ecx, 0x40
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(64 /*0x40*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdb1e  0f8c24ffffff           -jl 0x4fda48
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004fda48;
    }
    // 004fdb24  b99cfdffff             -mov ecx, 0xfffffd9c
    cpu.ecx = 4294966684 /*0xfffffd9c*/;
    // 004fdb29  be1f000000             -mov esi, 0x1f
    cpu.esi = 31 /*0x1f*/;
    // 004fdb2e  8b7c240c               -mov edi, dword ptr [esp + 0xc]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
    // 004fdb32  ba67ffffff             -mov edx, 0xffffff67
    cpu.edx = 4294967143 /*0xffffff67*/;
L_0x004fdb37:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fdb37  8d0412                 -lea eax, [edx + edx]
    cpu.eax = x86::reg32(cpu.edx + cpu.edx * 1);
    // 004fdb3a  0581000000             -add eax, 0x81
    (cpu.eax) += x86::reg32(x86::sreg32(129 /*0x81*/));
    // 004fdb3f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004fdb41  0f8c03010000           -jl 0x4fdc4a
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004fdc4a;
    }
    // 004fdb47  3dff000000             +cmp eax, 0xff
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(255 /*0xff*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdb4c  7e05                   -jle 0x4fdb53
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004fdb53;
    }
    // 004fdb4e  b8ff000000             -mov eax, 0xff
    cpu.eax = 255 /*0xff*/;
L_0x004fdb53:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fdb53  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004fdb55  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fdb58  c1f803                 -sar eax, 3
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (3 /*0x3*/ % 32));
    // 004fdb5b  c1fb03                 -sar ebx, 3
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (3 /*0x3*/ % 32));
    // 004fdb5e  83f81f                 +cmp eax, 0x1f
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(31 /*0x1f*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdb61  7e02                   -jle 0x4fdb65
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004fdb65;
    }
    // 004fdb63  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
L_0x004fdb65:
    // 004fdb65  83ff0f                 +cmp edi, 0xf
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(15 /*0xf*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdb68  0f85e3000000           -jne 0x4fdc51
    if (!cpu.flags.zf)
    {
        goto L_0x004fdc51;
    }
    // 004fdb6e  c1e00a                 -shl eax, 0xa
    cpu.eax <<= 10 /*0xa*/ % 32;
    // 004fdb71  c1e31a                 -shl ebx, 0x1a
    cpu.ebx <<= 26 /*0x1a*/ % 32;
L_0x004fdb74:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fdb74  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004fdb76  8981b45b9f00           -mov dword ptr [ecx + 0x9f5bb4], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(10443700) /* 0x9f5bb4 */) = cpu.eax;
    // 004fdb7c  42                     -inc edx
    (cpu.edx)++;
    // 004fdb7d  83c104                 -add ecx, 4
    (cpu.ecx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fdb80  81fa99000000           +cmp edx, 0x99
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(153 /*0x99*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdb86  7caf                   -jl 0x4fdb37
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004fdb37;
    }
    // 004fdb88  b9f4fdffff             -mov ecx, 0xfffffdf4
    cpu.ecx = 4294966772 /*0xfffffdf4*/;
    // 004fdb8d  ba7dffffff             -mov edx, 0xffffff7d
    cpu.edx = 4294967165 /*0xffffff7d*/;
    // 004fdb92  bfff000000             -mov edi, 0xff
    cpu.edi = 255 /*0xff*/;
    // 004fdb97  be3f000000             -mov esi, 0x3f
    cpu.esi = 63 /*0x3f*/;
L_0x004fdb9c:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fdb9c  8d0412                 -lea eax, [edx + edx]
    cpu.eax = x86::reg32(cpu.edx + cpu.edx * 1);
    // 004fdb9f  0581000000             -add eax, 0x81
    (cpu.eax) += x86::reg32(x86::sreg32(129 /*0x81*/));
    // 004fdba4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004fdba6  0f8cb0000000           -jl 0x4fdc5c
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004fdc5c;
    }
    // 004fdbac  3dff000000             +cmp eax, 0xff
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(255 /*0xff*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdbb1  7e02                   -jle 0x4fdbb5
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004fdbb5;
    }
    // 004fdbb3  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
L_0x004fdbb5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fdbb5  837c240c0f             +cmp dword ptr [esp + 0xc], 0xf
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(12) /* 0xc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(15 /*0xf*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdbba  0f85a3000000           -jne 0x4fdc63
    if (!cpu.flags.zf)
    {
        goto L_0x004fdc63;
    }
    // 004fdbc0  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004fdbc2  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fdbc5  c1f803                 -sar eax, 3
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (3 /*0x3*/ % 32));
    // 004fdbc8  c1fb03                 -sar ebx, 3
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (3 /*0x3*/ % 32));
    // 004fdbcb  83f81f                 +cmp eax, 0x1f
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(31 /*0x1f*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdbce  7e05                   -jle 0x4fdbd5
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004fdbd5;
    }
    // 004fdbd0  b81f000000             -mov eax, 0x1f
    cpu.eax = 31 /*0x1f*/;
L_0x004fdbd5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fdbd5  c1e005                 -shl eax, 5
    cpu.eax <<= 5 /*0x5*/ % 32;
    // 004fdbd8  c1e315                 -shl ebx, 0x15
    cpu.ebx <<= 21 /*0x15*/ % 32;
    // 004fdbdb  83c104                 -add ecx, 4
    (cpu.ecx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fdbde  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004fdbe0  42                     -inc edx
    (cpu.edx)++;
    // 004fdbe1  898140579f00           -mov dword ptr [ecx + 0x9f5740], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(10442560) /* 0x9f5740 */) = cpu.eax;
    // 004fdbe7  81fa83000000           +cmp edx, 0x83
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(131 /*0x83*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdbed  7cad                   -jl 0x4fdb9c
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004fdb9c;
    }
    // 004fdbef  b93cfdffff             -mov ecx, 0xfffffd3c
    cpu.ecx = 4294966588 /*0xfffffd3c*/;
    // 004fdbf4  bf1f000000             -mov edi, 0x1f
    cpu.edi = 31 /*0x1f*/;
    // 004fdbf9  beff000000             -mov esi, 0xff
    cpu.esi = 255 /*0xff*/;
    // 004fdbfe  ba4fffffff             -mov edx, 0xffffff4f
    cpu.edx = 4294967119 /*0xffffff4f*/;
L_0x004fdc03:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fdc03  8d0412                 -lea eax, [edx + edx]
    cpu.eax = x86::reg32(cpu.edx + cpu.edx * 1);
    // 004fdc06  0581000000             -add eax, 0x81
    (cpu.eax) += x86::reg32(x86::sreg32(129 /*0x81*/));
    // 004fdc0b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004fdc0d  7c6f                   -jl 0x4fdc7e
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004fdc7e;
    }
    // 004fdc0f  3dff000000             +cmp eax, 0xff
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(255 /*0xff*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdc14  7e02                   -jle 0x4fdc18
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004fdc18;
    }
    // 004fdc16  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
L_0x004fdc18:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fdc18  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004fdc1a  83c004                 -add eax, 4
    (cpu.eax) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fdc1d  c1f803                 -sar eax, 3
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (3 /*0x3*/ % 32));
    // 004fdc20  c1fb03                 -sar ebx, 3
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (3 /*0x3*/ % 32));
    // 004fdc23  83f81f                 +cmp eax, 0x1f
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(31 /*0x1f*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdc26  7e02                   -jle 0x4fdc2a
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004fdc2a;
    }
    // 004fdc28  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
L_0x004fdc2a:
    // 004fdc2a  c1e310                 -shl ebx, 0x10
    cpu.ebx <<= 16 /*0x10*/ % 32;
    // 004fdc2d  83c104                 -add ecx, 4
    (cpu.ecx) += x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004fdc30  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004fdc32  42                     -inc edx
    (cpu.edx)++;
    // 004fdc33  898170529f00           -mov dword ptr [ecx + 0x9f5270], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(10441328) /* 0x9f5270 */) = cpu.eax;
    // 004fdc39  81fab1000000           +cmp edx, 0xb1
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(177 /*0xb1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdc3f  7cc2                   -jl 0x4fdc03
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004fdc03;
    }
    // 004fdc41  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004fdc43  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fdc44  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fdc45  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fdc46  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fdc47  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fdc48  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fdc49  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004fdc4a:
    // 004fdc4a  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004fdc4c  e902ffffff             -jmp 0x4fdb53
    goto L_0x004fdb53;
L_0x004fdc51:
    // 004fdc51  c1e00b                 -shl eax, 0xb
    cpu.eax <<= 11 /*0xb*/ % 32;
    // 004fdc54  c1e31b                 +shl ebx, 0x1b
    {
        x86::reg8 tmp = 27 /*0x1b*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004fdc57  e918ffffff             -jmp 0x4fdb74
    goto L_0x004fdb74;
L_0x004fdc5c:
    // 004fdc5c  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004fdc5e  e952ffffff             -jmp 0x4fdbb5
    goto L_0x004fdbb5;
L_0x004fdc63:
    // 004fdc63  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004fdc65  83c002                 -add eax, 2
    (cpu.eax) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004fdc68  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 004fdc6b  c1fb02                 -sar ebx, 2
    cpu.ebx = x86::reg32(x86::sreg32(cpu.ebx) >> (2 /*0x2*/ % 32));
    // 004fdc6e  83f83f                 +cmp eax, 0x3f
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(63 /*0x3f*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdc71  0f8e5effffff           -jle 0x4fdbd5
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004fdbd5;
    }
    // 004fdc77  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004fdc79  e957ffffff             -jmp 0x4fdbd5
    goto L_0x004fdbd5;
L_0x004fdc7e:
    // 004fdc7e  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004fdc80  eb96                   -jmp 0x4fdc18
    goto L_0x004fdc18;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_4fdc90(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004fdc90  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004fdc91  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004fdc92  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004fdc93  81ec480a0000           -sub esp, 0xa48
    (cpu.esp) -= x86::reg32(x86::sreg32(2632 /*0xa48*/));
    // 004fdc99  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004fdc9b  899424100a0000         -mov dword ptr [esp + 0xa10], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2576) /* 0xa10 */) = cpu.edx;
    // 004fdca2  899c240c0a0000         -mov dword ptr [esp + 0xa0c], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2572) /* 0xa0c */) = cpu.ebx;
    // 004fdca9  89cf                   -mov edi, ecx
    cpu.edi = cpu.ecx;
    // 004fdcab  8b94245c0a0000         -mov edx, dword ptr [esp + 0xa5c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2652) /* 0xa5c */);
    // 004fdcb2  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004fdcb4  0f85f4010000           -jne 0x4fdeae
    if (!cpu.flags.zf)
    {
        goto L_0x004fdeae;
    }
    // 004fdcba  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
L_0x004fdcbf:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fdcbf  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004fdcc1  8a1510505600           -mov dl, byte ptr [0x565010]
    cpu.dl = app->getMemory<x86::reg8>(x86::reg32(5656592) /* 0x565010 */);
    // 004fdcc7  83fa0f                 +cmp edx, 0xf
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(15 /*0xf*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdcca  7409                   -je 0x4fdcd5
    if (cpu.flags.zf)
    {
        goto L_0x004fdcd5;
    }
    // 004fdccc  83fa10                 +cmp edx, 0x10
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdccf  0f8503020000           -jne 0x4fded8
    if (!cpu.flags.zf)
    {
        goto L_0x004fded8;
    }
L_0x004fdcd5:
    // 004fdcd5  3b1518649f00           +cmp edx, dword ptr [0x9f6418]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(10445848) /* 0x9f6418 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdcdb  740d                   -je 0x4fdcea
    if (cpu.flags.zf)
    {
        goto L_0x004fdcea;
    }
    // 004fdcdd  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004fdcdf  e83cfdffff             -call 0x4fda20
    cpu.esp -= 4;
    sub_4fda20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fdce4  891518649f00           -mov dword ptr [0x9f6418], edx
    app->getMemory<x86::reg32>(x86::reg32(10445848) /* 0x9f6418 */) = cpu.edx;
L_0x004fdcea:
    // 004fdcea  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004fdcec  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004fdcee  8b1524505600           -mov edx, dword ptr [0x565024]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656612) /* 0x565024 */);
    // 004fdcf4  8b0482                 -mov eax, dword ptr [edx + eax*4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.eax * 4);
    // 004fdcf7  8b1520505600           -mov edx, dword ptr [0x565020]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656608) /* 0x565020 */);
    // 004fdcfd  03049a                 -add eax, dword ptr [edx + ebx*4]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + cpu.ebx * 4)));
    // 004fdd00  030514505600           -add eax, dword ptr [0x565014]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656596) /* 0x565014 */)));
    // 004fdd06  83e003                 -and eax, 3
    cpu.eax &= x86::reg32(x86::sreg32(3 /*0x3*/));
    // 004fdd09  6683e6fe               -and si, 0xfffe
    cpu.si &= x86::reg16(x86::sreg16(65534 /*0xfffe*/));
    // 004fdd0d  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004fdd0f  09c6                   -or esi, eax
    cpu.esi |= x86::reg32(x86::sreg32(cpu.eax));
    // 004fdd11  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004fdd13  0fafc7                 -imul eax, edi
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edi)));
    // 004fdd16  8d1406                 -lea edx, [esi + eax]
    cpu.edx = x86::reg32(cpu.esi + cpu.eax * 1);
    // 004fdd19  8b8424580a0000         -mov eax, dword ptr [esp + 0xa58]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2648) /* 0xa58 */);
    // 004fdd20  0fafc1                 -imul eax, ecx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.ecx)));
    // 004fdd23  899424080a0000         -mov dword ptr [esp + 0xa08], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2568) /* 0xa08 */) = cpu.edx;
    // 004fdd2a  8b9424100a0000         -mov edx, dword ptr [esp + 0xa10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2576) /* 0xa10 */);
    // 004fdd31  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004fdd33  899424440a0000         -mov dword ptr [esp + 0xa44], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2628) /* 0xa44 */) = cpu.edx;
    // 004fdd3a  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004fdd3c  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004fdd3e  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004fdd41  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004fdd43  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004fdd45  8b1500505600           -mov edx, dword ptr [0x565000]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656576) /* 0x565000 */);
    // 004fdd4b  898424040a0000         -mov dword ptr [esp + 0xa04], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2564) /* 0xa04 */) = cpu.eax;
    // 004fdd52  39d6                   +cmp esi, edx
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
    // 004fdd54  7d22                   -jge 0x4fdd78
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004fdd78;
    }
    // 004fdd56  8d1c09                 -lea ebx, [ecx + ecx]
    cpu.ebx = x86::reg32(cpu.ecx + cpu.ecx * 1);
    // 004fdd59  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004fdd5b  8d53ff                 -lea edx, [ebx - 1]
    cpu.edx = x86::reg32(cpu.ebx + x86::reg32(-1) /* -0x1 */);
    // 004fdd5e  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004fdd60  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004fdd62  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004fdd64  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004fdd67  f7fb                   -idiv ebx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ebx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004fdd69  0fafd8                 -imul ebx, eax
    cpu.ebx = x86::reg32(x86::sreg64(x86::sreg32(cpu.ebx)) * x86::sreg64(x86::sreg32(cpu.eax)));
    // 004fdd6c  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004fdd6f  01de                   -add esi, ebx
    (cpu.esi) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004fdd71  0184240c0a0000         -add dword ptr [esp + 0xa0c], eax
    (app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2572) /* 0xa0c */)) += x86::reg32(x86::sreg32(cpu.eax));
L_0x004fdd78:
    // 004fdd78  8b8424080a0000         -mov eax, dword ptr [esp + 0xa08]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2568) /* 0xa08 */);
    // 004fdd7f  8b3d08505600           -mov edi, dword ptr [0x565008]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5656584) /* 0x565008 */);
    // 004fdd85  39f8                   +cmp eax, edi
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edi));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdd87  7e07                   -jle 0x4fdd90
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004fdd90;
    }
    // 004fdd89  89bc24080a0000         -mov dword ptr [esp + 0xa08], edi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2568) /* 0xa08 */) = cpu.edi;
L_0x004fdd90:
    // 004fdd90  8b8424100a0000         -mov eax, dword ptr [esp + 0xa10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2576) /* 0xa10 */);
    // 004fdd97  8b2d04505600           -mov ebp, dword ptr [0x565004]
    cpu.ebp = app->getMemory<x86::reg32>(x86::reg32(5656580) /* 0x565004 */);
    // 004fdd9d  39e8                   +cmp eax, ebp
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
    // 004fdd9f  7d47                   -jge 0x4fdde8
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004fdde8;
    }
    // 004fdda1  8b9424100a0000         -mov edx, dword ptr [esp + 0xa10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2576) /* 0xa10 */);
    // 004fdda8  89e8                   -mov eax, ebp
    cpu.eax = cpu.ebp;
    // 004fddaa  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004fddac  8d51ff                 -lea edx, [ecx - 1]
    cpu.edx = x86::reg32(cpu.ecx + x86::reg32(-1) /* -0x1 */);
    // 004fddaf  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004fddb1  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004fddb3  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004fddb6  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004fddb8  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004fddba  0fafd0                 -imul edx, eax
    cpu.edx = x86::reg32(x86::sreg64(x86::sreg32(cpu.edx)) * x86::sreg64(x86::sreg32(cpu.eax)));
    // 004fddbd  0faf8424040a0000       -imul eax, dword ptr [esp + 0xa04]
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2564) /* 0xa04 */))));
    // 004fddc5  8b9c24100a0000         -mov ebx, dword ptr [esp + 0xa10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2576) /* 0xa10 */);
    // 004fddcc  8bac240c0a0000         -mov ebp, dword ptr [esp + 0xa0c]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2572) /* 0xa0c */);
    // 004fddd3  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004fddd6  01d3                   -add ebx, edx
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.edx));
    // 004fddd8  01c5                   -add ebp, eax
    (cpu.ebp) += x86::reg32(x86::sreg32(cpu.eax));
    // 004fddda  899c24100a0000         -mov dword ptr [esp + 0xa10], ebx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2576) /* 0xa10 */) = cpu.ebx;
    // 004fdde1  89ac240c0a0000         -mov dword ptr [esp + 0xa0c], ebp
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2572) /* 0xa0c */) = cpu.ebp;
L_0x004fdde8:
    // 004fdde8  8b8424440a0000         -mov eax, dword ptr [esp + 0xa44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2628) /* 0xa44 */);
    // 004fddef  8b150c505600           -mov edx, dword ptr [0x56500c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656588) /* 0x56500c */);
    // 004fddf5  39d0                   +cmp eax, edx
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
    // 004fddf7  7e07                   -jle 0x4fde00
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004fde00;
    }
    // 004fddf9  899424440a0000         -mov dword ptr [esp + 0xa44], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2628) /* 0xa44 */) = cpu.edx;
L_0x004fde00:
    // 004fde00  8b9424080a0000         -mov edx, dword ptr [esp + 0xa08]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2568) /* 0xa08 */);
    // 004fde07  29f2                   -sub edx, esi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004fde09  01c9                   -add ecx, ecx
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004fde0b  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004fde0d  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004fde10  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004fde12  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004fde14  83f802                 +cmp eax, 2
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
    // 004fde17  0f8c83000000           -jl 0x4fdea0
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004fdea0;
    }
    // 004fde1d  8b8424040a0000         -mov eax, dword ptr [esp + 0xa04]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2564) /* 0xa04 */);
    // 004fde24  8b8c245c0a0000         -mov ecx, dword ptr [esp + 0xa5c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2652) /* 0xa5c */);
    // 004fde2b  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004fde2e  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004fde30  0f85b3000000           -jne 0x4fdee9
    if (!cpu.flags.zf)
    {
        goto L_0x004fdee9;
    }
    // 004fde36  8bac24100a0000         -mov ebp, dword ptr [esp + 0xa10]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2576) /* 0xa10 */);
    // 004fde3d  8b9c24440a0000         -mov ebx, dword ptr [esp + 0xa44]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2628) /* 0xa44 */);
    // 004fde44  8b8c240c0a0000         -mov ecx, dword ptr [esp + 0xa0c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2572) /* 0xa0c */);
    // 004fde4b  39dd                   +cmp ebp, ebx
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fde4d  7d51                   -jge 0x4fdea0
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004fdea0;
    }
    // 004fde4f  898424200a0000         -mov dword ptr [esp + 0xa20], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2592) /* 0xa20 */) = cpu.eax;
L_0x004fde56:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fde56  89eb                   -mov ebx, ebp
    cpu.ebx = cpu.ebp;
    // 004fde58  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004fde5a  8b1524505600           -mov edx, dword ptr [0x565024]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656612) /* 0x565024 */);
    // 004fde60  8b0482                 -mov eax, dword ptr [edx + eax*4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.eax * 4);
    // 004fde63  8b1520505600           -mov edx, dword ptr [0x565020]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656608) /* 0x565020 */);
    // 004fde69  03049a                 -add eax, dword ptr [edx + ebx*4]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + cpu.ebx * 4)));
    // 004fde6c  030514505600           -add eax, dword ptr [0x565014]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656596) /* 0x565014 */)));
    // 004fde72  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004fde74  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004fde76  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004fde78  45                     -inc ebp
    (cpu.ebp)++;
    // 004fde79  e89ef7ffff             -call 0x4fd61c
    cpu.esp -= 4;
    sub_4fd61c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fde7e  8b8424200a0000         -mov eax, dword ptr [esp + 0xa20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2592) /* 0xa20 */);
    // 004fde85  8b9424440a0000         -mov edx, dword ptr [esp + 0xa44]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2628) /* 0xa44 */);
    // 004fde8c  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004fde8e  39d5                   +cmp ebp, edx
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fde90  7cc4                   -jl 0x4fde56
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004fde56;
    }
    // 004fde92  8d8000000000           -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
    // 004fde98  8d9200000000           -lea edx, [edx]
    cpu.edx = x86::reg32(cpu.edx);
    // 004fde9e  8bc0                   -mov eax, eax
    cpu.eax = cpu.eax;
L_0x004fdea0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fdea0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004fdea2  81c4480a0000           -add esp, 0xa48
    (cpu.esp) += x86::reg32(x86::sreg32(2632 /*0xa48*/));
    // 004fdea8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fdea9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fdeaa  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fdeab  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x004fdeae:
    // 004fdeae  83fa01                 +cmp edx, 1
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
    // 004fdeb1  750a                   -jne 0x4fdebd
    if (!cpu.flags.zf)
    {
        goto L_0x004fdebd;
    }
L_0x004fdeb3:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fdeb3  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 004fdeb8  e902feffff             -jmp 0x4fdcbf
    goto L_0x004fdcbf;
L_0x004fdebd:
    // 004fdebd  83fa02                 +cmp edx, 2
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
    // 004fdec0  74f1                   -je 0x4fdeb3
    if (cpu.flags.zf)
    {
        goto L_0x004fdeb3;
    }
    // 004fdec2  83fa03                 +cmp edx, 3
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdec5  74ec                   -je 0x4fdeb3
    if (cpu.flags.zf)
    {
        goto L_0x004fdeb3;
    }
    // 004fdec7  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 004fdecc  81c4480a0000           -add esp, 0xa48
    (cpu.esp) += x86::reg32(x86::sreg32(2632 /*0xa48*/));
    // 004fded2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fded3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fded4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fded5  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x004fded8:
    // 004fded8  b8feffffff             -mov eax, 0xfffffffe
    cpu.eax = 4294967294 /*0xfffffffe*/;
    // 004fdedd  81c4480a0000           -add esp, 0xa48
    (cpu.esp) += x86::reg32(x86::sreg32(2632 /*0xa48*/));
    // 004fdee3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fdee4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fdee5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fdee6  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x004fdee9:
    // 004fdee9  83f903                 +cmp ecx, 3
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
    // 004fdeec  0f857c010000           -jne 0x4fe06e
    if (!cpu.flags.zf)
    {
        goto L_0x004fe06e;
    }
    // 004fdef2  8bac24100a0000         -mov ebp, dword ptr [esp + 0xa10]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2576) /* 0xa10 */);
    // 004fdef9  8b9c24440a0000         -mov ebx, dword ptr [esp + 0xa44]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2628) /* 0xa44 */);
    // 004fdf00  8b8c240c0a0000         -mov ecx, dword ptr [esp + 0xa0c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2572) /* 0xa0c */);
    // 004fdf07  39dd                   +cmp ebp, ebx
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdf09  0f8d71000000           -jge 0x4fdf80
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004fdf80;
    }
    // 004fdf0f  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004fdf11  4a                     -dec edx
    (cpu.edx)--;
    // 004fdf12  899424180a0000         -mov dword ptr [esp + 0xa18], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2584) /* 0xa18 */) = cpu.edx;
    // 004fdf19  8d143f                 -lea edx, [edi + edi]
    cpu.edx = x86::reg32(cpu.edi + cpu.edi * 1);
    // 004fdf1c  83ea02                 -sub edx, 2
    (cpu.edx) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004fdf1f  898424140a0000         -mov dword ptr [esp + 0xa14], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2580) /* 0xa14 */) = cpu.eax;
    // 004fdf26  899424280a0000         -mov dword ptr [esp + 0xa28], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2600) /* 0xa28 */) = cpu.edx;
L_0x004fdf2d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fdf2d  8b9c24180a0000         -mov ebx, dword ptr [esp + 0xa18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2584) /* 0xa18 */);
    // 004fdf34  89e2                   -mov edx, esp
    cpu.edx = cpu.esp;
    // 004fdf36  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004fdf38  e855f9ffff             -call 0x4fd892
    cpu.esp -= 4;
    sub_4fd892(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fdf3d  89eb                   -mov ebx, ebp
    cpu.ebx = cpu.ebp;
    // 004fdf3f  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004fdf41  8b1524505600           -mov edx, dword ptr [0x565024]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656612) /* 0x565024 */);
    // 004fdf47  8b0482                 -mov eax, dword ptr [edx + eax*4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.eax * 4);
    // 004fdf4a  8b1520505600           -mov edx, dword ptr [0x565020]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656608) /* 0x565020 */);
    // 004fdf50  03049a                 -add eax, dword ptr [edx + ebx*4]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + cpu.ebx * 4)));
    // 004fdf53  030514505600           -add eax, dword ptr [0x565014]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656596) /* 0x565014 */)));
    // 004fdf59  8b9c24280a0000         -mov ebx, dword ptr [esp + 0xa28]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2600) /* 0xa28 */);
    // 004fdf60  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004fdf62  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 004fdf64  83c502                 -add ebp, 2
    (cpu.ebp) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004fdf67  e8b0f6ffff             -call 0x4fd61c
    cpu.esp -= 4;
    sub_4fd61c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fdf6c  8b8424140a0000         -mov eax, dword ptr [esp + 0xa14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2580) /* 0xa14 */);
    // 004fdf73  8b9424440a0000         -mov edx, dword ptr [esp + 0xa44]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2628) /* 0xa44 */);
    // 004fdf7a  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004fdf7c  39d5                   +cmp ebp, edx
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdf7e  7cad                   -jl 0x4fdf2d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004fdf2d;
    }
L_0x004fdf80:
    // 004fdf80  8b8424440a0000         -mov eax, dword ptr [esp + 0xa44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2628) /* 0xa44 */);
    // 004fdf87  8b8c24100a0000         -mov ecx, dword ptr [esp + 0xa10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2576) /* 0xa10 */);
    // 004fdf8e  8b9424080a0000         -mov edx, dword ptr [esp + 0xa08]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2568) /* 0xa08 */);
    // 004fdf95  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004fdf97  29f2                   -sub edx, esi
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004fdf99  0fafc2                 -imul eax, edx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004fdf9c  3d00580200             +cmp eax, 0x25800
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(153600 /*0x25800*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdfa1  0f8fae000000           -jg 0x4fe055
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004fe055;
    }
L_0x004fdfa7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fdfa7  8bac24100a0000         -mov ebp, dword ptr [esp + 0xa10]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2576) /* 0xa10 */);
    // 004fdfae  8d57ff                 -lea edx, [edi - 1]
    cpu.edx = x86::reg32(cpu.edi + x86::reg32(-1) /* -0x1 */);
    // 004fdfb1  8b84240c0a0000         -mov eax, dword ptr [esp + 0xa0c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2572) /* 0xa0c */);
    // 004fdfb8  899424340a0000         -mov dword ptr [esp + 0xa34], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2612) /* 0xa34 */) = cpu.edx;
    // 004fdfbf  8b9424040a0000         -mov edx, dword ptr [esp + 0xa04]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2564) /* 0xa04 */);
    // 004fdfc6  45                     -inc ebp
    (cpu.ebp)++;
    // 004fdfc7  c1e202                 -shl edx, 2
    cpu.edx <<= 2 /*0x2*/ % 32;
    // 004fdfca  01ff                   -add edi, edi
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.edi));
    // 004fdfcc  899424240a0000         -mov dword ptr [esp + 0xa24], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2596) /* 0xa24 */) = cpu.edx;
    // 004fdfd3  8b9424440a0000         -mov edx, dword ptr [esp + 0xa44]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2628) /* 0xa44 */);
    // 004fdfda  83ef02                 -sub edi, 2
    (cpu.edi) -= x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004fdfdd  4a                     -dec edx
    (cpu.edx)--;
    // 004fdfde  89bc24300a0000         -mov dword ptr [esp + 0xa30], edi
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2608) /* 0xa30 */) = cpu.edi;
    // 004fdfe5  899424380a0000         -mov dword ptr [esp + 0xa38], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2616) /* 0xa38 */) = cpu.edx;
    // 004fdfec  39d5                   +cmp ebp, edx
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fdfee  0f8dacfeffff           -jge 0x4fdea0
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004fdea0;
    }
L_0x004fdff4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fdff4  8bbc24240a0000         -mov edi, dword ptr [esp + 0xa24]
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2596) /* 0xa24 */);
    // 004fdffb  8b8c24340a0000         -mov ecx, dword ptr [esp + 0xa34]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2612) /* 0xa34 */);
    // 004fe002  01c7                   -add edi, eax
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.eax));
    // 004fe004  89e3                   -mov ebx, esp
    cpu.ebx = cpu.esp;
    // 004fe006  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004fe008  e8e1f8ffff             -call 0x4fd8ee
    cpu.esp -= 4;
    sub_4fd8ee(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fe00d  89eb                   -mov ebx, ebp
    cpu.ebx = cpu.ebp;
    // 004fe00f  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004fe011  8b1524505600           -mov edx, dword ptr [0x565024]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656612) /* 0x565024 */);
    // 004fe017  8b0482                 -mov eax, dword ptr [edx + eax*4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.eax * 4);
    // 004fe01a  8b1520505600           -mov edx, dword ptr [0x565020]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656608) /* 0x565020 */);
    // 004fe020  03049a                 -add eax, dword ptr [edx + ebx*4]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + cpu.ebx * 4)));
    // 004fe023  030514505600           -add eax, dword ptr [0x565014]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656596) /* 0x565014 */)));
    // 004fe029  8b9c24300a0000         -mov ebx, dword ptr [esp + 0xa30]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2608) /* 0xa30 */);
    // 004fe030  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004fe032  89e0                   -mov eax, esp
    cpu.eax = cpu.esp;
    // 004fe034  83c502                 -add ebp, 2
    (cpu.ebp) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004fe037  e8e0f5ffff             -call 0x4fd61c
    cpu.esp -= 4;
    sub_4fd61c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fe03c  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004fe03e  3bac24380a0000         +cmp ebp, dword ptr [esp + 0xa38]
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2616) /* 0xa38 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fe045  7cad                   -jl 0x4fdff4
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004fdff4;
    }
    // 004fe047  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004fe049  81c4480a0000           -add esp, 0xa48
    (cpu.esp) += x86::reg32(x86::sreg32(2632 /*0xa48*/));
    // 004fe04f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fe050  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fe051  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fe052  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x004fe055:
    // 004fe055  e886b0ffff             -call 0x4f90e0
    cpu.esp -= 4;
    sub_4f90e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fe05a  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004fe05c  0f8545ffffff           -jne 0x4fdfa7
    if (!cpu.flags.zf)
    {
        goto L_0x004fdfa7;
    }
    // 004fe062  81c4480a0000           -add esp, 0xa48
    (cpu.esp) += x86::reg32(x86::sreg32(2632 /*0xa48*/));
    // 004fe068  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fe069  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fe06a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fe06b  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x004fe06e:
    // 004fe06e  8bac24100a0000         -mov ebp, dword ptr [esp + 0xa10]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2576) /* 0xa10 */);
    // 004fe075  8b9c24440a0000         -mov ebx, dword ptr [esp + 0xa44]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2628) /* 0xa44 */);
    // 004fe07c  8b8c240c0a0000         -mov ecx, dword ptr [esp + 0xa0c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2572) /* 0xa0c */);
    // 004fe083  39dd                   +cmp ebp, ebx
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fe085  7d49                   -jge 0x4fe0d0
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004fe0d0;
    }
    // 004fe087  8984241c0a0000         -mov dword ptr [esp + 0xa1c], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2588) /* 0xa1c */) = cpu.eax;
L_0x004fe08e:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fe08e  89eb                   -mov ebx, ebp
    cpu.ebx = cpu.ebp;
    // 004fe090  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004fe092  8b1524505600           -mov edx, dword ptr [0x565024]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656612) /* 0x565024 */);
    // 004fe098  8b0482                 -mov eax, dword ptr [edx + eax*4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.eax * 4);
    // 004fe09b  8b1520505600           -mov edx, dword ptr [0x565020]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656608) /* 0x565020 */);
    // 004fe0a1  03049a                 -add eax, dword ptr [edx + ebx*4]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + cpu.ebx * 4)));
    // 004fe0a4  030514505600           -add eax, dword ptr [0x565014]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656596) /* 0x565014 */)));
    // 004fe0aa  89fb                   -mov ebx, edi
    cpu.ebx = cpu.edi;
    // 004fe0ac  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004fe0ae  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004fe0b0  83c502                 -add ebp, 2
    (cpu.ebp) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004fe0b3  e837f6ffff             -call 0x4fd6ef
    cpu.esp -= 4;
    sub_4fd6ef(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fe0b8  8b84241c0a0000         -mov eax, dword ptr [esp + 0xa1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2588) /* 0xa1c */);
    // 004fe0bf  8b9424440a0000         -mov edx, dword ptr [esp + 0xa44]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2628) /* 0xa44 */);
    // 004fe0c6  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004fe0c8  39d5                   +cmp ebp, edx
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.edx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fe0ca  7cc2                   -jl 0x4fe08e
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004fe08e;
    }
    // 004fe0cc  8d442000               -lea eax, [eax]
    cpu.eax = x86::reg32(cpu.eax);
L_0x004fe0d0:
    // 004fe0d0  83bc245c0a000002       +cmp dword ptr [esp + 0xa5c], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2652) /* 0xa5c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fe0d8  0f85c2fdffff           -jne 0x4fdea0
    if (!cpu.flags.zf)
    {
        goto L_0x004fdea0;
    }
    // 004fe0de  8b9424440a0000         -mov edx, dword ptr [esp + 0xa44]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2628) /* 0xa44 */);
    // 004fe0e5  8b9c24100a0000         -mov ebx, dword ptr [esp + 0xa10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2576) /* 0xa10 */);
    // 004fe0ec  8b8424080a0000         -mov eax, dword ptr [esp + 0xa08]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2568) /* 0xa08 */);
    // 004fe0f3  29da                   -sub edx, ebx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ebx));
    // 004fe0f5  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004fe0f7  0fafc2                 -imul eax, edx
    cpu.eax = x86::reg32(x86::sreg64(x86::sreg32(cpu.eax)) * x86::sreg64(x86::sreg32(cpu.edx)));
    // 004fe0fa  3d00580200             +cmp eax, 0x25800
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(153600 /*0x25800*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fe0ff  0f8fab000000           -jg 0x4fe1b0
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004fe1b0;
    }
L_0x004fe105:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fe105  8b84240c0a0000         -mov eax, dword ptr [esp + 0xa0c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2572) /* 0xa0c */);
    // 004fe10c  898424400a0000         -mov dword ptr [esp + 0xa40], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2624) /* 0xa40 */) = cpu.eax;
    // 004fe113  8b8424040a0000         -mov eax, dword ptr [esp + 0xa04]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2564) /* 0xa04 */);
    // 004fe11a  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004fe11d  8984242c0a0000         -mov dword ptr [esp + 0xa2c], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2604) /* 0xa2c */) = cpu.eax;
    // 004fe124  8b8424440a0000         -mov eax, dword ptr [esp + 0xa44]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2628) /* 0xa44 */);
    // 004fe12b  8bac24100a0000         -mov ebp, dword ptr [esp + 0xa10]
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2576) /* 0xa10 */);
    // 004fe132  48                     -dec eax
    (cpu.eax)--;
    // 004fe133  45                     -inc ebp
    (cpu.ebp)++;
    // 004fe134  8984243c0a0000         -mov dword ptr [esp + 0xa3c], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2620) /* 0xa3c */) = cpu.eax;
    // 004fe13b  39c5                   +cmp ebp, eax
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fe13d  0f8d5dfdffff           -jge 0x4fdea0
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004fdea0;
    }
L_0x004fe143:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004fe143  89eb                   -mov ebx, ebp
    cpu.ebx = cpu.ebp;
    // 004fe145  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004fe147  8b8c242c0a0000         -mov ecx, dword ptr [esp + 0xa2c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2604) /* 0xa2c */);
    // 004fe14e  8b1524505600           -mov edx, dword ptr [0x565024]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656612) /* 0x565024 */);
    // 004fe154  8b0482                 -mov eax, dword ptr [edx + eax*4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + cpu.eax * 4);
    // 004fe157  8b1520505600           -mov edx, dword ptr [0x565020]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5656608) /* 0x565020 */);
    // 004fe15d  03049a                 -add eax, dword ptr [edx + ebx*4]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.edx + cpu.ebx * 4)));
    // 004fe160  030514505600           -add eax, dword ptr [0x565014]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(5656596) /* 0x565014 */)));
    // 004fe166  8b9424400a0000         -mov edx, dword ptr [esp + 0xa40]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2624) /* 0xa40 */);
    // 004fe16d  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004fe16f  83c502                 -add ebp, 2
    (cpu.ebp) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004fe172  8b8424400a0000         -mov eax, dword ptr [esp + 0xa40]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2624) /* 0xa40 */);
    // 004fe179  01ca                   -add edx, ecx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004fe17b  89f9                   -mov ecx, edi
    cpu.ecx = cpu.edi;
    // 004fe17d  899424000a0000         -mov dword ptr [esp + 0xa00], edx
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2560) /* 0xa00 */) = cpu.edx;
    // 004fe184  e82df6ffff             -call 0x4fd7b6
    cpu.esp -= 4;
    sub_4fd7b6(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fe189  8b8424000a0000         -mov eax, dword ptr [esp + 0xa00]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2560) /* 0xa00 */);
    // 004fe190  8b9c243c0a0000         -mov ebx, dword ptr [esp + 0xa3c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2620) /* 0xa3c */);
    // 004fe197  898424400a0000         -mov dword ptr [esp + 0xa40], eax
    app->getMemory<x86::reg32>(cpu.esp + x86::reg32(2624) /* 0xa40 */) = cpu.eax;
    // 004fe19e  39dd                   +cmp ebp, ebx
    {
        x86::reg32 tmp1 = cpu.ebp;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ebx));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004fe1a0  7ca1                   -jl 0x4fe143
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004fe143;
    }
    // 004fe1a2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004fe1a4  81c4480a0000           -add esp, 0xa48
    (cpu.esp) += x86::reg32(x86::sreg32(2632 /*0xa48*/));
    // 004fe1aa  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fe1ab  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fe1ac  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fe1ad  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
L_0x004fe1b0:
    // 004fe1b0  e82bafffff             -call 0x4f90e0
    cpu.esp -= 4;
    sub_4f90e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004fe1b5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004fe1b7  0f8548ffffff           -jne 0x4fe105
    if (!cpu.flags.zf)
    {
        goto L_0x004fe105;
    }
    // 004fe1bd  81c4480a0000           -add esp, 0xa48
    (cpu.esp) += x86::reg32(x86::sreg32(2632 /*0xa48*/));
    // 004fe1c3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fe1c4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fe1c5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004fe1c6  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* sub_4fe1d0: native, nfs3hp::watcom::prtf */

/* sub_4fe578: native, nfs3hp::watcom::prtfSpec */

/* sub_4fe6cc: native, nfs3hp::watcom::prtfFlags */

/* sub_4fe734: native, nfs3hp::watcom::prtfLength */

/* sub_4fe758: native, nfs3hp::watcom::prtfWideLength */

/* sub_4fe7c4: native, nfs3hp::watcom::prtfHex */

/* sub_4fe81e: native, nfs3hp::watcom::prtfHexEnd */

/* sub_4fe828: native, nfs3hp::watcom::prtfFixed */

/* sub_4fe924: native, nfs3hp::watcom::prtfFloat */

/* sub_4fe92c: native, nfs3hp::watcom::prtfZeros */

/* sub_4fe970: native, nfs3hp::watcom::prtfWideOut */

/* sub_4fe9d4: native, nfs3hp::watcom::prtfConvert */

/* sub_4fefdc: native, nfs3hp::watcom::prtfUpper */

/* align: skip 0x00 */
void Application::sub_4feffc(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004feffc  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004feffe  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4ff000: native, nfs3hp::watcom::file_putc */

/* sub_4ff014: native, nfs3hp::watcom::vfprintf */

/* sub_4ff0e0: native, nfs3hp::dead::stub_0x4ff0e0 */

/* sub_4ff0ec: native, nfs3hp::watcom::GetThreadPtr */

/* sub_4ff0f4: native, nfs3hp::watcom::NullAccess */

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4ff0f8(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ff0f8  e907f20100             -jmp 0x51e304
    return nfs3hp::watcom::allocPOSIXHandle(app, cpu.handOver());
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4ff100(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ff100  e957f30100             -jmp 0x51e45c
    return nfs3hp::watcom::freePOSIXHandle(app, cpu.handOver());
}

/* sub_4ff108: native, nfs3hp::watcom::NTInit */

/* sub_4ff338: native, nfs3hp::watcom::NTMain */

/* sub_4ff38c: native, nfs3hp::watcom::exit_ */

/* sub_4ff3d0: native, nfs3hp::watcom::freeEnvironment */

/* sub_4ff410: native, nfs3hp::watcom::callRoutine */

/* sub_4ff41c: native, nfs3hp::watcom::InitRtns */

/* sub_4ff46c: native, nfs3hp::watcom::FiniRtns */

/* sub_4ff4c0: native, nfs3hp::watcom::STOSB */

/* align: skip 0x90 */
void Application::sub_4ff4f2(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ff4f2  90                     -nop 
    ;
    // 004ff4f3  90                     -nop 
    ;
    // 004ff4f4  90                     -nop 
    ;
    // 004ff4f5  90                     -nop 
    ;
    // 004ff4f6  90                     -nop 
    ;
    // 004ff4f7  09c9                   +or ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx |= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004ff4f9  7467                   -je 0x4ff562
    if (cpu.flags.zf)
    {
        goto L_0x004ff562;
    }
L_0x004ff4fb:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ff4fb  a81f                   +test al, 0x1f
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & 31 /*0x1f*/));
    // 004ff4fd  7408                   -je 0x4ff507
    if (cpu.flags.zf)
    {
        goto L_0x004ff507;
    }
    // 004ff4ff  8910                   -mov dword ptr [eax], edx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.edx;
    // 004ff501  8d4004                 -lea eax, [eax + 4]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004ff504  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004ff505  75f4                   -jne 0x4ff4fb
    if (!cpu.flags.zf)
    {
        goto L_0x004ff4fb;
    }
L_0x004ff507:
    // 004ff507  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004ff508  c1e902                 +shr ecx, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.ecx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (tmp - 1));
            if (tmp == 1) cpu.flags.of = (1 & (op >> (32 - 1)));
            cpu.set_szp((op >>= tmp));
        }
    }
    // 004ff50b  743a                   -je 0x4ff547
    if (cpu.flags.zf)
    {
        goto L_0x004ff547;
    }
    // 004ff50d  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004ff50e  7429                   -je 0x4ff539
    if (cpu.flags.zf)
    {
        goto L_0x004ff539;
    }
L_0x004ff510:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004ff510  8910                   -mov dword ptr [eax], edx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.edx;
    // 004ff512  895004                 -mov dword ptr [eax + 4], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.edx;
    // 004ff515  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004ff516  895008                 -mov dword ptr [eax + 8], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = cpu.edx;
    // 004ff519  89500c                 -mov dword ptr [eax + 0xc], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */) = cpu.edx;
    // 004ff51c  7418                   -je 0x4ff536
    if (cpu.flags.zf)
    {
        goto L_0x004ff536;
    }
    // 004ff51e  385020                 +cmp byte ptr [eax + 0x20], dl
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(32) /* 0x20 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(cpu.dl));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004ff521  895010                 -mov dword ptr [eax + 0x10], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */) = cpu.edx;
    // 004ff524  895014                 -mov dword ptr [eax + 0x14], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */) = cpu.edx;
    // 004ff527  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004ff528  895018                 -mov dword ptr [eax + 0x18], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */) = cpu.edx;
    // 004ff52b  89501c                 -mov dword ptr [eax + 0x1c], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */) = cpu.edx;
    // 004ff52e  8d4020                 -lea eax, [eax + 0x20]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(32) /* 0x20 */);
    // 004ff531  75dd                   -jne 0x4ff510
    if (!cpu.flags.zf)
    {
        goto L_0x004ff510;
    }
    // 004ff533  8d40f0                 -lea eax, [eax - 0x10]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(-16) /* -0x10 */);
L_0x004ff536:
    // 004ff536  8d4010                 -lea eax, [eax + 0x10]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(16) /* 0x10 */);
L_0x004ff539:
    // 004ff539  8910                   -mov dword ptr [eax], edx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.edx;
    // 004ff53b  895004                 -mov dword ptr [eax + 4], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */) = cpu.edx;
    // 004ff53e  895008                 -mov dword ptr [eax + 8], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */) = cpu.edx;
    // 004ff541  89500c                 -mov dword ptr [eax + 0xc], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */) = cpu.edx;
    // 004ff544  8d4010                 -lea eax, [eax + 0x10]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(16) /* 0x10 */);
L_0x004ff547:
    // 004ff547  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ff548  83e103                 +and ecx, 3
    cpu.clear_co();
    cpu.set_szp((cpu.ecx &= x86::reg32(x86::sreg32(3 /*0x3*/))));
    // 004ff54b  7415                   -je 0x4ff562
    if (cpu.flags.zf)
    {
        goto L_0x004ff562;
    }
    // 004ff54d  8910                   -mov dword ptr [eax], edx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.edx;
    // 004ff54f  8d4004                 -lea eax, [eax + 4]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004ff552  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004ff553  740d                   -je 0x4ff562
    if (cpu.flags.zf)
    {
        goto L_0x004ff562;
    }
    // 004ff555  8910                   -mov dword ptr [eax], edx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.edx;
    // 004ff557  8d4004                 -lea eax, [eax + 4]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004ff55a  49                     +dec ecx
    {
        x86::reg32& tmp = cpu.ecx;
        cpu.flags.of = 1 & (tmp >> 31);
        tmp--;
        cpu.flags.of &= ~(1 & (tmp >> 31));
        cpu.set_szp(tmp);
    }
    // 004ff55b  7405                   -je 0x4ff562
    if (cpu.flags.zf)
    {
        goto L_0x004ff562;
    }
    // 004ff55d  8910                   -mov dword ptr [eax], edx
    app->getMemory<x86::reg32>(cpu.eax) = cpu.edx;
    // 004ff55f  8d4004                 -lea eax, [eax + 4]
    cpu.eax = x86::reg32(cpu.eax + x86::reg32(4) /* 0x4 */);
L_0x004ff562:
    // 004ff562  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4ff4f7: native, nfs3hp::watcom::STOSD */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4ff570(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ff570  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004ff571  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004ff572  ba40f15100             -mov edx, 0x51f140
    cpu.edx = 5370176 /*0x51f140*/;
    // 004ff577  bb54f25100             -mov ebx, 0x51f254
    cpu.ebx = 5370452 /*0x51f254*/;
    // 004ff57c  891564ac5600           -mov dword ptr [0x56ac64], edx
    app->getMemory<x86::reg32>(x86::reg32(5680228) /* 0x56ac64 */) = cpu.edx;
    // 004ff582  891d68ac5600           -mov dword ptr [0x56ac68], ebx
    app->getMemory<x86::reg32>(x86::reg32(5680232) /* 0x56ac68 */) = cpu.ebx;
    // 004ff588  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ff589  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ff58a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4ff590(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ff590  9b                     -wait 
    /*nothing*/;
    // 004ff591  dd30                   -fnsave dword ptr [eax]
    NFS2_ASSERT(false);
    // 004ff593  9b                     -wait 
    /*nothing*/;
    // 004ff594  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4ff598(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ff598  dd20                   -frstor dword ptr [eax]
    NFS2_ASSERT(false);
    // 004ff59a  9b                     -wait 
    /*nothing*/;
    // 004ff59b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4ff59c(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ff59c  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004ff59d  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004ff59e  803dc144560000         +cmp byte ptr [0x5644c1], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5653697) /* 0x5644c1 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004ff5a5  7416                   -je 0x4ff5bd
    if (cpu.flags.zf)
    {
        goto L_0x004ff5bd;
    }
    // 004ff5a7  ba90f54f00             -mov edx, 0x4ff590
    cpu.edx = 5240208 /*0x4ff590*/;
    // 004ff5ac  bb98f54f00             -mov ebx, 0x4ff598
    cpu.ebx = 5240216 /*0x4ff598*/;
    // 004ff5b1  8915f0ac5600           -mov dword ptr [0x56acf0], edx
    app->getMemory<x86::reg32>(x86::reg32(5680368) /* 0x56acf0 */) = cpu.edx;
    // 004ff5b7  891df4ac5600           -mov dword ptr [0x56acf4], ebx
    app->getMemory<x86::reg32>(x86::reg32(5680372) /* 0x56acf4 */) = cpu.ebx;
L_0x004ff5bd:
    // 004ff5bd  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004ff5bf  66a1f8ac5600           -mov ax, word ptr [0x56acf8]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(5680376) /* 0x56acf8 */);
    // 004ff5c5  e8a8fc0100             -call 0x51f272
    
    // 004ff5ca  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ff5cb  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ff5cc  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4ff5d0: native, nfs3hp::dead::stub_0x4ff5d0 */

/* align: skip 0x8b 0xc0 */
void Application::sub_4ff5dc(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ff5dc  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004ff5dd  8a25c0445600           -mov ah, byte ptr [0x5644c0]
    cpu.ah = app->getMemory<x86::reg8>(x86::reg32(5653696) /* 0x5644c0 */);
    // 004ff5e3  84e4                   +test ah, ah
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & cpu.ah));
    // 004ff5e5  7537                   -jne 0x4ff61e
    if (!cpu.flags.zf)
    {
        goto L_0x004ff61e;
    }
    // 004ff5e7  8825c1445600           -mov byte ptr [0x5644c1], ah
    app->getMemory<x86::reg8>(x86::reg32(5653697) /* 0x5644c1 */) = cpu.ah;
    // 004ff5ed  30f6                   -xor dh, dh
    cpu.dh ^= x86::reg8(x86::sreg8(cpu.dh));
    // 004ff5ef  2bc0                   -sub eax, eax
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004ff5f1  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004ff5f2  dbe3                   -fninit 
    cpu.fpu.init();
    // 004ff5f4  d93c24                 -fnstcw word ptr [esp]
    app->getMemory<x86::reg16>(cpu.esp) = cpu.fpu.control.word;
    // 004ff5f7  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ff5f8  8ac4                   -mov al, ah
    cpu.al = cpu.ah;
    // 004ff5fa  30d2                   -xor dl, dl
    cpu.dl ^= x86::reg8(x86::sreg8(cpu.dl));
    // 004ff5fc  3c03                   +cmp al, 3
    {
        x86::reg8 tmp1 = cpu.al;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(3 /*0x3*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004ff5fe  7509                   -jne 0x4ff609
    if (!cpu.flags.zf)
    {
        goto L_0x004ff609;
    }
    // 004ff600  e897ffffff             -call 0x4ff59c
    cpu.esp -= 4;
    sub_4ff59c(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004ff605  88c6                   -mov dh, al
    cpu.dh = cpu.al;
    // 004ff607  88c2                   -mov dl, al
    cpu.dl = cpu.al;
L_0x004ff609:
    // 004ff609  803d3478560000         +cmp byte ptr [0x567834], 0
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5666868) /* 0x567834 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(0 /*0x0*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004ff610  750c                   -jne 0x4ff61e
    if (!cpu.flags.zf)
    {
        goto L_0x004ff61e;
    }
    // 004ff612  8835c0445600           -mov byte ptr [0x5644c0], dh
    app->getMemory<x86::reg8>(x86::reg32(5653696) /* 0x5644c0 */) = cpu.dh;
    // 004ff618  8815c1445600           -mov byte ptr [0x5644c1], dl
    app->getMemory<x86::reg8>(x86::reg32(5653697) /* 0x5644c1 */) = cpu.dl;
L_0x004ff61e:
    // 004ff61e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004ff61f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4ff620(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004ff620  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4ff624: native, nfs3hp::dead::stub_0x4ff624 */

/* sub_4ff660: native, nfs3hp::port::setTopupFunction */

/* sub_4ff670: native, nfs3hp::port::topUpStream */

/* sub_4ff700: native, nfs3hp::port::waitTopupMain */

/* sub_4ff76e: native, nfs3hp::port::waitTopupEvent */

/* sub_4ff780: native, nfs3hp::port::topupDone */

/* sub_4ff7b0: native, nfs3hp::port::toppedUp */

/* sub_4ff7c0: native, nfs3hp::port::topUpBeforeLoad */

/* sub_4ff7f0: native, nfs3hp::port::waitTopupStream */

/* sub_4ff7e0: native, nfs3hp::port::topupReturn */

/* sub_4ff80f: native, nfs3hp::port::waitTopupDone */

/* sub_4ff820: native, nfs3hp::port::loadBlockSize */

/* sub_4ff830: native, nfs3hp::port::loadFlags */

/* sub_4ff840: native, nfs3hp::port::voicePicked */

/* sub_4ff86c: native, nfs3hp::port::iSNDallocchan */

/* sub_4ffb64: native, nfs3hp::port::iSNDfreechan */

/* sub_4ffc9c: native, nfs3hp::port::voiceOf */

}
