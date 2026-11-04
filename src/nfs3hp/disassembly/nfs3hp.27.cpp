#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{
// Port (tools/apply_widescreen.py): defined in nfs3hp_main.cpp.
bool widescreenMode(x86::reg32 width, x86::reg32 height);
x86::reg32 dynamicSavedMode(x86::reg32 found, x86::reg32 width);

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4be970(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004be970  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004be971  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004be972  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004be973  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004be974  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004be975  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004be976  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004be978  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004be97a  f605583a7a0002         +test byte ptr [0x7a3a58], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8010328) /* 0x7a3a58 */) & 2 /*0x2*/));
    // 004be981  7407                   -je 0x4be98a
    if (cpu.flags.zf)
    {
        goto L_0x004be98a;
    }
    // 004be983  b920000000             -mov ecx, 0x20
    cpu.ecx = 32 /*0x20*/;
    // 004be988  eb13                   -jmp 0x4be99d
    goto L_0x004be99d;
L_0x004be98a:
    // 004be98a  a168da7c00             -mov eax, dword ptr [0x7cda68]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8182376) /* 0x7cda68 */);
    // 004be98f  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004be991  b928000000             -mov ecx, 0x28
    cpu.ecx = 40 /*0x28*/;
    // 004be996  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004be999  f7f9                   -idiv ecx
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.ecx);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004be99b  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
L_0x004be99d:
    // 004be99d  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004be99f  7d02                   -jge 0x4be9a3
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004be9a3;
    }
    // 004be9a1  f7d9                   -neg ecx
    cpu.ecx = ~cpu.ecx + 1;
L_0x004be9a3:
    // 004be9a3  833d4cbc6f0000         +cmp dword ptr [0x6fbc4c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7322700) /* 0x6fbc4c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004be9aa  740d                   -je 0x4be9b9
    if (cpu.flags.zf)
    {
        goto L_0x004be9b9;
    }
    // 004be9ac  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 004be9b3  0f8571000000           -jne 0x4bea2a
    if (!cpu.flags.zf)
    {
        goto L_0x004bea2a;
    }
L_0x004be9b9:
    // 004be9b9  b888db7c00             -mov eax, 0x7cdb88
    cpu.eax = 8182664 /*0x7cdb88*/;
    // 004be9be  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004be9c0  8b3da0db7c00           -mov edi, dword ptr [0x7cdba0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(8182688) /* 0x7cdba0 */);
    // 004be9c6  e875000000             -call 0x4bea40
    cpu.esp -= 4;
    sub_4bea40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004be9cb  b880da7c00             -mov eax, 0x7cda80
    cpu.eax = 8182400 /*0x7cda80*/;
    // 004be9d0  8b1d94da7c00           -mov ebx, dword ptr [0x7cda94]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8182420) /* 0x7cda94 */);
    // 004be9d6  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004be9d8  8b359cda7c00           -mov esi, dword ptr [0x7cda9c]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8182428) /* 0x7cda9c */);
    // 004be9de  e85d000000             -call 0x4bea40
    cpu.esp -= 4;
    sub_4bea40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004be9e3  b8a0da7c00             -mov eax, 0x7cdaa0
    cpu.eax = 8182432 /*0x7cdaa0*/;
    // 004be9e8  891d94da7c00           -mov dword ptr [0x7cda94], ebx
    app->getMemory<x86::reg32>(x86::reg32(8182420) /* 0x7cda94 */) = cpu.ebx;
    // 004be9ee  89359cda7c00           -mov dword ptr [0x7cda9c], esi
    app->getMemory<x86::reg32>(x86::reg32(8182428) /* 0x7cda9c */) = cpu.esi;
    // 004be9f4  89ca                   -mov edx, ecx
    cpu.edx = cpu.ecx;
    // 004be9f6  8b1db4da7c00           -mov ebx, dword ptr [0x7cdab4]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8182452) /* 0x7cdab4 */);
    // 004be9fc  8b35bcda7c00           -mov esi, dword ptr [0x7cdabc]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8182460) /* 0x7cdabc */);
    // 004bea02  e839000000             -call 0x4bea40
    cpu.esp -= 4;
    sub_4bea40(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004bea07  891db4da7c00           -mov dword ptr [0x7cdab4], ebx
    app->getMemory<x86::reg32>(x86::reg32(8182452) /* 0x7cdab4 */) = cpu.ebx;
    // 004bea0d  8935bcda7c00           -mov dword ptr [0x7cdabc], esi
    app->getMemory<x86::reg32>(x86::reg32(8182460) /* 0x7cdabc */) = cpu.esi;
    // 004bea13  e8b8f9ffff             -call 0x4be3d0
    cpu.esp -= 4;
    nfs3hp::frontend::pauseFillViewQuads(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004bea18  3b3da0db7c00           +cmp edi, dword ptr [0x7cdba0]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182688) /* 0x7cdba0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004bea1e  7e0a                   -jle 0x4bea2a
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004bea2a;
    }
    // 004bea20  c705a8db7c0002000000   -mov dword ptr [0x7cdba8], 2
    app->getMemory<x86::reg32>(x86::reg32(8182696) /* 0x7cdba8 */) = 2 /*0x2*/;
L_0x004bea2a:
    // 004bea2a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bea2b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bea2c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bea2d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bea2e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bea2f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bea30  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4bea40(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004bea40  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004bea41  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004bea42  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004bea43  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004bea44  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004bea45  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004bea47  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004bea4a  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004bea4c  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 004bea4e  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004bea55  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004bea57  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004bea59  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004bea5c  c1e202                 +shl edx, 2
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
    // 004bea5f  1bc2                   -sbb eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx) + cpu.flags.cf);
    // 004bea61  c1f802                 -sar eax, 2
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (2 /*0x2*/ % 32));
    // 004bea64  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004bea66  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004bea68  0f8481000000           -je 0x4beaef
    if (cpu.flags.zf)
    {
        goto L_0x004beaef;
    }
    // 004bea6e  8b410c                 -mov eax, dword ptr [ecx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 004bea71  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 004bea73  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004bea76  a168da7c00             -mov eax, dword ptr [0x7cda68]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8182376) /* 0x7cda68 */);
    // 004bea7b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004bea7d  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004bea80  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004bea82  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004bea84  8b7908                 -mov edi, dword ptr [ecx + 8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 004bea87  01df                   -add edi, ebx
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004bea89  39c7                   +cmp edi, eax
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.eax));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004bea8b  7c62                   -jl 0x4beaef
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004beaef;
    }
    // 004bea8d  3b3d68da7c00           +cmp edi, dword ptr [0x7cda68]
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8182376) /* 0x7cda68 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004bea93  7f5a                   -jg 0x4beaef
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004beaef;
    }
    // 004bea95  837dfc00               +cmp dword ptr [ebp - 4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004bea99  7c54                   -jl 0x4beaef
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004beaef;
    }
    // 004bea9b  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004bea9d  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004bea9f  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004beaa2  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004beaa4  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004beaa6  8b11                   -mov edx, dword ptr [ecx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx);
    // 004beaa8  8b7910                 -mov edi, dword ptr [ecx + 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(16) /* 0x10 */);
    // 004beaab  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004beaad  29c7                   -sub edi, eax
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004beaaf  8911                   -mov dword ptr [ecx], edx
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.edx;
    // 004beab1  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004beab3  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004beab5  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004beab8  2bc2                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004beaba  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004beabc  897910                 -mov dword ptr [ecx + 0x10], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(16) /* 0x10 */) = cpu.edi;
    // 004beabf  8b7914                 -mov edi, dword ptr [ecx + 0x14]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(20) /* 0x14 */);
    // 004beac2  29c7                   -sub edi, eax
    (cpu.edi) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004beac4  8b5104                 -mov edx, dword ptr [ecx + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 004beac7  897914                 -mov dword ptr [ecx + 0x14], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(20) /* 0x14 */) = cpu.edi;
    // 004beaca  29c2                   -sub edx, eax
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.eax));
    // 004beacc  8b4108                 -mov eax, dword ptr [ecx + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 004beacf  895104                 -mov dword ptr [ecx + 4], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(4) /* 0x4 */) = cpu.edx;
    // 004bead2  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004bead4  8b791c                 -mov edi, dword ptr [ecx + 0x1c]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */);
    // 004bead7  894108                 -mov dword ptr [ecx + 8], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004beada  01f7                   -add edi, esi
    (cpu.edi) += x86::reg32(x86::sreg32(cpu.esi));
    // 004beadc  8b5118                 -mov edx, dword ptr [ecx + 0x18]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */);
    // 004beadf  89791c                 -mov dword ptr [ecx + 0x1c], edi
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */) = cpu.edi;
    // 004beae2  01da                   -add edx, ebx
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004beae4  8b590c                 -mov ebx, dword ptr [ecx + 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 004beae7  895118                 -mov dword ptr [ecx + 0x18], edx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(24) /* 0x18 */) = cpu.edx;
    // 004beaea  01f3                   -add ebx, esi
    (cpu.ebx) += x86::reg32(x86::sreg32(cpu.esi));
    // 004beaec  89590c                 -mov dword ptr [ecx + 0xc], ebx
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */) = cpu.ebx;
L_0x004beaef:
    // 004beaef  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004beaf1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004beaf2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004beaf3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004beaf4  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004beaf5  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004beaf6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4beb00: native, nfs3hp::frontend::pauseSetRectFirst */

/* sub_4beb20: native, nfs3hp::frontend::pauseReadRect */

/* sub_4beb40: native, nfs3hp::frontend::pauseSetRectSecond */

/* sub_4beb60: native, nfs3hp::dead::stub_0x4beb60 */

/* sub_4beb80: native, nfs3hp::port::text::viewport */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4bebb0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004bebb0  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004bebb1  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004bebb3  a360da7c00             -mov dword ptr [0x7cda60], eax
    app->getMemory<x86::reg32>(x86::reg32(8182368) /* 0x7cda60 */) = cpu.eax;
    // 004bebb8  891564da7c00           -mov dword ptr [0x7cda64], edx
    app->getMemory<x86::reg32>(x86::reg32(8182372) /* 0x7cda64 */) = cpu.edx;
    // 004bebbe  891d68da7c00           -mov dword ptr [0x7cda68], ebx
    app->getMemory<x86::reg32>(x86::reg32(8182376) /* 0x7cda68 */) = cpu.ebx;
    // 004bebc4  890d6cda7c00           -mov dword ptr [0x7cda6c], ecx
    app->getMemory<x86::reg32>(x86::reg32(8182380) /* 0x7cda6c */) = cpu.ecx;
    // 004bebca  e811000000             -call 0x4bebe0
    cpu.esp -= 4;
    nfs3hp::graphics::setClip(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004bebcf  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bebd0  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4bebe0: native, nfs3hp::graphics::setClip */

/* sub_4becd0: native, nfs3hp::dead::stub_0x4becd0 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_4bed20(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004bed20  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004bed21  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004bed22  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004bed23  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004bed25  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004bed27  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004bed28  8b480c                 -mov ecx, dword ptr [eax + 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004bed2b  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004bed2c  8b5808                 -mov ebx, dword ptr [eax + 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004bed2f  8b5004                 -mov edx, dword ptr [eax + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004bed32  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 004bed34  e8a7cf0100             -call 0x4dbce0
    cpu.esp -= 4;
    sub_4dbce0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004bed39  8b4e1c                 -mov ecx, dword ptr [esi + 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(28) /* 0x1c */);
    // 004bed3c  8b5e18                 -mov ebx, dword ptr [esi + 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(24) /* 0x18 */);
    // 004bed3f  8b5614                 -mov edx, dword ptr [esi + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 004bed42  8b4610                 -mov eax, dword ptr [esi + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(16) /* 0x10 */);
    // 004bed45  e896feffff             -call 0x4bebe0
    cpu.esp -= 4;
    nfs3hp::graphics::setClip(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004bed4a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bed4b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bed4c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bed4d  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8b 0xc0 */
void Application::sub_4bed50(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004bed50  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004bed51  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004bed52  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004bed54  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004bed55  8b480c                 -mov ecx, dword ptr [eax + 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004bed58  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004bed59  8b5808                 -mov ebx, dword ptr [eax + 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004bed5c  8b5004                 -mov edx, dword ptr [eax + 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(4) /* 0x4 */);
    // 004bed5f  8b00                   -mov eax, dword ptr [eax]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax);
    // 004bed61  e87acf0100             -call 0x4dbce0
    cpu.esp -= 4;
    sub_4dbce0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004bed66  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bed67  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bed68  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4bed70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004bed70  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004bed71  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004bed72  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004bed73  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004bed74  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004bed76  8b481c                 -mov ecx, dword ptr [eax + 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(28) /* 0x1c */);
    // 004bed79  8b5818                 -mov ebx, dword ptr [eax + 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 004bed7c  8b5014                 -mov edx, dword ptr [eax + 0x14]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 004bed7f  8b4010                 -mov eax, dword ptr [eax + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */);
    // 004bed82  e859feffff             -call 0x4bebe0
    cpu.esp -= 4;
    nfs3hp::graphics::setClip(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004bed87  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bed88  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bed89  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bed8a  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bed8b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4bed90: native, nfs3hp::graphics::listScreenModes */

/* sub_4bee80: native, nfs3hp::graphics::findScreenMode */

/* sub_4bef50: native, nfs3hp::render::screenMode */

/* align: skip 0x00 */
void Application::sub_4bf110(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
    // 004bf110  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004bf111  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004bf112  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004bf114  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004bf117  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004bf119  eb07                   -jmp 0x4bf122
    goto L_0x004bf122;
L_0x004bf11b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004bf11b  3d00010000             +cmp eax, 0x100
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(256 /*0x100*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004bf120  7d31                   -jge 0x4bf153
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004bf153;
    }
L_0x004bf122:
    // 004bf122  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004bf124  c1e20f                 -shl edx, 0xf
    cpu.edx <<= 15 /*0xf*/ % 32;
    // 004bf127  81ca0040803f           -or edx, 0x3f804000
    cpu.edx |= x86::reg32(x86::sreg32(1065369600 /*0x3f804000*/));
    // 004bf12d  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004bf130  d945fc                 -fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004bf133  d9e8                   -fld1 
    fp1 = 1.0;
    // 004bf135  def1                   -fdivrp st(1)
    fp0 = fp::div(fp1, x86::Float(fp0));
    // 004bf137  d95dfc                 -fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
    // 004bf13a  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004bf13d  81e2ffff7f00           -and edx, 0x7fffff
    cpu.edx &= x86::reg32(x86::sreg32(8388607 /*0x7fffff*/));
    // 004bf143  40                     -inc eax
    (cpu.eax)++;
    // 004bf144  81ca0000807e           +or edx, 0x7e800000
    cpu.clear_co();
    cpu.set_szp((cpu.edx |= x86::reg32(x86::sreg32(2122317824 /*0x7e800000*/))));
    // 004bf14a  891485d4307d00         -mov dword ptr [eax*4 + 0x7d30d4], edx
    app->getMemory<x86::reg32>(x86::reg32(8204500) /* 0x7d30d4 */ + cpu.eax * 4) = cpu.edx;
    // 004bf151  ebc8                   -jmp 0x4bf11b
    goto L_0x004bf11b;
L_0x004bf153:
    // 004bf153  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004bf155  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bf156  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bf157  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4bf160: native, nfs3hp::dead::stub_0x4bf160 */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4bf1b0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004bf1b0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004bf1b1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004bf1b2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004bf1b3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004bf1b5  8b4510                 -mov eax, dword ptr [ebp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004bf1b8  a3f4347d00             -mov dword ptr [0x7d34f4], eax
    app->getMemory<x86::reg32>(x86::reg32(8205556) /* 0x7d34f4 */) = cpu.eax;
    // 004bf1bd  8b4518                 -mov eax, dword ptr [ebp + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 004bf1c0  a3ec347d00             -mov dword ptr [0x7d34ec], eax
    app->getMemory<x86::reg32>(x86::reg32(8205548) /* 0x7d34ec */) = cpu.eax;
    // 004bf1c5  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004bf1c8  a3e0347d00             -mov dword ptr [0x7d34e0], eax
    app->getMemory<x86::reg32>(x86::reg32(8205536) /* 0x7d34e0 */) = cpu.eax;
    // 004bf1cd  8b451c                 -mov eax, dword ptr [ebp + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004bf1d0  a3dc347d00             -mov dword ptr [0x7d34dc], eax
    app->getMemory<x86::reg32>(x86::reg32(8205532) /* 0x7d34dc */) = cpu.eax;
    // 004bf1d5  8b4524                 -mov eax, dword ptr [ebp + 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(36) /* 0x24 */);
    // 004bf1d8  a3e8347d00             -mov dword ptr [0x7d34e8], eax
    app->getMemory<x86::reg32>(x86::reg32(8205544) /* 0x7d34e8 */) = cpu.eax;
    // 004bf1dd  8b4520                 -mov eax, dword ptr [ebp + 0x20]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(32) /* 0x20 */);
    // 004bf1e0  a3f0347d00             -mov dword ptr [0x7d34f0], eax
    app->getMemory<x86::reg32>(x86::reg32(8205552) /* 0x7d34f0 */) = cpu.eax;
    // 004bf1e5  8b4510                 -mov eax, dword ptr [ebp + 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004bf1e8  a3fc347d00             -mov dword ptr [0x7d34fc], eax
    app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */) = cpu.eax;
    // 004bf1ed  8b4518                 -mov eax, dword ptr [ebp + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 004bf1f0  a30c357d00             -mov dword ptr [0x7d350c], eax
    app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */) = cpu.eax;
    // 004bf1f5  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004bf1f8  a304357d00             -mov dword ptr [0x7d3504], eax
    app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */) = cpu.eax;
    // 004bf1fd  8b451c                 -mov eax, dword ptr [ebp + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004bf200  ba0000803f             -mov edx, 0x3f800000
    cpu.edx = 1065353216 /*0x3f800000*/;
    // 004bf205  a300357d00             -mov dword ptr [0x7d3500], eax
    app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */) = cpu.eax;
    // 004bf20a  a1e8347d00             -mov eax, dword ptr [0x7d34e8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205544) /* 0x7d34e8 */);
    // 004bf20f  8915e4347d00           -mov dword ptr [0x7d34e4], edx
    app->getMemory<x86::reg32>(x86::reg32(8205540) /* 0x7d34e4 */) = cpu.edx;
    // 004bf215  a3d8347d00             -mov dword ptr [0x7d34d8], eax
    app->getMemory<x86::reg32>(x86::reg32(8205528) /* 0x7d34d8 */) = cpu.eax;
    // 004bf21a  a1f0347d00             -mov eax, dword ptr [0x7d34f0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205552) /* 0x7d34f0 */);
    // 004bf21f  a3f8347d00             -mov dword ptr [0x7d34f8], eax
    app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */) = cpu.eax;
    // 004bf224  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004bf226  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004bf228  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004bf22a  c1e205                 -shl edx, 5
    cpu.edx <<= 5 /*0x5*/ % 32;
    // 004bf22d  40                     -inc eax
    (cpu.eax)++;
    // 004bf22e  898ac4f47c00           -mov dword ptr [edx + 0x7cf4c4], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8189124) /* 0x7cf4c4 */) = cpu.ecx;
L_0x004bf234:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004bf234  3dc8000000             +cmp eax, 0xc8
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(200 /*0xc8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004bf239  7d10                   -jge 0x4bf24b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004bf24b;
    }
    // 004bf23b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004bf23d  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004bf23f  c1e205                 +shl edx, 5
    {
        x86::reg8 tmp = 5 /*0x5*/ % 32;
        x86::reg32& op = cpu.edx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004bf242  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004bf243  898ac4f47c00           -mov dword ptr [edx + 0x7cf4c4], ecx
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8189124) /* 0x7cf4c4 */) = cpu.ecx;
    // 004bf249  ebe9                   -jmp 0x4bf234
    goto L_0x004bf234;
L_0x004bf24b:
    // 004bf24b  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bf24c  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bf24d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bf24e  c21800                 -ret 0x18
    cpu.esp += 4+24 /*0x18*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4bf260(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2, fp3;
    // 004bf260  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004bf261  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004bf262  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004bf263  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004bf264  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004bf266  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004bf269  d905c0015400           -fld dword ptr [0x5401c0]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(5505472) /* 0x5401c0 */));
    // 004bf26f  d9451c                 -fld dword ptr [ebp + 0x1c]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(28) /* 0x1c */));
    // 004bf272  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004bf274  d94520                 -fld dword ptr [ebp + 0x20]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(32) /* 0x20 */));
    // 004bf277  deca                   -fmulp st(2)
    fp0 = fp::mul(fp0, fp2);
    // 004bf279  8b451c                 -mov eax, dword ptr [ebp + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004bf27c  8b5524                 -mov edx, dword ptr [ebp + 0x24]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(36) /* 0x24 */);
    // 004bf27f  a39c005600             -mov dword ptr [0x56009c], eax
    app->getMemory<x86::reg32>(x86::reg32(5636252) /* 0x56009c */) = cpu.eax;
    // 004bf284  d91dac005600           -fstp dword ptr [0x5600ac]
    app->getMemory<float>(x86::reg32(5636268) /* 0x5600ac */) = float(fp1);
    // 004bf28a  d915b0005600           -fst dword ptr [0x5600b0]
    app->getMemory<float>(x86::reg32(5636272) /* 0x5600b0 */) = float(fp0);
    // 004bf290  d905ac005600           -fld dword ptr [0x5600ac]
    fp1 = x86::Float(app->getMemory<float>(x86::reg32(5636268) /* 0x5600ac */));
    // 004bf296  d84514                 -fadd dword ptr [ebp + 0x14]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(20) /* 0x14 */)));
    // 004bf299  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004bf29b  d84518                 -fadd dword ptr [ebp + 0x18]
    fp1 = fp::add(fp1, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(24) /* 0x18 */)));
    // 004bf29e  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004bf2a0  d91da4005600           -fstp dword ptr [0x5600a4]
    app->getMemory<float>(x86::reg32(5636260) /* 0x5600a4 */) = float(fp1);
    // 004bf2a6  d91da8005600           -fstp dword ptr [0x5600a8]
    app->getMemory<float>(x86::reg32(5636264) /* 0x5600a8 */) = float(fp0);
    // 004bf2ac  f7c2ffffff7f           +test edx, 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & 2147483647 /*0x7fffffff*/));
    // 004bf2b2  750c                   -jne 0x4bf2c0
    if (!cpu.flags.zf)
    {
        goto L_0x004bf2c0;
    }
    // 004bf2b4  d94520                 +fld dword ptr [ebp + 0x20]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(32) /* 0x20 */));
    // 004bf2b7  d9451c                 +fld dword ptr [ebp + 0x1c]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(28) /* 0x1c */));
    // 004bf2ba  d8f1                   +fdiv st(1)
    fp1 = fp::div(fp1, x86::Float(fp0));
    // 004bf2bc  dec9                   +fmulp st(1)
    fp0 = fp::mul(fp0, fp1);
    // 004bf2be  eb60                   -jmp 0x4bf320
    goto L_0x004bf320;
L_0x004bf2c0:
    // 004bf2c0  d94524                 -fld dword ptr [ebp + 0x24]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(36) /* 0x24 */));
    // 004bf2c3  d80dc4015400           -fmul dword ptr [0x5401c4]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5505476) /* 0x5401c4 */)));
    // 004bf2c9  8b4d28                 -mov ecx, dword ptr [ebp + 0x28]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(40) /* 0x28 */);
    // 004bf2cc  d95dfc                 -fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
    // 004bf2cf  f7c1ffffff7f           +test ecx, 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & 2147483647 /*0x7fffffff*/));
    // 004bf2d5  7525                   -jne 0x4bf2fc
    if (!cpu.flags.zf)
    {
        goto L_0x004bf2fc;
    }
    // 004bf2d7  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004bf2da  d9f2                   +fptan 
    fp0 = fp::tan(fp0);
    fp1 = 1.0;
    // 004bf2dc  ddd8                   +fstp st(0)
    fp1 = x86::Float(fp1);
    // 004bf2de  d9451c                 +fld dword ptr [ebp + 0x1c]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(28) /* 0x1c */));
    // 004bf2e1  d9c0                   +fld st(0)
    fp2 = x86::Float(fp1);
    // 004bf2e3  d9e8                   +fld1 
    fp3 = 1.0;
    // 004bf2e5  def3                   +fdivrp st(3)
    fp0 = fp::div(fp3, x86::Float(fp0));
    // 004bf2e7  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004bf2e9  d94520                 +fld dword ptr [ebp + 0x20]
    fp3 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(32) /* 0x20 */));
    // 004bf2ec  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp3;
        fp3 = fp2;
        fp2 = tmp;
    }
    // 004bf2ee  d8f1                   +fdiv st(1)
    fp3 = fp::div(fp3, x86::Float(fp2));
    // 004bf2f0  dec9                   +fmulp st(1)
    fp2 = fp::mul(fp2, fp3);
    // 004bf2f2  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004bf2f4  d8ca                   +fmul st(2)
    fp2 = fp::mul(fp2, x86::Float(fp0));
    // 004bf2f6  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp2;
        fp2 = fp1;
        fp1 = tmp;
    }
    // 004bf2f8  deca                   +fmulp st(2)
    fp0 = fp::mul(fp0, fp2);
    // 004bf2fa  eb1e                   -jmp 0x4bf31a
    goto L_0x004bf31a;
L_0x004bf2fc:
    // 004bf2fc  d945fc                 -fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004bf2ff  d9f2                   -fptan 
    fp0 = fp::tan(fp0);
    fp1 = 1.0;
    // 004bf301  ddd8                   -fstp st(0)
    fp1 = x86::Float(fp1);
    // 004bf303  d94528                 -fld dword ptr [ebp + 0x28]
    fp1 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(40) /* 0x28 */));
    // 004bf306  d80dc4015400           -fmul dword ptr [0x5401c4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(x86::reg32(5505476) /* 0x5401c4 */)));
    // 004bf30c  d9f2                   -fptan 
    fp1 = fp::tan(fp1);
    fp2 = 1.0;
    // 004bf30e  ddd8                   -fstp st(0)
    fp2 = x86::Float(fp2);
    // 004bf310  d9451c                 -fld dword ptr [ebp + 0x1c]
    fp2 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(28) /* 0x1c */));
    // 004bf313  def2                   -fdivrp st(2)
    fp0 = fp::div(fp2, x86::Float(fp0));
    // 004bf315  d87d20                 -fdivr dword ptr [ebp + 0x20]
    fp1 = fp::div(x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(32) /* 0x20 */)), fp1);
    // 004bf318  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
L_0x004bf31a:
    // 004bf31a  d91d9c005600           -fstp dword ptr [0x56009c]
    app->getMemory<float>(x86::reg32(5636252) /* 0x56009c */) = float(fp1);
L_0x004bf320:
    // 004bf320  d91da0005600           -fstp dword ptr [0x5600a0]
    app->getMemory<float>(x86::reg32(5636256) /* 0x5600a0 */) = float(fp0);
    // 004bf326  8a25a3005600           -mov ah, byte ptr [0x5600a3]
    cpu.ah = app->getMemory<x86::reg8>(x86::reg32(5636259) /* 0x5600a3 */);
    // 004bf32c  d9059c005600           -fld dword ptr [0x56009c]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(5636252) /* 0x56009c */));
    // 004bf332  80f480                 -xor ah, 0x80
    cpu.ah ^= x86::reg8(x86::sreg8(128 /*0x80*/));
    // 004bf335  d905c0015400           -fld dword ptr [0x5401c0]
    fp1 = x86::Float(app->getMemory<float>(x86::reg32(5505472) /* 0x5401c0 */));
    // 004bf33b  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004bf33d  d8c9                   -fmul st(1)
    fp1 = fp::mul(fp1, x86::Float(fp0));
    // 004bf33f  8825a3005600           -mov byte ptr [0x5600a3], ah
    app->getMemory<x86::reg8>(x86::reg32(5636259) /* 0x5600a3 */) = cpu.ah;
    // 004bf345  d905a0005600           -fld dword ptr [0x5600a0]
    fp2 = x86::Float(app->getMemory<float>(x86::reg32(5636256) /* 0x5600a0 */));
    // 004bf34b  deca                   -fmulp st(2)
    fp0 = fp::mul(fp0, fp2);
    // 004bf34d  8b5d2c                 -mov ebx, dword ptr [ebp + 0x2c]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(44) /* 0x2c */);
    // 004bf350  d91d9c005600           -fstp dword ptr [0x56009c]
    app->getMemory<float>(x86::reg32(5636252) /* 0x56009c */) = float(fp1);
    // 004bf356  d91da0005600           -fstp dword ptr [0x5600a0]
    app->getMemory<float>(x86::reg32(5636256) /* 0x5600a0 */) = float(fp0);
    // 004bf35c  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004bf35e  7407                   -je 0x4bf367
    if (cpu.flags.zf)
    {
        goto L_0x004bf367;
    }
    // 004bf360  80359f00560080         -xor byte ptr [0x56009f], 0x80
    app->getMemory<x86::reg8>(x86::reg32(5636255) /* 0x56009f */) ^= x86::reg8(x86::sreg8(128 /*0x80*/));
L_0x004bf367:
    // 004bf367  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004bf369  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bf36a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bf36b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bf36c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bf36d  c21c00                 -ret 0x1c
    cpu.esp += 4+28 /*0x1c*/;
    return;
}

/* align: skip  */
void Application::sub_4bf370(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1, fp2;
    // 004bf370  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004bf371  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004bf372  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004bf373  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004bf374  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004bf376  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004bf379  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004bf37c  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004bf37f  895df8                 -mov dword ptr [ebp - 8], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.ebx;
    // 004bf382  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004bf384  0f8e22010000           -jle 0x4bf4ac
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004bf4ac;
    }
    // 004bf38a  8b4a08                 -mov ecx, dword ptr [edx + 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 004bf38d  f7c1ffffff7f           +test ecx, 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & 2147483647 /*0x7fffffff*/));
    // 004bf393  7507                   -jne 0x4bf39c
    if (!cpu.flags.zf)
    {
        goto L_0x004bf39c;
    }
    // 004bf395  c7420880008037         -mov dword ptr [edx + 8], 0x37800080
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */) = 931135616 /*0x37800080*/;
L_0x004bf39c:
    // 004bf39c  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004bf39f  d9e8                   -fld1 
    fp0 = 1.0;
    // 004bf3a1  d87008                 -fdiv dword ptr [eax + 8]
    fp0 = fp::div(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 004bf3a4  d95df0                 -fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fp0);
L_0x004bf3a7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004bf3a7  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004bf3aa  4b                     -dec ebx
    (cpu.ebx)--;
    // 004bf3ab  895df4                 -mov dword ptr [ebp - 0xc], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ebx;
    // 004bf3ae  83fbff                 +cmp ebx, -1
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
    // 004bf3b1  0f84f5000000           -je 0x4bf4ac
    if (cpu.flags.zf)
    {
        goto L_0x004bf4ac;
    }
    // 004bf3b7  8d4df0                 -lea ecx, [ebp - 0x10]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004bf3ba  8b5df8                 -mov ebx, dword ptr [ebp - 8]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004bf3bd  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004bf3c0  d9059c005600           -fld dword ptr [0x56009c]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(5636252) /* 0x56009c */));
    // 004bf3c6  d809                   -fmul dword ptr [ecx]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ecx)));
    // 004bf3c8  d905a0005600           -fld dword ptr [0x5600a0]
    fp1 = x86::Float(app->getMemory<float>(x86::reg32(5636256) /* 0x5600a0 */));
    // 004bf3ce  d809                   -fmul dword ptr [ecx]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.ecx)));
    // 004bf3d0  8b11                   -mov edx, dword ptr [ecx]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ecx);
    // 004bf3d2  d84804                 -fmul dword ptr [eax + 4]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */)));
    // 004bf3d5  d9c9                   -fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004bf3d7  89530c                 -mov dword ptr [ebx + 0xc], edx
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(12) /* 0xc */) = cpu.edx;
    // 004bf3da  d808                   -fmul dword ptr [eax]
    fp1 = fp::mul(fp1, x86::Float(app->getMemory<float>(cpu.eax)));
    // 004bf3dc  8b5008                 -mov edx, dword ptr [eax + 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8) /* 0x8 */);
    // 004bf3df  d905a4005600           -fld dword ptr [0x5600a4]
    fp2 = x86::Float(app->getMemory<float>(x86::reg32(5636260) /* 0x5600a4 */));
    // 004bf3e5  dec1                   -faddp st(1)
    fp1 = fp::add(fp1, fp2);
    // 004bf3e7  d905a8005600           -fld dword ptr [0x5600a8]
    fp2 = x86::Float(app->getMemory<float>(x86::reg32(5636264) /* 0x5600a8 */));
    // 004bf3ed  dec2                   -faddp st(2)
    fp0 = fp::add(fp0, fp2);
    // 004bf3ef  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp1);
    // 004bf3f1  895308                 -mov dword ptr [ebx + 8], edx
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8) /* 0x8 */) = cpu.edx;
    // 004bf3f4  d95b04                 -fstp dword ptr [ebx + 4]
    app->getMemory<float>(cpu.ebx + x86::reg32(4) /* 0x4 */) = float(fp0);
    // 004bf3f7  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004bf3fa  48                     -dec eax
    (cpu.eax)--;
    // 004bf3fb  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004bf3fd  7e1e                   -jle 0x4bf41d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004bf41d;
    }
    // 004bf3ff  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004bf402  f74014ffffff7f         +test dword ptr [eax + 0x14], 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */) & 2147483647 /*0x7fffffff*/));
    // 004bf409  7507                   -jne 0x4bf412
    if (!cpu.flags.zf)
    {
        goto L_0x004bf412;
    }
    // 004bf40b  c7401480008037         -mov dword ptr [eax + 0x14], 0x37800080
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */) = 931135616 /*0x37800080*/;
L_0x004bf412:
    // 004bf412  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004bf415  d9e8                   -fld1 
    fp0 = 1.0;
    // 004bf417  d87014                 -fdiv dword ptr [eax + 0x14]
    fp0 = fp::div(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(20) /* 0x14 */)));
    // 004bf41a  d95df0                 -fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fp0);
L_0x004bf41d:
    // 004bf41d  8b7df8                 -mov edi, dword ptr [ebp - 8]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004bf420  8b75f8                 -mov esi, dword ptr [ebp - 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004bf423  83c714                 -add edi, 0x14
    (cpu.edi) += x86::reg32(x86::sreg32(20 /*0x14*/));
    // 004bf426  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004bf429  8b0df8347d00           -mov ecx, dword ptr [0x7d34f8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205560) /* 0x7d34f8 */);
    // 004bf42f  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004bf434  7558                   -jne 0x4bf48e
    if (!cpu.flags.zf)
    {
        goto L_0x004bf48e;
    }
    // 004bf436  39c8                   +cmp eax, ecx
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
    // 004bf438  7d54                   -jge 0x4bf48e
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004bf48e;
    }
    // 004bf43a  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004bf43d  bb00000000             -mov ebx, 0
    cpu.ebx = 0 /*0x0*/;
    // 004bf442  8b0d04357d00           -mov ecx, dword ptr [0x7d3504]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205572) /* 0x7d3504 */);
    // 004bf448  8b1500357d00           -mov edx, dword ptr [0x7d3500]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205568) /* 0x7d3500 */);
    // 004bf44e  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004bf453  750d                   -jne 0x4bf462
    if (!cpu.flags.zf)
    {
        goto L_0x004bf462;
    }
    // 004bf455  39c8                   +cmp eax, ecx
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
    // 004bf457  7c09                   -jl 0x4bf462
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004bf462;
    }
    // 004bf459  39d0                   +cmp eax, edx
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
    // 004bf45b  7e09                   -jle 0x4bf466
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004bf466;
    }
    // 004bf45d  83cb04                 +or ebx, 4
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(4 /*0x4*/))));
    // 004bf460  eb04                   -jmp 0x4bf466
    goto L_0x004bf466;
L_0x004bf462:
    // 004bf462  83cb08                 -or ebx, 8
    cpu.ebx |= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004bf465  90                     -nop 
    ;
L_0x004bf466:
    // 004bf466  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004bf468  90                     -nop 
    ;
    // 004bf469  8b0dfc347d00           -mov ecx, dword ptr [0x7d34fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205564) /* 0x7d34fc */);
    // 004bf46f  8b150c357d00           -mov edx, dword ptr [0x7d350c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205580) /* 0x7d350c */);
    // 004bf475  a900000080             +test eax, 0x80000000
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & 2147483648 /*0x80000000*/));
    // 004bf47a  750d                   -jne 0x4bf489
    if (!cpu.flags.zf)
    {
        goto L_0x004bf489;
    }
    // 004bf47c  39c8                   +cmp eax, ecx
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
    // 004bf47e  7c09                   -jl 0x4bf489
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004bf489;
    }
    // 004bf480  39d0                   +cmp eax, edx
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
    // 004bf482  7e0f                   -jle 0x4bf493
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004bf493;
    }
    // 004bf484  83cb02                 +or ebx, 2
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(2 /*0x2*/))));
    // 004bf487  eb0a                   -jmp 0x4bf493
    goto L_0x004bf493;
L_0x004bf489:
    // 004bf489  83cb01                 +or ebx, 1
    cpu.clear_co();
    cpu.set_szp((cpu.ebx |= x86::reg32(x86::sreg32(1 /*0x1*/))));
    // 004bf48c  eb05                   -jmp 0x4bf493
    goto L_0x004bf493;
L_0x004bf48e:
    // 004bf48e  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
L_0x004bf493:
    // 004bf493  881f                   -mov byte ptr [edi], bl
    app->getMemory<x86::reg8>(cpu.edi) = cpu.bl;
    // 004bf495  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004bf498  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004bf49b  83c00c                 -add eax, 0xc
    (cpu.eax) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004bf49e  83c220                 +add edx, 0x20
    {
        x86::reg32& tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(32 /*0x20*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004bf4a1  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004bf4a4  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 004bf4a7  e9fbfeffff             -jmp 0x4bf3a7
    goto L_0x004bf3a7;
L_0x004bf4ac:
    // 004bf4ac  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004bf4ae  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bf4af  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bf4b0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bf4b1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004bf4b2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4bf4c0: native, nfs3hp::vertices::objectVertices */

/* sub_4bf790: native, nfs3hp::render::clipByZ */

/* sub_4c0b20: native, nfs3hp::render::line */

/* sub_4c0f60: native, nfs3hp::graphics::dead_0x4c0f60 */

/* sub_4c11b0: native, nfs3hp::vertices::clipTriangle */

/* sub_4c12e0: native, nfs3hp::vertices::clipQuad */

/* align: skip 0x8b 0xc0 */
void Application::sub_4c1900(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c1900  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c1901  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c1902  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c1903  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c1905  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 004c1908  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004c190a  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 004c190c  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 004c190f  8a4014                 -mov al, byte ptr [eax + 0x14]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 004c1912  8845f8                 -mov byte ptr [ebp - 8], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.al;
    // 004c1915  8a4214                 -mov al, byte ptr [edx + 0x14]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(20) /* 0x14 */);
    // 004c1918  8845f9                 -mov byte ptr [ebp - 7], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-7) /* -0x7 */) = cpu.al;
    // 004c191b  8a4314                 -mov al, byte ptr [ebx + 0x14]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(20) /* 0x14 */);
    // 004c191e  8845fa                 -mov byte ptr [ebp - 6], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-6) /* -0x6 */) = cpu.al;
    // 004c1921  8a4114                 -mov al, byte ptr [ecx + 0x14]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(20) /* 0x14 */);
    // 004c1924  8845fb                 -mov byte ptr [ebp - 5], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-5) /* -0x5 */) = cpu.al;
    // 004c1927  8a45f8                 -mov al, byte ptr [ebp - 8]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c192a  2245f9                 -and al, byte ptr [ebp - 7]
    cpu.al &= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-7) /* -0x7 */)));
    // 004c192d  8a65fa                 -mov ah, byte ptr [ebp - 6]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 004c1930  20c4                   -and ah, al
    cpu.ah &= x86::reg8(x86::sreg8(cpu.al));
    // 004c1932  8a45fb                 -mov al, byte ptr [ebp - 5]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-5) /* -0x5 */);
    // 004c1935  20e0                   +and al, ah
    cpu.clear_co();
    cpu.set_szp((cpu.al &= x86::reg8(x86::sreg8(cpu.ah))));
    // 004c1937  0f8597000000           -jne 0x4c19d4
    if (!cpu.flags.zf)
    {
        goto L_0x004c19d4;
    }
    // 004c193d  8a45f8                 -mov al, byte ptr [ebp - 8]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c1940  0a45f9                 -or al, byte ptr [ebp - 7]
    cpu.al |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-7) /* -0x7 */)));
    // 004c1943  0a45fa                 -or al, byte ptr [ebp - 6]
    cpu.al |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-6) /* -0x6 */)));
    // 004c1946  8a5dfb                 -mov bl, byte ptr [ebp - 5]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-5) /* -0x5 */);
    // 004c1949  08c3                   +or bl, al
    cpu.clear_co();
    cpu.set_szp((cpu.bl |= x86::reg8(x86::sreg8(cpu.al))));
    // 004c194b  750f                   -jne 0x4c195c
    if (!cpu.flags.zf)
    {
        goto L_0x004c195c;
    }
    // 004c194d  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c194e  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c194f  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c1950  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c1951  ff154cf99e00           -call dword ptr [0x9ef94c]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(10418508) /* 0x9ef94c */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c1957  e978000000             -jmp 0x4c19d4
    goto L_0x004c19d4;
L_0x004c195c:
    // 004c195c  84e4                   +test ah, ah
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & cpu.ah));
    // 004c195e  7527                   -jne 0x4c1987
    if (!cpu.flags.zf)
    {
        goto L_0x004c1987;
    }
    // 004c1960  84c0                   +test al, al
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & cpu.al));
    // 004c1962  750b                   -jne 0x4c196f
    if (!cpu.flags.zf)
    {
        goto L_0x004c196f;
    }
    // 004c1964  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c1965  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c1966  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c1967  ff1574f99e00           -call dword ptr [0x9ef974]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(10418548) /* 0x9ef974 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c196d  eb18                   -jmp 0x4c1987
    goto L_0x004c1987;
L_0x004c196f:
    // 004c196f  897dec                 -mov dword ptr [ebp - 0x14], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.edi;
    // 004c1972  8955f0                 -mov dword ptr [ebp - 0x10], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.edx;
    // 004c1975  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004c1977  8d55f8                 -lea edx, [ebp - 8]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c197a  88c3                   -mov bl, al
    cpu.bl = cpu.al;
    // 004c197c  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004c197f  8975f4                 -mov dword ptr [ebp - 0xc], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.esi;
    // 004c1982  e849130000             -call 0x4c2cd0
    cpu.esp -= 4;
    nfs3hp::vertices::clipPolygon(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c1987:
    // 004c1987  8a45f8                 -mov al, byte ptr [ebp - 8]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c198a  8a4dfa                 -mov cl, byte ptr [ebp - 6]
    cpu.cl = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 004c198d  20c8                   -and al, cl
    cpu.al &= x86::reg8(x86::sreg8(cpu.cl));
    // 004c198f  8a6dfb                 -mov ch, byte ptr [ebp - 5]
    cpu.ch = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-5) /* -0x5 */);
    // 004c1992  20e8                   +and al, ch
    cpu.clear_co();
    cpu.set_szp((cpu.al &= x86::reg8(x86::sreg8(cpu.ch))));
    // 004c1994  753e                   -jne 0x4c19d4
    if (!cpu.flags.zf)
    {
        goto L_0x004c19d4;
    }
    // 004c1996  8a45f8                 -mov al, byte ptr [ebp - 8]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c1999  08c8                   -or al, cl
    cpu.al |= x86::reg8(x86::sreg8(cpu.cl));
    // 004c199b  88ec                   -mov ah, ch
    cpu.ah = cpu.ch;
    // 004c199d  08c4                   +or ah, al
    cpu.clear_co();
    cpu.set_szp((cpu.ah |= x86::reg8(x86::sreg8(cpu.al))));
    // 004c199f  7512                   -jne 0x4c19b3
    if (!cpu.flags.zf)
    {
        goto L_0x004c19b3;
    }
    // 004c19a1  8b5dfc                 -mov ebx, dword ptr [ebp - 4]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c19a4  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c19a5  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c19a6  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c19a7  ff1574f99e00           -call dword ptr [0x9ef974]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(10418548) /* 0x9ef974 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c19ad  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c19af  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c19b0  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c19b1  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c19b2  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c19b3:
    // 004c19b3  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c19b6  897dec                 -mov dword ptr [ebp - 0x14], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.edi;
    // 004c19b9  8975f0                 -mov dword ptr [ebp - 0x10], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.esi;
    // 004c19bc  884df9                 -mov byte ptr [ebp - 7], cl
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-7) /* -0x7 */) = cpu.cl;
    // 004c19bf  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004c19c1  8955f4                 -mov dword ptr [ebp - 0xc], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edx;
    // 004c19c4  88e3                   -mov bl, ah
    cpu.bl = cpu.ah;
    // 004c19c6  8d55f8                 -lea edx, [ebp - 8]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c19c9  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004c19cc  886dfa                 -mov byte ptr [ebp - 6], ch
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-6) /* -0x6 */) = cpu.ch;
    // 004c19cf  e8fc120000             -call 0x4c2cd0
    cpu.esp -= 4;
    nfs3hp::vertices::clipPolygon(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c19d4:
    // 004c19d4  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c19d6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c19d7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c19d8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c19d9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4c19e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c19e0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c19e1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c19e2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c19e3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c19e5  83ec14                 -sub esp, 0x14
    (cpu.esp) -= x86::reg32(x86::sreg32(20 /*0x14*/));
    // 004c19e8  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004c19ea  89df                   -mov edi, ebx
    cpu.edi = cpu.ebx;
    // 004c19ec  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 004c19ef  8a4014                 -mov al, byte ptr [eax + 0x14]
    cpu.al = app->getMemory<x86::reg8>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 004c19f2  8845f8                 -mov byte ptr [ebp - 8], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.al;
    // 004c19f5  8a4214                 -mov al, byte ptr [edx + 0x14]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(20) /* 0x14 */);
    // 004c19f8  8845f9                 -mov byte ptr [ebp - 7], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-7) /* -0x7 */) = cpu.al;
    // 004c19fb  8a4314                 -mov al, byte ptr [ebx + 0x14]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(20) /* 0x14 */);
    // 004c19fe  8845fa                 -mov byte ptr [ebp - 6], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-6) /* -0x6 */) = cpu.al;
    // 004c1a01  8a4114                 -mov al, byte ptr [ecx + 0x14]
    cpu.al = app->getMemory<x86::reg8>(cpu.ecx + x86::reg32(20) /* 0x14 */);
    // 004c1a04  8845fb                 -mov byte ptr [ebp - 5], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-5) /* -0x5 */) = cpu.al;
    // 004c1a07  8a45f8                 -mov al, byte ptr [ebp - 8]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c1a0a  8a65f9                 -mov ah, byte ptr [ebp - 7]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-7) /* -0x7 */);
    // 004c1a0d  8a5dfa                 -mov bl, byte ptr [ebp - 6]
    cpu.bl = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 004c1a10  20e0                   -and al, ah
    cpu.al &= x86::reg8(x86::sreg8(cpu.ah));
    // 004c1a12  20d8                   -and al, bl
    cpu.al &= x86::reg8(x86::sreg8(cpu.bl));
    // 004c1a14  8a65fb                 -mov ah, byte ptr [ebp - 5]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-5) /* -0x5 */);
    // 004c1a17  20c4                   +and ah, al
    cpu.clear_co();
    cpu.set_szp((cpu.ah &= x86::reg8(x86::sreg8(cpu.al))));
    // 004c1a19  0f857b000000           -jne 0x4c1a9a
    if (!cpu.flags.zf)
    {
        goto L_0x004c1a9a;
    }
    // 004c1a1f  84c0                   +test al, al
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.al & cpu.al));
    // 004c1a21  752d                   -jne 0x4c1a50
    if (!cpu.flags.zf)
    {
        goto L_0x004c1a50;
    }
    // 004c1a23  8a45f8                 -mov al, byte ptr [ebp - 8]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c1a26  0a45f9                 -or al, byte ptr [ebp - 7]
    cpu.al |= x86::reg8(x86::sreg8(app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-7) /* -0x7 */)));
    // 004c1a29  08d8                   +or al, bl
    cpu.clear_co();
    cpu.set_szp((cpu.al |= x86::reg8(x86::sreg8(cpu.bl))));
    // 004c1a2b  750b                   -jne 0x4c1a38
    if (!cpu.flags.zf)
    {
        goto L_0x004c1a38;
    }
    // 004c1a2d  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c1a2e  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c1a2f  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c1a30  ff1574f99e00           -call dword ptr [0x9ef974]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(10418548) /* 0x9ef974 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c1a36  eb18                   -jmp 0x4c1a50
    goto L_0x004c1a50;
L_0x004c1a38:
    // 004c1a38  8975ec                 -mov dword ptr [ebp - 0x14], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.esi;
    // 004c1a3b  8955f0                 -mov dword ptr [ebp - 0x10], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.edx;
    // 004c1a3e  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004c1a40  8d55f8                 -lea edx, [ebp - 8]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c1a43  88c3                   -mov bl, al
    cpu.bl = cpu.al;
    // 004c1a45  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004c1a48  897df4                 -mov dword ptr [ebp - 0xc], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edi;
    // 004c1a4b  e880120000             -call 0x4c2cd0
    cpu.esp -= 4;
    nfs3hp::vertices::clipPolygon(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c1a50:
    // 004c1a50  8a45f8                 -mov al, byte ptr [ebp - 8]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c1a53  8a6dfa                 -mov ch, byte ptr [ebp - 6]
    cpu.ch = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-6) /* -0x6 */);
    // 004c1a56  20e8                   -and al, ch
    cpu.al &= x86::reg8(x86::sreg8(cpu.ch));
    // 004c1a58  8a65fb                 -mov ah, byte ptr [ebp - 5]
    cpu.ah = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-5) /* -0x5 */);
    // 004c1a5b  20e0                   +and al, ah
    cpu.clear_co();
    cpu.set_szp((cpu.al &= x86::reg8(x86::sreg8(cpu.ah))));
    // 004c1a5d  753b                   -jne 0x4c1a9a
    if (!cpu.flags.zf)
    {
        goto L_0x004c1a9a;
    }
    // 004c1a5f  8a45f8                 -mov al, byte ptr [ebp - 8]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c1a62  08e8                   -or al, ch
    cpu.al |= x86::reg8(x86::sreg8(cpu.ch));
    // 004c1a64  08c4                   +or ah, al
    cpu.clear_co();
    cpu.set_szp((cpu.ah |= x86::reg8(x86::sreg8(cpu.al))));
    // 004c1a66  750e                   -jne 0x4c1a76
    if (!cpu.flags.zf)
    {
        goto L_0x004c1a76;
    }
    // 004c1a68  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c1a6b  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c1a6c  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c1a6d  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c1a6e  ff1574f99e00           -call dword ptr [0x9ef974]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(10418548) /* 0x9ef974 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c1a74  eb24                   -jmp 0x4c1a9a
    goto L_0x004c1a9a;
L_0x004c1a76:
    // 004c1a76  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c1a79  8a45fb                 -mov al, byte ptr [ebp - 5]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-5) /* -0x5 */);
    // 004c1a7c  8975ec                 -mov dword ptr [ebp - 0x14], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.esi;
    // 004c1a7f  897df0                 -mov dword ptr [ebp - 0x10], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.edi;
    // 004c1a82  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004c1a84  8955f4                 -mov dword ptr [ebp - 0xc], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.edx;
    // 004c1a87  8845fa                 -mov byte ptr [ebp - 6], al
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-6) /* -0x6 */) = cpu.al;
    // 004c1a8a  88e3                   -mov bl, ah
    cpu.bl = cpu.ah;
    // 004c1a8c  8d55f8                 -lea edx, [ebp - 8]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c1a8f  8d45ec                 -lea eax, [ebp - 0x14]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004c1a92  886df9                 -mov byte ptr [ebp - 7], ch
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-7) /* -0x7 */) = cpu.ch;
    // 004c1a95  e836120000             -call 0x4c2cd0
    cpu.esp -= 4;
    nfs3hp::vertices::clipPolygon(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c1a9a:
    // 004c1a9a  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c1a9c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c1a9d  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c1a9e  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c1a9f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4c1aa0: native, nfs3hp::vertices::clipTriangleByZ */

/* sub_4c1e20: native, nfs3hp::vertices::quadMesh */

/* sub_4c20e0: native, nfs3hp::render::quadByZ */

/* sub_4c2cd0: native, nfs3hp::vertices::clipPolygon */

/* sub_4c3ad0: native, nfs3hp::render::nearTriangle */

/* sub_4c3f90: native, nfs3hp::frontend::connectStartConnection */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4c4190(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c4190  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c4191  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c4192  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c4193  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c4194  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c4196  b914000000             -mov ecx, 0x14
    cpu.ecx = 20 /*0x14*/;
    // 004c419b  bf50367d00             -mov edi, 0x7d3650
    cpu.edi = 8205904 /*0x7d3650*/;
    // 004c41a0  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004c41a2  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c41a7  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c41a8  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004c41aa  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 004c41ad  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c41af  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 004c41b1  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 004c41b4  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 004c41b6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c41b7  668915a2227a00         -mov word ptr [0x7a22a2], dx
    app->getMemory<x86::reg16>(x86::reg32(8004258) /* 0x7a22a2 */) = cpu.dx;
    // 004c41be  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c41bf  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c41c0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c41c1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c41c2  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4c41d0: native, nfs3hp::frontend::connectListModems */

/* sub_4c4230: native, nfs3hp::frontend::connectSetUpLink */

/* sub_4c4310: native, nfs3hp::frontend::connectShakeHands */

/* sub_4c4480: native, nfs3hp::dead::stub_0x4c4480 */

/* sub_4c4490: native, nfs3hp::graphics::captureScreen */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4c45d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c45d0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c45d1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c45d2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c45d4  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c45d6  89158c367d00           -mov dword ptr [0x7d368c], edx
    app->getMemory<x86::reg32>(x86::reg32(8205964) /* 0x7d368c */) = cpu.edx;
    // 004c45dc  891590367d00           -mov dword ptr [0x7d3690], edx
    app->getMemory<x86::reg32>(x86::reg32(8205968) /* 0x7d3690 */) = cpu.edx;
    // 004c45e2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c45e3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c45e4  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_4c45f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c45f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c45f1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c45f2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c45f3  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c45f4  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c45f6  81ec80000000           -sub esp, 0x80
    (cpu.esp) -= x86::reg32(x86::sreg32(128 /*0x80*/));
    // 004c45fc  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c45fe  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004c4603  891594367d00           -mov dword ptr [0x7d3694], edx
    app->getMemory<x86::reg32>(x86::reg32(8205972) /* 0x7d3694 */) = cpu.edx;
    // 004c4609  890d98367d00           -mov dword ptr [0x7d3698], ecx
    app->getMemory<x86::reg32>(x86::reg32(8205976) /* 0x7d3698 */) = cpu.ecx;
    // 004c460f  89159c367d00           -mov dword ptr [0x7d369c], edx
    app->getMemory<x86::reg32>(x86::reg32(8205980) /* 0x7d369c */) = cpu.edx;
    // 004c4615  8915a0367d00           -mov dword ptr [0x7d36a0], edx
    app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */) = cpu.edx;
    // 004c461b  8915a8367d00           -mov dword ptr [0x7d36a8], edx
    app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */) = cpu.edx;
    // 004c4621  8915ac367d00           -mov dword ptr [0x7d36ac], edx
    app->getMemory<x86::reg32>(x86::reg32(8205996) /* 0x7d36ac */) = cpu.edx;
    // 004c4627  8915b0367d00           -mov dword ptr [0x7d36b0], edx
    app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */) = cpu.edx;
    // 004c462d  8915a4367d00           -mov dword ptr [0x7d36a4], edx
    app->getMemory<x86::reg32>(x86::reg32(8205988) /* 0x7d36a4 */) = cpu.edx;
    // 004c4633  891580367d00           -mov dword ptr [0x7d3680], edx
    app->getMemory<x86::reg32>(x86::reg32(8205952) /* 0x7d3680 */) = cpu.edx;
    // 004c4639  891584367d00           -mov dword ptr [0x7d3684], edx
    app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */) = cpu.edx;
    // 004c463f  e88cedfdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4644  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c464b  0f85db000000           -jne 0x4c472c
    if (!cpu.flags.zf)
    {
        goto L_0x004c472c;
    }
    // 004c4651  ba90000000             -mov edx, 0x90
    cpu.edx = 144 /*0x90*/;
    // 004c4656  b8ec015400             -mov eax, 0x5401ec
    cpu.eax = 5505516 /*0x5401ec*/;
    // 004c465b  b908000000             -mov ecx, 8
    cpu.ecx = 8 /*0x8*/;
    // 004c4660  e88b9bffff             -call 0x4be1f0
    cpu.esp -= 4;
    sub_4be1f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4665  ba90000000             -mov edx, 0x90
    cpu.edx = 144 /*0x90*/;
    // 004c466a  a390367d00             -mov dword ptr [0x7d3690], eax
    app->getMemory<x86::reg32>(x86::reg32(8205968) /* 0x7d3690 */) = cpu.eax;
    // 004c466f  b8f4015400             -mov eax, 0x5401f4
    cpu.eax = 5505524 /*0x5401f4*/;
    // 004c4674  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004c4676  e8759bffff             -call 0x4be1f0
    cpu.esp -= 4;
    sub_4be1f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c467b  ba70ce4200             -mov edx, 0x42ce70
    cpu.edx = 4378224 /*0x42ce70*/;
    // 004c4680  a38c367d00             -mov dword ptr [0x7d368c], eax
    app->getMemory<x86::reg32>(x86::reg32(8205964) /* 0x7d368c */) = cpu.eax;
    // 004c4685  a190367d00             -mov eax, dword ptr [0x7d3690]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205968) /* 0x7d3690 */);
    // 004c468a  e8c19bffff             -call 0x4be250
    cpu.esp -= 4;
    sub_4be250(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c468f  e83cedfdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4694  6834297a00             -push 0x7a2934
    app->getMemory<x86::reg32>(cpu.esp-4) = 8005940 /*0x7a2934*/;
    cpu.esp -= 4;
    // 004c4699  68fc015400             -push 0x5401fc
    app->getMemory<x86::reg32>(cpu.esp-4) = 5505532 /*0x5401fc*/;
    cpu.esp -= 4;
    // 004c469e  8d4580                 -lea eax, [ebp - 0x80]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-128) /* -0x80 */);
    // 004c46a1  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004c46a2  e8e9af0100             -call 0x4df690
    cpu.esp -= 4;
    nfs3hp::watcom::sprintf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46a7  83c40c                 -add esp, 0xc
    (cpu.esp) += x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004c46aa  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c46ac  8d4580                 -lea eax, [ebp - 0x80]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-128) /* -0x80 */);
    // 004c46af  e88c0bfbff             -call 0x475240
    cpu.esp -= 4;
    nfs3hp::port::view::glyphResource(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46b4  e817edfdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46b9  a1b4d36f00             -mov eax, dword ptr [0x6fd3b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328692) /* 0x6fd3b4 */);
    // 004c46be  e80d2bf7ff             -call 0x4371d0
    cpu.esp -= 4;
    nfs3hp::collide::seedHash(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46c3  e808edfdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46c8  e86381f6ff             -call 0x42c830
    cpu.esp -= 4;
    nfs3hp::raceload::addCarTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46cd  e8feecfdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46d2  e809cbfaff             -call 0x4711e0
    cpu.esp -= 4;
    nfs3hp::controls::startRaceForces(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46d7  e8f4ecfdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46dc  e87ffff3ff             -call 0x404660
    cpu.esp -= 4;
    nfs3hp::airace::nothingAtEndToo(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46e1  e87a17f4ff             -call 0x405e60
    cpu.esp -= 4;
    nfs3hp::aidrive::setDrivingConstants(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46e6  e8259df4ff             -call 0x40e410
    cpu.esp -= 4;
    nfs3hp::aiplan::setupTrackTuning(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46eb  e830cbf3ff             -call 0x401220
    cpu.esp -= 4;
    nfs3hp::airace::nothingAtStart(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46f0  e85b84f4ff             -call 0x40cb50
    cpu.esp -= 4;
    nfs3hp::aiplan::cacheTrafficSide(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46f5  e8567af4ff             -call 0x40c150
    cpu.esp -= 4;
    nfs3hp::pursuit::raceStartHookToo(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46fa  e821ccf3ff             -call 0x401320
    cpu.esp -= 4;
    nfs3hp::airace::radioOn(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c46ff  e8cc43f4ff             -call 0x408ad0
    cpu.esp -= 4;
    nfs3hp::pursuit::raceStartHook(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4704  e83705f4ff             -call 0x404c40
    cpu.esp -= 4;
    nfs3hp::airace::nothingAtStartToo(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4709  e8e25ff4ff             -call 0x40a6f0
    cpu.esp -= 4;
    nfs3hp::pursuit::namePhrases(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c470e  e8ddf7fcff             -call 0x493ef0
    cpu.esp -= 4;
    sub_493ef0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4713  e858560000             -call 0x4c9d70
    cpu.esp -= 4;
    sub_4c9d70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4718  e8e3950000             -call 0x4cdd00
    cpu.esp -= 4;
    sub_4cdd00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c471d  e89ee2f4ff             -call 0x4129c0
    cpu.esp -= 4;
    nfs3hp::port::raceSoundInit(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4722  e82972f8ff             -call 0x44b950
    cpu.esp -= 4;
    nfs3hp::frontend::racePauseBegin(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4727  e8e481ffff             -call 0x4bc910
    cpu.esp -= 4;
    sub_4bc910(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c472c:
    // 004c472c  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c472e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c472f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4730  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4731  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4732  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4c4740(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c4740  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c4741  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c4742  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c4743  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c4744  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c4745  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c4747  833d94367d0000         +cmp dword ptr [0x7d3694], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205972) /* 0x7d3694 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c474e  741c                   -je 0x4c476c
    if (cpu.flags.zf)
    {
        goto L_0x004c476c;
    }
    // 004c4750  bba4010000             -mov ebx, 0x1a4
    cpu.ebx = 420 /*0x1a4*/;
    // 004c4755  ba50bc6f00             -mov edx, 0x6fbc50
    cpu.edx = 7322704 /*0x6fbc50*/;
    // 004c475a  b828c66f00             -mov eax, 0x6fc628
    cpu.eax = 7325224 /*0x6fc628*/;
    // 004c475f  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004c4761  e88a5d0200             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4766  890d94367d00           -mov dword ptr [0x7d3694], ecx
    app->getMemory<x86::reg32>(x86::reg32(8205972) /* 0x7d3694 */) = cpu.ecx;
L_0x004c476c:
    // 004c476c  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004c476e  8b3da0d36f00           -mov edi, dword ptr [0x6fd3a0]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
    // 004c4774  891da0367d00           -mov dword ptr [0x7d36a0], ebx
    app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */) = cpu.ebx;
    // 004c477a  891da8367d00           -mov dword ptr [0x7d36a8], ebx
    app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */) = cpu.ebx;
    // 004c4780  891dac367d00           -mov dword ptr [0x7d36ac], ebx
    app->getMemory<x86::reg32>(x86::reg32(8205996) /* 0x7d36ac */) = cpu.ebx;
    // 004c4786  891db0367d00           -mov dword ptr [0x7d36b0], ebx
    app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */) = cpu.ebx;
    // 004c478c  891da4367d00           -mov dword ptr [0x7d36a4], ebx
    app->getMemory<x86::reg32>(x86::reg32(8205988) /* 0x7d36a4 */) = cpu.ebx;
    // 004c4792  891d80367d00           -mov dword ptr [0x7d3680], ebx
    app->getMemory<x86::reg32>(x86::reg32(8205952) /* 0x7d3680 */) = cpu.ebx;
    // 004c4798  891d84367d00           -mov dword ptr [0x7d3684], ebx
    app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */) = cpu.ebx;
    // 004c479e  891d9c367d00           -mov dword ptr [0x7d369c], ebx
    app->getMemory<x86::reg32>(x86::reg32(8205980) /* 0x7d369c */) = cpu.ebx;
    // 004c47a4  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004c47a6  7506                   -jne 0x4c47ae
    if (!cpu.flags.zf)
    {
        goto L_0x004c47ae;
    }
    // 004c47a8  891dacd36f00           -mov dword ptr [0x6fd3ac], ebx
    app->getMemory<x86::reg32>(x86::reg32(7328684) /* 0x6fd3ac */) = cpu.ebx;
L_0x004c47ae:
    // 004c47ae  a1b4d36f00             -mov eax, dword ptr [0x6fd3b4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7328692) /* 0x6fd3b4 */);
    // 004c47b3  e8182af7ff             -call 0x4371d0
    cpu.esp -= 4;
    nfs3hp::collide::seedHash(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47b8  e86334ffff             -call 0x4b7c20
    cpu.esp -= 4;
    sub_4b7c20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47bd  e86e7df6ff             -call 0x42c530
    cpu.esp -= 4;
    nfs3hp::raceload::restartRaceCars(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47c2  e82999f4ff             -call 0x40e0f0
    cpu.esp -= 4;
    nfs3hp::aiplan::cacheRoadWidth(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47c7  e864caf3ff             -call 0x401230
    cpu.esp -= 4;
    nfs3hp::airace::nothingAtLoad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47cc  e84f83f4ff             -call 0x40cb20
    cpu.esp -= 4;
    nfs3hp::aiplan::clearSensedCars(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47d1  e86a79f4ff             -call 0x40c140
    cpu.esp -= 4;
    nfs3hp::pursuit::raceEndHookToo(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47d6  e8e5caf3ff             -call 0x4012c0
    cpu.esp -= 4;
    nfs3hp::airace::setUpCops(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47db  e80043f4ff             -call 0x408ae0
    cpu.esp -= 4;
    nfs3hp::pursuit::raceEndHook(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47e0  e86b04f4ff             -call 0x404c50
    cpu.esp -= 4;
    nfs3hp::airace::catchUpSpeed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47e5  e866fef3ff             -call 0x404650
    cpu.esp -= 4;
    nfs3hp::airace::nothingAtEnd(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47ea  e881f1f3ff             -call 0x403970
    cpu.esp -= 4;
    nfs3hp::airace::initPursuits(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47ef  e8bc17f4ff             -call 0x405fb0
    cpu.esp -= 4;
    nfs3hp::aidrive::drivingNothing(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47f4  e857f6fcff             -call 0x493e50
    cpu.esp -= 4;
    sub_493e50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47f9  e8b2550000             -call 0x4c9db0
    cpu.esp -= 4;
    sub_4c9db0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c47fe  e8ad80ffff             -call 0x4bc8b0
    cpu.esp -= 4;
    sub_4bc8b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4803  e8f8940000             -call 0x4cdd00
    cpu.esp -= 4;
    sub_4cdd00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4808  e8f3c7faff             -call 0x471000
    cpu.esp -= 4;
    nfs3hp::controls::stopForcesOrActuatorsOn(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c480d  a1acc47900             -mov eax, dword ptr [0x79c4ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7980204) /* 0x79c4ac */);
    // 004c4812  a388367d00             -mov dword ptr [0x7d3688], eax
    app->getMemory<x86::reg32>(x86::reg32(8205960) /* 0x7d3688 */) = cpu.eax;
    // 004c4817  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4818  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4819  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c481a  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c481b  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c481c  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4c4820(WinApplication* app, x86::CPU& cpu)
{
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c4820  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c4821  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c4823  e8f8dffaff             -call 0x472820
    cpu.esp -= 4;
    nfs3hp::load::saveConfig(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4828  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4829  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4c4830(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c4830  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c4831  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c4832  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c4833  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c4834  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c4836  833d94367d0000         +cmp dword ptr [0x7d3694], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205972) /* 0x7d3694 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c483d  741c                   -je 0x4c485b
    if (cpu.flags.zf)
    {
        goto L_0x004c485b;
    }
    // 004c483f  bba4010000             -mov ebx, 0x1a4
    cpu.ebx = 420 /*0x1a4*/;
    // 004c4844  ba50bc6f00             -mov edx, 0x6fbc50
    cpu.edx = 7322704 /*0x6fbc50*/;
    // 004c4849  b828c66f00             -mov eax, 0x6fc628
    cpu.eax = 7325224 /*0x6fc628*/;
    // 004c484e  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004c4850  e89b5c0200             -call 0x4ea4f0
    cpu.esp -= 4;
    nfs3hp::ea::memmove(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4855  890d94367d00           -mov dword ptr [0x7d3694], ecx
    app->getMemory<x86::reg32>(x86::reg32(8205972) /* 0x7d3694 */) = cpu.ecx;
L_0x004c485b:
    // 004c485b  e83032fbff             -call 0x477a90
    cpu.esp -= 4;
    nfs3hp::load::saveGhost(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4860  e80bc8faff             -call 0x471070
    cpu.esp -= 4;
    nfs3hp::controls::stopForcesOrActuatorsOff(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4865  e8761c0000             -call 0x4c64e0
    cpu.esp -= 4;
    sub_4c64e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c486a  8b1d8c367d00           -mov ebx, dword ptr [0x7d368c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8205964) /* 0x7d368c */);
    // 004c4870  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004c4872  740e                   -je 0x4c4882
    if (cpu.flags.zf)
    {
        goto L_0x004c4882;
    }
    // 004c4874  ba80664200             -mov edx, 0x426680
    cpu.edx = 4351616 /*0x426680*/;
    // 004c4879  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004c487b  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004c487d  e86e9affff             -call 0x4be2f0
    cpu.esp -= 4;
    sub_4be2f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c4882:
    // 004c4882  e82982f6ff             -call 0x42cab0
    cpu.esp -= 4;
    nfs3hp::raceload::removeCarTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4887  e87434ffff             -call 0x4b7d00
    cpu.esp -= 4;
    sub_4b7d00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c488c  e8afa3f4ff             -call 0x40ec40
    cpu.esp -= 4;
    nfs3hp::aiplan::freeSpeedTable(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4891  e81af7fcff             -call 0x493fb0
    cpu.esp -= 4;
    sub_493fb0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4896  e8f5540000             -call 0x4c9d90
    cpu.esp -= 4;
    sub_4c9d90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c489b  e82029f7ff             -call 0x4371c0
    cpu.esp -= 4;
    nfs3hp::collide::messagesNothing(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c48a0  e86b12fbff             -call 0x475b10
    cpu.esp -= 4;
    sub_475b10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c48a5  a190367d00             -mov eax, dword ptr [0x7d3690]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205968) /* 0x7d3690 */);
    // 004c48aa  e89199ffff             -call 0x4be240
    cpu.esp -= 4;
    sub_4be240(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c48af  a18c367d00             -mov eax, dword ptr [0x7d368c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205964) /* 0x7d368c */);
    // 004c48b4  e88799ffff             -call 0x4be240
    cpu.esp -= 4;
    sub_4be240(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c48b9  e8e280ffff             -call 0x4bc9a0
    cpu.esp -= 4;
    sub_4bc9a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c48be  e85dffffff             -call 0x4c4820
    cpu.esp -= 4;
    sub_4c4820(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c48c3  e8f8d2f6ff             -call 0x431bc0
    cpu.esp -= 4;
    nfs3hp::collide::statesForScreens(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c48c8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c48c9  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c48ca  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c48cb  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c48cc  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 */
void Application::sub_4c48d0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c48d0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c48d1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c48d2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c48d3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c48d4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c48d5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c48d6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c48d8  833da0d36f0000         +cmp dword ptr [0x6fd3a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c48df  7514                   -jne 0x4c48f5
    if (!cpu.flags.zf)
    {
        goto L_0x004c48f5;
    }
    // 004c48e1  8b0d84367d00           -mov ecx, dword ptr [0x7d3684]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004c48e7  81f900020000           +cmp ecx, 0x200
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(512 /*0x200*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c48ed  7c06                   -jl 0x4c48f5
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c48f5;
    }
    // 004c48ef  890dacd36f00           -mov dword ptr [0x6fd3ac], ecx
    app->getMemory<x86::reg32>(x86::reg32(7328684) /* 0x6fd3ac */) = cpu.ecx;
L_0x004c48f5:
    // 004c48f5  8b1d80367d00           -mov ebx, dword ptr [0x7d3680]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8205952) /* 0x7d3680 */);
    // 004c48fb  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004c48fd  0f8562010000           -jne 0x4c4a65
    if (!cpu.flags.zf)
    {
        goto L_0x004c4a65;
    }
    // 004c4903  8b3584367d00           -mov esi, dword ptr [0x7d3684]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004c4909  81fe00020000           +cmp esi, 0x200
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(512 /*0x200*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c490f  7c5b                   -jl 0x4c496c
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c496c;
    }
    // 004c4911  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 004c4916  8a2558285500           -mov ah, byte ptr [0x552858]
    cpu.ah = app->getMemory<x86::reg8>(x86::reg32(5580888) /* 0x552858 */);
    // 004c491c  893d80367d00           -mov dword ptr [0x7d3680], edi
    app->getMemory<x86::reg32>(x86::reg32(8205952) /* 0x7d3680 */) = cpu.edi;
    // 004c4922  84e4                   +test ah, ah
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(cpu.ah & cpu.ah));
    // 004c4924  0f843b010000           -je 0x4c4a65
    if (cpu.flags.zf)
    {
        goto L_0x004c4a65;
    }
    // 004c492a  6a40                   -push 0x40
    app->getMemory<x86::reg32>(cpu.esp-4) = 64 /*0x40*/;
    cpu.esp -= 4;
    // 004c492c  b97f000000             -mov ecx, 0x7f
    cpu.ecx = 127 /*0x7f*/;
    // 004c4931  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
    // 004c4936  a16c115700             -mov eax, dword ptr [0x57116c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5706092) /* 0x57116c */);
    // 004c493b  e87039f5ff             -call 0x4182b0
    cpu.esp -= 4;
    nfs3hp::port::playEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4940  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 004c4945  30d2                   +xor dl, dl
    cpu.clear_co();
    cpu.set_szp((cpu.dl ^= x86::reg8(x86::sreg8(cpu.dl))));
    // 004c4947  bbffffffff             -mov ebx, 0xffffffff
    cpu.ebx = 4294967295 /*0xffffffff*/;
    // 004c494c  881558285500           -mov byte ptr [0x552858], dl
    app->getMemory<x86::reg8>(x86::reg32(5580888) /* 0x552858 */) = cpu.dl;
    // 004c4952  a388b05500             -mov dword ptr [0x55b088], eax
    app->getMemory<x86::reg32>(x86::reg32(5615752) /* 0x55b088 */) = cpu.eax;
    // 004c4957  b8407f00ff             -mov eax, 0xff007f40
    cpu.eax = 4278222656 /*0xff007f40*/;
    // 004c495c  8b15441b7900           -mov edx, dword ptr [0x791b44]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7936836) /* 0x791b44 */);
    // 004c4962  e80928fbff             -call 0x477170
    cpu.esp -= 4;
    nfs3hp::frontend::textLookSetDirectional(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4967  e9f9000000             -jmp 0x4c4a65
    goto L_0x004c4a65;
L_0x004c496c:
    // 004c496c  81fec0010000           +cmp esi, 0x1c0
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(448 /*0x1c0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4972  7c50                   -jl 0x4c49c4
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c49c4;
    }
    // 004c4974  803d5828550001         +cmp byte ptr [0x552858], 1
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5580888) /* 0x552858 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(1 /*0x1*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004c497b  0f84e4000000           -je 0x4c4a65
    if (cpu.flags.zf)
    {
        goto L_0x004c4a65;
    }
    // 004c4981  6a40                   -push 0x40
    app->getMemory<x86::reg32>(cpu.esp-4) = 64 /*0x40*/;
    cpu.esp -= 4;
    // 004c4983  b97f000000             -mov ecx, 0x7f
    cpu.ecx = 127 /*0x7f*/;
    // 004c4988  ba02000000             -mov edx, 2
    cpu.edx = 2 /*0x2*/;
    // 004c498d  a16c115700             -mov eax, dword ptr [0x57116c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5706092) /* 0x57116c */);
    // 004c4992  e81939f5ff             -call 0x4182b0
    cpu.esp -= 4;
    nfs3hp::port::playEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4997  b301                   -mov bl, 1
    cpu.bl = 1 /*0x1*/;
    // 004c4999  b903000000             -mov ecx, 3
    cpu.ecx = 3 /*0x3*/;
    // 004c499e  b800cfcfff             -mov eax, 0xffcfcf00
    cpu.eax = 4291809024 /*0xffcfcf00*/;
    // 004c49a3  8b15441b7900           -mov edx, dword ptr [0x791b44]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7936836) /* 0x791b44 */);
    // 004c49a9  881d58285500           -mov byte ptr [0x552858], bl
    app->getMemory<x86::reg8>(x86::reg32(5580888) /* 0x552858 */) = cpu.bl;
    // 004c49af  bbffffffff             -mov ebx, 0xffffffff
    cpu.ebx = 4294967295 /*0xffffffff*/;
    // 004c49b4  890d88b05500           -mov dword ptr [0x55b088], ecx
    app->getMemory<x86::reg32>(x86::reg32(5615752) /* 0x55b088 */) = cpu.ecx;
    // 004c49ba  e8b127fbff             -call 0x477170
    cpu.esp -= 4;
    nfs3hp::frontend::textLookSetDirectional(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c49bf  e9a1000000             -jmp 0x4c4a65
    goto L_0x004c4a65;
L_0x004c49c4:
    // 004c49c4  81fe80010000           +cmp esi, 0x180
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(384 /*0x180*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c49ca  7c4f                   -jl 0x4c4a1b
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c4a1b;
    }
    // 004c49cc  803d5828550002         +cmp byte ptr [0x552858], 2
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5580888) /* 0x552858 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(2 /*0x2*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004c49d3  0f848c000000           -je 0x4c4a65
    if (cpu.flags.zf)
    {
        goto L_0x004c4a65;
    }
    // 004c49d9  6a40                   -push 0x40
    app->getMemory<x86::reg32>(cpu.esp-4) = 64 /*0x40*/;
    cpu.esp -= 4;
    // 004c49db  b97f000000             -mov ecx, 0x7f
    cpu.ecx = 127 /*0x7f*/;
    // 004c49e0  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c49e5  a16c115700             -mov eax, dword ptr [0x57116c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5706092) /* 0x57116c */);
    // 004c49ea  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 004c49ec  be03000000             -mov esi, 3
    cpu.esi = 3 /*0x3*/;
    // 004c49f1  e8ba38f5ff             -call 0x4182b0
    cpu.esp -= 4;
    nfs3hp::port::playEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c49f6  b102                   -mov cl, 2
    cpu.cl = 2 /*0x2*/;
    // 004c49f8  bbffffffff             -mov ebx, 0xffffffff
    cpu.ebx = 4294967295 /*0xffffffff*/;
    // 004c49fd  b80077cfff             -mov eax, 0xffcf7700
    cpu.eax = 4291786496 /*0xffcf7700*/;
    // 004c4a02  8b15441b7900           -mov edx, dword ptr [0x791b44]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7936836) /* 0x791b44 */);
    // 004c4a08  893588b05500           -mov dword ptr [0x55b088], esi
    app->getMemory<x86::reg32>(x86::reg32(5615752) /* 0x55b088 */) = cpu.esi;
    // 004c4a0e  880d58285500           -mov byte ptr [0x552858], cl
    app->getMemory<x86::reg8>(x86::reg32(5580888) /* 0x552858 */) = cpu.cl;
    // 004c4a14  e85727fbff             -call 0x477170
    cpu.esp -= 4;
    nfs3hp::frontend::textLookSetDirectional(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4a19  eb4a                   -jmp 0x4c4a65
    goto L_0x004c4a65;
L_0x004c4a1b:
    // 004c4a1b  81fe40010000           +cmp esi, 0x140
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(320 /*0x140*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4a21  7c42                   -jl 0x4c4a65
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c4a65;
    }
    // 004c4a23  803d5828550003         +cmp byte ptr [0x552858], 3
    {
        x86::reg8 tmp1 = app->getMemory<x86::reg8>(x86::reg32(5580888) /* 0x552858 */);
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(3 /*0x3*/));
        x86::reg8 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) != (1 & (tmp2 >> 7));
        cpu.set_szp(result);
    }
    // 004c4a2a  7439                   -je 0x4c4a65
    if (cpu.flags.zf)
    {
        goto L_0x004c4a65;
    }
    // 004c4a2c  6a40                   -push 0x40
    app->getMemory<x86::reg32>(cpu.esp-4) = 64 /*0x40*/;
    cpu.esp -= 4;
    // 004c4a2e  b97f000000             -mov ecx, 0x7f
    cpu.ecx = 127 /*0x7f*/;
    // 004c4a33  a16c115700             -mov eax, dword ptr [0x57116c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5706092) /* 0x57116c */);
    // 004c4a38  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c4a3a  e87138f5ff             -call 0x4182b0
    cpu.esp -= 4;
    nfs3hp::port::playEffect(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4a3f  b003                   -mov al, 3
    cpu.al = 3 /*0x3*/;
    // 004c4a41  bbffffffff             -mov ebx, 0xffffffff
    cpu.ebx = 4294967295 /*0xffffffff*/;
    // 004c4a46  a258285500             -mov byte ptr [0x552858], al
    app->getMemory<x86::reg8>(x86::reg32(5580888) /* 0x552858 */) = cpu.al;
    // 004c4a4b  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 004c4a50  8b15441b7900           -mov edx, dword ptr [0x791b44]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7936836) /* 0x791b44 */);
    // 004c4a56  a388b05500             -mov dword ptr [0x55b088], eax
    app->getMemory<x86::reg32>(x86::reg32(5615752) /* 0x55b088 */) = cpu.eax;
    // 004c4a5b  b80000cfff             -mov eax, 0xffcf0000
    cpu.eax = 4291756032 /*0xffcf0000*/;
    // 004c4a60  e80b27fbff             -call 0x477170
    cpu.esp -= 4;
    nfs3hp::frontend::textLookSetDirectional(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c4a65:
    // 004c4a65  e866c2f6ff             -call 0x430cd0
    cpu.esp -= 4;
    nfs3hp::collide::clearCarList(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4a6a  f60584367d0001         +test byte ptr [0x7d3684], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8205956) /* 0x7d3684 */) & 1 /*0x1*/));
    // 004c4a71  751f                   -jne 0x4c4a92
    if (!cpu.flags.zf)
    {
        goto L_0x004c4a92;
    }
    // 004c4a73  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c4a75  e8b62b0200             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4a7a  a190367d00             -mov eax, dword ptr [0x7d3690]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205968) /* 0x7d3690 */);
    // 004c4a7f  e81c99ffff             -call 0x4be3a0
    cpu.esp -= 4;
    sub_4be3a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4a84  833da0367d0000         +cmp dword ptr [0x7d36a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4a8b  7505                   -jne 0x4c4a92
    if (!cpu.flags.zf)
    {
        goto L_0x004c4a92;
    }
    // 004c4a8d  e84ea00000             -call 0x4ceae0
    cpu.esp -= 4;
    sub_4ceae0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c4a92:
    // 004c4a92  a18c367d00             -mov eax, dword ptr [0x7d368c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205964) /* 0x7d368c */);
    // 004c4a97  e80499ffff             -call 0x4be3a0
    cpu.esp -= 4;
    sub_4be3a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4a9c  ff0584367d00           -inc dword ptr [0x7d3684]
    (app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */))++;
    // 004c4aa2  e859930000             -call 0x4cde00
    cpu.esp -= 4;
    sub_4cde00(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4aa7  f60584367d0001         +test byte ptr [0x7d3684], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8205956) /* 0x7d3684 */) & 1 /*0x1*/));
    // 004c4aae  7505                   -jne 0x4c4ab5
    if (!cpu.flags.zf)
    {
        goto L_0x004c4ab5;
    }
    // 004c4ab0  e85bc9f4ff             -call 0x411410
    cpu.esp -= 4;
    nfs3hp::port::raceSoundTick(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c4ab5:
    // 004c4ab5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4ab6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4ab7  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4ab8  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4ab9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4aba  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4abb  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4c4ac0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c4ac0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c4ac1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c4ac2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c4ac3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c4ac5  833dfc00560000         +cmp dword ptr [0x5600fc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5636348) /* 0x5600fc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4acc  7441                   -je 0x4c4b0f
    if (cpu.flags.zf)
    {
        goto L_0x004c4b0f;
    }
    // 004c4ace  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c4ad3  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004c4ad5  e8d6acfcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4ada  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c4adc  7513                   -jne 0x4c4af1
    if (!cpu.flags.zf)
    {
        goto L_0x004c4af1;
    }
    // 004c4ade  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c4ae3  b83b000000             -mov eax, 0x3b
    cpu.eax = 59 /*0x3b*/;
    // 004c4ae8  e8e3adfcff             -call 0x48f8d0
    cpu.esp -= 4;
    nfs3hp::controls::keyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4aed  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c4aef  741e                   -je 0x4c4b0f
    if (cpu.flags.zf)
    {
        goto L_0x004c4b0f;
    }
L_0x004c4af1:
    // 004c4af1  b81b000000             -mov eax, 0x1b
    cpu.eax = 27 /*0x1b*/;
    // 004c4af6  e845cef8ff             -call 0x451940
    cpu.esp -= 4;
    nfs3hp::frontend::connectFlushKeys(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4afb  833dfc00560000         +cmp dword ptr [0x5600fc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5636348) /* 0x5600fc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4b02  7507                   -jne 0x4c4b0b
    if (!cpu.flags.zf)
    {
        goto L_0x004c4b0b;
    }
    // 004c4b04  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c4b09  eb32                   -jmp 0x4c4b3d
    goto L_0x004c4b3d;
L_0x004c4b0b:
    // 004c4b0b  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004c4b0d  eb2e                   -jmp 0x4c4b3d
    goto L_0x004c4b3d;
L_0x004c4b0f:
    // 004c4b0f  833dfc00560000         +cmp dword ptr [0x5600fc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5636348) /* 0x5600fc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4b16  752a                   -jne 0x4c4b42
    if (!cpu.flags.zf)
    {
        goto L_0x004c4b42;
    }
    // 004c4b18  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c4b1d  b83b000000             -mov eax, 0x3b
    cpu.eax = 59 /*0x3b*/;
    // 004c4b22  e8a9adfcff             -call 0x48f8d0
    cpu.esp -= 4;
    nfs3hp::controls::keyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4b27  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c4b29  7417                   -je 0x4c4b42
    if (cpu.flags.zf)
    {
        goto L_0x004c4b42;
    }
    // 004c4b2b  833dfc00560000         +cmp dword ptr [0x5600fc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5636348) /* 0x5600fc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4b32  7507                   -jne 0x4c4b3b
    if (!cpu.flags.zf)
    {
        goto L_0x004c4b3b;
    }
    // 004c4b34  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c4b39  eb02                   -jmp 0x4c4b3d
    goto L_0x004c4b3d;
L_0x004c4b3b:
    // 004c4b3b  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004c4b3d:
    // 004c4b3d  a3fc005600             -mov dword ptr [0x5600fc], eax
    app->getMemory<x86::reg32>(x86::reg32(5636348) /* 0x5600fc */) = cpu.eax;
L_0x004c4b42:
    // 004c4b42  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c4b47  b83d000000             -mov eax, 0x3d
    cpu.eax = 61 /*0x3d*/;
    // 004c4b4c  e87fadfcff             -call 0x48f8d0
    cpu.esp -= 4;
    nfs3hp::controls::keyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4b51  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c4b53  7431                   -je 0x4c4b86
    if (cpu.flags.zf)
    {
        goto L_0x004c4b86;
    }
    // 004c4b55  833d1c30550000         +cmp dword ptr [0x55301c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5582876) /* 0x55301c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4b5c  7507                   -jne 0x4c4b65
    if (!cpu.flags.zf)
    {
        goto L_0x004c4b65;
    }
    // 004c4b5e  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c4b63  eb02                   -jmp 0x4c4b67
    goto L_0x004c4b67;
L_0x004c4b65:
    // 004c4b65  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004c4b67:
    // 004c4b67  a31c305500             -mov dword ptr [0x55301c], eax
    app->getMemory<x86::reg32>(x86::reg32(5582876) /* 0x55301c */) = cpu.eax;
    // 004c4b6c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c4b6e  740c                   -je 0x4c4b7c
    if (cpu.flags.zf)
    {
        goto L_0x004c4b7c;
    }
    // 004c4b70  a120305500             -mov eax, dword ptr [0x553020]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5582880) /* 0x553020 */);
    // 004c4b75  a370367d00             -mov dword ptr [0x7d3670], eax
    app->getMemory<x86::reg32>(x86::reg32(8205936) /* 0x7d3670 */) = cpu.eax;
    // 004c4b7a  eb0a                   -jmp 0x4c4b86
    goto L_0x004c4b86;
L_0x004c4b7c:
    // 004c4b7c  a170367d00             -mov eax, dword ptr [0x7d3670]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205936) /* 0x7d3670 */);
    // 004c4b81  a320305500             -mov dword ptr [0x553020], eax
    app->getMemory<x86::reg32>(x86::reg32(5582880) /* 0x553020 */) = cpu.eax;
L_0x004c4b86:
    // 004c4b86  b83c000000             -mov eax, 0x3c
    cpu.eax = 60 /*0x3c*/;
    // 004c4b8b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c4b8d  e83eadfcff             -call 0x48f8d0
    cpu.esp -= 4;
    nfs3hp::controls::keyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4b92  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c4b94  740c                   -je 0x4c4ba2
    if (cpu.flags.zf)
    {
        goto L_0x004c4ba2;
    }
    // 004c4b96  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 004c4b9b  a370367d00             -mov dword ptr [0x7d3670], eax
    app->getMemory<x86::reg32>(x86::reg32(8205936) /* 0x7d3670 */) = cpu.eax;
    // 004c4ba0  eb2e                   -jmp 0x4c4bd0
    goto L_0x004c4bd0;
L_0x004c4ba2:
    // 004c4ba2  b83e000000             -mov eax, 0x3e
    cpu.eax = 62 /*0x3e*/;
    // 004c4ba7  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c4ba9  e822adfcff             -call 0x48f8d0
    cpu.esp -= 4;
    nfs3hp::controls::keyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4bae  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c4bb0  7415                   -je 0x4c4bc7
    if (cpu.flags.zf)
    {
        goto L_0x004c4bc7;
    }
    // 004c4bb2  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004c4bb7  890d70367d00           -mov dword ptr [0x7d3670], ecx
    app->getMemory<x86::reg32>(x86::reg32(8205936) /* 0x7d3670 */) = cpu.ecx;
    // 004c4bbd  890d20305500           -mov dword ptr [0x553020], ecx
    app->getMemory<x86::reg32>(x86::reg32(5582880) /* 0x553020 */) = cpu.ecx;
    // 004c4bc3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4bc4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4bc5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4bc6  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c4bc7:
    // 004c4bc7  833d1c30550000         +cmp dword ptr [0x55301c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5582876) /* 0x55301c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4bce  7405                   -je 0x4c4bd5
    if (cpu.flags.zf)
    {
        goto L_0x004c4bd5;
    }
L_0x004c4bd0:
    // 004c4bd0  a320305500             -mov dword ptr [0x553020], eax
    app->getMemory<x86::reg32>(x86::reg32(5582880) /* 0x553020 */) = cpu.eax;
L_0x004c4bd5:
    // 004c4bd5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4bd6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4bd7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4bd8  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4c4be0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c4be0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c4be1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c4be2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c4be3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c4be4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c4be5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c4be7  8b15fcfd5e00           -mov edx, dword ptr [0x5efdfc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6225404) /* 0x5efdfc */);
    // 004c4bed  3b15a4367d00           +cmp edx, dword ptr [0x7d36a4]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8205988) /* 0x7d36a4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4bf3  0f8c76010000           -jl 0x4c4d6f
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c4d6f;
    }
    // 004c4bf9  8b0dfc005600           -mov ecx, dword ptr [0x5600fc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5636348) /* 0x5600fc */);
    // 004c4bff  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004c4c01  0f8568010000           -jne 0x4c4d6f
    if (!cpu.flags.zf)
    {
        goto L_0x004c4d6f;
    }
    // 004c4c07  e8b46ef8ff             -call 0x44bac0
    cpu.esp -= 4;
    nfs3hp::frontend::racePauseFrame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4c0c  e80f0b0000             -call 0x4c5720
    cpu.esp -= 4;
    sub_4c5720(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4c11  83f801                 +cmp eax, 1
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
    // 004c4c14  754e                   -jne 0x4c4c64
    if (!cpu.flags.zf)
    {
        goto L_0x004c4c64;
    }
    // 004c4c16  e8f5100100             -call 0x4d5d10
    cpu.esp -= 4;
    sub_4d5d10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4c1b  833da8367d0000         +cmp dword ptr [0x7d36a8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4c22  7505                   -jne 0x4c4c29
    if (!cpu.flags.zf)
    {
        goto L_0x004c4c29;
    }
    // 004c4c24  e8f739f5ff             -call 0x418620
    cpu.esp -= 4;
    nfs3hp::port::masterBack(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c4c29:
    // 004c4c29  31f6                   +xor esi, esi
    cpu.clear_co();
    cpu.set_szp((cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi))));
    // 004c4c2b  893508015600           -mov dword ptr [0x560108], esi
    app->getMemory<x86::reg32>(x86::reg32(5636360) /* 0x560108 */) = cpu.esi;
    // 004c4c31  8935a0367d00           -mov dword ptr [0x7d36a0], esi
    app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */) = cpu.esi;
    // 004c4c37  e844c3faff             -call 0x470f80
    cpu.esp -= 4;
    nfs3hp::controls::stopForcesAgain(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4c3c  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c4c41  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c4c46  e8d5abfcff             -call 0x48f820
    cpu.esp -= 4;
    nfs3hp::controls::pressFunctionKey(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4c4b  b81c000000             -mov eax, 0x1c
    cpu.eax = 28 /*0x1c*/;
    // 004c4c50  e87bacfcff             -call 0x48f8d0
    cpu.esp -= 4;
    nfs3hp::controls::keyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4c55  b81b000000             -mov eax, 0x1b
    cpu.eax = 27 /*0x1b*/;
    // 004c4c5a  e8e1ccf8ff             -call 0x451940
    cpu.esp -= 4;
    nfs3hp::frontend::connectFlushKeys(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4c5f  e90b010000             -jmp 0x4c4d6f
    goto L_0x004c4d6f;
L_0x004c4c64:
    // 004c4c64  83f802                 +cmp eax, 2
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
    // 004c4c67  7536                   -jne 0x4c4c9f
    if (!cpu.flags.zf)
    {
        goto L_0x004c4c9f;
    }
    // 004c4c69  e8a2100100             -call 0x4d5d10
    cpu.esp -= 4;
    sub_4d5d10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4c6e  e87d3af5ff             -call 0x4186f0
    cpu.esp -= 4;
    nfs3hp::port::silenceForRace(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4c73  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c4c78  890da0367d00           -mov dword ptr [0x7d36a0], ecx
    app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */) = cpu.ecx;
    // 004c4c7e  89159c367d00           -mov dword ptr [0x7d369c], edx
    app->getMemory<x86::reg32>(x86::reg32(8205980) /* 0x7d369c */) = cpu.edx;
    // 004c4c84  891598367d00           -mov dword ptr [0x7d3698], edx
    app->getMemory<x86::reg32>(x86::reg32(8205976) /* 0x7d3698 */) = cpu.edx;
    // 004c4c8a  e831c3faff             -call 0x470fc0
    cpu.esp -= 4;
    nfs3hp::controls::stopForcesOnceMore(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4c8f  b81c000000             -mov eax, 0x1c
    cpu.eax = 28 /*0x1c*/;
    // 004c4c94  e837acfcff             -call 0x48f8d0
    cpu.esp -= 4;
    nfs3hp::controls::keyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4c99  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4c9a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4c9b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4c9c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4c9d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4c9e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c4c9f:
    // 004c4c9f  83f804                 +cmp eax, 4
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
    // 004c4ca2  754a                   -jne 0x4c4cee
    if (!cpu.flags.zf)
    {
        goto L_0x004c4cee;
    }
    // 004c4ca4  e867100100             -call 0x4d5d10
    cpu.esp -= 4;
    sub_4d5d10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4ca9  e8f239f5ff             -call 0x4186a0
    cpu.esp -= 4;
    nfs3hp::port::silenceForMenus(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4cae  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004c4cb3  890da0367d00           -mov dword ptr [0x7d36a0], ecx
    app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */) = cpu.ecx;
    // 004c4cb9  890d98367d00           -mov dword ptr [0x7d3698], ecx
    app->getMemory<x86::reg32>(x86::reg32(8205976) /* 0x7d3698 */) = cpu.ecx;
    // 004c4cbf  89359c367d00           -mov dword ptr [0x7d369c], esi
    app->getMemory<x86::reg32>(x86::reg32(8205980) /* 0x7d369c */) = cpu.esi;
    // 004c4cc5  e8f6c2faff             -call 0x470fc0
    cpu.esp -= 4;
    nfs3hp::controls::stopForcesOnceMore(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4cca  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004c4ccc  e87f9a0000             -call 0x4ce750
    cpu.esp -= 4;
    sub_4ce750(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4cd1  833da0d36f0000         +cmp dword ptr [0x6fd3a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4cd8  0f8591000000           -jne 0x4c4d6f
    if (!cpu.flags.zf)
    {
        goto L_0x004c4d6f;
    }
    // 004c4cde  c705b4367d0002000000   -mov dword ptr [0x7d36b4], 2
    app->getMemory<x86::reg32>(x86::reg32(8206004) /* 0x7d36b4 */) = 2 /*0x2*/;
    // 004c4ce8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4ce9  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4cea  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4ceb  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4cec  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4ced  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c4cee:
    // 004c4cee  83f803                 +cmp eax, 3
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
    // 004c4cf1  7542                   -jne 0x4c4d35
    if (!cpu.flags.zf)
    {
        goto L_0x004c4d35;
    }
    // 004c4cf3  e818100100             -call 0x4d5d10
    cpu.esp -= 4;
    sub_4d5d10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4cf8  e8a339f5ff             -call 0x4186a0
    cpu.esp -= 4;
    nfs3hp::port::silenceForMenus(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4cfd  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004c4d02  890da0367d00           -mov dword ptr [0x7d36a0], ecx
    app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */) = cpu.ecx;
    // 004c4d08  890d98367d00           -mov dword ptr [0x7d3698], ecx
    app->getMemory<x86::reg32>(x86::reg32(8205976) /* 0x7d3698 */) = cpu.ecx;
    // 004c4d0e  891d9c367d00           -mov dword ptr [0x7d369c], ebx
    app->getMemory<x86::reg32>(x86::reg32(8205980) /* 0x7d369c */) = cpu.ebx;
    // 004c4d14  e8a7c2faff             -call 0x470fc0
    cpu.esp -= 4;
    nfs3hp::controls::stopForcesOnceMore(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4d19  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004c4d1b  e8309a0000             -call 0x4ce750
    cpu.esp -= 4;
    sub_4ce750(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4d20  833da0d36f0000         +cmp dword ptr [0x6fd3a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4d27  7546                   -jne 0x4c4d6f
    if (!cpu.flags.zf)
    {
        goto L_0x004c4d6f;
    }
    // 004c4d29  890db4367d00           -mov dword ptr [0x7d36b4], ecx
    app->getMemory<x86::reg32>(x86::reg32(8206004) /* 0x7d36b4 */) = cpu.ecx;
    // 004c4d2f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4d30  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4d31  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4d32  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4d33  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4d34  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c4d35:
    // 004c4d35  83f805                 +cmp eax, 5
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5 /*0x5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4d38  7535                   -jne 0x4c4d6f
    if (!cpu.flags.zf)
    {
        goto L_0x004c4d6f;
    }
    // 004c4d3a  e8d10f0100             -call 0x4d5d10
    cpu.esp -= 4;
    sub_4d5d10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4d3f  e85c39f5ff             -call 0x4186a0
    cpu.esp -= 4;
    nfs3hp::port::silenceForMenus(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4d44  890da0367d00           -mov dword ptr [0x7d36a0], ecx
    app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */) = cpu.ecx;
    // 004c4d4a  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004c4d4c  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004c4d51  891d98367d00           -mov dword ptr [0x7d3698], ebx
    app->getMemory<x86::reg32>(x86::reg32(8205976) /* 0x7d3698 */) = cpu.ebx;
    // 004c4d57  890d9c367d00           -mov dword ptr [0x7d369c], ecx
    app->getMemory<x86::reg32>(x86::reg32(8205980) /* 0x7d369c */) = cpu.ecx;
    // 004c4d5d  e85ec2faff             -call 0x470fc0
    cpu.esp -= 4;
    nfs3hp::controls::stopForcesOnceMore(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4d62  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004c4d64  e8e7990000             -call 0x4ce750
    cpu.esp -= 4;
    sub_4ce750(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4d69  890db4367d00           -mov dword ptr [0x7d36b4], ecx
    app->getMemory<x86::reg32>(x86::reg32(8206004) /* 0x7d36b4 */) = cpu.ecx;
L_0x004c4d6f:
    // 004c4d6f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4d70  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4d71  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4d72  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4d73  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4d74  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_4c4d80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c4d80  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c4d81  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c4d82  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c4d83  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c4d85  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c4d87  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c4d89  e8320f0100             -call 0x4d5cc0
    cpu.esp -= 4;
    nfs3hp::graphics::beginDrawing(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4d8e  833db0d36f0002         +cmp dword ptr [0x6fd3b0], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4d95  7c0a                   -jl 0x4c4da1
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c4da1;
    }
    // 004c4d97  a1fcfd5e00             -mov eax, dword ptr [0x5efdfc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225404) /* 0x5efdfc */);
    // 004c4d9c  83c020                 +add eax, 0x20
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(32 /*0x20*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004c4d9f  eb08                   -jmp 0x4c4da9
    goto L_0x004c4da9;
L_0x004c4da1:
    // 004c4da1  a1fcfd5e00             -mov eax, dword ptr [0x5efdfc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225404) /* 0x5efdfc */);
    // 004c4da6  83c010                 -add eax, 0x10
    (cpu.eax) += x86::reg32(x86::sreg32(16 /*0x10*/));
L_0x004c4da9:
    // 004c4da9  a3a4367d00             -mov dword ptr [0x7d36a4], eax
    app->getMemory<x86::reg32>(x86::reg32(8205988) /* 0x7d36a4 */) = cpu.eax;
    // 004c4dae  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004c4db3  e85838f5ff             -call 0x418610
    cpu.esp -= 4;
    nfs3hp::port::masterSilent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4db8  890d08015600           -mov dword ptr [0x560108], ecx
    app->getMemory<x86::reg32>(x86::reg32(5636360) /* 0x560108 */) = cpu.ecx;
    // 004c4dbe  890da0367d00           -mov dword ptr [0x7d36a0], ecx
    app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */) = cpu.ecx;
    // 004c4dc4  e877c1faff             -call 0x470f40
    cpu.esp -= 4;
    nfs3hp::controls::stopForces(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4dc9  b81b000000             -mov eax, 0x1b
    cpu.eax = 27 /*0x1b*/;
    // 004c4dce  e86dcbf8ff             -call 0x451940
    cpu.esp -= 4;
    nfs3hp::frontend::connectFlushKeys(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4dd3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4dd4  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4dd5  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4dd6  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4c4de0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c4de0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c4de1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c4de2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c4de3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c4de5  833d98367d0000         +cmp dword ptr [0x7d3698], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205976) /* 0x7d3698 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4dec  751c                   -jne 0x4c4e0a
    if (!cpu.flags.zf)
    {
        goto L_0x004c4e0a;
    }
    // 004c4dee  8b0d84367d00           -mov ecx, dword ptr [0x7d3684]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004c4df4  83f920                 +cmp ecx, 0x20
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(32 /*0x20*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4df7  7c11                   -jl 0x4c4e0a
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c4e0a;
    }
    // 004c4df9  81f940010000           +cmp ecx, 0x140
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(320 /*0x140*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4dff  7d0f                   -jge 0x4c4e10
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c4e10;
    }
    // 004c4e01  833da0d36f0001         +cmp dword ptr [0x6fd3a0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4e08  7f06                   -jg 0x4c4e10
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004c4e10;
    }
L_0x004c4e0a:
    // 004c4e0a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c4e0c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4e0d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4e0e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4e0f  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c4e10:
    // 004c4e10  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c4e12  740c                   -je 0x4c4e20
    if (cpu.flags.zf)
    {
        goto L_0x004c4e20;
    }
    // 004c4e14  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c4e19  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004c4e1b  e890a9fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c4e20:
    // 004c4e20  e87b070000             -call 0x4c55a0
    cpu.esp -= 4;
    sub_4c55a0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4e25  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c4e27  7504                   -jne 0x4c4e2d
    if (!cpu.flags.zf)
    {
        goto L_0x004c4e2d;
    }
    // 004c4e29  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4e2a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4e2b  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4e2c  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c4e2d:
    // 004c4e2d  e84effffff             -call 0x4c4d80
    cpu.esp -= 4;
    sub_4c4d80(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4e32  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c4e37  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4e38  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4e39  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c4e3a  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4c4e40(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c4e40  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c4e41  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c4e42  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c4e43  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c4e44  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c4e45  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c4e46  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c4e48  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004c4e4b  a1f8fd5e00             -mov eax, dword ptr [0x5efdf8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225400) /* 0x5efdf8 */);
    // 004c4e50  0510270000             -add eax, 0x2710
    (cpu.eax) += x86::reg32(x86::sreg32(10000 /*0x2710*/));
    // 004c4e55  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c4e57  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004c4e5a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c4e5c  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 004c4e5e  e8cd270200             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4e63  8b0df8fd5e00           -mov ecx, dword ptr [0x5efdf8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(6225400) /* 0x5efdf8 */);
    // 004c4e69  8955f8                 -mov dword ptr [ebp - 8], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.edx;
    // 004c4e6c  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004c4e6e  7f05                   -jg 0x4c4e75
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004c4e75;
    }
    // 004c4e70  e808dc0200             -call 0x4f2a7d
    cpu.esp -= 4;
    sub_4f2a7d(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c4e75:
    // 004c4e75  a1acc47900             -mov eax, dword ptr [0x79c4ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7980204) /* 0x79c4ac */);
    // 004c4e7a  a388367d00             -mov dword ptr [0x7d3688], eax
    app->getMemory<x86::reg32>(x86::reg32(8205960) /* 0x7d3688 */) = cpu.eax;
    // 004c4e7f  e80c5f0200             -call 0x4ead90
    cpu.esp -= 4;
    sub_4ead90(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c4e84:
    app->unlockContext(cpu);
    win32::Thread::sleep(0);
    app->lockContext(cpu);
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c4e84  833d9c367d0000         +cmp dword ptr [0x7d369c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205980) /* 0x7d369c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4e8b  0f855f060000           -jne 0x4c54f0
    if (!cpu.flags.zf)
    {
        goto L_0x004c54f0;
    }
    // 004c4e91  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c4e93  e898270200             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4e98  a104015600             -mov eax, dword ptr [0x560104]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5636356) /* 0x560104 */);
    // 004c4e9d  01c0                   -add eax, eax
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.eax));
    // 004c4e9f  a3ac367d00             -mov dword ptr [0x7d36ac], eax
    app->getMemory<x86::reg32>(x86::reg32(8205996) /* 0x7d36ac */) = cpu.eax;
    // 004c4ea4  a1f8fd5e00             -mov eax, dword ptr [0x5efdf8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225400) /* 0x5efdf8 */);
    // 004c4ea9  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c4eac  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004c4eae  8b4df8                 -mov ecx, dword ptr [ebp - 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c4eb1  29d6                   -sub esi, edx
    (cpu.esi) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c4eb3  8b15ac367d00           -mov edx, dword ptr [0x7d36ac]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205996) /* 0x7d36ac */);
    // 004c4eb9  29ca                   -sub edx, ecx
    (cpu.edx) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004c4ebb  39d6                   +cmp esi, edx
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
    // 004c4ebd  7e06                   -jle 0x4c4ec5
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c4ec5;
    }
    // 004c4ebf  ff05ac367d00           -inc dword ptr [0x7d36ac]
    (app->getMemory<x86::reg32>(x86::reg32(8205996) /* 0x7d36ac */))++;
L_0x004c4ec5:
    // 004c4ec5  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004c4ec8  a1ac367d00             -mov eax, dword ptr [0x7d36ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205996) /* 0x7d36ac */);
    // 004c4ecd  8b1588367d00           -mov edx, dword ptr [0x7d3688]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205960) /* 0x7d3688 */);
    // 004c4ed3  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004c4ed6  a1acc47900             -mov eax, dword ptr [0x79c4ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7980204) /* 0x79c4ac */);
    // 004c4edb  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004c4edd  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c4edf  893574367d00           -mov dword ptr [0x7d3674], esi
    app->getMemory<x86::reg32>(x86::reg32(8205940) /* 0x7d3674 */) = cpu.esi;
    // 004c4ee5  83f810                 +cmp eax, 0x10
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(16 /*0x10*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4ee8  7e0a                   -jle 0x4c4ef4
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c4ef4;
    }
    // 004c4eea  a1acc47900             -mov eax, dword ptr [0x79c4ac]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7980204) /* 0x79c4ac */);
    // 004c4eef  a388367d00             -mov dword ptr [0x7d3688], eax
    app->getMemory<x86::reg32>(x86::reg32(8205960) /* 0x7d3688 */) = cpu.eax;
L_0x004c4ef4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c4ef4  8b0dacc47900           -mov ecx, dword ptr [0x79c4ac]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(7980204) /* 0x79c4ac */);
    // 004c4efa  3b0d88367d00           +cmp ecx, dword ptr [0x7d3688]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8205960) /* 0x7d3688 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4f00  0f8c7b030000           -jl 0x4c5281
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c5281;
    }
    // 004c4f06  833d9c367d0000         +cmp dword ptr [0x7d369c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205980) /* 0x7d369c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4f0d  0f856e030000           -jne 0x4c5281
    if (!cpu.flags.zf)
    {
        goto L_0x004c5281;
    }
    // 004c4f13  833db0d36f0002         +cmp dword ptr [0x6fd3b0], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4f1a  7d05                   -jge 0x4c4f21
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c4f21;
    }
    // 004c4f1c  e8ff78ffff             -call 0x4bc820
    cpu.esp -= 4;
    sub_4bc820(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c4f21:
    // 004c4f21  833d58da7c0002         +cmp dword ptr [0x7cda58], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8182360) /* 0x7cda58 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4f28  0f856d000000           -jne 0x4c4f9b
    if (!cpu.flags.zf)
    {
        goto L_0x004c4f9b;
    }
    // 004c4f2e  8b15a0367d00           -mov edx, dword ptr [0x7d36a0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */);
    // 004c4f34  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004c4f36  0f855f000000           -jne 0x4c4f9b
    if (!cpu.flags.zf)
    {
        goto L_0x004c4f9b;
    }
    // 004c4f3c  e8ff86ffff             -call 0x4bd640
    cpu.esp -= 4;
    nfs3hp::frontend::simPanelKeys(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4f41  833d5cd97c0004         +cmp dword ptr [0x7cd95c], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8182108) /* 0x7cd95c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4f48  7551                   -jne 0x4c4f9b
    if (!cpu.flags.zf)
    {
        goto L_0x004c4f9b;
    }
    // 004c4f4a  8b359c367d00           -mov esi, dword ptr [0x7d369c]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8205980) /* 0x7d369c */);
    // 004c4f50  89155cd97c00           -mov dword ptr [0x7cd95c], edx
    app->getMemory<x86::reg32>(x86::reg32(8182108) /* 0x7cd95c */) = cpu.edx;
    // 004c4f56  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004c4f58  7416                   -je 0x4c4f70
    if (cpu.flags.zf)
    {
        goto L_0x004c4f70;
    }
    // 004c4f5a  833da8367d0000         +cmp dword ptr [0x7d36a8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4f61  740d                   -je 0x4c4f70
    if (cpu.flags.zf)
    {
        goto L_0x004c4f70;
    }
    // 004c4f63  8915a8367d00           -mov dword ptr [0x7d36a8], edx
    app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */) = cpu.edx;
    // 004c4f69  e83237f5ff             -call 0x4186a0
    cpu.esp -= 4;
    nfs3hp::port::silenceForMenus(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4f6e  eb2b                   -jmp 0x4c4f9b
    goto L_0x004c4f9b;
L_0x004c4f70:
    // 004c4f70  8b0d48d97c00           -mov ecx, dword ptr [0x7cd948]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8182088) /* 0x7cd948 */);
    // 004c4f76  3b0da8367d00           +cmp ecx, dword ptr [0x7d36a8]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4f7c  741d                   -je 0x4c4f9b
    if (cpu.flags.zf)
    {
        goto L_0x004c4f9b;
    }
    // 004c4f7e  890da8367d00           -mov dword ptr [0x7d36a8], ecx
    app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */) = cpu.ecx;
    // 004c4f84  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004c4f86  740e                   -je 0x4c4f96
    if (cpu.flags.zf)
    {
        goto L_0x004c4f96;
    }
    // 004c4f88  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004c4f8a  e8f116f6ff             -call 0x426680
    cpu.esp -= 4;
    nfs3hp::frontend::simCamerasStep(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4f8f  e87c36f5ff             -call 0x418610
    cpu.esp -= 4;
    nfs3hp::port::masterSilent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4f94  eb05                   -jmp 0x4c4f9b
    goto L_0x004c4f9b;
L_0x004c4f96:
    // 004c4f96  e88536f5ff             -call 0x418620
    cpu.esp -= 4;
    nfs3hp::port::masterBack(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c4f9b:
    // 004c4f9b  833d9c367d0000         +cmp dword ptr [0x7d369c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205980) /* 0x7d369c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4fa2  751a                   -jne 0x4c4fbe
    if (!cpu.flags.zf)
    {
        goto L_0x004c4fbe;
    }
    // 004c4fa4  833da0367d0000         +cmp dword ptr [0x7d36a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4fab  7407                   -je 0x4c4fb4
    if (cpu.flags.zf)
    {
        goto L_0x004c4fb4;
    }
    // 004c4fad  e82efcffff             -call 0x4c4be0
    cpu.esp -= 4;
    sub_4c4be0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4fb2  eb0a                   -jmp 0x4c4fbe
    goto L_0x004c4fbe;
L_0x004c4fb4:
    // 004c4fb4  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c4fb9  e822feffff             -call 0x4c4de0
    cpu.esp -= 4;
    sub_4c4de0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c4fbe:
    // 004c4fbe  b807000000             -mov eax, 7
    cpu.eax = 7 /*0x7*/;
    // 004c4fc3  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c4fc5  e8e6a7fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4fca  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c4fcc  740a                   -je 0x4c4fd8
    if (cpu.flags.zf)
    {
        goto L_0x004c4fd8;
    }
    // 004c4fce  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c4fd3  e89899ffff             -call 0x4be970
    cpu.esp -= 4;
    sub_4be970(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c4fd8:
    // 004c4fd8  b808000000             -mov eax, 8
    cpu.eax = 8 /*0x8*/;
    // 004c4fdd  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c4fdf  e8cca7fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c4fe4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c4fe6  740a                   -je 0x4c4ff2
    if (cpu.flags.zf)
    {
        goto L_0x004c4ff2;
    }
    // 004c4fe8  b8ffffffff             -mov eax, 0xffffffff
    cpu.eax = 4294967295 /*0xffffffff*/;
    // 004c4fed  e87e99ffff             -call 0x4be970
    cpu.esp -= 4;
    sub_4be970(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c4ff2:
    // 004c4ff2  833da0367d0000         +cmp dword ptr [0x7d36a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c4ff9  0f8536020000           -jne 0x4c5235
    if (!cpu.flags.zf)
    {
        goto L_0x004c5235;
    }
    // 004c4fff  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c5004  b809000000             -mov eax, 9
    cpu.eax = 9 /*0x9*/;
    // 004c5009  e8a2a7fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c500e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c5010  7405                   -je 0x4c5017
    if (cpu.flags.zf)
    {
        goto L_0x004c5017;
    }
    // 004c5012  e85988f5ff             -call 0x41d870
    cpu.esp -= 4;
    nfs3hp::track::nextViewDistance(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c5017:
    // 004c5017  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c501c  b80c000000             -mov eax, 0xc
    cpu.eax = 12 /*0xc*/;
    // 004c5021  e88aa7fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5026  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c5028  7407                   -je 0x4c5031
    if (cpu.flags.zf)
    {
        goto L_0x004c5031;
    }
    // 004c502a  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c502c  e82ff2fbff             -call 0x484260
    cpu.esp -= 4;
    nfs3hp::frontend::hudToggleShown(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c5031:
    // 004c5031  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c5036  b80d000000             -mov eax, 0xd
    cpu.eax = 13 /*0xd*/;
    // 004c503b  e870a7fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5040  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c5042  740a                   -je 0x4c504e
    if (cpu.flags.zf)
    {
        goto L_0x004c504e;
    }
    // 004c5044  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c5049  e812f2fbff             -call 0x484260
    cpu.esp -= 4;
    nfs3hp::frontend::hudToggleShown(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c504e:
    // 004c504e  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c5053  b80e000000             -mov eax, 0xe
    cpu.eax = 14 /*0xe*/;
    // 004c5058  e853a7fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c505d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c505f  7405                   -je 0x4c5066
    if (cpu.flags.zf)
    {
        goto L_0x004c5066;
    }
    // 004c5061  e8ba98fcff             -call 0x48e920
    cpu.esp -= 4;
    nfs3hp::frontend::hudStepMapPanes(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c5066:
    // 004c5066  b80f000000             -mov eax, 0xf
    cpu.eax = 15 /*0xf*/;
    // 004c506b  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c506d  e83ea7fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5072  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c5074  7407                   -je 0x4c507d
    if (cpu.flags.zf)
    {
        goto L_0x004c507d;
    }
    // 004c5076  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c5078  e8e39afcff             -call 0x48eb60
    cpu.esp -= 4;
    nfs3hp::frontend::hudStepZoom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c507d:
    // 004c507d  b810000000             -mov eax, 0x10
    cpu.eax = 16 /*0x10*/;
    // 004c5082  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c5084  e827a7fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5089  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c508b  740a                   -je 0x4c5097
    if (cpu.flags.zf)
    {
        goto L_0x004c5097;
    }
    // 004c508d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c5092  e8c99afcff             -call 0x48eb60
    cpu.esp -= 4;
    nfs3hp::frontend::hudStepZoom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c5097:
    // 004c5097  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c509c  b80b000000             -mov eax, 0xb
    cpu.eax = 11 /*0xb*/;
    // 004c50a1  e80aa7fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c50a6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c50a8  7405                   -je 0x4c50af
    if (cpu.flags.zf)
    {
        goto L_0x004c50af;
    }
    // 004c50aa  e8a1f3fbff             -call 0x484450
    cpu.esp -= 4;
    nfs3hp::frontend::hudToggleRearView(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c50af:
    // 004c50af  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c50b4  b814000000             -mov eax, 0x14
    cpu.eax = 20 /*0x14*/;
    // 004c50b9  e8f2a6fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c50be  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c50c0  7405                   -je 0x4c50c7
    if (cpu.flags.zf)
    {
        goto L_0x004c50c7;
    }
    // 004c50c2  e81937f5ff             -call 0x4187e0
    cpu.esp -= 4;
    nfs3hp::port::musicMuteToggle(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c50c7:
    // 004c50c7  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c50cc  b812000000             -mov eax, 0x12
    cpu.eax = 18 /*0x12*/;
    // 004c50d1  e8daa6fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c50d6  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c50d8  7405                   -je 0x4c50df
    if (cpu.flags.zf)
    {
        goto L_0x004c50df;
    }
    // 004c50da  e86137f5ff             -call 0x418840
    cpu.esp -= 4;
    nfs3hp::decompiled::swapRaceVolumes(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c50df:
    // 004c50df  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c50e4  b816000000             -mov eax, 0x16
    cpu.eax = 22 /*0x16*/;
    // 004c50e9  e8c2a6fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c50ee  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c50f0  7410                   -je 0x4c5102
    if (cpu.flags.zf)
    {
        goto L_0x004c5102;
    }
    // 004c50f2  833d58da7c0002         +cmp dword ptr [0x7cda58], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8182360) /* 0x7cda58 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c50f9  7d07                   -jge 0x4c5102
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c5102;
    }
    // 004c50fb  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c50fd  e87eeaf5ff             -call 0x423b80
    cpu.esp -= 4;
    nfs3hp::carstate::settleCamera(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c5102:
    // 004c5102  8b1db0d36f00           -mov ebx, dword ptr [0x6fd3b0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 004c5108  83fb01                 +cmp ebx, 1
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c510b  7520                   -jne 0x4c512d
    if (!cpu.flags.zf)
    {
        goto L_0x004c512d;
    }
    // 004c510d  b818000000             -mov eax, 0x18
    cpu.eax = 24 /*0x18*/;
    // 004c5112  89da                   -mov edx, ebx
    cpu.edx = cpu.ebx;
    // 004c5114  e897a6fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5119  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c511b  7410                   -je 0x4c512d
    if (cpu.flags.zf)
    {
        goto L_0x004c512d;
    }
    // 004c511d  833d58da7c0002         +cmp dword ptr [0x7cda58], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8182360) /* 0x7cda58 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5124  7d07                   -jge 0x4c512d
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c512d;
    }
    // 004c5126  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004c5128  e853eaf5ff             -call 0x423b80
    cpu.esp -= 4;
    nfs3hp::carstate::settleCamera(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c512d:
    // 004c512d  b815000000             -mov eax, 0x15
    cpu.eax = 21 /*0x15*/;
    // 004c5132  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c5134  e877a6fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5139  a3a0c47900             -mov dword ptr [0x79c4a0], eax
    app->getMemory<x86::reg32>(x86::reg32(7980192) /* 0x79c4a0 */) = cpu.eax;
    // 004c513e  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 004c5145  7511                   -jne 0x4c5158
    if (!cpu.flags.zf)
    {
        goto L_0x004c5158;
    }
    // 004c5147  b817000000             -mov eax, 0x17
    cpu.eax = 23 /*0x17*/;
    // 004c514c  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c514e  e85da6fcff             -call 0x48f7b0
    cpu.esp -= 4;
    nfs3hp::controls::functionKeyPressed(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5153  a3a4c47900             -mov dword ptr [0x79c4a4], eax
    app->getMemory<x86::reg32>(x86::reg32(7980196) /* 0x79c4a4 */) = cpu.eax;
L_0x004c5158:
    // 004c5158  bb78367d00             -mov ebx, 0x7d3678
    cpu.ebx = 8205944 /*0x7d3678*/;
    // 004c515d  ba7c367d00             -mov edx, 0x7d367c
    cpu.edx = 8205948 /*0x7d367c*/;
    // 004c5162  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004c5164  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c5166  e8a501f7ff             -call 0x435310
    cpu.esp -= 4;
    nfs3hp::controls::readMouse(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c516b  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004c516d  a17c367d00             -mov eax, dword ptr [0x7d367c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205948) /* 0x7d367c */);
    // 004c5172  8b15b8367d00           -mov edx, dword ptr [0x7d36b8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8206008) /* 0x7d36b8 */);
    // 004c5178  39d0                   +cmp eax, edx
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
    // 004c517a  750e                   -jne 0x4c518a
    if (!cpu.flags.zf)
    {
        goto L_0x004c518a;
    }
    // 004c517c  8b0dbc367d00           -mov ecx, dword ptr [0x7d36bc]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8206012) /* 0x7d36bc */);
    // 004c5182  3b0d78367d00           +cmp ecx, dword ptr [0x7d3678]
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8205944) /* 0x7d3678 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5188  741c                   -je 0x4c51a6
    if (cpu.flags.zf)
    {
        goto L_0x004c51a6;
    }
L_0x004c518a:
    // 004c518a  a17c367d00             -mov eax, dword ptr [0x7d367c]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205948) /* 0x7d367c */);
    // 004c518f  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004c5191  a3b8367d00             -mov dword ptr [0x7d36b8], eax
    app->getMemory<x86::reg32>(x86::reg32(8206008) /* 0x7d36b8 */) = cpu.eax;
    // 004c5196  a178367d00             -mov eax, dword ptr [0x7d3678]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205944) /* 0x7d3678 */);
    // 004c519b  891dece55500           -mov dword ptr [0x55e5ec], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629420) /* 0x55e5ec */) = cpu.ebx;
    // 004c51a1  a3bc367d00             -mov dword ptr [0x7d36bc], eax
    app->getMemory<x86::reg32>(x86::reg32(8206012) /* 0x7d36bc */) = cpu.eax;
L_0x004c51a6:
    // 004c51a6  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 004c51ad  751b                   -jne 0x4c51ca
    if (!cpu.flags.zf)
    {
        goto L_0x004c51ca;
    }
    // 004c51af  8b152cdb7c00           -mov edx, dword ptr [0x7cdb2c]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8182572) /* 0x7cdb2c */);
    // 004c51b5  3b1578367d00           +cmp edx, dword ptr [0x7d3678]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8205944) /* 0x7d3678 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c51bb  7e04                   -jle 0x4c51c1
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c51c1;
    }
    // 004c51bd  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004c51bf  eb0f                   -jmp 0x4c51d0
    goto L_0x004c51d0;
L_0x004c51c1:
    // 004c51c1  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004c51c6  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004c51c8  eb06                   -jmp 0x4c51d0
    goto L_0x004c51d0;
L_0x004c51ca:
    // 004c51ca  8b15f4d46f00           -mov edx, dword ptr [0x6fd4f4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
L_0x004c51d0:
    // 004c51d0  3b15f8d46f00           +cmp edx, dword ptr [0x6fd4f8]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c51d6  0f8467000000           -je 0x4c5243
    if (cpu.flags.zf)
    {
        goto L_0x004c5243;
    }
    // 004c51dc  f605ede5550002         +test byte ptr [0x55e5ed], 2
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(5629421) /* 0x55e5ed */) & 2 /*0x2*/));
    // 004c51e3  745e                   -je 0x4c5243
    if (cpu.flags.zf)
    {
        goto L_0x004c5243;
    }
    // 004c51e5  8b0495c8fa5e00         -mov eax, dword ptr [edx*4 + 0x5efac8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6224584) /* 0x5efac8 */ + cpu.edx * 4);
    // 004c51ec  f6800002000001         +test byte ptr [eax + 0x200], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 1 /*0x1*/));
    // 004c51f3  7409                   -je 0x4c51fe
    if (cpu.flags.zf)
    {
        goto L_0x004c51fe;
    }
    // 004c51f5  83b8c402000003         +cmp dword ptr [eax + 0x2c4], 3
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(708) /* 0x2c4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(3 /*0x3*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c51fc  7445                   -je 0x4c5243
    if (cpu.flags.zf)
    {
        goto L_0x004c5243;
    }
L_0x004c51fe:
    // 004c51fe  8b0495c8fa5e00         -mov eax, dword ptr [edx*4 + 0x5efac8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6224584) /* 0x5efac8 */ + cpu.edx * 4);
    // 004c5205  f6800002000001         +test byte ptr [eax + 0x200], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(cpu.eax + x86::reg32(512) /* 0x200 */) & 1 /*0x1*/));
    // 004c520c  7409                   -je 0x4c5217
    if (cpu.flags.zf)
    {
        goto L_0x004c5217;
    }
    // 004c520e  83b8c402000004         +cmp dword ptr [eax + 0x2c4], 4
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
    // 004c5215  742c                   -je 0x4c5243
    if (cpu.flags.zf)
    {
        goto L_0x004c5243;
    }
L_0x004c5217:
    // 004c5217  8915f8d46f00           -mov dword ptr [0x6fd4f8], edx
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.edx;
    // 004c521d  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 004c521f  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004c5221  891dece55500           -mov dword ptr [0x55e5ec], ebx
    app->getMemory<x86::reg32>(x86::reg32(5629420) /* 0x55e5ec */) = cpu.ebx;
    // 004c5227  e894c3f5ff             -call 0x4215c0
    cpu.esp -= 4;
    nfs3hp::carstate::attachCameras(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c522c  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004c522e  e80d04fcff             -call 0x485640
    cpu.esp -= 4;
    nfs3hp::frontend::hudRebuildView(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5233  eb0e                   -jmp 0x4c5243
    goto L_0x004c5243;
L_0x004c5235:
    // 004c5235  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 004c523c  7f05                   -jg 0x4c5243
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004c5243;
    }
    // 004c523e  e87df8ffff             -call 0x4c4ac0
    cpu.esp -= 4;
    sub_4c4ac0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c5243:
    // 004c5243  833db0d36f0002         +cmp dword ptr [0x6fd3b0], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c524a  7c09                   -jl 0x4c5255
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c5255;
    }
    // 004c524c  833da0d36f0001         +cmp dword ptr [0x6fd3a0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5253  7e21                   -jle 0x4c5276
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c5276;
    }
L_0x004c5255:
    // 004c5255  b838000000             -mov eax, 0x38
    cpu.eax = 56 /*0x38*/;
    // 004c525a  e8615f0200             -call 0x4eb1c0
    cpu.esp -= 4;
    sub_4eb1c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c525f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c5261  7413                   -je 0x4c5276
    if (cpu.flags.zf)
    {
        goto L_0x004c5276;
    }
    // 004c5263  b819000000             -mov eax, 0x19
    cpu.eax = 25 /*0x19*/;
    // 004c5268  e8535f0200             -call 0x4eb1c0
    cpu.esp -= 4;
    sub_4eb1c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c526d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c526f  7405                   -je 0x4c5276
    if (cpu.flags.zf)
    {
        goto L_0x004c5276;
    }
    // 004c5271  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
L_0x004c5276:
    // 004c5276  ff0588367d00           +inc dword ptr [0x7d3688]
    {
        auto tmp = app->getMemory<x86::reg32>(x86::reg32(8205960) /* 0x7d3688 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c527c  e973fcffff             -jmp 0x4c4ef4
    goto L_0x004c4ef4;
L_0x004c5281:
    // 004c5281  833da0367d0000         +cmp dword ptr [0x7d36a0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5288  0f85d9010000           -jne 0x4c5467
    if (!cpu.flags.zf)
    {
        goto L_0x004c5467;
    }
    // 004c528e  833da8367d0000         +cmp dword ptr [0x7d36a8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205992) /* 0x7d36a8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5295  0f85cc010000           -jne 0x4c5467
    if (!cpu.flags.zf)
    {
        goto L_0x004c5467;
    }
    // 004c529b  833d9c367d0000         +cmp dword ptr [0x7d369c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205980) /* 0x7d369c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c52a2  0f85bf010000           -jne 0x4c5467
    if (!cpu.flags.zf)
    {
        goto L_0x004c5467;
    }
    // 004c52a8  8b15ac367d00           -mov edx, dword ptr [0x7d36ac]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8205996) /* 0x7d36ac */);
    // 004c52ae  3b15b0367d00           +cmp edx, dword ptr [0x7d36b0]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c52b4  7d0a                   -jge 0x4c52c0
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c52c0;
    }
    // 004c52b6  c70574367d0001000000   -mov dword ptr [0x7d3674], 1
    app->getMemory<x86::reg32>(x86::reg32(8205940) /* 0x7d3674 */) = 1 /*0x1*/;
L_0x004c52c0:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c52c0  8b1dac367d00           -mov ebx, dword ptr [0x7d36ac]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8205996) /* 0x7d36ac */);
    // 004c52c6  3b1db0367d00           +cmp ebx, dword ptr [0x7d36b0]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c52cc  0f8c9c010000           -jl 0x4c546e
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c546e;
    }
    // 004c52d2  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c52d4  e807fbffff             -call 0x4c4de0
    cpu.esp -= 4;
    sub_4c4de0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c52d9  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c52db  0f858d010000           -jne 0x4c546e
    if (!cpu.flags.zf)
    {
        goto L_0x004c546e;
    }
    // 004c52e1  f605b0367d0001         +test byte ptr [0x7d36b0], 1
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8206000) /* 0x7d36b0 */) & 1 /*0x1*/));
    // 004c52e8  750c                   -jne 0x4c52f6
    if (!cpu.flags.zf)
    {
        goto L_0x004c52f6;
    }
    // 004c52ea  a1b0367d00             -mov eax, dword ptr [0x7d36b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */);
    // 004c52ef  d1f8                   -sar eax, 1
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (1 /*0x1*/ % 32));
    // 004c52f1  e8aa140000             -call 0x4c67a0
    cpu.esp -= 4;
    nfs3hp::controls::takeQueuedFrame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c52f6:
    // 004c52f6  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004c52f8  a158da7c00             -mov eax, dword ptr [0x7cda58]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8182360) /* 0x7cda58 */);
    // 004c52fd  8935ec005600           -mov dword ptr [0x5600ec], esi
    app->getMemory<x86::reg32>(x86::reg32(5636332) /* 0x5600ec */) = cpu.esi;
    // 004c5303  83f802                 +cmp eax, 2
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
    // 004c5306  0f8596000000           -jne 0x4c53a2
    if (!cpu.flags.zf)
    {
        goto L_0x004c53a2;
    }
    // 004c530c  833d4cd97c0000         +cmp dword ptr [0x7cd94c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8182092) /* 0x7cd94c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5313  7515                   -jne 0x4c532a
    if (!cpu.flags.zf)
    {
        goto L_0x004c532a;
    }
    // 004c5315  a1b0367d00             -mov eax, dword ptr [0x7d36b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */);
    // 004c531a  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004c531c  be04000000             -mov esi, 4
    cpu.esi = 4 /*0x4*/;
    // 004c5321  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004c5324  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004c5326  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004c5328  7429                   -je 0x4c5353
    if (cpu.flags.zf)
    {
        goto L_0x004c5353;
    }
L_0x004c532a:
    // 004c532a  833d4cd97c0001         +cmp dword ptr [0x7cd94c], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8182092) /* 0x7cd94c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5331  7515                   -jne 0x4c5348
    if (!cpu.flags.zf)
    {
        goto L_0x004c5348;
    }
    // 004c5333  a1b0367d00             -mov eax, dword ptr [0x7d36b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */);
    // 004c5338  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004c533a  be02000000             -mov esi, 2
    cpu.esi = 2 /*0x2*/;
    // 004c533f  c1fa1f                 -sar edx, 0x1f
    cpu.edx = x86::reg32(x86::sreg32(cpu.edx) >> (31 /*0x1f*/ % 32));
    // 004c5342  f7fe                   -idiv esi
    {
        x86::sreg64 tmp = x86::sreg64(cpu.edx_eax);
        x86::sreg32 d = x86::sreg32(cpu.esi);
        cpu.edx_eax = x86::reg64(tmp / d);
        cpu.edx = x86::reg32(tmp % d);
    }
    // 004c5344  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004c5346  740b                   -je 0x4c5353
    if (cpu.flags.zf)
    {
        goto L_0x004c5353;
    }
L_0x004c5348:
    // 004c5348  8b1d4cd97c00           -mov ebx, dword ptr [0x7cd94c]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8182092) /* 0x7cd94c */);
    // 004c534e  83fb02                 +cmp ebx, 2
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
    // 004c5351  750f                   -jne 0x4c5362
    if (!cpu.flags.zf)
    {
        goto L_0x004c5362;
    }
L_0x004c5353:
    // 004c5353  a1b0367d00             -mov eax, dword ptr [0x7d36b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */);
    // 004c5358  e873f5ffff             -call 0x4c48d0
    cpu.esp -= 4;
    sub_4c48d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c535d  e9fa000000             -jmp 0x4c545c
    goto L_0x004c545c;
L_0x004c5362:
    // 004c5362  83fb03                 +cmp ebx, 3
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
    // 004c5365  751f                   -jne 0x4c5386
    if (!cpu.flags.zf)
    {
        goto L_0x004c5386;
    }
    // 004c5367  a1b0367d00             -mov eax, dword ptr [0x7d36b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */);
    // 004c536c  e85ff5ffff             -call 0x4c48d0
    cpu.esp -= 4;
    sub_4c48d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5371  a1b0367d00             -mov eax, dword ptr [0x7d36b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */);
    // 004c5376  e855f5ffff             -call 0x4c48d0
    cpu.esp -= 4;
    sub_4c48d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c537b  ff05b0367d00           +inc dword ptr [0x7d36b0]
    {
        auto tmp = app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c5381  e93affffff             -jmp 0x4c52c0
    goto L_0x004c52c0;
L_0x004c5386:
    // 004c5386  c705ec00560001000000   -mov dword ptr [0x5600ec], 1
    app->getMemory<x86::reg32>(x86::reg32(5636332) /* 0x5600ec */) = 1 /*0x1*/;
    // 004c5390  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004c5392  e8e912f6ff             -call 0x426680
    cpu.esp -= 4;
    nfs3hp::frontend::simCamerasStep(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5397  ff05b0367d00           +inc dword ptr [0x7d36b0]
    {
        auto tmp = app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c539d  e91effffff             -jmp 0x4c52c0
    goto L_0x004c52c0;
L_0x004c53a2:
    // 004c53a2  83f804                 +cmp eax, 4
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
    // 004c53a5  752b                   -jne 0x4c53d2
    if (!cpu.flags.zf)
    {
        goto L_0x004c53d2;
    }
    // 004c53a7  e8f433f5ff             -call 0x4187a0
    cpu.esp -= 4;
    nfs3hp::port::slotsStop(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c53ac  e8ffdffdff             -call 0x4a33b0
    cpu.esp -= 4;
    sub_4a33b0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c53b1  b902000000             -mov ecx, 2
    cpu.ecx = 2 /*0x2*/;
    // 004c53b6  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004c53bb  893504015600           -mov dword ptr [0x560104], esi
    app->getMemory<x86::reg32>(x86::reg32(5636356) /* 0x560104 */) = cpu.esi;
    // 004c53c1  890d58da7c00           -mov dword ptr [0x7cda58], ecx
    app->getMemory<x86::reg32>(x86::reg32(8182360) /* 0x7cda58 */) = cpu.ecx;
    // 004c53c7  891d74367d00           -mov dword ptr [0x7d3674], ebx
    app->getMemory<x86::reg32>(x86::reg32(8205940) /* 0x7d3674 */) = cpu.ebx;
    // 004c53cd  e99c000000             -jmp 0x4c546e
    goto L_0x004c546e;
L_0x004c53d2:
    // 004c53d2  a1b0367d00             -mov eax, dword ptr [0x7d36b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */);
    // 004c53d7  e8e42d0000             -call 0x4c81c0
    cpu.esp -= 4;
    sub_4c81c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c53dc  a1b0367d00             -mov eax, dword ptr [0x7d36b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */);
    // 004c53e1  e8eaf4ffff             -call 0x4c48d0
    cpu.esp -= 4;
    sub_4c48d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c53e6  f6052eeb550020         +test byte ptr [0x55eb2e], 0x20
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(5630766) /* 0x55eb2e */) & 32 /*0x20*/));
    // 004c53ed  745a                   -je 0x4c5449
    if (cpu.flags.zf)
    {
        goto L_0x004c5449;
    }
    // 004c53ef  a1a0367d00             -mov eax, dword ptr [0x7d36a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */);
    // 004c53f4  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c53f6  7551                   -jne 0x4c5449
    if (!cpu.flags.zf)
    {
        goto L_0x004c5449;
    }
    // 004c53f8  833d9c367d0000         +cmp dword ptr [0x7d369c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205980) /* 0x7d369c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c53ff  7448                   -je 0x4c5449
    if (cpu.flags.zf)
    {
        goto L_0x004c5449;
    }
    // 004c5401  833db4367d0000         +cmp dword ptr [0x7d36b4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8206004) /* 0x7d36b4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5408  753f                   -jne 0x4c5449
    if (!cpu.flags.zf)
    {
        goto L_0x004c5449;
    }
    // 004c540a  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c540c  e8af080100             -call 0x4d5cc0
    cpu.esp -= 4;
    nfs3hp::graphics::beginDrawing(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5411  a1fcfd5e00             -mov eax, dword ptr [0x5efdfc]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(6225404) /* 0x5efdfc */);
    // 004c5416  83c010                 -add eax, 0x10
    (cpu.eax) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004c5419  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004c541e  a3a4367d00             -mov dword ptr [0x7d36a4], eax
    app->getMemory<x86::reg32>(x86::reg32(8205988) /* 0x7d36a4 */) = cpu.eax;
    // 004c5423  e8e831f5ff             -call 0x418610
    cpu.esp -= 4;
    nfs3hp::port::masterSilent(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5428  891d08015600           -mov dword ptr [0x560108], ebx
    app->getMemory<x86::reg32>(x86::reg32(5636360) /* 0x560108 */) = cpu.ebx;
    // 004c542e  891da0367d00           -mov dword ptr [0x7d36a0], ebx
    app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */) = cpu.ebx;
    // 004c5434  89359c367d00           -mov dword ptr [0x7d369c], esi
    app->getMemory<x86::reg32>(x86::reg32(8205980) /* 0x7d369c */) = cpu.esi;
    // 004c543a  e801bbfaff             -call 0x470f40
    cpu.esp -= 4;
    nfs3hp::controls::stopForces(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c543f  b81b000000             -mov eax, 0x1b
    cpu.eax = 27 /*0x1b*/;
    // 004c5444  e8f7c4f8ff             -call 0x451940
    cpu.esp -= 4;
    nfs3hp::frontend::connectFlushKeys(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c5449:
    // 004c5449  f605b0367d001f         +test byte ptr [0x7d36b0], 0x1f
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg8>(app->getMemory<x86::reg8>(x86::reg32(8206000) /* 0x7d36b0 */) & 31 /*0x1f*/));
    // 004c5450  750a                   -jne 0x4c545c
    if (!cpu.flags.zf)
    {
        goto L_0x004c545c;
    }
    // 004c5452  a1b0367d00             -mov eax, dword ptr [0x7d36b0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */);
    // 004c5457  e874270000             -call 0x4c7bd0
    cpu.esp -= 4;
    sub_4c7bd0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c545c:
    // 004c545c  ff05b0367d00           +inc dword ptr [0x7d36b0]
    {
        auto tmp = app->getMemory<x86::reg32>(x86::reg32(8206000) /* 0x7d36b0 */);
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c5462  e959feffff             -jmp 0x4c52c0
    goto L_0x004c52c0;
L_0x004c5467:
    // 004c5467  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c5469  e81212f6ff             -call 0x426680
    cpu.esp -= 4;
    nfs3hp::frontend::simCamerasStep(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c546e:
    // 004c546e  833d74367d0000         +cmp dword ptr [0x7d3674], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8205940) /* 0x7d3674 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5475  7517                   -jne 0x4c548e
    if (!cpu.flags.zf)
    {
        goto L_0x004c548e;
    }
    // 004c5477  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004c5479  7409                   -je 0x4c5484
    if (cpu.flags.zf)
    {
        goto L_0x004c5484;
    }
    // 004c547b  e810f0ffff             -call 0x4c4490
    cpu.esp -= 4;
    nfs3hp::graphics::captureScreen(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5480  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 004c5482  eb0a                   -jmp 0x4c548e
    goto L_0x004c548e;
L_0x004c5484:
    // 004c5484  a1a0367d00             -mov eax, dword ptr [0x7d36a0]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205984) /* 0x7d36a0 */);
    // 004c5489  e8f271ffff             -call 0x4bc680
    cpu.esp -= 4;
    sub_4bc680(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c548e:
    // 004c548e  833db0d36f0004         +cmp dword ptr [0x6fd3b0], 4
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(4 /*0x4*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5495  7517                   -jne 0x4c54ae
    if (!cpu.flags.zf)
    {
        goto L_0x004c54ae;
    }
    // 004c5497  833da0d36f0001         +cmp dword ptr [0x6fd3a0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c549e  7f0e                   -jg 0x4c54ae
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004c54ae;
    }
    // 004c54a0  833df4d46f0000         +cmp dword ptr [0x6fd4f4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c54a7  7505                   -jne 0x4c54ae
    if (!cpu.flags.zf)
    {
        goto L_0x004c54ae;
    }
    // 004c54a9  e8c297fdff             -call 0x49ec70
    cpu.esp -= 4;
    sub_49ec70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c54ae:
    // 004c54ae  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c54b6  0f84c8f9ffff           -je 0x4c4e84
    if (cpu.flags.zf)
    {
        goto L_0x004c4e84;
    }
    // 004c54bc  833da0d36f0001         +cmp dword ptr [0x6fd3a0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c54c3  7f11                   -jg 0x4c54d6
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004c54d6;
    }
    // 004c54c5  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004c54ca  890d0c015600           -mov dword ptr [0x56010c], ecx
    app->getMemory<x86::reg32>(x86::reg32(5636364) /* 0x56010c */) = cpu.ecx;
    // 004c54d0  890d08015600           -mov dword ptr [0x560108], ecx
    app->getMemory<x86::reg32>(x86::reg32(5636360) /* 0x560108 */) = cpu.ecx;
L_0x004c54d6:
    // 004c54d6  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c54db  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004c54e0  e86b920000             -call 0x4ce750
    cpu.esp -= 4;
    sub_4ce750(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c54e5  89359c367d00           -mov dword ptr [0x7d369c], esi
    app->getMemory<x86::reg32>(x86::reg32(8205980) /* 0x7d369c */) = cpu.esi;
    // 004c54eb  e994f9ffff             -jmp 0x4c4e84
    goto L_0x004c4e84;
L_0x004c54f0:
    // 004c54f0  833da0d36f0001         +cmp dword ptr [0x6fd3a0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c54f7  7f0f                   -jg 0x4c5508
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004c5508;
    }
    // 004c54f9  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c54fe  a30c015600             -mov dword ptr [0x56010c], eax
    app->getMemory<x86::reg32>(x86::reg32(5636364) /* 0x56010c */) = cpu.eax;
    // 004c5503  a308015600             -mov dword ptr [0x560108], eax
    app->getMemory<x86::reg32>(x86::reg32(5636360) /* 0x560108 */) = cpu.eax;
L_0x004c5508:
    // 004c5508  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c550a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c550b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c550c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c550d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c550e  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c550f  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5510  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4c5520(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c5520  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c5521  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c5522  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c5524  e8b75d0200             -call 0x4eb2e0
    cpu.esp -= 4;
    nfs3hp::ea::makeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5529  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c552e  a3d8367d00             -mov dword ptr [0x7d36d8], eax
    app->getMemory<x86::reg32>(x86::reg32(8206040) /* 0x7d36d8 */) = cpu.eax;
    // 004c5533  891500015600           -mov dword ptr [0x560100], edx
    app->getMemory<x86::reg32>(x86::reg32(5636352) /* 0x560100 */) = cpu.edx;
    // 004c5539  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c553a  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c553b  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x44 0x20 0x00 */
void Application::sub_4c5540(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c5540  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c5541  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c5542  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c5544  ba10000000             -mov edx, 0x10
    cpu.edx = 16 /*0x10*/;
    // 004c5549  b8c0367d00             -mov eax, 0x7d36c0
    cpu.eax = 8206016 /*0x7d36c0*/;
    // 004c554e  e8b9b10100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5553  c705cc367d0020000000   -mov dword ptr [0x7d36cc], 0x20
    app->getMemory<x86::reg32>(x86::reg32(8206028) /* 0x7d36cc */) = 32 /*0x20*/;
    // 004c555d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c555e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c555f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4c5560(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c5560  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c5561  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c5562  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c5564  ba07000000             -mov edx, 7
    cpu.edx = 7 /*0x7*/;
    // 004c5569  b878000000             -mov eax, 0x78
    cpu.eax = 120 /*0x78*/;
    // 004c556e  e89d14feff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5573  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c5575  a1d8367d00             -mov eax, dword ptr [0x7d36d8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8206040) /* 0x7d36d8 */);
    // 004c557a  891500015600           -mov dword ptr [0x560100], edx
    app->getMemory<x86::reg32>(x86::reg32(5636352) /* 0x560100 */) = cpu.edx;
    // 004c5580  e8db5d0200             -call 0x4eb360
    cpu.esp -= 4;
    nfs3hp::ea::freeLock(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5585  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c558d  7509                   -jne 0x4c5598
    if (!cpu.flags.zf)
    {
        goto L_0x004c5598;
    }
    // 004c558f  833db4367d0001         +cmp dword ptr [0x7d36b4], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8206004) /* 0x7d36b4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5596  7505                   -jne 0x4c559d
    if (!cpu.flags.zf)
    {
        goto L_0x004c559d;
    }
L_0x004c5598:
    // 004c5598  e8e3fc0200             -call 0x4f5280
    cpu.esp -= 4;
    nfs3hp::ea::connRestore(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c559d:
    // 004c559d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c559e  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c559f  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4c55a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c55a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c55a1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c55a2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c55a3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c55a4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c55a5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c55a6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c55a8  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004c55ab  8b15b0d36f00           -mov edx, dword ptr [0x6fd3b0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 004c55b1  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004c55b3  0f843f010000           -je 0x4c56f8
    if (cpu.flags.zf)
    {
        goto L_0x004c56f8;
    }
    // 004c55b9  83fa01                 +cmp edx, 1
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
    // 004c55bc  0f8436010000           -je 0x4c56f8
    if (cpu.flags.zf)
    {
        goto L_0x004c56f8;
    }
    // 004c55c2  833da0d36f0001         +cmp dword ptr [0x6fd3a0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c55c9  0f8f29010000           -jg 0x4c56f8
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004c56f8;
    }
    // 004c55cf  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c55d1  0f84a8000000           -je 0x4c567f
    if (cpu.flags.zf)
    {
        goto L_0x004c567f;
    }
    // 004c55d7  8b35c8367d00           -mov esi, dword ptr [0x7d36c8]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8206024) /* 0x7d36c8 */);
    // 004c55dd  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004c55df  0f859a000000           -jne 0x4c567f
    if (!cpu.flags.zf)
    {
        goto L_0x004c567f;
    }
    // 004c55e5  833dc0367d0000         +cmp dword ptr [0x7d36c0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8206016) /* 0x7d36c0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c55ec  0f858d000000           -jne 0x4c567f
    if (!cpu.flags.zf)
    {
        goto L_0x004c567f;
    }
    // 004c55f2  8b15cc367d00           -mov edx, dword ptr [0x7d36cc]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8206028) /* 0x7d36cc */);
    // 004c55f8  83c220                 -add edx, 0x20
    (cpu.edx) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 004c55fb  e8a0130000             -call 0x4c69a0
    cpu.esp -= 4;
    nfs3hp::controls::framesTaken(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5600  39c2                   +cmp edx, eax
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
    // 004c5602  0f8d77000000           -jge 0x4c567f
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c567f;
    }
    // 004c5608  b408                   -mov ah, 8
    cpu.ah = 8 /*0x8*/;
    // 004c560a  668b15da227a00         -mov dx, word ptr [0x7a22da]
    cpu.dx = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
    // 004c5611  8975f8                 -mov dword ptr [ebp - 8], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.esi;
    // 004c5614  8865f4                 -mov byte ptr [ebp - 0xc], ah
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ah;
    // 004c5617  6685d2                 +test dx, dx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.dx & cpu.dx));
    // 004c561a  7522                   -jne 0x4c563e
    if (!cpu.flags.zf)
    {
        goto L_0x004c563e;
    }
    // 004c561c  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004c5621  bb0c000000             -mov ebx, 0xc
    cpu.ebx = 12 /*0xc*/;
    // 004c5626  8d55f4                 -lea edx, [ebp - 0xc]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004c5629  b8e8227a00             -mov eax, 0x7a22e8
    cpu.eax = 8004328 /*0x7a22e8*/;
    // 004c562e  8975fc                 -mov dword ptr [ebp - 4], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.esi;
    // 004c5631  890dc0367d00           -mov dword ptr [0x7d36c0], ecx
    app->getMemory<x86::reg32>(x86::reg32(8206016) /* 0x7d36c0 */) = cpu.ecx;
    // 004c5637  e8a40afeff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c563c  eb41                   -jmp 0x4c567f
    goto L_0x004c567f;
L_0x004c563e:
    // 004c563e  8b1dd8367d00           -mov ebx, dword ptr [0x7d36d8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8206040) /* 0x7d36d8 */);
    // 004c5644  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c5645  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004c564a  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5650  e81b130000             -call 0x4c6970
    cpu.esp -= 4;
    nfs3hp::controls::syncFrame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5655  bb0c000000             -mov ebx, 0xc
    cpu.ebx = 12 /*0xc*/;
    // 004c565a  8d55f4                 -lea edx, [ebp - 0xc]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004c565d  a3c8367d00             -mov dword ptr [0x7d36c8], eax
    app->getMemory<x86::reg32>(x86::reg32(8206024) /* 0x7d36c8 */) = cpu.eax;
    // 004c5662  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004c5665  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c5667  8935c0367d00           -mov dword ptr [0x7d36c0], esi
    app->getMemory<x86::reg32>(x86::reg32(8206016) /* 0x7d36c0 */) = cpu.esi;
    // 004c566d  e86e0afeff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5672  8b3dd8367d00           -mov edi, dword ptr [0x7d36d8]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(8206040) /* 0x7d36d8 */);
    // 004c5678  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c5679  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c567f:
    // 004c567f  833dc0367d0000         +cmp dword ptr [0x7d36c0], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8206016) /* 0x7d36c0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5686  0f846a000000           -je 0x4c56f6
    if (cpu.flags.zf)
    {
        goto L_0x004c56f6;
    }
    // 004c568c  833dc8367d0000         +cmp dword ptr [0x7d36c8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8206024) /* 0x7d36c8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5693  7461                   -je 0x4c56f6
    if (cpu.flags.zf)
    {
        goto L_0x004c56f6;
    }
    // 004c5695  e806130000             -call 0x4c69a0
    cpu.esp -= 4;
    nfs3hp::controls::framesTaken(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c569a  8b0dc8367d00           -mov ecx, dword ptr [0x7d36c8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8206024) /* 0x7d36c8 */);
    // 004c56a0  39c8                   +cmp eax, ecx
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
    // 004c56a2  7c52                   -jl 0x4c56f6
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c56f6;
    }
    // 004c56a4  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004c56a9  ba08000000             -mov edx, 8
    cpu.edx = 8 /*0x8*/;
    // 004c56ae  b81e000000             -mov eax, 0x1e
    cpu.eax = 30 /*0x1e*/;
    // 004c56b3  890dcc367d00           -mov dword ptr [0x7d36cc], ecx
    app->getMemory<x86::reg32>(x86::reg32(8206028) /* 0x7d36cc */) = cpu.ecx;
    // 004c56b9  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004c56bb  891d08015600           -mov dword ptr [0x560108], ebx
    app->getMemory<x86::reg32>(x86::reg32(5636360) /* 0x560108 */) = cpu.ebx;
    // 004c56c1  8935c4367d00           -mov dword ptr [0x7d36c4], esi
    app->getMemory<x86::reg32>(x86::reg32(8206020) /* 0x7d36c4 */) = cpu.esi;
    // 004c56c7  e84413feff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c56cc  e82f81f6ff             -call 0x42d800
    cpu.esp -= 4;
    nfs3hp::raceload::clearMessages(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c56d1  8935c8367d00           -mov dword ptr [0x7d36c8], esi
    app->getMemory<x86::reg32>(x86::reg32(8206024) /* 0x7d36c8 */) = cpu.esi;
    // 004c56d7  8935c0367d00           -mov dword ptr [0x7d36c0], esi
    app->getMemory<x86::reg32>(x86::reg32(8206016) /* 0x7d36c0 */) = cpu.esi;
L_0x004c56dd:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c56dd  e81eaa0200             -call 0x4f0100
    cpu.esp -= 4;
    nfs3hp::ea::keyFromHook(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c56e2  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c56e4  7409                   -je 0x4c56ef
    if (cpu.flags.zf)
    {
        goto L_0x004c56ef;
    }
    // 004c56e6  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004c56e8  e8431f0200             -call 0x4e7630
    cpu.esp -= 4;
    nfs3hp::port::runSystemTasks(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c56ed  ebee                   -jmp 0x4c56dd
    goto L_0x004c56dd;
L_0x004c56ef:
    // 004c56ef  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c56f4  eb02                   -jmp 0x4c56f8
    goto L_0x004c56f8;
L_0x004c56f6:
    // 004c56f6  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004c56f8:
    // 004c56f8  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c56fa  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c56fb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c56fc  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c56fd  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c56fe  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c56ff  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5700  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4c5720(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 004c5720  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c5721  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c5722  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c5723  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c5724  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c5725  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c5726  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c5728  83ec18                 -sub esp, 0x18
    (cpu.esp) -= x86::reg32(x86::sreg32(24 /*0x18*/));
    // 004c572b  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004c572d  8b15b0d36f00           -mov edx, dword ptr [0x6fd3b0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 004c5733  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004c5735  740e                   -je 0x4c5745
    if (cpu.flags.zf)
    {
        goto L_0x004c5745;
    }
    // 004c5737  83fa01                 +cmp edx, 1
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
    // 004c573a  7409                   -je 0x4c5745
    if (cpu.flags.zf)
    {
        goto L_0x004c5745;
    }
    // 004c573c  833da0d36f0001         +cmp dword ptr [0x6fd3a0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5743  7e07                   -jle 0x4c574c
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c574c;
    }
L_0x004c5745:
    // 004c5745  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004c5747  e98f030000             -jmp 0x4c5adb
    goto L_0x004c5adb;
L_0x004c574c:
    // 004c574c  a1a4c17900             -mov eax, dword ptr [0x79c1a4]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(7979428) /* 0x79c1a4 */);
    // 004c5751  89c3                   -mov ebx, eax
    cpu.ebx = cpu.eax;
    // 004c5753  e818720000             -call 0x4cc970
    cpu.esp -= 4;
    nfs3hp::frontend::nextKey(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5758  25ffff0000             -and eax, 0xffff
    cpu.eax &= x86::reg32(x86::sreg32(65535 /*0xffff*/));
    // 004c575d  3dff000000             +cmp eax, 0xff
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
    // 004c5762  7d05                   -jge 0x4c5769
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c5769;
    }
    // 004c5764  e877960200             -call 0x4eede0
    cpu.esp -= 4;
    nfs3hp::watcom::toupper(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c5769:
    // 004c5769  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c576b  742d                   -je 0x4c579a
    if (cpu.flags.zf)
    {
        goto L_0x004c579a;
    }
    // 004c576d  3b05e0367d00           +cmp eax, dword ptr [0x7d36e0]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8206048) /* 0x7d36e0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5773  7508                   -jne 0x4c577d
    if (!cpu.flags.zf)
    {
        goto L_0x004c577d;
    }
    // 004c5775  3b1ddc367d00           +cmp ebx, dword ptr [0x7d36dc]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8206044) /* 0x7d36dc */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c577b  7e1d                   -jle 0x4c579a
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c579a;
    }
L_0x004c577d:
    // 004c577d  3b05e0367d00           +cmp eax, dword ptr [0x7d36e0]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(8206048) /* 0x7d36e0 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5783  7405                   -je 0x4c578a
    if (cpu.flags.zf)
    {
        goto L_0x004c578a;
    }
    // 004c5785  83c30f                 +add ebx, 0xf
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(15 /*0xf*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004c5788  eb03                   -jmp 0x4c578d
    goto L_0x004c578d;
L_0x004c578a:
    // 004c578a  83c305                 +add ebx, 5
    {
        x86::reg32& tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5 /*0x5*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
L_0x004c578d:
    // 004c578d  891ddc367d00           -mov dword ptr [0x7d36dc], ebx
    app->getMemory<x86::reg32>(x86::reg32(8206044) /* 0x7d36dc */) = cpu.ebx;
    // 004c5793  a3e0367d00             -mov dword ptr [0x7d36e0], eax
    app->getMemory<x86::reg32>(x86::reg32(8206048) /* 0x7d36e0 */) = cpu.eax;
    // 004c5798  eb0d                   -jmp 0x4c57a7
    goto L_0x004c57a7;
L_0x004c579a:
    // 004c579a  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004c579c  a3e0367d00             -mov dword ptr [0x7d36e0], eax
    app->getMemory<x86::reg32>(x86::reg32(8206048) /* 0x7d36e0 */) = cpu.eax;
    // 004c57a1  891ddc367d00           -mov dword ptr [0x7d36dc], ebx
    app->getMemory<x86::reg32>(x86::reg32(8206044) /* 0x7d36dc */) = cpu.ebx;
L_0x004c57a7:
    // 004c57a7  668b15da227a00         -mov dx, word ptr [0x7a22da]
    cpu.dx = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
    // 004c57ae  6685d2                 +test dx, dx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.dx & cpu.dx));
    // 004c57b1  0f84f9010000           -je 0x4c59b0
    if (cpu.flags.zf)
    {
        goto L_0x004c59b0;
    }
    // 004c57b7  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
L_0x004c57bc:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c57bc  3b1d14d56f00           +cmp ebx, dword ptr [0x6fd514]
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c57c2  0f8da8000000           -jge 0x4c5870
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c5870;
    }
    // 004c57c8  833dc4367d0000         +cmp dword ptr [0x7d36c4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8206020) /* 0x7d36c4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c57cf  740a                   -je 0x4c57db
    if (cpu.flags.zf)
    {
        goto L_0x004c57db;
    }
    // 004c57d1  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004c57d6  e995000000             -jmp 0x4c5870
    goto L_0x004c5870;
L_0x004c57db:
    // 004c57db  668b045dba227a00       -mov ax, word ptr [ebx*2 + 0x7a22ba]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(8004282) /* 0x7a22ba */ + cpu.ebx * 2);
    // 004c57e3  6685c0                 +test ax, ax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.ax & cpu.ax));
    // 004c57e6  0f867e000000           -jbe 0x4c586a
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004c586a;
    }
    // 004c57ec  663d0b00               +cmp ax, 0xb
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(11 /*0xb*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c57f0  7549                   -jne 0x4c583b
    if (!cpu.flags.zf)
    {
        goto L_0x004c583b;
    }
    // 004c57f2  833de4227a0000         +cmp dword ptr [0x7a22e4], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8004324) /* 0x7a22e4 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c57f9  7409                   -je 0x4c5804
    if (cpu.flags.zf)
    {
        goto L_0x004c5804;
    }
    // 004c57fb  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004c57fd  e8ee94fdff             -call 0x49ecf0
    cpu.esp -= 4;
    nfs3hp::ea::announceMachineLeft(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5802  eb05                   -jmp 0x4c5809
    goto L_0x004c5809;
L_0x004c5804:
    // 004c5804  be03000000             -mov esi, 3
    cpu.esi = 3 /*0x3*/;
L_0x004c5809:
    // 004c5809  8b0db8ec5500           -mov ecx, dword ptr [0x55ecb8]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004c580f  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c5810  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5816  8d041b                 -lea eax, [ebx + ebx]
    cpu.eax = x86::reg32(cpu.ebx + cpu.ebx * 1);
    // 004c5819  6683b8ba227a000b       +cmp word ptr [eax + 0x7a22ba], 0xb
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8004282) /* 0x7a22ba */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(11 /*0xb*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c5821  7509                   -jne 0x4c582c
    if (!cpu.flags.zf)
    {
        goto L_0x004c582c;
    }
    // 004c5823  31ff                   +xor edi, edi
    cpu.clear_co();
    cpu.set_szp((cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi))));
    // 004c5825  6689b8ba227a00         -mov word ptr [eax + 0x7a22ba], di
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8004282) /* 0x7a22ba */) = cpu.di;
L_0x004c582c:
    // 004c582c  8b3db8ec5500           -mov edi, dword ptr [0x55ecb8]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004c5832  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c5833  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5839  eb2f                   -jmp 0x4c586a
    goto L_0x004c586a;
L_0x004c583b:
    // 004c583b  a1b8ec5500             -mov eax, dword ptr [0x55ecb8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004c5840  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004c5841  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5847  8d041b                 -lea eax, [ebx + ebx]
    cpu.eax = x86::reg32(cpu.ebx + cpu.ebx * 1);
    // 004c584a  6683b8ba227a000b       +cmp word ptr [eax + 0x7a22ba], 0xb
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8004282) /* 0x7a22ba */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(11 /*0xb*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c5852  7409                   -je 0x4c585d
    if (cpu.flags.zf)
    {
        goto L_0x004c585d;
    }
    // 004c5854  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004c5856  668988ba227a00         -mov word ptr [eax + 0x7a22ba], cx
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(8004282) /* 0x7a22ba */) = cpu.cx;
L_0x004c585d:
    // 004c585d  8b15b8ec5500           -mov edx, dword ptr [0x55ecb8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004c5863  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c5864  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c586a:
    // 004c586a  43                     +inc ebx
    {
        x86::reg32& tmp = cpu.ebx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c586b  e94cffffff             -jmp 0x4c57bc
    goto L_0x004c57bc;
L_0x004c5870:
    // 004c5870  8d46ff                 -lea eax, [esi - 1]
    cpu.eax = x86::reg32(cpu.esi + x86::reg32(-1) /* -0x1 */);
    // 004c5873  83f804                 +cmp eax, 4
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
    // 004c5876  0f875d020000           -ja 0x4c5ad9
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004c5ad9;
    }
    // 004c587c  ff248504574c00         -jmp dword ptr [eax*4 + 0x4c5704]
    cpu.ip = app->getMemory<x86::reg32>(5003012 + cpu.eax * 4); goto dynamic_jump;
  case 0x004c5883:
    // 004c5883  ba09000000             -mov edx, 9
    cpu.edx = 9 /*0x9*/;
    // 004c5888  b814000000             -mov eax, 0x14
    cpu.eax = 20 /*0x14*/;
    // 004c588d  e87e11feff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5892  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004c5894  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c5899  890d0c015600           -mov dword ptr [0x56010c], ecx
    app->getMemory<x86::reg32>(x86::reg32(5636364) /* 0x56010c */) = cpu.ecx;
    // 004c589f  890dc4367d00           -mov dword ptr [0x7d36c4], ecx
    app->getMemory<x86::reg32>(x86::reg32(8206020) /* 0x7d36c4 */) = cpu.ecx;
    // 004c58a5  e8567ff6ff             -call 0x42d800
    cpu.esp -= 4;
    nfs3hp::raceload::clearMessages(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c58aa  b814000000             -mov eax, 0x14
    cpu.eax = 20 /*0x14*/;
    // 004c58af  e85c11feff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c58b4  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c58b9  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c58bb  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c58bc  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c58bd  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c58be  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c58bf  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c58c0  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c58c1  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x004c58c2:
L_0x004c58c2:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c58c2  ba0a000000             -mov edx, 0xa
    cpu.edx = 10 /*0xa*/;
    // 004c58c7  b814000000             -mov eax, 0x14
    cpu.eax = 20 /*0x14*/;
    // 004c58cc  e83f11feff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c58d1  e82a7ff6ff             -call 0x42d800
    cpu.esp -= 4;
    nfs3hp::raceload::clearMessages(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c58d6  b802000000             -mov eax, 2
    cpu.eax = 2 /*0x2*/;
    // 004c58db  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c58dd  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c58de  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c58df  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c58e0  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c58e1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c58e2  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c58e3  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x004c58e4:
    // 004c58e4  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004c58e6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c58e6  3b05a4fd5e00           +cmp eax, dword ptr [0x5efda4]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6225316) /* 0x5efda4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c58ec  7d14                   -jge 0x4c5902
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c5902;
    }
    // 004c58ee  8b148548fa5e00         -mov edx, dword ptr [eax*4 + 0x5efa48]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6224456) /* 0x5efa48 */ + cpu.eax * 4);
    // 004c58f5  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c58f6  c782c402000001000000   -mov dword ptr [edx + 0x2c4], 1
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(708) /* 0x2c4 */) = 1 /*0x1*/;
    // 004c5900  ebe4                   -jmp 0x4c58e6
    goto L_0x004c58e6;
L_0x004c5902:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c5902  ba20000000             -mov edx, 0x20
    cpu.edx = 32 /*0x20*/;
    // 004c5907  b814000000             -mov eax, 0x14
    cpu.eax = 20 /*0x14*/;
    // 004c590c  e8ff10feff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5911  e8ea7ef6ff             -call 0x42d800
    cpu.esp -= 4;
    nfs3hp::raceload::clearMessages(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5916  b804000000             -mov eax, 4
    cpu.eax = 4 /*0x4*/;
    // 004c591b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c591d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c591e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c591f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5920  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5921  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5922  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5923  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x004c5924:
    // 004c5924  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004c5926:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c5926  3b05a4fd5e00           +cmp eax, dword ptr [0x5efda4]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6225316) /* 0x5efda4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c592c  7d14                   -jge 0x4c5942
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c5942;
    }
    // 004c592e  8b148548fa5e00         -mov edx, dword ptr [eax*4 + 0x5efa48]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6224456) /* 0x5efa48 */ + cpu.eax * 4);
    // 004c5935  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c5936  c782c402000001000000   -mov dword ptr [edx + 0x2c4], 1
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(708) /* 0x2c4 */) = 1 /*0x1*/;
    // 004c5940  ebe4                   -jmp 0x4c5926
    goto L_0x004c5926;
L_0x004c5942:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c5942  ba0b000000             -mov edx, 0xb
    cpu.edx = 11 /*0xb*/;
    // 004c5947  b814000000             -mov eax, 0x14
    cpu.eax = 20 /*0x14*/;
    // 004c594c  e8bf10feff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5951  e8aa7ef6ff             -call 0x42d800
    cpu.esp -= 4;
    nfs3hp::raceload::clearMessages(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5956  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 004c595b  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c595d  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c595e  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c595f  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5960  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5961  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5962  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5963  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x004c5964:
    // 004c5964  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004c5966:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c5966  3b05a4fd5e00           +cmp eax, dword ptr [0x5efda4]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6225316) /* 0x5efda4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c596c  7d14                   -jge 0x4c5982
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c5982;
    }
    // 004c596e  8b148548fa5e00         -mov edx, dword ptr [eax*4 + 0x5efa48]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6224456) /* 0x5efa48 */ + cpu.eax * 4);
    // 004c5975  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c5976  c782c402000001000000   -mov dword ptr [edx + 0x2c4], 1
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(708) /* 0x2c4 */) = 1 /*0x1*/;
    // 004c5980  ebe4                   -jmp 0x4c5966
    goto L_0x004c5966;
L_0x004c5982:
    // 004c5982  ba0b000000             -mov edx, 0xb
    cpu.edx = 11 /*0xb*/;
    // 004c5987  b814000000             -mov eax, 0x14
    cpu.eax = 20 /*0x14*/;
    // 004c598c  be01000000             -mov esi, 1
    cpu.esi = 1 /*0x1*/;
    // 004c5991  e87a10feff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5996  e8657ef6ff             -call 0x42d800
    cpu.esp -= 4;
    nfs3hp::raceload::clearMessages(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c599b  b805000000             -mov eax, 5
    cpu.eax = 5 /*0x5*/;
    // 004c59a0  668935dc227a00         -mov word ptr [0x7a22dc], si
    app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */) = cpu.si;
    // 004c59a7  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c59a9  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c59aa  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c59ab  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c59ac  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c59ad  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c59ae  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c59af  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c59b0:
    // 004c59b0  83fe05                 +cmp esi, 5
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5 /*0x5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c59b3  0f84aa000000           -je 0x4c5a63
    if (cpu.flags.zf)
    {
        goto L_0x004c5a63;
    }
    // 004c59b9  66a19c227a00           -mov ax, word ptr [0x7a229c]
    cpu.ax = app->getMemory<x86::reg16>(x86::reg32(8004252) /* 0x7a229c */);
    // 004c59bf  663d0a00               +cmp ax, 0xa
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(10 /*0xa*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c59c3  7217                   -jb 0x4c59dc
    if (cpu.flags.cf)
    {
        goto L_0x004c59dc;
    }
    // 004c59c5  0f86f7feffff           -jbe 0x4c58c2
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004c58c2;
    }
    // 004c59cb  663d0b00               +cmp ax, 0xb
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(11 /*0xb*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c59cf  7670                   -jbe 0x4c5a41
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004c5a41;
    }
    // 004c59d1  663d2000               +cmp ax, 0x20
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(32 /*0x20*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c59d5  7448                   -je 0x4c5a1f
    if (cpu.flags.zf)
    {
        goto L_0x004c5a1f;
    }
    // 004c59d7  e987000000             -jmp 0x4c5a63
    goto L_0x004c5a63;
L_0x004c59dc:
    // 004c59dc  663d0900               +cmp ax, 9
    {
        x86::reg16 tmp1 = cpu.ax;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(9 /*0x9*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c59e0  0f857d000000           -jne 0x4c5a63
    if (!cpu.flags.zf)
    {
        goto L_0x004c5a63;
    }
    // 004c59e6  ba09000000             -mov edx, 9
    cpu.edx = 9 /*0x9*/;
    // 004c59eb  b814000000             -mov eax, 0x14
    cpu.eax = 20 /*0x14*/;
    // 004c59f0  e81b10feff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c59f5  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004c59f7  ba01000000             -mov edx, 1
    cpu.edx = 1 /*0x1*/;
    // 004c59fc  890d0c015600           -mov dword ptr [0x56010c], ecx
    app->getMemory<x86::reg32>(x86::reg32(5636364) /* 0x56010c */) = cpu.ecx;
    // 004c5a02  e8f97df6ff             -call 0x42d800
    cpu.esp -= 4;
    nfs3hp::raceload::clearMessages(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5a07  b814000000             -mov eax, 0x14
    cpu.eax = 20 /*0x14*/;
    // 004c5a0c  e8ff0ffeff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5a11  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c5a16  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c5a18  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5a19  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5a1a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5a1b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5a1c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5a1d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5a1e  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c5a1f:
    // 004c5a1f  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004c5a21:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c5a21  3b05a4fd5e00           +cmp eax, dword ptr [0x5efda4]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6225316) /* 0x5efda4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5a27  0f8dd5feffff           -jge 0x4c5902
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c5902;
    }
    // 004c5a2d  8b148548fa5e00         -mov edx, dword ptr [eax*4 + 0x5efa48]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6224456) /* 0x5efa48 */ + cpu.eax * 4);
    // 004c5a34  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c5a35  c782c402000001000000   -mov dword ptr [edx + 0x2c4], 1
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(708) /* 0x2c4 */) = 1 /*0x1*/;
    // 004c5a3f  ebe0                   -jmp 0x4c5a21
    goto L_0x004c5a21;
L_0x004c5a41:
    // 004c5a41  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004c5a43:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c5a43  3b05a4fd5e00           +cmp eax, dword ptr [0x5efda4]
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(6225316) /* 0x5efda4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5a49  0f8df3feffff           -jge 0x4c5942
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c5942;
    }
    // 004c5a4f  8b148548fa5e00         -mov edx, dword ptr [eax*4 + 0x5efa48]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(6224456) /* 0x5efa48 */ + cpu.eax * 4);
    // 004c5a56  40                     +inc eax
    {
        x86::reg32& tmp = cpu.eax;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c5a57  c782c402000001000000   -mov dword ptr [edx + 0x2c4], 1
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(708) /* 0x2c4 */) = 1 /*0x1*/;
    // 004c5a61  ebe0                   -jmp 0x4c5a43
    goto L_0x004c5a43;
L_0x004c5a63:
    // 004c5a63  83fe01                 +cmp esi, 1
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5a66  7519                   -jne 0x4c5a81
    if (!cpu.flags.zf)
    {
        goto L_0x004c5a81;
    }
    // 004c5a68  b408                   -mov ah, 8
    cpu.ah = 8 /*0x8*/;
    // 004c5a6a  bb0c000000             -mov ebx, 0xc
    cpu.ebx = 12 /*0xc*/;
    // 004c5a6f  8d55e8                 -lea edx, [ebp - 0x18]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004c5a72  8865e8                 -mov byte ptr [ebp - 0x18], ah
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.ah;
    // 004c5a75  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004c5a77  8975ec                 -mov dword ptr [ebp - 0x14], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.esi;
    // 004c5a7a  e86106feff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5a7f  eb58                   -jmp 0x4c5ad9
    goto L_0x004c5ad9;
L_0x004c5a81:
    // 004c5a81  83fe03                 +cmp esi, 3
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
    // 004c5a84  740a                   -je 0x4c5a90
    if (cpu.flags.zf)
    {
        goto L_0x004c5a90;
    }
    // 004c5a86  83fe04                 +cmp esi, 4
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
    // 004c5a89  7405                   -je 0x4c5a90
    if (cpu.flags.zf)
    {
        goto L_0x004c5a90;
    }
    // 004c5a8b  83fe05                 +cmp esi, 5
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5 /*0x5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5a8e  7549                   -jne 0x4c5ad9
    if (!cpu.flags.zf)
    {
        goto L_0x004c5ad9;
    }
L_0x004c5a90:
    // 004c5a90  b208                   -mov dl, 8
    cpu.dl = 8 /*0x8*/;
    // 004c5a92  b803000000             -mov eax, 3
    cpu.eax = 3 /*0x3*/;
    // 004c5a97  bb0c000000             -mov ebx, 0xc
    cpu.ebx = 12 /*0xc*/;
    // 004c5a9c  8855f4                 -mov byte ptr [ebp - 0xc], dl
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.dl;
    // 004c5a9f  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004c5aa2  8d55f4                 -lea edx, [ebp - 0xc]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004c5aa5  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c5aa7  e83406feff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5aac  8b15e4227a00           -mov edx, dword ptr [0x7a22e4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8004324) /* 0x7a22e4 */);
    // 004c5ab2  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004c5ab4  7505                   -jne 0x4c5abb
    if (!cpu.flags.zf)
    {
        goto L_0x004c5abb;
    }
    // 004c5ab6  83fe05                 +cmp esi, 5
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(5 /*0x5*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5ab9  751e                   -jne 0x4c5ad9
    if (!cpu.flags.zf)
    {
        goto L_0x004c5ad9;
    }
L_0x004c5abb:
    // 004c5abb  bf01000000             -mov edi, 1
    cpu.edi = 1 /*0x1*/;
    // 004c5ac0  89f0                   -mov eax, esi
    cpu.eax = cpu.esi;
    // 004c5ac2  66893dde227a00         -mov word ptr [0x7a22de], di
    app->getMemory<x86::reg16>(x86::reg32(8004318) /* 0x7a22de */) = cpu.di;
    // 004c5ac9  66893ddc227a00         -mov word ptr [0x7a22dc], di
    app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */) = cpu.di;
    // 004c5ad0  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c5ad2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ad3  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ad4  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ad5  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ad6  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ad7  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ad8  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c5ad9:
    // 004c5ad9  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004c5adb:
    // 004c5adb  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c5add  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ade  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5adf  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ae0  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ae1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ae2  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ae3  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4c5b00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  goto start;
dynamic_jump:
  switch(cpu.ip)
  {
start:
    // 004c5b00  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c5b01  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c5b02  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c5b03  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c5b04  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c5b06  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004c5b08  8b4204                 -mov eax, dword ptr [edx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 004c5b0b  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004c5b0d  83f803                 +cmp eax, 3
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
    // 004c5b10  0f87e0000000           -ja 0x4c5bf6
    if (!cpu.flags.cf && !cpu.flags.zf)
    {
        goto L_0x004c5bf6;
    }
    // 004c5b16  ff2485e45a4c00         -jmp dword ptr [eax*4 + 0x4c5ae4]
    cpu.ip = app->getMemory<x86::reg32>(5004004 + cpu.eax * 4); goto dynamic_jump;
  case 0x004c5b1d:
    // 004c5b1d  66833dda227a0000       +cmp word ptr [0x7a22da], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c5b25  7440                   -je 0x4c5b67
    if (cpu.flags.zf)
    {
        goto L_0x004c5b67;
    }
    // 004c5b27  8b15d8367d00           -mov edx, dword ptr [0x7d36d8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8206040) /* 0x7d36d8 */);
    // 004c5b2d  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c5b2e  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5b34  833dc8367d0000         +cmp dword ptr [0x7d36c8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8206024) /* 0x7d36c8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c5b3b  751b                   -jne 0x4c5b58
    if (!cpu.flags.zf)
    {
        goto L_0x004c5b58;
    }
    // 004c5b3d  e82e0e0000             -call 0x4c6970
    cpu.esp -= 4;
    nfs3hp::controls::syncFrame(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5b42  bb0c000000             -mov ebx, 0xc
    cpu.ebx = 12 /*0xc*/;
    // 004c5b47  a3c8367d00             -mov dword ptr [0x7d36c8], eax
    app->getMemory<x86::reg32>(x86::reg32(8206024) /* 0x7d36c8 */) = cpu.eax;
    // 004c5b4c  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004c5b4e  894608                 -mov dword ptr [esi + 8], eax
    app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004c5b51  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004c5b53  e88805feff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c5b58:
    // 004c5b58  8b1dd8367d00           -mov ebx, dword ptr [0x7d36d8]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8206040) /* 0x7d36d8 */);
    // 004c5b5e  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c5b5f  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5b65  eb08                   -jmp 0x4c5b6f
    goto L_0x004c5b6f;
L_0x004c5b67:
    // 004c5b67  8b4608                 -mov eax, dword ptr [esi + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 004c5b6a  a3c8367d00             -mov dword ptr [0x7d36c8], eax
    app->getMemory<x86::reg32>(x86::reg32(8206024) /* 0x7d36c8 */) = cpu.eax;
L_0x004c5b6f:
    // 004c5b6f  c705c0367d0001000000   -mov dword ptr [0x7d36c0], 1
    app->getMemory<x86::reg32>(x86::reg32(8206016) /* 0x7d36c0 */) = 1 /*0x1*/;
    // 004c5b79  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5b7a  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5b7b  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5b7c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5b7d  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x004c5b7e:
    // 004c5b7e  66833dda227a0000       +cmp word ptr [0x7a22da], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c5b86  740f                   -je 0x4c5b97
    if (cpu.flags.zf)
    {
        goto L_0x004c5b97;
    }
    // 004c5b88  c705c4367d0001000000   -mov dword ptr [0x7d36c4], 1
    app->getMemory<x86::reg32>(x86::reg32(8206020) /* 0x7d36c4 */) = 1 /*0x1*/;
    // 004c5b92  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5b93  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5b94  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5b95  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5b96  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c5b97:
    // 004c5b97  66c7059c227a000900     -mov word ptr [0x7a229c], 9
    app->getMemory<x86::reg16>(x86::reg32(8004252) /* 0x7a229c */) = 9 /*0x9*/;
    // 004c5ba0  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ba1  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ba2  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ba3  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5ba4  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x004c5ba5:
    // 004c5ba5  66c7059c227a000a00     -mov word ptr [0x7a229c], 0xa
    app->getMemory<x86::reg16>(x86::reg32(8004252) /* 0x7a229c */) = 10 /*0xa*/;
    // 004c5bae  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5baf  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5bb0  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5bb1  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5bb2  c3                     -ret 
    cpu.esp += 4;
    return;
  case 0x004c5bb3:
    // 004c5bb3  66833dda227a0000       +cmp word ptr [0x7a22da], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c5bbb  7430                   -je 0x4c5bed
    if (cpu.flags.zf)
    {
        goto L_0x004c5bed;
    }
    // 004c5bbd  a1b8ec5500             -mov eax, dword ptr [0x55ecb8]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004c5bc2  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004c5bc3  ff15b8445300           -call dword ptr [0x5344b8]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457080) /* 0x5344b8 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5bc9  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004c5bcb  e8800afeff             -call 0x4a6650
    cpu.esp -= 4;
    nfs3hp::ea::machineNumber(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5bd0  98                     -cwde 
    cpu.eax = x86::reg32(static_cast<x86::sreg16>(cpu.ax));
    // 004c5bd1  66c70445ba227a000b00   -mov word ptr [eax*2 + 0x7a22ba], 0xb
    app->getMemory<x86::reg16>(x86::reg32(8004282) /* 0x7a22ba */ + cpu.eax * 2) = 11 /*0xb*/;
    // 004c5bdb  8b15b8ec5500           -mov edx, dword ptr [0x55ecb8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5631160) /* 0x55ecb8 */);
    // 004c5be1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c5be2  ff1588455300           -call dword ptr [0x534588]
    cpu.ip = app->getMemory<x86::reg32>(x86::reg32(5457288) /* 0x534588 */);
    cpu.esp -= 4;
    app->dynamic_call(cpu.ip, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5be8  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5be9  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5bea  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5beb  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5bec  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c5bed:
    // 004c5bed  66c7059c227a000b00     -mov word ptr [0x7a229c], 0xb
    app->getMemory<x86::reg16>(x86::reg32(8004252) /* 0x7a229c */) = 11 /*0xb*/;
L_0x004c5bf6:
    // 004c5bf6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5bf7  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5bf8  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5bf9  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5bfa  c3                     -ret 
    cpu.esp += 4;
    return;
  default:
    NFS2_ASSERT(false);
  }
}

/* sub_4c5c00: native, nfs3hp::graphics::showWaitMessage */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 0x8b 0xdb */
void Application::sub_4c5e80(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c5e80  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c5e81  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c5e82  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c5e83  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c5e84  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c5e85  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c5e86  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c5e88  81ece4000000           -sub esp, 0xe4
    (cpu.esp) -= x86::reg32(x86::sreg32(228 /*0xe4*/));
    // 004c5e8e  81ed82000000           -sub ebp, 0x82
    (cpu.ebp) -= x86::reg32(x86::sreg32(130 /*0x82*/));
    // 004c5e94  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c5e96  b99cd36f00             -mov ecx, 0x6fd39c
    cpu.ecx = 7328668 /*0x6fd39c*/;
    // 004c5e9b  8915d0367d00           -mov dword ptr [0x7d36d0], edx
    app->getMemory<x86::reg32>(x86::reg32(8206032) /* 0x7d36d0 */) = cpu.edx;
    // 004c5ea1  668b15da227a00         -mov dx, word ptr [0x7a22da]
    cpu.dx = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
    // 004c5ea8  890dd4367d00           -mov dword ptr [0x7d36d4], ecx
    app->getMemory<x86::reg32>(x86::reg32(8206036) /* 0x7d36d4 */) = cpu.ecx;
    // 004c5eae  6685d2                 +test dx, dx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.dx & cpu.dx));
    // 004c5eb1  7548                   -jne 0x4c5efb
    if (!cpu.flags.zf)
    {
        goto L_0x004c5efb;
    }
    // 004c5eb3  bb10000000             -mov ebx, 0x10
    cpu.ebx = 16 /*0x10*/;
    // 004c5eb8  bac0060000             -mov edx, 0x6c0
    cpu.edx = 1728 /*0x6c0*/;
    // 004c5ebd  b814025400             -mov eax, 0x540214
    cpu.eax = 5505556 /*0x540214*/;
    // 004c5ec2  e859b70100             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5ec7  89457a                 -mov dword ptr [ebp + 0x7a], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */) = cpu.eax;
    // 004c5eca  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004c5ecc  eb05                   -jmp 0x4c5ed3
    goto L_0x004c5ed3;
L_0x004c5ece:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c5ece  83fa10                 +cmp edx, 0x10
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
    // 004c5ed1  7d28                   -jge 0x4c5efb
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c5efb;
    }
L_0x004c5ed3:
    // 004c5ed3  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c5eda  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c5edc  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004c5edf  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004c5ee1  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004c5ee4  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004c5ee6  8b7d7a                 -mov edi, dword ptr [ebp + 0x7a]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004c5ee9  b91b000000             -mov ecx, 0x1b
    cpu.ecx = 27 /*0x1b*/;
    // 004c5eee  8db02cd56f00           -lea esi, [eax + 0x6fd52c]
    cpu.esi = x86::reg32(cpu.eax + x86::reg32(7329068) /* 0x6fd52c */);
    // 004c5ef4  01c7                   +add edi, eax
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
    // 004c5ef6  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c5ef7  f3a5                   -rep movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c5ef9  ebd3                   -jmp 0x4c5ece
    goto L_0x004c5ece;
L_0x004c5efb:
    // 004c5efb  ba03000000             -mov edx, 3
    cpu.edx = 3 /*0x3*/;
    // 004c5f00  b85a000000             -mov eax, 0x5a
    cpu.eax = 90 /*0x5a*/;
    // 004c5f05  e8060bfeff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5f0a  66833dda227a0000       +cmp word ptr [0x7a22da], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c5f12  0f847c000000           -je 0x4c5f94
    if (cpu.flags.zf)
    {
        goto L_0x004c5f94;
    }
    // 004c5f18  b405                   -mov ah, 5
    cpu.ah = 5 /*0x5*/;
    // 004c5f1a  bbd0000000             -mov ebx, 0xd0
    cpu.ebx = 208 /*0xd0*/;
    // 004c5f1f  88659e                 -mov byte ptr [ebp - 0x62], ah
    app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(-98) /* -0x62 */) = cpu.ah;
    // 004c5f22  b82cdc6f00             -mov eax, 0x6fdc2c
    cpu.eax = 7330860 /*0x6fdc2c*/;
    // 004c5f27  895d72                 -mov dword ptr [ebp + 0x72], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */) = cpu.ebx;
    // 004c5f2a  2d9cd36f00             -sub eax, 0x6fd39c
    (cpu.eax) -= x86::reg32(x86::sreg32(7328668 /*0x6fd39c*/));
    // 004c5f2f  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c5f31  7f02                   -jg 0x4c5f35
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004c5f35;
    }
    // 004c5f33  f7d8                   -neg eax
    cpu.eax = ~cpu.eax + 1;
L_0x004c5f35:
    // 004c5f35  89457e                 -mov dword ptr [ebp + 0x7e], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.eax;
L_0x004c5f38:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c5f38  8b757e                 -mov esi, dword ptr [ebp + 0x7e]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004c5f3b  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004c5f3d  7e55                   -jle 0x4c5f94
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c5f94;
    }
    // 004c5f3f  8b7d72                 -mov edi, dword ptr [ebp + 0x72]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */);
    // 004c5f42  39fe                   +cmp esi, edi
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
    // 004c5f44  7d05                   -jge 0x4c5f4b
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c5f4b;
    }
    // 004c5f46  897576                 -mov dword ptr [ebp + 0x76], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */) = cpu.esi;
    // 004c5f49  eb03                   -jmp 0x4c5f4e
    goto L_0x004c5f4e;
L_0x004c5f4b:
    // 004c5f4b  897d76                 -mov dword ptr [ebp + 0x76], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */) = cpu.edi;
L_0x004c5f4e:
    // 004c5f4e  8b4d76                 -mov ecx, dword ptr [ebp + 0x76]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004c5f51  8d7d9f                 -lea edi, [ebp - 0x61]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-97) /* -0x61 */);
    // 004c5f54  8b5d76                 -mov ebx, dword ptr [ebp + 0x76]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004c5f57  8b35d4367d00           -mov esi, dword ptr [0x7d36d4]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8206036) /* 0x7d36d4 */);
    // 004c5f5d  8d559e                 -lea edx, [ebp - 0x62]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-98) /* -0x62 */);
    // 004c5f60  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c5f61  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004c5f63  c1e902                 -shr ecx, 2
    cpu.ecx >>= 2 /*0x2*/ % 32;
    // 004c5f66  f2a5                   -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c5f68  8ac8                   -mov cl, al
    cpu.cl = cpu.al;
    // 004c5f6a  80e103                 +and cl, 3
    cpu.clear_co();
    cpu.set_szp((cpu.cl &= x86::reg8(x86::sreg8(3 /*0x3*/))));
    // 004c5f6d  f2a4                   -repne movsb byte ptr es:[edi], byte ptr [esi]
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
    // 004c5f6f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c5f70  43                     -inc ebx
    (cpu.ebx)++;
    // 004c5f71  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c5f73  e86801feff             -call 0x4a60e0
    cpu.esp -= 4;
    nfs3hp::ea::sendPacket(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5f78  8b4576                 -mov eax, dword ptr [ebp + 0x76]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(118) /* 0x76 */);
    // 004c5f7b  8b15d4367d00           -mov edx, dword ptr [0x7d36d4]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(8206036) /* 0x7d36d4 */);
    // 004c5f81  8b4d7e                 -mov ecx, dword ptr [ebp + 0x7e]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */);
    // 004c5f84  01c2                   -add edx, eax
    (cpu.edx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004c5f86  2b4d72                 +sub ecx, dword ptr [ebp + 0x72]
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(114) /* 0x72 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004c5f89  8915d4367d00           -mov dword ptr [0x7d36d4], edx
    app->getMemory<x86::reg32>(x86::reg32(8206036) /* 0x7d36d4 */) = cpu.edx;
    // 004c5f8f  894d7e                 -mov dword ptr [ebp + 0x7e], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(126) /* 0x7e */) = cpu.ecx;
    // 004c5f92  eba4                   -jmp 0x4c5f38
    goto L_0x004c5f38;
L_0x004c5f94:
    // 004c5f94  ba04000000             -mov edx, 4
    cpu.edx = 4 /*0x4*/;
    // 004c5f99  b81e000000             -mov eax, 0x1e
    cpu.eax = 30 /*0x1e*/;
    // 004c5f9e  e86d0afeff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c5fa3  66833dda227a0000       +cmp word ptr [0x7a22da], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004314) /* 0x7a22da */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c5fab  0f8578000000           -jne 0x4c6029
    if (!cpu.flags.zf)
    {
        goto L_0x004c6029;
    }
    // 004c5fb1  31d2                   +xor edx, edx
    cpu.clear_co();
    cpu.set_szp((cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx))));
    // 004c5fb3  eb46                   -jmp 0x4c5ffb
    goto L_0x004c5ffb;
L_0x004c5fb5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c5fb5  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c5fbc  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c5fbe  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004c5fc1  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004c5fc3  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004c5fc6  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004c5fc8  8b4d7a                 -mov ecx, dword ptr [ebp + 0x7a]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004c5fcb  8b4c0114               -mov ecx, dword ptr [ecx + eax + 0x14]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(20) /* 0x14 */ + cpu.eax * 1);
    // 004c5fcf  898840d56f00           -mov dword ptr [eax + 0x6fd540], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7329088) /* 0x6fd540 */) = cpu.ecx;
L_0x004c5fd5:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c5fd5  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c5fdc  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c5fde  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004c5fe1  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004c5fe3  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004c5fe6  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004c5fe8  8b4d7a                 -mov ecx, dword ptr [ebp + 0x7a]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004c5feb  8b4c0168               -mov ecx, dword ptr [ecx + eax + 0x68]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(104) /* 0x68 */ + cpu.eax * 1);
    // 004c5fef  42                     -inc edx
    (cpu.edx)++;
    // 004c5ff0  898894d56f00           -mov dword ptr [eax + 0x6fd594], ecx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(7329172) /* 0x6fd594 */) = cpu.ecx;
    // 004c5ff6  83fa10                 +cmp edx, 0x10
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
    // 004c5ff9  7d26                   -jge 0x4c6021
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c6021;
    }
L_0x004c5ffb:
    // 004c5ffb  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c6002  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c6004  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004c6007  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004c6009  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004c600c  8b5d7a                 -mov ebx, dword ptr [ebp + 0x7a]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004c600f  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004c6011  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004c6013  8b7014                 -mov esi, dword ptr [eax + 0x14]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(20) /* 0x14 */);
    // 004c6016  85f6                   +test esi, esi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.esi & cpu.esi));
    // 004c6018  7cbb                   -jl 0x4c5fd5
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c5fd5;
    }
    // 004c601a  83fe31                 +cmp esi, 0x31
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(49 /*0x31*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c601d  7e96                   -jle 0x4c5fb5
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c5fb5;
    }
    // 004c601f  ebb4                   -jmp 0x4c5fd5
    goto L_0x004c5fd5;
L_0x004c6021:
    // 004c6021  8b457a                 -mov eax, dword ptr [ebp + 0x7a]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(122) /* 0x7a */);
    // 004c6024  e867b80100             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c6029:
    // 004c6029  a176227a00             -mov eax, dword ptr [0x7a2276]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8004214) /* 0x7a2276 */);
    // 004c602e  c1f810                 -sar eax, 0x10
    cpu.eax = x86::reg32(x86::sreg32(cpu.eax) >> (16 /*0x10*/ % 32));
    // 004c6031  a3f4d46f00             -mov dword ptr [0x6fd4f4], eax
    app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */) = cpu.eax;
    // 004c6036  a3f8d46f00             -mov dword ptr [0x6fd4f8], eax
    app->getMemory<x86::reg32>(x86::reg32(7329016) /* 0x6fd4f8 */) = cpu.eax;
    // 004c603b  833d0cd56f0000         +cmp dword ptr [0x6fd50c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7329036) /* 0x6fd50c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c6042  742e                   -je 0x4c6072
    if (cpu.flags.zf)
    {
        goto L_0x004c6072;
    }
    // 004c6044  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
L_0x004c6046:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c6046  3b1514d56f00           +cmp edx, dword ptr [0x6fd514]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329044) /* 0x6fd514 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c604c  7d24                   -jge 0x4c6072
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c6072;
    }
    // 004c604e  3b15f4d46f00           +cmp edx, dword ptr [0x6fd4f4]
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(x86::reg32(7329012) /* 0x6fd4f4 */)));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c6054  7419                   -je 0x4c606f
    if (cpu.flags.zf)
    {
        goto L_0x004c606f;
    }
    // 004c6056  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c605d  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c605f  c1e002                 +shl eax, 2
    {
        x86::reg8 tmp = 2 /*0x2*/ % 32;
        x86::reg32& op = cpu.eax;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004c6062  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004c6064  c784c148d56f0001000000 -mov dword ptr [ecx + eax*8 + 0x6fd548], 1
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(7329096) /* 0x6fd548 */ + cpu.eax * 8) = 1 /*0x1*/;
L_0x004c606f:
    // 004c606f  42                     +inc edx
    {
        x86::reg32& tmp = cpu.edx;
        cpu.flags.of = ~(1 & (tmp >> 31));
        tmp++;
        cpu.flags.of &= 1 & (tmp >> 31);
        cpu.set_szp(tmp);
    }
    // 004c6070  ebd4                   -jmp 0x4c6046
    goto L_0x004c6046;
L_0x004c6072:
    // 004c6072  8da582000000           -lea esp, [ebp + 0x82]
    cpu.esp = x86::reg32(cpu.ebp + x86::reg32(130) /* 0x82 */);
    // 004c6078  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6079  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c607a  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c607b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c607c  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c607d  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c607e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4c6080: native, nfs3hp::controls::appendToPacket */

/* sub_4c60c0: native, nfs3hp::controls::sendOwnInput */

/* sub_4c61e0: native, nfs3hp::controls::receiveInput */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4c62a0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c62a0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c62a1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c62a2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c62a3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c62a5  8b15b0d36f00           -mov edx, dword ptr [0x6fd3b0]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(7328688) /* 0x6fd3b0 */);
    // 004c62ab  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004c62ad  7420                   -je 0x4c62cf
    if (cpu.flags.zf)
    {
        goto L_0x004c62cf;
    }
    // 004c62af  83fa01                 +cmp edx, 1
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
    // 004c62b2  741b                   -je 0x4c62cf
    if (cpu.flags.zf)
    {
        goto L_0x004c62cf;
    }
    // 004c62b4  66833ddc227a0000       +cmp word ptr [0x7a22dc], 0
    {
        x86::reg16 tmp1 = app->getMemory<x86::reg16>(x86::reg32(8004316) /* 0x7a22dc */);
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(0 /*0x0*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c62bc  7411                   -je 0x4c62cf
    if (cpu.flags.zf)
    {
        goto L_0x004c62cf;
    }
    // 004c62be  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004c62c3  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004c62c5  891d8ce85500           -mov dword ptr [0x55e88c], ebx
    app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */) = cpu.ebx;
    // 004c62cb  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c62cc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c62cd  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c62ce  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c62cf:
    // 004c62cf  e8cca7f4ff             -call 0x410aa0
    cpu.esp -= 4;
    nfs3hp::port::musicStreamEnd(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c62d4  e8f7d0fdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c62d9  b864000000             -mov eax, 0x64
    cpu.eax = 100 /*0x64*/;
    // 004c62de  e88da0f4ff             -call 0x410370
    cpu.esp -= 4;
    nfs3hp::port::musicStreamMakeOf(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c62e3  e8e8d0fdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c62e8  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c62ef  7409                   -je 0x4c62fa
    if (cpu.flags.zf)
    {
        goto L_0x004c62fa;
    }
    // 004c62f1  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c62f6  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c62f7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c62f8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c62f9  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c62fa:
    // 004c62fa  e8b179f6ff             -call 0x42dcb0
    cpu.esp -= 4;
    nfs3hp::raceload::startRaceControlsTick(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c62ff  e8ccd0fdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6304  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c630b  7409                   -je 0x4c6316
    if (cpu.flags.zf)
    {
        goto L_0x004c6316;
    }
    // 004c630d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c6312  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6313  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6314  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6315  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c6316:
    // 004c6316  e8a5d8f4ff             -call 0x413bc0
    cpu.esp -= 4;
    nfs3hp::port::raceBanksLoad(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c631b  e8b0d0fdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6320  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c6327  7409                   -je 0x4c6332
    if (cpu.flags.zf)
    {
        goto L_0x004c6332;
    }
    // 004c6329  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c632e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c632f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6330  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6331  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c6332:
    // 004c6332  e8b9e2ffff             -call 0x4c45f0
    cpu.esp -= 4;
    sub_4c45f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6337  e894d0fdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c633c  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c6343  7409                   -je 0x4c634e
    if (cpu.flags.zf)
    {
        goto L_0x004c634e;
    }
    // 004c6345  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c634a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c634b  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c634c  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c634d  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c634e:
    // 004c634e  e88d65ffff             -call 0x4bc8e0
    cpu.esp -= 4;
    sub_4bc8e0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6353  e878d0fdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6358  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c635f  7409                   -je 0x4c636a
    if (cpu.flags.zf)
    {
        goto L_0x004c636a;
    }
    // 004c6361  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c6366  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6367  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6368  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6369  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c636a:
    // 004c636a  e8f14c0300             -call 0x4fb060
    cpu.esp -= 4;
    sub_4fb060(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c636f  e85cd0fdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6374  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c637b  7409                   -je 0x4c6386
    if (cpu.flags.zf)
    {
        goto L_0x004c6386;
    }
    // 004c637d  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c6382  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6383  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6384  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6385  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c6386:
    // 004c6386  833db0d36f0001         +cmp dword ptr [0x6fd3b0], 1
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
    // 004c638d  7e05                   -jle 0x4c6394
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c6394;
    }
    // 004c638f  e86cf8ffff             -call 0x4c5c00
    cpu.esp -= 4;
    nfs3hp::graphics::showWaitMessage(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c6394:
    // 004c6394  e827000000             -call 0x4c63c0
    cpu.esp -= 4;
    sub_4c63c0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6399  e832d0fdff             -call 0x4a33d0
    cpu.esp -= 4;
    sub_4a33d0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c639e  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c63a5  7409                   -je 0x4c63b0
    if (cpu.flags.zf)
    {
        goto L_0x004c63b0;
    }
    // 004c63a7  b801000000             -mov eax, 1
    cpu.eax = 1 /*0x1*/;
    // 004c63ac  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c63ad  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c63ae  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c63af  c3                     -ret 
    cpu.esp += 4;
    return;
L_0x004c63b0:
    // 004c63b0  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c63b2  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c63b3  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c63b4  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c63b5  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4c63c0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c63c0  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c63c1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c63c2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c63c4  833da0d36f0001         +cmp dword ptr [0x6fd3a0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c63cb  7f20                   -jg 0x4c63ed
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004c63ed;
    }
    // 004c63cd  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
    // 004c63d2  890d08015600           -mov dword ptr [0x560108], ecx
    app->getMemory<x86::reg32>(x86::reg32(5636360) /* 0x560108 */) = cpu.ecx;
    // 004c63d8  890d0c015600           -mov dword ptr [0x56010c], ecx
    app->getMemory<x86::reg32>(x86::reg32(5636364) /* 0x56010c */) = cpu.ecx;
    // 004c63de  e84d170000             -call 0x4c7b30
    cpu.esp -= 4;
    sub_4c7b30(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c63e3  e838f1ffff             -call 0x4c5520
    cpu.esp -= 4;
    sub_4c5520(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c63e8  e853060000             -call 0x4c6a40
    cpu.esp -= 4;
    nfs3hp::controls::setQueueDelays(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c63ed:
    // 004c63ed  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c63ee  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c63ef  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip  */
void Application::sub_4c63f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c63f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c63f1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c63f2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c63f3  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c63f4  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c63f5  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c63f6  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c63f8  833da0d36f0001         +cmp dword ptr [0x6fd3a0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c63ff  7e0d                   -jle 0x4c640e
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c640e;
    }
    // 004c6401  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004c6403  890d04015600           -mov dword ptr [0x560104], ecx
    app->getMemory<x86::reg32>(x86::reg32(5636356) /* 0x560104 */) = cpu.ecx;
    // 004c6409  e9c5000000             -jmp 0x4c64d3
    goto L_0x004c64d3;
L_0x004c640e:
    // 004c640e  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004c6413  ba05000000             -mov edx, 5
    cpu.edx = 5 /*0x5*/;
    // 004c6418  b85a000000             -mov eax, 0x5a
    cpu.eax = 90 /*0x5a*/;
    // 004c641d  891d08015600           -mov dword ptr [0x560108], ebx
    app->getMemory<x86::reg32>(x86::reg32(5636360) /* 0x560108 */) = cpu.ebx;
    // 004c6423  891d0c015600           -mov dword ptr [0x56010c], ebx
    app->getMemory<x86::reg32>(x86::reg32(5636364) /* 0x56010c */) = cpu.ebx;
    // 004c6429  e8e205feff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c642e  e81d170000             -call 0x4c7b50
    cpu.esp -= 4;
    sub_4c7b50(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6433  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c6435  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004c6437  eb39                   -jmp 0x4c6472
    goto L_0x004c6472;
L_0x004c6439:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c6439  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004c643b  898c8314477d00         -mov dword ptr [ebx + eax*4 + 0x7d4714], ecx
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8210196) /* 0x7d4714 */ + cpu.eax * 4) = cpu.ecx;
L_0x004c6442:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c6442  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 004c6444  40                     -inc eax
    (cpu.eax)++;
    // 004c6445  c1e309                 -shl ebx, 9
    cpu.ebx <<= 9 /*0x9*/ % 32;
    // 004c6448  30c9                   -xor cl, cl
    cpu.cl ^= x86::reg8(x86::sreg8(cpu.cl));
    // 004c644a  888c8310377d00         -mov byte ptr [ebx + eax*4 + 0x7d3710], cl
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(8206096) /* 0x7d3710 */ + cpu.eax * 4) = cpu.cl;
    // 004c6451  888c8311377d00         -mov byte ptr [ebx + eax*4 + 0x7d3711], cl
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(8206097) /* 0x7d3711 */ + cpu.eax * 4) = cpu.cl;
    // 004c6458  888c8312377d00         -mov byte ptr [ebx + eax*4 + 0x7d3712], cl
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(8206098) /* 0x7d3712 */ + cpu.eax * 4) = cpu.cl;
    // 004c645f  888c8313377d00         -mov byte ptr [ebx + eax*4 + 0x7d3713], cl
    app->getMemory<x86::reg8>(cpu.ebx + x86::reg32(8206099) /* 0x7d3713 */ + cpu.eax * 4) = cpu.cl;
    // 004c6466  3d80000000             +cmp eax, 0x80
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(128 /*0x80*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c646b  7d17                   -jge 0x4c6484
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c6484;
    }
    // 004c646d  83f804                 +cmp eax, 4
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
    // 004c6470  7dc7                   -jge 0x4c6439
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c6439;
    }
L_0x004c6472:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c6472  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 004c6474  c1e309                 +shl ebx, 9
    {
        x86::reg8 tmp = 9 /*0x9*/ % 32;
        x86::reg32& op = cpu.ebx;
        if (tmp)
        {
            cpu.flags.cf = 1 & (op >> (32 - tmp));
            cpu.set_szp((op <<= tmp));
            if (tmp == 1) cpu.flags.of = (cpu.flags.cf != (1 & (op >> (32 - 1))));
        }
    }
    // 004c6477  c7848314477d0001000000 -mov dword ptr [ebx + eax*4 + 0x7d4714], 1
    app->getMemory<x86::reg32>(cpu.ebx + x86::reg32(8210196) /* 0x7d4714 */ + cpu.eax * 4) = 1 /*0x1*/;
    // 004c6482  ebbe                   -jmp 0x4c6442
    goto L_0x004c6442;
L_0x004c6484:
    // 004c6484  42                     -inc edx
    (cpu.edx)++;
    // 004c6485  c7049514577d0004000000 -mov dword ptr [edx*4 + 0x7d5714], 4
    app->getMemory<x86::reg32>(x86::reg32(8214292) /* 0x7d5714 */ + cpu.edx * 4) = 4 /*0x4*/;
    // 004c6490  83fa08                 +cmp edx, 8
    {
        x86::reg32 tmp1 = cpu.edx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(8 /*0x8*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c6493  7d04                   -jge 0x4c6499
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c6499;
    }
    // 004c6495  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004c6497  ebd9                   -jmp 0x4c6472
    goto L_0x004c6472;
L_0x004c6499:
    // 004c6499  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004c649b  bf03000000             -mov edi, 3
    cpu.edi = 3 /*0x3*/;
    // 004c64a0  893514577d00           -mov dword ptr [0x7d5714], esi
    app->getMemory<x86::reg32>(x86::reg32(8214292) /* 0x7d5714 */) = cpu.esi;
    // 004c64a6  893d04015600           -mov dword ptr [0x560104], edi
    app->getMemory<x86::reg32>(x86::reg32(5636356) /* 0x560104 */) = cpu.edi;
    // 004c64ac  e88ff0ffff             -call 0x4c5540
    cpu.esp -= 4;
    sub_4c5540(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c64b1  30c0                   -xor al, al
    cpu.al ^= x86::reg8(x86::sreg8(cpu.al));
    // 004c64b3  ba06000000             -mov edx, 6
    cpu.edx = 6 /*0x6*/;
    // 004c64b8  a240577d00             -mov byte ptr [0x7d5740], al
    app->getMemory<x86::reg8>(x86::reg32(8214336) /* 0x7d5740 */) = cpu.al;
    // 004c64bd  b83c000000             -mov eax, 0x3c
    cpu.eax = 60 /*0x3c*/;
    // 004c64c2  89350c015600           -mov dword ptr [0x56010c], esi
    app->getMemory<x86::reg32>(x86::reg32(5636364) /* 0x56010c */) = cpu.esi;
    // 004c64c8  e84305feff             -call 0x4a6a10
    cpu.esp -= 4;
    sub_4a6a10(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c64cd  893508015600           -mov dword ptr [0x560108], esi
    app->getMemory<x86::reg32>(x86::reg32(5636360) /* 0x560108 */) = cpu.esi;
L_0x004c64d3:
    // 004c64d3  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c64d4  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c64d5  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c64d6  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c64d7  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c64d8  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c64d9  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4c64e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c64e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c64e1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c64e2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c64e4  833d8ce8550000         +cmp dword ptr [0x55e88c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(5630092) /* 0x55e88c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c64eb  7524                   -jne 0x4c6511
    if (!cpu.flags.zf)
    {
        goto L_0x004c6511;
    }
    // 004c64ed  833da0d36f0001         +cmp dword ptr [0x6fd3a0], 1
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(7328672) /* 0x6fd3a0 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1 /*0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c64f4  7f1b                   -jg 0x4c6511
    if (!cpu.flags.zf && (cpu.flags.sf == cpu.flags.of))
    {
        goto L_0x004c6511;
    }
    // 004c64f6  bb01000000             -mov ebx, 1
    cpu.ebx = 1 /*0x1*/;
    // 004c64fb  891d08015600           -mov dword ptr [0x560108], ebx
    app->getMemory<x86::reg32>(x86::reg32(5636360) /* 0x560108 */) = cpu.ebx;
    // 004c6501  891d0c015600           -mov dword ptr [0x56010c], ebx
    app->getMemory<x86::reg32>(x86::reg32(5636364) /* 0x56010c */) = cpu.ebx;
    // 004c6507  e8a4160000             -call 0x4c7bb0
    cpu.esp -= 4;
    sub_4c7bb0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c650c  e84ff0ffff             -call 0x4c5560
    cpu.esp -= 4;
    sub_4c5560(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c6511:
    // 004c6511  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6512  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6513  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4c6530: native, nfs3hp::controls::queueInput */

/* sub_4c66a0: native, nfs3hp::controls::queueRemoteInput */

/* sub_4c67a0: native, nfs3hp::controls::takeQueuedFrame */

/* sub_4c6840: native, nfs3hp::controls::takenControls */

/* sub_4c6870: native, nfs3hp::controls::repeatMissingInputs */

/* sub_4c6970: native, nfs3hp::controls::syncFrame */

/* sub_4c69a0: native, nfs3hp::controls::framesTaken */

/* sub_4c69b0: native, nfs3hp::controls::framesQueued */

/* sub_4c69c0: native, nfs3hp::controls::queueLead */

/* sub_4c6a10: native, nfs3hp::controls::noteLag */

/* sub_4c6a40: native, nfs3hp::controls::setQueueDelays */

/* sub_4c6ae0: native, nfs3hp::controls::queueFull */

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4c6c70(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c6c70  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c6c71  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c6c72  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c6c73  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c6c75  89c7                   -mov edi, eax
    cpu.edi = cpu.eax;
    // 004c6c77  89de                   -mov esi, ebx
    cpu.esi = cpu.ebx;
    // 004c6c79  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c6c7a  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c6c7b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c6c7c  8d780c                 -lea edi, [eax + 0xc]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004c6c7f  8d730c                 -lea esi, [ebx + 0xc]
    cpu.esi = x86::reg32(cpu.ebx + x86::reg32(12) /* 0xc */);
    // 004c6c82  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c6c83  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c6c84  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c6c85  8d7818                 -lea edi, [eax + 0x18]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(24) /* 0x18 */);
    // 004c6c88  8d710c                 -lea esi, [ecx + 0xc]
    cpu.esi = x86::reg32(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 004c6c8b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c6c8c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c6c8d  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c6c8e  89ce                   -mov esi, ecx
    cpu.esi = cpu.ecx;
    // 004c6c90  8d7824                 -lea edi, [eax + 0x24]
    cpu.edi = x86::reg32(cpu.eax + x86::reg32(36) /* 0x24 */);
    // 004c6c93  8a4510                 -mov al, byte ptr [ebp + 0x10]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004c6c96  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c6c97  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c6c98  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c6c99  8802                   -mov byte ptr [edx], al
    app->getMemory<x86::reg8>(cpu.edx) = cpu.al;
    // 004c6c9b  8a4514                 -mov al, byte ptr [ebp + 0x14]
    cpu.al = app->getMemory<x86::reg8>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004c6c9e  884201                 -mov byte ptr [edx + 1], al
    app->getMemory<x86::reg8>(cpu.edx + x86::reg32(1) /* 0x1 */) = cpu.al;
    // 004c6ca1  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6ca2  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6ca3  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6ca4  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4c6cb0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0, fp1;
  x86::reg16 fsw = 0;
    // 004c6cb0  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c6cb1  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c6cb2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c6cb3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c6cb5  83ec4c                 -sub esp, 0x4c
    (cpu.esp) -= x86::reg32(x86::sreg32(76 /*0x4c*/));
    // 004c6cb8  89c6                   -mov esi, eax
    cpu.esi = cpu.eax;
    // 004c6cba  88cf                   -mov bh, cl
    cpu.bh = cpu.cl;
    // 004c6cbc  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004c6cbe  8945b4                 -mov dword ptr [ebp - 0x4c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-76) /* -0x4c */) = cpu.eax;
    // 004c6cc1  d90518025400           -fld dword ptr [0x540218]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(5505560) /* 0x540218 */));
    // 004c6cc7  d94204                 -fld dword ptr [edx + 4]
    fp1 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */));
    // 004c6cca  d8c1                   -fadd st(1)
    fp1 = fp::add(fp1, x86::Float(fp0));
    // 004c6ccc  d95db8                 -fstp dword ptr [ebp - 0x48]
    app->getMemory<float>(cpu.ebp + x86::reg32(-72) /* -0x48 */) = float(fp1);
    // 004c6ccf  8b4208                 -mov eax, dword ptr [edx + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 004c6cd2  8945bc                 -mov dword ptr [ebp - 0x44], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-68) /* -0x44 */) = cpu.eax;
    // 004c6cd5  8b420c                 -mov eax, dword ptr [edx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
    // 004c6cd8  8945c0                 -mov dword ptr [ebp - 0x40], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-64) /* -0x40 */) = cpu.eax;
    // 004c6cdb  d94210                 -fld dword ptr [edx + 0x10]
    fp1 = x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(16) /* 0x10 */));
    // 004c6cde  d8c1                   -fadd st(1)
    fp1 = fp::add(fp1, x86::Float(fp0));
    // 004c6ce0  d95dc4                 -fstp dword ptr [ebp - 0x3c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-60) /* -0x3c */) = float(fp1);
    // 004c6ce3  8b4214                 -mov eax, dword ptr [edx + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */);
    // 004c6ce6  8945c8                 -mov dword ptr [ebp - 0x38], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-56) /* -0x38 */) = cpu.eax;
    // 004c6ce9  8b06                   -mov eax, dword ptr [esi]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi);
    // 004c6ceb  8945cc                 -mov dword ptr [ebp - 0x34], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-52) /* -0x34 */) = cpu.eax;
    // 004c6cee  d94604                 -fld dword ptr [esi + 4]
    fp1 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(4) /* 0x4 */));
    // 004c6cf1  d8c1                   -fadd st(1)
    fp1 = fp::add(fp1, x86::Float(fp0));
    // 004c6cf3  d95dd0                 -fstp dword ptr [ebp - 0x30]
    app->getMemory<float>(cpu.ebp + x86::reg32(-48) /* -0x30 */) = float(fp1);
    // 004c6cf6  8b4608                 -mov eax, dword ptr [esi + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(8) /* 0x8 */);
    // 004c6cf9  8945d4                 -mov dword ptr [ebp - 0x2c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-44) /* -0x2c */) = cpu.eax;
    // 004c6cfc  8b460c                 -mov eax, dword ptr [esi + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(12) /* 0xc */);
    // 004c6cff  8945d8                 -mov dword ptr [ebp - 0x28], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-40) /* -0x28 */) = cpu.eax;
    // 004c6d02  d84610                 -fadd dword ptr [esi + 0x10]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(16) /* 0x10 */)));
    // 004c6d05  d95ddc                 -fstp dword ptr [ebp - 0x24]
    app->getMemory<float>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = float(fp0);
    // 004c6d08  8b4614                 -mov eax, dword ptr [esi + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(20) /* 0x14 */);
    // 004c6d0b  8b4d10                 -mov ecx, dword ptr [ebp + 0x10]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
    // 004c6d0e  8945e0                 -mov dword ptr [ebp - 0x20], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.eax;
    // 004c6d11  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004c6d13  0f84d0010000           -je 0x4c6ee9
    if (cpu.flags.zf)
    {
        goto L_0x004c6ee9;
    }
    // 004c6d19  8b0d54577d00           -mov ecx, dword ptr [0x7d5754]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8214356) /* 0x7d5754 */);
    // 004c6d1f  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 004c6d26  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004c6d28  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004c6d2b  01c8                   -add eax, ecx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ecx));
    // 004c6d2d  8b0d10015600           -mov ecx, dword ptr [0x560110]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5636368) /* 0x560110 */);
    // 004c6d33  c1e006                 -shl eax, 6
    cpu.eax <<= 6 /*0x6*/ % 32;
    // 004c6d36  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004c6d38  894dfc                 -mov dword ptr [ebp - 4], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.ecx;
    // 004c6d3b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c6d3e  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004c6d40  8b700c                 -mov esi, dword ptr [eax + 0xc]
    cpu.esi = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004c6d43  83fe10                 +cmp esi, 0x10
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
    // 004c6d46  0f84aa000000           -je 0x4c6df6
    if (cpu.flags.zf)
    {
        goto L_0x004c6df6;
    }
    // 004c6d4c  d900                   +fld dword ptr [eax]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax));
    // 004c6d4e  d822                   +fsub dword ptr [edx]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 004c6d50  d9ee                   +fldz 
    fp1 = 0.0;
    // 004c6d52  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004c6d54  d95df0                 +fstp dword ptr [ebp - 0x10]
    app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = float(fp1);
    // 004c6d57  d85df0                 +fcomp dword ptr [ebp - 0x10]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */)));
    // 004c6d5a  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004c6d5c  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004c6d5d  760a                   -jbe 0x4c6d69
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004c6d69;
    }
    // 004c6d5f  d945f0                 +fld dword ptr [ebp - 0x10]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-16) /* -0x10 */));
    // 004c6d62  d9e0                   +fchs 
    fp0 = -fp0;
    // 004c6d64  d95dec                 +fstp dword ptr [ebp - 0x14]
    app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = float(fp0);
    // 004c6d67  eb06                   -jmp 0x4c6d6f
    goto L_0x004c6d6f;
L_0x004c6d69:
    // 004c6d69  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004c6d6c  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
L_0x004c6d6f:
    // 004c6d6f  d945ec                 +fld dword ptr [ebp - 0x14]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-20) /* -0x14 */));
    // 004c6d72  d81d1c025400           +fcomp dword ptr [0x54021c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(5505564) /* 0x54021c */)));
    // 004c6d78  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004c6d7a  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004c6d7b  0f836c000000           -jae 0x4c6ded
    if (!cpu.flags.cf)
    {
        goto L_0x004c6ded;
    }
    // 004c6d81  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c6d84  d94004                 +fld dword ptr [eax + 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(4) /* 0x4 */));
    // 004c6d87  d86204                 +fsub dword ptr [edx + 4]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(4) /* 0x4 */)));
    // 004c6d8a  d9ee                   +fldz 
    fp1 = 0.0;
    // 004c6d8c  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004c6d8e  d95df4                 +fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp1);
    // 004c6d91  d85df4                 +fcomp dword ptr [ebp - 0xc]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 004c6d94  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004c6d96  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004c6d97  760a                   -jbe 0x4c6da3
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004c6da3;
    }
    // 004c6d99  d945f4                 +fld dword ptr [ebp - 0xc]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */));
    // 004c6d9c  d9e0                   +fchs 
    fp0 = -fp0;
    // 004c6d9e  d95de4                 +fstp dword ptr [ebp - 0x1c]
    app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = float(fp0);
    // 004c6da1  eb06                   -jmp 0x4c6da9
    goto L_0x004c6da9;
L_0x004c6da3:
    // 004c6da3  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004c6da6  8945e4                 -mov dword ptr [ebp - 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */) = cpu.eax;
L_0x004c6da9:
    // 004c6da9  d945e4                 +fld dword ptr [ebp - 0x1c]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-28) /* -0x1c */));
    // 004c6dac  d81d1c025400           +fcomp dword ptr [0x54021c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(5505564) /* 0x54021c */)));
    // 004c6db2  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004c6db4  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004c6db5  7336                   -jae 0x4c6ded
    if (!cpu.flags.cf)
    {
        goto L_0x004c6ded;
    }
    // 004c6db7  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c6dba  d94008                 +fld dword ptr [eax + 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004c6dbd  d86208                 +fsub dword ptr [edx + 8]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.edx + x86::reg32(8) /* 0x8 */)));
    // 004c6dc0  d9ee                   +fldz 
    fp1 = 0.0;
    // 004c6dc2  d9c9                   +fxch st(1)
    {
        x86::Float tmp = fp1;
        fp1 = fp0;
        fp0 = tmp;
    }
    // 004c6dc4  d95df8                 +fstp dword ptr [ebp - 8]
    app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = float(fp1);
    // 004c6dc7  d85df8                 +fcomp dword ptr [ebp - 8]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004c6dca  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004c6dcc  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004c6dcd  760a                   -jbe 0x4c6dd9
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004c6dd9;
    }
    // 004c6dcf  d945f8                 +fld dword ptr [ebp - 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 004c6dd2  d9e0                   +fchs 
    fp0 = -fp0;
    // 004c6dd4  d95de8                 +fstp dword ptr [ebp - 0x18]
    app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = float(fp0);
    // 004c6dd7  eb06                   -jmp 0x4c6ddf
    goto L_0x004c6ddf;
L_0x004c6dd9:
    // 004c6dd9  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c6ddc  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
L_0x004c6ddf:
    // 004c6ddf  d945e8                 +fld dword ptr [ebp - 0x18]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-24) /* -0x18 */));
    // 004c6de2  d81d1c025400           +fcomp dword ptr [0x54021c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(x86::reg32(5505564) /* 0x54021c */)));
    // 004c6de8  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004c6dea  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004c6deb  7205                   -jb 0x4c6df2
    if (cpu.flags.cf)
    {
        goto L_0x004c6df2;
    }
L_0x004c6ded:
    // 004c6ded  b901000000             -mov ecx, 1
    cpu.ecx = 1 /*0x1*/;
L_0x004c6df2:
    // 004c6df2  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004c6df4  7462                   -je 0x4c6e58
    if (cpu.flags.zf)
    {
        goto L_0x004c6e58;
    }
L_0x004c6df6:
    // 004c6df6  8b3d58577d00           -mov edi, dword ptr [0x7d5758]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(8214360) /* 0x7d5758 */);
    // 004c6dfc  83ff40                 +cmp edi, 0x40
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(64 /*0x40*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c6dff  7d10                   -jge 0x4c6e11
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c6e11;
    }
    // 004c6e01  893d54577d00           -mov dword ptr [0x7d5754], edi
    app->getMemory<x86::reg32>(x86::reg32(8214356) /* 0x7d5754 */) = cpu.edi;
    // 004c6e07  8d4701                 -lea eax, [edi + 1]
    cpu.eax = x86::reg32(cpu.edi + x86::reg32(1) /* 0x1 */);
    // 004c6e0a  a358577d00             -mov dword ptr [0x7d5758], eax
    app->getMemory<x86::reg32>(x86::reg32(8214360) /* 0x7d5758 */) = cpu.eax;
    // 004c6e0f  eb1d                   -jmp 0x4c6e2e
    goto L_0x004c6e2e;
L_0x004c6e11:
    // 004c6e11  a154577d00             -mov eax, dword ptr [0x7d5754]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8214356) /* 0x7d5754 */);
    // 004c6e16  83f83f                 +cmp eax, 0x3f
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
    // 004c6e19  7d0b                   -jge 0x4c6e26
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c6e26;
    }
    // 004c6e1b  8d4801                 -lea ecx, [eax + 1]
    cpu.ecx = x86::reg32(cpu.eax + x86::reg32(1) /* 0x1 */);
    // 004c6e1e  890d54577d00           -mov dword ptr [0x7d5754], ecx
    app->getMemory<x86::reg32>(x86::reg32(8214356) /* 0x7d5754 */) = cpu.ecx;
    // 004c6e24  eb08                   -jmp 0x4c6e2e
    goto L_0x004c6e2e;
L_0x004c6e26:
    // 004c6e26  31f6                   -xor esi, esi
    cpu.esi ^= x86::reg32(x86::sreg32(cpu.esi));
    // 004c6e28  893554577d00           -mov dword ptr [0x7d5754], esi
    app->getMemory<x86::reg32>(x86::reg32(8214356) /* 0x7d5754 */) = cpu.esi;
L_0x004c6e2e:
    // 004c6e2e  8b0d54577d00           -mov ecx, dword ptr [0x7d5754]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8214356) /* 0x7d5754 */);
    // 004c6e34  8d048d00000000         -lea eax, [ecx*4]
    cpu.eax = x86::reg32(cpu.ecx * 4);
    // 004c6e3b  29c8                   -sub eax, ecx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.ecx));
    // 004c6e3d  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004c6e40  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004c6e42  a110015600             -mov eax, dword ptr [0x560110]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(5636368) /* 0x560110 */);
    // 004c6e47  c1e106                 -shl ecx, 6
    cpu.ecx <<= 6 /*0x6*/ % 32;
    // 004c6e4a  01c8                   +add eax, ecx
    {
        x86::reg32& tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(cpu.ecx));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004c6e4c  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004c6e4f  c7400c00000000         -mov dword ptr [eax + 0xc], 0
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */) = 0 /*0x0*/;
    // 004c6e56  eb09                   -jmp 0x4c6e61
    goto L_0x004c6e61;
L_0x004c6e58:
    // 004c6e58  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c6e5b  83780c00               +cmp dword ptr [eax + 0xc], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c6e5f  7508                   -jne 0x4c6e69
    if (!cpu.flags.zf)
    {
        goto L_0x004c6e69;
    }
L_0x004c6e61:
    // 004c6e61  8b7dfc                 -mov edi, dword ptr [ebp - 4]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c6e64  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004c6e66  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c6e67  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c6e68  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x004c6e69:
    // 004c6e69  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c6e6b  88f8                   -mov al, bh
    cpu.al = cpu.bh;
    // 004c6e6d  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004c6e6e  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c6e70  88d8                   -mov al, bl
    cpu.al = cpu.bl;
    // 004c6e72  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004c6e73  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c6e76  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c6e79  8b400c                 -mov eax, dword ptr [eax + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004c6e7c  81c220030000           -add edx, 0x320
    (cpu.edx) += x86::reg32(x86::sreg32(800 /*0x320*/));
    // 004c6e82  01c0                   -add eax, eax
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.eax));
    // 004c6e84  8d3402                 -lea esi, [edx + eax]
    cpu.esi = x86::reg32(cpu.edx + cpu.eax * 1);
    // 004c6e87  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c6e8a  8b520c                 -mov edx, dword ptr [edx + 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
    // 004c6e8d  8d049500000000         -lea eax, [edx*4]
    cpu.eax = x86::reg32(cpu.edx * 4);
    // 004c6e94  29d0                   -sub eax, edx
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.edx));
    // 004c6e96  8d4db4                 -lea ecx, [ebp - 0x4c]
    cpu.ecx = x86::reg32(cpu.ebp + x86::reg32(-76) /* -0x4c */);
    // 004c6e99  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004c6e9b  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c6e9e  c1e204                 -shl edx, 4
    cpu.edx <<= 4 /*0x4*/ % 32;
    // 004c6ea1  83c010                 -add eax, 0x10
    (cpu.eax) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004c6ea4  8d5dcc                 -lea ebx, [ebp - 0x34]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-52) /* -0x34 */);
    // 004c6ea7  01d0                   -add eax, edx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.edx));
    // 004c6ea9  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004c6eab  e8c0fdffff             -call 0x4c6c70
    cpu.esp -= 4;
    sub_4c6c70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6eb0  837d1002               +cmp dword ptr [ebp + 0x10], 2
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(16) /* 0x10 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2 /*0x2*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c6eb4  7517                   -jne 0x4c6ecd
    if (!cpu.flags.zf)
    {
        goto L_0x004c6ecd;
    }
    // 004c6eb6  e8cd9e0100             -call 0x4e0d88
    cpu.esp -= 4;
    nfs3hp::port::eaRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6ebb  88c2                   -mov dl, al
    cpu.dl = cpu.al;
    // 004c6ebd  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c6ec0  8b580c                 -mov ebx, dword ptr [eax + 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */);
    // 004c6ec3  80e201                 -and dl, 1
    cpu.dl &= x86::reg8(x86::sreg8(1 /*0x1*/));
    // 004c6ec6  01d8                   -add eax, ebx
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.ebx));
    // 004c6ec8  80c202                 +add dl, 2
    {
        x86::reg8& tmp1 = cpu.dl;
        x86::reg8 tmp2 = x86::reg8(x86::sreg8(2 /*0x2*/));
        x86::reg8 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 7);
        cpu.flags.of ^= 1 & (result >> 7);
        cpu.flags.of &= (1 & (tmp1 >> 7)) == (1 & (tmp2 >> 7));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004c6ecb  eb10                   -jmp 0x4c6edd
    goto L_0x004c6edd;
L_0x004c6ecd:
    // 004c6ecd  e8b69e0100             -call 0x4e0d88
    cpu.esp -= 4;
    nfs3hp::port::eaRandom(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6ed2  88c2                   -mov dl, al
    cpu.dl = cpu.al;
    // 004c6ed4  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c6ed7  03400c                 -add eax, dword ptr [eax + 0xc]
    (cpu.eax) += x86::reg32(x86::sreg32(app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */)));
    // 004c6eda  80e201                 -and dl, 1
    cpu.dl &= x86::reg8(x86::sreg8(1 /*0x1*/));
L_0x004c6edd:
    // 004c6edd  889010030000           -mov byte ptr [eax + 0x310], dl
    app->getMemory<x86::reg8>(cpu.eax + x86::reg32(784) /* 0x310 */) = cpu.dl;
    // 004c6ee3  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c6ee6  ff400c                 -inc dword ptr [eax + 0xc]
    (app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */))++;
L_0x004c6ee9:
    // 004c6ee9  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c6eeb  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6eec  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6eed  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c6eee  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8d 0x40 0x00 */
void Application::sub_4c6f00(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c6f00  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c6f01  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c6f02  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c6f03  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c6f04  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c6f05  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c6f06  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c6f08  83ec24                 -sub esp, 0x24
    (cpu.esp) -= x86::reg32(x86::sreg32(36 /*0x24*/));
    // 004c6f0b  8945f0                 -mov dword ptr [ebp - 0x10], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.eax;
    // 004c6f0e  8b1d30bc6f00           -mov ebx, dword ptr [0x6fbc30]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(7322672) /* 0x6fbc30 */);
    // 004c6f14  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c6f16  8b3558577d00           -mov esi, dword ptr [0x7d5758]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8214360) /* 0x7d5758 */);
    // 004c6f1c  8955ec                 -mov dword ptr [ebp - 0x14], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.edx;
    // 004c6f1f  8955fc                 -mov dword ptr [ebp - 4], edx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.edx;
    // 004c6f22  83fb02                 +cmp ebx, 2
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
    // 004c6f25  0f84b7010000           -je 0x4c70e2
    if (cpu.flags.zf)
    {
        goto L_0x004c70e2;
    }
L_0x004c6f2b:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c6f2b  4e                     -dec esi
    (cpu.esi)--;
    // 004c6f2c  83feff                 +cmp esi, -1
    {
        x86::reg32 tmp1 = cpu.esi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c6f2f  0f8488010000           -je 0x4c70bd
    if (cpu.flags.zf)
    {
        goto L_0x004c70bd;
    }
    // 004c6f35  8d04b500000000         -lea eax, [esi*4]
    cpu.eax = x86::reg32(cpu.esi * 4);
    // 004c6f3c  29f0                   -sub eax, esi
    (cpu.eax) -= x86::reg32(x86::sreg32(cpu.esi));
    // 004c6f3e  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004c6f41  01f0                   -add eax, esi
    (cpu.eax) += x86::reg32(x86::sreg32(cpu.esi));
    // 004c6f43  8b0d10015600           -mov ecx, dword ptr [0x560110]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(5636368) /* 0x560110 */);
    // 004c6f49  c1e006                 -shl eax, 6
    cpu.eax <<= 6 /*0x6*/ % 32;
    // 004c6f4c  8b55f0                 -mov edx, dword ptr [ebp - 0x10]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004c6f4f  01c1                   -add ecx, eax
    (cpu.ecx) += x86::reg32(x86::sreg32(cpu.eax));
    // 004c6f51  83c208                 -add edx, 8
    (cpu.edx) += x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004c6f54  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004c6f56  6800004842             -push 0x42480000
    app->getMemory<x86::reg32>(cpu.esp-4) = 1112014848 /*0x42480000*/;
    cpu.esp -= 4;
    // 004c6f5b  e8c0fffcff             -call 0x496f20
    cpu.esp -= 4;
    sub_496f20(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6f60  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c6f62  74c7                   -je 0x4c6f2b
    if (cpu.flags.zf)
    {
        goto L_0x004c6f2b;
    }
    // 004c6f64  8b410c                 -mov eax, dword ptr [ecx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 004c6f67  c1e002                 -shl eax, 2
    cpu.eax <<= 2 /*0x2*/ % 32;
    // 004c6f6a  8945e8                 -mov dword ptr [ebp - 0x18], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */) = cpu.eax;
    // 004c6f6d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c6f6f  74ba                   -je 0x4c6f2b
    if (cpu.flags.zf)
    {
        goto L_0x004c6f2b;
    }
    // 004c6f71  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004c6f73  8d45dc                 -lea eax, [ebp - 0x24]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004c6f76  c1e205                 -shl edx, 5
    cpu.edx <<= 5 /*0x5*/ % 32;
    // 004c6f79  e8624effff             -call 0x4bbde0
    cpu.esp -= 4;
    sub_4bbde0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6f7e  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c6f80  7502                   -jne 0x4c6f84
    if (!cpu.flags.zf)
    {
        goto L_0x004c6f84;
    }
    // 004c6f82  eba7                   -jmp 0x4c6f2b
    goto L_0x004c6f2b;
L_0x004c6f84:
    // 004c6f84  8b5df0                 -mov ebx, dword ptr [ebp - 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004c6f87  8d8110030000           -lea eax, [ecx + 0x310]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(784) /* 0x310 */);
    // 004c6f8d  8b790c                 -mov edi, dword ptr [ecx + 0xc]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 004c6f90  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004c6f93  8d8120030000           -lea eax, [ecx + 0x320]
    cpu.eax = x86::reg32(cpu.ecx + x86::reg32(800) /* 0x320 */);
    // 004c6f99  83c344                 -add ebx, 0x44
    (cpu.ebx) += x86::reg32(x86::sreg32(68 /*0x44*/));
    // 004c6f9c  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004c6f9f  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004c6fa2  83c110                 -add ecx, 0x10
    (cpu.ecx) += x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004c6fa5  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004c6fa6  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004c6fa9  894de0                 -mov dword ptr [ebp - 0x20], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */) = cpu.ecx;
    // 004c6fac  83c038                 -add eax, 0x38
    (cpu.eax) += x86::reg32(x86::sreg32(56 /*0x38*/));
    // 004c6faf  8b55e0                 -mov edx, dword ptr [ebp - 0x20]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-32) /* -0x20 */);
    // 004c6fb2  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004c6fb4  8b45e8                 -mov eax, dword ptr [ebp - 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-24) /* -0x18 */);
    // 004c6fb7  e80485ffff             -call 0x4bf4c0
    cpu.esp -= 4;
    nfs3hp::vertices::objectVertices(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6fbc  ba05000000             -mov edx, 5
    cpu.edx = 5 /*0x5*/;
    // 004c6fc1  89f8                   -mov eax, edi
    cpu.eax = cpu.edi;
    // 004c6fc3  e858acf6ff             -call 0x431c20
    cpu.esp -= 4;
    nfs3hp::collide::passLayerNothing(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c6fc8:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c6fc8  4f                     -dec edi
    (cpu.edi)--;
    // 004c6fc9  83ffff                 +cmp edi, -1
    {
        x86::reg32 tmp1 = cpu.edi;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(-1 /*-0x1*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c6fcc  0f8459ffffff           -je 0x4c6f2b
    if (cpu.flags.zf)
    {
        goto L_0x004c6f2b;
    }
    // 004c6fd2  ba20000000             -mov edx, 0x20
    cpu.edx = 32 /*0x20*/;
    // 004c6fd7  8d45e4                 -lea eax, [ebp - 0x1c]
    cpu.eax = x86::reg32(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004c6fda  e8014effff             -call 0x4bbde0
    cpu.esp -= 4;
    sub_4bbde0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c6fdf  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c6fe1  0f84b5000000           -je 0x4c709c
    if (cpu.flags.zf)
    {
        goto L_0x004c709c;
    }
    // 004c6fe7  837dec00               +cmp dword ptr [ebp - 0x14], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c6feb  7508                   -jne 0x4c6ff5
    if (!cpu.flags.zf)
    {
        goto L_0x004c6ff5;
    }
    // 004c6fed  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004c6ff0  8945ec                 -mov dword ptr [ebp - 0x14], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */) = cpu.eax;
    // 004c6ff3  eb08                   -jmp 0x4c6ffd
    goto L_0x004c6ffd;
L_0x004c6ff5:
    // 004c6ff5  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c6ff8  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004c6ffb  8902                   -mov dword ptr [edx], eax
    app->getMemory<x86::reg32>(cpu.edx) = cpu.eax;
L_0x004c6ffd:
    // 004c6ffd  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004c7000  c70000000000           -mov dword ptr [eax], 0
    app->getMemory<x86::reg32>(cpu.eax) = 0 /*0x0*/;
    // 004c7006  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004c7009  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004c700c  66c740040000           -mov word ptr [eax + 4], 0
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(4) /* 0x4 */) = 0 /*0x0*/;
    // 004c7012  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004c7015  66c740060000           -mov word ptr [eax + 6], 0
    app->getMemory<x86::reg16>(cpu.eax + x86::reg32(6) /* 0x6 */) = 0 /*0x0*/;
    // 004c701b  8b55e4                 -mov edx, dword ptr [ebp - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004c701e  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004c7021  894208                 -mov dword ptr [edx + 8], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */) = cpu.eax;
    // 004c7024  8b55e4                 -mov edx, dword ptr [ebp - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004c7027  83c020                 -add eax, 0x20
    (cpu.eax) += x86::reg32(x86::sreg32(32 /*0x20*/));
    // 004c702a  89420c                 -mov dword ptr [edx + 0xc], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */) = cpu.eax;
    // 004c702d  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004c7030  8b55e4                 -mov edx, dword ptr [ebp - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004c7033  83c040                 -add eax, 0x40
    (cpu.eax) += x86::reg32(x86::sreg32(64 /*0x40*/));
    // 004c7036  894210                 -mov dword ptr [edx + 0x10], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(16) /* 0x10 */) = cpu.eax;
    // 004c7039  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004c703c  8b55e4                 -mov edx, dword ptr [ebp - 0x1c]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004c703f  83c060                 -add eax, 0x60
    (cpu.eax) += x86::reg32(x86::sreg32(96 /*0x60*/));
    // 004c7042  894214                 -mov dword ptr [edx + 0x14], eax
    app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */) = cpu.eax;
    // 004c7045  8b55f4                 -mov edx, dword ptr [ebp - 0xc]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004c7048  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c704a  8a02                   -mov al, byte ptr [edx]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx);
    // 004c704c  8b1485c8418b00         -mov edx, dword ptr [eax*4 + 0x8b41c8]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(9126344) /* 0x8b41c8 */ + cpu.eax * 4);
    // 004c7053  8b45e4                 -mov eax, dword ptr [ebp - 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-28) /* -0x1c */);
    // 004c7056  895018                 -mov dword ptr [eax + 0x18], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(24) /* 0x18 */) = cpu.edx;
    // 004c7059  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c705c  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c705e  8a02                   -mov al, byte ptr [edx]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx);
    // 004c7060  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004c7062  c1e218                 -shl edx, 0x18
    cpu.edx <<= 24 /*0x18*/ % 32;
    // 004c7065  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004c7068  81caffffff00           -or edx, 0xffffff
    cpu.edx |= x86::reg32(x86::sreg32(16777215 /*0xffffff*/));
    // 004c706e  895030                 -mov dword ptr [eax + 0x30], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(48) /* 0x30 */) = cpu.edx;
    // 004c7071  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004c7074  8b5030                 -mov edx, dword ptr [eax + 0x30]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(48) /* 0x30 */);
    // 004c7077  895010                 -mov dword ptr [eax + 0x10], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(16) /* 0x10 */) = cpu.edx;
    // 004c707a  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c707d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
    // 004c707f  8a4201                 -mov al, byte ptr [edx + 1]
    cpu.al = app->getMemory<x86::reg8>(cpu.edx + x86::reg32(1) /* 0x1 */);
    // 004c7082  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004c7084  c1e218                 -shl edx, 0x18
    cpu.edx <<= 24 /*0x18*/ % 32;
    // 004c7087  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004c708a  81caffffff00           -or edx, 0xffffff
    cpu.edx |= x86::reg32(x86::sreg32(16777215 /*0xffffff*/));
    // 004c7090  895070                 -mov dword ptr [eax + 0x70], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(112) /* 0x70 */) = cpu.edx;
    // 004c7093  8b45dc                 -mov eax, dword ptr [ebp - 0x24]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004c7096  8b5070                 -mov edx, dword ptr [eax + 0x70]
    cpu.edx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(112) /* 0x70 */);
    // 004c7099  895050                 -mov dword ptr [eax + 0x50], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(80) /* 0x50 */) = cpu.edx;
L_0x004c709c:
    // 004c709c  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004c709f  8b45f8                 -mov eax, dword ptr [ebp - 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c70a2  8b4ddc                 -mov ecx, dword ptr [ebp - 0x24]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */);
    // 004c70a5  43                     -inc ebx
    (cpu.ebx)++;
    // 004c70a6  83c002                 -add eax, 2
    (cpu.eax) += x86::reg32(x86::sreg32(2 /*0x2*/));
    // 004c70a9  81c180000000           +add ecx, 0x80
    {
        x86::reg32& tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(128 /*0x80*/));
        x86::reg32 result = tmp1 + tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) == (1 & (tmp2 >> 31));
        cpu.flags.cf = result < tmp1;
        tmp1 = result;
        cpu.set_szp(tmp1);
    }
    // 004c70af  895df4                 -mov dword ptr [ebp - 0xc], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.ebx;
    // 004c70b2  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004c70b5  894ddc                 -mov dword ptr [ebp - 0x24], ecx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-36) /* -0x24 */) = cpu.ecx;
    // 004c70b8  e90bffffff             -jmp 0x4c6fc8
    goto L_0x004c6fc8;
L_0x004c70bd:
    // 004c70bd  8b55fc                 -mov edx, dword ptr [ebp - 4]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c70c0  8b45ec                 -mov eax, dword ptr [ebp - 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-20) /* -0x14 */);
    // 004c70c3  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004c70c5  7406                   -je 0x4c70cd
    if (cpu.flags.zf)
    {
        goto L_0x004c70cd;
    }
    // 004c70c7  c70200000000           -mov dword ptr [edx], 0
    app->getMemory<x86::reg32>(cpu.edx) = 0 /*0x0*/;
L_0x004c70cd:
    // 004c70cd  ff35eccf7d00           -push dword ptr [0x7dcfec]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(x86::reg32(8245228) /* 0x7dcfec */);
    cpu.esp -= 4;
    // 004c70d3  e8b8060000             -call 0x4c7790
    cpu.esp -= 4;
    nfs3hp::vertices::depthBuckets(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c70d8  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004c70dd  a350577d00             -mov dword ptr [0x7d5750], eax
    app->getMemory<x86::reg32>(x86::reg32(8214352) /* 0x7d5750 */) = cpu.eax;
L_0x004c70e2:
    // 004c70e2  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c70e4  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c70e5  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c70e6  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c70e7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c70e8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c70e9  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c70ea  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4c70f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c70f0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c70f1  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c70f2  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c70f3  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c70f5  ba00d00000             -mov edx, 0xd000
    cpu.edx = 53248 /*0xd000*/;
    // 004c70fa  b820025400             -mov eax, 0x540220
    cpu.eax = 5505568 /*0x540220*/;
    // 004c70ff  31db                   -xor ebx, ebx
    cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx));
    // 004c7101  e81aa50100             -call 0x4e1620
    cpu.esp -= 4;
    nfs3hp::ea::reservememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7106  31d2                   -xor edx, edx
    cpu.edx ^= x86::reg32(x86::sreg32(cpu.edx));
    // 004c7108  a310015600             -mov dword ptr [0x560110], eax
    app->getMemory<x86::reg32>(x86::reg32(5636368) /* 0x560110 */) = cpu.eax;
    // 004c710d  891558577d00           -mov dword ptr [0x7d5758], edx
    app->getMemory<x86::reg32>(x86::reg32(8214360) /* 0x7d5758 */) = cpu.edx;
    // 004c7113  89500c                 -mov dword ptr [eax + 0xc], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(12) /* 0xc */) = cpu.edx;
    // 004c7116  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004c711b  891554577d00           -mov dword ptr [0x7d5754], edx
    app->getMemory<x86::reg32>(x86::reg32(8214356) /* 0x7d5754 */) = cpu.edx;
    // 004c7121  a350577d00             -mov dword ptr [0x7d5750], eax
    app->getMemory<x86::reg32>(x86::reg32(8214352) /* 0x7d5750 */) = cpu.eax;
    // 004c7126  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7127  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7128  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7129  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 */
void Application::sub_4c7130(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7130  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7131  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7133  e818000000             -call 0x4c7150
    cpu.esp -= 4;
    sub_4c7150(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7138  e8b3ffffff             -call 0x4c70f0
    cpu.esp -= 4;
    sub_4c70f0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c713d  a184367d00             -mov eax, dword ptr [0x7d3684]
    cpu.eax = app->getMemory<x86::reg32>(x86::reg32(8205956) /* 0x7d3684 */);
    // 004c7142  a350577d00             -mov dword ptr [0x7d5750], eax
    app->getMemory<x86::reg32>(x86::reg32(8214352) /* 0x7d5750 */) = cpu.eax;
    // 004c7147  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7148  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4c7150(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7150  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7151  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7152  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7153  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7155  8b1510015600           -mov edx, dword ptr [0x560110]
    cpu.edx = app->getMemory<x86::reg32>(x86::reg32(5636368) /* 0x560110 */);
    // 004c715b  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004c715d  740f                   -je 0x4c716e
    if (cpu.flags.zf)
    {
        goto L_0x004c716e;
    }
    // 004c715f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004c7161  31c9                   -xor ecx, ecx
    cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx));
    // 004c7163  e828a70100             -call 0x4e1890
    cpu.esp -= 4;
    nfs3hp::ea::purgememadr(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7168  890d10015600           -mov dword ptr [0x560110], ecx
    app->getMemory<x86::reg32>(x86::reg32(5636368) /* 0x560110 */) = cpu.ecx;
L_0x004c716e:
    // 004c716e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c716f  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7170  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7171  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 */
void Application::sub_4c7180(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
  x86::reg16 fsw = 0;
    // 004c7180  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7181  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c7182  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c7183  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7184  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7186  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004c7189  89c1                   -mov ecx, eax
    cpu.ecx = cpu.eax;
    // 004c718b  bef02374c9             -mov esi, 0xc97423f0
    cpu.esi = 3379831792 /*0xc97423f0*/;
    // 004c7190  c702f0237449           -mov dword ptr [edx], 0x497423f0
    app->getMemory<x86::reg32>(cpu.edx) = 1232348144 /*0x497423f0*/;
    // 004c7196  8975f8                 -mov dword ptr [ebp - 8], esi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.esi;
L_0x004c7199:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c7199  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004c719b  0f848c000000           -je 0x4c722d
    if (cpu.flags.zf)
    {
        goto L_0x004c722d;
    }
    // 004c71a1  668b7104               -mov si, word ptr [ecx + 4]
    cpu.si = app->getMemory<x86::reg16>(cpu.ecx + x86::reg32(4) /* 0x4 */);
    // 004c71a5  6685f6                 +test si, si
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.si & cpu.si));
    // 004c71a8  7406                   -je 0x4c71b0
    if (cpu.flags.zf)
    {
        goto L_0x004c71b0;
    }
    // 004c71aa  6683fe02               +cmp si, 2
    {
        x86::reg16 tmp1 = cpu.si;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2 /*0x2*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c71ae  7529                   -jne 0x4c71d9
    if (!cpu.flags.zf)
    {
        goto L_0x004c71d9;
    }
L_0x004c71b0:
    // 004c71b0  8b7108                 -mov esi, dword ptr [ecx + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 004c71b3  8b410c                 -mov eax, dword ptr [ecx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 004c71b6  8b7910                 -mov edi, dword ptr [ecx + 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(16) /* 0x10 */);
    // 004c71b9  d94608                 +fld dword ptr [esi + 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(8) /* 0x8 */));
    // 004c71bc  897df0                 -mov dword ptr [ebp - 0x10], edi
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.edi;
    // 004c71bf  d84008                 +fadd dword ptr [eax + 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 004c71c2  8b45f0                 -mov eax, dword ptr [ebp - 0x10]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004c71c5  d84008                 +fadd dword ptr [eax + 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 004c71c8  8b7914                 -mov edi, dword ptr [ecx + 0x14]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(20) /* 0x14 */);
    // 004c71cb  d84708                 +fadd dword ptr [edi + 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(8) /* 0x8 */)));
    // 004c71ce  d80d24025400           +fmul dword ptr [0x540224]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5505572) /* 0x540224 */)));
    // 004c71d4  d95dfc                 +fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
    // 004c71d7  eb27                   -jmp 0x4c7200
    goto L_0x004c7200;
L_0x004c71d9:
    // 004c71d9  6683fe01               +cmp si, 1
    {
        x86::reg16 tmp1 = cpu.si;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(1 /*0x1*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c71dd  7406                   -je 0x4c71e5
    if (cpu.flags.zf)
    {
        goto L_0x004c71e5;
    }
    // 004c71df  6683fe03               +cmp si, 3
    {
        x86::reg16 tmp1 = cpu.si;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(3 /*0x3*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c71e3  7541                   -jne 0x4c7226
    if (!cpu.flags.zf)
    {
        goto L_0x004c7226;
    }
L_0x004c71e5:
    // 004c71e5  8b7108                 -mov esi, dword ptr [ecx + 8]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(8) /* 0x8 */);
    // 004c71e8  8b410c                 -mov eax, dword ptr [ecx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(12) /* 0xc */);
    // 004c71eb  d94608                 +fld dword ptr [esi + 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(8) /* 0x8 */));
    // 004c71ee  d84008                 +fadd dword ptr [eax + 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 004c71f1  8b7910                 -mov edi, dword ptr [ecx + 0x10]
    cpu.edi = app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(16) /* 0x10 */);
    // 004c71f4  d84708                 +fadd dword ptr [edi + 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.edi + x86::reg32(8) /* 0x8 */)));
    // 004c71f7  d80d28025400           +fmul dword ptr [0x540228]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5505576) /* 0x540228 */)));
    // 004c71fd  d95dfc                 +fstp dword ptr [ebp - 4]
    app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = float(fp0);
L_0x004c7200:
    // 004c7200  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004c7203  d81a                   +fcomp dword ptr [edx]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 004c7205  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004c7207  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004c7208  7305                   -jae 0x4c720f
    if (!cpu.flags.cf)
    {
        goto L_0x004c720f;
    }
    // 004c720a  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c720d  8902                   -mov dword ptr [edx], eax
    app->getMemory<x86::reg32>(cpu.edx) = cpu.eax;
L_0x004c720f:
    // 004c720f  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004c7212  d85df8                 +fcomp dword ptr [ebp - 8]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */)));
    // 004c7215  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004c7217  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004c7218  7606                   -jbe 0x4c7220
    if (cpu.flags.cf || cpu.flags.zf)
    {
        goto L_0x004c7220;
    }
    // 004c721a  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c721d  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
L_0x004c7220:
    // 004c7220  8b45fc                 -mov eax, dword ptr [ebp - 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c7223  89411c                 -mov dword ptr [ecx + 0x1c], eax
    app->getMemory<x86::reg32>(cpu.ecx + x86::reg32(28) /* 0x1c */) = cpu.eax;
L_0x004c7226:
    // 004c7226  8b09                   -mov ecx, dword ptr [ecx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ecx);
    // 004c7228  e96cffffff             -jmp 0x4c7199
    goto L_0x004c7199;
L_0x004c722d:
    // 004c722d  d945f8                 -fld dword ptr [ebp - 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-8) /* -0x8 */));
    // 004c7230  d822                   -fsub dword ptr [edx]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.edx)));
    // 004c7232  d95df4                 -fstp dword ptr [ebp - 0xc]
    app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */) = float(fp0);
    // 004c7235  f745f4ffffff7f         +test dword ptr [ebp - 0xc], 0x7fffffff
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) & 2147483647 /*0x7fffffff*/));
    // 004c723c  7504                   -jne 0x4c7242
    if (!cpu.flags.zf)
    {
        goto L_0x004c7242;
    }
    // 004c723e  890b                   -mov dword ptr [ebx], ecx
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.ecx;
    // 004c7240  eb0b                   -jmp 0x4c724d
    goto L_0x004c724d;
L_0x004c7242:
    // 004c7242  d9052c025400           -fld dword ptr [0x54022c]
    fp0 = x86::Float(app->getMemory<float>(x86::reg32(5505580) /* 0x54022c */));
    // 004c7248  d875f4                 -fdiv dword ptr [ebp - 0xc]
    fp0 = fp::div(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-12) /* -0xc */)));
    // 004c724b  d91b                   -fstp dword ptr [ebx]
    app->getMemory<float>(cpu.ebx) = float(fp0);
L_0x004c724d:
    // 004c724d  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c724f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7250  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7251  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7252  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7253  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4c7260(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
  x86::reg16 fsw = 0;
    // 004c7260  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c7261  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7262  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7263  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c7264  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c7265  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7266  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7268  83ec0c                 -sub esp, 0xc
    (cpu.esp) -= x86::reg32(x86::sreg32(12 /*0xc*/));
    // 004c726b  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
L_0x004c726d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c726d  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004c726f  0f849d000000           -je 0x4c7312
    if (cpu.flags.zf)
    {
        goto L_0x004c7312;
    }
    // 004c7275  8b02                   -mov eax, dword ptr [edx]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx);
    // 004c7277  668b5a04               -mov bx, word ptr [edx + 4]
    cpu.bx = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 004c727b  8945f8                 -mov dword ptr [ebp - 8], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */) = cpu.eax;
    // 004c727e  6685db                 +test bx, bx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.bx & cpu.bx));
    // 004c7281  7416                   -je 0x4c7299
    if (cpu.flags.zf)
    {
        goto L_0x004c7299;
    }
    // 004c7283  6683fb01               +cmp bx, 1
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(1 /*0x1*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c7287  7410                   -je 0x4c7299
    if (cpu.flags.zf)
    {
        goto L_0x004c7299;
    }
    // 004c7289  6683fb02               +cmp bx, 2
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(2 /*0x2*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c728d  740a                   -je 0x4c7299
    if (cpu.flags.zf)
    {
        goto L_0x004c7299;
    }
    // 004c728f  6683fb03               +cmp bx, 3
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(3 /*0x3*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c7293  0f8571000000           -jne 0x4c730a
    if (!cpu.flags.zf)
    {
        goto L_0x004c730a;
    }
L_0x004c7299:
    // 004c7299  8b421c                 -mov eax, dword ptr [edx + 0x1c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(28) /* 0x1c */);
    // 004c729c  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
    // 004c729f  d945fc                 -fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004c72a2  d8651c                 -fsub dword ptr [ebp + 0x1c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(28) /* 0x1c */)));
    // 004c72a5  d84d20                 -fmul dword ptr [ebp + 0x20]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(32) /* 0x20 */)));
    // 004c72a8  31ff                   -xor edi, edi
    cpu.edi ^= x86::reg32(x86::sreg32(cpu.edi));
    // 004c72aa  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004c72ad  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004c72b0  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c72b1  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
    // 004c72b4  8b34c5dc9d7d00         -mov esi, dword ptr [eax*8 + 0x7d9ddc]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8232412) /* 0x7d9ddc */ + cpu.eax * 8);
    // 004c72bb  8b1cc5e09d7d00         -mov ebx, dword ptr [eax*8 + 0x7d9de0]
    cpu.ebx = app->getMemory<x86::reg32>(x86::reg32(8232416) /* 0x7d9de0 */ + cpu.eax * 8);
    // 004c72c2  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
L_0x004c72c4:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c72c4  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004c72c6  7411                   -je 0x4c72d9
    if (cpu.flags.zf)
    {
        goto L_0x004c72d9;
    }
    // 004c72c8  d945fc                 +fld dword ptr [ebp - 4]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(-4) /* -0x4 */));
    // 004c72cb  d8591c                 +fcomp dword ptr [ecx + 0x1c]
    fsw = fp::compare(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(28) /* 0x1c */)));
    // 004c72ce  dfe0                   -fnstsw ax
    cpu.ax = fsw;
    // 004c72d0  9e                     +sahf 
    cpu.flags.lo = 0x02 | (cpu.ah & 0xD7);
    // 004c72d1  7206                   -jb 0x4c72d9
    if (cpu.flags.cf)
    {
        goto L_0x004c72d9;
    }
    // 004c72d3  89cf                   -mov edi, ecx
    cpu.edi = cpu.ecx;
    // 004c72d5  8b09                   -mov ecx, dword ptr [ecx]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ecx);
    // 004c72d7  ebeb                   -jmp 0x4c72c4
    goto L_0x004c72c4;
L_0x004c72d9:
    // 004c72d9  39f1                   +cmp ecx, esi
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
    // 004c72db  750c                   -jne 0x4c72e9
    if (!cpu.flags.zf)
    {
        goto L_0x004c72e9;
    }
    // 004c72dd  8932                   -mov dword ptr [edx], esi
    app->getMemory<x86::reg32>(cpu.edx) = cpu.esi;
    // 004c72df  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004c72e1  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004c72e3  7514                   -jne 0x4c72f9
    if (!cpu.flags.zf)
    {
        goto L_0x004c72f9;
    }
    // 004c72e5  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 004c72e7  eb10                   -jmp 0x4c72f9
    goto L_0x004c72f9;
L_0x004c72e9:
    // 004c72e9  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004c72eb  7508                   -jne 0x4c72f5
    if (!cpu.flags.zf)
    {
        goto L_0x004c72f5;
    }
    // 004c72ed  890a                   -mov dword ptr [edx], ecx
    app->getMemory<x86::reg32>(cpu.edx) = cpu.ecx;
    // 004c72ef  8913                   -mov dword ptr [ebx], edx
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.edx;
    // 004c72f1  89d3                   -mov ebx, edx
    cpu.ebx = cpu.edx;
    // 004c72f3  eb04                   -jmp 0x4c72f9
    goto L_0x004c72f9;
L_0x004c72f5:
    // 004c72f5  8917                   -mov dword ptr [edi], edx
    app->getMemory<x86::reg32>(cpu.edi) = cpu.edx;
    // 004c72f7  890a                   -mov dword ptr [edx], ecx
    app->getMemory<x86::reg32>(cpu.edx) = cpu.ecx;
L_0x004c72f9:
    // 004c72f9  8b45f4                 -mov eax, dword ptr [ebp - 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004c72fc  8934c5dc9d7d00         -mov dword ptr [eax*8 + 0x7d9ddc], esi
    app->getMemory<x86::reg32>(x86::reg32(8232412) /* 0x7d9ddc */ + cpu.eax * 8) = cpu.esi;
    // 004c7303  891cc5e09d7d00         -mov dword ptr [eax*8 + 0x7d9de0], ebx
    app->getMemory<x86::reg32>(x86::reg32(8232416) /* 0x7d9de0 */ + cpu.eax * 8) = cpu.ebx;
L_0x004c730a:
    // 004c730a  8b55f8                 -mov edx, dword ptr [ebp - 8]
    cpu.edx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c730d  e95bffffff             -jmp 0x4c726d
    goto L_0x004c726d;
L_0x004c7312:
    // 004c7312  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c7314  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7315  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7316  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7317  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7318  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7319  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c731a  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* sub_4c7320: native, nfs3hp::vertices::gatherSorted */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8b 0xd2 */
void Application::sub_4c7380(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7380  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c7381  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7382  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7383  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c7384  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7385  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7387  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004c738a  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 004c738c  ba00190000             -mov edx, 0x1900
    cpu.edx = 6400 /*0x1900*/;
    // 004c7391  b8dc9d7d00             -mov eax, 0x7d9ddc
    cpu.eax = 8232412 /*0x7d9ddc*/;
    // 004c7396  8d5dfc                 -lea ebx, [ebp - 4]
    cpu.ebx = x86::reg32(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c7399  e86e930100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c739e  8b4518                 -mov eax, dword ptr [ebp + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 004c73a1  8d55f8                 -lea edx, [ebp - 8]
    cpu.edx = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c73a4  e8d7fdffff             -call 0x4c7180
    cpu.esp -= 4;
    sub_4c7180(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c73a9  ff75fc                 -push dword ptr [ebp - 4]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    cpu.esp -= 4;
    // 004c73ac  8d75f0                 -lea esi, [ebp - 0x10]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004c73af  ff75f8                 -push dword ptr [ebp - 8]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    cpu.esp -= 4;
    // 004c73b2  8b4518                 -mov eax, dword ptr [ebp + 0x18]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 004c73b5  89cf                   -mov edi, ecx
    cpu.edi = cpu.ecx;
    // 004c73b7  e8a4feffff             -call 0x4c7260
    cpu.esp -= 4;
    sub_4c7260(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c73bc  e85fffffff             -call 0x4c7320
    cpu.esp -= 4;
    nfs3hp::vertices::gatherSorted(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c73c1  8d75f0                 -lea esi, [ebp - 0x10]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004c73c4  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c73c5  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c73c6  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004c73c8  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c73ca  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c73cb  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c73cc  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c73cd  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c73ce  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c73cf  c20800                 -ret 8
    cpu.esp += 4+8 /*0x8*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_4c73e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c73e0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c73e1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c73e2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c73e4  833d603a7a0000         +cmp dword ptr [0x7a3a60], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010336) /* 0x7a3a60 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c73eb  750f                   -jne 0x4c73fc
    if (!cpu.flags.zf)
    {
        goto L_0x004c73fc;
    }
    // 004c73ed  ba803e0000             -mov edx, 0x3e80
    cpu.edx = 16000 /*0x3e80*/;
    // 004c73f2  b85c5f7d00             -mov eax, 0x7d5f5c
    cpu.eax = 8216412 /*0x7d5f5c*/;
    // 004c73f7  e810930100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c73fc:
    // 004c73fc  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c73fd  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c73fe  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x90 */
void Application::sub_4c7400(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 004c7400  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c7401  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7402  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c7403  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c7404  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7405  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7407  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004c740a  833d603a7a0000         +cmp dword ptr [0x7a3a60], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010336) /* 0x7a3a60 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c7411  755a                   -jne 0x4c746d
    if (!cpu.flags.zf)
    {
        goto L_0x004c746d;
    }
    // 004c7413  d94520                 -fld dword ptr [ebp + 0x20]
    fp0 = x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(32) /* 0x20 */));
    // 004c7416  d805dccf7d00           -fadd dword ptr [0x7dcfdc]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(x86::reg32(8245212) /* 0x7dcfdc */)));
    // 004c741c  d80d30025400           -fmul dword ptr [0x540230]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5505584) /* 0x540230 */)));
    // 004c7422  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004c7425  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004c7428  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7429  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c742b  7d04                   -jge 0x4c7431
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c7431;
    }
    // 004c742d  31c0                   +xor eax, eax
    cpu.clear_co();
    cpu.set_szp((cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax))));
    // 004c742f  eb0c                   -jmp 0x4c743d
    goto L_0x004c743d;
L_0x004c7431:
    // 004c7431  3dcf070000             +cmp eax, 0x7cf
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1999 /*0x7cf*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c7436  7e05                   -jle 0x4c743d
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c743d;
    }
    // 004c7438  b8cf070000             -mov eax, 0x7cf
    cpu.eax = 1999 /*0x7cf*/;
L_0x004c743d:
    // 004c743d  8b4d1c                 -mov ecx, dword ptr [ebp + 0x1c]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    // 004c7440  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7441  8b5d18                 -mov ebx, dword ptr [ebp + 0x18]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 004c7444  8d3cc500000000         -lea edi, [eax*8]
    cpu.edi = x86::reg32(cpu.eax * 8);
    // 004c744b  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c744c  8bb7605f7d00           -mov esi, dword ptr [edi + 0x7d5f60]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(8216416) /* 0x7d5f60 */);
    // 004c7452  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c7453  8b875c5f7d00           -mov eax, dword ptr [edi + 0x7d5f5c]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edi + x86::reg32(8216412) /* 0x7d5f5c */);
    // 004c7459  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004c745a  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c745d  e85e4affff             -call 0x4bbec0
    cpu.esp -= 4;
    sub_4bbec0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7462  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c7465  8dbf5c5f7d00           -lea edi, [edi + 0x7d5f5c]
    cpu.edi = x86::reg32(cpu.edi + x86::reg32(8216412) /* 0x7d5f5c */);
    // 004c746b  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c746c  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x004c746d:
    // 004c746d  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c746f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7470  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7471  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7472  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7473  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7474  c20c00                 -ret 0xc
    cpu.esp += 4+12 /*0xc*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4c7480(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
  x86::Float fp0;
    // 004c7480  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c7481  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7482  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7483  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c7484  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c7485  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7486  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7488  8b0d603a7a00           -mov ecx, dword ptr [0x7a3a60]
    cpu.ecx = app->getMemory<x86::reg32>(x86::reg32(8010336) /* 0x7a3a60 */);
    // 004c748e  89c2                   -mov edx, eax
    cpu.edx = cpu.eax;
    // 004c7490  85c9                   +test ecx, ecx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ecx & cpu.ecx));
    // 004c7492  0f85d6000000           -jne 0x4c756e
    if (!cpu.flags.zf)
    {
        goto L_0x004c756e;
    }
L_0x004c7498:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c7498  85d2                   +test edx, edx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edx & cpu.edx));
    // 004c749a  0f84ce000000           -je 0x4c756e
    if (cpu.flags.zf)
    {
        goto L_0x004c756e;
    }
    // 004c74a0  8b3a                   -mov edi, dword ptr [edx]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edx);
    // 004c74a2  668b5a04               -mov bx, word ptr [edx + 4]
    cpu.bx = app->getMemory<x86::reg16>(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 004c74a6  c70200000000           -mov dword ptr [edx], 0
    app->getMemory<x86::reg32>(cpu.edx) = 0 /*0x0*/;
    // 004c74ac  6685db                 +test bx, bx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg16>(cpu.bx & cpu.bx));
    // 004c74af  7546                   -jne 0x4c74f7
    if (!cpu.flags.zf)
    {
        goto L_0x004c74f7;
    }
    // 004c74b1  8b4208                 -mov eax, dword ptr [edx + 8]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 004c74b4  8b4a0c                 -mov ecx, dword ptr [edx + 0xc]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
    // 004c74b7  d94008                 -fld dword ptr [eax + 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */));
    // 004c74ba  d84108                 -fadd dword ptr [ecx + 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(8) /* 0x8 */)));
    // 004c74bd  8b7210                 -mov esi, dword ptr [edx + 0x10]
    cpu.esi = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(16) /* 0x10 */);
    // 004c74c0  d84608                 -fadd dword ptr [esi + 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.esi + x86::reg32(8) /* 0x8 */)));
    // 004c74c3  8b5a14                 -mov ebx, dword ptr [edx + 0x14]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(20) /* 0x14 */);
    // 004c74c6  d84308                 -fadd dword ptr [ebx + 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */)));
    // 004c74c9  dc0d34025400           -fmul qword ptr [0x540234]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505588) /* 0x540234 */)));
    // 004c74cf  d8651c                 -fsub dword ptr [ebp + 0x1c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(28) /* 0x1c */)));
    // 004c74d2  d805dccf7d00           -fadd dword ptr [0x7dcfdc]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(x86::reg32(8245212) /* 0x7dcfdc */)));
    // 004c74d8  d80d3c025400           -fmul dword ptr [0x54023c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5505596) /* 0x54023c */)));
    // 004c74de  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004c74e1  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004c74e4  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c74e5  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c74e7  7c54                   -jl 0x4c753d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c753d;
    }
    // 004c74e9  3dcf070000             +cmp eax, 0x7cf
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1999 /*0x7cf*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c74ee  7e4f                   -jle 0x4c753f
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c753f;
    }
    // 004c74f0  b8cf070000             -mov eax, 0x7cf
    cpu.eax = 1999 /*0x7cf*/;
    // 004c74f5  eb48                   -jmp 0x4c753f
    goto L_0x004c753f;
L_0x004c74f7:
    // 004c74f7  6683fb01               +cmp bx, 1
    {
        x86::reg16 tmp1 = cpu.bx;
        x86::reg16 tmp2 = x86::reg16(x86::sreg16(1 /*0x1*/));
        x86::reg16 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 15);
        cpu.flags.of ^= 1 & (result >> 15);
        cpu.flags.of &= (1 & (tmp1 >> 15)) != (1 & (tmp2 >> 15));
        cpu.set_szp(result);
    }
    // 004c74fb  7540                   -jne 0x4c753d
    if (!cpu.flags.zf)
    {
        goto L_0x004c753d;
    }
    // 004c74fd  8b4a08                 -mov ecx, dword ptr [edx + 8]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(8) /* 0x8 */);
    // 004c7500  8b420c                 -mov eax, dword ptr [edx + 0xc]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(12) /* 0xc */);
    // 004c7503  d94108                 -fld dword ptr [ecx + 8]
    fp0 = x86::Float(app->getMemory<float>(cpu.ecx + x86::reg32(8) /* 0x8 */));
    // 004c7506  d84008                 -fadd dword ptr [eax + 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.eax + x86::reg32(8) /* 0x8 */)));
    // 004c7509  8b5a10                 -mov ebx, dword ptr [edx + 0x10]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(16) /* 0x10 */);
    // 004c750c  d84308                 -fadd dword ptr [ebx + 8]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(cpu.ebx + x86::reg32(8) /* 0x8 */)));
    // 004c750f  dc0d44025400           -fmul qword ptr [0x540244]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<double>(x86::reg32(5505604) /* 0x540244 */)));
    // 004c7515  d8651c                 -fsub dword ptr [ebp + 0x1c]
    fp0 = fp::sub(fp0, x86::Float(app->getMemory<float>(cpu.ebp + x86::reg32(28) /* 0x1c */)));
    // 004c7518  d805dccf7d00           -fadd dword ptr [0x7dcfdc]
    fp0 = fp::add(fp0, x86::Float(app->getMemory<float>(x86::reg32(8245212) /* 0x7dcfdc */)));
    // 004c751e  d80d3c025400           -fmul dword ptr [0x54023c]
    fp0 = fp::mul(fp0, x86::Float(app->getMemory<float>(x86::reg32(5505596) /* 0x54023c */)));
    // 004c7524  83ec04                 -sub esp, 4
    (cpu.esp) -= x86::reg32(x86::sreg32(4 /*0x4*/));
    // 004c7527  db1c24                 -fistp dword ptr [esp]
    app->getMemory<x86::reg32>(cpu.esp) = x86::reg32(fp::toInteger<x86::sreg32>(fp0));
    // 004c752a  58                     -pop eax
    cpu.eax = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c752b  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c752d  7c0e                   -jl 0x4c753d
    if (cpu.flags.sf != cpu.flags.of)
    {
        goto L_0x004c753d;
    }
    // 004c752f  3dcf070000             +cmp eax, 0x7cf
    {
        x86::reg32 tmp1 = cpu.eax;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(1999 /*0x7cf*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c7534  7e09                   -jle 0x4c753f
    if (cpu.flags.zf || (cpu.flags.sf != cpu.flags.of))
    {
        goto L_0x004c753f;
    }
    // 004c7536  b8cf070000             -mov eax, 0x7cf
    cpu.eax = 1999 /*0x7cf*/;
    // 004c753b  eb02                   -jmp 0x4c753f
    goto L_0x004c753f;
L_0x004c753d:
    // 004c753d  31c0                   -xor eax, eax
    cpu.eax ^= x86::reg32(x86::sreg32(cpu.eax));
L_0x004c753f:
    // 004c753f  c1e003                 -shl eax, 3
    cpu.eax <<= 3 /*0x3*/ % 32;
    // 004c7542  83b85c5f7d0000         +cmp dword ptr [eax + 0x7d5f5c], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8216412) /* 0x7d5f5c */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c7549  7410                   -je 0x4c755b
    if (cpu.flags.zf)
    {
        goto L_0x004c755b;
    }
    // 004c754b  8b88605f7d00           -mov ecx, dword ptr [eax + 0x7d5f60]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8216416) /* 0x7d5f60 */);
    // 004c7551  8911                   -mov dword ptr [ecx], edx
    app->getMemory<x86::reg32>(cpu.ecx) = cpu.edx;
    // 004c7553  8990605f7d00           -mov dword ptr [eax + 0x7d5f60], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8216416) /* 0x7d5f60 */) = cpu.edx;
    // 004c7559  eb0c                   -jmp 0x4c7567
    goto L_0x004c7567;
L_0x004c755b:
    // 004c755b  8990605f7d00           -mov dword ptr [eax + 0x7d5f60], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8216416) /* 0x7d5f60 */) = cpu.edx;
    // 004c7561  89905c5f7d00           -mov dword ptr [eax + 0x7d5f5c], edx
    app->getMemory<x86::reg32>(cpu.eax + x86::reg32(8216412) /* 0x7d5f5c */) = cpu.edx;
L_0x004c7567:
    // 004c7567  89fa                   -mov edx, edi
    cpu.edx = cpu.edi;
    // 004c7569  e92affffff             -jmp 0x4c7498
    goto L_0x004c7498;
L_0x004c756e:
    // 004c756e  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c756f  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7570  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7571  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7572  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7573  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7574  c20400                 -ret 4
    cpu.esp += 4+4 /*0x4*/;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x52 0x00 */
void Application::sub_4c7580(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7580  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c7581  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7582  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7583  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c7584  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7585  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7587  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004c758a  89f1                   -mov ecx, esi
    cpu.ecx = cpu.esi;
    // 004c758c  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c758f  e8dc48ffff             -call 0x4bbe70
    cpu.esp -= 4;
    sub_4bbe70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7594  833d603a7a0000         +cmp dword ptr [0x7a3a60], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010336) /* 0x7a3a60 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c759b  7535                   -jne 0x4c75d2
    if (!cpu.flags.zf)
    {
        goto L_0x004c75d2;
    }
    // 004c759d  bad49d7d00             -mov edx, 0x7d9dd4
    cpu.edx = 8232404 /*0x7d9dd4*/;
    // 004c75a2  31db                   +xor ebx, ebx
    cpu.clear_co();
    cpu.set_szp((cpu.ebx ^= x86::reg32(x86::sreg32(cpu.ebx))));
    // 004c75a4  eb1d                   -jmp 0x4c75c3
    goto L_0x004c75c3;
L_0x004c75a6:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c75a6  8b3a                   -mov edi, dword ptr [edx]
    cpu.edi = app->getMemory<x86::reg32>(cpu.edx);
    // 004c75a8  85ff                   +test edi, edi
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.edi & cpu.edi));
    // 004c75aa  740b                   -je 0x4c75b7
    if (cpu.flags.zf)
    {
        goto L_0x004c75b7;
    }
    // 004c75ac  8b75fc                 -mov esi, dword ptr [ebp - 4]
    cpu.esi = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */);
    // 004c75af  893e                   -mov dword ptr [esi], edi
    app->getMemory<x86::reg32>(cpu.esi) = cpu.edi;
    // 004c75b1  8b4204                 -mov eax, dword ptr [edx + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.edx + x86::reg32(4) /* 0x4 */);
    // 004c75b4  8945fc                 -mov dword ptr [ebp - 4], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-4) /* -0x4 */) = cpu.eax;
L_0x004c75b7:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c75b7  83ea08                 -sub edx, 8
    (cpu.edx) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004c75ba  43                     -inc ebx
    (cpu.ebx)++;
    // 004c75bb  81fbd0070000           +cmp ebx, 0x7d0
    {
        x86::reg32 tmp1 = cpu.ebx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(2000 /*0x7d0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c75c1  7d0f                   -jge 0x4c75d2
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c75d2;
    }
L_0x004c75c3:
    // 004c75c3  837df800               +cmp dword ptr [ebp - 8], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-8) /* -0x8 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c75c7  75dd                   -jne 0x4c75a6
    if (!cpu.flags.zf)
    {
        goto L_0x004c75a6;
    }
    // 004c75c9  8d7df8                 -lea edi, [ebp - 8]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c75cc  89d6                   -mov esi, edx
    cpu.esi = cpu.edx;
    // 004c75ce  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c75cf  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c75d0  ebe5                   -jmp 0x4c75b7
    goto L_0x004c75b7;
L_0x004c75d2:
    // 004c75d2  8d7df0                 -lea edi, [ebp - 0x10]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004c75d5  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c75d8  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c75d9  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c75da  8d75f0                 -lea esi, [ebp - 0x10]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004c75dd  89cf                   -mov edi, ecx
    cpu.edi = cpu.ecx;
    // 004c75df  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c75e0  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c75e1  89c8                   -mov eax, ecx
    cpu.eax = cpu.ecx;
    // 004c75e3  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c75e5  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c75e6  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c75e7  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c75e8  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c75e9  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c75ea  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x40 0x00 0x8b 0xc9 */
void Application::sub_4c75f0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c75f0  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c75f1  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c75f2  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c75f4  833d603a7a0000         +cmp dword ptr [0x7a3a60], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010336) /* 0x7a3a60 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c75fb  740f                   -je 0x4c760c
    if (cpu.flags.zf)
    {
        goto L_0x004c760c;
    }
    // 004c75fd  ba00190000             -mov edx, 0x1900
    cpu.edx = 6400 /*0x1900*/;
    // 004c7602  b8dcb67d00             -mov eax, 0x7db6dc
    cpu.eax = 8238812 /*0x7db6dc*/;
    // 004c7607  e800910100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c760c:
    // 004c760c  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c760d  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c760e  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4c7610: native, nfs3hp::vertices::sortedBuckets */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 0x8b 0xc0 */
void Application::sub_4c76e0(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c76e0  53                     -push ebx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebx;
    cpu.esp -= 4;
    // 004c76e1  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c76e2  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c76e3  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c76e4  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c76e5  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c76e7  83ec10                 -sub esp, 0x10
    (cpu.esp) -= x86::reg32(x86::sreg32(16 /*0x10*/));
    // 004c76ea  89f2                   -mov edx, esi
    cpu.edx = cpu.esi;
    // 004c76ec  8d75f0                 -lea esi, [ebp - 0x10]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004c76ef  e87c47ffff             -call 0x4bbe70
    cpu.esp -= 4;
    sub_4bbe70(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c76f4  833d603a7a0000         +cmp dword ptr [0x7a3a60], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010336) /* 0x7a3a60 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c76fb  7433                   -je 0x4c7730
    if (cpu.flags.zf)
    {
        goto L_0x004c7730;
    }
    // 004c76fd  bed4cf7d00             -mov esi, 0x7dcfd4
    cpu.esi = 8245204 /*0x7dcfd4*/;
    // 004c7702  31c9                   +xor ecx, ecx
    cpu.clear_co();
    cpu.set_szp((cpu.ecx ^= x86::reg32(x86::sreg32(cpu.ecx))));
    // 004c7704  eb19                   -jmp 0x4c771f
    goto L_0x004c771f;
L_0x004c7706:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c7706  89d8                   -mov eax, ebx
    cpu.eax = cpu.ebx;
    // 004c7708  8b5df4                 -mov ebx, dword ptr [ebp - 0xc]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */);
    // 004c770b  8903                   -mov dword ptr [ebx], eax
    app->getMemory<x86::reg32>(cpu.ebx) = cpu.eax;
L_0x004c770d:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c770d  8b4604                 -mov eax, dword ptr [esi + 4]
    cpu.eax = app->getMemory<x86::reg32>(cpu.esi + x86::reg32(4) /* 0x4 */);
    // 004c7710  8945f4                 -mov dword ptr [ebp - 0xc], eax
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-12) /* -0xc */) = cpu.eax;
L_0x004c7713:
    if (app->contextWanted()) app->yieldContext(cpu); /* safepoint */
    // 004c7713  83ee08                 -sub esi, 8
    (cpu.esi) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004c7716  41                     -inc ecx
    (cpu.ecx)++;
    // 004c7717  81f920030000           +cmp ecx, 0x320
    {
        x86::reg32 tmp1 = cpu.ecx;
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(800 /*0x320*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c771d  7d11                   -jge 0x4c7730
    if (cpu.flags.sf == cpu.flags.of)
    {
        goto L_0x004c7730;
    }
L_0x004c771f:
    // 004c771f  8b1e                   -mov ebx, dword ptr [esi]
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esi);
    // 004c7721  85db                   +test ebx, ebx
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.ebx & cpu.ebx));
    // 004c7723  74ee                   -je 0x4c7713
    if (cpu.flags.zf)
    {
        goto L_0x004c7713;
    }
    // 004c7725  837df000               +cmp dword ptr [ebp - 0x10], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c7729  75db                   -jne 0x4c7706
    if (!cpu.flags.zf)
    {
        goto L_0x004c7706;
    }
    // 004c772b  895df0                 -mov dword ptr [ebp - 0x10], ebx
    app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(-16) /* -0x10 */) = cpu.ebx;
    // 004c772e  ebdd                   -jmp 0x4c770d
    goto L_0x004c770d;
L_0x004c7730:
    // 004c7730  8d7df8                 -lea edi, [ebp - 8]
    cpu.edi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c7733  8d75f0                 -lea esi, [ebp - 0x10]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-16) /* -0x10 */);
    // 004c7736  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c7737  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c7738  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c773b  89d7                   -mov edi, edx
    cpu.edi = cpu.edx;
    // 004c773d  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c773e  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c773f  89d0                   -mov eax, edx
    cpu.eax = cpu.edx;
    // 004c7741  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c7743  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7744  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7745  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7746  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7747  5b                     -pop ebx
    cpu.ebx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7748  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x90 */
void Application::sub_4c7750(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7750  52                     -push edx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edx;
    cpu.esp -= 4;
    // 004c7751  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7752  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7754  833d603a7a0000         +cmp dword ptr [0x7a3a60], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010336) /* 0x7a3a60 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c775b  7422                   -je 0x4c777f
    if (cpu.flags.zf)
    {
        goto L_0x004c777f;
    }
    // 004c775d  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c775f  740f                   -je 0x4c7770
    if (cpu.flags.zf)
    {
        goto L_0x004c7770;
    }
    // 004c7761  ba803e0000             -mov edx, 0x3e80
    cpu.edx = 16000 /*0x3e80*/;
    // 004c7766  b85c5f7d00             -mov eax, 0x7d5f5c
    cpu.eax = 8216412 /*0x7d5f5c*/;
    // 004c776b  e89c8f0100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c7770:
    // 004c7770  ba00080000             -mov edx, 0x800
    cpu.edx = 2048 /*0x800*/;
    // 004c7775  b85c577d00             -mov eax, 0x7d575c
    cpu.eax = 8214364 /*0x7d575c*/;
    // 004c777a  e88d8f0100             -call 0x4e070c
    cpu.esp -= 4;
    nfs3hp::fillZero(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
L_0x004c777f:
    // 004c777f  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7780  5a                     -pop edx
    cpu.edx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c7781  c3                     -ret 
    cpu.esp += 4;
    return;
}

/* sub_4c7790: native, nfs3hp::vertices::depthBuckets */

/* align: skip 0x8d 0x80 0x00 0x00 0x00 0x00 0x8d 0x92 0x00 0x00 0x00 0x00 */
void Application::sub_4c7940(WinApplication* __restrict app, x86::CPU& cpu_)
{
  x86::Local cpu(cpu_);
  NFS2_USE(cpu);
  NFS2_USE(app);
    // 004c7940  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7941  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c7942  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c7943  55                     -push ebp
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ebp;
    cpu.esp -= 4;
    // 004c7944  89e5                   -mov ebp, esp
    cpu.ebp = cpu.esp;
    // 004c7946  83ec08                 -sub esp, 8
    (cpu.esp) -= x86::reg32(x86::sreg32(8 /*0x8*/));
    // 004c7949  8b4514                 -mov eax, dword ptr [ebp + 0x14]
    cpu.eax = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(20) /* 0x14 */);
    // 004c794c  85c0                   +test eax, eax
    cpu.clear_co();
    cpu.set_szp(static_cast<x86::reg32>(cpu.eax & cpu.eax));
    // 004c794e  7438                   -je 0x4c7988
    if (cpu.flags.zf)
    {
        goto L_0x004c7988;
    }
    // 004c7950  833d603a7a0000         +cmp dword ptr [0x7a3a60], 0
    {
        x86::reg32 tmp1 = app->getMemory<x86::reg32>(x86::reg32(8010336) /* 0x7a3a60 */);
        x86::reg32 tmp2 = x86::reg32(x86::sreg32(0 /*0x0*/));
        x86::reg32 result = tmp1 - tmp2;
        cpu.flags.cf = tmp1 < tmp2;
        cpu.flags.of = 1 & (tmp1 >> 31);
        cpu.flags.of ^= 1 & (result >> 31);
        cpu.flags.of &= (1 & (tmp1 >> 31)) != (1 & (tmp2 >> 31));
        cpu.set_szp(result);
    }
    // 004c7957  750a                   -jne 0x4c7963
    if (!cpu.flags.zf)
    {
        goto L_0x004c7963;
    }
    // 004c7959  ff751c                 -push dword ptr [ebp + 0x1c]
    app->getMemory<x86::reg32>(cpu.esp-4) = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(28) /* 0x1c */);
    cpu.esp -= 4;
    // 004c795c  e81ffbffff             -call 0x4c7480
    cpu.esp -= 4;
    sub_4c7480(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c7961  eb25                   -jmp 0x4c7988
    goto L_0x004c7988;
L_0x004c7963:
    // 004c7963  8b4d18                 -mov ecx, dword ptr [ebp + 0x18]
    cpu.ecx = app->getMemory<x86::reg32>(cpu.ebp + x86::reg32(24) /* 0x18 */);
    // 004c7966  51                     -push ecx
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.ecx;
    cpu.esp -= 4;
    // 004c7967  50                     -push eax
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.eax;
    cpu.esp -= 4;
    // 004c7968  8b35585f7d00           -mov esi, dword ptr [0x7d5f58]
    cpu.esi = app->getMemory<x86::reg32>(x86::reg32(8216408) /* 0x7d5f58 */);
    // 004c796e  56                     -push esi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.esi;
    cpu.esp -= 4;
    // 004c796f  8b3d545f7d00           -mov edi, dword ptr [0x7d5f54]
    cpu.edi = app->getMemory<x86::reg32>(x86::reg32(8216404) /* 0x7d5f54 */);
    // 004c7975  57                     -push edi
    app->getMemory<x86::reg32>(cpu.esp-4) = cpu.edi;
    cpu.esp -= 4;
    // 004c7976  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c7979  e84245ffff             -call 0x4bbec0
    cpu.esp -= 4;
    sub_4bbec0(app, cpu.sync());
    cpu.reload();
    if (cpu.terminate) return;
    // 004c797e  8d75f8                 -lea esi, [ebp - 8]
    cpu.esi = x86::reg32(cpu.ebp + x86::reg32(-8) /* -0x8 */);
    // 004c7981  bf545f7d00             -mov edi, 0x7d5f54
    cpu.edi = 8216404 /*0x7d5f54*/;
    // 004c7986  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
    // 004c7987  a5                     -movsd dword ptr es:[edi], dword ptr [esi]
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
L_0x004c7988:
    // 004c7988  89ec                   -mov esp, ebp
    cpu.esp = cpu.ebp;
    // 004c798a  5d                     -pop ebp
    cpu.ebp = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c798b  5f                     -pop edi
    cpu.edi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c798c  5e                     -pop esi
    cpu.esi = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c798d  59                     -pop ecx
    cpu.ecx = app->getMemory<x86::reg32>(cpu.esp);
    cpu.esp += 4;
    // 004c798e  c20c00                 -ret 0xc
    cpu.esp += 4+12 /*0xc*/;
    return;
}

/* sub_4c79a0: native, nfs3hp::vertices::gatherBuckets */

}
